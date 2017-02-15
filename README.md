# 君はC++クラスのフレンズなんだね

フレンズと言ったらC++でしょう。

```c++
class Train {
    // すごーい! シリアライザはクラスを永続化できるフレンズなんだね
    friend std::ostream& operator <<(std::ostream& os, const Train& train);
    friend std::istream& operator >>(std::istream& is, Train& train);
    friend boost::serialization::access;

    // ユニットテストはprivateメンバを読めるフレンズなんだね。たーのしー!
    FRIEND_TEST(TestSerialization, Initialize);
    FRIEND_TEST(TestSerialization, Std);
    FRIEND_TEST(TestSerialization, Boost);
    FRIEND_TEST(TestSerialization, Invalid);
};
```

[cppFriends.cpp](cppFriends.cpp)では、friend関数以外にも「フレンズなんだね」を連発していますが、まあ流行語ですし。そういう意味で、「すごいHaskellたのしく学ぼう!」という題の本は、時代の先を行っていたのですね。

ライセンスは、[MITライセンス](LICENSE.txt)です。
