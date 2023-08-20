#include <algorithm>
#include <chrono>
#include <execution>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include <boost/program_options.hpp>
#include <gtest/gtest.h>

#define OPTION_TRIALS "trial"
#define OPTION_VEC_LEN "size"
#define OPTION_MAX_NUM "max"

namespace {
using Number = int64_t;
using Numbers = std::vector<Number>;
template <typename T, std::size_t S> constexpr std::size_t SizeOfArray(T (&)[S]) {
    return S;
}
} // namespace

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

Numbers generate_numbers(size_t size, Number max_num) {
    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());
    std::uniform_int_distribution<Number> dist(static_cast<Number>(0), max_num);

    Numbers vec(size, 0);
    for (auto&& e : vec) {
        e = dist(engine);
    }
    return vec;
}

void print_duration_in_msec(const std::chrono::steady_clock::time_point& start, std::ostream& os) {
    const auto timestamp = std::chrono::steady_clock::now();
    const auto dur =
        std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - start).count();
    os << dur << "msec\n";
}

struct Setting {
    size_t n_trials {1};
    size_t vec_len {10};
    Number max_num {5};
};

void parse_command_line(int argc, char* argv[], Setting& setting) {
    boost::program_options::options_description description("Options");
    std::string str_strategy;

    description.add_options()
        (OPTION_TRIALS, boost::program_options::value<decltype(setting.n_trials)>(), "# of trials")
        (OPTION_VEC_LEN, boost::program_options::value<decltype(setting.vec_len)>(), "The length of an input vector")
        (OPTION_MAX_NUM, boost::program_options::value<decltype(setting.max_num)>(), "The upper limit of vector elements");

    boost::program_options::variables_map var_map;
    boost::program_options::store(parse_command_line(argc, argv, description), var_map);
    boost::program_options::notify(var_map);

    if (var_map.count(OPTION_TRIALS)) {
        setting.n_trials = var_map[OPTION_TRIALS].as<decltype(setting.n_trials)>();
    }

    if (var_map.count(OPTION_VEC_LEN)) {
        setting.vec_len = var_map[OPTION_VEC_LEN].as<decltype(setting.vec_len)>();
    }

    if (var_map.count(OPTION_MAX_NUM)) {
        setting.max_num = var_map[OPTION_MAX_NUM].as<decltype(setting.max_num)>();
    }
}

void for_each_all_cases(const Setting& setting, Numbers& input,
                        Numbers& vec_sequential, Numbers& vec_vectorized,
                        Numbers& vec_parallel, Numbers& vec_parallel_vectorized) {
    input = generate_numbers(setting.vec_len, setting.max_num);

    vec_sequential = input;
    const auto start_sequential = std::chrono::steady_clock::now();
    for_each_sequential(vec_sequential);
    print_duration_in_msec(start_sequential, std::cout);

    vec_vectorized = input;
    const auto start_vectorized = std::chrono::steady_clock::now();
    for_each_parallel(vec_vectorized);
    print_duration_in_msec(start_vectorized, std::cout);

    vec_parallel = input;
    const auto start_parallel = std::chrono::steady_clock::now();
    for_each_parallel(vec_parallel);
    print_duration_in_msec(start_parallel, std::cout);

    vec_parallel_vectorized = input;
    const auto start_parallel_vectorized = std::chrono::steady_clock::now();
    for_each_parallel_vectorized(vec_parallel_vectorized);
    print_duration_in_msec(start_parallel_vectorized, std::cout);

    return;
}

void for_each_all_cases(const Setting& setting) {
    Numbers input;
    Numbers vec_sequential;
    Numbers vec_vectorized;
    Numbers vec_parallel;
    Numbers vec_parallel_vectorized;

    for_each_all_cases(setting, input, vec_sequential, vec_vectorized,
                       vec_parallel, vec_parallel_vectorized);
    return;
}

void sort_all_cases(const Setting& setting, Numbers& input,
                    Numbers& vec_sequential, Numbers& vec_vectorized,
                    Numbers& vec_parallel, Numbers& vec_parallel_vectorized) {
    input = generate_numbers(setting.vec_len, setting.max_num);

    vec_sequential = input;
    const auto start_sequential = std::chrono::steady_clock::now();
    sort_sequential(vec_sequential);
    print_duration_in_msec(start_sequential, std::cout);

    vec_vectorized = input;
    const auto start_vectorized = std::chrono::steady_clock::now();
    sort_parallel(vec_vectorized);
    print_duration_in_msec(start_vectorized, std::cout);

    vec_parallel = input;
    const auto start_parallel = std::chrono::steady_clock::now();
    sort_parallel(vec_parallel);
    print_duration_in_msec(start_parallel, std::cout);

    vec_parallel_vectorized = input;
    const auto start_parallel_vectorized = std::chrono::steady_clock::now();
    sort_parallel_vectorized(vec_parallel_vectorized);
    print_duration_in_msec(start_parallel_vectorized, std::cout);

    return;
}

void sort_all_cases(const Setting& setting) {
    Numbers input;
    Numbers vec_sequential;
    Numbers vec_vectorized;
    Numbers vec_parallel;
    Numbers vec_parallel_vectorized;

    sort_all_cases(setting, input, vec_sequential, vec_vectorized,
                   vec_parallel, vec_parallel_vectorized);
    return;
}

class TestFunctions : public ::testing::Test {};

TEST_F(TestFunctions, ForEachFixed) {
    const Numbers input{1, 7, 6, 5};
    const Numbers expected{2, 14, 12, 10};

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

    EXPECT_TRUE(
        std::equal(expected.begin(), expected.end(), vec_sequential.begin(), vec_sequential.end()));
    EXPECT_TRUE(
        std::equal(expected.begin(), expected.end(), vec_vectorized.begin(), vec_vectorized.end()));
    EXPECT_TRUE(
        std::equal(expected.begin(), expected.end(), vec_parallel.begin(), vec_parallel.end()));
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), vec_parallel_vectorized.begin(),
                           vec_parallel_vectorized.end()));
}

TEST_F(TestFunctions, ForEachRandom) {
    constexpr Number max_num = std::numeric_limits<Number>::max() / 8;
    const Setting setting {1, 4000000, max_num};
    Numbers input;
    Numbers vec_sequential;
    Numbers vec_vectorized;
    Numbers vec_parallel;
    Numbers vec_parallel_vectorized;
    for_each_all_cases(setting, input, vec_sequential, vec_vectorized,
                       vec_parallel, vec_parallel_vectorized);

    ASSERT_EQ(setting.vec_len, input.size());
    ASSERT_EQ(input.size(), vec_sequential.size());
    ASSERT_EQ(input.size(), vec_vectorized.size());
    ASSERT_EQ(input.size(), vec_parallel.size());
    ASSERT_EQ(input.size(), vec_parallel_vectorized.size());

    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(), vec_vectorized.begin(),
                           vec_vectorized.end()));
    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(), vec_parallel.begin(),
                           vec_parallel.end()));
    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(),
                           vec_parallel_vectorized.begin(), vec_parallel_vectorized.end()));
}

TEST_F(TestFunctions, SortFixed) {
    const Numbers input{1, 7, 6, 2, 3, 4, 5, 3};
    const Numbers expected{1, 2, 3, 3, 4, 5, 6, 7};

    auto vec_sequential = input;
    sort_sequential(vec_sequential);

    auto vec_vectorized = input;
    sort_vectorized(vec_vectorized);

    auto vec_parallel = input;
    sort_parallel(vec_parallel);

    auto vec_parallel_vectorized = input;
    sort_parallel_vectorized(vec_parallel_vectorized);

    ASSERT_EQ(expected.size(), input.size());
    ASSERT_EQ(expected.size(), vec_sequential.size());
    ASSERT_EQ(expected.size(), vec_vectorized.size());
    ASSERT_EQ(expected.size(), vec_parallel.size());
    ASSERT_EQ(expected.size(), vec_parallel_vectorized.size());

    EXPECT_TRUE(
        std::equal(expected.begin(), expected.end(), vec_sequential.begin(), vec_sequential.end()));
    EXPECT_TRUE(
        std::equal(expected.begin(), expected.end(), vec_vectorized.begin(), vec_vectorized.end()));
    EXPECT_TRUE(
        std::equal(expected.begin(), expected.end(), vec_parallel.begin(), vec_parallel.end()));
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), vec_parallel_vectorized.begin(),
                           vec_parallel_vectorized.end()));
}

TEST_F(TestFunctions, SortRandom) {
    constexpr Number max_num = std::numeric_limits<Number>::max() / 8;
    const Setting setting {1, 4000000, max_num};
    Numbers input;
    Numbers vec_sequential;
    Numbers vec_vectorized;
    Numbers vec_parallel;
    Numbers vec_parallel_vectorized;

    sort_all_cases(setting, input,vec_sequential, vec_vectorized,
                   vec_parallel, vec_parallel_vectorized);

    ASSERT_EQ(setting.vec_len, input.size());
    ASSERT_EQ(input.size(), vec_sequential.size());
    ASSERT_EQ(input.size(), vec_vectorized.size());
    ASSERT_EQ(input.size(), vec_parallel.size());
    ASSERT_EQ(input.size(), vec_parallel_vectorized.size());

    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(), vec_vectorized.begin(),
                           vec_vectorized.end()));
    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(), vec_parallel.begin(),
                           vec_parallel.end()));
    EXPECT_TRUE(std::equal(vec_sequential.begin(), vec_sequential.end(),
                           vec_parallel_vectorized.begin(), vec_parallel_vectorized.end()));
}

TEST_F(TestFunctions, EmptyArg) {
    const Setting expected{23, 45, 67};
    Setting setting = expected;

    std::string arg0{"command"};
    char* argv[]{const_cast<char*>(arg0.c_str()), nullptr};
    parse_command_line(1, argv, setting);

    EXPECT_EQ(expected.n_trials, setting.n_trials);
    EXPECT_EQ(expected.vec_len, setting.vec_len);
    EXPECT_EQ(expected.max_num, setting.max_num);
}

TEST_F(TestFunctions, ParseCommandLine) {
    Setting setting{0, 0, 0};
    std::string arg0{"command"};
    std::string arg1{"--"};
    arg1 += OPTION_TRIALS;
    std::string arg2{"123"};
    std::string arg3{"--"};
    arg3 += OPTION_VEC_LEN;
    std::string arg4{"456"};
    std::string arg5{"--"};
    arg5 += OPTION_MAX_NUM;
    std::string arg6{"789"};

    char* argv[]{const_cast<char*>(arg0.c_str()), const_cast<char*>(arg1.c_str()),
                 const_cast<char*>(arg2.c_str()), const_cast<char*>(arg3.c_str()),
                 const_cast<char*>(arg4.c_str()), const_cast<char*>(arg5.c_str()),
                 const_cast<char*>(arg6.c_str()), nullptr};

    parse_command_line(SizeOfArray(argv) - 1, argv, setting);
    EXPECT_EQ(123, setting.n_trials);
    EXPECT_EQ(456, setting.vec_len);
    EXPECT_EQ(789, setting.max_num);
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    } else {
        Setting setting;
        parse_command_line(argc, argv, setting);
        for(decltype(setting.n_trials) trial{0}; trial<setting.n_trials; ++trial) {
            for_each_all_cases(setting);
            sort_all_cases(setting);
        }
    }
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
Last:
*/
