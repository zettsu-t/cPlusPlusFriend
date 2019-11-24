#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <time.h>
#include <gtest/gtest.h>

namespace {
    using Count = uint64_t;
    using IntElement = uint32_t;
    using TrialCount = uint32_t;
    constexpr size_t BytesInYmmRegister = 32;

    enum class MethodToFill {
        ZEROS,
        ONES,
        SEQUENTIAL,
        RANDOM,
    };

    struct AlignedBuffer final {
        AlignedBuffer(size_t nElments, MethodToFill method) {
            // Check the Ymm registers alignment
            constexpr size_t nYmmRegisters = 4;
            size_t unit = BytesInYmmRegister * nYmmRegisters;
            unit /= sizeof(nElments);
            assert((nElments % unit) == 0);

            nElments_ = nElments;
            sizeInByte_ = sizeof(IntElement) * nElments;
            sizeOfYmmRegister_ = sizeInByte_ / BytesInYmmRegister;
            auto pData = aligned_alloc(BytesInYmmRegister, sizeInByte_);
            if (pData) {
                pData_ = static_cast<IntElement*>(pData);
                initialize(method);
            }
        }

        ~AlignedBuffer(void) {
            std::free(pData_);
            pData_ = nullptr;
        }

        void initialize(MethodToFill method) {
            Count count = 0;

            switch(method) {
            case MethodToFill::ONES:
                count = fillByOnes();
                break;
            case MethodToFill::SEQUENTIAL:
                count = fillSequential();
                break;
            case MethodToFill::RANDOM:
                count = fillRandom();
                break;
            case MethodToFill::ZEROS:
            default:
                count = fillByZeros();
                break;
            }

            actualCount_ = count;
        }

        Count fillByZeros(void) {
            std::fill(pData_, pData_ + nElments_, 0);
            return 0;
        }

        Count fillByOnes(void) {
            std::fill(pData_, pData_ + nElments_, std::numeric_limits<IntElement>::max());
            Count count = nElments_;
            count *= sizeof(IntElement);
            count *= 8;  // bits per byte
            return count;
        }

        Count fillSequential(void) {
            Count count = 0;
            for(size_t i=0; i<nElments_; ++i) {
                IntElement num = static_cast<IntElement>(i & std::numeric_limits<IntElement>::max());
                pData_[i] = num;
                count += static_cast<decltype(count)>(__builtin_popcount(num));
            }
            return count;
        }

        Count fillRandom(void) {
            std::random_device seedGen;
            std::default_random_engine engine(seedGen());
            std::uniform_int_distribution<IntElement> dist;

            Count count = 0;
            for(size_t i=0; i<nElments_; ++i) {
                auto num = dist(engine);
                pData_[i] = num;
                count += static_cast<decltype(count)>(__builtin_popcount(num));
            }
            return count;
        }

        size_t nElments_ {0};
        size_t sizeInByte_ {0};
        size_t sizeOfYmmRegister_ {0};
        IntElement* pData_ {nullptr};
        Count actualCount_ {0};
    };

    struct TimeResult {
        Count count;
        decltype(timespec::tv_nsec) timeInNsec;
    };

    template <typename Func>
    TimeResult MeasureTime(Func& func) {
        struct timespec startTimestamp;
        clock_gettime(CLOCK_MONOTONIC, &startTimestamp);
        auto count = func();
        struct timespec endTimestamp;
        clock_gettime(CLOCK_MONOTONIC, &endTimestamp);

        decltype(timespec::tv_nsec) timeDiff = endTimestamp.tv_sec - startTimestamp.tv_sec;
        timeDiff *= 1000000000;  // nsec per sec
        timeDiff += endTimestamp.tv_nsec;
        timeDiff -= startTimestamp.tv_nsec;

        TimeResult result {count, timeDiff};
        return result;
    }

    Count CountByAsm(const IntElement* pData, size_t sizeOfYmmRegister, TrialCount nTrial) {
        Count total = 0;
        for(auto trial = decltype(nTrial){0}; trial<nTrial; ++trial) {
            auto ptr = pData;
            auto size = sizeOfYmmRegister;
            Count count = 0;
            asm volatile (
                "call countPopulation\n\t"
                :"=a"(count),"+S"(ptr),"+c"(size)::"r15");
            total += count;
        }

        return total;
    }

    Count CountByIntrinsic(const IntElement* pData, size_t nElments, TrialCount nTrial) {
        Count total = 0;
        for(auto trial = decltype(nTrial){0}; trial<nTrial; ++trial) {
            for(size_t i=0; i<nElments; ++i) {
                total += static_cast<decltype(total)>(__builtin_popcount(pData[i]));
            }
        }

        return total;
    }
}

class TestPopulationCount : public ::testing::Test {
protected:
    void fillAndCount(size_t nElements, size_t width, MethodToFill method) {
        std::unique_ptr<AlignedBuffer> pBuf = std::make_unique<AlignedBuffer>(nElements, method);
        if (method == MethodToFill::ZEROS) {
            EXPECT_FALSE(pBuf->pData_[0]);
        } else if (method == MethodToFill::ONES) {
            auto element = pBuf->pData_[0];
            ++element;
            EXPECT_FALSE(element);
        }

        constexpr TrialCount nTrial = 1;
        EXPECT_EQ(pBuf->actualCount_, CountByAsm(pBuf->pData_, pBuf->sizeOfYmmRegister_, nTrial));
        EXPECT_EQ(pBuf->actualCount_, CountByIntrinsic(pBuf->pData_, pBuf->nElments_, nTrial));
    }

    void fillSequentialAndCount(size_t nElements, size_t width) {
        // Expects nElements is an integer 2^n
        ASSERT_FALSE(nElements & (nElements - 1));
        size_t expected = nElements * width;
        expected >>= 1;
        constexpr MethodToFill method = MethodToFill::SEQUENTIAL;
        std::unique_ptr<AlignedBuffer> pBuf = std::make_unique<AlignedBuffer>(nElements, method);
        EXPECT_EQ(expected, pBuf->actualCount_);

        constexpr TrialCount nTrial = 1;
        auto funcAsm = std::bind(CountByAsm, pBuf->pData_, pBuf->sizeOfYmmRegister_, nTrial);
        auto actual = MeasureTime(funcAsm);
        EXPECT_EQ(expected, actual.count);

        auto funcIntrinsic = std::bind(CountByIntrinsic, pBuf->pData_, pBuf->nElments_, nTrial);
        actual = MeasureTime(funcIntrinsic);
        EXPECT_EQ(expected, actual.count);
    }

    void fillRandomAndCount(size_t nElements, size_t width) {
        constexpr MethodToFill method = MethodToFill::RANDOM;
        std::unique_ptr<AlignedBuffer> pBuf = std::make_unique<AlignedBuffer>(nElements, method);

        constexpr TrialCount nTrial = 1;
        auto funcAsm = std::bind(CountByAsm, pBuf->pData_, pBuf->sizeOfYmmRegister_, nTrial);
        auto actual = MeasureTime(funcAsm);
        auto funcIntrinsic = std::bind(CountByIntrinsic, pBuf->pData_, pBuf->nElments_, nTrial);
        auto expected = MeasureTime(funcIntrinsic);
        EXPECT_EQ(pBuf->actualCount_, expected.count);
        EXPECT_EQ(expected.count, actual.count);
    }

    void measureTime(size_t width, TrialCount nTrial) {
        size_t nElements = 1;
        nElements <<= width;
        size_t nBytes = nElements * sizeof(IntElement);
        nBytes *= nTrial;
        size_t nBits = nBytes;
        nBits *= 8;

        constexpr MethodToFill method = MethodToFill::RANDOM;
        std::unique_ptr<AlignedBuffer> pBuf = std::make_unique<AlignedBuffer>(nElements, method);

        auto funcAsm = std::bind(CountByAsm, pBuf->pData_, pBuf->sizeOfYmmRegister_, nTrial);
        auto funcIntrinsic = std::bind(CountByIntrinsic, pBuf->pData_, pBuf->nElments_, nTrial);
        TimeResult asmResult {0, 0};
        TimeResult intrinsicResult {0, 0};

        // Throw away fitst execution due to the cold cache problem
        for(TrialCount loop = 0; loop < 2; ++loop) {
            asmResult = MeasureTime(funcAsm);
            intrinsicResult = MeasureTime(funcIntrinsic);
        }

        std::cout << "Counting 1s in " << nBytes << " bytes, " << nBits << " bits\n";
        std::cout << asmResult.timeInNsec << "[nsec], answer=" << asmResult.count << " : asm\n";
        std::cout << intrinsicResult.timeInNsec << "[nsec], answer=" << intrinsicResult.count << " : popcount\n";
        Count expected = pBuf->actualCount_;
        expected *= nTrial;
        EXPECT_EQ(expected, asmResult.count);
        EXPECT_EQ(expected, intrinsicResult.count);
    }
};

TEST_F(TestPopulationCount, Zeros) {
    for(size_t width = 5; width < 30; ++width) {
        size_t nElements = 1;
        nElements <<= width;
        fillAndCount(nElements, width, MethodToFill::ZEROS);
    }
}

TEST_F(TestPopulationCount, Ones) {
    for(size_t width = 5; width < 30; ++width) {
        size_t nElements = 1;
        nElements <<= width;
        fillAndCount(nElements, width, MethodToFill::ONES);
    }
}

TEST_F(TestPopulationCount, Sequential) {
    for(size_t width = 5; width < 30; width += 6) {
        size_t nElements = 1;
        nElements <<= width;
        fillSequentialAndCount(nElements, width);
    }
}

TEST_F(TestPopulationCount, Random) {
    for(size_t width = 5; width < 30; width += 3) {
        size_t nElements = 1;
        nElements <<= width;
        fillRandomAndCount(nElements, width);
    }
}

TEST_F(TestPopulationCount, MeasureTime1) {
    constexpr size_t width = 25;
    constexpr TrialCount nTrial = 2;
    measureTime(width, nTrial);
}

TEST_F(TestPopulationCount, MeasureTime2) {
    constexpr size_t width = 27;
    constexpr TrialCount nTrial = 2;
    measureTime(width, nTrial);
}

TEST_F(TestPopulationCount, MeasureTime3) {
    constexpr size_t width = 29;
    constexpr TrialCount nTrial = 2;
    measureTime(width, nTrial);
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
