#include <array>
#include <bitset>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <random>
#include <gtest/gtest.h>

struct Board {
    using Coordinate = int32_t;
    using HashKey = int64_t;
    using Point = std::pair<Coordinate, Coordinate>;
    using Points = std::vector<Point>;

    static inline constexpr Coordinate ColumnSize = 7;
    static inline constexpr Coordinate MaxHeight = 6;
    static inline constexpr Coordinate FullWidth = 8;
    static inline constexpr Coordinate FullHeight = 8;
    static_assert(ColumnSize <= FullWidth);
    static_assert(MaxHeight <= FullHeight);
    static inline constexpr Coordinate FullSize = FullWidth * FullHeight;

    using Cells = std::bitset<FullSize>;
    static inline constexpr Coordinate LineTypes = 4;
    static inline constexpr Coordinate MinLen = 4;

    Cells placed_;
    Cells mask_;
    Coordinate last_ {-1};
    std::array<Cells, LineTypes> linemasks_;
    std::array<HashKey, FullSize> hashkeys_ {0};

    Board() {
        for(Coordinate column{0}; column < ColumnSize; ++column) {
            for(Coordinate height{0}; height < MaxHeight; ++height) {
                const auto index = to_index(column, height);
                mask_.set(index);
            }
        }

        for(Coordinate i{0}; i < MinLen; ++i) {
            linemasks_.at(0).set(i);
            linemasks_.at(1).set(i * FullHeight);
            linemasks_.at(2).set(i * (FullHeight + 1));
            linemasks_.at(3).set(MinLen - 1 + i * (FullHeight - 1));
        }

        const auto upper = std::numeric_limits<HashKey>::max();
        const auto lower = upper / 4;
        std::random_device seed_gen;
        std::mt19937 engine(seed_gen());
        std::uniform_int_distribution<HashKey> dist(lower, upper);

        for(auto&& key : hashkeys_) {
            key = dist(engine);
        }
    }

    static Coordinate to_index(Coordinate column, Coordinate height) {
        return column * FullHeight + height;
    }

    static std::string to_string(const Cells& cells, char mark) {
        std::ostringstream oss;
        for(Coordinate height{FullHeight-1}; height >= 0; --height) {
            for(Coordinate column{0}; column < FullWidth; ++column) {
                const auto c = (cells[to_index(column, height)]) ? mark : '_';
                oss << c;
            }
            oss << "\n";
        }

        return oss.str();
    }

    bool check_line(Coordinate line_index, Coordinate left_shift) const {
        if (left_shift < 0) {
            return false;
        }

        auto line = linemasks_.at(line_index);
        line <<= left_shift;
        line &= mask_;
        line &= placed_;
        return (line.count() >= MinLen);
    }

    bool check_vertical_line(Coordinate bottom, Coordinate top, Coordinate column) const {
        for(Coordinate y {bottom}; y <= top; ++y) {
            if (check_line(0, to_index(column, y))) {
                return true;
            }
        }

        return false;
    }

    bool check_horizontal_line(Coordinate left, Coordinate right, Coordinate height) const {
        for(Coordinate x {left}; x <= right; ++x) {
            if (check_line(1, to_index(x, height))) {
                return true;
            }
        }

        return false;
    }

    bool check_line2(Coordinate left, Coordinate right, Coordinate column, Coordinate height) const {
        for(Coordinate x {left}; x <= right; ++x) {
            const auto offset = x - column;
            const auto y = height + offset;
            if ((y >= 0) && (y < MaxHeight)) {
                if (check_line(2, to_index(x, y))) {
                    return true;
                }
            }
        }

        return false;
    }

    bool check_line3(Coordinate left, Coordinate right, Coordinate column, Coordinate height) const {
        for(Coordinate x {left}; x <= right; ++x) {
            const auto offset = x - column;
            const auto y = height - offset;
            if ((y >= 0) && (y < MaxHeight)) {
                const auto left_shift = to_index(x, y) - (MinLen - 1);
                if (check_line(3, left_shift)) {
                    return true;
                }
            }
        }

        return false;
    }

    bool check(Coordinate column, Coordinate height) const {
        const auto left = std::max(0, column - (MinLen - 1));
        const auto right = std::min(ColumnSize - MinLen, column);
        const auto bottom = std::max(0, height - (MinLen - 1));
        const auto top = std::min(MaxHeight - MinLen, height);

        if (check_vertical_line(bottom, top, column)) {
            return true;
        }

        if (check_horizontal_line(left, right, height)) {
            return true;
        }

        if (check_line2(left, right, column, height)) {
            return true;
        }

        return check_line3(left, right, column, height);
    }

    Points legal_actions() const {
        Points points;
        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
                auto index = to_index(column, height);
                if (!placed_[index]) {
                    points.push_back(std::make_pair(column, height));
                    break;
                }
            }
        }

        return points;
    }
};

class TestBoard : public ::testing::Test {};

TEST_F(TestBoard, Initialize) {
    Board board;

    ASSERT_EQ(Board::FullSize, board.placed_.size());
    ASSERT_EQ(Board::FullSize, board.mask_.size());
    auto size = board.mask_.size();

    for(decltype(size) i{0}; i<size; ++i) {
        ASSERT_FALSE(board.placed_[i]);
        const auto expected = ((i / Board::FullHeight) < Board::ColumnSize) &
            ((i % Board::FullHeight) < Board::MaxHeight);
        ASSERT_EQ(expected, board.mask_[i]);
    }

    ASSERT_GT(0, board.last_);

    ASSERT_EQ(Board::FullSize, board.hashkeys_.size());
    std::set<Board::HashKey> keys;
    for(const auto& key: board.hashkeys_) {
        ASSERT_NE(0, key);
        ASSERT_FALSE(keys.contains(key));
    }
}

TEST_F(TestBoard, InitializeLine) {
    Board board;

    for(const auto& line : board.linemasks_) {
        ASSERT_EQ(Board::MinLen, line.count());
    }

    for(Board::Coordinate i{0}; i < Board::MinLen; ++i) {
        ASSERT_TRUE(board.linemasks_.at(0)[i]);
    }

    for(Board::Coordinate i{0}; i < Board::MinLen; ++i) {
        ASSERT_TRUE(board.linemasks_.at(1)[i * Board::FullHeight]);
    }

    for(Board::Coordinate i{0}; i < Board::MinLen; ++i) {
        ASSERT_TRUE(board.linemasks_.at(2)[i * (Board::FullHeight + 1)]);
    }

    for(Board::Coordinate i{0}; i < Board::MinLen; ++i) {
        ASSERT_TRUE(board.linemasks_.at(3)[Board::MinLen - 1 + i * (Board::FullHeight - 1)]);
    }
}

TEST_F(TestBoard, ToIndex) {
    Board board;

    ASSERT_EQ(0, board.to_index(0, 0));
    ASSERT_EQ(Board::MaxHeight - 1, board.to_index(0, Board::MaxHeight - 1));
    ASSERT_EQ(Board::FullHeight - 1, board.to_index(0, Board::FullHeight - 1));
    ASSERT_EQ(Board::FullHeight, board.to_index(1, 0));
    ASSERT_EQ(Board::FullHeight + Board::MaxHeight - 1, board.to_index(1, Board::MaxHeight - 1));
    ASSERT_EQ(Board::FullHeight * 2 - 1, board.to_index(1, Board::FullHeight - 1));
    ASSERT_EQ(Board::FullSize - 1, board.to_index(Board::FullWidth - 1, Board::FullHeight - 1));
}

TEST_F(TestBoard, ToStringEmpty) {
    Board board;
    const char mark {'+'};

    const std::string line(Board::FullWidth, '_');
    std::string s;
    for(Board::Coordinate i{0}; i<Board::FullHeight; ++i) {
        s += line;
        s += "\n";
    }

    ASSERT_EQ(s, board.to_string(board.placed_, mark));
}

TEST_F(TestBoard, ToStringMask) {
    Board board;
    const char mark {'+'};

    const std::string empty_line(Board::FullWidth, '_');
    std::string filled_line(Board::ColumnSize, mark);
    for(Board::Coordinate i{Board::ColumnSize}; i<Board::FullHeight; ++i) {
        filled_line += " ";
    }

    std::string s;
    for(Board::Coordinate i{0}; i<Board::FullHeight; ++i) {
        if ((i + Board::MaxHeight) < Board::FullHeight) {
            s += empty_line;
        } else {
            s += filled_line;
        }
        s += "\n";
    }

    ASSERT_EQ(s, board.to_string(board.mask_, mark));
}

TEST_F(TestBoard, ToString) {
    Board board;
    const char mark {'+'};

    for(Board::Coordinate offset{0}; offset < 2; ++offset) {
        Board::Cells cells;
        std::string lines(Board::FullSize, '_');

        for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
            for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
                if (((column ^ height) ^ offset) > 0) {
                    cells.set(Board::to_index(column, height));
                    lines.at(column + (Board::FullHeight - 1 - height) * Board::FullWidth) = mark;
                }
            }
        }

        std::string s;
        for(Board::Coordinate y{0}; y<Board::FullHeight; ++y) {
            s += lines.substr(y * Board::FullWidth, Board::FullWidth);
            s += "\n";
        }

        ASSERT_EQ(s, board.to_string(cells, mark));
    }
}

TEST_F(TestBoard, CheckVerticalLineAll) {
    for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
        Board board;

        for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
            board.placed_.set(Board::to_index(column, height));
        }

        for(Board::Coordinate i{0}; i < board.placed_.size(); ++i) {
            const auto x = i / Board::FullHeight;
            const auto y = i % Board::FullHeight;
            const bool expected = (x == column) & (x < Board::ColumnSize) &
                (y <= (Board::MaxHeight - Board::MinLen));
            ASSERT_EQ(expected, board.check_line(0, i));
        }
    }
}

TEST_F(TestBoard, CheckVerticalLineRanged) {
    constexpr Board::Coordinate column {2};
    Board board;

    for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
        board.placed_.set(Board::to_index(column, height));
    }

    const auto top = Board::MaxHeight - Board::MinLen;
    EXPECT_FALSE(board.check_vertical_line(top + 1, Board::FullHeight - 1, column));
    EXPECT_TRUE(board.check_vertical_line(0, top, column));
}

TEST_F(TestBoard, CheckLineHorizontalAll) {
    for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
        Board board;

        for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
            board.placed_.set(Board::to_index(column, height));
        }

        for(Board::Coordinate i{0}; i < board.placed_.size(); ++i) {
            const auto x = i / Board::FullHeight;
            const auto y = i % Board::FullHeight;
            const bool expected = (y == height) & (y < Board::MaxHeight) &
                (x <= (Board::ColumnSize - Board::MinLen));
            ASSERT_EQ(expected, board.check_line(1, i));
        }

    }
}

TEST_F(TestBoard, CheckLineHorizontalRanged) {
    constexpr Board::Coordinate height {2};
    Board board;

    for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
        board.placed_.set(Board::to_index(column, height));
    }

    const auto right = Board::ColumnSize - Board::MinLen;
    EXPECT_FALSE(board.check_horizontal_line(right + 1, Board::FullWidth - 1, height));
    EXPECT_TRUE(board.check_horizontal_line(0, right, height));
}

TEST_F(TestBoard, CheckLine2All) {
    for(Board::Coordinate column{-Board::FullWidth}; column < Board::FullWidth; ++column) {
        Board board;

        for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
            const auto x = column + height;
            if ((x >= 0) && (x < Board::FullWidth)) {
                board.placed_.set(Board::to_index(x, height));
            }
        }

        for(Board::Coordinate i{0}; i < board.placed_.size(); ++i) {
            const auto x = i / Board::FullHeight;
            const auto y = i % Board::FullHeight;
            const bool expected = ((x - y) == column) &
                (x <= (Board::ColumnSize - Board::MinLen)) &
                (y <= (Board::MaxHeight - Board::MinLen));
            ASSERT_EQ(expected, board.check_line(2, i));
        }
    }
}

TEST_F(TestBoard, CheckLine2Ranged) {
    Board board;
    for(Board::Coordinate i{0}; i < Board::FullWidth; ++i) {
        board.placed_.set(Board::to_index(i, i));
    }

    const auto right = Board::ColumnSize - 1;
    for(Board::Coordinate column{0}; column <= right; ++column) {
        const auto expected = column <= (std::min(Board::ColumnSize, Board::MaxHeight) - Board::MinLen);
        EXPECT_EQ(expected, board.check_line2(column, right, right, right));
    }
}

TEST_F(TestBoard, CheckLine3All) {
    for(Board::Coordinate column{-Board::FullWidth}; column < Board::FullWidth; ++column) {
        Board board;

        for(Board::Coordinate height{Board::FullHeight - 1}; height >= 0; --height) {
            const auto x = column + Board::FullHeight - 1 - height;
            if ((x >= 0) && (x < Board::FullWidth)) {
                board.placed_.set(Board::to_index(x, height));
            }
        }

        for(Board::Coordinate i{0}; i < board.placed_.size(); ++i) {
            const auto x = i / Board::FullHeight;
            const auto y = i % Board::FullHeight;
            const auto total = std::min(Board::ColumnSize - x, y + 1);
            const bool expected = ((x + y) == (column + Board::FullHeight - 1)) &
                (x < Board::ColumnSize) & (y < Board::MaxHeight) && (total >= Board::MinLen);
            EXPECT_EQ(expected, board.check_line(3, i - (Board::MinLen - 1)));
        }
    }
}

TEST_F(TestBoard, CheckLine3Ranged) {
    Board board;
    const auto size = std::min(Board::ColumnSize, Board::MaxHeight);
    for(Board::Coordinate i{0}; i < size; ++i) {
        board.placed_.set(Board::to_index(i, size - 1 - i));
    }

    const auto right = size - 1;
    for(Board::Coordinate column{0}; column < size; ++column) {
        const auto expected = column <= (size - Board::MinLen);
        EXPECT_EQ(expected, board.check_line3(column, right, column, size - 1 - column));
    }
}

TEST_F(TestBoard, CheckLineCorners) {
    using Coord = Board::Coordinate;
    const std::vector<std::tuple<Coord, Coord, Coord, Coord>> cases {
        {0, 0, 1, 0}, {0, 0, 0, 1}, {0, 0, 1, 1},
        {0, Board::MaxHeight - 1, 1, 0}, {0, Board::MaxHeight - 1, 0, -1}, {0, Board::MaxHeight - 1, 1, -1},
        {Board::ColumnSize - 1, 0, -1, 0}, {Board::ColumnSize - 1, 0, 0, 1}, {Board::ColumnSize - 1, 0, -1, 1},
        {Board::ColumnSize - 1, Board::MaxHeight - 1, -1, 0},
        {Board::ColumnSize - 1, Board::MaxHeight - 1, 0, -1},
        {Board::ColumnSize - 1, Board::MaxHeight - 1, -1, -1}
    };

    for(const auto& [sx, sy, dx, dy] : cases) {
        std::set<std::pair<Coord, Coord>> ps;
        Board board;
        auto x = sx;
        auto y = sy;

        for(Board::Coordinate i{0}; i < Board::MinLen; ++i) {
            board.placed_.set(Board::to_index(x, y));
            ps.insert(std::make_pair(x, y));
            x += dx;
            y += dy;
        }
        EXPECT_TRUE(board.check(sx, sy));

        for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
            for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
                const auto p = std::make_pair(column, height);
                const auto expected = ps.contains(p);
                EXPECT_EQ(expected, board.check(p.first, p.second));
            }
        }
    }
}

TEST_F(TestBoard, CheckLineAll) {
    using Coord = Board::Coordinate;
    const std::vector<std::tuple<Coord, Coord>> dxys {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
        for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
            for(const auto& [dx, dy] : dxys) {
                Board board;
                std::set<std::pair<Coord, Coord>> ps;
                auto x = column;
                auto y = height;

                Coord total {0};
                for(;;) {
                    if ((x < 0) || (x >= Board::ColumnSize) || (y < 0) || (y >= Board::MaxHeight)) {
                        break;
                    }

                    board.placed_.set(Board::to_index(x, y));
                    ps.insert(std::make_pair(x, y));
                    x += dx;
                    y += dy;
                    ++total;
                }

                for(Board::Coordinate sx{0}; sx < Board::FullWidth; ++sx) {
                    for(Board::Coordinate sy{0}; sy < Board::FullHeight; ++sy) {
                        const auto p = std::make_pair(sx, sy);
                        const auto expected = (total >= Board::MinLen) && ps.contains(p);
                        EXPECT_EQ(expected, board.check(p.first, p.second));
                    }
                }
            }
        }
    }
}

TEST_F(TestBoard, LegalActionsEmpty) {
    Board board;

    const auto actual = board.legal_actions();
    ASSERT_EQ(Board::ColumnSize, actual.size());

    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        ASSERT_EQ(column, actual.at(column).first);
        ASSERT_EQ(0, actual.at(column).second);
    }
}

TEST_F(TestBoard, LegalActionsFull) {
    Board board;

    for(size_t i{0}; i<board.placed_.size(); ++i) {
        board.placed_.set(i);
    }

    const auto actual = board.legal_actions();
    ASSERT_TRUE(actual.empty());
}

TEST_F(TestBoard, LegalActionsEach) {
    for(Board::Coordinate i{0}; i < (Board::MaxHeight - 1); ++i) {
        const auto h = Board::MaxHeight - 1 - i;
        std::vector<Board::Coordinate> expected {i, h, i, h, i, h, i};
        ASSERT_EQ(Board::ColumnSize, expected.size());

        Board board;
        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            for(Board::Coordinate height{0}; height < expected.at(column); ++height) {
                board.placed_.set(Board::to_index(column, height));
            }
        }

            const auto actual = board.legal_actions();
        ASSERT_EQ(Board::ColumnSize, actual.size());

        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            ASSERT_EQ(column, actual.at(column).first);
            ASSERT_EQ(expected.at(column), actual.at(column).second);
        }
    }
}

TEST_F(TestBoard, LegalActionsMax) {
    std::vector<Board::Coordinate> expected(Board::ColumnSize, 0);
    std::iota(expected.begin(), expected.end(), 0);

    for(Board::Coordinate i{0}; i < Board::MaxHeight; ++i) {
        Board board;
        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            for(Board::Coordinate height{0}; height < expected.at(column); ++height) {
                board.placed_.set(Board::to_index(column, height));
            }
        }

        const auto actual = board.legal_actions();
        std::map<Board::Coordinate, Board::Coordinate> pos;
        for(const auto& [k,v] : actual) {
            pos[k] = v;
        }

        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            const auto& val = expected.at(column);
            if (val >= Board::MaxHeight) {
                ASSERT_TRUE(pos.find(column) == pos.end());
            } else {
                ASSERT_TRUE(pos.find(column) != pos.end());
                ASSERT_EQ(val, pos[column]);
            }
        }

        for(auto&& x : expected) {
            x = std::max(x, Board::MaxHeight - 1);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    Board board;
    for(;;) {
        auto s = board.to_string(board.placed_, '.');
        const auto actual = board.legal_actions();
        for(const auto& [x, y] : actual) {
            const auto pos = (Board::FullWidth + 1) * (Board::FullHeight - 1 - y) + x;
            s.at(pos) = '*';
        }
        std::cout << s << std::endl;

        Board::Coordinate dx {0};
        Board::Coordinate dy {0};
        std::cin >> dx >> dy;
        board.placed_.set(board.to_index(dx, dy));

        if (board.check(dx, dy)) {
            std::cout << board.to_string(board.placed_, '#') << "Complete!" << std::endl;
            break;
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
