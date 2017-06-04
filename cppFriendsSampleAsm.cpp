// やめるのだフェネックで学ぶC++の実証コード(インラインアセンブリ)
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/math/constants/constants.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <gtest/gtest.h>
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
        ProcessorClock clock = 0;
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

    using NallowClock = uint32_t;
    // コンテキストスイッチがなければ、これ以上の時刻差は観測されないはず
    constexpr NallowClock ClockThreshold = 4000;
    // Signedとみなして巨大な数なら時刻が逆転している
    constexpr NallowClock MaxDiff = 2 + std::numeric_limits<NallowClock>::max() / 2;

    // atomicにリアルタイムクロックを読まないと不適切な値が得られるという例
    inline NallowClock getProcessorClockBad(ProcessorClock& clock) {
        uint32_t mixed = 0;
        auto pMixed = &mixed;
        auto pClock = &clock;

        asm volatile (
            "rdtsc \n\t"  // 下位12bitを読む
            "andl $0xfff, %%eax \n\t"
            "movl %%eax,  %%ebx \n\t"
            "rdtsc \n\t"  // 上位bitを読む
            "movl %%eax, (%1)  \n\t"
            "movl %%edx, 4(%1) \n\t"
            "andl $0xfffff000, %%eax \n\t"
            "orl  %%ebx, %%eax \n\t"
            "movl %%eax, (%0)  \n\t"
            ::"r"(pMixed),"r"(pClock):"eax","ebx","edx","memory");

        return static_cast<NallowClock>(mixed);
    }

    NallowClock getProcessorClockGood(ProcessorClock& clock) {
        uint32_t mixed = 0;
        auto pMixed = &mixed;
        auto pClock = &clock;

        asm volatile (
            "1: \n\t"
            "rdtsc \n\t"  // 上位bitを読む
            "andl $0xfffff000, %%eax \n\t"
            "movl %%eax,  %%ecx \n\t"
            "rdtsc \n\t"  // 下位12bitを読む
            "andl $0xfff, %%eax \n\t"
            "movl %%eax,  %%edi \n\t"
            "rdtsc \n\t"  // もう一度上位bitを読む
            "movl %%eax, (%1)  \n\t"
            "movl %%edx, 4(%1) \n\t"
            "andl $0xfffff000, %%eax \n\t"
            "cmpl %%ecx,  %%eax \n\t"
            "jne  1b \n\t"  // 読み直す
            "orl  %%edi, %%eax \n\t"
            "movl %%eax, (%0)  \n\t"
            ::"r"(pMixed),"r"(pClock):"eax","ecx","edx","edi","memory");

        return static_cast<NallowClock>(mixed);
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

TEST_F(TestProcessorClock, Bad) {
    ProcessorClock previousClock = 0;
    auto previous = getProcessorClockBad(previousClock);
    bool found = false;

    // 1秒くらい、十分たくさん行うが、失敗する可能性がないとは言えない
    for(int i=0; i<0x4000000 && !found; ++i) {
        ProcessorClock currentClock = 0;
        auto current = getProcessorClockBad(currentClock);
        // Wrap around
        auto diff = current - previous;
        ASSERT_TRUE(diff);

        // クロックが大きく飛んだ時は、コンテキストスイッチが入ったとみなす
        if (ClockThreshold > (currentClock - previousClock)) {
            // 閾値より大きい差は、getProcessorClockで弾いているはず
            found |= (MaxDiff <= diff);
        }
        previous = current;
        previousClock = currentClock;
    }

    ASSERT_TRUE(found);
}

TEST_F(TestProcessorClock, Good) {
    ProcessorClock previousClock = 0;
    auto previous = getProcessorClockGood(previousClock);

    for(int i=0; i<0x2000000; ++i) {
        ProcessorClock currentClock = 0;
        auto current = getProcessorClockGood(currentClock);
        auto diff = current - previous;
        ASSERT_TRUE(diff);

        // クロックが大きく飛んだ時は、コンテキストスイッチが入ったとみなす
        if (ClockThreshold > (currentClock - previousClock)) {
            ASSERT_GT(MaxDiff, diff);
        }
        previous = current;
        previousClock = currentClock;
    }
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

class TestDivideBy2 : public ::testing::Test{};

/*
負の数に対する除算に対して、コンパイラがどんなコードを出力するか確認する
Cで書くとこうなる
  int divide_by_2(int src) {
      return src / 2;
  }

以下はGCCの出力である
  mov  eax, ecx
  shr  eax, 31
  add  eax, ecx
  sar  eax

以下はclang-LLVMの出力である
  mov  eax, ecx
  shr  eax, 31
  lea  eax, [rax + rcx]
  sar  eax

http://www.microhowto.info/howto/round_towards_minus_infinity_when_dividing_integers_in_c_or_c++.html
によると、割る数または割られる数の少なくとも一方が負の場合の丸めは実装依存である。
*/

TEST_F(TestDivideBy2, RoundToZero) {
    // 負の数を2で割るときに、単にsar 1回ではうまくいかない
    // -1を2で割っても-1になってしまう
    // 負の数で小数点以下切り捨て(round to 0)なら1足してからシフトする必要がある
    // -8 : 1000b <+1> 1001b <sar> 1100b (-4)
    // -7 : 1001b <+1> 1010b <sar> 1101b (-3)
    // -2 : 1110b <+1> 1111b <sar> 1111b (-1)
    // -1 : 1111b <+1> 0 <sar> 0
    // 正の数には1足さないので、単にシフトしてLSBを捨てるだけである

    struct TestCase {
        int32_t arg;
        int32_t expected;
        int32_t expectedShifted;
    };

    const TestCase testCaseSet[] = {
        {0, 0, 0},
        {1, 0, 0},
        {2, 1, 1},
        {3, 1, 1},
        {4, 2, 2},
        {7, 3, 3},
        {8, 4, 4},
        {9, 4, 4},
        { 2147483645,  1073741822,  1073741822},
        { 2147483646,  1073741823,  1073741823},
        { 2147483647,  1073741823,  1073741823},
        {-2147483648, -1073741824, -1073741824},
        {-2147483647, -1073741823, -1073741824},
        {-2147483646, -1073741823, -1073741823},
        {-9, -4, -5},
        {-8, -4, -4},
        {-7, -3, -4},
        {-4, -2, -2},
        {-3, -1, -2},
        {-2, -1, -1},
        {-1,  0, -1}
    };

    static_assert(std::numeric_limits<int32_t>::min() == -2147483648, "");
    static_assert(std::numeric_limits<int32_t>::max() == 2147483647, "");

    for(const auto& testCase : testCaseSet) {
        auto arg = testCase.arg;
        decltype(arg) actual = 0;

        // 最近のCPUなら、3オペレータシフト命令を使える
        asm volatile (
            "movl  %%ecx, %%eax \n\t"  // 第一引数 = ecx
            "shrl  $31,   %%eax \n\t"  // 第一引数が非負なら0, 負なら1
            "addl  %%ecx, %%eax \n\t"  // 第一引数が非負ならそのまま, 負なら1足したものを
            "sarl  %%eax        \n\t"  // 符号を保ったままシフトする
            :"=a"(actual):"c"(arg):);

        EXPECT_EQ(testCase.expected, actual);
        EXPECT_EQ(testCase.expected, testCase.arg / 2);

        // 負の数では、/2 を負の無限大方向に切り上げるのと同じになる
        // それはそれで正しいのかもしれないが、-1を2で割って-1になるのは直観に反する
        auto shifted = testCase.arg;
        shifted >>= 1;
        EXPECT_EQ(testCase.expectedShifted, shifted);
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
