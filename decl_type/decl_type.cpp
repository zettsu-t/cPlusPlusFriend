#include <boost/cast.hpp>
#include <boost/version.hpp>
#include <gtest/gtest.h>

class TestNumericCast : public ::testing::Test {};

TEST_F(TestNumericCast, ExplicitType) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        double a = boost::numeric_cast<double>(2)/v;
        EXPECT_DOUBLE_EQ(0.5, a);
    }
}

TEST_F(TestNumericCast, ReceiveAsAuto) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        auto a = boost::numeric_cast<double>(2)/v;
        EXPECT_DOUBLE_EQ(0.5, a);
    }
}

// These tests cause crashes due to references to temporary objects
// in /usr/include/boost/numeric/conversion/converter_policies.hpp : line 186
#if 0
TEST_F(TestNumericCast, ExplicitRef) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        double a = boost::numeric_cast<const double&>(2)/v;
        EXPECT_DOUBLE_EQ(0.5, a);
    }
}

TEST_F(TestNumericCast, DeclTypeBoost) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        auto a = boost::numeric_cast<decltype(v)>(2)/v;
        EXPECT_DOUBLE_EQ(0.5, a);
    }
}

TEST_F(TestNumericCast, DeclTypeStatic) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        auto a = static_cast<decltype(v)>(2)/v;
        EXPECT_DOUBLE_EQ(0.5, a);
    }
}
#endif

TEST_F(TestNumericCast, RemoveRef) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        auto a = boost::numeric_cast<std::remove_reference_t<decltype(v)>>(2)/v;
        EXPECT_DOUBLE_EQ(0.5, a);
    }
}

template <typename Target, typename Source,
          std::enable_if_t<
              std::is_arithmetic_v<std::remove_reference_t<Target>>,
              std::nullptr_t> = nullptr>
auto wrap_numeric_cast(Source&& s) {
    return boost::numeric_cast<std::remove_reference_t<Target>>(std::forward<Source>(s));
}

class TestWrapNumericCast : public ::testing::Test {};

TEST_F(TestWrapNumericCast, ExplicitType) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        double a = wrap_numeric_cast<double>(2)/v;
        EXPECT_DOUBLE_EQ(0.5, a);
    }
}

TEST_F(TestWrapNumericCast, ReceiveAsAuto) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        auto a = wrap_numeric_cast<double>(2)/v;
        static_assert(std::is_same_v<double, decltype(a)>);
        EXPECT_DOUBLE_EQ(0.5, a);
    }
}

TEST_F(TestWrapNumericCast, ExplicitRef) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        double a = wrap_numeric_cast<const double&>(2)/v;
        EXPECT_DOUBLE_EQ(0.5, a);
    }
}

TEST_F(TestWrapNumericCast, RefAndAuto) {
    std::vector<double> vs {4.0};
    for(const auto& v : vs) {
        const auto a = wrap_numeric_cast<decltype(v)>(2)/v;
        static_assert(std::is_same_v<const double, decltype(a)>);
        EXPECT_DOUBLE_EQ(0.5, a);
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
