#include <algorithm>
#include <chrono>
#include <execution>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include <gtest/gtest.h>

namespace {
using Number = int;
using Numbers = std::vector<Number>;
}

void sort_sequential(Numbers& nums) {
    std::sort(nums.begin(), nums.end());
}

void sort_vectorized(Numbers& nums) {
    std::sort(std::execution::unseq, nums.begin(), nums.end());
}

void sort_parallel(Numbers& nums) {
    std::sort(std::execution::par, nums.begin(), nums.end());
}

void sort_parallel_vectorized(Numbers& nums) {
    std::sort(std::execution::par_unseq, nums.begin(), nums.end());
}

constexpr void twice(Number& x) {
    x *= 2;
}

void for_each_sequential(Numbers& nums) {
    std::for_each(nums.begin(), nums.end(), twice);
}

void for_each_vectorized(Numbers& nums) {
    std::for_each(std::execution::unseq, nums.begin(), nums.end(), twice);
}

void for_each_parallel(Numbers& nums) {
    std::for_each(std::execution::par, nums.begin(), nums.end(), twice);
}

void for_each_parallel_vectorized(Numbers& nums) {
    std::for_each(std::execution::par_unseq, nums.begin(), nums.end(), twice);
}

Numbers generate_numbers(size_t size, Number max_num) {
    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());
    std::uniform_int_distribution<Number> dist(static_cast<Number>(0), max_num);

    Numbers vec(size, 0);
    for(auto&& e : vec) {
        e = dist(engine);
    }
    return vec;
}

void print_duration_in_msec(const std::chrono::steady_clock::time_point& start,
                            std::ostream& os) {
    const auto timestamp = std::chrono::steady_clock::now();
    const auto dur = std::chrono::duration_cast<std::chrono::milliseconds>
        (timestamp - start).count();
    os << dur << "msec\n";
}

class TestFunctions : public ::testing::Test {};

TEST_F(TestFunctions, SortFixed) {
    const Numbers input {1, 7, 6, 2, 3, 4, 5, 3};
    const Numbers expected {1, 2, 3, 3, 4, 5, 6, 7};
    const size_t size = input.size();

    auto vec_sequential = input;
    const auto start_sequential = std::chrono::steady_clock::now();
    sort_sequential(vec_sequential);

    auto vec_vectorized = input;
    const auto start_vectorized = std::chrono::steady_clock::now();
    sort_vectorized(vec_vectorized);

    auto vec_parallel = input;
    const auto start_parallel = std::chrono::steady_clock::now();
    sort_parallel(vec_parallel);

    auto vec_parallel_vectorized = input;
    const auto start_parallel_vectorized = std::chrono::steady_clock::now();
    sort_parallel_vectorized(vec_parallel_vectorized);

    ASSERT_EQ(expected.size(), input.size());
    ASSERT_EQ(expected.size(), vec_sequential.size());
    ASSERT_EQ(expected.size(), vec_vectorized.size());
    ASSERT_EQ(expected.size(), vec_parallel.size());
    ASSERT_EQ(expected.size(), vec_parallel_vectorized.size());

    EXPECT_TRUE(std::equal(expected.begin(), expected.end(),
                           vec_sequential.begin(), vec_sequential.end()));
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(),
                           vec_vectorized.begin(), vec_vectorized.end()));
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(),
                           vec_parallel.begin(), vec_parallel.end()));
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(),
                           vec_parallel_vectorized.begin(), vec_parallel_vectorized.end()));
}

TEST_F(TestFunctions, SortRandom) {
    constexpr size_t size = 10000000;
    constexpr Number max_num = std::numeric_limits<Number>::max() / 8;
    const auto input = generate_numbers(size, max_num);

    auto vec_sequential = input;
    const auto start_sequential = std::chrono::steady_clock::now();
    sort_sequential(vec_sequential);
    print_duration_in_msec(start_sequential, std::cout);

    auto vec_vectorized = input;
    const auto start_vectorized = std::chrono::steady_clock::now();
    sort_parallel(vec_vectorized);
    print_duration_in_msec(start_vectorized, std::cout);

    auto vec_parallel = input;
    const auto start_parallel = std::chrono::steady_clock::now();
    sort_parallel(vec_parallel);
    print_duration_in_msec(start_parallel, std::cout);

    auto vec_parallel_vectorized = input;
    const auto start_parallel_vectorized = std::chrono::steady_clock::now();
    sort_parallel_vectorized(vec_parallel_vectorized);
    print_duration_in_msec(start_parallel_vectorized, std::cout);

    ASSERT_EQ(input.size(), vec_sequential.size());
    ASSERT_EQ(input.size(), vec_vectorized.size());
    ASSERT_EQ(input.size(), vec_parallel.size());
    ASSERT_EQ(input.size(), vec_parallel_vectorized.size());

    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(),
                           vec_vectorized.begin(), vec_vectorized.end()));
    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(),
                           vec_parallel.begin(), vec_parallel.end()));
    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(),
                           vec_parallel_vectorized.begin(), vec_parallel_vectorized.end()));
}

TEST_F(TestFunctions, ForEachFixed) {
    const Numbers input {1, 7, 6, 5};
    const Numbers expected {2, 14, 12, 10};

    auto vec_sequential = input;
    for_each_sequential(vec_sequential);

    auto vec_vectorized = input;
    for_each_sequential(vec_vectorized);

    auto vec_parallel = input;
    for_each_parallel(vec_parallel);

    auto vec_parallel_vectorized = input;
    for_each_parallel_vectorized(vec_parallel_vectorized);

    ASSERT_EQ(input.size(), vec_sequential.size());
    ASSERT_EQ(input.size(), vec_vectorized.size());
    ASSERT_EQ(input.size(), vec_parallel.size());
    ASSERT_EQ(input.size(), vec_parallel_vectorized.size());

    EXPECT_TRUE(std::equal(expected.begin(), expected.end(),
                           vec_sequential.begin(), vec_sequential.end()));
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(),
                           vec_vectorized.begin(), vec_vectorized.end()));
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(),
                           vec_parallel.begin(), vec_parallel.end()));
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(),
                           vec_parallel_vectorized.begin(), vec_parallel_vectorized.end()));
}

TEST_F(TestFunctions, ForEachRandom) {
    constexpr size_t size = 80000000;
    constexpr Number max_num = std::numeric_limits<Number>::max() / 8;
    const auto input = generate_numbers(size, max_num);

    auto vec_sequential = input;
    const auto start_sequential = std::chrono::steady_clock::now();
    for_each_sequential(vec_sequential);
    print_duration_in_msec(start_sequential, std::cout);

    auto vec_vectorized = input;
    const auto start_vectorized = std::chrono::steady_clock::now();
    for_each_parallel(vec_vectorized);
    print_duration_in_msec(start_vectorized, std::cout);

    auto vec_parallel = input;
    const auto start_parallel = std::chrono::steady_clock::now();
    for_each_parallel(vec_parallel);
    print_duration_in_msec(start_parallel, std::cout);

    auto vec_parallel_vectorized = input;
    const auto start_parallel_vectorized = std::chrono::steady_clock::now();
    for_each_parallel_vectorized(vec_parallel_vectorized);
    print_duration_in_msec(start_parallel_vectorized, std::cout);

    ASSERT_EQ(input.size(), vec_sequential.size());
    ASSERT_EQ(input.size(), vec_vectorized.size());
    ASSERT_EQ(input.size(), vec_parallel.size());
    ASSERT_EQ(input.size(), vec_parallel_vectorized.size());

    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(),
                           vec_vectorized.begin(), vec_vectorized.end()));
    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(),
                           vec_parallel.begin(), vec_parallel.end()));
    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(),
                           vec_parallel_vectorized.begin(), vec_parallel_vectorized.end()));
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
Last:
*/
