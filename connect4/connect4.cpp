#include <array>
#include <bitset>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <random>
#include <gtest/gtest.h>

struct Board {
    using Coordinate = int32_t;
    using HashKey = int64_t;

    struct Point {
        Coordinate column {0};
        Coordinate height {0};
    };

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
    std::array<Cells, LineTypes> linemasks_;
    std::array<HashKey, FullSize> hashkeys_ {0};
    HashKey digest_ {0};

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

    void place(Board::Coordinate column, Board::Coordinate height) {
        const auto index = to_index(column, height);
        placed_.set(index);
        digest_ ^= hashkeys_.at(index);
    }

    Board merge(const auto& rhs) const {
        auto retval = *this;
        retval.placed_ ^= rhs.placed_;
        retval.digest_ ^= rhs.digest_;
        return retval;
    }

    static Coordinate to_index(Coordinate column, Coordinate height) {
        return column * FullHeight + height;
    }

    static std::string to_string(const Cells& cells, char blank, char mark) {
        std::ostringstream oss;
        for(Coordinate height{MaxHeight-1}; height >= 0; --height) {
            for(Coordinate column{0}; column < ColumnSize; ++column) {
                const auto c = (cells[to_index(column, height)]) ? mark : blank;
                oss << c;
            }
            oss << "\n";
        }

        return oss.str();
    }

    std::string to_string(char blank, char mark) const {
        return to_string(placed_, blank, mark);
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
                    points.push_back(Point{column, height});
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

    ASSERT_EQ(Board::FullSize, board.hashkeys_.size());
    std::set<Board::HashKey> keys;
    for(const auto& key: board.hashkeys_) {
        ASSERT_NE(0, key);
        ASSERT_FALSE(keys.contains(key));
    }

    ASSERT_EQ(0, board.digest_);
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

TEST_F(TestBoard, Place) {
    Board full;
    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            Board one;

            const auto index = Board::to_index(column, height);
            const auto digest_one = one.hashkeys_.at(index);
            const auto digest_old = full.digest_;
            const auto expected_digest = digest_old ^ full.hashkeys_.at(index);
            ASSERT_TRUE(digest_one);
            ASSERT_NE(digest_old, expected_digest);

            full.place(column, height);
            one.place(column, height);
            ASSERT_TRUE(full.placed_[index]);
            ASSERT_TRUE(one.placed_[index]);

            ASSERT_EQ(digest_one, one.digest_);
            ASSERT_EQ(expected_digest, full.digest_);
            ASSERT_TRUE(one.digest_);
            ASSERT_TRUE(full.digest_);
        }
    }
}

TEST_F(TestBoard, Merge) {
    Board zero;
    for(Board::Coordinate offset{0}; offset < 2; ++offset) {
        Board lhs;
        Board rhs;
        size_t expected {0};
        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
                const auto index = Board::to_index(column, height);
                if (((column & 1) ^ (height & 1)) == offset) {
                    lhs.placed_.set(index);
                    ++expected;
                } else {
                    rhs.placed_.set(index);
                }
            }
        }

        const auto expected_digest = lhs.digest_ ^ rhs.digest_;
        const auto actual_left = lhs.merge(rhs);
        const auto actual_right = rhs.merge(lhs);

        ASSERT_EQ(Board::ColumnSize * Board::MaxHeight, actual_left.placed_.count());
        ASSERT_EQ(Board::ColumnSize * Board::MaxHeight, actual_right.placed_.count());
        ASSERT_EQ(expected_digest, actual_left.digest_);
        ASSERT_EQ(expected_digest, actual_left.digest_);

        const auto actual_left_self = lhs.merge(lhs);
        const auto actual_right_self = rhs.merge(rhs);
        ASSERT_EQ(zero.placed_, actual_left_self.placed_);
        ASSERT_EQ(zero.placed_, actual_right_self.placed_);
        ASSERT_EQ(0, actual_left_self.digest_);
        ASSERT_EQ(0, actual_right_self.digest_);

        Board zero;
        const auto zero_left = zero.merge(lhs);
        const auto zero_right = lhs.merge(zero);
        ASSERT_EQ(expected, zero_left.placed_.count());
        ASSERT_EQ(expected, zero_right.placed_.count());
        ASSERT_EQ(lhs.digest_, zero_left.digest_);
        ASSERT_EQ(lhs.digest_, zero_right.digest_);
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

TEST_F(TestBoard, ToString) {
    Board board;
    const char blank {'_'};
    const char mark {'+'};

    for(Board::Coordinate offset{0}; offset < 2; ++offset) {
        Board::Cells cells;
        std::string lines(Board::ColumnSize * Board::MaxHeight, blank);

        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
                if (((column ^ height) ^ offset) > 0) {
                    cells.set(Board::to_index(column, height));
                    lines.at(column + (Board::MaxHeight - 1 - height) * Board::ColumnSize) = mark;
                }
            }
        }

        std::string s;
        for(Board::Coordinate y{0}; y<Board::MaxHeight; ++y) {
            s += lines.substr(y * Board::ColumnSize, Board::ColumnSize);
            s += "\n";
        }

        ASSERT_EQ(s, board.to_string(cells, blank, mark));
    }
}

TEST_F(TestBoard, ToStringMask) {
    Board board;
    const char blank {'_'};
    const char mark {'+'};

    std::string filled_line(Board::ColumnSize, mark);
    filled_line += "\n";

    std::string s;
    for(Board::Coordinate i{0}; i<Board::MaxHeight; ++i) {
        s += filled_line;
    }

    ASSERT_EQ(s, board.to_string(board.mask_, blank, mark));
}

TEST_F(TestBoard, ToStringPlaced) {
    Board board;
    const char blank {'_'};
    const char mark {'+'};

    const std::string line(Board::ColumnSize, blank);
    std::string s;
    for(Board::Coordinate i{0}; i<Board::MaxHeight; ++i) {
        s += line;
        s += "\n";
    }

    ASSERT_EQ(s, board.to_string(blank, mark));

    board.placed_.set(Board::FullHeight + Board::MaxHeight - 1);
    const auto pos = 1;
    s.at(pos) = mark;
    ASSERT_EQ(s, board.to_string(blank, mark));
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
        ASSERT_EQ(column, actual.at(column).column);
        ASSERT_EQ(0, actual.at(column).height);
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
            ASSERT_EQ(column, actual.at(column).column);
            ASSERT_EQ(expected.at(column), actual.at(column).height);
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

struct Stage {
    using Player = int;
    static constexpr Player SizeOfPlayers {2};
    static inline const char Blank {'_'};
    static inline const std::string Marks {'+', '-'};

    enum class Result {
        Invalid,
        Placed,
        Won,
    };

    std::array<std::unique_ptr<Board>, SizeOfPlayers> boards_;
    Player player_ {0};

    Stage() {
        for(auto&& p : boards_) {
            p = std::make_unique<Board>();
        }
    }

    std::string to_string() const {
        std::string str_full;
        for(Player player {0}; player < SizeOfPlayers; ++player) {
            const auto str_board = boards_.at(player)->to_string(Blank, Marks.at(player));
            if (player == 0) {
                str_full = str_board;
            } else {
                auto size = str_board.size();
                for(decltype(size) i {0}; i < size; ++i) {
                    str_full.at(i) = (str_board.at(i) != Blank) ? str_board.at(i) : str_full.at(i);
                }
            }
        }

        return str_full;
    }

    Result advance(Board::Coordinate column, Board::Coordinate height) {
        const auto merged = boards_.at(0)->merge(*(boards_.at(1).get()));
        const auto points = merged.legal_actions();

        for(const auto& [c, h] : points) {
            if ((column == c) && (height == h)) {
                boards_.at(player_)->place(column, height);
                if (boards_.at(player_)->check(column, height)) {
                    return Result::Won;
                }

                player_ = (player_ == 0) ? 1 : 0;
                return Result::Placed;
            }
        }

        return Result::Invalid;
    }
};

class TestStage : public ::testing::Test {
protected:
    std::string get_blank_lines() {
        std::string line (Board::ColumnSize, Stage::Blank);
        line += "\n";

        std::string expected;
        for(Board::Coordinate i{0}; i<Board::MaxHeight; ++i) {
            expected += line;
        }
        return expected;
    }
};

TEST_F(TestStage, Initialize) {
    Stage stage;

    for(const auto& p : stage.boards_) {
        ASSERT_TRUE(p.get());
    }

    ASSERT_EQ(0, stage.player_);
}

TEST_F(TestStage, ToStringInitial) {
    Stage stage;
    const auto expected = get_blank_lines();
    const auto actual = stage.to_string();
    ASSERT_EQ(expected, actual);
}

TEST_F(TestStage, ToStringFull) {
    Stage stage_0;
    Stage stage_1;
    auto expected_0 = get_blank_lines();
    auto expected_1 = get_blank_lines();

    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            const auto index_left = Board::to_index(column, height);
            const auto column_r = Board::ColumnSize - column - 1;
            const auto height_r = Board::MaxHeight - height - 1;
            if ((column == column_r) && (height == height_r)) {
                continue;
            }

            Stage stage_two;
            stage_two.boards_.at(0)->place(column, height);
            stage_two.boards_.at(1)->place(column_r, height_r);
            stage_0.boards_.at(0)->place(column, height);
            stage_1.boards_.at(1)->place(column_r, height_r);

            auto expected_two = get_blank_lines();
            const auto index_0 = column + (Board::ColumnSize + 1) * (Board::MaxHeight - height - 1);
            const auto index_1 = column_r + (Board::ColumnSize + 1) * (Board::MaxHeight - height_r - 1);
            expected_two.at(index_0) = Stage::Marks.at(0);
            expected_two.at(index_1) = Stage::Marks.at(1);
            expected_0.at(index_0) = Stage::Marks.at(0);
            expected_1.at(index_1) = Stage::Marks.at(1);

            const auto actual = stage_two.to_string();
            const auto actual_0 = stage_0.to_string();
            const auto actual_1 = stage_1.to_string();
            ASSERT_EQ(expected_two, actual);
            ASSERT_EQ(expected_0, actual_0);
            ASSERT_EQ(expected_1, actual_1);
        }
    }
}

TEST_F(TestStage, Advance1) {
    struct Step {
        Board::Coordinate column {0};
        Board::Coordinate height {0};
        Stage::Result expected {Stage::Result::Invalid};
    };

    /*
     * 3   A
     * 2   BA
     * 1   BBA
     * 0AAABBBA
     *  0123456
     */
    const std::vector<Step> steps {
        {0, 0, Stage::Result::Placed},
        {1, 1, Stage::Result::Invalid},
        {3, 0, Stage::Result::Placed},
        {1, 0, Stage::Result::Placed},
        {4, 0, Stage::Result::Placed},
        {2, 0, Stage::Result::Placed},
        {5, 0, Stage::Result::Placed},
        {6, 0, Stage::Result::Placed},
        {3, 1, Stage::Result::Placed},
        {5, 1, Stage::Result::Placed},
        {4, 1, Stage::Result::Placed},
        {4, 2, Stage::Result::Placed},
        {3, 2, Stage::Result::Placed},
        {0, 2, Stage::Result::Invalid},
        {1, 2, Stage::Result::Invalid},
        {2, 2, Stage::Result::Invalid},
        {3, 5, Stage::Result::Invalid},
        {3, 3, Stage::Result::Won},
    };

    Stage stage;
    for(const auto& step : steps) {
        const auto actual = stage.advance(step.column, step.height);
        ASSERT_EQ(step.expected, actual);
    }

    ASSERT_EQ(0, stage.player_);
}

TEST_F(TestStage, Advance2) {
    struct Step {
        Board::Coordinate column {0};
        Board::Coordinate height {0};
        Stage::Result expected {Stage::Result::Invalid};
    };

    /*
     * 3  B
     * 2  B
     * 1  B
     * 0 ABAA A
     *  0123456
     */
    const std::vector<Step> steps {
        {0, 0, Stage::Result::Placed},
        {1, 0, Stage::Result::Placed},
        {0, 0, Stage::Result::Invalid},
        {0, 0, Stage::Result::Invalid},
        {2, 0, Stage::Result::Placed},
        {1, 1, Stage::Result::Placed},
        {3, 0, Stage::Result::Placed},
        {1, 2, Stage::Result::Placed},
        {6, 0, Stage::Result::Placed},
        {1, 4, Stage::Result::Invalid},
        {1, 3, Stage::Result::Won},
    };

    Stage stage;
    for(const auto& step : steps) {
        const auto actual = stage.advance(step.column, step.height);
        ASSERT_EQ(step.expected, actual);
    }

    ASSERT_EQ(1, stage.player_);
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    Stage stage;
    for(;;) {
        std::cout << stage.to_string() << std::endl;
        std::cout << "Player " << (1 + stage.player_) << " moves\n";

        Board::Coordinate column {0};
        Board::Coordinate height {0};
        std::cin >> column >> height;

        const auto result = stage.advance(column, height);

        if (result == Stage::Result::Won) {
            std::cout << "Player " << (1 + stage.player_) << " Win\n";
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
