#include <limits>
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

class TestStructuredBinding  : public ::testing::Test {};

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
        auto [result, quotient] = MyDiv(n, zero);
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

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
