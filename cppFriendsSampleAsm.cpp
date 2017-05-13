// やめるのだフェネックで学ぶC++の実証コード(インラインアセンブリ)
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/math/constants/constants.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "cFriendsCommon.h"
#include "cppFriends.hpp"

class TestSaturationArithmetic : public ::testing::Test{};

TEST_F(TestSaturationArithmetic, Add) {
    constexpr size_t XmmRegisterSizeInByte = 16;
    constexpr size_t NumberOfRegister = 2;
    std::aligned_storage<XmmRegisterSizeInByte, XmmRegisterSizeInByte>::type xmmRegisters[NumberOfRegister];
    const uint8_t presetValue[XmmRegisterSizeInByte * NumberOfRegister] = {
        0,   0, 1, 1,   1,   1, 2, 2,   2,   2,   2, 254, 254, 254, 255, 255,   // 足される数
        0, 255, 0, 1, 254, 255, 0, 1, 253, 254, 255,   0,   1,   2,   0,   1    // 足す数
    };

    static_assert(sizeof(xmmRegisters) >= sizeof(presetValue), "Too large");
    ::memmove(xmmRegisters, presetValue, sizeof(presetValue));

    const uint8_t expected[XmmRegisterSizeInByte] = {
        0, 255, 1, 2, 255, 255, 2, 3, 255, 255, 255, 254, 255, 255, 255, 255};  // 和

    // Windows/Linuxの両方で使えるscratch registers
    asm volatile (
        "movdqa  (%0),   %%xmm4 \n\t"
        "movdqa  16(%0), %%xmm5 \n\t"
        "paddusb %%xmm5, %%xmm4 \n\t"
        "movdqa  %%xmm4, (%0)    \n\t"
        ::"r"(xmmRegisters):"memory");

    static_assert(sizeof(expected) <= sizeof(xmmRegisters), "Too large");
    EXPECT_EQ(0, ::memcmp(expected, xmmRegisters, sizeof(expected)));
}

TEST_F(TestSaturationArithmetic, Sub) {
    constexpr size_t XmmRegisterSizeInByte = 16;
    constexpr size_t NumberOfRegister = 2;
    std::aligned_storage<XmmRegisterSizeInByte, XmmRegisterSizeInByte>::type xmmRegisters[NumberOfRegister];
    const uint8_t presetValue[XmmRegisterSizeInByte * NumberOfRegister] = {
        0,   0, 0,   0,   0, 1, 1, 1, 254, 254, 254, 254, 255, 255, 255, 255,  // 引かれる数
        0,   1, 2, 254, 255, 0, 1, 2,   1, 253, 254, 255,   0,   1, 254, 255   // 引く数
    };
    static_assert(sizeof(xmmRegisters) >= sizeof(presetValue), "Too large");
    ::memmove(xmmRegisters, presetValue, sizeof(presetValue));

    const uint8_t expected[XmmRegisterSizeInByte] = {
        0,   0, 0,   0,   0, 1, 0, 0, 253,   1,   0,   0, 255, 254,   1,   0}; // 差

    // Windows/Linuxの両方で使えるscratch registers
    asm volatile (
        "movdqa  (%0),   %%xmm4 \n\t"
        "movdqa  16(%0), %%xmm5 \n\t"
        "psubusb %%xmm5, %%xmm4 \n\t"
        "movdqa  %%xmm4, (%0)    \n\t"
        ::"r"(xmmRegisters):"memory");

    static_assert(sizeof(expected) <= sizeof(xmmRegisters), "Too large");
    EXPECT_EQ(0, ::memcmp(expected, xmmRegisters, sizeof(expected)));
}


namespace {
    using ProcessorClock = uint64_t;
    using ProcessorClockSet = std::vector<ProcessorClock>;
    using ProcessorClockMap = std::unordered_map<ProcessorClock, size_t>;
    std::string g_expString;

    ProcessorClock getProcessorClock() {
        uint64_t clock = 0;
        auto pClock = &clock;

        // クロックだけ測る
        asm volatile (
            "rdtsc \n\t"
            "mov %%eax, (%0)  \n\t"
            "mov %%edx, 4(%0) \n\t"
            ::"r"(pClock):"eax", "edx", "memory");
        return clock;
    }

    ProcessorClock getProcessorClockWithLoad() {
        // 負荷を掛ける
        using LongFloat = boost::multiprecision::number<boost::multiprecision::cpp_dec_float<50>>;
        g_expString += boost::math::constants::e<LongFloat>().str().at(0);
        return getProcessorClock();
    }

    using FuncGetClock = std::function<ProcessorClock()>;
    void checkProcessorClock(FuncGetClock& f) {
        ProcessorClockSet clockSet;
        for(size_t i = 0; i < 120000; ++i) {
            clockSet.push_back(f());
        }

        // 周回しないものと仮定している
        ProcessorClockMap clockMap;
        constexpr ProcessorClock mod = 12;

        for(ProcessorClock i=0; i<mod; ++i) {
            clockMap[i] = 0;
        }

        decltype(clockSet)::value_type prevTimestamp = clockSet.at(0);
        for(auto timestamp : clockSet) {
            ASSERT_LE(prevTimestamp, timestamp);
            clockMap[timestamp % mod] += 1;
            prevTimestamp = timestamp;
        }

        // 出現確率が二項分布なら、標準偏差は sqrt(n*p*(1-p))になる
        // p=1/12なら、1σ = sqrt(n) * 0.28, 6σ = sqrt(n) * 1.66
        // n=120000なら、平均=10000, 6σ=575なので、9000回は出るはず
        // 9000回に満たないのはおかしい
        bool foundBias = false;
        for(ProcessorClock i=0; i<mod; ++i) {
            auto count = clockMap[i];
            std::cout << count << ":";
            foundBias |= (count < 9000);
        }

        EXPECT_TRUE(foundBias);
        std::cout << "\n";
        return;
    }
}

class TestProcessorClock : public ::testing::Test{
    virtual void SetUp() override {
        g_expString.clear();
    }

    virtual void TearDown() override {
        g_expString.clear();
    }
};

TEST_F(TestProcessorClock, Light) {
    FuncGetClock f(getProcessorClock);
    checkProcessorClock(f);
}

TEST_F(TestProcessorClock, Heavy) {
    FuncGetClock f(getProcessorClockWithLoad);
    checkProcessorClock(f);
}

namespace {
    using Log2Arg = unsigned int;
    using Log2Result = int;
    constexpr Log2Result Log2ResultInvalid = -1;

    Log2Result log2Builtin(Log2Arg arg) {
        return (arg) ? (31 - __builtin_clz(arg)) : Log2ResultInvalid;
    }

    Log2Result log2Asm(Log2Arg arg) {
        Log2Result result = 0;
        Log2Result invalid = Log2ResultInvalid;

        asm volatile (
            "bsr    %1, %0 \n\t"
            "cmovz  %2, %0 \n\t"
            :"=r"(result):"r"(arg),"r"(invalid):);

        return result;
    }

    struct Log2TestCase {
        Log2Arg arg;
        Log2Result expected;
    };

    const Log2TestCase g_log2TestCaseSet [] = {
        {0, Log2ResultInvalid},
        {1,0}, {2,1}, {3,1}, {4,2}, {5,2}, {6,2}, {7,2},
        {8,3}, {9,3}, {14,3}, {15,3}, {16,4}, {17,4}, {31,4},
        {0x7fffffffu, 30}, {0x80000000u, 31}, {0xffffffffu, 31}
    };
}

class TestLog2 : public ::testing::Test{};

TEST_F(TestLog2, Builtin) {
    for(auto& testcase : g_log2TestCaseSet) {
        EXPECT_EQ(testcase.expected, log2Builtin(testcase.arg));
    }
}

TEST_F(TestLog2, Asm) {
    for(auto& testcase : g_log2TestCaseSet) {
        EXPECT_EQ(testcase.expected, log2Asm(testcase.arg));
    }
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
