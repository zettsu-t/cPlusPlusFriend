#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
#include <boost/core/demangle.hpp>
#include <boost/program_options.hpp>
#include <gtest/gtest.h>

// How to pass array elements to threads
#define USE_C_ARRAY 1
#define USE_STD_ARRAY 1

namespace {
    template <typename T, std::size_t S> constexpr std::size_t SizeOfArray(T (&)[S]) {
        return S;
    }
}

// Constants and type aliases
constexpr unsigned int MIN_HARDWARE_CONCURRENCY {1};
constexpr unsigned int MAX_HARDWARE_CONCURRENCY {128};
using LoopCount = size_t;
using RandomNumber = uint64_t;

// Command line option(s) and runtime parameters
#define OPTION_REF "ref"

enum class PassRefToThread {
    Ref,
    ArrayElement,
};

struct Setting {
    PassRefToThread pass_ref_to_thread {PassRefToThread::Ref};
    LoopCount loop_count {1000000};
};

// Results of worker threads
struct Result {
    LoopCount count {0};
    RandomNumber sum {0};
};

constexpr RandomNumber LOWER_RAND = 500000000ULL;
constexpr RandomNumber UPPER_RAND = 1000000000ULL;

// Returns true if it found invalid options, false otherwise
bool parse_command_line(int argc, char* argv[], Setting& setting) {
    boost::program_options::options_description description("Options");
    std::string str_ref;
    description.add_options()(
        OPTION_REF,
        boost::program_options::value<decltype(str_ref)>(), "How to pass refs to threads");

    boost::program_options::variables_map var_map;
    boost::program_options::store(parse_command_line(argc, argv, description), var_map);
    boost::program_options::notify(var_map);

    bool invalid {false};
    if (var_map.count(OPTION_REF)) {
        str_ref = var_map[OPTION_REF].as<decltype(str_ref)>();
        if (str_ref == "ref") {
            setting.pass_ref_to_thread = PassRefToThread::Ref;
        } else if (str_ref == "array") {
            setting.pass_ref_to_thread = PassRefToThread::ArrayElement;
        } else {
            invalid = true;
        }
    }

    return invalid;
}

class TestParseCommandLine : public ::testing::Test {};

TEST_F(TestParseCommandLine, Ref) {
    std::vector<std::tuple<std::string, PassRefToThread, bool>> targets{
        {"ref", PassRefToThread::Ref, false},
        {"array", PassRefToThread::ArrayElement, false},
        {"none", PassRefToThread::Ref, true}
    };

    for (const auto& [arg, value, invalid] : targets) {
        Setting setting{PassRefToThread::Ref};

        std::string arg0{"command"};
        std::string arg1{"--"};
        arg1 += OPTION_REF;
        std::string arg2 = arg;

        char* argv[]{const_cast<char*>(arg0.c_str()), const_cast<char*>(arg1.c_str()),
                     const_cast<char*>(arg2.c_str()), nullptr};

        const bool actual = parse_command_line(SizeOfArray(argv) - 1, argv, setting);
        EXPECT_EQ(value, setting.pass_ref_to_thread);
        EXPECT_EQ(invalid, actual);
    }
}

// Runs in worker threads
void do_in_thread(LoopCount loop, Result& result) {
    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());
    std::uniform_int_distribution<RandomNumber> dist(LOWER_RAND, UPPER_RAND);

    for(decltype(loop) i{0}; i<loop; ++i) {
        ++result.count;
        result.sum += dist(engine);
    }

    return;
}

class TestDoInThread : public ::testing::Test {};

TEST_F(TestDoInThread, All) {
    constexpr LoopCount loop = 200000;
    constexpr LoopCount n_trials = 10;

    std::set<RandomNumber> nums;
    for(LoopCount trial{0}; trial < n_trials; ++trial) {
        Result result;
        do_in_thread(loop, result);
        ASSERT_EQ(loop, result.count);

        const auto actual = result.sum;
        ASSERT_LT(LOWER_RAND * loop, actual);
        ASSERT_GT(UPPER_RAND * loop, actual);
        nums.insert(actual);
    }

    EXPECT_EQ(n_trials, nums.size());
}

// Runs in the main thread
std::vector<Result> dispatch(const Setting& setting) {
    auto n_threads = std::clamp(std::thread::hardware_concurrency(),
        MIN_HARDWARE_CONCURRENCY, MAX_HARDWARE_CONCURRENCY);

#if defined(USE_C_ARRAY)
    Result results[MAX_HARDWARE_CONCURRENCY];
#elif defined(USE_STD_ARRAY)
    std::array<Result, MAX_HARDWARE_CONCURRENCY> results;
#else
    std::vector<Result> results;
#endif

    std::vector<std::thread> threads;
    threads.reserve(n_threads);

    for(decltype(n_threads) i{0}; i<n_threads; ++i) {
        if (setting.pass_ref_to_thread == PassRefToThread::Ref) {
            auto& result = results[i];
            if (i == 0) {
                char const* name = typeid(result).name();
                std::cout << "typename: " << boost::core::demangle(name) << std::endl;
            }

            threads.emplace_back(std::thread([&]{
                do_in_thread(setting.loop_count, std::ref(result));
            }));
        } else {
            if (i == 0) {
                char const* name = typeid(results[0]).name();
                std::cout << "typename: " << boost::core::demangle(name) << std::endl;
            }

            threads.emplace_back(std::thread([&]{
                do_in_thread(setting.loop_count, std::ref(results[i]));
            }));
        }
    }

    for(decltype(n_threads) i{0}; i<n_threads; ++i) {
        threads.at(i).join();
    }

    std::vector<Result> ret_val;
    for(decltype(n_threads) i{0}; i<n_threads; ++i) {
        ret_val.push_back(results[i]);
    }

    return ret_val;
}

class TestDispatch : public ::testing::Test {};

TEST_F(TestDispatch, Ref) {
    constexpr LoopCount loop_count {1000000};
    Setting setting {PassRefToThread::Ref, loop_count};
    const auto results = dispatch(setting);

    std::set<RandomNumber> nums;
    for(const auto& result : results) {
        ASSERT_EQ(loop_count, result.count);
        nums.insert(result.sum);
    }
    EXPECT_EQ(results.size(), nums.size());
}

TEST_F(TestDispatch, ArrayElement) {
    constexpr LoopCount loop_count {1000000};
    Setting setting {PassRefToThread::ArrayElement, loop_count};
    const auto results = dispatch(setting);

    std::set<RandomNumber> nums;
    for(const auto& result : results) {
        ASSERT_EQ(loop_count, result.count);
        nums.insert(result.sum);
    }
    EXPECT_LT(results.size(), nums.size());
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    } else {
        Setting setting;
        parse_command_line(argc, argv, setting);
        const auto results = dispatch(setting);

        for(const auto& result : results) {
            std::cout << result.count << "," << result.sum << "\n";
        }
    }
    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
Last:
*/
