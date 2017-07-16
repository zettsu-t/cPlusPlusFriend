// クイックソートの解説にある初歩的な実装例を試す
// ググって見つけたコードは、正しさと性能を確認してから使いましょう
// 正しい実装は、教科書を読んで確認してください
#include <cassert>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
#include <gtest/gtest.h>
#include "cFriendsCommon.h"
#include "cppFriends.hpp"

// ググって見つけたクイックソートの解説を元に、私が再実装したもの
class TestQuickSort : public ::testing::Test {
protected:
    using StackDepth = int;
    using Element = int;
    using ElementArray = std::vector<Element>;
    using Position = std::make_signed_t<ElementArray::size_type>;

    bool IsSorted(const ElementArray& array) const {
        if (array.empty()) {
            return true;
        }

        auto previous = array.front();
        auto i = array.cbegin();
        ++i;

        for(;i != array.cend(); ++i) {
            auto current = *i;
            if (previous > current) {
                return false;
            }
            previous = current;
        }

        return true;
    }

    void Print(const ElementArray& array, std::ostream& os) const {
        for(const auto& n : array) {
            os << n << ":";
        }
        return;
    }

    StackDepth QuickSort(ElementArray& array) const {
        auto arraySize = array.size();
        if (arraySize < 2) {
            return 0;
        }

        return quickSort(array, 0, static_cast<Position>(arraySize) - 1, 1);
    }

private:
    StackDepth quickSort(ElementArray& array, Position leftPos, Position rightPos, StackDepth depth) const {
        static_assert(std::is_signed<decltype(leftPos)>::value,  "Must be signed");
        static_assert(std::is_signed<decltype(rightPos)>::value, "Must be signed");
        // 要素数が1以下
        if (leftPos >= rightPos) {
            return depth;
        }

        // 並び順で真ん中にある要素で、それより大か小かで分ける
        // 真ん中である必要はないのだが、すでにソート済ならちょうどよいという考え
        const auto pivot = array.at((leftPos + rightPos) / 2);
        auto l = leftPos;
        auto r = rightPos;

        for (;;) {
            // pivotと==比較すると、pivotと同じ値が複数あるときに二等分できなくなる
            // vectorの末尾の先は指せるが(=end())、範囲チェックは行う
            while((l <= rightPos) && (array.at(l) < pivot)) {
                // 見つからなかったときはrightPosの次を指している
                ++l;
            }

            // 符号ありなので負になることができる
            // vectorの先頭より前は指せないので、rの範囲チェックは必須
            while((leftPos <= r) && (array.at(r) > pivot)) {
                // 見つからなかったときはleftPosの前を指している
                // rが0より小さくなっても正しく動作しなければならない
                static_assert(std::is_signed<decltype(r)>::value, "Must be signed");
                --r;
            }

            // l == rを外すと、以下で同じものをswapする。実害はないが。
            if (l >= r) {
                break;
            }

            // lが指しているものがpivotちょうどで、lが指しているものがpivot未満なら、
            // pivotを右に吹っ飛ばすことになる。結果として、pivot未満の境界が右に寄ることになる。
            std::swap(array.at(l), array.at(r));

            // lとrが指しているものがともにpivotちょうどのとき、lとrを無限に交換しないようにする
            ++l;
            --r;
        }

        const auto newDepth = depth + 1;
        // pivot以上またはpivot以下のとき要素しかないときに、無限ループを避ける
        const auto leftDepth = quickSort(array, leftPos, std::min(rightPos - 1, l - 1), newDepth);
        const auto rightDepth = quickSort(array, std::max(leftPos + 1, r + 1), rightPos, newDepth);

        return std::max(leftDepth, rightDepth);
    }
};

TEST_F(TestQuickSort, IsSorted) {
    ElementArray emptyArray;
    EXPECT_TRUE(IsSorted(emptyArray));

    ElementArray single {0};
    EXPECT_TRUE(IsSorted(single));

    ElementArray ascending {-1,1};
    EXPECT_TRUE(IsSorted(ascending));

    ElementArray flat {1,1};
    EXPECT_TRUE(IsSorted(flat));

    ElementArray descending {1,-1};
    EXPECT_FALSE(IsSorted(descending));

    ElementArray mixed {2,3,1};
    EXPECT_FALSE(IsSorted(mixed));

    ElementArray plateau1 {1,2,2,3};
    EXPECT_TRUE(IsSorted(plateau1));

    ElementArray plateau2 {1,2,2,2};
    EXPECT_TRUE(IsSorted(plateau2));

    ElementArray plateau3 {3,2,2,2};
    EXPECT_FALSE(IsSorted(plateau3));

    ElementArray plateau4 {2,2,2,1};
    EXPECT_FALSE(IsSorted(plateau4));
}

TEST_F(TestQuickSort, Print) {
    std::ostringstream os;
    ElementArray array {3,2,1};
    Print(array, os);
    std::string expected = "3:2:1:";
    EXPECT_EQ(expected, os.str());
}

// そもそもソートする必要がない
TEST_F(TestQuickSort, SpecialCases) {
    ElementArray emptyArray;
    EXPECT_EQ(0, QuickSort(emptyArray));
    EXPECT_TRUE(emptyArray.empty());
    EXPECT_TRUE(IsSorted(emptyArray));

    ElementArray single {2};
    EXPECT_EQ(0, QuickSort(single));
    ASSERT_EQ(1, single.size());
    EXPECT_EQ(2, single.at(0));
}

// 要素数2
TEST_F(TestQuickSort, Two) {
    const ElementArray expected {-1,1};
    ElementArray ascending  {-1,1};
    EXPECT_EQ(2, QuickSort(ascending));
    EXPECT_EQ(expected, ascending);

    ElementArray descending {-1,1};
    EXPECT_EQ(2, QuickSort(descending));
    EXPECT_EQ(expected, descending);

    const ElementArray expectedFlat {1,1};
    ElementArray flat {1,1};
    EXPECT_EQ(2, QuickSort(flat));
    EXPECT_EQ(expectedFlat, flat);
}

// すべての要素が同じ
TEST_F(TestQuickSort, Same) {
    // 要素数が2のn乗個
    for(int depth = 2; depth < 8; ++depth) {
        int size = 1;
        size <<= depth;
        ElementArray expected(size, 0);
        ElementArray array = expected;

        // 全要素が同じだと、再帰深度がO(N)になってしまうのは避けたい
        StackDepth expectedDepth = depth + 1;
        EXPECT_EQ(expected, array);
        EXPECT_EQ(expectedDepth, QuickSort(array));
    }

    // 要素数が2のn乗個以外でもソートできる
    for(int size = 3; size < 16; ++size) {
        ElementArray expected(size, 1);
        ElementArray array = expected;
        EXPECT_EQ(expected, array);
    }
}

// 要素に重複がない
TEST_F(TestQuickSort, Unique) {
    for(int size = 3; size <= 8; ++size) {
        ElementArray expected;
        for(int i = 1; i <= size; ++i) {
            expected.push_back(i);
        }

        int count = 0;
        ElementArray array = expected;
        do {
            ElementArray actual = array;
            QuickSort(actual);
            EXPECT_EQ(expected, actual);
            ++count;
        } while(std::next_permutation(array.begin(), array.end()));

        EXPECT_LT(1, count);
    }
}

// 要素に重複がある
TEST_F(TestQuickSort, NotUnique) {
    const int maskSet[] = {1,3,5};
    for(auto mask : maskSet) {
        int maskBits = ~mask;
        for(int size = 3; size <= 8; ++size) {
            // 0..を準備する
            ElementArray array;
            for(int i = 0; i < size; ++i) {
                array.push_back(i);
            }

            // マスクを掛けて重複を作る
            ElementArray expected = array;
            for(auto& v : expected) {
                v &= maskBits;
            }
            std::sort(expected.begin(), expected.end());

            int count = 0;
            do {
                ElementArray actual = array;
                // 重複を作る
                for(auto& v : actual) {
                    v &= maskBits;
                }

                QuickSort(actual);
                EXPECT_EQ(expected, actual);
                ++count;
            } while(std::next_permutation(array.begin(), array.end()));

            EXPECT_LT(1, count);
        }
    }
}

// 既にソート済
TEST_F(TestQuickSort, AlreadySorted) {
    constexpr int size = 32;
    ElementArray expected;
    for(int i = 1; i <= size; ++i) {
        expected.push_back(i);
    }

    ElementArray ascending = expected;
    ElementArray descending = expected;
    std::reverse(descending.begin(), descending.end());
    EXPECT_TRUE(IsSorted(ascending));
    EXPECT_FALSE(IsSorted(descending));

    EXPECT_EQ(6, QuickSort(ascending));
    EXPECT_EQ(6, QuickSort(descending));
}

// ある種の入力では再帰深度がO(N)になってしまう
TEST_F(TestQuickSort, DeepStack) {
    for(int scale=2; scale<8; ++scale) {
        int size = 1;
        size <<= scale;
        const int arraySize = size * 2;
        int n = 1;
        int l = 0;
        int r = arraySize - 1;
        ElementArray array(arraySize, 0);

        for(int i=0; i<size; ++i) {
            array.at(l) = n;
            ++n;
            ++l;
            array.at(r) = n;
            ++n;
            --r;
        }

        ElementArray expected = array;
        std::sort(expected.begin(), expected.end());
        EXPECT_EQ(size + 1, QuickSort(array));
        EXPECT_EQ(expected, array);
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
