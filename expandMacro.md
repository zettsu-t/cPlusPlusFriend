# 複数行から成るマクロを正しく展開する

ソースコードは[cppFriendsClangTest.cpp](cppFriendsClangTest.cpp)です。

## マクロが必要な状況とは

C++ならマクロよりもテンプレートとインライン関数を使う方がよいのが一般的です。ですがマクロでなければ、どうしてもできないことがあります。それはソースコードの位置を適切に埋め込むことです。

デバッグprintfをしたいときに、デバッグしたい箇所の```__PRETTY_FUNCTION__, __FILE__, __LINE__```を埋め込むとします。これらを共通化しようとして関数にすると、その共通関数が定義された場所の情報が入ってしまいます。マクロにすれば、マクロを定義した場所ではなく、マクロを展開する場所の情報が入ります。

```c++
#define DEBUG_PRINT_COUT(osout, str) osout << str << "@" << __PRETTY_FUNCTION__ << "in " << __FILE__ << " : " << __LINE__;
if (cond) DEBUG_PRINT_COUT(out, message);
```

は、プリプロセッサによって以下のように展開されます。セミコロンが一個多いような気がしますが、プリプロセッサが文字通り置き換えると確かにその通りになります(この点は後述)。

```c++
if (cond) out << message << "@" << __PRETTY_FUNCTION__ << "in " << "cppFriendsClangTest.cpp" << " : " << 116;;
```

## 複数行から成るマクロを書く

実際には上記のように一文で済むとは限らず、ファイルにログを残すとか、もっと複雑なことをするかもしれません。二文の例を挙げましょう。

```c++
#define ILL_DEBUG_PRINT(osout, oserr, str) \
    osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \
```

さてこれをif文の直後に置くとどうなるでしょうか。

```c++
if (cond) ILL_DEBUG_PRINT(out, err, message);
```

は、

```c++
if (cond) out << message << "@" << __PRETTY_FUNCTION__ << ":";
err << message << "@" << __PRETTY_FUNCTION__ << ":";;
```

と展開されます(プリプロセッサは改行をつけませんが見やすくするために後で加えました、以下同様)。マクロの二文目は、if文の条件に関わらず実行されます。これはマクロを使う人が望む動作ではないでしょう。

## 正しく展開する

このようになってしまったのは、マクロの中身が一まとまりではないからです。ブロックにしましょう。

```c++
#define WELL_DEBUG_PRINT(osout, oserr, str) \
    do { \
        osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    } while(0)
```

```c++
if (cond) WELL_DEBUG_PRINT(out, err, message);
```

は、

```c++
if (cond) do {
    out << message << "@" << __PRETTY_FUNCTION__ << ":";
    err << message << "@" << __PRETTY_FUNCTION__ << ":";
} while(0);
```

と展開されます。これが正しい動作ですね。```while(0)```にセミコロンがないのは、マクロの直後にelseが来てもコンパイルエラーにしないためです。```WELL_DEBUG_PRINT```は見た目が関数っぽいので、使うときはセミコロンをつけると思いますが、

```c++
if (!out.tellp()) BAD_DEBUG_PRINT(out, err, message); else {}
```

は```while(0);```ですと、以下のように展開されて、elseの前にifがないと言われてコンパイルエラーになります。

```c++
if (cond) do {
    out << message << "@" << __PRETTY_FUNCTION__ << ":";
    err << message << "@" << __PRETTY_FUNCTION__ << ":";
} while(0);; else {}
```

同様に```do {} while(0)```ではなく、単なる{}としたときも、elseの前にifがないと言われてコンパイルエラーになります。

```c++
if (cond) {
    out << message << "@" << __PRETTY_FUNCTION__ << ":";
    err << message << "@" << __PRETTY_FUNCTION__ << ":";
}; else {}
```

## Compound statement

同じことを(void)({...}) でできるのではないか、というご指摘を頂きました。GCC拡張として、[Compound statement](https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html)というものがあるようです。これは、```({...})```の中身を評価して、最後の値を式を値渡しで返すというものです。スクリプト言語っぽいですね。

```c++
#define FUNC_DEBUG_PRINTF(fdout, fderr, str) \
    ({ \
        fprintf(fdout, "%s@%s", str, __PRETTY_FUNCTION__ ); \
        fprintf(fderr, "%s@%s", str, __PRETTY_FUNCTION__ ); \
    }) \

if (true) FUNC_DEBUG_PRINTF(stdout, stderr, "Printf in compound statements");
```

動作はdo-while版と同じです。

さてC++なのでiostreamを渡したいのですが、実はこのように書いて呼び出すとコンパイルエラーになります。

```c++
#define FUNC_DEBUG_COUT(osout, oserr, str) \
    ({ \
        osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    }) \

std::ostringstream out;
std::ostringstream err;
if (cond) FUNC_DEBUG_COUT(out, err, message);
```

原因は、outを値渡しで返そうとしますが、std::ostringstreamはコピーできないからです。fprintfはintを返すので問題になりませんでした。これは無意味な値を返すことで回避できます。

```c++
#define FUNC_DEBUG_COUT(osout, oserr, str) \
    ({ \
        osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        0; \
    }) \
```

このマクロは、do-whileを書けない場所に置くことができます。少しわざとらしいですが、以下のように書けます。ただしclang++は、FUNC_DEBUG_COUTの返り値を使っていないという警告を出します。

```c++
for(int i=1; i<=2; ++i, FUNC_DEBUG_COUT(out, err, message)) {
    message = boost::lexical_cast<decltype(message)>(i);
    // 何かする
};
```

をプリプロセッサで展開すると以下のようになります。このコードはコンパイルできます。

```c++
for(int i=1; i<=2; ++i, ({
    out << message << "@" << __PRETTY_FUNCTION__ << ":";
    err << message << "@" << __PRETTY_FUNCTION__ << ":"; 1;
})) { ... }
```

## より詳しい情報

以下をご覧ください

* https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Multi-statement_Macro
* https://stackoverflow.com/questions/154136/why-use-apparently-meaningless-do-while-and-if-else-statements-in-macros
