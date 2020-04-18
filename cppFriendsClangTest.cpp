#include <iostream>
#include <limits>
#include <boost/lexical_cast.hpp>
#include <gtest/gtest.h>
#include "cppFriendsClang.hpp"

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

class TestShiftTooManyLto : public ::testing::Test{};

TEST_F(TestShiftTooManyLto, All) {
    EXPECT_EQ(8, ShiftManyFor1(3));

#ifdef CPPFRIENDS_ENABLE_LTO
    EXPECT_EQ(0, ShiftManyFor1(35));
#else
    EXPECT_EQ(8, ShiftManyFor1(35));
#endif
}

class TestNarrowingCast : public ::testing::Test{};

TEST_F(TestNarrowingCast, LongLongToDouble) {
    // 0x7fffffffffff00ff
    const std::string base = "9223372036854710527";
    long long int llNumber = boost::lexical_cast<decltype(llNumber)>(base);
    // g++もclang++も警告を出さないが、64-bitを仮数部52-bitに入れるので精度が失われる
    double d = llNumber;  // ビット数が減るので警告が出るが意図的

    std::ostringstream os;
    // 整数部と.の分だけ多く精度を取る
    os << std::setprecision(3 + std::numeric_limits<decltype(llNumber)>::digits10) << d;
    const std::string actual = os.str();

    // 下位の桁は保存されない
#if !defined(__MINGW32__) || defined(__MINGW64__)
    EXPECT_TRUE(std::numeric_limits<decltype(d)>::is_iec559);
#endif
    EXPECT_NE(base, actual);

    // 上位の桁は保存される
    std::string expected = base;
    expected.substr(0, base.size() - 3);
    EXPECT_EQ(0, actual.find_first_of(expected));
}

// 敢えてdo-whileで囲まない
#define DEBUG_PRINT_COUT(osout, str) osout << str << "@" << __PRETTY_FUNCTION__ << "in " << __FILE__ << " : " << __LINE__;

// strをosoutとoserrの両方に書き出す
#define ILL_DEBUG_PRINT(osout, oserr, str) \
    osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \


#define WELL_DEBUG_PRINT(osout, oserr, str) \
    do { \
        osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    } while(0)

// ;が余計
#define BAD_DEBUG_PRINT1(osout, oserr, str) \
    do { \
        osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    } while(0);                                            \

#define BAD_DEBUG_PRINT2(osout, oserr, str) \
    { \
        osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    }

class TestMacroExpansion : public ::testing::Test {};

TEST_F(TestMacroExpansion, SingleStatement) {
    std::ostringstream out;

    std::string message {"Event"};
    std::string log = message;
    log += "@";
    log += __PRETTY_FUNCTION__;
    log += "in cppFriendsClangTest.cpp : 118";

    bool cond = !out.tellp();
    if (cond) DEBUG_PRINT_COUT(out, message);
    EXPECT_EQ(log, out.str());
}

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
    bool cond = !out.tellp();
    if (cond) ILL_DEBUG_PRINT(out, err, message);
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

    bool cond = !out.tellp();
    if (cond) WELL_DEBUG_PRINT(out, err, message);
    EXPECT_EQ(log, out.str());
    EXPECT_EQ(log, err.str());

#ifdef ONLY_PREPROCESSING
    if (cond) BAD_DEBUG_PRINT1(out, err, message); else {}

    if (cond) BAD_DEBUG_PRINT2(out, err, message); else {}
#endif
}

// https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html
// で規定されるGCC拡張
// fprintfはintを返す
#define FUNC_DEBUG_PRINTF(fdout, fderr, str) \
    ({ \
        fprintf(fdout, "%s@%s", str, __PRETTY_FUNCTION__ ); \
        fprintf(fderr, "%s@%s", str, __PRETTY_FUNCTION__ ); \
    }) \

// oserrで終わると、oserrをコピーできないというコンパイルエラーが出るので
// 何か返す
#define FUNC_DEBUG_COUT(osout, oserr, str) \
    ({ \
        osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        0; \
    }) \

// Makefileからgrepで調べる
class TestCompoundStatement : public ::testing::Test{};

TEST_F(TestCompoundStatement, Printf) {
    if (true) FUNC_DEBUG_PRINTF(stdout, stderr, "Printf in compound statements");
}

TEST_F(TestCompoundStatement, Cout) {
    std::ostringstream out;
    std::ostringstream err;

    std::string message {"Cout in compound statements"};
    std::string log = message;
    log += "@";
    log += __PRETTY_FUNCTION__;
    log += ":";

    bool cond = !out.tellp();
    if (cond) FUNC_DEBUG_COUT(out, err, message);
    EXPECT_EQ(log, out.str());
    EXPECT_EQ(log, err.str());
}

TEST_F(TestCompoundStatement, Loop) {
    std::ostringstream out;
    std::ostringstream err;

    std::string message;
    std::string expected;
    for(int i=1; i<=2; ++i, FUNC_DEBUG_COUT(out, err, message)) {
        message = boost::lexical_cast<decltype(message)>(i);
        std::string log = message;
        log += "@";
        log += __PRETTY_FUNCTION__;
        log += ":";
        expected += log;
    };

    EXPECT_EQ(expected, out.str());
    EXPECT_EQ(expected, err.str());
}

class TestConditionalMove : public ::testing::Test{};

TEST_F(TestConditionalMove, All) {
    ConditionalMove::SudokuCell cell;
    ConditionalMove::SudokuCell other;
    EXPECT_FALSE(cell.candidates_);
    EXPECT_FALSE(other.candidates_);

    cell.candidates_ = 3;
    other.candidates_ = 0;
    EXPECT_FALSE(cell.has_unique_candidate());
    EXPECT_FALSE(cell.has_no_or_unique_candidates());
    EXPECT_FALSE(other.has_unique_candidate());
    EXPECT_TRUE(other.has_no_or_unique_candidates());
    cell.filter_by_candidates_1(other);
    EXPECT_EQ(3, cell.candidates_);

    cell.candidates_ = 3;
    other.candidates_ = 0;
    cell.filter_by_candidates_2(other);
    EXPECT_EQ(3, cell.candidates_);

    cell.candidates_ = 3;
    other.candidates_ = 2;
    EXPECT_TRUE(other.has_unique_candidate());
    EXPECT_TRUE(other.has_no_or_unique_candidates());
    cell.filter_by_candidates_1(other);
    EXPECT_EQ(1, cell.candidates_);

    cell.candidates_ = 3;
    other.candidates_ = 2;
    cell.filter_by_candidates_2(other);
    EXPECT_EQ(1, cell.candidates_);
}

namespace InferVariadicTemplate {
#if !defined(__clang__)
    auto MyApply3(auto& f, auto&&... v) {
        return f(std::forward<decltype(v)>(v)...);
    };
#endif
}

class TestInferVariadicTemplate : public ::testing::Test{};

TEST_F(TestInferVariadicTemplate, All) {
    auto sumTwo = [](const auto& lhs, const auto& rhs) {
        return lhs + rhs;
    };

    EXPECT_EQ(5, InferVariadicTemplate::MyApply(sumTwo, 2, 3));

#if !defined(__clang__)
    EXPECT_EQ(7, InferVariadicTemplate::MyApply2(sumTwo, 3, 4));
    EXPECT_EQ(9, InferVariadicTemplate::MyApply3(sumTwo, 4, 5));
#endif
}

class TestMyDivider : public ::testing::Test{};

TEST_F(TestMyDivider, Int8) {
    using Number = int8_t;
    constexpr Number dividend = 13;
    constexpr Number divider = 3;
    constexpr Number expectedQuo = 4;
    constexpr Number expectedRem = 1;
    EXPECT_EQ(expectedQuo, DivInt8(dividend, divider));

    const auto actual = DivRemInt8(dividend, divider);
    EXPECT_EQ(expectedQuo, actual.first);
    EXPECT_EQ(expectedRem, actual.second);

    const auto actualNegative = DivRemInt8(-dividend, divider);
    EXPECT_EQ(-expectedQuo, actualNegative.first);
    EXPECT_EQ(-expectedRem, actualNegative.second);
}

TEST_F(TestMyDivider, Uint8) {
    using Number = uint8_t;
    constexpr Number dividend = 133;
    constexpr Number divider = 33;
    constexpr Number expectedQuo = 4;
    constexpr Number expectedRem = 1;
    EXPECT_EQ(expectedQuo, DivUint8(dividend, divider));

    const auto actual = DivRemUint8(dividend, divider);
    EXPECT_EQ(expectedQuo, actual.first);
    EXPECT_EQ(expectedRem, actual.second);
}

TEST_F(TestMyDivider, Int16) {
    using Number = int16_t;
    constexpr Number dividend = 23333;
    constexpr Number divider = 7;
    constexpr Number expectedQuo = 3333;
    constexpr Number expectedRem = 2;
    EXPECT_EQ(expectedQuo, DivInt16(dividend, divider));

    const auto actual = DivRemInt16(dividend, divider);
    EXPECT_EQ(expectedQuo, actual.first);
    EXPECT_EQ(expectedRem, actual.second);

    const auto actualNegative = DivRemInt16(-dividend, divider);
    EXPECT_EQ(-expectedQuo, actualNegative.first);
    EXPECT_EQ(-expectedRem, actualNegative.second);
}

TEST_F(TestMyDivider, Uint16) {
    using Number = uint16_t;
    constexpr Number dividend = 53333;
    constexpr Number divider = 17;
    constexpr Number expectedQuo = 3137;
    constexpr Number expectedRem = 4;
    EXPECT_EQ(expectedQuo, DivUint16(dividend, divider));

    const auto actual = DivRemUint16(dividend, divider);
    EXPECT_EQ(expectedQuo, actual.first);
    EXPECT_EQ(expectedRem, actual.second);
}

TEST_F(TestMyDivider, Int32) {
    using Number = int32_t;
    constexpr Number dividend = 1333333333;
    constexpr Number divider = 19;
    constexpr Number expectedQuo = 70175438;
    constexpr Number expectedRem = 11;
    EXPECT_EQ(expectedQuo, DivInt32(dividend, divider));

    const auto actual = DivRemInt32(dividend, divider);
    EXPECT_EQ(expectedQuo, actual.first);
    EXPECT_EQ(expectedRem, actual.second);

    const auto actualNegative = DivRemInt32(-dividend, divider);
    EXPECT_EQ(-expectedQuo, actualNegative.first);
    EXPECT_EQ(-expectedRem, actualNegative.second);
}

TEST_F(TestMyDivider, Uint32) {
    using Number = uint32_t;
    constexpr Number dividend = 3333333333u;
    constexpr Number divider = 19u;
    constexpr Number expectedQuo = 175438596u;
    constexpr Number expectedRem = 9u;
    EXPECT_EQ(expectedQuo, DivUint32(dividend, divider));

    const auto actual = DivRemUint32(dividend, divider);
    EXPECT_EQ(expectedQuo, actual.first);
    EXPECT_EQ(expectedRem, actual.second);
}

TEST_F(TestMyDivider, Int64) {
    using Number = int64_t;
    constexpr Number dividend = 8070450532247928832ll;
    constexpr Number divider = 23ll;
    constexpr Number expectedQuo = 350889153575996905ll;
    constexpr Number expectedRem = 17ll;
    EXPECT_EQ(expectedQuo, DivInt64(dividend, divider));

    const auto actual = DivRemInt64(dividend, divider);
    EXPECT_EQ(expectedQuo, actual.first);
    EXPECT_EQ(expectedRem, actual.second);

    const auto actualNegative = DivRemInt64(-dividend, divider);
    EXPECT_EQ(-expectedQuo, actualNegative.first);
    EXPECT_EQ(-expectedRem, actualNegative.second);
}

TEST_F(TestMyDivider, Uint64) {
    using Number = uint64_t;
    constexpr Number dividend = 0xf0000000000000ffull;
    constexpr Number divider = 256ull;
    constexpr Number expectedQuo = 0xf0000000000000ull;
    constexpr Number expectedRem = 255ull;
    EXPECT_EQ(expectedQuo, DivUint64(dividend, divider));

    const auto actual = DivRemUint64(dividend, divider);
    EXPECT_EQ(expectedQuo, actual.first);
    EXPECT_EQ(expectedRem, actual.second);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
