// GoogleTest 1.10はC++11以上が必須
#if 0
#include <boost/fusion/container/vector.hpp>
#include <gtest/gtest.h>

// C++98にはstatic_assertもstd::is_sameもない
// Boostのを使うこともできますが
class TestCpp98BoostFusionVector : public ::testing::Test{};

#if __cplusplus >= 201103L
// C++98にはstatic_assertがない
static_assert(sizeof(char) == 1, "Unexpected char size");
#endif

TEST_F(TestCpp98BoostFusionVector, Numbered) {
#if (__GNUC__ >= 6)
    // GCC 6.3.0は、-std=gnu++98 と明記しないと-std=gnu++14になる
    // なので、以下のコメントアウトを外すと、C++14として通る
    // static_assert(sizeof(char) == 1, "");
#endif
    EXPECT_NE(typeid(boost::fusion::vector<int,int>), typeid(boost::fusion::vector2<int,int>));
}
#endif

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
