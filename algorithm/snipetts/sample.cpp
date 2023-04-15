#include <gtest/gtest.h>

namespace {
    using Num = long long int;
}

Num add(Num a, Num b) {
    return a + b;
}

class TestAll : public ::testing::Test {};

TEST_F(TestAll, TestAdd) {
    EXPECT_EQ(3, add(1, 2));
}

TEST_F(TestAll, TestAdd2) {
    EXPECT_EQ(-8, add(-5, -3));
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
