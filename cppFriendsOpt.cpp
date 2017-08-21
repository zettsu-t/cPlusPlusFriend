// 最適化の有無で結果が変わるテストと
// デバッガから実行したかどうかで結果が変わるテスト
#include <cstdint>
#include <algorithm>
#include <atomic>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include <windows.h>
#include "cFriendsCommon.h"
#include "cppFriends.hpp"

class MyCounter {
public:
    using Number = int;
    explicit MyCounter(Number count) : count_(count) {}
    virtual ~MyCounter(void) = default;

    void Run(void) {
        // コンパイラが最適化すれば、足した結果をまとめてnonVolatileCounter_に格納する可能性がある
        for(Number i = 0; i < count_; ++i) {
            ++nonVolatileCounter_;
        }

        // volatileはatomicではないので競合する可能性がある
        for(Number i = 0; i < count_; ++i) {
            ++volatileCounter_;
            ++atomicCounter_;
        }
    }

    static void Reset(void) {
        nonVolatileCounter_ = 0;
        volatileCounter_ = 0;
        atomicCounter_ = 0;
    }

    static Number GetValue(void) {
        return nonVolatileCounter_;
    }

    static Number GetVolatileValue(void) {
        return volatileCounter_;
    }

    static Number GetAtomicValue(void) {
        return atomicCounter_;
    }

private:
    Number count_ {0};
    static Number nonVolatileCounter_;
    static volatile Number volatileCounter_;
    static std::atomic<Number> atomicCounter_;
};

MyCounter::Number MyCounter::nonVolatileCounter_ {0};
volatile MyCounter::Number MyCounter::volatileCounter_ {0};
std::atomic<MyCounter::Number> MyCounter::atomicCounter_ {0};

class TestOptMyCounter : public ::testing::Test {
protected:
    using SizeOfThreads = int;

    virtual void SetUp() override {
        MyCounter::Reset();
        hardwareConcurrency_ = static_cast<decltype(hardwareConcurrency_)>(std::thread::hardware_concurrency());
        processAffinityMask_ = 0;
        systemAffinityMask_ = 0;

        // https://msdn.microsoft.com/ja-jp/library/cc429135.aspx
        // https://msdn.microsoft.com/ja-jp/library/windows/desktop/ms683213(v=vs.85).aspx
        // で引数の型が異なる。下が正しい。
        if (!GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask_, &systemAffinityMask_)) {
            // 取得に失敗したらシングルコアとみなす
            std::cout << "GetProcessAffinityMask failed\n";
            processAffinityMask_ = 1;
            systemAffinityMask_ = 1;
        }
    }

    virtual void TearDown() override {
        if (!SetProcessAffinityMask(GetCurrentProcess(), processAffinityMask_)) {
            std::cout << "Restoring ProcessAffinityMask failed\n";
        } else {
            std::cout << "ProcessAffinityMask restored\n";
        }
    }

    void runCounters(SizeOfThreads sizeOfThreads, MyCounter::Number count) {
        std::vector<std::unique_ptr<MyCounter>> counterSet;
        std::vector<std::future<void>> futureSet;

        // 並行処理を作って、後で実行できるようにする
        for(decltype(sizeOfThreads) index = 0; index < sizeOfThreads; ++index) {
            auto pCounter = std::make_unique<MyCounter>(count);
            MyCounter* pRawCounter = pCounter.get();
            futureSet.push_back(std::async(std::launch::async, [=](void) -> void { pRawCounter->Run(); }));
            counterSet.push_back(std::move(pCounter));
        }

        // 並行して評価して、結果がそろうのを待つ
        for(auto& f : futureSet) {
            f.get();
        }

        std::cout << "MyCounter::GetValue() = " << MyCounter::GetValue() << "\n";
        std::cout << "MyCounter::GetVolatileValue() = " << MyCounter::GetVolatileValue() << "\n";
        std::cout << "MyCounter::GetAtomicValue() = " << MyCounter::GetAtomicValue() << "\n";
        return;
    }

    class IntBox {
    public:
        using Data = int;
        explicit IntBox(Data n) : var_(n), mvar_(n), cvar_(n) {}
        virtual ~IntBox() = default;

        Data Get(void) const { return var_; }
        Data GetMutableVar(void) const { return mvar_; }
        Data GetConstVar(void) const { return cvar_; }
        void Set(Data n) { var_ = n; }
        void SetMutableVar(Data n) const { mvar_ = n; }

        // コンパイラはメンバ変数が不変だとは断定できないので、動作するだろう
        // Exceptional C++ Style 項目24を参照
        void Overwrite1(Data n) const {
            const_cast<IntBox*>(this)->var_ = n;
        }

        void Overwrite2(Data n) const {
            *(const_cast<Data*>(&var_)) = n;
            *(const_cast<Data*>(&cvar_)) = n;
        }

    private:
        Data var_;
        mutable Data mvar_;
        const Data cvar_;
    };

    SizeOfThreads hardwareConcurrency_ {0};

private:
    DWORD_PTR processAffinityMask_ {1};
    DWORD_PTR systemAffinityMask_  {1};
};

TEST_F(TestOptMyCounter, MultiCore) {
    if (hardwareConcurrency_ <= 1) {
        return;
    }

    const MyCounter::Number count = 4000000;  // 十分大きくする
    SizeOfThreads sizeOfThreads = hardwareConcurrency_;
    std::cout << "Run on " << sizeOfThreads << " threads\n";
    runCounters(sizeOfThreads, count);

    MyCounter::Number expected = static_cast<decltype(expected)>(sizeOfThreads) * count;
    // 最適化しなければこうはならない
#ifdef CPPFRIENDS_NO_OPTIMIZATION
    // 偶然割り切れることもあり得なくもないが
    EXPECT_TRUE(MyCounter::GetValue() % count);
    EXPECT_GT(expected, MyCounter::GetValue());
#else
    EXPECT_FALSE(MyCounter::GetValue() % count);
#if (__GNUC__ < 6)
    EXPECT_EQ(expected, MyCounter::GetValue());
    // GCC6では物理コア数分しか増えない
    // 2コア4論理スレッドなら、4倍ではなく2倍になるということ
    // 競合動作なのだから、そもそも正しいexpectationは書けない
#endif
#endif
    // マルチコアなら競合が起きるので成り立つはずだが、条件次第では競合しない可能性もある
    EXPECT_GT(expected, MyCounter::GetVolatileValue());
    // これは常に成り立つはず
    EXPECT_EQ(expected, MyCounter::GetAtomicValue());
}

TEST_F(TestOptMyCounter, SingleCore) {
    /* 使うCPUを1個に固定する */
    DWORD_PTR procMask = 1;
    if (!SetProcessAffinityMask(GetCurrentProcess(), procMask)) {
        std::cout << "SetProcessAffinityMask failed\n";
    }

    SizeOfThreads sizeOfThreads = std::max(hardwareConcurrency_, 4);
    const MyCounter::Number count = 4000000;
    std::cout << "Run on a single thread\n";
    runCounters(sizeOfThreads, count);

    MyCounter::Number expected = static_cast<decltype(expected)>(sizeOfThreads) * count;
    EXPECT_EQ(expected, MyCounter::GetValue());
    // シングルコアなら競合しない
    EXPECT_EQ(expected, MyCounter::GetVolatileValue());
    // これは常に成り立つはず
    EXPECT_EQ(expected, MyCounter::GetAtomicValue());
}

TEST_F(TestOptMyCounter, ConstMemberFunction) {
    IntBox::Data n = 1;
    IntBox box {n};
    EXPECT_EQ(n, box.Get());
    EXPECT_EQ(n, box.GetMutableVar());
    EXPECT_EQ(n, box.GetConstVar());

    ++n;
    box.Set(n);
    EXPECT_EQ(n, box.Get());

    ++n;
    box.SetMutableVar(n);
    EXPECT_EQ(n, box.GetMutableVar());

    ++n;
    box.Overwrite1(n);
    EXPECT_EQ(n, box.Get());

    ++n;
    box.Overwrite2(n);
    EXPECT_EQ(n, box.Get());
    EXPECT_EQ(n, box.GetConstVar());
}

class TestOptShifter : public ::testing::Test{};

TEST_F(TestOptShifter, All) {
    constexpr int32_t var = 2;
    int32_t result = -1;
    constexpr int32_t expected = var << 3;

    // 35回のはずが3回しかシフトされないのは、シフト回数がCLレジスタの値 mod 32だから
    Shift35ForInt32Asm(result, var);

#ifdef CPPFRIENDS_NO_OPTIMIZATION
    // 0でもexpectedでもない値が返る
    // EXPECT_EQ(expected, result);
    EXPECT_EQ(expected, Shift35ForInt32(var));
#else
    // N-bit整数をNビット以上シフトしたときの値は未定義なので、
    // コンパイラは35回シフトして0になったと考えるが、CPUの動作とは一致しない
    EXPECT_EQ(expected, result);
    EXPECT_EQ(0, Shift35ForInt32(var));
#endif

    EXPECT_EQ(expected, ShiftForInt32(var, CPPFRIENDS_SHIFT_COUNT));
}

namespace {
    size_t g_objDeleteCount = 0;

    class ObjectHolded {
    public:
        using Value = int;
        ObjectHolded(Value arg) : value_(arg) {}
        virtual ~ObjectHolded(void) { ++g_objDeleteCount; }
        Value GetValue(void) const { return value_; }
    private:
        Value value_ {0};
    };

    class RawPointerHolder {
    public:
        RawPointerHolder(ObjectHolded* pObj) : pObj_(pObj) {}

        virtual ~RawPointerHolder(void) {
            delete pObj_;
            pObj_ = nullptr;
        }

        ObjectHolded::Value GetValue(void) const {
            return pObj_->GetValue();
        }

    private:
        ObjectHolded* pObj_ {nullptr};
    };
}

class TestOptReleaseUniquePtr : public ::testing::Test {
    virtual void SetUp() override {
        g_objDeleteCount = 0;
    }
};

TEST_F(TestOptReleaseUniquePtr, UniquePtr) {
    EXPECT_EQ(0, g_objDeleteCount);
    {
        constexpr ObjectHolded::Value value = 23;
        auto pObj = std::make_unique<ObjectHolded>(value);
        EXPECT_EQ(value, pObj->GetValue());
        EXPECT_EQ(0, g_objDeleteCount);
    }
    EXPECT_EQ(1, g_objDeleteCount);
}

TEST_F(TestOptReleaseUniquePtr, Reset) {
    constexpr ObjectHolded::Value value = 34;
    auto pObj = std::make_unique<ObjectHolded>(value);
    EXPECT_EQ(0, g_objDeleteCount);
    EXPECT_EQ(value, pObj->GetValue());
    pObj.reset();
    EXPECT_EQ(1, g_objDeleteCount);
}

TEST_F(TestOptReleaseUniquePtr, Holder) {
    constexpr ObjectHolded::Value value = 45;
    auto pObj = std::make_unique<ObjectHolded>(value);
    EXPECT_EQ(0, g_objDeleteCount);
    {
        RawPointerHolder holder(pObj.get());
        // 所有権は放棄するが破棄はしない
        pObj.release();
        EXPECT_EQ(0, g_objDeleteCount);
        EXPECT_EQ(value, holder.GetValue());
    }
    EXPECT_EQ(1, g_objDeleteCount);
}

namespace {
    std::atomic<bool> g_breakPointHandled;

    LONG CALLBACK MyVectoredHandler(PEXCEPTION_POINTERS pExceptionInfo) {
        if (pExceptionInfo &&
            pExceptionInfo->ContextRecord &&
            pExceptionInfo->ExceptionRecord &&
            pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT) {
            // 次の命令に移る
#ifdef __x86_64__
            pExceptionInfo->ContextRecord->Rip++;
#else
            pExceptionInfo->ContextRecord->Eip++;
#endif
            g_breakPointHandled = true;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

class TestDebuggerDetecter : public ::testing::Test {
protected:
    virtual void SetUp() override {
        g_breakPointHandled = false;
    }
};

TEST_F(TestDebuggerDetecter, Trap) {
    EXPECT_FALSE(g_breakPointHandled.load());
    // ハンドラを登録する。最後に呼ばれるようにする。
    PVOID handler = ::AddVectoredExceptionHandler(0, MyVectoredHandler);
    ASSERT_TRUE(handler);

    // Int3を発生させる
    ::DebugBreak();
    // ハンドラを元に戻す
    ::RemoveVectoredExceptionHandler(handler);
    handler = 0;
    // デバッガから起動したときには、ハンドラに制御が渡らないのでFALSEになる
    EXPECT_TRUE(g_breakPointHandled.load());
}

class TestOverwriteVtable : public ::testing::Test {};

TEST_F(TestOverwriteVtable, Standard) {
    auto pObj = std::make_unique<SubDynamicObjectMemFunc>(2,3);
    std::ostringstream os;
    pObj->Print(os);
    EXPECT_EQ("23", os.str());

    ExtraMemFunc ex;
    ex.Print(os);
    EXPECT_EQ("23Extra", os.str());
}

#if 0
TEST_F(TestOverwriteVtable, Overwrite) {
    auto pObj = std::make_unique<SubDynamicObjectMemFunc>(2,3);
    std::ostringstream os;
    pObj->Print(os);

    auto pBase = reinterpret_cast<size_t**>(pObj.get());
    auto pVtable = *pBase;
    // 本当はC++では、unionを使ったキャストはできない
    union FuncPtr{
        size_t addr;
        decltype(&ExtraMemFunc::Print) f;
    };

    // (gdb) info address ExtraMemFunc::Print
    // アドレスはコンパイルすると変わる
    // &ExtraMemFunc::Printを代入するととても小さい数字(25)が入る
    FuncPtr fptr = {0};
    if (sizeof(size_t) > 4) {
        FuncPtr bafPtr = {static_cast<size_t>(0x1004038b2ull)};
        fptr = bafPtr;
    } else {
        FuncPtr bafPtr = {static_cast<size_t>(0xffff38b2u)};
        fptr = bafPtr;
    }

    // 0:デストラクタ, 1:Clear, 2:Print
    pVtable[1] = fptr.addr;
    // というようには、残念ながら書き換えることはできない
    // (gdb) x/4gx pBase
    // (gdb) x/4xg pVtable
    // (gdb) maintenance info sections
    // [2] 0x100433000->0x100438f34 at 0x00030e00: .rdata ALLOC LOAD READONLY DATA HAS_CONTENTS
    // つまりvtableは書き換えられない
    pObj->Print(os);
    EXPECT_EQ("23Extra", os.str());
}
#endif

class TestDoublePrecision : public ::testing::Test{};

TEST_F(TestDoublePrecision, Round) {
    const double one = 1.0;
    double divider = 49.0;
    auto oneThird = one / divider;
    auto nearOne = oneThird * divider;
    EXPECT_DOUBLE_EQ(1.0, nearOne);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
