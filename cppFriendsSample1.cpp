// やめるのだフェネックで学ぶC++の実証コード
#include <cstring>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/any.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
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

TEST_F(TestTypeCast, AnyCast) {
    boost::any func = arg1;
    auto result1 = (boost::any_cast<Arg1Type>(func))(10);
    EXPECT_EQ(12, result1);

    func = arg2;
    auto result2 = (boost::any_cast<Arg2Type>(func))(3, 5);
    EXPECT_EQ(17, result2);
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

    EXPECT_EQ(31, __builtin_popcount(intEnum));
    // __builtin_popcountの引数はunsigned int : テンプレートではない
    static_assert(std::is_same<boost::function_traits<decltype(arg1)>::arg1_type,
                  int>::value,"not int");
    static_assert(std::is_same<boost::function_traits<decltype(__builtin_popcount)>::arg1_type,
                  unsigned int>::value, "not unsigned int");
    EXPECT_EQ(32, __builtin_popcount(llEnum));
    EXPECT_EQ(63, __builtin_popcountll(llEnum));

    std::ostringstream os;
    os << std::hex << intEnum << std::hex << uintEnum << std::hex << llEnum;
    EXPECT_EQ("7fffffffefffffff7fffffffffffffff", os.str());
}

TEST_F(TestTypeCast, PopCount) {
    constexpr short svalue = 0x7fff;
    EXPECT_EQ(15, MyPopCount(svalue));

    // 1111 0111 0011 0000 0100 0010 0010 0101b
    //    4    7    9        10   11   12   14個
    EXPECT_EQ(14, MySlowPopCount(0xf7304225u));
}

// typeidを比較する
namespace {
    class TypeIdAssigner {
    public:
        virtual ~TypeIdAssigner() = default;
    };

    class SubTypeIdAssigner : public TypeIdAssigner {
        virtual ~SubTypeIdAssigner() = default;
    };
}

class TestTypeId : public ::testing::Test {};

TEST_F(TestTypeId, Compare) {
    EXPECT_EQ(typeid(TypeIdAssigner), typeid(TypeIdAssigner));
    EXPECT_EQ(typeid(const TypeIdAssigner), typeid(TypeIdAssigner));
    EXPECT_NE(typeid(SubTypeIdAssigner), typeid(TypeIdAssigner));
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

// Min-maxの使い方
class TestMinMax : public ::testing::Test{};

TEST_F(TestMinMax, Vector) {
    std::vector<int> numbers = {1, -1, 2, -2, 3, -3};

    auto iMin = std::min_element(numbers.begin(), numbers.end());
    EXPECT_EQ(-3, *iMin);
    auto iMax = std::max_element(numbers.begin(), numbers.end());
    EXPECT_EQ(3, *iMax);

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

class TestZeroInitialize : public ::testing::Test {
protected:
    virtual void SetUp() override {
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
    std::unique_ptr<DynamicObjectMemFunc> pObj(new SubDynamicObjectMemFunc);
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

// strをosoutとoserrの両方に書き出す
#define ILL_DEBUG_PRINT(osout, oserr, str) \
    osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \

#define WELL_DEBUG_PRINT(osout, oserr, str) \
    do { \
        osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    } while(0) \

class TestMacroExpansion : public ::testing::Test {};

TEST_F(TestMacroExpansion, Ill) {
    std::ostringstream out;
    std::ostringstream err;

    std::string message {"Event"};
    std::string log = message;
    log += "@";
    log += __PRETTY_FUNCTION__;
    log += ":";

    EXPECT_FALSE(out.tellp());
    ILL_DEBUG_PRINT(out, err, message);
    EXPECT_EQ(log, out.str());
    EXPECT_EQ(log, err.str());

    // outには書いたので、条件はfalse
    if (!out.tellp()) ILL_DEBUG_PRINT(out, err, message);
    // outにはログが書かれないが、errには書かれてしまう
    EXPECT_EQ(log, out.str());
    EXPECT_NE(log, err.str());
}

TEST_F(TestMacroExpansion, Well) {
    std::ostringstream out;
    std::ostringstream err;

    std::string message {"Event"};
    std::string log = message;
    log += "@";
    log += __PRETTY_FUNCTION__;
    log += ":";

    WELL_DEBUG_PRINT(out, err, message);
    EXPECT_EQ(log, out.str());
    EXPECT_EQ(log, err.str());

    if (!out.tellp()) WELL_DEBUG_PRINT(out, err, message);
    EXPECT_EQ(log, out.str());
    EXPECT_EQ(log, err.str());
}

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
    return std::unique_ptr<BaseCommand>(new ConcreteCommand<T>(f));
}

template <typename Result, typename ... ArgTypes>
auto CreateConcreteCommand(Result(&f)(ArgTypes...)) {
    // Named Return Value Optimization を期待している
    std::unique_ptr<BaseCommand> command(new ConcreteCommandF<Result, ArgTypes...>(f));
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

class TestSwitchCase : public ::testing::Test{};

TEST_F(TestSwitchCase, LookUpTable) {
    struct TestCase {
        SwitchCase::Shape shape;
        double expected;
    };

    const TestCase testSet[] = {{SwitchCase::Shape::UNKNOWN, 0.0},
                                {SwitchCase::Shape::CIRCLE, 12.566370614359172},
                                {SwitchCase::Shape::RECTANGULAR, 6.0},
                                {SwitchCase::Shape::TRIANGLE, 24.0},
                                {SwitchCase::Shape::SQUARE, 49.0}};
    for(auto& test : testSet) {
        EXPECT_DOUBLE_EQ(test.expected, GetFixedTestValue(test.shape));
    }
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

// Devirtualizationによって結果は変わらないが、コードは変わるので.sを確認する
class TestDevirtualization : public ::testing::Test{};

TEST_F(TestDevirtualization, Inline) {
    auto actual = Devirtualization::GetStringInline();
    std::string expected = "BaseInlineDerivedInline";
    EXPECT_EQ(expected, actual);
}

TEST_F(TestDevirtualization, Outline) {
    auto actual = Devirtualization::GetStringOutline();
    std::string expected = "BaseOutlineDerivedOutline";
    EXPECT_EQ(expected, actual);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
