#include <cctype>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>
#include <string>
#include <type_traits>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <gtest/gtest.h>
#include "cFriendsCommon.h"
#include "cppFriends.hpp"

namespace {
    void TrimAndExpand(std::string& str) {
        boost::replace_all(str, "\r\n", " ");
        boost::replace_all(str, "\r", " ");
        boost::replace_all(str, "\n", " ");
        boost::trim(str);
        return;
    }
}

class Train {
    // すごーい! シリアライザはクラスを永続化できるフレンズなんだね
    friend std::ostream& operator <<(std::ostream& os, const Train& train);
    friend std::istream& operator >>(std::istream& is, Train& train);
    friend boost::serialization::access;

    // +演算子は列車を連結できるフレンズなんだね
    friend Train operator +(const Train& left, const Train& right) {
        Train train(left.name_ + "-" + right.name_, left.type_);
        return train;
    }

    // std::swapはメンバ変数を一括置換できるフレンズなんだね。おもしろーい！
    template<typename T>
    friend void std::swap(T& a, T& b);

    // ユニットテストはprivateメンバを読めるフレンズなんだね。たーのしー!
    FRIEND_TEST(TestSerialization, Initialize);
    FRIEND_TEST(TestSerialization, Concatenate);
    FRIEND_TEST(TestSerialization, ToString);
    FRIEND_TEST(TestSerialization, Std);
    FRIEND_TEST(TestSerialization, Boost);
    FRIEND_TEST(TestSerialization, StdInvalid);
    FRIEND_TEST(TestSerialization, BoostInvalidOut);
    FRIEND_TEST(TestSerialization, BoostInvalidIn);

public:
    // 君ははまなす廃止後、定期運行されている急行がないと知っているフレンズなんだね
    enum class Type { Local, Rapid, Limited_Express };
    Train(void) = default;
    Train(const std::string& name, Type type) : name_(name), type_(type) {
        TrimAndExpand(name_);
    }

    virtual ~Train(void) = default;  // {}ではない

    std::string ToString(void) const {
        std::string str;
        auto i = std::find_if(descriptions_.begin(), descriptions_.end(),
                              [=](auto& s) { return (s.first == type_); });
        std::string desc = (i != descriptions_.end()) ? i->second : "??";
        str = name_ + "は" + desc + "で走る列車なんだね";
        return str;
    }

    // GCC 6.3.0ではprivate, protectedにするとエラーになる
    // private within this context
// protected:
// private:
    Train(Train&&) = default;
    Train& operator =(Train&&) = default;

private:
    // Effective Modern C++ 項目17を参照
    Train(const Train&) = default;
    Train& operator =(const Train&) = default;

    class InvalidValue : public std::ios_base::failure {
    public:
        explicit InvalidValue(const std::string& message) : std::ios_base::failure(message) {}
    };

    // 例外は、返り値が得られないことを教えてくれるフレンズなんだね
    // boost::optionalという手もありますけど
    static Type toType(std::underlying_type<Type>::type type) {
        if (std::none_of(
                descriptions_.begin(), descriptions_.end(),
                [=](auto& s) {
                    return (static_cast<decltype(type)>(s.first) == type);
                })) {
            std::string message = "Invalid number "
                + boost::lexical_cast<decltype(message)>(type);

            throw InvalidValue(message);
        }

        return static_cast<Type>(type);
    };

    template<typename T>
    void serialize(T& archive, const unsigned int version) {
        auto original = *this;
        archive & name_;

        try {
            // すごーい! Boost Serializationはenumをキャストなしで読み書きできるフレンズなんだね
            // stdはstd::underlying_typeにしないと、>>で読み書きしたり、
            // unorderedのキーにしたりできないみたいです
            archive & type_;
            toType(static_cast<boost::function_traits<decltype(toType)>::arg1_type>(type_));
        } catch(InvalidValue& e) {
            // type_が得られないときはロールバックする
            std::swap(*this, original);
            throw e;
            // 読み込み失敗による例外は、そのまま呼び出し元に伝える
        }

        return;
    }

    // メンバがすべてmove assignableかつmove constructibleだから、このクラスはswappable
    std::string name_;
    Type type_ {Type::Local};
    // 種別を読めるようにする
    static const std::vector<std::pair<Type, std::string>> descriptions_;
};

const std::vector<std::pair<Train::Type, std::string>> Train::descriptions_ {
    {Train::Type::Local, "普通"},
    {Train::Type::Rapid, "快速"},
    {Train::Type::Limited_Express, "特急"}};

std::ostream& operator <<(std::ostream& os, const Train& train) {
    // osが例外を出すよう設定するが後で戻す
    boost::io::ios_flags_saver saver(os);
    os.exceptions(std::ios::failbit | std::ios::badbit);

    using IntType = std::underlying_type<decltype(train.type_)>::type;
    // 列車名に改行がないと想定しているが、あったら空白に置き換える
    std::string name = train.name_;
    TrimAndExpand(name);
    os << name << "\n";
    os << static_cast<IntType>(train.type_);
    return os;
}

std::istream& operator >>(std::istream& is, Train& train) {
    // isが例外を出すよう設定するが後で戻す
    boost::io::ios_flags_saver saver(is);
    is.exceptions(std::ios::failbit | std::ios::badbit);

    Train t;
    // 改行コードの違うプラットフォーム超えてデータを運ぶことは想定していない
    std::getline(is, t.name_);

    // ストリームから読み込んだ内容は戻さないので、基本例外安全とはいえても
    // 強い例外安全とは言えない
    std::underlying_type<decltype(train.type_)>::type type;
    is >> type;
    t.type_ = Train::toType(type);

    // std::swapはno-throw保証なので、同時に強い例外安全でもある
    std::swap(train, t);
    // 君は全メンバの読み込みに成功したオブジェクトを返すフレンズなんだね
    return is;
}

template<typename Derived>
class RandomNumber {
public:
    RandomNumber(void) = default;
    uint_fast32_t Get(void) {
        return static_cast<Derived*>(this)->impl();
    }

    RandomNumber(const RandomNumber&) = delete;
    RandomNumber& operator =(const RandomNumber&) = delete;
};

class SoftwareRand : public RandomNumber<SoftwareRand> {
public:
    SoftwareRand(void) : gen(rd()) {}
private:
    // すごーい! CRTPは静的ポリモーフィズムができるフレンズなんだね
    friend class RandomNumber<SoftwareRand>;
    std::result_of<decltype(
        &RandomNumber<SoftwareRand>::Get)(RandomNumber<SoftwareRand>)>::type impl() {
        return gen();
    }

    std::random_device rd;
    std::mt19937 gen;
};

class HardwareRand : public RandomNumber<HardwareRand> {
public:
    HardwareRand(void) {}
private:
    friend class RandomNumber<HardwareRand>;
    std::result_of<decltype(
        &RandomNumber<SoftwareRand>::Get)(RandomNumber<SoftwareRand>)>::type impl() {
        uint32_t number = 0;
        asm volatile (
            "rdrand %0 \n\t"
            :"=r"(number)::);
        return number;
    }
};

class TestSerialization : public ::testing::Test {
protected:
    void checkIfStartWith(const std::string& actual, const std::string& keyword) {
        EXPECT_EQ(0, actual.find(keyword));
        return;
    }
};

TEST_F(TestSerialization, Initialize) {
    Train train;
    EXPECT_TRUE(train.name_.empty());
    EXPECT_EQ(Train::Type::Local, train.type_);

    const std::string rapidName = "Acty";
    constexpr Train::Type rapidType = Train::Type::Rapid;
    Train rapid(rapidName, rapidType);
    EXPECT_EQ(rapidName, rapid.name_);
    EXPECT_EQ(rapidType, rapid.type_);

    const std::string ltdExpressName = " フラノラベンダー\nエクスプレス ";
    constexpr Train::Type ltdExpressType = Train::Type::Limited_Express;
    Train ltdExpress(ltdExpressName, ltdExpressType);
    EXPECT_EQ("フラノラベンダー エクスプレス", ltdExpress.name_);
    EXPECT_EQ(ltdExpressType, ltdExpress.type_);
}

TEST_F(TestSerialization, Concatenate) {
    constexpr Train::Type type = Train::Type::Limited_Express;
    const std::string leftName =  "Sunrise Seto";
    const std::string rightName = "Sunrise Izumo";
    const Train left(leftName, type);
    const Train right(rightName, type);

    const Train ltdExpress = left + right;
    EXPECT_EQ("Sunrise Seto-Sunrise Izumo", ltdExpress.name_);
    EXPECT_EQ(type, ltdExpress.type_);
}

TEST_F(TestSerialization, ToString) {
    const Train local("いさぶろう", Train::Type::Local);
    const Train rapid("ラビット", Train::Type::Rapid);
    const Train ltdExpress("サンダーバード", Train::Type::Limited_Express);

    EXPECT_EQ("いさぶろうは普通で走る列車なんだね", local.ToString());
    EXPECT_EQ("ラビットは快速で走る列車なんだね", rapid.ToString());
    EXPECT_EQ("サンダーバードは特急で走る列車なんだね", ltdExpress.ToString());

    Train outOfService("回送", Train::Type::Local);
    outOfService.type_ = static_cast<Train::Type>(3);
    EXPECT_EQ("回送は??で走る列車なんだね", outOfService.ToString());
}

TEST_F(TestSerialization, Std) {
    {
        const std::string name = "Sunrise Seto";
        const Train::Type type = Train::Type::Limited_Express;

        const Train original(name, type);
        std::stringstream ss;
        ss << original;

        Train restored;
        ss >> restored;
        EXPECT_EQ(name, restored.name_);
        EXPECT_EQ(type, restored.type_);
    }

    {
        const std::string name = " Tottori\nLiner ";
        constexpr Train::Type type = Train::Type::Rapid;

        const Train original(name, type);
        std::stringstream ss;
        ss << original;

        Train restored;
        ss >> restored;
        EXPECT_EQ("Tottori Liner", restored.name_);
        EXPECT_EQ(type, restored.type_);
    }
}

TEST_F(TestSerialization, Boost) {
    const std::string name = "Sunrise Izumo";
    constexpr Train::Type type = Train::Type::Limited_Express;

    const Train original(name, type);
    std::stringstream ss;
    boost::archive::text_oarchive oarch(ss);
    oarch << original;

    Train restored;
    boost::archive::text_iarchive iarch(ss);
    iarch >> restored;
    EXPECT_EQ(name, restored.name_);
    EXPECT_EQ(type, restored.type_);
}

TEST_F(TestSerialization, StdInvalid) {
    const std::string name = "Unnamed";
    {
        const std::string arg = name + "\n3";
        std::istringstream is(arg);
        Train notRestored;
        std::string actual;

        try {
            is >> notRestored;
        } catch(Train::InvalidValue& e) {
            // 入力を適切にenumへとキャストできなかった
            actual = e.what();
        }

        // MinGWでは": iostream error"が後ろにつく
        const std::string expected = "Invalid number 3";
        checkIfStartWith(actual, expected);
    }

    {
        std::istringstream is(name);
        Train notRestored;
        std::string actual;

        try {
            is >> notRestored;
        } catch(std::exception& e) {
            // 入力が短すぎて読めなかった
            actual = e.what();
        }

        EXPECT_FALSE(actual.empty());
    }
}

TEST_F(TestSerialization, BoostInvalidOut) {
    const std::string name = "Unnamed";
    const Train::Type type = static_cast<Train::Type>(3);
    const Train original(name, type);
    std::ostringstream os;
    std::string actual;

    try {
        boost::archive::text_oarchive oarch(os);
        oarch << original;
    } catch(Train::InvalidValue& e) {
        // 出力値が壊れている
        actual = e.what();
    }

    const std::string expected = "Invalid number 3";
    checkIfStartWith(actual, expected);
}

TEST_F(TestSerialization, BoostInvalidIn) {
    const std::string name = "Unnamed";
    const Train::Type type = Train::Type::Local;
    const Train original(name, type);
    std::ostringstream os;
    boost::archive::text_oarchive oarch(os);
    oarch << original;

    auto baseText = os.str();
    baseText.pop_back();

    const std::vector<std::pair<std::string, std::string>> testCases {
        {"3", "Invalid number 3"}, {"A", "input stream error"}};

    for(auto& test : testCases) {
        const std::string text = baseText + test.first;
        std::istringstream is(text);
        std::string actual;
        Train notRestored;

        try {
            boost::archive::text_iarchive iarch(is);
            iarch >> notRestored;
        } catch(std::exception& e) {
            // 入力が壊れている
            actual = e.what();
        }

        checkIfStartWith(actual, test.second);
    }
}

// 通常の速度計
class SpeedController {
public:
    virtual ~SpeedController(void) = default;

    SpeedKph GetSpeed(const SpeedParameter& param) const {
        SpeedKph speed = 0;

        if (param.size >= sizeof(param)) {
            // 本来ならテーブルをあらかじめ作っておくが、本題ではないので簡単に済ませる
            switch(param.index) {
            case SPEED_STOP:
                break;
            case SPEED_LOW:
                speed = 45;
                break;
            case SPEED_MIDDLE:
                speed = 70;
                break;
            case SPEED_HIGH:
                // もっと速く走れる場合は読み替える
                speed = 130;
                break;
            default:
                break;
            }
        }

        return speed;
    }
};

// 高速走行に対応した速度計
class HighSpeedController {
public:
    HighSpeedController(SpeedKph maxSpeed) : maxSpeed_(maxSpeed) {}
    virtual ~HighSpeedController(void) = default;

    SpeedKph GetSpeed(const SpeedParameter& param) const {
        // 入れ子の構造体は、必ず先頭でなければならない
        static_assert(offsetof(ExtSpeedParameter, base) == 0, "Must be its first member");
        static_assert(std::is_same<
                      decltype(ExtSpeedParameter::base),
                      std::remove_cv_t<std::remove_reference_t<decltype(param)>>>::value,
                      "Must have a same type");

        // Chain of responsibility
        if (param.size < sizeof(ExtSpeedParameter) || (param.index != SPEED_HIGH)) {
            return base_.GetSpeed(param);
        }

        const auto& extParam = reinterpret_cast<const ExtSpeedParameter&>(param);
        return std::min(maxSpeed_, extParam.maxSpeed);
    }

private:
    SpeedController base_;
    SpeedKph maxSpeed_ {0};
};

class TestVersioning : public ::testing::Test {};

TEST_F(TestVersioning, InvalidSize) {
    SpeedParameter zero {0, SPEED_LOW};
    SpeedParameter minusOne {sizeof(SpeedParameter) - 1, SPEED_LOW};

    SpeedController ctrl;
    EXPECT_EQ(0, ctrl.GetSpeed(zero));
    EXPECT_EQ(0, ctrl.GetSpeed(minusOne));

    HighSpeedController highCtrl(160);
    EXPECT_EQ(0, highCtrl.GetSpeed(zero));
    EXPECT_EQ(0, highCtrl.GetSpeed(minusOne));
}

TEST_F(TestVersioning, PartialInvalidSize) {
    ExtSpeedParameter param {{sizeof(param) - 1, SPEED_LOW}, 150};
    SpeedController ctrl;
    HighSpeedController highCtrl(160);
    EXPECT_EQ(45, ctrl.GetSpeed(param.base));
    EXPECT_EQ(45, highCtrl.GetSpeed(param.base));
}

TEST_F(TestVersioning, InvalidParam) {
    SpeedParameter param {sizeof(param), SPEED_COUNT};
    SpeedController ctrl;
    HighSpeedController highCtrl(160);
    EXPECT_EQ(0, ctrl.GetSpeed(param));
    EXPECT_EQ(0, highCtrl.GetSpeed(param));
}

TEST_F(TestVersioning, Valid) {
    struct TestCase {
        SpeedIndex index;
        SpeedKph   expected;
        SpeedKph   expectedExt;
    };

    constexpr TestCase testCases[] = {
        {SPEED_STOP, 0, 0}, {SPEED_LOW, 45, 45}, {SPEED_MIDDLE, 70, 70}, {SPEED_HIGH, 130, 140}};

    SpeedController ctrl;
    HighSpeedController highCtrl(150);
    for(const auto& test : testCases) {
        SpeedParameter param {sizeof(param), test.index};
        EXPECT_EQ(test.expected, ctrl.GetSpeed(param));
        EXPECT_EQ(test.expected, highCtrl.GetSpeed(param));

        ExtSpeedParameter paramExt {{sizeof(paramExt), test.index}, 140};
        EXPECT_EQ(test.expectedExt, highCtrl.GetSpeed(paramExt.base));
    }

    ExtSpeedParameter paramLower = {{sizeof(paramLower), SPEED_HIGH}, 140};
    ExtSpeedParameter paramOver  = {{sizeof(paramOver), SPEED_HIGH}, 160};
    EXPECT_EQ(130, ctrl.GetSpeed(paramOver.base));
    EXPECT_EQ(130, ctrl.GetSpeed(paramLower.base));
    EXPECT_EQ(140, highCtrl.GetSpeed(paramLower.base));
    EXPECT_EQ(150, highCtrl.GetSpeed(paramOver.base));
}

class TestRandomNumber : public ::testing::Test{};

namespace {
    using Count = size_t;
    template<typename T,
             typename R = typename std::result_of<decltype(&T::Get)(T)>::type>
    void CountRandomNumber(T& randomNumber, std::ostream& os) {
        std::unordered_map<R, bool> board;
        Count count=0;
        for(count=0; count<10; ++count) {
            os << randomNumber.Get() << ":";
        }
        os << "\n";

        for(count=0; count<1000000; ++count) {
            board[randomNumber.Get()] = true;
        }

        std::cout << board.size() << "/" << count << " numbers are found \n";
        EXPECT_LE((count / 100) * 99, count);
        return;
    }
}

TEST_F(TestRandomNumber, List) {
    SoftwareRand sr;
    HardwareRand hr;
    CountRandomNumber(sr, std::cout);
    CountRandomNumber(hr, std::cout);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
