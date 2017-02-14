#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <type_traits>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class Train {
    // すごーい! シリアライザはクラスを永続化できるフレンズなんだね
    friend std::ostream& operator <<(std::ostream& os, const Train& train);
    friend std::istream& operator >>(std::istream& is, Train& train);
    friend boost::serialization::access;

    // std::swapはメンバ変数を一括置換できるフレンズなんだね。おもしろーい！
    template<typename T>
    friend void std::swap(T& is, T& train);

    // ユニットテストはprivateメンバを読めるフレンズなんだね。たーのしー!
    FRIEND_TEST(TestSerialization, Initialize);
    FRIEND_TEST(TestSerialization, Std);
    FRIEND_TEST(TestSerialization, Boost);
    FRIEND_TEST(TestSerialization, Invalid);

public:
    // 君ははまなす廃止後、定期運行されている急行がないと知っているフレンズなんだね
    enum class Type { Local, Rapid, Limited_Express };
    Train(void) = default;
    Train(const std::string& name, Type type) : name_(name), type_(type) {}
    virtual ~Train(void) = default;

private:
    Train(const Train&) = default;
    Train& operator =(const Train&) = default;

    template<typename T>
    void serialize(T& archive, const unsigned int version) {
        // すごーい! Boost Serializationはenumをキャストなしで読み書きできるフレンズなんだね
        archive & name_;
        archive & type_;
    }

    // 君は範囲外のenum値を教えてくれるフレンズなんだね
    class InvalidValue : public std::ios_base::failure {
    public:
        explicit InvalidValue(const std::string& message) : std::ios_base::failure(message) {}
    };

    static Type toType(std::underlying_type<Type>::type type) {
        static const std::array<Type,3> validType_ {
            Type::Local, Type::Rapid, Type::Limited_Express };

        if (std::none_of(validType_.begin(), validType_.end(),
                         [=](Type t) { return (static_cast<decltype(type)>(t) == type); })) {
            std::string message = "Invalie number " + boost::lexical_cast<decltype(message)>(type);
            throw InvalidValue(message);
        }

        return static_cast<Type>(type);
    };

    std::string name_;
    Type type_ {Type::Local};
};

std::ostream& operator <<(std::ostream& os, const Train& train) {
    using IntType = std::underlying_type<decltype(train.type_)>::type;
    // 君は列車名に改行がないと思っているフレンズなんだね
    os << train.name_ << "\n";
    os << static_cast<IntType>(train.type_);
    return os;
}

std::istream& operator >>(std::istream& is, Train& train) {
    Train t;
    // 君は改行コードの違うプラットフォーム超えてデータを運ぶことを気にしないフレンズなんだね
    // (すみません、このコードはそれほど作り込んでいません...)
    std::getline(is, t.name_);

    std::underlying_type<decltype(train.type_)>::type type;
    is >> type;
    t.type_ = Train::toType(type);

    // 君は全メンバの読み込みに成功したオブジェクトを返すフレンズなんだね
    std::swap(train, t);
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

TEST_F(TestSerialization, Invalid) {
    const std::string name = "Unnamed";
    const Train::Type type = static_cast<Train::Type>(3);

    Train original(name, type);
    std::stringstream ss;
    ss << original;

    Train notRestored;
    EXPECT_THROW(ss >> notRestored, Train::InvalidValue);
    EXPECT_TRUE(notRestored.name_.empty());
    EXPECT_EQ(Train::Type::Local, notRestored.type_);

    std::string actual;
    try {
        ss >> notRestored;
    } catch(Train::InvalidValue& e){
        actual = e.what();
    }

    const std::string expected = "Invalie number 3";
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
