# switch-caseは整数しか振り分けられない

今更ですが、C++のswitch-caseは整数を定数でしか振り分けられないことを知りました。Rubyのcase-whenが便利なので、ついC++もそうだと思ったのですが、C++はCと同じなのですね。

## C/C++のswitch-case

こういうクラス階層があったとします。

```c++
class Animal;
class Cat : public Animal;
class Serval : public Cat;
```

フレンズに名乗ってもらうためには、IntroduceSelfで名乗ってもらうのが、オブジェクト指向では普通の方法です。

```c++
JapariPark::Animal animal;
animal.IntroduceSelf();  // "I'm an animal"
JapariPark::Cat cat;
cat.IntroduceSelf();     // "I'm a cat"
JapariPark::Serval serval;
serval.IntroduceSelf();  // "I'm a serval"
```

ここを敢えて手続き型言語らしく書くと、このようになります。

```c++
auto& tid = typeid(obj);
if (tid == typeid(Animal)) {
   line = "She is an animal";
} else if (tid == typeid(Cat)) {
   line = "She is a cat";
} else if (tid == typeid(Serval)) {
   line = "She is a serval";
} else {
   line = "教えてやるので、おかわりをよこすのです";
}
```

さてここでは多段if-elseを使いましたが、switch-caseで書けないでしょうか。実は書けません。なぜならtypeidが返す型はstd::type_info(またはその派生クラス)であって整数ではなく、そしてswitchは整数しか受け付けないからです。

```c++
switch(typeid(obj)) {
case typeid(Animal):
```

std::type_info.hash_code()を使うと整数が得られます(ハッシュ衝突が心配ですが)。ですが今度はcaseが定数ではないのでコンパイルエラーになります。

```c++
switch(typeid(obj).hash_code()) {
case typeid(Animal).hash_code():
```

そうだったのかーッ

## Rubyのcase-when

Rubyのcase-whenは、C系言語のswitch-caseとは似て非なるものです。くわしくは[外部サイト](http://melborne.github.io/2013/02/25/i-wanna-say-something-about-rubys-case/)に解説があります。

Rubyでポリモーフィズムはできるに決まってますのでそれは省略して、case-whenと===演算子の挙動を確かめてみましょう。===演算子が呼び出されるたびに、呼び出された回数を記録します。

```ruby
class Animal
  attr_reader :count
  def initialize
    @count = 0
  end

  # 本来の===の使い方とは違うが...
  def ===(other)
    # 呼び出された回数を後で返す
    @count += 1
    # 派生クラスは、派生クラスであるかどうかを返す
    instance_of?(other.class)
  end

  def introduceSelf
    "けものだよ"
  end
end

class Cat < Animal
  def introduceSelf
    "ネコ科だよ"
  end
end

class Serval < Cat
  def introduceSelf
    "私はサーバルキャットのサーバル!"
  end
end
```

WhiteFacedScopsOwl(アフリカオオコノハズク)はかしこいので、case-whenをつかって何のフレンズか教えてくれます。

```ruby
class WhiteFacedScopsOwl
  def initialize
    @base   = Animal.new
    @sub    = Cat.new
    @subsub = Serval.new
  end

  def matchUpto(obj)
    # objと一致する型を見つけて、結果を文字列で返す
    case obj
    when @base then
      "けものです"
    when @sub then
      "ネコ科です"
    when @subsub then
      "サーバルです"
    else
      "教えてやるので、おかわりをよこすのです"
    end
  end
end
```

詳しくは[caseWhen.rb](caseWhen.rb)にユニットテストがあるので、そちらをご参照ください。以下のことが分かります。
* 整数に限らず、何でも===で比較する
* whenが定数である必要はなく、その場で評価する
* 各whenは上から下に順に評価し、当てはまるものがあったらそこで終わる。Fall throughはしない。

## それではC++は

C++のswitch-caseがなぜこうなっているか、という私の推測です。
* caseが整数なら、caseを評価するということ自体発生しないので、副作用が生じない
* よってcaseをどういう順序で評価しても、評価しなくても構わない
* そのためコンパイル時にルックアップテーブルに置き換えることができる

C++ではoperator==を持つオブジェクトであれば、Rubyてき振る舞いをしてもよかったのでしょうが、敢えてそうしなかったようです。考えられる理由としては、constメンバ関数が副作用を持たないとは限らないからです。constメンバ関数から、C関数(自由関数)を呼ぶことも、グローバル変数を書き換えることも、例外を投げることも、mutable, const_castをつかってメンバ変数を書き換えることも何でもできます。

それならそれで、caseが定数でなければcaseの評価順序は不定にしてもよかったのでしょうが、そうしないのは私の知らない理由があるのでしょう。

なお三項演算子```(expr) ? t : f```は、tやfがstatementでも構いません。この場合if-elseの単なるsyntactic sugarになります。[cppFriendsClang.cpp](cppFriendsClang.cpp)をコンパイルして、出力されるアセンブリをご確認ください。
