#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class Train {
    // すごーい! シリアライザはクラスを永続化できるフレンズなんだね
    friend std::ostream& operator <<(std::ostream& os, const Train& train);
    friend std::istream& operator >>(std::istream& is, Train& train);
    friend boost::serialization::access;

    // std::swapはメンバ変数を一括置換できるフレンズなんだね。おもしろーい！
    template<typename T>
    friend void std::swap(T& a, T& b);

    // ユニットテストはprivateメンバを読めるフレンズなんだね。たーのしー!
    FRIEND_TEST(TestSerialization, Initialize);
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
    Train(const std::string& name, Type type) : name_(name), type_(type) {}
    virtual ~Train(void) = default;  // {}ではない

    std::string ToString(void) const {
        std::string str;
        auto i = std::find_if(descriptions_.begin(), descriptions_.end(),
                              [=](auto& s) { return (s.first == type_); });
        std::string desc = (i != descriptions_.end()) ? i->second : "??";
        str = name_ + "は" + desc + "で走る列車なんだね";
        return str;
    }

private:
    // Effective Modern C++ 項目17を参照
    Train(const Train&) = default;
    Train& operator =(const Train&) = default;
    Train(Train&&) = default;
    Train& operator =(Train&&) = default;

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
        auto name = name_;
        auto type = type_;
        archive & name_;

        try {
            // すごーい! Boost Serializationはenumをキャストなしで読み書きできるフレンズなんだね
            // stdはstd::underlying_typeにしないと、>>で読み書きしたり、
            // unorderedのキーにしたりできないみたいです
            archive & type_;
            toType(static_cast<boost::function_traits<decltype(toType)>::arg1_type>(type_));
        } catch(InvalidValue& e) {
            // type_が得られないときはロールバックする
            std::swap(type, type_);
            std::swap(name, name_);
            throw e;
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
    // 列車名に改行がないと想定している
    os << train.name_ << "\n";
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

class TestSerialization : public ::testing::Test{};

TEST_F(TestSerialization, Initialize) {
    Train train;
    EXPECT_TRUE(train.name_.empty());
    EXPECT_EQ(Train::Type::Local, train.type_);

    std::string rapidName = "Acty";
    Train::Type rapidType = Train::Type::Rapid;
    Train rapid(rapidName, rapidType);
    EXPECT_EQ(rapidName, rapid.name_);
    EXPECT_EQ(rapidType, rapid.type_);
}

TEST_F(TestSerialization, ToString) {
    Train local("いさぶろう", Train::Type::Local);
    Train rapid("ラビット", Train::Type::Rapid);
    Train ltdExpress("サンダーバード", Train::Type::Limited_Express);

    EXPECT_EQ("いさぶろうは普通で走る列車なんだね", local.ToString());
    EXPECT_EQ("ラビットは快速で走る列車なんだね", rapid.ToString());
    EXPECT_EQ("サンダーバードは特急で走る列車なんだね", ltdExpress.ToString());

    Train outOfService("回送", Train::Type::Local);
    outOfService.type_ = static_cast<Train::Type>(3);
    EXPECT_EQ("回送は??で走る列車なんだね", outOfService.ToString());
}

TEST_F(TestSerialization, Std) {
    const std::string name = "Sunrise Seto";
    const Train::Type type = Train::Type::Limited_Express;

    Train original(name, type);
    std::stringstream ss;
    ss << original;

    Train restored;
    ss >> restored;
    EXPECT_EQ(name, restored.name_);
    EXPECT_EQ(type, restored.type_);
}

TEST_F(TestSerialization, Boost) {
    const std::string name = "Sunrise Izumo";
    const Train::Type type = Train::Type::Limited_Express;

    Train original(name, type);
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
        std::string arg = name + "\n3";
        std::istringstream is(arg);
        Train notRestored;
        std::string actual;

        try {
            is >> notRestored;
        } catch(Train::InvalidValue& e) {
            // 入力を適切にenumへとキャストできなかった
            actual = e.what();
        }

        const std::string expected = "Invalid number 3";
        EXPECT_EQ(expected, actual);
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
    Train original(name, type);
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
    EXPECT_EQ(expected, actual);
}

TEST_F(TestSerialization, BoostInvalidIn) {
    const std::string name = "Unnamed";
    const Train::Type type = Train::Type::Local;
    Train original(name, type);
    std::ostringstream os;
    boost::archive::text_oarchive oarch(os);
    oarch << original;

    auto baseText = os.str();
    baseText.pop_back();

    const std::vector<std::pair<std::string, std::string>> testCases {
        {"3", "Invalid number 3"}, {"A", "input stream error"}};

    for(auto& test : testCases) {
        std::string actual;
        std::string text = baseText + test.first;
        std::istringstream is(text);
        Train notRestored;
        try {
            boost::archive::text_iarchive iarch(is);
            iarch >> notRestored;
        } catch(std::exception& e) {
            // 入力が壊れている
            actual = e.what();
        }

        EXPECT_EQ(test.second, actual);
    }
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
