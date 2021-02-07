// やめるのだフェネックで学ぶC++の実証コード
#define  __STDC_LIMIT_MACROS
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <atomic>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <bitset>
#include <functional>
#include <fstream>
#include <iomanip>
#include <list>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <boost/any.hpp>
#include <boost/io/ios_state.hpp>
#include <gtest/gtest.h>
#include "cFriendsCommon.h"
#include "cppFriends.hpp"
#include "cppFriendsClang.hpp"

// キャストが正しくできることを確認する
namespace {
    int arg1(int a) {
        return a + 2;
    }

    int arg2(int a, int b) {
        return a * b + 2;
    }

    using Arg1Type  = int(*)(int);
    using Arg2Type  = int(*)(int, int);
}

class TestTypeCast : public ::testing::Test {
protected:
    enum class IntEnum : int {
        INT_ENUM_MEMBER = 0x7fffffff
    };

    enum UintEnum : unsigned int{
        UINT_ENUM_MEMBER = 0xefffffffu
    };

    enum LongLongEnum : long long int {
        LL_ENUM_MEMBER = 0x7fffffffffffffffll
    };
};

TEST_F(TestTypeCast, AnyCastBoost) {
    boost::any func = arg1;
    auto result1 = (boost::any_cast<Arg1Type>(func))(10);
    EXPECT_EQ(12, result1);

    func = arg2;
    auto result2 = (boost::any_cast<Arg2Type>(func))(3, 5);
    EXPECT_EQ(17, result2);
}

// https://www.ipa.go.jp/files/000055043.pdf
// R1.3.2
TEST_F(TestTypeCast, PtrDiff) {
    int off, var1[10];
    int *p1, *p2;
    p1 = &var1[5];
    p2 = &var1[2];
//   off = p1 - p2; // intではなくptrdiff_t
    ptrdiff_t offp = p1 - p2; // intではなくptrdiff_t
}

TEST_F(TestTypeCast, EnumCast) {
    auto intEnum = static_cast<std::underlying_type_t<decltype(IntEnum::INT_ENUM_MEMBER)>>
        (IntEnum::INT_ENUM_MEMBER);
    auto uintEnum = static_cast<std::underlying_type_t<decltype(UINT_ENUM_MEMBER)>>
        (UINT_ENUM_MEMBER);
    auto llEnum = static_cast<std::underlying_type_t<decltype(LL_ENUM_MEMBER)>>
        (LL_ENUM_MEMBER);
    static_assert(std::is_same<decltype(intEnum), int>::value, "not int");
    static_assert(std::is_same<decltype(uintEnum), unsigned int>::value, "not unsigned int");
    static_assert(std::is_same<decltype(llEnum), long long int>::value, "not long long int");

    // ついでに
    static_assert(std::is_signed<std::ptrdiff_t>::value, "ptrdiff_t is unsigned");
    int anArray[2];
    static_assert(sizeof(std::ptrdiff_t) != sizeof(int), "std::ptrdiff_t is same as int here");
    static_assert(std::is_same<decltype((anArray + 1) - anArray), std::ptrdiff_t>::value, "Not std::ptrdiff_t?");

    EXPECT_EQ(31, __builtin_popcount(intEnum));
    // __builtin_popcountの引数はunsigned int : テンプレートではない
    static_assert(std::is_same<boost::function_traits<decltype(arg1)>::arg1_type,
                  int>::value,"not int");
    static_assert(std::is_same<boost::function_traits<decltype(__builtin_popcount)>::arg1_type,
                  unsigned int>::value, "not unsigned int");
    EXPECT_EQ(32, __builtin_popcount(llEnum));  // ビット数が減るので警告が出るが意図的
    EXPECT_EQ(63, __builtin_popcountll(llEnum));

    std::ostringstream os;
    os << std::hex << intEnum << std::hex << uintEnum << std::hex << llEnum;
    EXPECT_EQ("7fffffffefffffff7fffffffffffffff", os.str());

    // これもついでに
    static_assert(std::is_same<decltype(sizeof(4)), size_t>::value, "");
    static_assert(sizeof(int) == sizeof(4), "");
    static_assert(alignof(int) == alignof(4), "");

    struct alignas(32) IntBox {
        int member;
    };

    IntBox intBox {1};
    static_assert((alignof(decltype(intBox)) % 32) == 0, "");
    EXPECT_EQ(1, intBox.member);
}

TEST_F(TestTypeCast, PopCount) {
    constexpr short svalue = 0x7fff;
    EXPECT_EQ(15, MyPopCount(svalue));

    // 1111 0111 0011 0000 0100 0010 0010 0101b
    //    4    7    9        10   11   12   14個
    EXPECT_EQ(14, MySlowPopCount(0xf7304225u));
}

TEST_F(TestTypeCast, VoidCast) {
//  int* pInt = std::malloc(16);
//  int* pInt = (int*)(std::malloc(16));
    int* pInt = static_cast<decltype(pInt)>(std::malloc(16));
    std::free(pInt);
    pInt = nullptr;
}

namespace {
    // typeidを比較する
    class TypeIdAssigner {
    public:
        virtual ~TypeIdAssigner() = default;
        virtual int GetValue(void) { return 3; }
    };

    class SubTypeIdAssigner : public TypeIdAssigner {
    public:
        virtual ~SubTypeIdAssigner() = default;
        virtual int GetValue(void) override { return 4; }
    };
}

class TestTypeId : public ::testing::Test {};

TEST_F(TestTypeId, Compare) {
    EXPECT_EQ(typeid(TypeIdAssigner), typeid(TypeIdAssigner));
    EXPECT_EQ(typeid(const TypeIdAssigner), typeid(TypeIdAssigner));
    EXPECT_NE(typeid(SubTypeIdAssigner), typeid(TypeIdAssigner));

    using Base = TypeIdAssigner;
    using Derived = SubTypeIdAssigner;

    // ポリモーフィズムを無視してtypeidで場合分けする
    Base objBase;
    Derived objDerived;
    Base& refBase = objBase;
    Base& refDerived = objDerived;
    EXPECT_EQ(typeid(Base), typeid(refBase));
    EXPECT_EQ(typeid(Derived), typeid(refDerived));

    bool foundBase = false;
    bool foundDerived = false;
    auto& tid = typeid(objBase);
//  switch(tid) {
//  case typeid(Base):
    if (tid == typeid(Base)) {
        foundBase = true;
    }
    EXPECT_TRUE(foundBase);

    auto& tidDerived = typeid(objDerived);
    if (tidDerived == typeid(Derived)) {
        foundDerived = true;
    }
    EXPECT_TRUE(foundDerived);
}

namespace {
    class BaseDoubler {
    public:
        explicit BaseDoubler(const IntHolderImplicit& arg) : value_(arg * 2) {
            implicit_ = true;
        }
        explicit BaseDoubler(const IntHolderExplicit& arg) : value_(arg.Get() * 2) {
            explicit_ = true;
        }
        virtual ~BaseDoubler() = default;
        int Get() const { return value_; }
        bool TakenImplicit() const { return implicit_; }
        bool TakenExplicit() const { return explicit_; }
    private:
        int value_ {0};
        bool implicit_ {false};
        bool explicit_ {false};
    };

    // 継承コンストラクタ
    class DerivedDoubler : public BaseDoubler {
    public:
        using BaseDoubler::BaseDoubler;
        virtual ~DerivedDoubler() = default;
    };

    // explicitではない
    class ImplicitDoubler1 {
    public:
        constexpr ImplicitDoubler1(int arg) : value_(arg * 2) {}
        virtual ~ImplicitDoubler1() = default;
        int Get() const { return value_; }
    private:
        int value_ {0};
    };

    class ImplicitDoubler2 {
    public:
        ImplicitDoubler2(const IntHolderImplicit& arg) : value_(arg * 2) {}
        virtual ~ImplicitDoubler2() = default;
        int Get() const { return value_; }
    private:
        int value_ {0};
    };

    class ImplicitDoubler3 {
    public:
        // explicitではない
        ImplicitDoubler3(const IntHolderExplicit& arg) : value_(arg.Get() * 2) {}
        virtual ~ImplicitDoubler3() = default;
        int Get() const { return value_; }
    private:
        int value_ {0};
    };

    class ExplicitDoubler1 {
    public:
        constexpr explicit ExplicitDoubler1(int arg) : value_(arg * 2) {}
        virtual ~ExplicitDoubler1() = default;
        int Get() const { return value_; }
    private:
        int value_ {0};
    };

    class ExplicitDoubler2 {
    public:
        explicit ExplicitDoubler2(const IntHolderImplicit& arg) : value_(arg * 2) {}
        virtual ~ExplicitDoubler2() = default;
        int Get() const { return value_; }
    private:
        int value_ {0};
    };

    class ExplicitDoubler3 {
    public:
        explicit ExplicitDoubler3(const IntHolderExplicit& arg) : value_(arg.Get() * 2) {}
        virtual ~ExplicitDoubler3() = default;
        int Get() const { return value_; }
    private:
        int value_ {0};
    };
}

class TestInheritingConstructors : public ::testing::Test {};

// #define BUILD_TEST_FULL_HOLDERS

TEST_F(TestInheritingConstructors, Int) {
    constexpr int arg = 5;
    constexpr int expected = 10;

    // 呼び出し先があいまいかというと、そうではない
    BaseDoubler instBase(arg);
    EXPECT_TRUE(instBase.TakenImplicit());
    EXPECT_FALSE(instBase.TakenExplicit());

    DerivedDoubler instDerived(arg);
    EXPECT_TRUE(instDerived.TakenImplicit());
    EXPECT_FALSE(instDerived.TakenExplicit());

#ifdef BUILD_TEST_FULL_HOLDERS
    // 引数無しのコンストラクタは削除されている
    EXPECT_EQ(0, DerivedHolder());
#endif

    ImplicitDoubler1 id1 = 1;
#ifdef BUILD_TEST_FULL_HOLDERS
    ImplicitDoubler2 id2 = 1;
    ImplicitDoubler3 id3 = 1;
    ExplicitDoubler1 ed1 = 1;
    ExplicitDoubler2 ed2 = 1;
    ExplicitDoubler3 ed3 = 1;
#endif

    EXPECT_EQ(expected, ImplicitDoubler1(arg).Get());
    EXPECT_EQ(expected, ExplicitDoubler1(arg).Get());
    EXPECT_EQ(expected, ImplicitDoubler2(arg).Get());
    EXPECT_EQ(expected, ExplicitDoubler2(arg).Get());
#ifdef BUILD_TEST_FULL_HOLDERS
    EXPECT_EQ(expected, ImplicitDoubler3(arg).Get());
    EXPECT_EQ(expected, ExplicitDoubler3(arg).Get());
#endif
}

TEST_F(TestInheritingConstructors, HolderImplicit) {
    constexpr int arg = 5;
    constexpr int expected = 10;
    const IntHolderImplicit holder {arg};

    BaseDoubler inst(holder);
    EXPECT_TRUE(inst.TakenImplicit());
    EXPECT_FALSE(inst.TakenExplicit());

    EXPECT_EQ(expected, BaseDoubler(holder).Get());
    EXPECT_EQ(expected, DerivedDoubler(holder).Get());
    EXPECT_EQ(expected, ImplicitDoubler1(holder).Get());
    EXPECT_EQ(expected, ExplicitDoubler1(holder).Get());
    EXPECT_EQ(expected, ImplicitDoubler2(holder).Get());
    EXPECT_EQ(expected, ExplicitDoubler2(holder).Get());
#ifdef BUILD_TEST_FULL_HOLDERS
    EXPECT_EQ(expected, ImplicitDoubler3(holder).Get());
    EXPECT_EQ(expected, ExplicitDoubler3(holder).Get());
#endif
}

TEST_F(TestInheritingConstructors, HolderExplicit) {
    constexpr int arg = 5;
    constexpr int expected = 10;
    const IntHolderExplicit holder {arg};

    BaseDoubler inst(holder);
    EXPECT_FALSE(inst.TakenImplicit());
    EXPECT_TRUE(inst.TakenExplicit());

    EXPECT_EQ(expected, BaseDoubler(holder).Get());
    EXPECT_EQ(expected, DerivedDoubler(holder).Get());
#ifdef BUILD_TEST_FULL_HOLDERS
    EXPECT_EQ(expected, ImplicitDoubler1(holder).Get());
    EXPECT_EQ(expected, ImplicitDoubler2(holder).Get());
    EXPECT_EQ(expected, ExplicitDoubler1(holder).Get());
    EXPECT_EQ(expected, ExplicitDoubler2(holder).Get());
#endif
    EXPECT_EQ(expected, ImplicitDoubler3(holder).Get());
    EXPECT_EQ(expected, ExplicitDoubler3(holder).Get());
}

TEST_F(TestTypeId, SharedPtr) {
    std::shared_ptr<TypeIdAssigner> pBase = std::make_shared<SubTypeIdAssigner>();
    EXPECT_EQ(1, pBase.use_count());
    auto pSub = std::dynamic_pointer_cast<SubTypeIdAssigner>(pBase);
    EXPECT_EQ(2, pBase.use_count());
    EXPECT_EQ(2, pSub.use_count());

    ASSERT_TRUE(pSub.get() != nullptr);
    EXPECT_EQ(4, pBase->GetValue());
    EXPECT_EQ(4, pSub->GetValue());
}

// https://twitter.com/kuina_ch/status/1256547386958032896
TEST_F(TestInheritingConstructors, IsSame) {
    constexpr int arg = 5;
    constexpr int expected = 10;
    BaseDoubler instBase(arg);
    DerivedDoubler instDerived(arg);
    EXPECT_EQ(expected, instBase.Get());
    EXPECT_EQ(expected, instDerived.Get());

    static_assert(!std::is_same<BaseDoubler, DerivedDoubler>::value, "Failed");
    static_assert(std::is_base_of<BaseDoubler, DerivedDoubler>::value, "Failed");
    static_assert(!std::is_same<decltype(instBase), decltype(instDerived)>::value, "Failed");
    static_assert(std::is_base_of<decltype(instBase), decltype(instDerived)>::value, "Failed");

    const BaseDoubler constBase(arg);
    const DerivedDoubler constDerived(arg);
    EXPECT_EQ(expected, constBase.Get());
    EXPECT_EQ(expected, constDerived.Get());
    static_assert(std::is_base_of<decltype(instBase), decltype(constDerived)>::value, "Failed");
    static_assert(std::is_base_of<decltype(constBase), decltype(instDerived)>::value, "Failed");

    auto pBase = std::make_unique<BaseDoubler>(arg);
    auto pDoubler = std::make_unique<DerivedDoubler>(arg);
    auto pRawBase = pBase.get();
    auto pRawDoubler = pDoubler.get();
    static_assert(!std::is_same<decltype(pRawBase), decltype(pRawDoubler)>::value, "Failed");
    static_assert(std::is_base_of<
                  typename std::remove_pointer<decltype(pRawBase)>::type,
                  typename std::remove_pointer<decltype(pRawDoubler)>::type>::value, "Failed");

    // Upcast
    const BaseDoubler* pConstBase = pRawDoubler;
    const DerivedDoubler* pConstDoubler = pRawDoubler;
    static_assert(!std::is_same<decltype(pConstDoubler), const BaseDoubler*>::value, "Failed");
    static_assert(std::is_same<decltype(pConstDoubler), const DerivedDoubler*>::value, "Failed");
    static_assert(!std::is_same<decltype(pConstDoubler), BaseDoubler*>::value, "Failed");
    static_assert(!std::is_same<decltype(pConstDoubler), DerivedDoubler*>::value, "Failed");
    static_assert(!std::is_same<typename std::remove_cv<decltype(pConstBase)>::type,
                  BaseDoubler*>::value, "Failed");
    static_assert(std::is_same<
                  typename std::remove_cv<
                  typename std::remove_pointer<decltype(pConstBase)>::type>::type,
                  BaseDoubler>::value, "Failed");
}

namespace {
    struct StandardLayoutObject {
        uint64_t memberA_;
        uint64_t memberB_;
    };

    class DynamicObject {
    public:
        virtual ~DynamicObject() = default;
        uint64_t memberA_;
        uint64_t memberB_;
    };

    struct NestedStandardLayoutObject {
        StandardLayoutObject memberA_;
        uint64_t memberB_;
    };

    struct NestedDynamicObject {
        DynamicObject memberA_;
        uint64_t memberB_;
    };
}

static_assert(sizeof(char) == 1, "Unexpected char size");
static_assert(offsetof(StandardLayoutObject, memberA_) == 0, "Not standard layout");
static_assert(offsetof(StandardLayoutObject, memberB_) == 8, "Not standard layout");
// 警告が出ることの確認
static_assert(offsetof(DynamicObject, memberA_) > 0, "Standard layout");

static_assert(std::is_standard_layout<StandardLayoutObject>::value, "Not standard layout");
static_assert(std::is_standard_layout<NestedStandardLayoutObject>::value, "Not standard layout");
static_assert(!std::is_standard_layout<DynamicObject>::value, "Standard layout");
static_assert(!std::is_standard_layout<NestedDynamicObject>::value, "Standard layout");

namespace {
    // Aはポインタだが、Bは違う
    int* g_confusingA, g_confusingB;
    static_assert(std::is_pointer<decltype(g_confusingA)>::value);
    static_assert(!std::is_pointer<decltype(g_confusingB)>::value);
}

// 本当はどこかのヘッダファイルにまとめて書き、必要な.cppファイルだけがインクルードする
enum class FriendType : int {
    SERVAL,
    COMMON_RACCOON,
    FENNEC_FOX,
};

class TestEnumClass : public ::testing::Test{};

TEST_F(TestEnumClass, ForwardDecl) {
    constexpr FriendTypeBox serval {FriendType::SERVAL};
    EXPECT_EQ(0, std::underlying_type<decltype(serval.type_)>::type(serval.type_));

    constexpr FriendTypeBox raccoon {FriendType::COMMON_RACCOON};
    EXPECT_EQ(1, std::underlying_type<decltype(raccoon.type_)>::type(raccoon.type_));

    constexpr FriendTypeBox fennec {FriendType::FENNEC_FOX};
    EXPECT_EQ(2, std::underlying_type<decltype(fennec.type_)>::type(fennec.type_));
}

// 宣言と定義の型が違ったら困る例
// そもそもここにexternを書くのが間違い。ヘッダファイルに書く。
#if 0
extern unsigned int* g_pointerOrArray;
class TestODRviolation : public ::testing::Test{};

TEST_F(TestODRviolation, All) {
    // g_pointerOrArrayという配列の先頭ではなく、
    // *(g_pointerOrArray)を読もうとするがg_pointerOrArrayの中身は、
    // rax 0xe0000000f0000000 なので、
    // mov (%rax),%eax でsegmentation faultが発生する
    auto i = g_pointerOrArray[0];
    std::cout << i;
}
#endif

class TestExternVariable : public ::testing::Test{};
TEST_F(TestExternVariable, ExternVariable) {
    EXPECT_EQ(1, g_externIntValue);
}

// オブジェクトエイリアシングを考慮しないと、メンバが消失することがある
class NaiveCopy {
public:
    NaiveCopy(void) = default;
    virtual ~NaiveCopy(void) = default;

    void Add(int e) {
        array_.push_back(e);
    }

    int At(size_t i) {
        return array_.at(i);
    }

    void CopyTo(NaiveCopy& other) const {
        // this == otherのときは、otherつまりthisの要素を消してしまう!
        // メンバ関数宣言にconstがあっても無駄である
        other.array_.clear();

        // otherを消してしまったのなら、何もコピーされない
        for(const auto& e : array_) {
            other.array_.push_back(e);
        }
    }

    void CopyTo_s(NaiveCopy& other) const {
        if (this == &other) {
            return;
        }

        other.array_.clear();
        for(const auto& e : array_) {
            other.array_.push_back(e);
        }
    }

private:
    std::vector<int> array_;
};

class TestNaiveCopy : public ::testing::Test {};

TEST_F(TestNaiveCopy, CopyTo) {
    NaiveCopy original;
    original.Add(1);
    original.Add(2);
    EXPECT_EQ(2, original.At(1));

    NaiveCopy other;
    original.CopyTo(other);
    EXPECT_EQ(2, original.At(1));
    EXPECT_EQ(2, other.At(1));

    original.CopyTo(original);
    ASSERT_ANY_THROW(original.At(1));
}

TEST_F(TestNaiveCopy, CopyTo_s) {
    NaiveCopy original;
    original.Add(1);
    original.Add(2);
    EXPECT_EQ(2, original.At(1));
    original.CopyTo_s(original);
    EXPECT_EQ(2, original.At(1));

    NaiveCopy other;
    other.Add(3);
    original.CopyTo_s(other);
    EXPECT_EQ(2, other.At(1));
}

// Ranged-forの使い方
class TestForEach : public ::testing::Test{};

TEST_F(TestForEach, Vector) {
    const std::vector<int> vec = {2,3,5,7,11,13,17};
    decltype(vec)::value_type sum = 0;
    for(auto e : vec) {
        sum += e;
    }
    EXPECT_EQ(58, sum);

    int product = 1;
    std::for_each(std::begin(vec), std::end(vec), [&](auto e) { product *= e; });
    EXPECT_EQ(510510, product);
}

TEST_F(TestForEach, Array) {
    const int vec[] = {2,3,5,7,11,13,17};
    int sum = 0;
    for(auto e : vec) {
        sum += e;
    }
    EXPECT_EQ(58, sum);

    int product = 1;
    std::for_each(std::begin(vec), std::end(vec), [&](auto e) { product *= e; });
    EXPECT_EQ(510510, product);
}

TEST_F(TestForEach, EraseAfterRemove) {
    std::vector<int> vec = {1,3,5};
    // これはイディオム
    vec.erase(std::remove(vec.begin(), vec.end(), 3));
    EXPECT_EQ(2, vec.size());
    int sum = 0;
    for(auto e : vec) {
        sum += e;
    }
    EXPECT_EQ(6, sum);
}

TEST_F(TestForEach, EraseAfterRemoveIf) {
    std::vector<int> vec = {1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39};
    // これはイディオム
    vec.erase(std::remove_if(vec.begin(), vec.end(), [](auto i) { return ((i % 10) != 5);}), vec.end());
    int sum = 0;
    for(auto e : vec) {
        sum += e;
    }
    EXPECT_EQ(80, sum);
}

TEST_F(TestForEach, RawPointer) {
    std::ostringstream os;
    std::vector<int> vec1(0x40000, 123);  // 1Mbytes
    auto pData1 = vec1.data();
    os << *pData1;

    std::vector<int> vec2(0x40000, 456);
    auto pData2 = vec2.data();
    os << *pData2;

    vec2.resize(1, 987);
    vec1.resize(0x80000, 789);
    // pData1 と pData2 は無効になっているかもしれないので、
    // Segmentation faultが発生する(ことがある)
//  os << *pData1;
//  EXPECT_NE(pData1, vec1.data());
    EXPECT_EQ("123456", os.str());
}

class TestLastElement : public ::testing::Test{};

TEST_F(TestLastElement, NotEmpty) {
    const std::vector<int> vec {2, 4, 8};
    int count = 0;
    for (auto i = decltype(vec.size()){0}; i < (vec.size() - 1); ++i) {
        ++count;
    }
    EXPECT_EQ(2, count);
}

#if 0
TEST_F(TestLastElement, Empty) {
    std::vector<int> vec;
    int c=0;
    auto n=vec.size();
    --n;
    // vec.size() - 1 < 0 を符号無し整数で表現するので、未定義動作になるが...
    for (auto i=decltype(n){0}; i<n; ++i) { ++c; }
    EXPECT_EQ(-1, c);
}
#endif

// Min-maxの使い方
class TestMinMax : public ::testing::Test{};

TEST_F(TestMinMax, Vector) {
    std::vector<int> numbers = {1, -1, 2, -2, 3, -3};

    auto iMin = std::min_element(numbers.begin(), numbers.end());
    EXPECT_EQ(-3, *iMin);
    EXPECT_EQ(5, std::distance(numbers.begin(), iMin));

    auto iMax = std::max_element(numbers.begin(), numbers.end());
    EXPECT_EQ(3, *iMax);
    EXPECT_EQ(4, std::distance(numbers.begin(), iMax));
    EXPECT_EQ(6, std::distance(numbers.begin(), numbers.end()));

    auto iMinMax = std::minmax_element(numbers.begin(), numbers.end());
    EXPECT_EQ(-3, *(iMinMax.first));
    EXPECT_EQ(3,  *(iMinMax.second));

    // 自作するが、要素が空でないときしか動作しない
    int result = INT_MAX;
    for(auto n : numbers) {
        result = (result < n) ? result : n;
    }
    EXPECT_EQ(-3, result);
}

TEST_F(TestMinMax, List) {
    std::list<int> numbers;
    numbers.push_back(10);
    numbers.push_back(-10);
    numbers.push_back(40);
    numbers.push_back(-40);
    numbers.push_back(20);

    auto iMax = std::max_element(numbers.begin(), numbers.end());
    EXPECT_EQ(40, *iMax);
    EXPECT_EQ(2, std::distance(numbers.begin(), iMax));
    // distanceを測っても変わらない
    EXPECT_EQ(40, *iMax);
    EXPECT_EQ(5, std::distance(numbers.begin(), numbers.end()));
}

TEST_F(TestMinMax, Empty) {
    std::vector<int> numbers;

    auto iMin = std::min_element(numbers.begin(), numbers.end());
    EXPECT_EQ(numbers.end(), iMin);
    auto iMax = std::max_element(numbers.begin(), numbers.end());
    EXPECT_EQ(numbers.end(), iMax);

    auto iMinMax = std::minmax_element(numbers.begin(), numbers.end());
    EXPECT_EQ(numbers.end(), iMinMax.first);
    EXPECT_EQ(numbers.end(), iMinMax.second);

    // 自作するが、要素が空でないときしか動作しない
    int result = INT_MAX;
    for(auto n : numbers) {
        result = (result < n) ? result : n;
    }
    EXPECT_EQ(INT_MAX, result);
}

TEST_F(TestMinMax, WrongMinUsage) {
    std::vector<int> numbers = {1, -1, 3, 2};
    auto iMin = std::min_element(numbers.begin(), numbers.end());
    EXPECT_EQ(-1, *iMin);

    auto iForward = std::min(numbers.begin(), numbers.end());
    ASSERT_NE(numbers.end(), iForward);
    EXPECT_EQ(1, *iForward);

    auto iReverse = std::min(numbers.rbegin(), numbers.rend());
    ASSERT_NE(numbers.rend(), iReverse);
    EXPECT_EQ(2, *iReverse);
}

// TEST_Fのように、書くだけでテーブルに登録されるものを作る
class GlobalVariableTable {
public:
    void Register(const void* p, const std::string& name) {
        std::uintptr_t address = reinterpret_cast<decltype(address)>(p);
        table_[address] = name;
    }

    std::string GetName(const void* p) const {
        // pは解放済かもしれないので、アクセスしてはならない
        std::ostringstream os;
        std::uintptr_t address = reinterpret_cast<decltype(address)>(p);

        os << "0x" << std::hex << address << " : ";
        auto i = table_.find(address);
        if (i != table_.cend()) {
            os << i->second;
        }
        return os.str();
    }

    static GlobalVariableTable& GetInstance(void) {
// グローバル変数の初期化順序に依存するコードを書いてはならないので、
// construct on first use イディオムを使う。
// main実行前で、シングルスレッドで動作する前提である
//      return instance_;
        static GlobalVariableTable instance;
        return instance;
    }

private:
    GlobalVariableTable(void) = default;
    virtual ~GlobalVariableTable(void) = default;
    // ヒープには置かないようにする
    static void* operator new(std::size_t) = delete;
    static void* operator new[](std::size_t) = delete;

    std::unordered_map<std::uintptr_t, std::string> table_;
    static GlobalVariableTable instance_;  // こうはできない
};

#define CPPFRIENDS_REGISTER_OBJECT(ptr, name) \
    GlobalVariableTable::GetInstance().Register(ptr, name)

class RegisteredObject {
public:
    RegisteredObject(void) {
        CPPFRIENDS_REGISTER_OBJECT(this, __PRETTY_FUNCTION__);
    }
    virtual ~RegisteredObject(void) = default;
private:
    long long int member_ {0};
};

// ここで定義すると、初期化済の GlobalVariableTable::instance_ を
// g_obj1 が使用するので動作する。しかしグローバル変数の初期化順序に
// 依存するコードを書いてはならない
GlobalVariableTable GlobalVariableTable::instance_;

namespace {
    RegisteredObject g_obj1;
    RegisteredObject g_obj2;
}

// ここで定義すると、未初期化の GlobalVariableTable::instance_ を
// g_obj1 が使用するので異常終了する
// GlobalVariableTable GlobalVariableTable::instance_;

class TestCtorOnFirstUse : public ::testing::Test{};

TEST_F(TestCtorOnFirstUse, GetName) {
    RegisteredObject obj;
    auto actual = GlobalVariableTable::GetInstance().GetName(nullptr);
    std::string expected = "0x0 : ";
    EXPECT_EQ(expected, actual);

    std::regex re("0x([0-9a-fA-F]+)\\s+:\\s+RegisteredObject::RegisteredObject.*");
    auto toHex = [=](const std::string& str) -> auto {
        std::istringstream is(str);
        uintptr_t addr;
        is >> std::hex >> addr;
        return addr;
    };

    {
        const auto str = GlobalVariableTable::GetInstance().GetName(&obj);
        std::smatch match;
        ASSERT_TRUE(std::regex_match(str, match, re));
        ASSERT_EQ(2, match.size());
        uintptr_t expected = reinterpret_cast<decltype(expected)>(&obj);
        ASSERT_EQ(expected, toHex(match[1]));
    }

    {
        const auto str = GlobalVariableTable::GetInstance().GetName(&g_obj1);
        std::smatch match;
        ASSERT_TRUE(std::regex_match(str, match, re));
        ASSERT_EQ(2, match.size());
        uintptr_t expected = reinterpret_cast<decltype(expected)>(&g_obj1);
        ASSERT_EQ(expected, toHex(match[1]));
    }

    {
        const auto str = GlobalVariableTable::GetInstance().GetName(&g_obj2);
        std::smatch match;
        ASSERT_TRUE(std::regex_match(str, match, re));
        ASSERT_EQ(2, match.size());
        uintptr_t expected = reinterpret_cast<decltype(expected)>(&g_obj2);
        ASSERT_EQ(expected, toHex(match[1]));
    }
}

// RAIIのもうすこし簡潔な例
template <size_t Address, uint32_t Maskbit>
class ScopedInterruptMask final {
    FRIEND_TEST(TestScopedInterruptMask, MaskAll);
    FRIEND_TEST(TestScopedInterruptMask, Partial);
public:
    // 本当はもっと厳密に作る
    ScopedInterruptMask(void) {
        reg_ |= Maskbit;
    }

    ~ScopedInterruptMask(void) {
        reg_ &= ~Maskbit;
    }

    // ヒープにはおかない
    static void* operator new(std::size_t) = delete;
    static void* operator new[](std::size_t) = delete;
    // スコープを超えない
    ScopedInterruptMask(const ScopedInterruptMask&) = delete;
    ScopedInterruptMask& operator=(const ScopedInterruptMask&) = delete;
    ScopedInterruptMask(ScopedInterruptMask&&) = delete;
    ScopedInterruptMask& operator=(ScopedInterruptMask&&) = delete;
private:
    // 本当はAddressに置きたい
    static volatile uint32_t reg_;
};

template <size_t Address, uint32_t Maskbit>
volatile uint32_t ScopedInterruptMask<Address, Maskbit>::reg_;

class TestScopedInterruptMask : public ::testing::Test {};

TEST_F(TestScopedInterruptMask, MaskAll) {
    using RegA1 = ScopedInterruptMask<0xa000, 1>;
    using RegB1 = ScopedInterruptMask<0xb000, 1>;  // アドレスが異なるので別物
    using RegC6 = ScopedInterruptMask<0xc000, 6>;
    RegA1::reg_ = 0;
    RegB1::reg_ = 0;
    RegC6::reg_ = 0;
    {
        RegA1 mask;
        EXPECT_EQ(1, RegA1::reg_);
        EXPECT_EQ(0, RegB1::reg_);
        EXPECT_EQ(0, RegC6::reg_);
        {
            RegB1 mask;
            EXPECT_EQ(1, RegA1::reg_);
            EXPECT_EQ(1, RegB1::reg_);
            EXPECT_EQ(0, RegC6::reg_);
            {
                RegC6 mask;
                EXPECT_EQ(1, RegA1::reg_);
                EXPECT_EQ(1, RegB1::reg_);
                EXPECT_EQ(6, RegC6::reg_);
            }
            EXPECT_EQ(1, RegA1::reg_);
            EXPECT_EQ(1, RegB1::reg_);
            EXPECT_EQ(0, RegC6::reg_);
        }
        EXPECT_EQ(1, RegA1::reg_);
        EXPECT_EQ(0, RegB1::reg_);
        EXPECT_EQ(0, RegC6::reg_);
    }
    EXPECT_EQ(0, RegA1::reg_);
    EXPECT_EQ(0, RegB1::reg_);
    EXPECT_EQ(0, RegC6::reg_);
}

TEST_F(TestScopedInterruptMask, Partial) {
    using RegEven = ScopedInterruptMask<0xc000, 0xaaaa0000u>;
    using RegOdd  = ScopedInterruptMask<0xd000, 0x5555u>;
    RegEven::reg_ = 0xffffu;
    RegOdd::reg_  = 0xffff0000u;
    {
        RegEven mask;
        EXPECT_EQ(0xaaaaffffu, RegEven::reg_);
        EXPECT_EQ(0xffff0000u, RegOdd::reg_);
        {
            RegOdd mask;
            EXPECT_EQ(0xaaaaffffu, RegEven::reg_);
            EXPECT_EQ(0xffff5555u, RegOdd::reg_);
        }
        EXPECT_EQ(0xaaaaffffu, RegEven::reg_);
        EXPECT_EQ(0xffff0000u, RegOdd::reg_);
    }
    EXPECT_EQ(0xffffu, RegEven::reg_);
    EXPECT_EQ(0xffff0000u, RegOdd::reg_);
}

class TestZeroInitialize : public ::testing::Test {
protected:
    virtual void SetUp() override {
        clear();
    }

    virtual void TearDown() override {
        clear();
    }

    void clear() {
        // リセット
        decltype(os_) os;
        os_.swap(os);
    }

    // PODの配列
    template <typename T, size_t S>
    typename std::enable_if_t<std::is_pod<T>::value, void>
    ZeroInitialize(T (&argArray)[S]) {
        // どのパターンにマッチングしたか記録を残す
        os_ << "array(" << sizeof(argArray) << "),";
        // 配列の先頭要素だけでなく、配列全体のサイズ
        ::memset(argArray, 0, sizeof(argArray));
        return;
    }

    // nullptr
    template <typename T>
    typename std::enable_if_t<std::is_null_pointer<T>::value, void>
    ZeroInitialize(T) {
        os_ << "nullptr,";
        return;
    }

    // PODを指すポインタ
    template <typename T>
    typename std::enable_if_t<std::is_pointer<T>::value && std::is_pod<std::remove_pointer_t<T>>::value, void>
    ZeroInitialize(T pObject) {
        if (pObject) {
            os_ << "pointer(" << sizeof(*pObject) << "),";
            ::memset(pObject, 0, sizeof(*pObject));
        } else {
            os_ << "null-pointer,";
        }
        return;
    }

    // PODへの参照
    template <typename T>
    typename std::enable_if_t<!std::is_pointer<T>::value && std::is_pod<T>::value, void>
        ZeroInitialize(T& object) {
        os_ << "reference(" << sizeof(object) << "),";
        ::memset(&object, 0, sizeof(object));
        return;
    }

    // NULL=0は何もしない
    // 0以外の整数も無意味なので何もしない(本当はコンパイルエラーにしたい)
    template <typename T>
    typename std::enable_if_t<std::is_integral<T>::value, void>
    ZeroInitialize(const T&) {
        os_ << "integral,";
        return;
    }

    // 呼び出しログ
    std::ostringstream os_;

    class Inner {
    public:
        Inner(void) = default;
        explicit Inner(int a) : a_(a) {}
        virtual ~Inner(void) = default;
        virtual int Get(void) const { return a_; }
    private:
        int a_ {0};
    };
};

TEST_F(TestZeroInitialize, Templates) {
    std::string expected;

    short primitive = 1;
    {
        EXPECT_TRUE(primitive);
        ZeroInitialize(primitive);
        EXPECT_FALSE(primitive);
        expected += "reference(2),";

        primitive = 2;
        EXPECT_TRUE(primitive);
        ZeroInitialize(&primitive);
        EXPECT_FALSE(primitive);
        expected += "pointer(2),";
    }

    constexpr size_t arraySize = 5;
    {
        int array1[arraySize] {2,3,4,5,6};
        ZeroInitialize(array1);
        for(const auto e : array1) {
            EXPECT_FALSE(e);
        }
        expected += "array(20),";
    }
    {
        int array2[arraySize] {2,3,4,5,6};
        ZeroInitialize(&array2[0]);
        EXPECT_FALSE(array2[0]);
        for(size_t i = 1; i < arraySize; ++i) {
            EXPECT_TRUE(array2[i]);
        }
        expected += "pointer(4),";

        ZeroInitialize(array2[arraySize - 1]);
        EXPECT_FALSE(array2[arraySize - 1]);
        expected += "reference(4),";
    }
    {
        int v = 0;
        int* arrayP[arraySize] {&v,&v,&v,&v,&v};
        ZeroInitialize(arrayP);
        for(const auto e : arrayP) {
            EXPECT_FALSE(e);
        }
        // 32bit環境では異なる
        expected += "array(40),";
    }

    struct Data {
        int m1;
        int* p;
        int m2;
    };
    {
        Data data1 {1, nullptr, 2};
        data1.p = &data1.m1;
        ZeroInitialize(data1);
        EXPECT_FALSE(data1.m1);
        EXPECT_FALSE(data1.p);
        EXPECT_FALSE(data1.m2);
        // 32bit環境では異なる
        expected += "reference(24),";
    }
    {
        Data data2 {3, nullptr, 4};
        data2.p = &data2.m1;
        ZeroInitialize(&data2);
        EXPECT_FALSE(data2.m1);
        EXPECT_FALSE(data2.p);
        EXPECT_FALSE(data2.m2);
        expected += "pointer(24),";
    }
    {
        Data dataArray[] {{1, nullptr, 2}, {3, nullptr, 4}};
        ZeroInitialize(dataArray);
        for(const auto& e : dataArray) {
            EXPECT_FALSE(e.m1);
            EXPECT_FALSE(e.p);
            EXPECT_FALSE(e.m2);
        }
        expected += "array(48),";
    }

    static_assert(!std::is_pointer<decltype(nullptr)>::value, "nullptr is a pointer");
    static_assert(std::is_integral<decltype(NULL)>::value, "NULL is a number");
    static_assert(!std::is_pod<Inner>::value, "Inner is a non-POD type");

    ZeroInitialize(nullptr);
    expected += "nullptr,";
    Data* pNull = nullptr;
    ZeroInitialize(pNull);
    expected += "null-pointer,";

    ZeroInitialize(NULL);
    ZeroInitialize(0);
    ZeroInitialize(1);
    ZeroInitialize(0x123456789ull);
    expected += "integral,integral,integral,integral,";

    // vtableへのポインタをクリアすると異常になるので、コンパイルエラーにする
#if 0
    Inner inner(1);
    Inner innerArray[2];
    ZeroInitialize(inner);
    ZeroInitialize(&inner);
    ZeroInitialize(innerArray);
#endif

    // 32-bitモードは正解が異なる
    if (sizeof(size_t) > 4) {
        EXPECT_EQ(expected, os_.str());
    }
}

namespace {
    struct StandardLayoutObjectMemFunc {
        uint64_t memberA_;
        uint64_t memberB_;
        void Clear() {
            memset(this, 0, sizeof(*this));
        }
    };
}

static_assert(std::is_standard_layout<StandardLayoutObjectMemFunc>::value, "Not standard layout");
TEST_F(TestZeroInitialize, StandardLayout) {
    StandardLayoutObjectMemFunc obj {2,3};
    EXPECT_EQ(2, obj.memberA_);
    EXPECT_EQ(3, obj.memberB_);
    obj.Clear();
    EXPECT_FALSE(obj.memberA_);
    EXPECT_FALSE(obj.memberB_);
    obj.Clear();
}

#if 0
static_assert(!std::is_standard_layout<DynamicObjectMemFunc>::value, "Must not be standard layout");
TEST_F(TestZeroInitialize, NonStandardLayout) {
    auto pObj = std::make_unique<SubDynamicObjectMemFunc>();
    pObj->memberA_ = 2;
    pObj->memberB_ = 3;

    // この呼び出しは、メンバだけでなくvtableへのポインタもクリアしてしまう
    pObj->Clear();
    EXPECT_FALSE(pObj->memberA_);
    EXPECT_FALSE(pObj->memberB_);

    // vtableへのポインタがクリアされているので、異常終了する
    std::ostringstream os;
    pObj->Print(os);
}
#endif

// std::functionなどを遅延実行するオブジェクトをコンテナに入れるための
// 処理に共通なクラス
class BaseCommand {
protected:
    BaseCommand(void) = default;
    virtual void exec(void) = 0;
public:
    virtual ~BaseCommand(void) = default;
    void Exec(void) { exec(); }
};

// &関数名, std::function, ラムダ式, operator() をマッチングする
template <typename Functor>
class ConcreteCommand : public BaseCommand {
public:
    explicit ConcreteCommand(const Functor& f) : BaseCommand(), f_(f) {}
    virtual ~ConcreteCommand(void) = default;
    virtual void exec(void) override { f_(); }
private:
    Functor f_;
};

// 関数名をマッチングする
template <typename Result, typename ... ArgTypes>
class ConcreteCommandF : public BaseCommand {
    using Function = Result(&)(ArgTypes...);
public:
    explicit ConcreteCommandF(const Function f) : BaseCommand(), f_(f) {}
    virtual ~ConcreteCommandF(void) = default;
    virtual void exec(void) override { f_(); }
private:
    Function f_;
};

template <typename T>
auto CreateConcreteCommand(const T& f) {
    // Return Value Optimization を期待している
    return std::make_unique<ConcreteCommand<T>>(f);
}

template <typename Result, typename ... ArgTypes>
auto CreateConcreteCommand(Result(&f)(ArgTypes...)) {
    // Named Return Value Optimization を期待している
    auto command = std::make_unique<ConcreteCommandF<Result, ArgTypes...>>(f);
    return command;
}

namespace {
    int g_anGlobalVariable = 0;
    void setVarToOne(void) {
        g_anGlobalVariable = 1;
    }

    void doubleVar(void) {
        g_anGlobalVariable *= 2;
    }

    struct TripleVar {
        void operator()(void) { g_anGlobalVariable *= 3; }
    };

    struct ClassFunction {
        static void decrement(void) { --g_anGlobalVariable; }
    };

    void squareVar(void) {
        g_anGlobalVariable = g_anGlobalVariable * g_anGlobalVariable;
    }
}

class TestCommandPattern : public ::testing::Test{};

TEST_F(TestCommandPattern, All) {
    std::vector<std::unique_ptr<BaseCommand>> commands;

    g_anGlobalVariable = 0;
    commands.push_back(CreateConcreteCommand(&setVarToOne));
    std::function<decltype(doubleVar)> func = doubleVar;
    commands.push_back(CreateConcreteCommand(func));
    commands.push_back(CreateConcreteCommand([=](void){ g_anGlobalVariable += 5;}));
    TripleVar op;
    commands.push_back(CreateConcreteCommand(op));
    commands.push_back(CreateConcreteCommand(&ClassFunction::decrement));
    commands.push_back(CreateConcreteCommand(squareVar));
    commands.push_back(CreateConcreteCommand(ClassFunction::decrement));

    for(auto& command : commands) {
        command->Exec();
    }

    // (((1 * 2 + 5) * 3) - 1) ^ 2 - 1
    EXPECT_EQ(399, g_anGlobalVariable);
}

// ビットフィールの定義
struct TestingBitFields1 {
    unsigned int version  : 2;
    unsigned int protocol : 4;
    unsigned int sender   : 4;
    unsigned int receiver : 4;
    unsigned int parameter1 : 8;
    unsigned int parameter2 : 8;
    unsigned int padding    : 2;
    uint8_t body[4];
};

// パラメータが1bit 増えた
struct TestingBitFields2 {
    unsigned int version  : 2;
    unsigned int protocol : 4;
    unsigned int sender   : 4;
    unsigned int receiver : 4;
    unsigned int parameter1 : 8;
    unsigned int parameter2 : 9; // 増やしてみた
    unsigned int padding    : 2; // 減らし忘れた
    uint8_t body[4];
};

// こうすればよい。intのサイズが決め打ちだが、そもそもビットフィールドに移植性はない。
struct TestingBitFields3 {
    unsigned int version  : 2;
    unsigned int protocol : 4;
    unsigned int sender   : 4;
    unsigned int receiver : 4;
    unsigned int parameter1 : 8;
    unsigned int parameter2 : 9; // 増やしてみた
    unsigned int : 0;  // アラインメント
    uint8_t body[4];
};

// 併せてサイズを確認する
static_assert(sizeof(TestingBitFields1) == 8, "Must be 8 bytes");
static_assert(sizeof(TestingBitFields2) >  8, "Must be more than 8 bytes");
static_assert(sizeof(TestingBitFields3) == 8, "Must be 8 bytes");

class TestOperatorPrecedence : public ::testing::Test{};

TEST_F(TestOperatorPrecedence, All) {
    auto expr1 = [=](int expr) -> int { return expr ? 2 : 3 + 4; };   // expr ? 2 : (3 + 4)
    auto expr2 = [=](int expr) -> int { return 4 +  expr ? 2 : 3; };  // (4 + expr) ? 2 : 3
    auto expr3 = [=](int expr) -> int { return 4 + (expr ? 2 : 3); };

    EXPECT_EQ(2, expr1(1));
    EXPECT_EQ(7, expr1(0));
    EXPECT_EQ(2, expr2(1));
    EXPECT_EQ(2, expr2(0));
    EXPECT_EQ(3, expr2(-4));
    EXPECT_EQ(6, expr3(1));
    EXPECT_EQ(7, expr3(0));
}

// http://qiita.com/Nabetani/items/9d0fa9e750310d1be24e
// を確認する
// https://stackoverflow.com/questions/42454016/ternary-conditional-operator-on-the-left-of-assignment-operator
TEST_F(TestOperatorPrecedence, LeftTernaryOperator) {
    int a = 0;
    int b = 0;
    for(int i=0; i<2; ++i) {
        ((i) ? a : b) = 1;
        ((!i) ? a : b) = 2;
        if (i == 0) {
            EXPECT_EQ(2, a);
            EXPECT_EQ(1, b);
        } else {
            EXPECT_EQ(1, a);
            EXPECT_EQ(2, b);
        }
    }
}

namespace {
    // std::cout的なもの
    std::ostringstream g_globalLogStream;

    void FreeFuncWriteNothing(const std::string& message) {
        return;
    }

    void FreeFuncWriteLog(const std::string& message) {
        g_globalLogStream << message;
        return;
    }

    class MyFunctionHolder {
    public:
        // typedefより分かりやすい
        using Func = void(*)(const std::string& message);

        MyFunctionHolder(void) = default;
        MyFunctionHolder(Func f) : f_(f ? f : &FreeFuncWriteNothing) {}
        virtual ~MyFunctionHolder() = default;

        void Write(const std::string& message) {
            // null検査は不要
            f_(message);
            return;
        }
    private:
        Func f_ {&FreeFuncWriteNothing};
    };
}

class TestNullFuncPtr : public ::testing::Test {
    virtual void SetUp() override {
        clear();
    }

    virtual void TearDown() override {
        clear();
    }

    void clear() {
        // リセット
        decltype(g_globalLogStream) os;
        g_globalLogStream.swap(os);
    }
};

TEST_F(TestNullFuncPtr, Default) {
    MyFunctionHolder holder;
    const std::string message = "Message";
    holder.Write("");
    EXPECT_TRUE(g_globalLogStream.str().empty());

    holder.Write(message);
    EXPECT_TRUE(g_globalLogStream.str().empty());

    MyFunctionHolder holderNull;
    holderNull.Write(message);
    EXPECT_TRUE(g_globalLogStream.str().empty());
};

TEST_F(TestNullFuncPtr, Write) {
    MyFunctionHolder holder(FreeFuncWriteLog);
    const std::string message = "Message";
    holder.Write(message);
    EXPECT_EQ(message, g_globalLogStream.str());
};

template <typename T>
class MyCounter {
private:
    void incrementImpl(void) {
        ++count_;
    }

    void decrementImpl(void) {
        --count_;
    }

    using funcType = void(MyCounter::*)(void);
    T count_;
    funcType inc_;
    funcType dec_;

public:
    MyCounter(void) : count_(0), inc_(&incrementImpl), dec_(&decrementImpl) {}
    virtual ~MyCounter(void) {}

    const T& Get(void) const {
        return count_;
    }

    void Increment(void) {
        (this->*inc_)();
    }

    void Decrement(void) {
        (this->*dec_)();
    }
};

class TestFuncPtr : public ::testing::Test {
protected:
    static int myIncrement(int n) {
        return n + 1;
    }
};

TEST_F(TestFuncPtr, Write) {
    int(*func)(int) = &myIncrement;
    EXPECT_EQ(12, func(11));

    MyCounter<uint64_t> counter;
    counter.Increment();

    auto actual = counter.Get();
    decltype(actual) expected = 1;
    EXPECT_EQ(expected, actual);

    counter.Decrement();
    counter.Decrement();
    actual = counter.Get();
    expected = std::numeric_limits<decltype(actual)>::max();
    EXPECT_EQ(expected, actual);
};

class TestMaxSize : public ::testing::Test{};

TEST_F(TestMaxSize, Print) {
    boost::io::ios_flags_saver saver(std::cerr);
    std::cerr << std::hex << "SIZE_MAX = 0x" << SIZE_MAX << "\n";
    std::cerr << std::hex << "SSIZE_MAX = 0x" << SSIZE_MAX << "\n";
}

class TestClearStream : public ::testing::Test{};

TEST_F(TestClearStream, SwapAndMove) {
    std::ostringstream emptyOs;
    std::ostringstream os;
    os << "ABC";

    EXPECT_TRUE(emptyOs.str().empty());
    EXPECT_FALSE(os.str().empty());

    os.swap(emptyOs);
    EXPECT_FALSE(emptyOs.str().empty());
    EXPECT_TRUE(os.str().empty());

    std::swap(os, emptyOs);
    EXPECT_TRUE(emptyOs.str().empty());
    EXPECT_FALSE(os.str().empty());

    os = std::ostringstream();
    EXPECT_TRUE(os.str().empty());
}

class TestDanglingIterator : public ::testing::Test{};

TEST_F(TestDanglingIterator, Clear) {
    constexpr size_t n = 1000;
    MyStringList strList(n);

    std::ostringstream os;
    strList.Print(os);
    EXPECT_FALSE(os.str().empty());

    for(const auto& str : strList.dataSet_) {
        EXPECT_FALSE(str.empty());
#if 0
        // 要素を消すと無効イテレータにアクセスして落ちる
        strList.Clear();
#endif
    }
}

TEST_F(TestDanglingIterator, Pop) {
    constexpr size_t n = 1000;
    MyStringList strList(n);

    for(const auto& str : strList.dataSet_) {
        EXPECT_FALSE(str.empty());
#if 0
        // 要素を消すと無効イテレータにアクセスして落ちる
        strList.Pop();
#endif
    }
}

class TestMatchingBOM : public ::testing::Test {
protected:
    virtual void SetUp() override {
        clearAll();
    }

    virtual void TearDown() override {
        clearAll();
    }

    void readHeadLine(const char* filename) {
        std::ifstream is(filename);
        std::getline(is, str1_);
        std::getline(is, str2_);
    }

    void clearAll(void) {
        str1_.clear();
        str2_.clear();
    }

    std::string str1_;
    std::string str2_;
};

TEST_F(TestMatchingBOM, ByteOrderMark) {
    readHeadLine("sampleWithBom.txt");
    ASSERT_FALSE(str1_.empty());
    ASSERT_FALSE(str2_.empty());

    std::regex reWithoutBom("^Word[\\r\\n]*");
    std::smatch matchWithoutBom;
    EXPECT_FALSE(std::regex_match(str1_, matchWithoutBom, reWithoutBom));
    EXPECT_TRUE(std::regex_match(str2_, matchWithoutBom, reWithoutBom));

    std::regex reWithBom("^...Word[\\r\\n]*");
    std::smatch matchWithBom;
    EXPECT_TRUE(std::regex_match(str1_, matchWithBom, reWithBom));
    EXPECT_FALSE(std::regex_match(str2_, matchWithBom, reWithBom));
}

class TestFloatingNumber : public ::testing::Test {
protected:
    double mySqrt(double a, double b) {
        return sqrt(a/b);
    }
};

TEST_F(TestFloatingNumber, Sqrt) {
    auto resultIsNaN = std::isnan(mySqrt(1.0, -1.0));
    EXPECT_TRUE(resultIsNaN);
}

namespace {
    template<typename T>
    std::string ConvertToBits(const T& obj) {
        uint8_t bitPattern[sizeof(T)] {0};
        static_assert(sizeof(bitPattern) == sizeof(T), "");
        ::memmove(bitPattern, &obj, sizeof(T));

        // リトルエンディアン
        std::ostringstream os;
        size_t i = sizeof(T);
        while(i > 0) {
            os << std::bitset<8>(bitPattern[i-1]);
            --i;
        }

        return os.str();
    }

    // 符号、指数部、仮数部で区切る
    std::string SplitDoubleComponents(const std::string& str) {
        return str.substr(0, 1) + ":" + str.substr(1, 11) + ":" + str.substr(12, 52);
    }

    // 仮数部のビットパターンが最後にあることを確認する
    void CheckLastDigits(const std::string& digits, const std::string& subDigits) {
        ASSERT_GT(digits.size(), subDigits.size());
        EXPECT_EQ(digits.size() - subDigits.size(), digits.rfind(subDigits));
   }
}

TEST_F(TestFloatingNumber, TooLarge) {
    constexpr long long int HalfBigMoneyInt = 2500000000000001ll;
    const double HalfBigMoneyDouble = HalfBigMoneyInt;
    auto digits = SplitDoubleComponents(ConvertToBits(HalfBigMoneyInt));
    CheckLastDigits(digits, "0100000000000001");
    digits = SplitDoubleComponents(ConvertToBits(HalfBigMoneyDouble));
    CheckLastDigits(digits, "01000000000000010");

    // doubleで、5000兆円 + 1 では1を表現できる。
    // doubleの仮数部は、最上位bitは1であることを暗黙に仮定している。
    constexpr long long int BigMoneyInt = 5000000000000001ll;
    const double BigMoneyDouble = BigMoneyInt;
    digits = SplitDoubleComponents(ConvertToBits(BigMoneyInt));
    CheckLastDigits(digits, "01000000000000001");
    digits = SplitDoubleComponents(ConvertToBits(BigMoneyDouble));
    CheckLastDigits(digits, "01000000000000001");

    // doubleでは、5000兆円*2 + 1 では1を表現できない
    constexpr long long int TwiceBigMoneyInt = 10000000000000001ll;
    double TwiceBigMoneyDouble = TwiceBigMoneyInt;
    digits = SplitDoubleComponents(ConvertToBits(TwiceBigMoneyInt));
    CheckLastDigits(digits, "010000000000000001");
    digits = SplitDoubleComponents(ConvertToBits(TwiceBigMoneyDouble));
    CheckLastDigits(digits, "01000000000000000");
}

static_assert((2 * 2) == 4, "I expect 2 * 2 is equal to 4");
static_assert((0b1 * 0b1) == 0b1, "");
// static_assert((0b2 * 0b2) == 0b4, "");

int MyMaxTest1(int l, int r) {
    auto max = std::max(l, r);
    return max;
}

using std::max;
int MyMaxTest2(int l, int r) {
    auto max = std::max(l, r);
    return max;
}

int MyMaxTest3(int l, int r) {
    auto value = max(l, r);
    return value;
}

#if 0
// こうは書けない。maxが衝突する
int MyMaxTest4(int l, int r) {
    auto max = max(l, r);
    return max;
}
#endif

class TestOneEqualsTwo : public ::testing::Test {};

// 1=2を証明せよ
TEST_F(TestOneEqualsTwo, StringPtr) {
    bool eq = false;
    if ("1=2") {
        eq = true;
//      std::cout << "YES";
    }
    EXPECT_TRUE(eq);
}

namespace {
    using Inner = std::string;
    struct Outer {
        using Inner = int;
        Outer(Inner arg) { inner_ = arg; }
        Inner Foo(const Inner& other);
        std::string Bar(const Inner& other);
        Inner inner_ {0};
    };

    Outer::Inner Outer::Foo(const Inner& other) {
        return inner_;
    }

    Inner Outer::Bar(const Inner& other) {
        std::string s("bar");
        return s;
    }
}

class TestInnerClass : public ::testing::Test {};

TEST_F(TestInnerClass, All) {
    Outer a(1);
    Outer b(2);
    EXPECT_EQ(1, a.Foo(b.inner_));
    const std::string expected = "bar";
    EXPECT_EQ(expected, a.Bar(b.inner_));
}

namespace {
    int InitializeInterator(const std::vector<int>& vec) {
        const auto vecSize = vec.size();
        static_assert(std::is_const<decltype(vecSize)>::value, "Must be const");
        int value = 0;

        // こう書けば、sizeのconstを外せる
        for(auto i = decltype(vecSize){0}; i < vecSize; ++i) {
            const auto& v = vec.at(i);
            if (v < 0) {
                value = v;
            }
        }

        return value;
    }
}

class TestInitializeInterator : public ::testing::Test {};

TEST_F(TestInitializeInterator, All) {
    const std::vector<int> vec {0, 2, -2, 3, -1, 4};
    EXPECT_EQ(-1, InitializeInterator(vec));
}

namespace {
    std::atomic<size_t> sharedCounter {0};
    struct Counter {
        Counter(void) { ++sharedCounter; }
        ~Counter(void) { --sharedCounter; }
    };
}

class TestSmartPtr : public ::testing::Test {
protected:
    virtual void SetUp() override {
        sharedCounter = 0;
    }

    virtual void TearDown() override {
        EXPECT_FALSE(sharedCounter);
        sharedCounter = 0;
    }
};

TEST_F(TestSmartPtr, UniqueOne) {
    Counter counter;
    EXPECT_EQ(1, sharedCounter);
}

TEST_F(TestSmartPtr, UniquePtr) {
    {
        auto pCounter = std::make_unique<Counter>();
        EXPECT_EQ(1, sharedCounter);
    }
    EXPECT_EQ(0, sharedCounter);
}

TEST_F(TestSmartPtr, UniqueMulti) {
    constexpr size_t size = 5;
    {
        auto pCounter = std::unique_ptr<Counter[]>(new Counter[size]);
        EXPECT_EQ(size, sharedCounter);
    }
    EXPECT_EQ(0, sharedCounter);
}

TEST_F(TestSmartPtr, SharedPtr) {
    std::shared_ptr<Counter> pAlias;
    {
        auto pCounter = std::make_shared<Counter>();
        EXPECT_EQ(1, sharedCounter);
        pAlias = pCounter;
        EXPECT_EQ(1, sharedCounter);
    }

    EXPECT_EQ(1, sharedCounter);
    pAlias.reset();
    EXPECT_EQ(0, sharedCounter);
}

TEST_F(TestSmartPtr, SharedMulti) {
    constexpr size_t size = 7;
    {
        auto pCounter = std::shared_ptr<Counter[]>(new Counter[size]);
        EXPECT_EQ(size, sharedCounter);
    }
    EXPECT_EQ(0, sharedCounter);
}

namespace {
    class MyException : public std::runtime_error {
    public:
        explicit MyException(const std::string& what_arg) : std::runtime_error(what_arg) {}
        explicit MyException(const char* what_arg) : std::runtime_error(what_arg) {}
        virtual ~MyException(void) = default;
    };
}

class TestExceptionWhat : public ::testing::Test {};

TEST_F(TestExceptionWhat, StrMessage) {
    const std::string expected("Message");
    bool thrown = false;

    try {
        throw MyException(expected);
    } catch (MyException& e) {
        thrown = true;
        EXPECT_EQ(expected, e.what());
        EXPECT_NE("Message", e.what());
        static_assert(std::is_same<const char*, decltype(e.what())>::value);
        static_assert(!std::is_same<const std::string, decltype(e.what())>::value);
    }
    EXPECT_TRUE(thrown);
}

TEST_F(TestExceptionWhat, CharMessage) {
    bool thrown = false;
    const char* pMessage = "Message";

    try {
        throw MyException(pMessage);
    } catch (MyException& e) {
        thrown = true;
        const std::string expected(pMessage);
        EXPECT_EQ(expected, e.what());
        EXPECT_NE(pMessage, e.what());
        static_assert(std::is_same<const char*, decltype(e.what())>::value);
        static_assert(!std::is_same<const std::string, decltype(e.what())>::value);
    }
    EXPECT_TRUE(thrown);
}

class TestUnreadableConstant : public ::testing::Test {};

TEST_F(TestUnreadableConstant, True) {
    constexpr auto mytrue =
    !!!!!  !!!!!   !    !  !!!!!
      !    !    !  !    !  !
      !    !!!!!   !    !  !!!!!
      !    !    !  !    !  !
      !    !    !   !!!!   !!!!!
    "";
    static_assert(std::is_same<const bool, decltype(mytrue)>::value, "");
    EXPECT_TRUE(mytrue);
}

class TestAtomicOps : public ::testing::Test {};

TEST_F(TestAtomicOps, Or) {
    std::atomic<int32_t> base {0};
    base |= 1;
    EXPECT_EQ(1, base.load());
    EXPECT_EQ(1, base.fetch_or(2));
    EXPECT_EQ(3, base.fetch_or(4));
}

namespace {
    void takeStrStream(std::istream& is) {
        std::string line;
        std::getline(is, line);
        EXPECT_EQ("test", line);
    }
}

class TestCastStream : public ::testing::Test {};

TEST_F(TestCastStream, Or) {
    std::stringstream is("test\n");
    takeStrStream(is);
//  takeStrStream("test\n");
}

namespace {
    struct ParamPair {
        int a;
        double b;
    };

    struct ParamTaker {
        explicit ParamTaker(const ParamPair& params) : a_(params.a), b_(params.b) {}
        int a_;
        double b_;
    };
}

class TestSetPair : public ::testing::Test {};

TEST_F(TestSetPair, Explicit) {
    int a = 2;
    double b = 3.0;
    ParamPair params {a, b};
    ParamTaker obj(params);
    EXPECT_EQ(a, obj.a_);
    EXPECT_EQ(b, obj.b_);

//  こう書いても自動的にParamPairにはならない
//  ParamTaker alt(a, b);
}

class TestShortCircuit : public ::testing::Test {};

TEST_F(TestShortCircuit, NullPointer) {
    int* p = nullptr;
    int actual = 0;
    if (!p || (*p == 0)) {
        actual += 1;
    }

    int value = 0;
    p = &value;
    if (!p || (*p == 0)) {
        actual += 2;
    }

    ++value;
    if (!p || (*p == 0)) {
        actual += 4;
    }

    EXPECT_EQ(3, actual);
}

namespace {
    std::ostringstream g_actual_message;

    class TooBigObject {
        class MemPool {
        public:
            using Pool = std::vector<uint8_t>;
            MemPool(int serial, size_t s) :
                serial_(serial), p_(new Pool(s, 0)) {}

            virtual ~MemPool(void) {
                g_actual_message << "MemPool " << serial_ << " Released!";
            }
        private:
            int serial_ {0};
            std::unique_ptr<Pool> p_;
        };
    public:
        TooBigObject(size_t s) : p1_(1, 0x4), p2_(2, s) {}
        virtual ~TooBigObject(void) = default;
    private:
        MemPool p1_;
        MemPool p2_;
    };

    void MakeTooBigObject(void) {
        TooBigObject small(0x4);
        TooBigObject big(0x1000000000000000ull);
    }

    using MonthSetType = short;
    enum class MonthSet : MonthSetType {
        Jan = 1,
        Dec = 12,
    };
    static_assert(sizeof(MonthSet::Dec) == sizeof(MonthSetType));
    static_assert(sizeof(std::uintptr_t) > sizeof(int));

    // 元記事の要所だけ再現
    // http://www.gotw.ca/gotw/005.htm
    class BaseWithValue {
    public:
        BaseWithValue(long long int value) : value_(value) {}
        virtual ~BaseWithValue(void) = default;

        virtual void Say(int a=1) {
            g_actual_message << a << "b";
        }
    private:
        long long int value_ {0};
    };

    class DerivedWithValue : public BaseWithValue {
    public:
        DerivedWithValue(long long int value) : BaseWithValue(value) {}
        virtual ~DerivedWithValue(void) = default;

        virtual void Say(int a=2) override {
            g_actual_message << a << "d";
        }
    };

    using Array4 = int[4];
    int Sum4(Array4& a) {
        int s = 0;
        for(auto i = std::begin(a); i != std::end(a); ++i) {
            s += *i;
        }
        return s;
    }
}

class TestCppSpecifications : public ::testing::Test {
    virtual void SetUp() override {
        clear();
    }

    virtual void TearDown() override {
        clear();
    }

    void clear() {
        std::ostringstream oss;
        std::swap(oss, g_actual_message);
    }
};

TEST_F(TestCppSpecifications, BadAlloc) {
    ASSERT_THROW(MakeTooBigObject(), std::bad_alloc);
    EXPECT_EQ("MemPool 1 Released!MemPool 2 Released!MemPool 1 Released!", g_actual_message.str());
}

TEST_F(TestCppSpecifications, DefaultParameters) {
    BaseWithValue base(1);
    DerivedWithValue derived(2);

    base.Say();
    derived.Say();
    base.Say(3);
    derived.Say(4);

    BaseWithValue& refBase = derived;
    refBase.Say();
    EXPECT_EQ("1b2d3b4d1d", g_actual_message.str());
}

TEST_F(TestCppSpecifications, Compare) {
    BaseWithValue base1(1);
    BaseWithValue base2(2);
    DerivedWithValue derived1(1);
    DerivedWithValue derived2(2);

    EXPECT_FALSE(std::memcmp(&base1, &base1, sizeof(base1)));
    EXPECT_FALSE(std::memcmp(&derived1, &derived1, sizeof(base1)));
    EXPECT_FALSE(std::memcmp(&derived1, &derived1, sizeof(derived1)));

    EXPECT_TRUE(std::memcmp(&base1, &base2, sizeof(base1)));
    EXPECT_TRUE(std::memcmp(&derived1, &derived2, sizeof(derived1)));
    EXPECT_TRUE(std::memcmp(&base1, &derived1, sizeof(base1)));
}

TEST_F(TestCppSpecifications, ArrayParameters) {
    Array4 a {1,3,5,10};
    EXPECT_EQ(19, Sum4(a));
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
