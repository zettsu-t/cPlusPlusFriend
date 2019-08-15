// Accumulate even integers
#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <vector>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <gtest/gtest.h>

namespace {
    // Expect RangePtr is a std::shared_ptr<> type
    template <typename Value, typename RangePtr>
    class MyRangeIterator : public boost::iterator_facade<
        MyRangeIterator<Value, RangePtr>,
        Value, boost::forward_traversal_tag, Value> {
    public:
        MyRangeIterator(void);
        explicit MyRangeIterator(RangePtr& pRange);
    private:
        friend class boost::iterator_core_access;
        void increment(void);
        Value dereference(void) const;
        bool equal(const MyRangeIterator& other) const;
        void update(void);
        RangePtr pRange_;
        Value current_ {0};
        Value value_ {RangePtr::element_type::OutOfBound};
    };

    template <typename T>
    class MyRange {
    public:
        using Value = T;
        using Iterator = MyRangeIterator<T, MyRange>;
        static const T OutOfBound;

        MyRange(const T& limit) : limit_(limit) {
            assert((OutOfBound == 0) || (limit < OutOfBound));
        }

        MyRange(void) {
            return;
        }

        const T& GetLimit(void) const {
            return limit_;
        }

        T GetValue(const T& current) const {
            if (current <= 0) {
                return OutOfBound;
            }

            const auto value = limit_ - current;
            const auto result = (value % Value{2}) ? Value{0} : value;
            return result;
        }

    private:
        const T limit_ {0};
    };

    template <typename T>
    const T MyRange<T>::OutOfBound = std::numeric_limits<T>::max();

    template <typename Value, typename RangePtr>
    MyRangeIterator<Value, RangePtr>::MyRangeIterator(void) {
        update();
        return;
    }

    template <typename Value, typename RangePtr>
    MyRangeIterator<Value, RangePtr>::MyRangeIterator(RangePtr& pRange) :
        pRange_(pRange), current_(pRange->GetLimit()),
        value_(pRange_->GetValue(current_)) {
        update();
        return;
    }

    template <typename Value, typename RangePtr>
    void MyRangeIterator<Value, RangePtr>::increment(void) {
        if (current_ > 0) {
            --current_;
        }
        update();
        return;
    }

    template <typename Value, typename RangePtr>
    Value MyRangeIterator<Value, RangePtr>::dereference(void) const {
        // Cannot return member variables as Value& from const member functions
        return value_;
    }

    template <typename Value, typename RangePtr>
    bool MyRangeIterator<Value, RangePtr>::equal(const MyRangeIterator& other) const {
        return (!pRange_ && !other.pRange_) || (pRange_ && other.pRange_ && (current_ == other.current_));
    }

    template <typename Value, typename RangePtr>
    void MyRangeIterator<Value, RangePtr>::update(void) {
        if (current_ <= 0) {
            pRange_.reset();
        }

        value_ = (pRange_) ? pRange_->GetValue(current_) : RangePtr::element_type::OutOfBound;
        return;
    }
}

template <typename RangePtr>
auto begin(RangePtr& pRange) {
    return MyRangeIterator<typename RangePtr::element_type::Value, RangePtr>(pRange);
}

template <typename RangePtr>
auto end(RangePtr& pRange) {
    return MyRangeIterator<typename RangePtr::element_type::Value, RangePtr>();
}

class TestRangeStream : public ::testing::Test {};

TEST_F(TestRangeStream, RangeInt) {
    using Value = int;
    const std::vector<Value> testedSet {1,2,3,6,8,9};
    for(const auto& limit : testedSet) {
        MyRange<Value> range(limit);

        for (Value i=1; i<limit; ++i) {
            EXPECT_EQ(limit, range.GetLimit());
            const auto diff = limit - i;
            const auto expected = ((diff % 2) == 0) ? diff : 0;
            EXPECT_EQ(expected, range.GetValue(i));
        }

        const auto expected = std::numeric_limits<Value>::max();
        EXPECT_EQ(expected, range.GetValue(0));
        EXPECT_EQ(expected, range.GetValue(-1));
    }
}

TEST_F(TestRangeStream, RangeEmpty) {
    using Value = int;
    MyRange<Value> rangeZero(0);
    EXPECT_FALSE(rangeZero.GetLimit());
    EXPECT_EQ(std::numeric_limits<Value>::max(), rangeZero.GetValue(0));

    MyRange<Value> rangeNegative(-1);
    EXPECT_EQ(-1, rangeNegative.GetLimit());
    EXPECT_EQ(std::numeric_limits<Value>::max(), rangeNegative.GetValue(0));
}

TEST_F(TestRangeStream, RangeBigInt) {
    using Value = boost::multiprecision::cpp_int;
    std::vector<Value> testedSet {1,15,16};
    for(const auto& limit : testedSet) {
        MyRange<Value> range(limit);
        for (Value i=1; i<limit; ++i) {
            EXPECT_EQ(limit, range.GetLimit());
            const auto diff = limit - i;
            const auto expected = ((diff % Value{2}) == Value{0}) ? diff : Value{0};
            EXPECT_EQ(expected, range.GetValue(i));
        }

        EXPECT_EQ(0, range.GetValue(0));
        EXPECT_EQ(0, range.GetValue(-1));
    }
}

TEST_F(TestRangeStream, Iterators) {
    using Value = int;
    constexpr Value length = 5;
    auto range = std::make_shared<MyRange<Value>>(length);

    auto ia = begin(range);
    auto ib = begin(range);
    auto ie = end(range);
    EXPECT_FALSE(*ia);
    EXPECT_FALSE(*ib);
    EXPECT_EQ(ia, ib);
    EXPECT_EQ(ib, ia);
    EXPECT_NE(ie, ia);
    EXPECT_NE(ie, ib);

    ++ia;
    EXPECT_FALSE(*ia);
    EXPECT_FALSE(*ib);
    EXPECT_NE(ia, ib);
    EXPECT_NE(ib, ia);
    EXPECT_NE(ie, ia);
    EXPECT_NE(ie, ib);

    ++ia;
    EXPECT_EQ(2, *ia);
    EXPECT_FALSE(*ib);
    EXPECT_NE(ia, ib);
    EXPECT_NE(ib, ia);
    EXPECT_NE(ie, ia);
    EXPECT_NE(ie, ib);

    ++ia;
    ++ia;
    EXPECT_EQ(4, *ia);
    EXPECT_FALSE(*ib);
    EXPECT_NE(ia, ib);
    EXPECT_NE(ib, ia);
    EXPECT_NE(ie, ia);
    EXPECT_NE(ie, ib);

    ++ib;
    ++ib;
    EXPECT_EQ(4, *ia);
    EXPECT_EQ(2, *ib);
    EXPECT_NE(ia, ib);
    EXPECT_NE(ib, ia);
    EXPECT_NE(ie, ia);
    EXPECT_NE(ie, ib);

    ++ib;
    ++ib;
    EXPECT_EQ(ia, ib);
    EXPECT_EQ(ib, ia);
    EXPECT_NE(ie, ia);
    EXPECT_NE(ie, ib);

    ++ia;
    EXPECT_NE(ia, ib);
    EXPECT_NE(ib, ia);
    const auto expected = std::numeric_limits<Value>::max();
    EXPECT_EQ(expected, *ia);
    EXPECT_EQ(ie, ia);
    EXPECT_NE(ie, ib);

    ++ib;
    EXPECT_EQ(ia, ib);
    EXPECT_EQ(ib, ia);
    EXPECT_EQ(ie, ia);
    EXPECT_EQ(ie, ib);
    EXPECT_EQ(expected, *ib);

    ++ia;
    ++ib;
    EXPECT_EQ(ia, ib);
    EXPECT_EQ(ib, ia);
    EXPECT_EQ(ie, ia);
    EXPECT_EQ(ie, ib);
    EXPECT_EQ(expected, *ia);
    EXPECT_EQ(expected, *ib);
}

TEST_F(TestRangeStream, IteratorEmpty) {
    using Value = int;
    auto range = std::make_shared<MyRange<Value>>(0);
    auto ia = begin(range);
    auto ie = end(range);
    EXPECT_EQ(ia, ie);

    const auto expected = std::numeric_limits<Value>::max();
    EXPECT_EQ(expected, *ia);
    EXPECT_EQ(ia, ie);
}

TEST_F(TestRangeStream, Member) {
    using Value = int;
    std::vector<Value> testedSet {1,2,9,10};
    for(const auto& length : testedSet) {
        auto range = std::make_shared<MyRange<Value>>(length);
        std::vector<Value> actual;

        auto it = begin(range);
        while(it != end(range)) {
            const auto value = *it;
            actual.push_back(value);
            ++it;
        }

        ASSERT_EQ(length, actual.size());
        for(Value i = 0; i < length; ++i) {
            if ((i % 2) == 0) {
                EXPECT_EQ(i, actual.at(i));
            } else {
                EXPECT_FALSE(actual.at(i));
            }
        }
    }
}

TEST_F(TestRangeStream, IntSum) {
    using Value = int;
    auto range = std::make_shared<MyRange<Value>>(101);
    const auto actual = std::accumulate(begin(range), end(range), Value{0},
                                  [](const auto& acc, const auto& element) {
                                      return acc + element;
                                  });
    EXPECT_EQ(2550, actual);
}

TEST_F(TestRangeStream, LongLongSum) {
    using Value = long long;
    auto range = std::make_shared<MyRange<Value>>(1000001);
    const auto actual = std::accumulate(begin(range), end(range), Value{0},
                                        [](const auto& acc, const auto& element) {
                                            return acc + element;
                                        });
    const Value expected = 250000500000ll;
    EXPECT_EQ(expected, actual);
}

TEST_F(TestRangeStream, BigInt) {
    using Value = boost::multiprecision::cpp_int;
    auto range = std::make_shared<MyRange<Value>>(1000001);
    const auto actual = std::accumulate(begin(range), end(range), Value{0},
                                        [](const auto& acc, const auto& element) {
                                            return acc + element;
                                        });
    const Value expected = 250000500000ll;
    EXPECT_EQ(expected, actual);
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
End:
*/
