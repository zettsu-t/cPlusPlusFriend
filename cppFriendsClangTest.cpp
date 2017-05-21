#include <iostream>
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

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
