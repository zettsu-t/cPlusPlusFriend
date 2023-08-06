// マルチスレッドの競合は、一貫性のない変更、SEGV、無限ループなど
// あらゆる未定義動作が起きることを試す
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <map>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>
#include <gtest/gtest.h>

// Command line options
#define OPTION_THREADS "threads"
#define OPTION_TRIALS "trials"
#define OPTION_DICT_SIZE "dict_size"
#define OPTION_DOC_SIZE "doc_size"
#define OPTION_FIX_DICT "fix_dict"

namespace {
using Size = size_t;
using WordId = long long int;

// Document as sequence of words
using Doc = std::vector<std::string>;
using Words = std::vector<std::string>;
// Words to counts
using WordCount = std::map<std::string, Size>;
// Words to IDs
using WordToId = std::map<std::string, WordId>;
// Fixed-entries word IDs to counts
using IdToCount = std::vector<Size>;

struct Setting {
    Size n_threads {1};
    Size n_trials  {1};
    Size dict_size {1};
    Size doc_size  {1};
    bool fixed_dict {false};
};

std::ostream& operator<< (std::ostream &os, const Setting& setting) {
    os <<
        setting.n_threads << " threads, " <<
        setting.n_trials << " trials, " <<
        setting.dict_size << " # of dictionary entries, " <<
        setting.doc_size << " # of words in a doc\n";
    return os;
}

struct State {
    // Notification from a producer to consumers
    std::atomic<int> ready {0};
    std::mutex mtx;
    std::condition_variable cond;
    // Count words
    Doc       doc;
    WordCount word_count;
    // Count word IDs
    WordToId  word_to_id;
    IdToCount id_count;
};

// An shared instance
State shared_state;
}

void count_words(const Doc& doc, size_t left, size_t len, WordCount& counts) {
    auto it_left = doc.begin() + left;
    auto it_right = it_left + len;
    for(auto it = it_left; (it != doc.end()) && (it != it_right); ++it) {
        const auto& word = *it;
        counts[word] += 1;
    }
    return;
}

void count_word_ids(const Doc& doc, size_t left, size_t len, IdToCount& counts) {
    auto it_left = doc.begin() + left;
    auto it_right = it_left + len;
    for(auto it = it_left; (it != doc.end()) && (it != it_right); ++it) {
        const auto& word = *it;
        const auto& id = shared_state.word_to_id.at(*it);
        counts[id] += 1;
    }
    return;
}

Words generate_words(size_t dict_size) {
    constexpr Size n_choice = ('z' - 'a') + 1;
    Size n {1};
    Size digits {2};
    do {
        n *= n_choice;
        ++digits;
    } while(n < dict_size);

    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());

    WordId seq_id {0};
    std::uniform_int_distribution<int> dist_generate_word(0, n_choice - 1);
    std::vector<std::string> words;
    while(seq_id < dict_size) {
        std::string s;
        for(size_t j{0}; j<digits; ++j) {
            const char c = dist_generate_word(engine) + 'a';
            s.push_back(c);
        }

        if (shared_state.word_to_id.find(s) != shared_state.word_to_id.end()) {
            continue;
        }

        shared_state.word_to_id[s] = seq_id;
        words.push_back(s);
        ++seq_id;
    }

    shared_state.id_count.resize(seq_id);
    return words;
}

Doc generate_doc(const Words& words, Size doc_size) {
    Doc doc;
    doc.reserve(doc_size);

    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());
    const auto dict_size = words.size();
    std::uniform_int_distribution<Size> dist_choose_word(0, dict_size - 1);
    for(size_t i{0}; i<doc_size; ++i) {
        const auto& s = words.at(dist_choose_word(engine));
        doc.push_back(s);
    }

    return doc;
}

void consumer(size_t left, size_t len, bool fixed_dict) {
    {
        std::unique_lock<std::mutex> l(shared_state.mtx);
        shared_state.cond.wait(l, [&] { return shared_state.ready.load() != 0; });
    }

    if (fixed_dict) {
        count_word_ids(shared_state.doc, left, len, shared_state.id_count);
    } else {
        count_words(shared_state.doc, left, len, shared_state.word_count);
    }
}

void producer(size_t dict_size, size_t doc_size) {
    if (shared_state.ready.load()) {
        std::terminate();
    }

    {
        std::unique_lock<std::mutex> l(shared_state.mtx);
        const auto words = generate_words(dict_size);
        shared_state.doc = generate_doc(words, doc_size);
        shared_state.ready.store(1);
    }
    shared_state.cond.notify_all();
}

void parse_command_line(int argc, char* argv[], Setting& setting) {
    boost::program_options::options_description description("Options");
    description.add_options()
        (OPTION_THREADS,
         boost::program_options::value<decltype(setting.n_threads)>(),
         "Number of trials")
        (OPTION_TRIALS,
         boost::program_options::value<decltype(setting.n_trials)>(),
         "Number of trials")
        (OPTION_DICT_SIZE,
         boost::program_options::value<decltype(setting.dict_size)>(),
         "Size of a dictionary")
        (OPTION_DOC_SIZE,
         boost::program_options::value<decltype(setting.doc_size)>(),
         "Length of a document")
        (OPTION_FIX_DICT,
         "Assing word ids")
        ;

    boost::program_options::variables_map var_map;
    boost::program_options::store(parse_command_line(argc, argv, description), var_map);
    boost::program_options::notify(var_map);

    if (var_map.count(OPTION_THREADS)) {
        setting.n_threads = var_map[OPTION_THREADS].as<decltype(setting.n_threads)>();
    }

    if (var_map.count(OPTION_TRIALS)) {
        setting.n_trials = var_map[OPTION_TRIALS].as<decltype(setting.n_trials)>();
    }

    if (var_map.count(OPTION_DICT_SIZE)) {
        setting.dict_size = var_map[OPTION_DICT_SIZE].as<decltype(setting.dict_size)>();
    }

    if (var_map.count(OPTION_DOC_SIZE)) {
        setting.doc_size = var_map[OPTION_DOC_SIZE].as<decltype(setting.doc_size)>();
    }

    if (var_map.count(OPTION_FIX_DICT)) {
        setting.fixed_dict = true;
    }
}

Size execute_all(const Setting& setting) {
    shared_state.ready.store(0);
    Doc doc;
    WordCount word_count;
    WordToId  word_to_id;
    IdToCount id_count;
    std::swap(shared_state.doc, doc);
    std::swap(shared_state.word_count, word_count);
    std::swap(shared_state.word_to_id, word_to_id);
    std::swap(shared_state.id_count, id_count);

    std::vector<std::thread> threads;
    for(decltype(setting.n_threads) i{0}; i<setting.n_threads; ++i) {
        const auto left = setting.doc_size * i;
        threads.emplace_back(consumer, left, setting.doc_size, setting.fixed_dict);
    }

    const auto total_len = setting.doc_size * setting.n_threads;
    producer(setting.dict_size, total_len);

    for(auto& thr: threads) {
        thr.join();
    }

    Size total {0};
    for(const auto& [k, v] : shared_state.word_count) {
        total += v;
    }
    return total;
}

class TestAll : public ::testing::Test {
    virtual void SetUp() override {
        shared_state.ready.store(0);
        Doc doc;
        WordCount word_count;
        WordToId  word_to_id;
        IdToCount id_count;
        std::swap(shared_state.doc, doc);
        std::swap(shared_state.word_count, word_count);
        std::swap(shared_state.word_to_id, word_to_id);
        std::swap(shared_state.id_count, id_count);
    }
};

TEST_F(TestAll, CountWords) {
    const Doc doc {"a", "b", "b", "c", "c", "c"};
    WordCount counts_full;
    count_words(doc, 0, 6, counts_full);
    EXPECT_EQ(1, counts_full["a"]);
    EXPECT_EQ(2, counts_full["b"]);
    EXPECT_EQ(3, counts_full["c"]);

    WordCount counts_left;
    count_words(doc, 0, 3, counts_left);
    EXPECT_EQ(1, counts_left["a"]);
    EXPECT_EQ(2, counts_left["b"]);
    EXPECT_EQ(0, counts_left["c"]);

    WordCount counts_right;
    count_words(doc, 3, 3, counts_right);
    EXPECT_EQ(0, counts_right["a"]);
    EXPECT_EQ(0, counts_right["b"]);
    EXPECT_EQ(3, counts_right["c"]);
}

TEST_F(TestAll, CountWordIds) {
    const Doc doc {"a", "b", "b", "c", "c", "c"};
    WordToId word_to_id;
    word_to_id["a"] = 4;
    word_to_id["b"] = 5;
    word_to_id["c"] = 6;
    std::swap(shared_state.word_to_id, word_to_id);

    IdToCount counts_full(7, 0);
    count_word_ids(doc, 0, 6, counts_full);
    EXPECT_EQ(1, counts_full.at(4));
    EXPECT_EQ(2, counts_full.at(5));
    EXPECT_EQ(3, counts_full.at(6));

    IdToCount counts_left(7, 0);
    count_word_ids(doc, 0, 3, counts_left);
    EXPECT_EQ(1, counts_left.at(4));
    EXPECT_EQ(2, counts_left.at(5));
    EXPECT_EQ(0, counts_left.at(6));

    IdToCount counts_right(7, 0);
    count_word_ids(doc, 3, 3, counts_right);
    EXPECT_EQ(0, counts_right.at(4));
    EXPECT_EQ(0, counts_right.at(5));
    EXPECT_EQ(3, counts_right.at(6));
}

TEST_F(TestAll, GenerateWords) {
    const std::vector<Size> dict_size_set {1, 26, 100, 676};
    const std::vector<Size> expected_str_len {3, 3, 4, 4};
    for(size_t i{0}; i<dict_size_set.size(); ++i) {
        const auto dict_size = dict_size_set.at(i);
        const auto str_len = expected_str_len.at(i);

        IdToCount id_count;
        std::swap(shared_state.id_count, id_count);
        const auto actual = generate_words(dict_size);
        ASSERT_EQ(dict_size, actual.size());
        EXPECT_EQ(dict_size, shared_state.id_count.size());

        for(const auto& key : actual) {
            EXPECT_EQ(str_len, key.size());
            for(const auto& c : key) {
                EXPECT_TRUE((c >= 'a') && (c <= 'z'));
            }
        }
    }
}

TEST_F(TestAll, GenerateDoc) {
    const Words words {"a", "b", "c"};
    constexpr Size doc_size {10000};
    const auto actual = generate_doc(words, doc_size);
    ASSERT_EQ(doc_size, actual.size());

    for(const auto& word : words) {
        EXPECT_TRUE(std::find(actual.begin(), actual.end(), word) != actual.end());
    }
}

TEST_F(TestAll, ConsumerFull) {
    Doc doc {"a", "b", "c", "a", "b", "b", "c", "c", "c"};
    WordToId  word_to_id;
    word_to_id["c"] = 0;
    word_to_id["b"] = 1;
    word_to_id["a"] = 2;
    IdToCount id_count(3, 0);

    std::swap(shared_state.doc, doc);
    std::swap(shared_state.word_to_id, word_to_id);
    std::swap(shared_state.id_count, id_count);

    {
        std::unique_lock<std::mutex> l(shared_state.mtx);
        shared_state.ready.store(1);
    }

    consumer(0, 9, false);
    EXPECT_EQ(2, shared_state.word_count["a"]);
    EXPECT_EQ(3, shared_state.word_count["b"]);
    EXPECT_EQ(4, shared_state.word_count["c"]);
    EXPECT_EQ(0, shared_state.id_count[0]);
    EXPECT_EQ(0, shared_state.id_count[1]);
    EXPECT_EQ(0, shared_state.id_count[2]);

    consumer(0, 9, true);
    EXPECT_EQ(2, shared_state.id_count[2]);
    EXPECT_EQ(3, shared_state.id_count[1]);
    EXPECT_EQ(4, shared_state.id_count[0]);
}

TEST_F(TestAll, ConsumerPartial) {
    Doc doc {"a", "a", "b", "b", "b"};
    WordToId  word_to_id;
    word_to_id["a"] = 1;
    word_to_id["b"] = 0;
    IdToCount id_count(2, 0);

    std::swap(shared_state.doc, doc);
    std::swap(shared_state.word_to_id, word_to_id);
    std::swap(shared_state.id_count, id_count);

    {
        std::unique_lock<std::mutex> l(shared_state.mtx);
        shared_state.ready.store(1);
    }

    consumer(2, 3, false);
    EXPECT_EQ(0, shared_state.word_count["a"]);
    EXPECT_EQ(3, shared_state.word_count["b"]);
    EXPECT_EQ(0, shared_state.id_count[0]);
    EXPECT_EQ(0, shared_state.id_count[1]);

    consumer(2, 3, true);
    EXPECT_EQ(3, shared_state.id_count[0]);
    EXPECT_EQ(0, shared_state.id_count[1]);

    consumer(0, 2, false);
    EXPECT_EQ(2, shared_state.word_count["a"]);
    EXPECT_EQ(3, shared_state.word_count["b"]);
    EXPECT_EQ(3, shared_state.id_count[0]);
    EXPECT_EQ(0, shared_state.id_count[1]);

    consumer(0, 2, true);
    EXPECT_EQ(3, shared_state.id_count[0]);
    EXPECT_EQ(2, shared_state.id_count[1]);
}

TEST_F(TestAll, Producer) {
    constexpr decltype(Setting::doc_size) dict_size{20};
    constexpr decltype(Setting::doc_size) doc_size{3000};
    producer(dict_size, doc_size);

    ASSERT_TRUE(shared_state.ready.load());
    EXPECT_EQ(doc_size, shared_state.doc.size());
    EXPECT_EQ(dict_size, shared_state.word_to_id.size());
    EXPECT_EQ(dict_size, shared_state.id_count.size());
    EXPECT_TRUE(std::all_of(
                    shared_state.id_count.begin(), shared_state.id_count.end(),
                    [&](const auto& e) { return e == 0; }));
}

TEST_F(TestAll, EmptyArg) {
    const Setting expected {123, 234, 345, 456, false};
    Setting setting = expected;

    std::string arg0 {"command"};
    char* argv[] {const_cast<char*>(arg0.c_str()), nullptr};
    parse_command_line(1, argv, setting);

    EXPECT_EQ(expected.n_threads, setting.n_threads);
    EXPECT_EQ(expected.n_trials, setting.n_trials);
    EXPECT_EQ(expected.dict_size, setting.dict_size);
    EXPECT_EQ(expected.doc_size, setting.doc_size);
    EXPECT_FALSE(setting.fixed_dict);
}

TEST_F(TestAll, ParseCommandLine) {
    Setting setting {0, 0, 0, 0};
    std::string arg0 {"command"};
    std::string arg1 {"--"};
    arg1 += OPTION_THREADS;
    std::string arg2 {"12"};
    std::string arg3 {"--"};
    arg3 += OPTION_TRIALS;
    std::string arg4 {"34"};
    std::string arg5 {"--"};
    arg5 += OPTION_DICT_SIZE;
    std::string arg6 {"56"};
    std::string arg7 {"--"};
    arg7 += OPTION_DOC_SIZE;
    std::string arg8 {"78"};
    std::string arg9 {"--"};
    arg9 += OPTION_FIX_DICT;

    char* argv[] {
        const_cast<char*>(arg0.c_str()),
        const_cast<char*>(arg1.c_str()),
        const_cast<char*>(arg2.c_str()),
        const_cast<char*>(arg3.c_str()),
        const_cast<char*>(arg4.c_str()),
        const_cast<char*>(arg5.c_str()),
        const_cast<char*>(arg6.c_str()),
        const_cast<char*>(arg7.c_str()),
        const_cast<char*>(arg8.c_str()),
        const_cast<char*>(arg9.c_str()),
        nullptr
    };

    parse_command_line(10, argv, setting);
    EXPECT_EQ(12, setting.n_threads);
    EXPECT_EQ(34, setting.n_trials);
    EXPECT_EQ(56, setting.dict_size);
    EXPECT_EQ(78, setting.doc_size);
    EXPECT_TRUE(setting.fixed_dict);
}

TEST_F(TestAll, SingleThread) {
    constexpr decltype(Setting::doc_size) expected {1000000};
    Setting setting {1, 1, 100, expected};
    const auto acutal = execute_all(setting);
    ASSERT_EQ(expected, acutal);
}

TEST_F(TestAll, MultiThread) {
    constexpr decltype(Setting::n_threads) n_threads {4};
    constexpr decltype(Setting::doc_size) doc_size{1000000};
    constexpr auto expected = n_threads * doc_size;
    Setting setting {n_threads, 1, 2, doc_size};

    const auto acutal = execute_all(setting);
    ASSERT_NE(expected, acutal);
    ASSERT_GT(expected, acutal);
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    Setting setting;
    parse_command_line(argc, argv, setting);
    std::cout << setting;

    for(size_t trial{0}; trial < setting.n_trials; ++trial) {
        std::cout << execute_all(setting) << std::endl;
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
