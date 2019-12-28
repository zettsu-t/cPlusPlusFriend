// Cygwin g++ 7.4.0 does not contain charconv
// #define USE_STD_TO_CHARS
#ifdef USE_STD_TO_CHARS
#include <charconv>
#endif

#include <any>
#include <array>
#include <iostream>
#include <limits>
#include <sstream>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <gtest/gtest.h>

// C++17
enum class DivisionResult {
    OK,
    DIVIDE_BY_ZERO,
    OVERFLOW,
};

// 浮動小数と0との比較は警告が出る
template <typename T,
          typename std::enable_if_t<std::numeric_limits<T>::has_denorm, std::nullptr_t> = nullptr>
constexpr std::tuple<DivisionResult, T> MyDiv(const T& numerator, const T& denominator) {
    const bool isZero = (std::is_signed<T>::value) ? (
            (-std::numeric_limits<T>::denorm_min() <= denominator) &&
            (denominator <= std::numeric_limits<T>::denorm_min())) :
         (denominator <= std::numeric_limits<T>::denorm_min());

    if (isZero) {
        return {DivisionResult::DIVIDE_BY_ZERO, static_cast<T>(0)};
    }
    return {DivisionResult::OK, numerator / denominator};
}

template <typename T,
          typename std::enable_if_t<!std::numeric_limits<T>::has_denorm, std::nullptr_t> = nullptr>
constexpr std::tuple<DivisionResult, T> MyDiv(const T& numerator, const T& denominator) {
    if (!denominator) {
        return {DivisionResult::DIVIDE_BY_ZERO, static_cast<T>(0)};
    }

    if ((numerator == std::numeric_limits<T>::max()) &&
        (denominator == std::numeric_limits<T>::min())) {
        return {DivisionResult::OVERFLOW, static_cast<T>(0)};
    }

    if ((numerator == std::numeric_limits<T>::min()) &&
        (denominator == std::numeric_limits<T>::max())) {
        return {DivisionResult::OVERFLOW, static_cast<T>(0)};
    }

    return {DivisionResult::OK, numerator / denominator};
}

class TestStructuredBinding : public ::testing::Test {};

TEST_F(TestStructuredBinding, Int) {
    constexpr int n = 7;
    constexpr int d = 2;
    {
        const auto [result, quotient] = MyDiv(n, d);
        EXPECT_EQ(DivisionResult::OK, result);
        EXPECT_EQ(3, quotient);
    }

    {
        constexpr int zero = 0;
        const auto [result, quotient] = MyDiv(n, zero);
        EXPECT_EQ(DivisionResult::DIVIDE_BY_ZERO, result);
        EXPECT_FALSE(quotient);
    }

    {
        const auto [result, quotient] = MyDiv(std::numeric_limits<int>::max(),
                                              std::numeric_limits<int>::min());
        EXPECT_EQ(DivisionResult::OVERFLOW, result);
        EXPECT_FALSE(quotient);
    }

    {
        const auto [result, quotient] = MyDiv(std::numeric_limits<int>::min(),
                                              std::numeric_limits<int>::max());
        EXPECT_EQ(DivisionResult::OVERFLOW, result);
        EXPECT_FALSE(quotient);
    }
}

TEST_F(TestStructuredBinding, UnsignedInt) {
    constexpr unsigned int n = 0xffff;
    constexpr unsigned int d = 2;
    {
        const auto [result, quotient] = MyDiv(n, d);
        EXPECT_EQ(DivisionResult::OK, result);
        EXPECT_EQ(0x7fff, quotient);
    }

    {
        constexpr unsigned int zero = 0;
        const auto [result, quotient] = MyDiv(n, zero);
        EXPECT_EQ(DivisionResult::DIVIDE_BY_ZERO, result);
        EXPECT_FALSE(quotient);
    }

    {
        const auto [result, quotient] = MyDiv(std::numeric_limits<unsigned int>::max(), 1u);
        EXPECT_EQ(DivisionResult::OK, result);
        EXPECT_EQ(std::numeric_limits<unsigned int>::max(), quotient);
    }
}

TEST_F(TestStructuredBinding, Double) {
    constexpr double n = 7;
    constexpr double d = 2;
    {
        const auto [result, quotient] = MyDiv(n, d);
        EXPECT_EQ(DivisionResult::OK, result);
        EXPECT_DOUBLE_EQ(3.5, quotient);
    }

    {
        constexpr double zero = 0;
        const auto [result, quotient] = MyDiv(n, zero);
        EXPECT_EQ(DivisionResult::DIVIDE_BY_ZERO, result);
        EXPECT_DOUBLE_EQ(0.0, quotient);
    }

    {
        const auto [result, quotient] = MyDiv(n, std::numeric_limits<double>::denorm_min());
        EXPECT_EQ(DivisionResult::DIVIDE_BY_ZERO, result);
        EXPECT_DOUBLE_EQ(0.0, quotient);
    }

    {
        const auto [result, quotient] = MyDiv(n, -std::numeric_limits<double>::denorm_min());
        EXPECT_EQ(DivisionResult::DIVIDE_BY_ZERO, result);
        EXPECT_DOUBLE_EQ(0.0, quotient);
    }

    {
        const auto [result, quotient] = MyDiv(std::numeric_limits<double>::max(), 0.25);
        EXPECT_EQ(DivisionResult::OK, result);
        EXPECT_DOUBLE_EQ(std::numeric_limits<double>::infinity(), quotient);
    }

    {
        const auto [result, quotient] = MyDiv(std::numeric_limits<double>::denorm_min(), 256.0);
        EXPECT_EQ(DivisionResult::OK, result);
        EXPECT_DOUBLE_EQ(0.0, quotient);
    }
}

class TestAnyCastCpp17 : public ::testing::Test {};

TEST_F(TestAnyCastCpp17, AnyCast17) {
    int n = 10;
    int* p = &n;
    std::any obj = p;
    auto pResult = std::any_cast<int*>(p);
    EXPECT_EQ(n, *pResult);
    ASSERT_ANY_THROW(std::any_cast<const int*>(p));
}

// CSV風に、書式を指定して数値を文字列にしたものをjoinする
#ifdef USE_STD_TO_CHARS
class TestJoinFloating : public ::testing::Test {};

TEST_F(TestJoinFloating, Integer) {
    std::ostringstream os;
    std::array<char, 100> str = {0};

    int value = 10;
    constexpr int size = 4;
    for (int i=decltype(size){0}; i<size; ++i) {
        if (auto [p, ec] = std::to_chars(str.data(), str.data() + str.size(), value);
            ec == std::errc()) {
            os << std::string_view(str.data(), p - str.data());
        }

        if ((i + 1) < size) {
            os << ",";
        }
        value /= 3;
    }

    const std::string expected = "10,3,1,0";
    EXPECT_EQ(expected, os.str());
}

TEST_F(TestJoinFloating, ShortBuffer) {
    std::ostringstream os;
    std::array<char, 5> strShort = {0};

    constexpr int value = 123456;
    auto [p, ec] = std::to_chars(strShort.data(), strShort.data() + strShort.size(), value);
    ASSERT_EQ(std::errc::value_too_large, ec);

    std::array<char, 6> strExact = {0};
    if (auto [p, ec] = std::to_chars(strExact.data(), strExact.data() + strExact.size(), value);
        ec == std::errc()) {
        os << std::string_view(strExact.data(), p - strExact.data());
    }

    const std::string expected = "123456";
    EXPECT_EQ(expected, os.str());
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
