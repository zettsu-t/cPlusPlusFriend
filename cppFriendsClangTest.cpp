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
    double d = llNumber;

    std::ostringstream os;
    // 整数部と.の分だけ多く精度を取る
    os << std::setprecision(3 + std::numeric_limits<decltype(llNumber)>::digits10) << d;
    const std::string actual = os.str();

    // 下位の桁は保存されない
    EXPECT_TRUE(std::numeric_limits<decltype(d)>::is_iec559);
    EXPECT_NE(base, actual);

    // 上位の桁は保存される
    std::string expected = base;
    expected.substr(0, base.size() - 3);
    EXPECT_EQ(0, actual.find_first_of(expected));
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
