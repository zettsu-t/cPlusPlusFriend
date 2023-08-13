// Race conditions in C++ multi-threading may occur data inconsistencies,
// segmentation faults, or infinite loops. It is known as nasal demons.
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>
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
#define OPTION_STRATEGY "strategy"

namespace {
template<typename T, std::size_t S>
constexpr std::size_t SizeOfArray(T(&)[S]) {
    return S;
}

using Size = size_t;
// A document as a sequence of words
using Doc = std::vector<std::string>;
// Words in a document
using Words = std::vector<std::string>;

// Strategies to avoid race conditions
enum class CountingStrategy {
    RELAXED,  // do nothing
    FETCH_ADD,
    COMPARE_AND_SWAP,
};

struct Setting {
    Size n_threads {1};
    Size n_trials  {1};
    Size dict_size {1};
    Size doc_size  {1};
    CountingStrategy strategy {CountingStrategy::RELAXED};
};

std::ostream& operator<< (std::ostream &os, const Setting& setting) {
    os <<
        setting.n_threads << " threads, " <<
        setting.n_trials << " trials, " <<
        setting.dict_size << " # of dictionary entries, " <<
        setting.doc_size << " # of words in a doc\n";
    return os;
}

class WordCount {
public:
    WordCount (const WordCount&) = delete;
    WordCount& operator=(const WordCount&) = delete;
    virtual ~WordCount() = default;
    virtual void preset_zero(const std::string& s) = 0;
    virtual void increment(const std::string& s) = 0;
    virtual Size get(const std::string& s) const = 0;
    virtual Size total() const = 0;
    virtual Size size() const = 0;
    static std::unique_ptr<WordCount> create(CountingStrategy strategy);

protected:
    WordCount() = default;
};

class WordCountPlain : public WordCount {
public:
    WordCountPlain() {}
    virtual ~WordCountPlain() = default;

    void preset_zero(const std::string& s) override {
        // do nothing
    }

    void increment(const std::string& s) override {
        counts_[s] +=1;
    }

    Size get(const std::string& s) const override {
        auto it = counts_.find(s);
        if (it == counts_.end()) {
            return 0;
        }
        return it->second;
    }

    Size total() const override {
        Size total {0};
        for(const auto& [k, v] : counts_) {
            total += v;
        }
        return total;
    }

    Size size() const override {
        return counts_.size();
    }

    std::map<std::string, Size> counts_;
};

class WordCountFetchAdd : public WordCount {
public:
    WordCountFetchAdd() {}
    virtual ~WordCountFetchAdd() = default;

    void preset_zero(const std::string& s) override {
        return counts_[s].store(0);
    }

    void increment(const std::string& s) override {
        counts_[s].fetch_add(1);
    }

    Size get(const std::string& s) const override {
        auto it = counts_.find(s);
        if (it == counts_.end()) {
            return 0;
        }
        return it->second.load();
    }

    Size total() const override {
        Size total {0};
        for(const auto& [k, v] : counts_) {
            total += v.load();
        }
        return total;
    }

    Size size() const override {
        return counts_.size();
    }

    std::map<std::string, std::atomic<Size>> counts_;
};

class WordCountCas : public WordCount {
public:
    WordCountCas() {}
    virtual ~WordCountCas() = default;

    void preset_zero(const std::string& s) override {
        return counts_[s].store(0);
    }

    void increment(const std::string& s) override {
        Size expected = counts_[s].load();
        Size desired = expected + 1;
        do {
            desired = expected + 1;
        } while(!counts_[s].compare_exchange_weak(expected, desired));
    }

    Size get(const std::string& s) const override {
        auto it = counts_.find(s);
        if (it == counts_.end()) {
            return 0;
        }
        return it->second.load();
    }

    Size total() const override {
        Size total {0};
        for(const auto& [k, v] : counts_) {
            total += v.load();
        }
        return total;
    }

    Size size() const override {
        return counts_.size();
    }

    std::map<std::string, std::atomic<Size>> counts_;
};

std::unique_ptr<WordCount> WordCount::create(CountingStrategy strategy) {
    std::unique_ptr<WordCount> obj;

    switch(strategy) {
    case CountingStrategy::FETCH_ADD:
        obj = std::make_unique<WordCountFetchAdd>();
        break;
    case CountingStrategy::COMPARE_AND_SWAP:
        obj = std::make_unique<WordCountCas>();
        break;
    default:
        obj = std::make_unique<WordCountPlain>();
        break;
    }

    return obj;
}

struct State {
    State() {
        // Grantees word_count is non-NULL
        word_count = WordCount::create(CountingStrategy::RELAXED);
    }

    State (const State&) = delete;
    State& operator=(const State&) = delete;

    // A notification from a producer to consumers
    std::atomic<int> ready {0};
    std::mutex mtx;
    std::condition_variable cond;
    // Count words
    Doc doc;
    std::unique_ptr<WordCount> word_count;
};

// An shared instance
State shared_state;
}

void count_words(const Doc& doc, size_t left, size_t len, std::unique_ptr<WordCount>& word_count) {
    auto it_left = doc.begin() + left;
    auto it_right = it_left + len;
    for(auto it = it_left; (it != doc.end()) && (it != it_right); ++it) {
        const auto& word = *it;
        word_count->increment(word);
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

    std::uniform_int_distribution<int> dist_generate_word(0, n_choice - 1);
    std::set<std::string> word_set;
    std::vector<std::string> words;

    Size size {0};
    while(size < dict_size) {
        std::string s;
        for(size_t j{0}; j<digits; ++j) {
            const char c = dist_generate_word(engine) + 'a';
            s.push_back(c);
        }

        if (word_set.find(s) != word_set.end()) {
            continue;
        }

        word_set.insert(s);
        words.push_back(s);
        ++size;
    }

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

void consumer(size_t left, size_t len) {
    {
        std::unique_lock<std::mutex> l(shared_state.mtx);
        shared_state.cond.wait(l, [&] { return shared_state.ready.load() != 0; });
    }

    count_words(shared_state.doc, left, len, shared_state.word_count);
}

void producer(size_t dict_size, size_t doc_size, CountingStrategy strategy) {
    if (shared_state.ready.load()) {
        std::terminate();
    }

    {
        std::unique_lock<std::mutex> l(shared_state.mtx);
        const auto words = generate_words(dict_size);
        shared_state.doc = generate_doc(words, doc_size);

        for(const auto& word : words) {
            shared_state.word_count->preset_zero(word);
        }

        shared_state.ready.store(1);
    }
    shared_state.cond.notify_all();
}

void parse_command_line(int argc, char* argv[], Setting& setting) {
    boost::program_options::options_description description("Options");
    std::string str_strategy;

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
        (OPTION_STRATEGY,
         boost::program_options::value<decltype(str_strategy)>(),
         "A strategy to avoid race conditions")
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

    if (var_map.count(OPTION_STRATEGY)) {
       str_strategy = var_map[OPTION_STRATEGY].as<decltype(str_strategy)>();
       if ((str_strategy == "") || (str_strategy == "relaxed")) {
           setting.strategy = CountingStrategy::RELAXED;
       } else if (str_strategy == "fetch_add") {
           setting.strategy = CountingStrategy::FETCH_ADD;
       } else if ((str_strategy == "cas") || (str_strategy == "compare_and_swap")) {
           setting.strategy = CountingStrategy::COMPARE_AND_SWAP;
       }
    }
}

Size execute_all(const Setting& setting) {
    {
        std::unique_lock<std::mutex> l(shared_state.mtx);
        shared_state.ready.store(0);
    }

    Doc doc;
    auto word_count = WordCount::create(setting.strategy);
    std::swap(shared_state.doc, doc);
    std::swap(shared_state.word_count, word_count);

    std::vector<std::thread> threads;
    for(decltype(setting.n_threads) i{0}; i<setting.n_threads; ++i) {
        const auto left = setting.doc_size * i;
        threads.emplace_back(consumer, left, setting.doc_size);
    }

    const auto total_len = setting.doc_size * setting.n_threads;
    producer(setting.dict_size, total_len, setting.strategy);

    for(auto& thr: threads) {
        thr.join();
    }

    return shared_state.word_count->total();
}

class TestFunctions : public ::testing::Test {
protected:
    virtual void SetUp() override {
        {
            std::unique_lock<std::mutex> l(shared_state.mtx);
            shared_state.ready.store(0);
        }

        Doc doc;
        auto word_count = WordCount::create(CountingStrategy::RELAXED);
        std::swap(shared_state.doc, doc);
        std::swap(shared_state.word_count, word_count);
    }
};

TEST_F(TestFunctions, GenerateDoc) {
    const Words words {"a", "b", "c"};
    constexpr Size doc_size {10000};
    auto actual = generate_doc(words, doc_size);
    ASSERT_EQ(doc_size, actual.size());

    for(const auto& word : words) {
        auto it = std::find(actual.begin(), actual.end(), word);
        ASSERT_TRUE(it != actual.end());
        EXPECT_TRUE(*it == word);
    }

    std::sort(actual.begin(), actual.end());
    auto it = std::unique(actual.begin(), actual.end());
    actual.erase(it, actual.end());

    ASSERT_EQ(words.size(), actual.size());
    for(const auto& word : actual) {
        auto it = std::find(words.begin(), words.end(), word);
        ASSERT_TRUE(it != words.end());
        EXPECT_TRUE(*it == word);
    }
}

TEST_F(TestFunctions, EmptyArg) {
    const Setting expected {123, 234, 345, 456, CountingStrategy::COMPARE_AND_SWAP};
    Setting setting = expected;

    std::string arg0 {"command"};
    char* argv[] {const_cast<char*>(arg0.c_str()), nullptr};
    parse_command_line(1, argv, setting);

    EXPECT_EQ(expected.n_threads, setting.n_threads);
    EXPECT_EQ(expected.n_trials, setting.n_trials);
    EXPECT_EQ(expected.dict_size, setting.dict_size);
    EXPECT_EQ(expected.doc_size, setting.doc_size);
    EXPECT_EQ(expected.strategy, setting.strategy);
}

TEST_F(TestFunctions, ParseCommandLine) {
    Setting setting {0, 0, 0, 0, CountingStrategy::RELAXED};
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
    arg9 += OPTION_STRATEGY;
    std::string arg10 {"fetch_add"};

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
        const_cast<char*>(arg10.c_str()),
        nullptr
    };

    parse_command_line(SizeOfArray(argv) - 1, argv, setting);
    EXPECT_EQ(12, setting.n_threads);
    EXPECT_EQ(34, setting.n_trials);
    EXPECT_EQ(56, setting.dict_size);
    EXPECT_EQ(78, setting.doc_size);
    EXPECT_EQ(CountingStrategy::FETCH_ADD, setting.strategy);
}

TEST_F(TestFunctions, Strategies) {
    struct Param {
        std::string arg;
        CountingStrategy expected;
    };

    std::vector<Param> params {
        {"", CountingStrategy::RELAXED},
        {"relaxed", CountingStrategy::RELAXED},
        {"fetch_add", CountingStrategy::FETCH_ADD},
        {"cas", CountingStrategy::COMPARE_AND_SWAP},
        {"compare_and_swap", CountingStrategy::COMPARE_AND_SWAP}
    };

    const std::vector<CountingStrategy> initial = {
        CountingStrategy::RELAXED, CountingStrategy::FETCH_ADD, CountingStrategy::COMPARE_AND_SWAP
    };

    for(const auto& init : initial) {
        for(const auto& p : params) {
            std::string arg0 {"command"};
            std::string arg1 {"--"};
            arg1 += OPTION_STRATEGY;
            std::string arg2 {p.arg};

            Setting setting {0, 0, 0, 0, init};
            char* argv[] {
                const_cast<char*>(arg0.c_str()),
                const_cast<char*>(arg1.c_str()),
                const_cast<char*>(arg2.c_str()),
                nullptr
            };

            parse_command_line(SizeOfArray(argv) - 1, argv, setting);
            EXPECT_EQ(0, setting.n_threads);
            EXPECT_EQ(0, setting.n_trials);
            EXPECT_EQ(0, setting.dict_size);
            EXPECT_EQ(0, setting.doc_size);
            EXPECT_EQ(p.expected, setting.strategy);
        }
    }
}

TEST_F(TestFunctions, SingleThread) {
    constexpr decltype(Setting::doc_size) expected {1000000};
    Setting setting {1, 1, 100, expected};
    const auto acutal = execute_all(setting);
    ASSERT_EQ(expected, acutal);
}

class TestStrategies : public ::testing::TestWithParam<CountingStrategy> {
protected:
    virtual void SetUp() override {
        {
            std::unique_lock<std::mutex> l(shared_state.mtx);
            shared_state.ready.store(0);
        }
        Doc doc;
        auto word_count = WordCount::create(GetParam());
        std::swap(shared_state.doc, doc);
        std::swap(shared_state.word_count, word_count);
    }
};

TEST_P(TestStrategies, CreateWordCount) {
    switch(GetParam()) {
    case CountingStrategy::RELAXED:
        EXPECT_TRUE(dynamic_cast<WordCountPlain*>(shared_state.word_count.get()));
        break;
    case CountingStrategy::FETCH_ADD:
        EXPECT_TRUE(dynamic_cast<WordCountFetchAdd*>(shared_state.word_count.get()));
        break;
    case CountingStrategy::COMPARE_AND_SWAP:
        EXPECT_TRUE(dynamic_cast<WordCountCas*>(shared_state.word_count.get()));
        break;
    default:
        EXPECT_TRUE(dynamic_cast<WordCountPlain*>(shared_state.word_count.get()));
        break;
    }
}

TEST_P(TestStrategies, WordCountMethods) {
    auto word_count = WordCount::create(GetParam());
    switch(GetParam()) {
    case CountingStrategy::RELAXED:
        ASSERT_TRUE(dynamic_cast<WordCountPlain*>(word_count.get()));
        break;
    case CountingStrategy::FETCH_ADD:
        ASSERT_TRUE(dynamic_cast<WordCountFetchAdd*>(word_count.get()));
        break;
    case CountingStrategy::COMPARE_AND_SWAP:
        ASSERT_TRUE(dynamic_cast<WordCountCas*>(word_count.get()));
        break;
    default:
        ASSERT_TRUE(dynamic_cast<WordCountPlain*>(word_count.get()));
        break;
    }

    const std::string key1 ("key1");
    const std::string key2 ("2key");
    EXPECT_FALSE(word_count->get(key1));

    word_count->increment(key1);
    EXPECT_EQ(1, word_count->get(key1));
    EXPECT_EQ(0, word_count->get(key2));
    EXPECT_EQ(1, word_count->total());
    EXPECT_EQ(1, word_count->size());

    word_count->increment(key1);
    EXPECT_EQ(2, word_count->get(key1));
    EXPECT_EQ(0, word_count->get(key2));
    EXPECT_EQ(2, word_count->total());
    EXPECT_EQ(1, word_count->size());

    word_count->increment(key2);
    EXPECT_EQ(2, word_count->get(key1));
    EXPECT_EQ(1, word_count->get(key2));
    EXPECT_EQ(3, word_count->total());
    EXPECT_EQ(2, word_count->size());

    word_count->preset_zero(key1);
    if (GetParam() == CountingStrategy::RELAXED) {
        EXPECT_EQ(2, word_count->get(key1));
        EXPECT_EQ(3, word_count->total());
    } else {
        EXPECT_EQ(0, word_count->get(key1));
        EXPECT_EQ(1, word_count->total());
    }
    EXPECT_EQ(1, word_count->get(key2));
    EXPECT_EQ(2, word_count->size());
}

TEST_P(TestStrategies, WordCountPresetZero) {
    auto word_count = WordCount::create(GetParam());
    const std::string key ("key");
    EXPECT_FALSE(word_count->get(key));

    word_count->preset_zero(key);
    EXPECT_EQ(0, word_count->get(key));
    EXPECT_EQ(0, word_count->total());
    if (GetParam() == CountingStrategy::RELAXED) {
        EXPECT_EQ(0, word_count->size());
    } else {
        EXPECT_EQ(1, word_count->size());
    }
}

TEST_P(TestStrategies, CountWords) {
    const Doc doc {"a", "b", "b", "c", "c", "c"};
    auto counts_full = WordCount::create(GetParam());
    count_words(doc, 0, 6, counts_full);
    EXPECT_EQ(1, counts_full->get("a"));
    EXPECT_EQ(2, counts_full->get("b"));
    EXPECT_EQ(3, counts_full->get("c"));

    auto counts_left = WordCount::create(GetParam());
    count_words(doc, 0, 3, counts_left);
    EXPECT_EQ(1, counts_left->get("a"));
    EXPECT_EQ(2, counts_left->get("b"));
    EXPECT_EQ(0, counts_left->get("c"));

    auto counts_right = WordCount::create(GetParam());
    count_words(doc, 3, 3, counts_right);
    EXPECT_EQ(0, counts_right->get("a"));
    EXPECT_EQ(0, counts_right->get("b"));
    EXPECT_EQ(3, counts_right->get("c"));
}

TEST_P(TestStrategies, GenerateWords) {
    const std::vector<Size> dict_size_set {1, 26, 100, 676};
    const std::vector<Size> expected_str_len {3, 3, 4, 4};
    for(size_t i{0}; i<dict_size_set.size(); ++i) {
        const auto dict_size = dict_size_set.at(i);
        const auto str_len = expected_str_len.at(i);

        const auto actual = generate_words(dict_size);
        ASSERT_EQ(dict_size, actual.size());

        std::set<std::string> word_set;
        for(const auto& key : actual) {
            EXPECT_TRUE(word_set.find(key) == word_set.end());
            word_set.insert(key);

            EXPECT_EQ(str_len, key.size());
            for(const auto& c : key) {
                EXPECT_TRUE((c >= 'a') && (c <= 'z'));
            }
        }
    }
}

TEST_P(TestStrategies, ConsumerFull) {
    Doc doc {"a", "b", "c", "a", "b", "b", "c", "c", "c"};
    std::swap(shared_state.doc, doc);

    {
        std::unique_lock<std::mutex> l(shared_state.mtx);
        shared_state.ready.store(1);
    }

    consumer(0, 9);
    EXPECT_EQ(2, shared_state.word_count->get("a"));
    EXPECT_EQ(3, shared_state.word_count->get("b"));
    EXPECT_EQ(4, shared_state.word_count->get("c"));
}

TEST_P(TestStrategies, ConsumerPartial) {
    Doc doc {"a", "a", "b", "b", "b"};
    std::swap(shared_state.doc, doc);

    {
        std::unique_lock<std::mutex> l(shared_state.mtx);
        shared_state.ready.store(1);
    }

    consumer(2, 3);
    EXPECT_EQ(0, shared_state.word_count->get("a"));
    EXPECT_EQ(3, shared_state.word_count->get("b"));

    consumer(0, 2);
    EXPECT_EQ(2, shared_state.word_count->get("a"));
    EXPECT_EQ(3, shared_state.word_count->get("b"));
}

TEST_P(TestStrategies, ProducerPrepare) {
    constexpr decltype(Setting::doc_size) dict_size{20};
    constexpr decltype(Setting::doc_size) doc_size{3000};
    producer(dict_size, doc_size, GetParam());

    ASSERT_TRUE(shared_state.ready.load());
    EXPECT_EQ(doc_size, shared_state.doc.size());
    const Size expected = (GetParam() == CountingStrategy::RELAXED) ? 0 : dict_size;
    EXPECT_EQ(expected, shared_state.word_count->size());
}

TEST_P(TestStrategies, MultiThread) {
    constexpr decltype(Setting::n_threads) n_threads {4};
    constexpr decltype(Setting::doc_size) doc_size{1000000};
    constexpr auto expected = n_threads * doc_size;

    const auto strategy = GetParam();
    Setting setting {n_threads, 1, 2, doc_size, strategy};

    const auto acutal = execute_all(setting);
    if (strategy == CountingStrategy::RELAXED) {
        ASSERT_NE(expected, acutal);
        ASSERT_GT(expected, acutal);
    } else {
        ASSERT_EQ(expected, acutal);
    }
}

INSTANTIATE_TEST_CASE_P(AllStrategies, TestStrategies,
                        ::testing::Values(CountingStrategy::RELAXED,
                                          CountingStrategy::FETCH_ADD,
                                          CountingStrategy::COMPARE_AND_SWAP));

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
