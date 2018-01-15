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

class TestMySingletonTest : public ::testing::Test {};

TEST_F(TestMySingletonTest, All) {
    auto& instance1 = MySingletonClass::GetInstance();
    EXPECT_EQ(1, instance1.GetValue());
    auto& instance2 = MySingletonClass::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
