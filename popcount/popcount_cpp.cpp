#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <utility>
#include <time.h>
#include <gtest/gtest.h>

namespace {
    using Count = uint64_t;
    using TrialCount = uint64_t;
    constexpr size_t BytesInYmmRegister = 32;

    enum class MethodToFill {
        ZEROS,
        ONES,
        SEQUENTIAL,
        RANDOM,
    };

    template <typename T, typename std::enable_if_t<std::is_same_v<T, uint32_t>, std::nullptr_t> = nullptr>
    auto BuitinPopcount(const T& num) {
        return __builtin_popcount(num);
    }

    template <typename T, typename std::enable_if_t<std::is_same_v<T, uint64_t>, std::nullptr_t> = nullptr>
    auto BuitinPopcount(const T& num) {
        return __builtin_popcountll(num);
    }

    template<typename ElementType>
    struct AlignedBuffer final {
        AlignedBuffer(size_t nElments, MethodToFill method) {
            // Check the Ymm registers alignment
            constexpr size_t nYmmRegisters = 4;
            size_t unit = BytesInYmmRegister * nYmmRegisters;
            unit /= sizeof(nElments);
            assert((nElments > 0) && (nElments % unit) == 0);

            nElments_ = nElments;
            nElments32_ = nElments * sizeof(ElementType) / sizeof(uint32_t);
            sizeInByte_ = sizeof(ElementType) * nElments;
            sizeOfYmmRegister_ = sizeInByte_ / BytesInYmmRegister;
            void* pData = aligned_alloc(BytesInYmmRegister, sizeInByte_);
            if (pData) {
                pData_ = static_cast<decltype(pData_)>(pData);
                static_assert((alignof(std::remove_pointer_t<decltype(pData_)>) % alignof(std::remove_pointer_t<decltype(pData32_)>)) == 0);
                // May not aligned
                pData32_ = static_cast<decltype(pData32_)>(pData);
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
            std::fill(pData_, pData_ + nElments_, std::numeric_limits<ElementType>::max());
            Count count = nElments_;
            count *= sizeof(ElementType);
            count *= 8;  // bits per byte
            return count;
        }

        Count fillSequential(void) {
            Count count = 0;
            for(size_t i=0; i<nElments_; ++i) {
                ElementType num = static_cast<ElementType>(i & std::numeric_limits<ElementType>::max());
                pData_[i] = num;
                count += static_cast<decltype(count)>(BuitinPopcount(num));
            }
            return count;
        }

        Count fillRandom(void) {
            std::random_device seedGen;
            std::default_random_engine engine(seedGen());
            std::uniform_int_distribution<ElementType> dist;

            Count count = 0;
            for(size_t i=0; i<nElments_; ++i) {
                auto num = dist(engine);
                pData_[i] = num;
                count += static_cast<decltype(count)>(BuitinPopcount(num));
            }
            return count;
        }

        size_t nElments_ {0};
        size_t nElments32_ {0};
        size_t sizeInByte_ {0};
        size_t sizeOfYmmRegister_ {0};
        ElementType* pData_ {nullptr};
        uint32_t* pData32_ {nullptr};
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

    template<typename ElementType>
    Count CountByAsm(const ElementType* pData, size_t sizeOfYmmRegister, TrialCount nTrial) {
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

    template<typename ElementType>
    Count CountByIntrinsic(const ElementType* pData, size_t nElments, TrialCount nTrial) {
        Count total = 0;
        for(auto trial = decltype(nTrial){0}; trial<nTrial; ++trial) {
            for(size_t i=0; i<nElments; ++i) {
                total += static_cast<decltype(total)>(BuitinPopcount(pData[i]));
            }
        }

        return total;
    }
}

template <typename ElementType>
class TestPopulationCount : public ::testing::Test {
protected:
    size_t getSizeWidth(void) {
        size_t width = 0;
        size_t size = sizeof(ElementType);
        while(size) {
            ++width;
            size >>= 1;
        }
        width += 2;
        return width;
    }

    void fillAndCount(size_t nElements, MethodToFill method) {
        using BufType = AlignedBuffer<ElementType>;
        std::unique_ptr<BufType> pBuf = std::make_unique<BufType>(nElements, method);
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
        using BufType = AlignedBuffer<ElementType>;
        std::unique_ptr<BufType> pBuf = std::make_unique<BufType>(nElements, method);
        EXPECT_EQ(expected, pBuf->actualCount_);

        constexpr TrialCount nTrial = 1;
        auto funcAsm = std::bind(CountByAsm<ElementType>, pBuf->pData_, pBuf->sizeOfYmmRegister_, nTrial);
        auto actual = MeasureTime(funcAsm);
        EXPECT_EQ(expected, actual.count);

        auto funcIntrinsic = std::bind(CountByIntrinsic<ElementType>, pBuf->pData_, pBuf->nElments_, nTrial);
        actual = MeasureTime(funcIntrinsic);
        EXPECT_EQ(expected, actual.count);
    }

    void fillRandomAndCount(size_t nElements) {
        constexpr MethodToFill method = MethodToFill::RANDOM;
        using BufType = AlignedBuffer<ElementType>;
        std::unique_ptr<BufType> pBuf = std::make_unique<BufType>(nElements, method);

        constexpr TrialCount nTrial = 1;
        auto funcAsm = std::bind(CountByAsm<ElementType>, pBuf->pData_, pBuf->sizeOfYmmRegister_, nTrial);
        auto actual = MeasureTime(funcAsm);
        auto funcIntrinsic = std::bind(CountByIntrinsic<ElementType>, pBuf->pData_, pBuf->nElments_, nTrial);
        auto expected = MeasureTime(funcIntrinsic);
        EXPECT_EQ(pBuf->actualCount_, expected.count);
        EXPECT_EQ(expected.count, actual.count);
    }
};

typedef ::testing::Types<uint32_t, uint64_t> ElementTypeSet;
TYPED_TEST_SUITE(TestPopulationCount, ElementTypeSet);

TYPED_TEST(TestPopulationCount, Zeros) {
    for(size_t sizeBitWidth = 10; sizeBitWidth < 35; ++sizeBitWidth) {
        size_t nElements = 1;
        nElements <<= (sizeBitWidth - this->getSizeWidth());
        this->fillAndCount(nElements, MethodToFill::ZEROS);
    }
}

TYPED_TEST(TestPopulationCount, Ones) {
    for(size_t sizeBitWidth = 10; sizeBitWidth < 35; ++sizeBitWidth) {
        size_t nElements = 1;
        nElements <<= (sizeBitWidth - this->getSizeWidth());
        this->fillAndCount(nElements, MethodToFill::ONES);
    }
}

TYPED_TEST(TestPopulationCount, Sequential) {
    for(size_t sizeBitWidth = 10; sizeBitWidth < 35; sizeBitWidth += 6) {
        size_t nElements = 1;
        auto elementSizeWidth = sizeBitWidth - this->getSizeWidth();
        nElements <<= elementSizeWidth;
        this->fillSequentialAndCount(nElements, elementSizeWidth);
    }
}

TYPED_TEST(TestPopulationCount, Random) {
    for(size_t sizeBitWidth = 10; sizeBitWidth < 35; sizeBitWidth += 4) {
        size_t nElements = 1;
        nElements <<= (sizeBitWidth - this->getSizeWidth());
        this->fillRandomAndCount(nElements);
    }
}

namespace {
    void measureTime(size_t sizeBitWidth, TrialCount nTrial) {
        using ElementType64 = uint64_t;
        size_t nBytes = 1;
        nBytes <<= sizeBitWidth;
        size_t nBits = nBytes;
        nBits *= 8;
        size_t nElements = nBytes;
        nElements /= sizeof(ElementType64);

        constexpr MethodToFill method = MethodToFill::RANDOM;
        using BufType = AlignedBuffer<ElementType64>;
        std::unique_ptr<BufType> pBuf = std::make_unique<BufType>(nElements, method);

        using ElementType32 = uint32_t;
        auto funcAsm = std::bind(CountByAsm<ElementType64>, pBuf->pData_, pBuf->sizeOfYmmRegister_, nTrial);
        auto funcIntrinsic64 = std::bind(CountByIntrinsic<ElementType64>, pBuf->pData_, pBuf->nElments_, nTrial);
        auto funcIntrinsic32 = std::bind(CountByIntrinsic<ElementType32>, pBuf->pData32_, pBuf->nElments32_, nTrial);
        TimeResult asmResult {0, 0};
        TimeResult intrinsicResult64 {0, 0};
        TimeResult intrinsicResult32 {0, 0};

        // Throw away fitst execution due to the cold cache problem
        for(TrialCount loop = 0; loop < 2; ++loop) {
            asmResult = MeasureTime(funcAsm);
            intrinsicResult64 = MeasureTime(funcIntrinsic64);
            intrinsicResult32 = MeasureTime(funcIntrinsic32);
        }

        std::cout << "Counting 1s in " << nBytes << " bytes, " << nBits << " bits\n";
        std::cout << intrinsicResult64.timeInNsec << "[nsec], answer=" << intrinsicResult64.count << " : popcnt64\n";
        std::cout << asmResult.timeInNsec << "[nsec], answer=" << asmResult.count << " : SIMD asm\n";
        std::cout << intrinsicResult32.timeInNsec << "[nsec], answer=" << intrinsicResult32.count << " : popcnt32\n";

        Count expected = pBuf->actualCount_;
        expected *= nTrial;
        EXPECT_EQ(expected, asmResult.count);
        EXPECT_EQ(expected, intrinsicResult64.count);
        EXPECT_EQ(expected, intrinsicResult32.count);
    }
}

class TestTimeToCountPopulation : public ::testing::Test {};

TEST_F(TestTimeToCountPopulation, MeasureTime1) {
    constexpr size_t sizeBitWidth = 25;
    constexpr TrialCount nTrial = 2;
    measureTime(sizeBitWidth, nTrial);
}

TEST_F(TestTimeToCountPopulation, MeasureTime2) {
    constexpr size_t sizeBitWidth = 28;
    constexpr TrialCount nTrial = 2;
    measureTime(sizeBitWidth, nTrial);
}

TEST_F(TestTimeToCountPopulation, MeasureTime3) {
    constexpr size_t sizeBitWidth = 31;
    constexpr TrialCount nTrial = 2;
    measureTime(sizeBitWidth, nTrial);
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
