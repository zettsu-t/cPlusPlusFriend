#include <gtest/gtest.h>
#include "code.h"

class TestPopcount : public ::testing::Test {};

TEST_F(TestPopcount, Ordinary) {
    const NumberVector xs {0, 1, 2, 3, 4, 5, 6, 7, 8};
    const NumberVector expected {0, 1, 1, 2, 1, 2, 2, 3, 1};
    const auto actual = cpp_popcount(xs);

    ASSERT_EQ(expected.size(), actual.size());
    decltype(expected.size()) i = 0;
    for (const auto& y : expected){
        ASSERT_EQ(expected.at(i), y);
        ++i;
    }
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
