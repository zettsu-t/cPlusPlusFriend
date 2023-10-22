#include <cmath>
#include <array>
#include <bitset>
#include <chrono>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <random>
#include <vector>
#include <gtest/gtest.h>

namespace {
// 参加者
using Player = int;

// ミリ秒
using MilliSec = std::chrono::milliseconds;
const MilliSec ZeroMilliSec {0};
}

// 盤面
class Board final {
    FRIEND_TEST(TestBoard, Initialize);
    FRIEND_TEST(TestBoard, SetupLines);
    FRIEND_TEST(TestBoard, Digest);
    FRIEND_TEST(TestBoard, Place);
    FRIEND_TEST(TestBoard, PlaceTwice);
    FRIEND_TEST(TestBoard, PlaceOutOfBounds);
    FRIEND_TEST(TestBoard, Remove);
    FRIEND_TEST(TestBoard, RemoveOutOfBounds);
    FRIEND_TEST(TestBoard, Merge);
    FRIEND_TEST(TestBoard, ToIndex);
    FRIEND_TEST(TestBoard, ToStringCells);
    FRIEND_TEST(TestBoard, ToStringMask);
    FRIEND_TEST(TestBoard, ToStringPlaced);
    FRIEND_TEST(TestBoard, CheckVerticalLineAll);
    FRIEND_TEST(TestBoard, CheckVerticalLineRanged);
    FRIEND_TEST(TestBoard, CheckLineHorizontalAll);
    FRIEND_TEST(TestBoard, CheckLineHorizontalRanged);
    FRIEND_TEST(TestBoard, CheckLine2All);
    FRIEND_TEST(TestBoard, CheckLine3All);
    FRIEND_TEST(TestBoard, CheckLineCorners);
    FRIEND_TEST(TestBoard, CheckLineAll);
    FRIEND_TEST(TestBoard, CheckLoop);
    FRIEND_TEST(TestBoard, LegalActionsEmpty);
    FRIEND_TEST(TestBoard, LegalActionsFull);
    FRIEND_TEST(TestBoard, LegalActionsHeight);
    FRIEND_TEST(TestBoard, LegalActionsMax);
    FRIEND_TEST(TestBoard, Full);
    FRIEND_TEST(TestStage, Initialize);
    FRIEND_TEST(TestStage, MergeBoards);
    FRIEND_TEST(TestStage, Digest);
    FRIEND_TEST(TestStage, Full);
    FRIEND_TEST(TestStage, WinAtLastMove);
    FRIEND_TEST(TestNode, Stage);
    FRIEND_TEST(TestNodeSet, Add);

public:
    // 盤面上の座標(0-based indexing)
    using Coordinate = int32_t;  // 差を取るので符号ありにする
    struct Position {
        Coordinate column {0};  // 横方向
        Coordinate height {0};  // 縦方向
    };
    using Positions = std::vector<Position>;  // 合法手(可能手)

    static constexpr Coordinate ColumnSize {7};  // 列数(横方向)
    static constexpr Coordinate MaxHeight  {6};  // 最大の高さ
    static constexpr Coordinate FullWidth  {8};  // 余白込みの盤面の幅
    static constexpr Coordinate FullHeight {8};  // 余白込みの盤面の高さ
    static constexpr Coordinate FullSize = FullWidth * FullHeight;  // 余白込みの盤面のマス数
    using Cells = std::bitset<FullSize>;  // 全マス

    // 上下と左右をつなげないために、上と右に余白を必ず設ける
    static_assert(ColumnSize < FullWidth);
    static_assert(MaxHeight < FullHeight);

    static constexpr Coordinate LineTypes {4};  // 縦横斜めの線種の数
    static constexpr Coordinate MinLen {4};     // 線上のマスが何個並んだら勝ちか
    using HashKey = uint64_t;  // Zobrist hashing のキー
    using HashKeySet = std::array<HashKey, FullSize>;  // プレイヤーとマスごとのキー

private:
    Cells placed_;        // 石を置いたマス
    HashKey digest_ {0};  // 盤面全体のキー

    // メンバ変数のコピーを速くするために、インスタンスごとにハッシュキーを持つのではなく
    // 参照にする。所有権を持たないので、スマートポインタではなく素のポインタにする。
    // 参照にするとコピー不可になるので、盤面を複製できない。
    // staticにするとマルチスレッド化が難しい
    const HashKeySet* hashkeys_;  // それぞれのマスのキー

    // メンバ変数のコピーを速くするためにSingletonにする
    // こうするとインスタンスが24 byteになる
    // 設定したら変更しない値は、read-onlyなのでマルチスレッド化で問題にならない
    static inline Cells mask_;  // 余白を取り除くマスク
    static inline std::array<Cells, LineTypes> linemasks_;  // 縦横斜めの線それぞれのマスク

public:
    explicit Board(const HashKeySet& hashkeys) : hashkeys_(&hashkeys) {
        // 石が置ける位置をマスクする
        for(Coordinate column{0}; column < ColumnSize; ++column) {
            for(Coordinate height{0}; height < MaxHeight; ++height) {
                const auto index = to_index(column, height);
                mask_.set(index);
            }
        }

        // 初めて使うときに初期化する
        for(auto&& mask : linemasks_) {
            if (mask.count() == 0) {
                setup();
                break;
            }
        }
    }

    ~Board() = default;

    static void setup() {
        // 石が置ける位置をマスクする
        for(Coordinate column{0}; column < ColumnSize; ++column) {
            for(Coordinate height{0}; height < MaxHeight; ++height) {
                const auto index = to_index(column, height);
                mask_.set(index);
            }
        }

        // 線種ごとにマスクを生成する
        for(Coordinate i{0}; i < MinLen; ++i) {
            // 縦一列
            linemasks_.at(0).set(i);
            // 横一列
            linemasks_.at(1).set(i * FullHeight);
            // 左下から右上
            linemasks_.at(2).set(i * (FullHeight + 1));
            // 右上から左下。この線だけ原点が(0,0)ではない。
            linemasks_.at(3).set(MinLen - 1 + i * (FullHeight - 1));
        }
    }

    // 盤面全体のダイジェストを返す
    HashKey digest() const {
        return digest_;
    }

    // 石を指定位置に置く。置ける場所は空のマスで範囲内。
    void place(Board::Coordinate column, Board::Coordinate height) {
        const auto index = to_index(column, height);
        if (!placed_[index] && mask_[index]) {
            placed_.set(index);
            digest_ ^= hashkeys_->at(index);
        }
    }

    // 石を指定から除く。置く場所に石が必要である。
    void remove(Board::Coordinate column, Board::Coordinate height) {
        const auto index = to_index(column, height);
        if (placed_[index]) {
            placed_.reset(index);
            digest_ ^= hashkeys_->at(index);
        }
    }

    // 盤面を合成する。同じマスに複数の石が無いことが前提である。
    // ハッシュキーはthisの方が残る
    Board merge(const auto& rhs) const {
        auto retval = *this;
        retval.placed_ ^= rhs.placed_;
        retval.digest_ ^= rhs.digest_;
        return retval;
    }

    // 座標をビットボードの添え字に変換する
    static Coordinate to_index(Coordinate column, Coordinate height) {
        return column * FullHeight + height;
    }

    // ビットボードを読める文字列に変換する
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

    // 盤面を読める文字列に変換する
    std::string to_string(char blank, char mark) const {
        return to_string(placed_, blank, mark);
    }

    // 指定された位置から線が始まるかどうか調べる
    bool check_line(Coordinate line_index, Coordinate left_shift) const {
        if (left_shift < 0) {
            return false;
        }

        // 線のビットマスクを、線の開始位置までシフトする。多すぎるマスは捨てる。
        auto line = linemasks_.at(line_index);
        line <<= left_shift;
        line &= mask_;
        line &= placed_;

        // 余白を数えないので、上下と左右がループしていたらマスが足りなくなる。
        return (line.count() >= MinLen);
    }

    // 指定された範囲[column, bottom..top]から垂直線が始まるかどうか調べる
    bool check_vertical_line(Coordinate bottom, Coordinate top, Coordinate column) const {
        for(Coordinate y {bottom}; y <= top; ++y) {
            if (check_line(0, to_index(column, y))) {
                return true;
            }
        }

        return false;
    }

    // 指定された範囲から[left..right, height]水平線が始まるかどうか調べる
    bool check_horizontal_line(Coordinate left, Coordinate right, Coordinate height) const {
        for(Coordinate x {left}; x <= right; ++x) {
            if (check_line(1, to_index(x, height))) {
                return true;
            }
        }

        return false;
    }

    // 指定された範囲から左上に向かって線が始まるかどうか調べる
    // 座標は[column + offset, height + offset] ただし offset = left-column..right-column
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

    // 指定された範囲から左下に向かって線が始まるかどうか調べる
    // 座標は[column + offset, height - offset] ただし offset = left-column..right-column
    bool check_line3(Coordinate left, Coordinate right, Coordinate column, Coordinate height) const {
        for(Coordinate x {left}; x <= right; ++x) {
            const auto offset = x - column;
            const auto y = height - offset;
            if ((y >= 0) && (y < MaxHeight)) {
                // 原点は(0,0)ではなく(0, MinLen - 1)
                const auto left_shift = to_index(x, y) - (MinLen - 1);
                if (check_line(3, left_shift)) {
                    return true;
                }
            }
        }

        return false;
    }

    // 今打ったマスに線が完成したかどうか調べる
    bool check(Coordinate column, Coordinate height) const {
        // 右と上を無駄に調べない
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

    // 合法手を列挙する
    // Count leading zeros を使っても速くならない
    Positions legal_actions() const {
        Positions positions;
        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
                auto index = to_index(column, height);
                if (!placed_[index]) {
                    positions.push_back(Position{column, height});
                    break;
                }
            }
        }

        return positions;
    }

    // 置ける場所が全て埋まっている
    bool full() const {
        return placed_.count() == mask_.count();
    }
};

// 固定ハッシュキー
class CommonHashKey final {
    FRIEND_TEST(TestCommonHashKey, Initialize);
private:
    std::vector<Board::HashKeySet> hashkeys_;  // それぞれのマスのキー(一人分)

public:
    explicit CommonHashKey(Player n_players) {
        const Board::HashKey upper = std::numeric_limits<Board::HashKey>::max() - 1;
        const Board::HashKey lower = 1;
        std::random_device seed_gen;
        std::mt19937 engine(seed_gen());
        std::uniform_int_distribution<Board::HashKey> dist(lower, upper);

        std::vector<Board::HashKeySet> hashkeys(n_players);

        for(decltype(n_players) i{0}; i<n_players; ++i) {
            // 非0の乱数をキーにする。乱数はプレイヤーごとに異なる。
            for(auto&& key : hashkeys.at(i)) {
                key = dist(engine);
            }
        }

        std::swap(hashkeys_, hashkeys);
    }

    ~CommonHashKey() = default;

    // 固定ハッシュキー一人分を返す
    // 参照を返すので、このオブジェクトの寿命を超えて使うと
    // dangling reference になる
    const auto& hashkeys(Player player) const {
        return hashkeys_.at(player);
    }
};

// 局面
class Stage final {
    FRIEND_TEST(TestBoard, Initialize);
    FRIEND_TEST(TestStage, Initialize);
    FRIEND_TEST(TestStage, Player);
    FRIEND_TEST(TestStage, ToStringInitial);
    FRIEND_TEST(TestStage, ToStringFull);
    FRIEND_TEST(TestStage, MergeBoards);
    FRIEND_TEST(TestStage, Digest);
    FRIEND_TEST(TestStage, Full);
    FRIEND_TEST(TestStage, LegalActions);
    FRIEND_TEST(TestStage, Advance1);
    FRIEND_TEST(TestStage, Advance2);
    FRIEND_TEST(TestStage, Draw);
    FRIEND_TEST(TestStage, WinAtLastMove);
    FRIEND_TEST(TestNode, Stage);
    FRIEND_TEST(TestNodeSet, Add);
public:
    static constexpr Player SizeOfPlayers {2};  // プレイヤーの数
    static inline const char Blank {'.'};       // 何も置いてないマスの記号
    static inline const std::string Marks {"+-"};  // それぞれのプレイヤーが置いたマスの記号

    // 手を打った結果
    enum class Result {
        Invalid,  // 無効な手を打った
        Won,      // 有効な手を打って勝った
        Draw,     // 有効な手を打って引き分けた
        Placed,   // 有効な手を打ち、勝敗はまだ決まっていない
    };

private:
    std::array<Board, SizeOfPlayers> boards_;  // 先手と後手の盤面
    Board  merged_board_;  // 先手番と後手番を統合した盤面
    Player player_ {0};    // 先手番なら0, 後手番なら1

public:
    explicit Stage(const CommonHashKey& keys) :
        boards_({Board(keys.hashkeys(0)), Board(keys.hashkeys(1))}),
        merged_board_(keys.hashkeys(0)) {
        merge_boards();
    }

    ~Stage() = default;

    // 現在のプレイヤーを取得する
    Player player() const {
        return player_;
    }

    // 文字列表記を返す
    std::string to_string() const {
        std::string str_full;
        for(Player player {0}; player < SizeOfPlayers; ++player) {
            const auto str_board = boards_.at(player).to_string(Blank, Marks.at(player));
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

    // 先手番と後手番を統合する
    void merge_boards() {
        merged_board_ = boards_.at(player_).merge(boards_.at(1 - player_));
    }

    // 局面を表すダイジェスト
    Board::HashKey digest() const {
        return merged_board_.digest();
    }

    // 打てる場所が無ければtrue、あればfalse
    bool full() const {
        return merged_board_.full();
    }

    // 合法手を列挙する
    Board::Positions legal_actions() const {
        return merged_board_.legal_actions();
    }

    // 指定した場所に一手打つ
    // 勝負がついていなければ打ち手を逆にする
    Result advance(Board::Coordinate column, Board::Coordinate height) {
        const auto positions = legal_actions();
        if (positions.empty()) {
            return Result::Draw;
        }

        for(const auto& [c, h] : positions) {
            if ((column == c) && (height == h)) {
                // 合法手かどうかは呼び出し先では確認しない
                boards_.at(player_).place(column, height);
                // 統合した盤面は後で使うので、適切に更新する
                merge_boards();

                if (boards_.at(player_).check(column, height)) {
                    return Result::Won;
                }

                if (full()) {
                    return Result::Draw;
                }

                player_ ^= 1;
                return Result::Placed;
            }
        }

        // 打てる手が無かった
        return Result::Invalid;
    }
};

// 局面を納めるノード
class Node final {
    FRIEND_TEST(TestNode, Stage);
    FRIEND_TEST(TestNode, Tried);
    FRIEND_TEST(TestNode, FirstPlayerWon);
    FRIEND_TEST(TestNode, SecondPlayerWon);
    FRIEND_TEST(TestNode, Expanded);
    FRIEND_TEST(TestNode, AddParent);
    FRIEND_TEST(TestNode, AddChild);
public:
    using Count = long long int;  // 試行回数

private:
    Stage stage_;  // 局面
    Count n_tried_ {0};              // このノードを探索した回数
    Count n_first_player_won_ {0};   // 先手が勝った回数
    Count n_second_player_won_ {0};  // 後手が勝った回数
    bool expanded_ {false};  // 子ノードを展開済

    // 一手ごとに石が増えるので、親子関係が循環することはあり得ない
    // ノードの所有権はNodeSetが持っているので、ここでは生ポインタにする
    // そうしないと親子ノードでshared_ptrが循環参照してオブジェクトを解放できなくなる
    std::vector<Node*> parents_;   // 親ノード一覧
    std::vector<Node*> children_;  // 子ノード一覧

public:
    explicit Node(const Stage& stage) : stage_(stage), expanded_{false} {}

    ~Node() = default;

    // 局面を返す。参照なのでコピーしない。
    const Stage& stage() const {
        return stage_;
    }

    // このノードを探索した回数を取得する
    Count n_tried() const {
        return n_tried_;
    }

    // このノードを探索した回数を一回増やす
    void increment_n_tried() {
        ++n_tried_;
    }

    // 先手が勝った回数を取得する
    Count n_first_player_won() const {
        return n_first_player_won_;
    }

    // 先手が勝った回数を一回増やす
    void increment_n_first_player_won() {
        ++n_first_player_won_;
    }

    // 後手が勝った回数を取得する
    Count n_second_player_won() const {
        return n_second_player_won_;
    }

    // 後手が勝った回数を一回増やす
    void increment_n_second_player_won() {
        ++n_second_player_won_;
    }

    // 子ノードを展開済かどうか返す
    bool expanded() const {
        return expanded_;
    }

    // 子ノードを展開済にする
    void set_expanded() {
        expanded_ = true;
    }

    // このノードのダイジェストを取得する
    Board::HashKey digest() const {
        return stage_.digest();
    }

    // 親ノードを追加する
    void add_parent(Node* node) {
        parents_.push_back(node);
    }

    // 親ノード一覧を取得する
    const std::vector<Node*>& parents() const {
        return parents_;
    }

    // 子ノードを追加する
    void add_child(Node* node) {
        children_.push_back(node);
    }

    // 子ノード一覧を取得する
    const std::vector<Node*>& children() const {
        return children_;
    }
};

// 重複するノードを持たない集合
class NodeSet final {
public:
    std::map<Board::HashKey, std::unique_ptr<Node>> set_;  // ノードの集合

    NodeSet() = default;
    ~NodeSet() = default;

    // ノードを追加する。既にあるならあったものを返す。
    // いずれにせよ呼び出し元のスマートポインタは空になる
    Node* add(std::unique_ptr<Node>& node) {
        const auto digest = node->digest();
        if (set_.find(digest) == set_.end()) {
            set_[digest] = std::move(node);
        } else {
            node.reset();
        }
        return set_[digest].get();
    }

    // 盤面からノードを探す
    Node* find(const Stage& stage) const {
        const auto digest = stage.digest();
        auto it = set_.find(digest);
        return (it != set_.end()) ? (it->second).get() : nullptr;
    }
};

// MCTS (Monte Carlo Tree Search)
class MctsEngine final {
    FRIEND_TEST(TestMctsEngine, ExpandRoot);
    FRIEND_TEST(TestMctsEngine, Join);
    FRIEND_TEST(TestMctsEngine, ExpandColumns);
    FRIEND_TEST(TestMctsEngine, Play);
    FRIEND_TEST(TestMctsEngine, Playout);
private:
    static constexpr Node::Count ToExpand {15};  // 何回たどり着いたら展開するか
    static_assert(ToExpand > 0);
    using Depth = Node::Count;  // 探索の深さ
    using Metric = double;      // 評価指標
    static constexpr Metric Epsilon = 1e-8;   // 0除算防止のガード
    static constexpr Metric UcbConst = 1e+2;  // UCB1の定数

    NodeSet nodeset_;  // 探索木のノード
    CommonHashKey hashkeys_;  // 盤面に共通のハッシュキー
    Stage initial_stage_;     // 初期盤面
    Node* root_ {nullptr};    // 根つまり初期局面

    // 乱数生成器
    std::random_device rand_dev;
    std::mt19937 rand_gen;

public:
    // 葉まで探索した結果
    struct Result {
        Stage::Result result;  // 勝ちか引き分け
        Player winner;  // 勝者
        Depth depth;    // 探索の深さ
    };

    MctsEngine() :
        hashkeys_(Stage::SizeOfPlayers), initial_stage_(hashkeys_), rand_gen(rand_dev()) {
        auto node = std::make_unique<Node>(initial_stage_);
        root_ = nodeset_.add(node);
    }

    ~MctsEngine() = default;

    // ハッシュキーをこの探索と共有する初期局面のコピーを返す
    // ハッシュキーを共有しない局面は探しても見つからない
    Stage initial_stage() const {
        return root_->stage();
    }

    // ノードを深くたどり着けるところまで選ぶ
    std::pair<Node*, Node::Count> visit(Node* node, Node::Count depth) {
        if (node->n_tried() < ToExpand) {
            return std::make_pair(node, depth);
        }

        // 親ノードを共有しても展開は一回だけ行う
        if (node->n_tried() >= ToExpand) {
            expand(node);
        }

        auto child = select(node);
        if (child == node) {
            // 子ノードが無かった
            return std::make_pair(node, depth);
        }

        return visit(child, depth + 1);
    }

    // ノードを子から選ぶ
    Node* select(Node* parent) {
        if (parent->children().empty()) {
            // 自身を返す
            return parent;
        }

        auto size = parent->children().size();
        std::vector<Metric> scores(size);

        Node::Count all_tried {0};
        for(const auto& child : parent->children()) {
            const auto n_tried = child->n_first_player_won() + child->n_second_player_won();
            if (n_tried == 0) {
                return child;
            }
            all_tried += n_tried;
        }

        for(decltype(size) i{0}; i<size; ++i) {
            auto& child = parent->children().at(i);
            scores.at(i) = ucb1(parent->stage().player(), all_tried, child);
        }

        const auto index = std::max_element(scores.begin(), scores.end()) - scores.begin();
        return parent->children().at(index);
    }

    // UCB1
    Metric ucb1(Player player, Node::Count all_tried, Node* node) {
        Metric n_tried = node->n_first_player_won() + node->n_second_player_won();
        n_tried += Epsilon;

        // 先手も後手もそれぞれ自分が勝てる子ノードを探索する
        // 親ノードの始点でこのノード(node)を選ぶと勝てるかどうか見ているので、
        // 親ノードのplayerとこのノード(node)のnode->stage_.player_は反対であることに注意する
        // 多くの実装では勝率を反転することで視点切り替えを組み込んでいる
        const Metric n_win = (player == 0) ?
            node->n_first_player_won() : node->n_second_player_won();

        Metric d = 2.0;
        d *= std::log(all_tried);
        d /= n_tried;
        return n_win / n_tried + UcbConst * std::sqrt(d);
    }

    // ノードを一段展開する
    void expand(Node* parent) {
        // 展開済なら何もしない
        if (parent->expanded()) {
            return;
        }

        const auto actions = parent->stage().legal_actions();
        // Early returnしても展開済にする
        parent->set_expanded();

        std::vector<std::unique_ptr<Node>> children;
        for(const auto& action : actions) {
            // 変更するのでコピーする
            Stage next_stage = parent->stage();
            const auto result = next_stage.advance(action.column, action.height);

            if (result == Stage::Result::Invalid) {
                continue;
            }

            auto child = std::make_unique<Node>(next_stage);
            if (result == Stage::Result::Won) {
                // 必勝手以外は子ノードとして登録しない
                auto ptr = nodeset_.add(child);
                parent->add_child(ptr);
                ptr->add_parent(parent);
                return;
            }

            children.push_back(std::move(child));
        }

        for(auto&& child : children) {
            auto ptr = nodeset_.add(child);
            parent->add_child(ptr);
            ptr->add_parent(parent);
        }
    }

    // 指定した局面以降をランダムにプレイする
    Result play(const auto& stage, Node::Count depth) {
        const auto actions = stage.legal_actions();
        if (actions.empty()) {
            return Result{Stage::Result::Draw, stage.player(), depth};
        }

        for(const auto& action : actions) {
            auto next_stage = stage;
            const auto result = next_stage.advance(action.column, action.height);
            if ((result == Stage::Result::Won) || (result == Stage::Result::Invalid)) {
                return Result{result, stage.player(), depth};
            }
        }

        std::uniform_int_distribution<size_t> dist(0, actions.size() - 1);
        const auto action = actions.at(dist(rand_gen));
        auto next_stage = stage;
        next_stage.advance(action.column, action.height);
        return play(next_stage, depth + 1);
    }

    // 木を探索して試行する
    void playout(Node* root_node) {
        auto [top_node, depth] = visit(root_node, 0);
        const auto result = play(top_node->stage(), 0);

        std::set<Node*> visited_nodes;
        std::queue<Node*> nodes;
        nodes.push(top_node);

        while(!nodes.empty()) {
            auto node = nodes.front();
            nodes.pop();

            if (visited_nodes.contains(node)) {
                continue;
            }
            visited_nodes.insert(node);

            node->increment_n_tried();
            if (result.result == Stage::Result::Won) {
                if (result.winner == 0) {
                    node->increment_n_first_player_won();
                } else {
                    node->increment_n_second_player_won();
                }
            }

            for(const auto& parent : node->parents()) {
                nodes.push(parent);
            }
        }
    }

    // 初期盤面からプレイする
    void playout() {
        playout(root_);
    }

    // 指定した局面からプレイする
    // 指定した局面は既に登録されていることが前提である
    void playout(const Stage& stage) {
        auto node_obj = std::make_unique<Node>(stage);
        auto node = nodeset_.add(node_obj);
        playout(node);
    }

    // 対戦中に一手進める
    // ここまでの手番は既に登録されていることが前提である
    void advance(const Stage& stage, const Board::Position& action) {
        auto parent_obj = std::make_unique<Node>(stage);
        auto parent = nodeset_.add(parent_obj);
        auto next_stage = stage;
        const auto result = next_stage.advance(action.column, action.height);

        if (result == Stage::Result::Invalid) {
            return;
        }

        auto child_obj = std::make_unique<Node>(next_stage);
        auto child = nodeset_.add(child_obj);
        parent->add_child(child);
        child->add_parent(parent);
        return;
    }

    // ランダムな手を選ぶ
    std::optional<Board::Position> select_random(const Stage& stage) {
        const auto actions = stage.legal_actions();
        if (actions.empty()) {
            std::optional<Board::Position> zero;
            return zero;
        }

        std::uniform_int_distribution<size_t> dist(0, actions.size() - 1);
        return actions.at(dist(rand_gen));
    }

    // 学習した手を選ぶ
    std::optional<Board::Position> select(const Stage& stage) {
        const auto actions = stage.legal_actions();
        if (actions.empty()) {
            std::optional<Board::Position> zero;
            return zero;
        }

        const auto player = stage.player();
        std::optional<Board::Position> best_action;
        Metric max_value = std::numeric_limits<Metric>::min();

        for(const auto& action : actions) {
            auto next_stage = stage;
            next_stage.advance(action.column, action.height);
            auto node = nodeset_.find(next_stage);
            if (!node) {
                continue;
            }

            const auto n_tried = node->n_first_player_won() + node->n_second_player_won();
            const auto n_win = (player == 0) ?
                node->n_first_player_won() : node->n_second_player_won();

            Metric ratio = n_win;
            ratio /= n_tried;
            if (max_value < ratio) {
                best_action = Board::Position {action.column, action.height};
            }
            max_value = std::max(max_value, ratio);
        }

        if (best_action.has_value()) {
            return best_action;
        }

        return select_random(stage);
    }
};

// 線種とハッシュキー
class TestBoard : public ::testing::Test {};

// 線種以外のメンバ変数を調べる
TEST_F(TestBoard, Initialize) {
    CommonHashKey keys(1);
    Board board(keys.hashkeys(0));

    // できるだけ小さくする
    ASSERT_GE(32, sizeof(board));

    ASSERT_EQ(Board::FullSize, board.placed_.size());
    ASSERT_EQ(Board::FullSize, board.mask_.size());
    auto size = board.mask_.size();

    for(decltype(size) i{0}; i<size; ++i) {
        ASSERT_FALSE(board.placed_[i]);
        const auto expected = ((i / Board::FullHeight) < Board::ColumnSize) &
            ((i % Board::FullHeight) < Board::MaxHeight);
        ASSERT_EQ(expected, board.mask_[i]);
    }

    ASSERT_EQ(0, board.digest_);
}

// 線種を調べる
TEST_F(TestBoard, SetupLines) {
    CommonHashKey keys(1);
    Board board(keys.hashkeys(0));

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

// 盤面全体のダイジェストを返す
TEST_F(TestBoard, Digest) {
    CommonHashKey keys(1);
    Board board(keys.hashkeys(0));

    const std::vector<Board::HashKey> testcase {
        0,
        std::numeric_limits<Board::HashKey>::max() / 2,
        std::numeric_limits<Board::HashKey>::max()
    };

    for(const auto& expected : testcase) {
        board.digest_ = expected;
        ASSERT_EQ(expected, board.digest());
    }
}

// 石を指定位置に置く
TEST_F(TestBoard, Place) {
    CommonHashKey keys(2);
    const auto& keys_full = keys.hashkeys(0);
    const auto& keys_one = keys.hashkeys(1);
    Board full(keys_full);  // 石を一個以上置いた盤面

    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            const auto index = Board::to_index(column, height);
            Board one(keys_one);  // 石を一個だけ置いた盤面

            ASSERT_TRUE(one.hashkeys_);
            ASSERT_TRUE(full.hashkeys_);
            const auto digest_one = one.hashkeys_->at(index);
            const auto digest_old = full.digest_;
            const auto expected_digest = digest_old ^ full.hashkeys_->at(index);
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

// すでに石がある場所に石を置く
TEST_F(TestBoard, PlaceTwice) {
    CommonHashKey keys(1);

    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            const auto index = Board::to_index(column, height);
            Board one(keys.hashkeys(0));

            one.place(column, height);
            const auto expected = one.digest_;
            one.place(column, height);

            ASSERT_TRUE(one.placed_[index]);
            ASSERT_EQ(expected , one.digest_);
            ASSERT_TRUE(one.digest_);
        }
    }
}

// 範囲外に石を置けない
TEST_F(TestBoard, PlaceOutOfBounds) {
    CommonHashKey keys(1);

    for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
        for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
            if ((column < Board::ColumnSize) && (height < Board::MaxHeight)) {
                continue;
            }

            Board one(keys.hashkeys(0));
            const auto index = Board::to_index(column, height);
            one.place(column, height);
            ASSERT_FALSE(one.placed_[index]);
            ASSERT_FALSE(one.digest_);
        }
    }
}

// 石を指定位置から除く
TEST_F(TestBoard, Remove) {
    CommonHashKey keys(2);
    const auto& keys_one = keys.hashkeys(0);
    const auto& keys_full = keys.hashkeys(1);

    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            const auto index = Board::to_index(column, height);
            Board one(keys_one);

            one.remove(column, height);
            ASSERT_FALSE(one.placed_[index]);
            ASSERT_FALSE(one.digest_);

            one.place(column, height);
            one.remove(column, height);

            ASSERT_FALSE(one.placed_[index]);
            ASSERT_FALSE(one.digest_);

            one.remove(column, height);
            ASSERT_FALSE(one.placed_[index]);
            ASSERT_FALSE(one.digest_);
        }
    }

    Board full(keys_full);  // 石を一個以上置いた盤面
    std::vector<Board::HashKey> digests {0};
    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            const auto index = Board::to_index(column, height);
            const auto expected = full.digest_;

            full.remove(column, height);
            ASSERT_FALSE(full.placed_[index]);
            ASSERT_EQ(expected, full.digest_);

            full.place(column, height);
            digests.push_back(full.digest_);
        }
    }

    digests.pop_back();
    for(Board::Coordinate column{Board::ColumnSize - 1}; column >= 0; --column) {
        for(Board::Coordinate height{Board::MaxHeight - 1}; height >= Board::MaxHeight; --height) {
            const auto index = Board::to_index(column, height);
            full.remove(column, height);
            ASSERT_FALSE(full.placed_[index]);
            ASSERT_EQ(digests.back(), full.digest_);

            full.remove(column, height);
            ASSERT_FALSE(full.placed_[index]);
            ASSERT_EQ(digests.back(), full.digest_);

            digests.pop_back();
        }
    }
}

// 範囲外に石は無いので除いても何も起きない
TEST_F(TestBoard, RemoveOutOfBounds) {
    CommonHashKey keys(1);

    for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
        for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
            Board one(keys.hashkeys(0));
            const auto index = Board::to_index(column, height);
            one.place(column, height);
            one.remove(column, height);

            ASSERT_FALSE(one.placed_[index]);
            ASSERT_FALSE(one.digest_);
        }
    }
}

// 盤面を合成する
TEST_F(TestBoard, Merge) {
    CommonHashKey keys(3);
    const auto& keys_zero = keys.hashkeys(0);
    const auto& keys_left = keys.hashkeys(0);
    const auto& keys_right = keys.hashkeys(0);
    Board zero(keys_zero);

    for(Board::Coordinate offset{0}; offset < 2; ++offset) {
        Board lhs(keys_left);
        Board rhs(keys_right);
        size_t expected {0};
        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
                // 互い違いに配置する
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

        // 自分自身と合成すると、石とキーが打ち消し合って0になってしまう
        const auto actual_left_self = lhs.merge(lhs);
        const auto actual_right_self = rhs.merge(rhs);
        ASSERT_EQ(zero.placed_, actual_left_self.placed_);
        ASSERT_EQ(zero.placed_, actual_right_self.placed_);
        ASSERT_EQ(0, actual_left_self.digest_);
        ASSERT_EQ(0, actual_right_self.digest_);

        // 空の盤面と合成するとそのままの値が返る
        const auto zero_left = zero.merge(lhs);
        const auto zero_right = lhs.merge(zero);
        ASSERT_EQ(expected, zero_left.placed_.count());
        ASSERT_EQ(expected, zero_right.placed_.count());
        ASSERT_EQ(lhs.digest_, zero_left.digest_);
        ASSERT_EQ(lhs.digest_, zero_right.digest_);
    }
}

// 座標をビットボードの添え字に変換する
TEST_F(TestBoard, ToIndex) {
    ASSERT_EQ(0, Board::to_index(0, 0));
    ASSERT_EQ(Board::MaxHeight - 1, Board::to_index(0, Board::MaxHeight - 1));
    ASSERT_EQ(Board::FullHeight - 1, Board::to_index(0, Board::FullHeight - 1));
    ASSERT_EQ(Board::FullHeight, Board::to_index(1, 0));
    ASSERT_EQ(Board::FullHeight + Board::MaxHeight - 1, Board::to_index(1, Board::MaxHeight - 1));
    ASSERT_EQ(Board::FullHeight * 2 - 1, Board::to_index(1, Board::FullHeight - 1));
    ASSERT_EQ(Board::FullSize - 1, Board::to_index(Board::FullWidth - 1, Board::FullHeight - 1));
}

// ビットボードを読める文字列に変換する
TEST_F(TestBoard, ToStringCells) {
    const char blank {'.'};
    const char mark {'+'};

    for(Board::Coordinate offset{0}; offset < 2; ++offset) {
        Board::Cells cells;
        std::string lines(Board::ColumnSize * Board::MaxHeight, blank);

        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
                // 互い違いに配置する
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

        ASSERT_EQ(s, Board::to_string(cells, blank, mark));
    }
}

// マスクを読める文字列に変換する
TEST_F(TestBoard, ToStringMask) {
    CommonHashKey keys(1);
    Board board(keys.hashkeys(0));
    const char blank {'.'};
    const char mark {'+'};

    std::string filled_line(Board::ColumnSize, mark);
    filled_line += "\n";

    std::string s;
    for(Board::Coordinate i{0}; i<Board::MaxHeight; ++i) {
        s += filled_line;
    }

    ASSERT_EQ(s, board.to_string(board.mask_, blank, mark));
}

// 盤面を読める文字列に変換する
TEST_F(TestBoard, ToStringPlaced) {
    CommonHashKey keys(1);
    Board board(keys.hashkeys(0));
    const char blank {'.'};
    const char mark {'+'};

    const std::string line(Board::ColumnSize, blank);
    std::string s;
    for(Board::Coordinate i{0}; i<Board::MaxHeight; ++i) {
        s += line;
        s += "\n";
    }
    ASSERT_EQ(s, board.to_string(blank, mark));

    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            const auto pos = column + (Board::MaxHeight - 1 - height) * (Board::ColumnSize + 1);
            s.at(pos) = mark;
            board.place(column, height);
            ASSERT_EQ(s, board.to_string(blank, mark));
        }
    }
}

// 垂直線が引かれているかどうか調べる
TEST_F(TestBoard, CheckVerticalLineAll) {
    CommonHashKey keys(1);
    for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
        Board board(keys.hashkeys(0));

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
    CommonHashKey keys(1);
    constexpr auto top = Board::MaxHeight - Board::MinLen;

    for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
        Board board(keys.hashkeys(0));

        for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
            board.placed_.set(Board::to_index(column, height));
        }

        for(Board::Coordinate x{0}; x < Board::FullWidth; ++x) {
            if ((x == column) & (x < Board::ColumnSize)) {
                EXPECT_FALSE(board.check_vertical_line(top + 1, Board::FullHeight - 1, x));
                EXPECT_TRUE(board.check_vertical_line(0, top, column));
            } else {
                EXPECT_FALSE(board.check_vertical_line(0, Board::FullHeight - 1, x));
            }
        }
    }
}

// 水平線が引かれているかどうか調べる
TEST_F(TestBoard, CheckLineHorizontalAll) {
    CommonHashKey keys(1);
    for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
        Board board(keys.hashkeys(0));

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
    CommonHashKey keys(1);
    constexpr auto right = Board::ColumnSize - Board::MinLen;

    for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
        Board board(keys.hashkeys(0));

        for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
            board.placed_.set(Board::to_index(column, height));
        }

        for(Board::Coordinate y{0}; y < Board::FullHeight; ++y) {
            if ((y == height) & (y < Board::MaxHeight)) {
                EXPECT_FALSE(board.check_horizontal_line(right + 1, Board::FullWidth - 1, y));
                EXPECT_TRUE(board.check_horizontal_line(0, right, height));
            } else {
                EXPECT_FALSE(board.check_horizontal_line(0, Board::FullWidth - 1, y));
            }
        }
    }
}

// 指定された範囲から左上に向かって線が始まるかどうか調べる
TEST_F(TestBoard, CheckLine2All) {
    CommonHashKey keys(1);
    for(Board::Coordinate column{-Board::FullWidth}; column < Board::FullWidth; ++column) {
        Board board(keys.hashkeys(0));

        Board::Coordinate n_placed {0};
        for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
            const auto x = column + height;
            if ((x >= 0) && (x < Board::FullWidth)) {
                board.placed_.set(Board::to_index(x, height));
                n_placed += (x < Board::ColumnSize) & (height < Board::MaxHeight);
            }
        }

        for(Board::Coordinate i{0}; i < board.placed_.size(); ++i) {
            const auto x = i / Board::FullHeight;
            const auto y = i % Board::FullHeight;
            const bool expected = ((x - y) == column) &
                (x <= (Board::ColumnSize - Board::MinLen)) &
                (y <= (Board::MaxHeight - Board::MinLen));

            ASSERT_EQ(expected, board.check_line(2, i));

            if ((x < Board::ColumnSize) && (y < Board::MaxHeight)) {
                const bool expected_line = ((x - y) == column) & (n_placed >= Board::MinLen);
                ASSERT_EQ(expected_line, board.check_line2(0, x, x, y));
            }
        }
    }
}

// 指定された範囲から左下に向かって線が始まるかどうか調べる
TEST_F(TestBoard, CheckLine3All) {
    CommonHashKey keys(1);
    for(Board::Coordinate column{-Board::FullWidth}; column < Board::FullWidth; ++column) {
        Board board(keys.hashkeys(0));

        Board::Coordinate n_placed {0};
        for(Board::Coordinate height{Board::FullHeight - 1}; height >= 0; --height) {
            const auto x = column + Board::FullHeight - 1 - height;
            if ((x >= 0) && (x < Board::FullWidth)) {
                board.placed_.set(Board::to_index(x, height));
                n_placed += (x < Board::ColumnSize) & (height < Board::MaxHeight);
            }
        }

        for(Board::Coordinate i{0}; i < board.placed_.size(); ++i) {
            const auto x = i / Board::FullHeight;
            const auto y = i % Board::FullHeight;
            const auto total = std::min(Board::ColumnSize - x, y + 1);
            const auto xy_sum = column + Board::FullHeight - 1;
            const bool expected = ((x + y) == xy_sum) &
                (x < Board::ColumnSize) & (y < Board::MaxHeight) & (total >= Board::MinLen);
            EXPECT_EQ(expected, board.check_line(3, i - (Board::MinLen - 1)));

            if ((x < Board::ColumnSize) && (y < Board::MaxHeight)) {
                const bool expected_line = ((x + y) == xy_sum) & (n_placed >= Board::MinLen);
                ASSERT_EQ(expected_line, board.check_line3(0, x, x, y));
            }
        }
    }
}

// 角だけ調べる
TEST_F(TestBoard, CheckLineCorners) {
    using Coord = Board::Coordinate;
    // 線の始点X,Y, 線の方向(差分) DX,DY
    const std::vector<std::tuple<Coord, Coord, Coord, Coord>> cases {
        {0, 0, 1, 0}, {0, 0, 0, 1}, {0, 0, 1, 1},
        {0, Board::MaxHeight - 1, 1, 0}, {0, Board::MaxHeight - 1, 0, -1}, {0, Board::MaxHeight - 1, 1, -1},
        {Board::ColumnSize - 1, 0, -1, 0}, {Board::ColumnSize - 1, 0, 0, 1}, {Board::ColumnSize - 1, 0, -1, 1},
        {Board::ColumnSize - 1, Board::MaxHeight - 1, -1, 0},
        {Board::ColumnSize - 1, Board::MaxHeight - 1, 0, -1},
        {Board::ColumnSize - 1, Board::MaxHeight - 1, -1, -1}
    };

    CommonHashKey keys(1);
    for(const auto& [sx, sy, dx, dy] : cases) {
        std::set<std::pair<Coord, Coord>> ps;
        Board board(keys.hashkeys(0));
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

// すべての起点のすべての線
TEST_F(TestBoard, CheckLineAll) {
    using Coord = Board::Coordinate;
    const std::vector<std::tuple<Coord, Coord>> dxys {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    CommonHashKey keys(1);
    for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
        for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
            for(const auto& [dx, dy] : dxys) {
                Board board(keys.hashkeys(0));
                std::set<std::pair<Coord, Coord>> ps;
                auto x = column;
                auto y = height;

                Coord total {0};
                for(Board::Coordinate len{0}; len < Board::MinLen; ++len) {
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

// 上下左右がつながっている場合(余白があればつながらない)
TEST_F(TestBoard, CheckLoop) {
    using Coord = Board::Coordinate;
    const std::vector<std::tuple<Coord, Coord>> dxys {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    CommonHashKey keys(1);
    for(Board::Coordinate column{0}; column < Board::FullWidth; ++column) {
        for(Board::Coordinate height{0}; height < Board::FullHeight; ++height) {
            for(const auto& [dx, dy] : dxys) {
                Board board(keys.hashkeys(0));
                std::set<std::pair<Coord, Coord>> ps;
                auto x = column;
                auto y = height;

                Coord total {0};
                for(Board::Coordinate len{0}; len < Board::MinLen; ++len) {
                    if ((x >= 0) && (x < Board::ColumnSize) && (y >= 0) && (y < Board::MaxHeight)) {
                        ++total;
                    }

                    board.placed_.set(Board::to_index(x, y));
                    ps.insert(std::make_pair(x, y));
                    x = (x + dx + Board::FullWidth) % Board::FullWidth;
                    y = (y + dy + Board::FullHeight) % Board::FullHeight;
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

// 初手の合法手
TEST_F(TestBoard, LegalActionsEmpty) {
    CommonHashKey keys(1);
    Board board(keys.hashkeys(0));

    const auto actual = board.legal_actions();
    ASSERT_EQ(Board::ColumnSize, actual.size());

    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        ASSERT_EQ(column, actual.at(column).column);
        ASSERT_EQ(0, actual.at(column).height);
    }
}

// 全部のマスが埋まっているときの合法手
TEST_F(TestBoard, LegalActionsFull) {
    CommonHashKey keys(1);
    Board board(keys.hashkeys(0));

    for(size_t i{0}; i<board.placed_.size(); ++i) {
        board.placed_.set(i);
    }

    const auto actual = board.legal_actions();
    ASSERT_TRUE(actual.empty());
}

// 一番上のマスの上以外は置ける
TEST_F(TestBoard, LegalActionsHeight) {
    CommonHashKey keys(1);

    for(Board::Coordinate i{0}; i < (Board::MaxHeight - 1); ++i) {
        const auto h = Board::MaxHeight - 1 - i;
        std::vector<Board::Coordinate> expected {i, h, i, h, i, h, i};
        ASSERT_EQ(Board::ColumnSize, expected.size());

        Board board(keys.hashkeys(0));
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

// 一番上のマスの上には置けない
TEST_F(TestBoard, LegalActionsMax) {
    CommonHashKey keys(1);
    std::vector<Board::Coordinate> expected(Board::ColumnSize, 0);
    std::iota(expected.begin(), expected.end(), 0);

    for(Board::Coordinate i{0}; i < Board::MaxHeight; ++i) {
        Board board(keys.hashkeys(0));
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

        // 全部の列を一段上げる
        for(auto&& x : expected) {
            x = std::min(x + 1, Board::MaxHeight - 1);
        }
    }
}

// 置ける場所が全て埋まっている
TEST_F(TestBoard, Full) {
    CommonHashKey keys(1);
    Board board(keys.hashkeys(0));
    Board::Coordinate total {0};
    ASSERT_FALSE(board.full());

    for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
        for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
            board.placed_.set(Board::to_index(column, height));
            ++total;
            const auto expected = (total == (Board::MaxHeight * Board::ColumnSize));
            ASSERT_EQ(expected, board.full());
        }
    }
}

class TestCommonHashKey : public ::testing::Test {};

// ハッシュキー
TEST_F(TestCommonHashKey, Initialize) {
    for(Player n_players {1}; n_players <= 2; ++n_players) {
        CommonHashKey keys(n_players);
        ASSERT_EQ(n_players, keys.hashkeys_.size());

        for(Player i{0}; i < n_players; ++i) {
            const auto& hashkeys = keys.hashkeys_.at(i);
            ASSERT_EQ(Board::FullSize, hashkeys.size());

            std::set<Board::HashKey> unique_keys;
            for(const auto& key: hashkeys) {
                ASSERT_NE(0, key);
                ASSERT_FALSE(unique_keys.contains(key));
            }

            const auto actual = keys.hashkeys(i);
            ASSERT_EQ(keys.hashkeys_.at(i), actual);
        }
    }
}

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

// メンバ変数を調べる
TEST_F(TestStage, Initialize) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    ASSERT_EQ(0, stage.player_);

    ASSERT_TRUE(stage.merged_board_.hashkeys_);
    for(const auto& key : *(stage.merged_board_.hashkeys_)) {
        ASSERT_TRUE(key);
    }
}

// 現在のプレイヤーを取得する
TEST_F(TestStage, Player) {
    for(Player player{0}; player < Stage::SizeOfPlayers; ++player) {
        CommonHashKey keys(Stage::SizeOfPlayers);
        Stage stage(keys);
        stage.player_ = player;
        ASSERT_EQ(player, stage.player());
    }
}

// 初期状態を文字列にする
TEST_F(TestStage, ToStringInitial) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    const auto expected = get_blank_lines();
    const auto actual = stage.to_string();
    ASSERT_EQ(expected, actual);
}

// 局面を文字列にする
TEST_F(TestStage, ToStringFull) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage_0(keys);
    Stage stage_1(keys);
    auto expected_0 = get_blank_lines();
    auto expected_1 = get_blank_lines();

    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            // 左右対称に打つ。ここでは合法手とは限らない。
            const auto index_left = Board::to_index(column, height);
            const auto column_r = Board::ColumnSize - column - 1;
            const auto height_r = Board::MaxHeight - height - 1;
            if ((column == column_r) && (height == height_r)) {
                continue;
            }

            Stage stage_two(keys);
            stage_two.boards_.at(0).place(column, height);
            stage_two.boards_.at(1).place(column_r, height_r);
            stage_0.boards_.at(0).place(column, height);
            stage_1.boards_.at(1).place(column_r, height_r);

            // 打ったうちに対応して出力文字列を変える
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

TEST_F(TestStage, MergeBoards) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    constexpr Board::Coordinate index0 {2};
    constexpr Board::Coordinate index1 {5};
    ASSERT_FALSE(stage.merged_board_.placed_[index0]);
    ASSERT_FALSE(stage.merged_board_.placed_[index1]);

    stage.boards_.at(0).placed_.set(index0);
    stage.merge_boards();
    ASSERT_EQ(stage.boards_.at(0).hashkeys_, stage.merged_board_.hashkeys_);
    ASSERT_TRUE(stage.merged_board_.placed_[index0]);
    ASSERT_FALSE(stage.merged_board_.placed_[index1]);

    stage.player_ = 1;
    stage.boards_.at(1).placed_.set(index1);
    stage.merge_boards();
    ASSERT_EQ(stage.boards_.at(1).hashkeys_, stage.merged_board_.hashkeys_);
    ASSERT_TRUE(stage.merged_board_.placed_[index0]);
    ASSERT_TRUE(stage.merged_board_.placed_[index1]);
}

TEST_F(TestStage, Digest) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    constexpr Board::Coordinate index0 {3};
    constexpr Board::Coordinate index1 {7};

    stage.boards_.at(0).placed_.set(index0);
    auto expected = stage.boards_.at(0).merge(stage.boards_.at(1)).digest();
    stage.merge_boards();
    ASSERT_EQ(expected, stage.merged_board_.digest());

    stage.boards_.at(1).placed_.set(index1);
    expected = stage.boards_.at(0).merge(stage.boards_.at(1)).digest();
    stage.merge_boards();
    ASSERT_EQ(expected, stage.merged_board_.digest());
}

TEST_F(TestStage, Full) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);

    // 本来あり得ない盤面だが、テストケースなのでよしとする
    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            const auto index = Board::to_index(column, height);
            if ((height & 1) == 0) {
                stage.boards_.at(0).placed_.set(index);
            } else {
                stage.boards_.at(1).placed_.set(index);
            }

            stage.merge_boards();
            const auto expected = (((column + 1) == Board::ColumnSize) && ((height + 1) == Board::MaxHeight));
            ASSERT_EQ(expected, stage.full());
        }
    }
}

// 一手ずつ交互に合法手を打つ
TEST_F(TestStage, LegalActions) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    auto actual = stage.legal_actions();
    ASSERT_EQ(Board::ColumnSize, actual.size());

    for(Board::Coordinate i{0}; i < Board::ColumnSize; ++i) {
        ASSERT_EQ(i, actual.at(i).column);
    }

    for(Board::Coordinate i{0}; i < Board::MaxHeight; ++i) {
        stage.advance(0, i);
    }

    actual = stage.legal_actions();
    ASSERT_EQ(Board::ColumnSize - 1, actual.size());

    for(Board::Coordinate i{0}; i < (Board::ColumnSize - 1); ++i) {
        ASSERT_EQ(i+1, actual.at(i).column);
    }
}

// 一手ずつ交互に合法手を打つ
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

    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    decltype(stage.player_) player {0};
    for(const auto& step : steps) {
        const auto actual = stage.advance(step.column, step.height);
        ASSERT_EQ(step.expected, actual);

        if (step.expected == Stage::Result::Placed) {
            player = (player == 0) ? 1 : 0;
        }
        ASSERT_EQ(player, stage.player_);
    }

    ASSERT_EQ(0, stage.player_);
}

// 一手ずつ交互に合法手を打つ
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

    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    decltype(stage.player_) player {0};
    for(const auto& step : steps) {
        const auto actual = stage.advance(step.column, step.height);
        ASSERT_EQ(step.expected, actual);

        if (step.expected == Stage::Result::Placed) {
            player = (player == 0) ? 1 : 0;
        }
        ASSERT_EQ(player, stage.player_);
    }

    ASSERT_EQ(1, stage.player_);
}

// 盤面が埋まったら引き分け
TEST_F(TestStage, Draw) {
    constexpr auto size = Board::MaxHeight * Board::ColumnSize;
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    decltype(stage.player_) turn {0};

    const auto half_x = (Board::ColumnSize / 2);
    for(Board::Coordinate column{0}; column < Board::ColumnSize; ++column) {
        if (column == half_x) {
            continue;
        }

        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            if (((column + 1) != Board::ColumnSize) || ((height + 1) != Board::MaxHeight)) {
                const auto actual = stage.advance(column, height);
                ++turn;
                ASSERT_EQ(Stage::Result::Placed, actual);
                ASSERT_EQ(turn & 1, stage.player_);
            }
        }
    }

    for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
        const auto actual = stage.advance(half_x, height);
        ++turn;
        ASSERT_EQ(Stage::Result::Placed, actual);
        ASSERT_EQ(turn & 1, stage.player_);
    }

    // 最後の一手で引き分けが決まる
    auto actual = stage.advance(Board::ColumnSize - 1, Board::MaxHeight - 1);
    ASSERT_EQ(Stage::Result::Draw, actual);
    ASSERT_EQ(1, stage.player_);

    // どこに打っても引き分ける
    actual = stage.advance(0, 0);
    ASSERT_EQ(Stage::Result::Draw, actual);
    ASSERT_EQ(1, stage.player_);
}

// 最後の一手で勝ち
TEST_F(TestStage, WinAtLastMove) {
    // 本来あり得ない盤面だが、テストケースなのでよしとする
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    const auto size = Board::FullHeight * (Board::ColumnSize - 1) + Board::MaxHeight;

    for(Board::Coordinate i{0}; i < (size - 4); ++i) {
        if ((i % Board::FullHeight) < Board::MaxHeight) {
            stage.boards_.at(1).placed_.set(i);
        }
    }

    for(Board::Coordinate i{size-4}; i < (size-1); ++i) {
        stage.boards_.at(0).placed_.set(i);
    }
    stage.merge_boards();

    // 最後の一手で勝つ
    auto actual = stage.advance(Board::ColumnSize - 1, Board::MaxHeight - 1);
    ASSERT_EQ(Stage::Result::Won, actual);
    ASSERT_EQ(0, stage.player_);

    // どこに打っても引き分ける
    actual = stage.advance(Board::ColumnSize - 1, Board::MaxHeight - 1);
    ASSERT_EQ(Stage::Result::Draw, actual);
    ASSERT_EQ(0, stage.player_);
}

class TestNode : public ::testing::Test {};

// 局面
TEST_F(TestNode, Stage) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    Node node(stage);
    const Stage& actual = node.stage();
    ASSERT_EQ(&node.stage_, &actual);
    ASSERT_EQ(node.stage_.merged_board_.hashkeys_, actual.merged_board_.hashkeys_);
}

// このノードを探索した回数
TEST_F(TestNode, Tried) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    Node node(stage);

    ASSERT_EQ(0, node.n_tried());
    node.increment_n_tried();
    ASSERT_EQ(1, node.n_tried());
    ASSERT_FALSE(node.n_first_player_won());
    ASSERT_FALSE(node.n_second_player_won());
}

// 先手が勝った回数
TEST_F(TestNode, FirstPlayerWon) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    Node node(stage);

    ASSERT_EQ(0, node.n_first_player_won());
    node.increment_n_first_player_won();
    ASSERT_EQ(1, node.n_first_player_won());
    ASSERT_FALSE(node.n_tried());
    ASSERT_FALSE(node.n_second_player_won());
}

// 後手が勝った回数
TEST_F(TestNode, SecondPlayerWon) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    Node node(stage);

    ASSERT_EQ(0, node.n_second_player_won());
    node.increment_n_second_player_won();
    ASSERT_EQ(1, node.n_second_player_won());
    ASSERT_FALSE(node.n_tried());
    ASSERT_FALSE(node.n_first_player_won());
}

// 子ノードを展開済かどうか
TEST_F(TestNode, Expanded) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    Node node(stage);

    ASSERT_FALSE(node.expanded());
    node.set_expanded();
    ASSERT_TRUE(node.expanded());
}

// 初期状態から順に追加する
TEST_F(TestNode, Digest) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    Node node(stage);

    const auto expected = stage.digest();
    const auto actual = node.digest();
    ASSERT_EQ(expected, actual);

    // 先手
    stage.advance(0, 0);
    Node node1(stage);
    const auto expected1 = stage.digest();
    ASSERT_NE(expected1, expected);
    const auto actual1 = node1.digest();
    ASSERT_EQ(expected1, actual1);

    // 後手
    stage.advance(1, 0);
    Node node2(stage);
    const auto expected2 = stage.digest();
    ASSERT_NE(expected2, expected);
    ASSERT_NE(expected2, expected1);
    const auto actual2 = node2.digest();
    ASSERT_EQ(expected2, actual2);
}

// 親ノードを追加する
TEST_F(TestNode, AddParent) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    Node  node(stage);
    using Size = decltype(node.parents_)::size_type;

    for(Size i{1}; i<=3; ++i) {
        auto parent = std::make_unique<Node>(stage);
        node.add_parent(parent.get());
        ASSERT_EQ(i, node.parents_.size());
        ASSERT_EQ(parent.get(), node.parents_.at(i-1));

        const auto& actual = node.parents();
        ASSERT_EQ(&actual, &node.parents_);
        ASSERT_EQ(actual, node.parents_);
    };

    ASSERT_FALSE(node.children_.size());
}

// 子ノードを追加する
TEST_F(TestNode, AddChild) {
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);
    Node  node(stage);
    using Size = decltype(node.children_)::size_type;

    for(Size i{1}; i<=3; ++i) {
        auto child = std::make_unique<Node>(stage);
        node.add_child(child.get());
        ASSERT_EQ(i, node.children_.size());
        ASSERT_EQ(child.get(), node.children_.at(i-1));

        const auto& actual = node.children();
        ASSERT_EQ(&actual, &node.children_);
        ASSERT_EQ(actual, node.children_);
    };

    ASSERT_FALSE(node.parents_.size());
}

class TestNodeSet : public ::testing::Test {};

// 初期状態から順に追加する
TEST_F(TestNodeSet, Add) {
    NodeSet nodeset;
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);

    auto node0 = std::make_unique<Node>(stage);
    auto ptr0 = node0.get();
    auto actual = nodeset.add(node0);
    ASSERT_FALSE(node0);
    ASSERT_EQ(ptr0, actual);
    ASSERT_EQ(1, nodeset.set_.size());

    // 二度追加すると最初の物が得られる
    auto node0_alt = std::make_unique<Node>(stage);
    auto node02 = std::make_unique<Node>(stage);
    actual = nodeset.add(node0_alt);
    ASSERT_FALSE(node0_alt);
    ASSERT_EQ(ptr0, actual);
    ASSERT_EQ(1, nodeset.set_.size());

    // 初手
    stage.advance(0, 0);
    auto node1 = std::make_unique<Node>(stage);
    auto node12 = std::make_unique<Node>(stage);
    auto ptr1 = node1.get();
    actual = nodeset.add(node1);
    ASSERT_FALSE(node1);
    ASSERT_EQ(ptr1, actual);
    ASSERT_EQ(2, nodeset.set_.size());

    auto node1_ptr = std::make_unique<Node>(stage);
    actual = nodeset.add(node1_ptr);
    ASSERT_EQ(ptr1, actual);
    ASSERT_EQ(2, nodeset.set_.size());

    // 二番手
    stage.advance(0, 1);
    auto node2 = std::make_unique<Node>(stage);
    auto ptr2 = node2.get();
    actual = nodeset.add(node2);
    ASSERT_EQ(ptr2, actual);
    ASSERT_EQ(3, nodeset.set_.size());

    // Stageはノード間で共有しないので、初期状態も初手も残っている
    ASSERT_EQ(ptr0, nodeset.add(node02));
    ASSERT_EQ(0, ptr0->stage().merged_board_.placed_.count());
    ASSERT_EQ(ptr1, nodeset.add(node12));
    ASSERT_EQ(1, ptr1->stage().merged_board_.placed_.count());
    ASSERT_EQ(2, ptr2->stage().merged_board_.placed_.count());
}

// 盤面からノードを探す
TEST_F(TestNodeSet, Find) {
    NodeSet nodeset;
    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);

    auto node0 = std::make_unique<Node>(stage);
    auto ptr0 = node0.get();
    auto actual = nodeset.add(node0);
    ASSERT_EQ(ptr0, actual);
    ASSERT_EQ(ptr0, nodeset.find(stage));

    stage.advance(0, 0);
    ASSERT_FALSE(nodeset.find(stage));
}

// MCTS (Monte Carlo Tree Search)
class TestMctsEngine : public ::testing::Test {
protected:
    // プレイヤーの戦略
    enum class Strategy {
        Random,      // 合法手からランダムに選ぶ
        OnlineMcts,  // 対戦中もMCTSで最善手を探す
    };

    // 対戦結果(どのプレイヤーが、どの戦略で、何回勝ったか
    using ResultMatrix = std::array<std::array<Node::Count, Stage::SizeOfPlayers>, Stage::SizeOfPlayers>;
    // プレイヤーの戦略。試行ごとに入れ替えて実行する。
    using Strategies = std::array<Strategy, Stage::SizeOfPlayers>;

    // 対戦結果
    struct Result {
        Node::Count n_trials {0};  // 試行回数
        Node::Count n_draw {0};    // 勝敗が付かなかった=引き分けの回数
        Node::Count n_playout {0}; // Playout回数
        ResultMatrix matrix {{{0, 0}, {0, 0}}};  // 勝った回数

        Result(Node::Count arg_n_trials, Node::Count arg_n_playout, const ResultMatrix& mat) :
            n_trials(arg_n_trials), n_draw(arg_n_trials), n_playout(arg_n_playout), matrix(mat) {
            for(const auto& vec : matrix) {
                for(const auto& ct : vec) {
                    n_draw -= ct;
                }
            }
        }
    };

    // 対戦する
    // n_trials: 試行回数(対戦回数)
    // n_train_online: 一手ごとのMCTS探索回数(0なら探索しない)
    // limit_msec: 一手ごとのMCTS探索時間(ミリ秒、0なら探索しない)
    // strategies: プレイヤーの戦略
    // engine: MCTSの探索エンジン
    Result match(Node::Count n_trials, Node::Count n_train_online, MilliSec limit_msec,
                 const Strategies& strategies, MctsEngine& engine) {
        // 各プレイヤーが何番目の戦略を使うか
        std::vector<size_t> strategy_index(Stage::SizeOfPlayers);
        std::iota(strategy_index.begin(), strategy_index.end(), 0);

        // プレイヤー * 戦略の結果
        ResultMatrix result;
        for(auto&& vec : result) {
            for(auto&& ct : vec) {
                ct = 0;
            }
        }

        Node::Count n_playout {0}; // Playout回数

        for(Node::Count i{0}; i<n_trials; ++i) {
            // 初期盤面は、ハッシュキーを共通にするためにMCTSエンジンから取得する
            auto stage = engine.initial_stage();

            bool gameover {false};  // ゲーム終了
            while(!gameover) {
                for(Player player{0}; player < Stage::SizeOfPlayers; ++player) {
                    // 先手/後手が打つ手番を探す
                    std::optional<Board::Position> action;

                    // ランダムな手番か、MCTSで探すか
                    const auto strategy = strategies.at(strategy_index.at(player));
                    switch(strategy) {
                    case Strategy::OnlineMcts:
                        action = engine.select(stage);
                        break;
                    default:
                        action = engine.select_random(stage);
                        break;
                    }

                    // 打つ手がなければ引き分け
                    if (!action.has_value()) {
                        gameover = true;
                        break;
                    }

                    // MCTSエンジンにこの局面と次の局面を登録する
                    engine.advance(stage, action.value());

                    // 先手/後手が打つ
                    if (stage.advance(action.value().column, action.value().height) == Stage::Result::Won) {
                        result.at(player).at(strategy_index.at(player)) += 1;
                        gameover = true;
                        break;
                    }

                    // MCTSエンジンでこの局面から探索を開始する
                    // 指定回数まで時間いっぱいまでの少ない方まで探索する。時間0なら全く検索しない。
                    // 1ミリ秒で10-100回くらいplayoutできる
                    const auto start_time = std::chrono::high_resolution_clock::now();
                    for(decltype(n_train_online) i{0}; i<n_train_online; ++i) {
                        const auto diff = std::chrono::high_resolution_clock::now() - start_time;
                        if ((limit_msec == ZeroMilliSec) || (diff >= limit_msec)) {
                            break;
                        }

                        engine.playout(stage);
                        ++n_playout;
                    }
                }
            }

            // 先手と後手の戦略を入れ替える
            std::rotate(strategy_index.begin(), strategy_index.begin() + 1, strategy_index.end());
        }

        return Result(n_trials, n_playout, result);
    }
};

// ノードを一段展開する
TEST_F(TestMctsEngine, ExpandRoot) {
    MctsEngine engine;
    Stage stage(engine.hashkeys_);

    ASSERT_TRUE(engine.root_);
    ASSERT_FALSE(engine.root_->expanded());
    engine.expand(engine.root_);
    ASSERT_TRUE(engine.root_->expanded());
    ASSERT_EQ(Board::ColumnSize, engine.root_->children().size());

    for(const auto& child : engine.root_->children()) {
        ASSERT_FALSE(child->children().size());
        ASSERT_EQ(1, child->parents().size());
        ASSERT_EQ(engine.root_, child->parents().at(0));
    }
}

// 異なる手順で同じ盤面に到達する
TEST_F(TestMctsEngine, Join) {
    MctsEngine engine;
    Stage stage(engine.hashkeys_);

    auto stage12 = stage;
    auto stage21 = stage;
    stage12.advance(1, 0);
    stage12.advance(3, 0);
    stage12.advance(2, 0);
    stage12.advance(4, 0);

    stage21.advance(2, 0);
    stage21.advance(4, 0);
    stage21.advance(1, 0);
    stage21.advance(3, 0);

    auto node12_obj = std::make_unique<Node>(stage12);
    auto node12 = engine.nodeset_.add(node12_obj);
    ASSERT_FALSE(node12->expanded());
    engine.expand(node12);
    ASSERT_TRUE(node12->expanded());
    ASSERT_EQ(Board::ColumnSize, node12->children().size());

    auto node21_obj = std::make_unique<Node>(stage21);
    auto node21 = engine.nodeset_.add(node21_obj);
    ASSERT_EQ(node21, node12);
    ASSERT_TRUE(node21->expanded());
    EXPECT_EQ(Board::ColumnSize, node21->children().size());

    for(const auto& child : node12->children()) {
        ASSERT_EQ(1, child->parents().size());
        ASSERT_EQ(node12, child->parents().at(0));
    }
}

// 上まで積みあげる
TEST_F(TestMctsEngine, ExpandColumns) {
    MctsEngine engine;
    Stage stage(engine.hashkeys_);
    constexpr auto len = Board::MinLen - 1;

    for(Board::Coordinate column{0}; column < len; ++column) {
        for(Board::Coordinate height{0}; height < Board::MaxHeight; ++height) {
            stage.advance(column, height);
        }

        auto new_stage = stage;
        auto parent_obj = std::make_unique<Node>(new_stage);
        auto parent = engine.nodeset_.add(parent_obj);
        ASSERT_FALSE(parent->expanded());
        engine.expand(parent);
        ASSERT_TRUE(parent->expanded());

        if ((column + 1) == len) {
            ASSERT_EQ(1, parent->children().size());
            const auto& child = parent->children().at(0);
            ASSERT_FALSE(child->children().size());
            ASSERT_EQ(1, child->parents().size());
            ASSERT_EQ(parent, child->parents().at(0));
        } else {
            const auto expected = Board::ColumnSize - 1 - column;
            ASSERT_EQ(expected, parent->children().size());

            for(const auto& child : parent->children()) {
                for(const auto& p : child->parents()) {
                    ASSERT_FALSE(child->children().size());
                    ASSERT_EQ(1, child->parents().size());
                    ASSERT_EQ(parent, child->parents().at(0));
                }
            }
        }
    }
}

// 初期状態からランダムにプレイする
// ランダムなら先手有利らしい
TEST_F(TestMctsEngine, Play) {
    MctsEngine engine;

    std::array<Player, Stage::SizeOfPlayers> ct {0};
    for(Node::Count i{0}; i<10000; ++i) {
        MctsEngine::Result result = engine.play(engine.initial_stage_, 0);
        if (result.result == Stage::Result::Won) {
            ct.at(result.winner) += 1;
        }
    }

    EXPECT_GT(ct.at(0), ct.at(1));
    std::cout << "Random match 1st vs 2nd : " << ct.at(0) << " , " << ct.at(1) << "\n";
}

TEST_F(TestMctsEngine, Playout) {
    MctsEngine engine;

    std::array<Player, Stage::SizeOfPlayers> ct {0};
    for(Node::Count i{0}; i<1000; ++i) {
        engine.playout();
    }

    EXPECT_GT(engine.root_->n_first_player_won(), engine.root_->n_second_player_won());
}

// ランダム同士で対戦する
TEST_F(TestMctsEngine, MatchRandom) {
    MctsEngine engine;
    const Node::Count n_trials = 2000;
    const Strategies strategies {Strategy::Random, Strategy::Random};
    const auto result = match(n_trials, 0, ZeroMilliSec, strategies, engine);
    ASSERT_EQ(n_trials, result.n_trials);

    std::cout << "Random vs random\n";
    std::cout << "1st player win and loss : " <<
        result.matrix.at(0).at(0) << " , " << result.matrix.at(0).at(1) << "\n";
    std::cout << "2nd player win and loss : " <<
        result.matrix.at(1).at(0) << " , " << result.matrix.at(1).at(1) << "\n";
    std::cout << "draw : " << result.n_draw << "\n";
}

// MCTS対ランダムで対戦する。対戦中も探索する。
TEST_F(TestMctsEngine, MatchMctsOnlineRandom) {
    MctsEngine engine;
    Node::Count n_train_full = 25000;

    for(decltype(n_train_full) i{0}; i<n_train_full; ++i) {
        engine.playout();
    }

    const Node::Count n_trials = 2000;
    Node::Count n_train_online = 10000;
    const MilliSec limit_msec {1};
    const Strategies strategies {Strategy::OnlineMcts, Strategy::Random};
    const auto result = match(n_trials, n_train_online, limit_msec, strategies, engine);
    ASSERT_EQ(n_trials, result.n_trials);

    std::cout << "MCTS online vs random\n";
    std::cout << "1st player learned win and loss : " <<
        result.matrix.at(0).at(0) << " , " << result.matrix.at(0).at(1) << "\n";
    std::cout << "2nd player learned win and loss : " <<
        result.matrix.at(1).at(0) << " , " << result.matrix.at(1).at(1) << "\n";
    std::cout << "draw : " << result.n_draw << "\n";
    std::cout << "n_trials : " << result.n_trials << "\n";
    std::cout << "n_playout : " << result.n_playout << "\n";
}

// MCTS同士で対戦する。対戦中も探索する。
TEST_F(TestMctsEngine, MatchMctsOnline) {
    MctsEngine engine;
    Node::Count n_train_full = 25000;

    for(decltype(n_train_full) i{0}; i<n_train_full; ++i) {
        engine.playout();
    }

    const Node::Count n_trials = 200;
    Node::Count n_train_online = 100000;
    const MilliSec limit_msec {1};
    const Strategies strategies {Strategy::OnlineMcts, Strategy::OnlineMcts};
    const auto result = match(n_trials, n_train_online, limit_msec, strategies, engine);
    ASSERT_EQ(n_trials, result.n_trials);

    std::cout << "MCTS online vs MCTS online\n";
    std::cout << "1st player win and loss : " <<
        result.matrix.at(0).at(0) << " , " << result.matrix.at(0).at(1) << "\n";
    std::cout << "2nd player win and loss : " <<
        result.matrix.at(1).at(0) << " , " << result.matrix.at(1).at(1) << "\n";
    std::cout << "draw : " << result.n_draw << "\n";
}

// MCTSだが対戦中は学習しない vs ランダム
TEST_F(TestMctsEngine, MatchMctsOfflineRandom) {
    MctsEngine engine;
    Node::Count n_train_full = 25000;

//  初期盤面から学習する
//  これくらい長く実行するとよいが、21分掛かる
//  TestMctsEngine.Match (1321686 ms)
//  Node::Count n_train = 300000 * 6 * 15;
//  1st player learned win and loss : 4236 , 748
//  2nd player learned win and loss : 3668 , 1321

    for(decltype(n_train_full) i{0}; i<n_train_full; ++i) {
        engine.playout();
    }

    const Node::Count n_trials = 2000;
    const Strategies strategies {Strategy::OnlineMcts, Strategy::Random};
    const auto result = match(n_trials, 0, ZeroMilliSec, strategies, engine);
    ASSERT_EQ(n_trials, result.n_trials);

    std::cout << "MCTS offline vs random\n";
    std::cout << "1st player learned win and loss : " <<
        result.matrix.at(0).at(0) << " , " << result.matrix.at(0).at(1) << "\n";
    std::cout << "2nd player learned win and loss : " <<
        result.matrix.at(1).at(0) << " , " << result.matrix.at(1).at(1) << "\n";
    std::cout << "draw : " << result.n_draw << "\n";
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    CommonHashKey keys(Stage::SizeOfPlayers);
    Stage stage(keys);

    for(;;) {
        std::cout << stage.to_string() << std::endl;
        std::cout << "Player " << (1 + stage.player()) << " moves\n";

        Board::Coordinate column {0};
        Board::Coordinate height {0};
        std::cin >> column >> height;

        const auto result = stage.advance(column, height);

        if (result == Stage::Result::Won) {
            std::cout << "Player " << (1 + stage.player()) << " Win\n";
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
