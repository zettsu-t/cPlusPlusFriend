// やめるのだフェネックで学ぶC++の実証コード(ライブラリの使い方)
#include <cctype>
#include <ctime>
#include <codecvt>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/time_facet.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/miller_rabin.hpp>
#include <boost/random.hpp>
#include <boost/random/random_device.hpp>
#include <boost/regex.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <gtest/gtest.h>
#include "cFriendsCommon.h"
#include "cppFriends.hpp"

// C++98では違う型、C++11は同じ型
static_assert(std::is_same<boost::fusion::vector<int,int>, boost::fusion::vector2<int,int>>::value, "Different");

class TestRegex : public ::testing::Test {
protected:
    static const std::string pattern_;
    static const std::string input_;
    static const std::string expected_;

    void createStdRegex(std::regex_constants::syntax_option_type t) {
        // 再帰正規表現はサポートしていない
        std::regex expr(pattern_, t);
    }
};

// ()の入れ子を、最も外側の()で分ける、再帰正規表現
const std::string TestRegex::pattern_  {"((?>[^\\s(]+|(\\((?>[^()]+|(?-1))*\\))))"};
const std::string TestRegex::input_    {" (a) ((b)) (((c))) (d) "};
const std::string TestRegex::expected_ {"(a)::((b))::(((c)))::(d)::"};

TEST_F(TestRegex, StdTypes1) {
    const auto typeSet = {std::regex_constants::ECMAScript, std::regex_constants::extended,
                          std::regex_constants::awk, std::regex_constants::egrep};

    for(auto t : typeSet) {
        ASSERT_ANY_THROW(createStdRegex(t));
    }
}

TEST_F(TestRegex, StdTypes2) {
    const auto typeSet = {std::regex_constants::basic, std::regex_constants::grep};

    for(auto t : typeSet) {
        using Iter = std::string::const_iterator;
        Iter startI = input_.begin();
        Iter endI = input_.end();
        std::match_results<Iter> results;
        std::regex expr(pattern_, t);
        std::regex_constants::match_flag_type flags = std::regex_constants::match_default;

        std::ostringstream os;
        while(std::regex_search(startI, endI, results, expr, flags)) {
            auto& head = results[0];
            const std::string substr(head.first, head.second);
            os << substr << "::";
            startI = head.second;
            flags |= std::regex_constants::match_prev_avail;
        }
        EXPECT_TRUE(os.str().empty());
    }
}

TEST_F(TestRegex, Boost) {
    using Iter = std::string::const_iterator;
    Iter startI = input_.begin();
    Iter endI = input_.end();
    boost::match_results<Iter> results;
    boost::regex expr(pattern_);
    boost::match_flag_type flags = boost::match_default;

    std::ostringstream osSearch;
    while(boost::regex_search(startI, endI, results, expr, flags)) {
        auto& head = results[0];
        const std::string substr(head.first, head.second);
        osSearch << substr << "::";
        startI = head.second;
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
    }
    EXPECT_EQ(expected_, osSearch.str());

    std::ostringstream osIter;
    boost::sregex_token_iterator i {input_.begin(), input_.end(), expr, 1};
    boost::sregex_token_iterator e;
    while(i != e) {
        osIter << *i << "::";
        ++i;
    }
    EXPECT_EQ(expected_, osIter.str());
}

namespace {
    void parseComplexRegex(void) {
        // https://www.checkmarx.com/wp-content/uploads/2015/03/ReDoS-Attacks.pdf
        std::regex expr("^[a-zA-Z]+(([\\'\\,\\.\\- ][a-zA-Z ])?[a-zA-Z]*)*$");
        std::smatch match;
        std::string str = "aaaaaaaaaaaaaaaaaaaaaaaaaaaa!";
        ASSERT_TRUE(std::regex_match(str, match, expr));
    }
}

TEST_F(TestRegex, ReDos) {
    ASSERT_ANY_THROW(parseComplexRegex());
}

namespace {
    static_assert(sizeof(char) == 1, "Expect sizeof(char) == 1");
    static_assert(sizeof('a') == 1, "Expect sizeof(char) == 1");
    static_assert(sizeof(g_arrayForTestingSize) == 1, "g_arrayForTestingSize must have one element");
    static_assert(sizeof(TestingOuterStruct1) > sizeof(TestingOuterStruct1::member), "Unexpected struct size");
    static_assert(sizeof(TestingEmptyStruct) == 1, "Expect sizeof(empty struct) == 1");
    __attribute__((unused)) void funcTakesByte(uint8_t e) {}

    /* これら上とを同時には定義できない
    using BYTETYPE = uint8_t;
    void funcTakesByte(unsigned char e) {}
    void funcTakesByte(BYTETYPE e) {}
    */

    using Paragraph = std::vector<std::string>;
    const std::string MyJoinStrings(const Paragraph& paragraph) {
        std::ostringstream os;
        auto size = paragraph.size();

        for(decltype(size) i=0; i<size; ++i) {
            const auto& str = paragraph.at(i);
            os << str;
            if (((i + 1) < size) && !str.empty() && ::isascii(*(str.rbegin()))) {
                // 正規のUTF-8を仮定し、MSBが1でなければUS-ASCIIとみなす
                os << " ";
            }
        }

        const std::string result = os.str();
        return result;
    }
}

// 行を結合して一行にする
class TestJoinStrings : public ::testing::Test {
protected:
    static const Paragraph sentence1_;
    static const Paragraph sentence2_;
    static const std::string expectedNoSpaces_;
    static const std::string expectedWithSpaces1_;
    static const std::string expectedWithSpaces2_;

    static const Paragraph sentenceJ1_;
    static const Paragraph sentenceJ2_;
    static const std::string expectedJapanese_;
    static const std::string expectedWithSpacesJ1_;
    static const std::string expectedWithSpacesJ2_;
};

// https://www.rain.org/~mkummel/stumpers/15feb02a.html
const Paragraph TestJoinStrings::sentence1_ {"Now", "here", "is", "water"};
const Paragraph TestJoinStrings::sentence2_ {"Nowhere", "is", "water"};
const std::string TestJoinStrings::expectedNoSpaces_ = "Nowhereiswater";
const std::string TestJoinStrings::expectedWithSpaces1_ = "Now here is water";
const std::string TestJoinStrings::expectedWithSpaces2_ = "Nowhere is water";

const Paragraph TestJoinStrings::sentenceJ1_ {"弁慶が", "なぎなたを振り回し"};
const Paragraph TestJoinStrings::sentenceJ2_ {"弁慶がな", "ぎなたを振り回し"};
const std::string TestJoinStrings::expectedJapanese_ = "弁慶がなぎなたを振り回し";
const std::string TestJoinStrings::expectedWithSpacesJ1_ = "弁慶が なぎなたを振り回し";
const std::string TestJoinStrings::expectedWithSpacesJ2_ = "弁慶がな ぎなたを振り回し";

TEST_F(TestJoinStrings, English) {
    const auto actualNoSpaces1 = boost::algorithm::join(sentence1_, "");
    const auto actualNoSpaces2 = boost::algorithm::join(sentence2_, "");
    const auto actualWithSpaces1 = boost::algorithm::join(sentence1_, " ");
    const auto actualWithSpaces2 = boost::algorithm::join(sentence2_, " ");
    EXPECT_EQ(expectedNoSpaces_, actualNoSpaces1);
    EXPECT_EQ(expectedNoSpaces_, actualNoSpaces2);
    EXPECT_EQ(expectedWithSpaces1_, actualWithSpaces1);
    EXPECT_EQ(expectedWithSpaces2_, actualWithSpaces2);

    const auto actual1 = MyJoinStrings(sentence1_);
    const auto actual2 = MyJoinStrings(sentence2_);
    EXPECT_EQ(expectedWithSpaces1_, actual1);
    EXPECT_EQ(expectedWithSpaces2_, actual2);
}

TEST_F(TestJoinStrings, NonAlpha) {
    const Paragraph vec {"1", "2 ", "", "3!", "$"};
    const std::string expected = "1 2  3! $";
    const auto actual = MyJoinStrings(vec);
    EXPECT_EQ(expected, actual);
}

TEST_F(TestJoinStrings, Japanese) {
    const auto actualWithSpaces1 = boost::algorithm::join(sentenceJ1_, " ");
    const auto actualWithSpaces2 = boost::algorithm::join(sentenceJ2_, " ");
    EXPECT_EQ(expectedWithSpacesJ1_, actualWithSpaces1);
    EXPECT_EQ(expectedWithSpacesJ2_, actualWithSpaces2);

    const auto actual1 = MyJoinStrings(sentenceJ1_);
    const auto actual2 = MyJoinStrings(sentenceJ2_);
    EXPECT_EQ(expectedJapanese_, actual1);
    EXPECT_EQ(expectedJapanese_, actual2);
}

// UTF-8を解釈する
class TestUtfCharCounter : public ::testing::Test{};

TEST_F(TestUtfCharCounter, Well) {
    // http://en.cppreference.com/w/cpp/locale/wstring_convert/from_bytes
    // の例で、utf8.data()を使っているが、utf8.data()は
    // C++11以前では null terminateされているとは限らない(C++11ではされている)
    std::string utf8jp = "かばんちゃん急に何を言い出すの";
    std::u16string utf16jp = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8jp);
    EXPECT_EQ(15, utf16jp.size());

    // ラッキービーストは複数いるはず
    std::string utf8 = "I'm a lucky beast";
    std::u16string utf16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8);
    EXPECT_EQ(17, utf16.size());

    constexpr size_t length = 5;
    std::vector<char> vec(length, ' ');
    vec.push_back(0);
    std::u16string utf16sp = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(vec.data());
    EXPECT_EQ(length, utf16sp.size());
}

TEST_F(TestUtfCharCounter, ByteOrderMark) {
    // BOM + 半角空白
    const std::vector<uint8_t> elements {0xef, 0xbb, 0xbf, 0x20};

    std::vector<char> vec;
    for(auto e : elements) {
        vec.push_back(static_cast<char>(e));
    }
    vec.push_back(0);

    bool thrown = false;
    try {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(vec.data());
    } catch(std::range_error& e) {
        thrown = true;
    }
    EXPECT_FALSE(thrown);
}

TEST_F(TestUtfCharCounter, Ill) {
    // 半角空白 = 00100000 をわざと冗長なUTF-8で表現する
    // 11100000 10000000 10100000
    const std::vector<uint8_t> elements {0xe0, 0x80, 0xa0};

    constexpr size_t length = 5;
    std::vector<char> vec;
    for(size_t i=0; i<length; ++i) {
        for(auto e : elements) {
            vec.push_back(static_cast<char>(e));
        }
    }
    vec.push_back(0);

    bool thrown = false;
    try {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(vec.data());
    } catch(std::range_error& e) {
        thrown = true;
    }
    EXPECT_TRUE(thrown);
}

// ファイル出力
class TestFileStream : public ::testing::Test {};

TEST_F(TestFileStream, Close) {
    EXPECT_FALSE(errno);
    {
        std::ofstream fs1;
        fs1.exceptions(std::ifstream::failbit);
        // ファイルに関連付けられていなければcloseに失敗する
        ASSERT_ANY_THROW(fs1.close());
    }
    EXPECT_FALSE(errno);

    {
        std::ofstream fs2;
        fs2.exceptions(std::ifstream::failbit);
        fs2 << "test";
        // ofstreamのデストラクタは失敗しても何も教えてくれない
    }
    EXPECT_FALSE(errno);
}

// 整数型が持てる最大桁数
static_assert(std::numeric_limits<uint64_t>::digits10 == 19, "");
static_assert(std::numeric_limits<int64_t>::digits10 == 18, "");

// 敢えて自作する
template <typename T>
constexpr int MyNumericLimits(T a, int digits) {
    auto n = a * 10 + 9;
    return (n > a) ? MyNumericLimits(n, digits + 1) : digits;
}

template <typename T>
constexpr int MyNumericLimits(void) {
    return MyNumericLimits<T>(9,1);
}

static_assert(MyNumericLimits<uint64_t>() == 19, "");
static_assert(MyNumericLimits<int64_t>() == 18, "");

constexpr int MyIntMinExplicit(int l, int r) {
    return (l < r) ? l : r;
}

template <typename T>
constexpr T MyMin(T l, T r) {
    return (l < r) ? l : r;
}
// typedefは使えない
auto const MyIntMinAlias = &MyMin<int>;

// Cではこうするが、C++ではfunction traitsが使えない
#define CPPFRIENDS_MY_MACRO_MIN(l, r) ((l < r) ? l : r)

static_assert(std::is_same<int, decltype(MyIntMinExplicit(0,0))>::value, "");
static_assert(std::is_same<int, decltype(MyIntMinAlias(0,0))>::value, "");
static_assert(std::is_same<int, boost::function_traits<decltype(MyIntMinExplicit)>::arg1_type>::value, "");
static_assert(std::is_same<int, boost::function_traits<decltype(*MyIntMinExplicit)>::arg1_type>::value, "");
static_assert(std::is_same<int, boost::function_traits<decltype(*MyIntMinAlias)>::arg1_type>::value, "");
// こうは書けない
// static_assert(std::is_same<int, boost::function_traits<decltype(MyIntMinAlias)>::arg1_type>::value, "");
// static_assert(std::is_same<int, boost::function_traits<decltype(CPPFRIENDS_MY_MACRO_MIN)>::arg1_type>::value, "");

// #define MY_MACRO_POW(base, exp) ((exp) ? (base * MY_MACRO_POW(base, exp - 1)) : 1)

// 桁あふれは考慮していない
template <typename T>
constexpr T MyIntegerPow(T base, T exp) {
    return (exp) ? (base * MyIntegerPow(base, exp - 1)) : 1;
}

class TestCppMacro : public ::testing::Test {};

TEST_F(TestCppMacro, Plain) {
    EXPECT_EQ(-1, MyIntMinExplicit(-1, 0));
    EXPECT_EQ(-1, MyIntMinExplicit(0, -1));
    EXPECT_EQ(-1, MyIntMinAlias(-1, 0));
    EXPECT_EQ(-1, MyIntMinAlias(0, -1));
    EXPECT_EQ(-1, CPPFRIENDS_MY_MACRO_MIN(-1, 0));
    EXPECT_EQ(-1, CPPFRIENDS_MY_MACRO_MIN(0, -1));
}

TEST_F(TestCppMacro, Recursive) {
    using Data = unsigned int;
    Data base = 2;
    for(Data i=0; i<10; ++i) {
        EXPECT_EQ(1 << i, MyIntegerPow(base, i));
    }

    base = 3;
    Data expected = 1;
    for(Data i=0; i<5; ++i) {
        EXPECT_EQ(expected, MyIntegerPow(base, i));
        expected *= 3;
    }

    // こうは書けない
    // std::cout << MY_MACRO_POW(2,0);
}

class TestPrimalityTesting : public ::testing::Test{};

TEST_F(TestPrimalityTesting, MersenneNumber) {
    boost::random::random_device seed;
    std::mt19937 gen(seed);

    const unsigned int index[] = {3,5,7,13,17,19,31,61,89,107,127,521,607};
    using BigNumber = boost::multiprecision::uint1024_t;
    BigNumber base = 2;
    for(auto i : index) {
        BigNumber n = boost::multiprecision::pow(base, i);
        n -= 1;
        EXPECT_TRUE(boost::multiprecision::miller_rabin_test(n, 25, gen));
    }
}

// Googleの看板"{first 10-digit prime found in consecutive digits of e}.com"を解く
TEST_F(TestPrimalityTesting, QuizBoard) {
    using LongFloat = boost::multiprecision::number<boost::multiprecision::cpp_dec_float<200>>;
    // const std::string にすると、770行のエラーメッセージが出る
    // decltype(str) = const std::string なので変換できない
    // std::string str = boost::math::constants::e<LongFloat>().convert_to<decltype(str)>();
    const auto str = boost::math::constants::e<LongFloat>().convert_to<std::string>();

    boost::random::random_device seed;
    std::mt19937 gen(seed);
    long long int digits = 0;
    decltype(digits) solution = 0;
    constexpr decltype(digits) expected = 7427466391ll;
    constexpr decltype(digits) base = 10000000000ll;  // 10桁
    static_assert(std::numeric_limits<decltype(digits)>::max() > base, "Too small");

    size_t i = 0;
    char digit[] = {'\0', '\0'};
    for(auto c : str) {
        ++i;
        if (!::isdigit(c)) {
            continue;
        }

        digits *= 10;
        digit[0] = c;
        digits += boost::lexical_cast<decltype(digits)>(digit);
        digits %= base;
        if ((i >= 10) && (boost::multiprecision::miller_rabin_test(digits, 25, gen))) {
            solution = digits;
            break;
        }
    }

    EXPECT_EQ(expected, solution);
}

// 時刻と時差を扱う
namespace {
    std::string convertLocalTimeToUTC(const std::string& timeStr, const std::string& localeStr) {
        auto* input_facet = new boost::posix_time::time_input_facet("%d/%m/%Y %H:%M:%S %ZP");
        std::istringstream is(timeStr);
        boost::locale::generator gen;
        if (!localeStr.empty()) {
            is.imbue(gen(localeStr));
        }
        is.imbue(std::locale(is.getloc(), input_facet));
        boost::local_time::local_date_time lt(boost::posix_time::not_a_date_time);
        is >> lt;

        std::ostringstream os;
        os << lt.utc_time();
        return os.str();
    }
}

class TestDateFormat : public ::testing::Test{};

TEST_F(TestDateFormat, LeapSecond) {
    const char* dataDormat = "%Y-%m-%d %H:%M:%S";
    std::tm t = {};

    {
        std::istringstream is("2017-01-01 08:59:59");
        is >> std::get_time(&t, dataDormat);
        ASSERT_FALSE(is.fail());
        EXPECT_EQ(1483228799, std::mktime(&t));
    }

    {
        std::istringstream is("2017-01-01 08:59:60");
        is >> std::get_time(&t, dataDormat);
        ASSERT_FALSE(is.fail());
        EXPECT_EQ(1483228800, std::mktime(&t));

    }

    {
        std::istringstream is("2017-01-01 08:59:61");
        is >> std::get_time(&t, dataDormat);
        ASSERT_TRUE(is.fail());
    }
}

TEST_F(TestDateFormat, RepeatTime) {
    EXPECT_EQ("2017-Oct-29 00:30:00", convertLocalTimeToUTC("29/10/2017 01:30:00 BST+1", "en_GB.UTF-8"));
    EXPECT_EQ("2017-Oct-29 01:30:00", convertLocalTimeToUTC("29/10/2017 01:30:00 GMT+0", "en_GB.UTF-8"));
    EXPECT_EQ("2017-Oct-28 16:30:00", convertLocalTimeToUTC("29/10/2017 01:30:00 JST+9", "ja_JP.UTF-8"));
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
