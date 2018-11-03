// [[Rcpp::depends(BH)]]
#include <cstdint>
#include <cstring>
#include <bitset>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>

#ifndef TESTING_FROM_MAIN
#include <Rcpp.h>
#else
#include <gtest/gtest.h>
#endif

namespace {
    using WidthSize = std::string::size_type;
    constexpr WidthSize WidthSign = 1;
    constexpr WidthSize WidthExponent = 11;
    constexpr WidthSize WidthFraction = 52;
    const std::vector<WidthSize> WidthSet {WidthSign, WidthExponent, WidthFraction};
    static_assert((WidthSign + WidthExponent + WidthFraction) == sizeof(double) * 8,
                  "Unexpected size");

    template <typename T,
              typename std::enable_if_t<std::is_integral<T>::value, std::nullptr_t> = nullptr>
    std::string intToString(T num) {
        std::bitset<sizeof(num) * 8> bits(num);
        return bits.to_string();
    }

    void SplitDouble(double num, std::vector<std::string>& stringSet) {
        uint64_t intNum = 0;
        static_assert(sizeof(intNum) == sizeof(num), "Unexpected size");
        std::memmove(&intNum, &num, sizeof(num));
        auto bitString = intToString(intNum);

        stringSet.clear();
        WidthSize pos = 0;
        for (auto width : WidthSet) {
            stringSet.push_back(bitString.substr(pos, width));
            pos += width;
        }
        return;
    }
}

#ifndef TESTING_FROM_MAIN
// [[Rcpp::export]]
Rcpp::DataFrame split_double_components(Rcpp::NumericVector vec_number) {
    auto length = vec_number.length();
    Rcpp::CharacterVector vec_sign(length);
    Rcpp::CharacterVector vec_exponent(length);
    Rcpp::CharacterVector vec_fraction(length);

    decltype(length) i = 0;
    for(const auto& num : vec_number) {
        std::vector<std::string> stringSet;
        SplitDouble(num, stringSet);
        vec_sign[i] = stringSet.at(0);
        vec_exponent[i] = stringSet.at(1);
        vec_fraction[i] = stringSet.at(2);
        ++i;
    }

    Rcpp::DataFrame df = Rcpp::DataFrame::create(
        Rcpp::Named("number") = vec_number, Rcpp::Named("sign") = vec_sign,
        Rcpp::Named("exponent") = vec_exponent, Rcpp::Named("fraction") = vec_fraction);
    return df;
}
#else

class TestSampleRcpp : public ::testing::Test {};

TEST_F(TestSampleRcpp, All) {
    struct TestCase {
        double num;
        std::vector<std::string> expected;
    };

    const TestCase testCases[] = {
        {0.0,  {"0", "00000000000", "0000000000000000000000000000000000000000000000000000"}},
        {1.0,  {"0", "01111111111", "0000000000000000000000000000000000000000000000000000"}},
        {1.5,  {"0", "01111111111", "1000000000000000000000000000000000000000000000000000"}},
        {-3.5, {"1", "10000000000", "1100000000000000000000000000000000000000000000000000"}},
        {std::numeric_limits<double>::infinity(),
         {"0", "11111111111", "0000000000000000000000000000000000000000000000000000"}},
        {-std::numeric_limits<double>::infinity(),
         {"1", "11111111111", "0000000000000000000000000000000000000000000000000000"}}
    };

    for(const auto& testCase : testCases) {
        std::vector<std::string> actual;
        SplitDouble(testCase.num, actual);
        decltype(actual)::size_type i = 0;
        for(const auto& expected : testCase.expected) {
            EXPECT_EQ(expected, actual.at(i));
            ++i;
        }
    }
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
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
