#include <boost/cast.hpp>
#include <boost/version.hpp>
#include <gtest/gtest.h>

class TestNumericCast : public ::testing::Test {};

TEST_F(TestNumericCast, ExplicitType) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        double a=boost::numeric_cast<double>(2)/v;
        EXPECT_DOUBLE_EQ(0.5,a);
    }
}

TEST_F(TestNumericCast, ReceiveAsAuto) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        auto a=boost::numeric_cast<double>(2)/v;
        EXPECT_DOUBLE_EQ(0.5,a);
    }
}

TEST_F(TestNumericCast, ExplicitRef) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        double a=boost::numeric_cast<const double&>(2)/v;
        EXPECT_DOUBLE_EQ(0.5,a);
    }
}

TEST_F(TestNumericCast, DeclType) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        auto a=boost::numeric_cast<decltype(v)>(2)/v;
        EXPECT_DOUBLE_EQ(0.5,a);
    }
}

TEST_F(TestNumericCast, RemoveRef) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        auto a=boost::numeric_cast<std::remove_reference_t<decltype(v)>>(2)/v;
        EXPECT_DOUBLE_EQ(0.5,a);
    }
}

int main(int argc, char* argv[]) {
    std::cerr << BOOST_VERSION << std::endl;
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
