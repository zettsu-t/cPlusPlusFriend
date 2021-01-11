# プログラマたんbot 実証コード集

### 一通り実行する

Cygwin 64bitのターミナルまたは、MinGWをコマンドプロンプトから、

```bash
make
```

を実行すると、一通りテストをビルドして実行します。最後はコンパイルエラーで終わりますが、これはコンパイルエラーを意図的に再現しているものです。

当方の実行環境は以下の通りです。Google Test / Mockは$HOME直下にあると仮定していますので、それ以外の場合はMakefileを変更してください(C:\home\usernameから/cygdrive/c/home/usernameへの変換は、Makefile中で行います)。AVX命令を使用することが前提ですので、サポートしていないプロセッサの場合は、Makefileの _CPPFLAGS_ARCH_ をコメントアウトしてください。

* Windows 10 November 2019 Update (バージョン 1909、OSビルド 18363.778)
* Google Test / Mock [(最新)](https://github.com/google/googletest)

|Tools|Cygwin 64-bit|MinGW-w64|
|:------|:------|:------|
|Base|3.1.4|MinGW Distro 17.1|
|GCC (g++)|9.3.0|9.2.0|
|LLVM (clang++)|8.0.1|9.0.1|
|Boost C++ Libraries|1.66.0|1.71.0|
|Ruby|2.6.4p104|2.7.0p0|

RDTSC命令の下の桁に偏りがある、という判定は実行環境によっては失敗するようです。何回か試して失敗するようでしたら、閾値を期待値に寄せるか、諦めてコメントアウトしてください。

### MinGWで実行する

[こちら](upgradeCompiler.md)にまとめましたが、情報が古いです。Cygwinでは実行するが、MinGWでは実行しないテストがあります。コンパイラのバージョンによって動作が異なる件も記載しています。

## その他もろもろ

### strlenで文字列が空かどうか調べる

GCCで、strlen(pStr) == 0を使って文字列が空かどうか調べると、先頭が0かどうかだけ判定し、文字列の長さは調べないようです。

cFriendsCommon.hとcFriends.cから作られる、cFriends64.sのアセンブリコードを確認すると分かります。main関数のコードを見ると分かります。

```c++
static const size_t LongStringLength = 0xefffffff;

static inline char* CreateLongString(void) {
    const size_t LongStringBufferLength = LongStringLength + 1;
    char* pStr = (char*)(malloc(sizeof(char) * LongStringBufferLength));
    assert(pStr);

    memset(pStr, 'a', LongStringLength);
    pStr[LongStringLength] = '\0';
    return pStr;
}

static inline int IsLongStringEmpty(void) {
    char* pStr = CreateLongString();
    int result = (strlen(pStr) == 0);
    free(pStr);
    pStr = NULL;
    return result;
}

static inline size_t GetLongStringLength(void) {
    char* pStr = CreateLongString();
    size_t length = strlen(pStr);
    free(pStr);
    pStr = NULL;
    return length;
}

int main(int argc, char* argv[]) {
    assert(!IsLongStringEmpty());
    DWORD lengthTime = GetTickCount();
    assert(GetLongStringLength() == LongStringLength);
}
```

```gas
call    CreateLongString     # 返り値のraxは文字列の先頭を指す
movzx   edi, BYTE PTR [rax]  # 先頭の文字を取得する
mov     rcx, rax
call    free()を呼ぶ
test    dil, dil             # 先頭の文字が0かどうか
je      assert失敗時の処理に飛ぶ

call    GetTickCount()を呼ぶ
mov     edi, eax

call    CreateLongString
mov     rbp, rax
mov     rcx, rax
call    strlen()を呼ぶ
mov     rcx, rbp
# 以下略
```

Cygwinでテストを実行すると、以下の処理に掛かった実行時間を順に表示します。上記のアセンブリから推測される通り、長さが0かどうか調べるためにはstrlenを呼ばないのでその処理時間が掛からないことが分かります。1番目と2番目は差がないので、時間の長短が逆転することがあります。

1. 文字列の確保と解放だけ
1. 文字列の確保と解放に加えて、長さが0かどうか調べる
1. 文字列の確保と解放に加えて、長さを調べる

```text
1734, 1735, 2047 [msec]
```

### Singletonとスレッドセーフ

よく知られたSingletonの実装方法として、以下のコードがあります。

```cpp
MySingletonClass& MySingletonClass::GetInstance(void) {
    static MySingletonClass instance(1);
    return instance;
}
```

かつてこの方法はスレッドセーフではない、と言われていました。インスタンスを作ったかどうかのフラグを複数スレッドが同時に確認して、同時に複数のインスタンスができてしまうことがあるからです。C++11ではスレッドセーフになり、Cygwin GCCであれば-fno-threadsafe-staticsオプションを付けなければC++98でもスレッドセーフになります。

makeするとcppFriendsSingleton.s (C++11), cppFriendsSingleton_thread_safe.s (C++98), cppFriendsSingleton_no_thread_safe.s (C++98 -fno-threadsafe-statics)ができますので、_ZN16MySingletonClass11GetInstanceEv に __cxa_guard_acquire があるかどうかご確認ください。

### LTO(Link Time Optimization)

既述の通りmakeを実行すると、LTOを有効にした実行ファイルと、そうでないものを生成します。実行ファイルのシンボルテーブルを確認すると、UnusedFunctionの定義が以下の通りになります。

```bash
$ objdump -x cppFriends_gcc_lto | grep UnusedFunction
[780](sec -1)(fl 0x00)(ty   0)(scl   2) (nx 0) 0x0000000000000000 _Z14UnusedFunctionv

$ objdump -x cppFriends | grep UnusedFunction
[15586](sec  1)(fl 0x00)(ty  20)(scl   2) (nx 0) 0x000000000001d230 _Z14UnusedFunctionv

$ objdump -d cppFriends | less
000000010041e230 <_Z14UnusedFunctionv>:
   10041e230:   31 c0  xor    %eax,%eax
   10041e232:   c3     retq
```

### MinGWで何種類の空白文字を認識するか確認する

くいなちゃんさんによると、Unicodeの空白文字は17種類あるそうです。[ここに](https://twitter.com/kuina_ch/status/816977065480069121)あるものは、サーバで変換されて14種類になっているので、U+00A0を加えた15種類を空白文字として扱うかどうかを、[cppFriendsSpace.cpp](cppFriendsSpace.cpp)で調べます。

コマンドプロンプトから、

```bash
cppFriendsSpace.bat
```

を実行すると、ビルドして実行します。MinGWのインストール先はC:\MinGWに固定していますので、適宜cppFriendsSpace.batを変更してください。Boost C++ Librariesのファイル名が異なる場合(-mtなどがついている)場合も適宜変更してください。

MinGWでは、Boost.Regexの空白文字(\sと[:space:])は、15文字すべてを空白とみなしました。しかしCygwin GCC 6.3.0では12文字しか空白とみなしませんでした。

### ファイルにUS-ASCII以外の文字が含まれないことを確認する

fileコマンドを使うのが簡単です。ファイルの何行目にUS-ASCII以外の文字があるかを表示したければ、下記のようなRubyのワンライナーを書けばよいです。

```bash
$ ruby -ne '$_.ascii_only? ? 0 : (puts "#{$.} : #{$_}" ; abort)' LICENSE.txt ; echo $?
0
$ ruby -ne '$_.ascii_only? ? 0 : (puts "#{$.} : #{$_}" ; abort)' cppFriends.cpp ; echo $?
31 :     // すごーい! シリアライザはクラスを永続化できるフレンズなんだね
1
```

全文ではなく、ソースコードのコメントだけを抽出して、US-ASCII以外の文字が含まれないことを確認したり、文法チェックを掛けたりしたいことがあります。そのためのスクリプトを作りました。

```bash
$ ruby asciiOnlyChecker.rb *.h *.cpp *.md *.pl *.rb Makefile* ; echo $?
1
```

* 調べたいファイル名(複数可)を引数で渡してください。bashに*を展開させてもよいです。
* 指定されたファイルが存在しなかったり、ディレクトリだったりする場合は無視します。バックアップファイル(*~, *.bak)も無視します。
* スクリプトの返り値は、上記で渡したファイルにUS-ASCII以外の文字が含まれない場合は0、含まれるときは非0です。
* このスクリプトに登録されている拡張子のファイル=ソースコードについては、コメントを段落単位で一行にまとめて、ファイル名とその先頭行をつけて標準出力に書き出します(下記参照)。そのまま文法チェッカーに掛けることができます。
* このスクリプトに登録されていないファイルについては、空行を挟まない連続行を一段落として、同様に出力します。
* US-ASCII以外の文字を含む段落には、下記のように、ファイル名の前に警告を付けます。

```text
foo.h : 47
Command line arguments

Non-ASCII characters found in bar.rb : 50
この構造体を含む構造体のbyteサイズ
```

### switch-caseは整数しか振り分けられない

[こちら](switchCase.md)に説明を書きました。Rubyのcase-whenは便利ですね。

### 複数行のマクロを安全に展開する

Compound statementがあることを教えて頂きました。[こちら](expandMacro.md)に説明を書きました。

### ツイッターbotの投稿順序を並び替える

[こちら](shuffleLines.md)に説明を置きました。

### C++/Pythonのコメントを抽出する

英文チェッカーに入力するために、コメントを取り出す[スクリプト](scripts/comment_extractor/comment_extractor.py)を作成しました。//コメント、/* コメント */、#コメント、'''コメント'''、"""コメント"""を抽出します。

### 秘書問題の解を探索する

[秘書問題の最適解](https://mathtrain.jp/hisyomondai)は、自然対数の底の逆数分だけパスすればよいことが証明されています。これを敢えて最適値を探りながらシミュレーションを繰り返すよう、[Rスクリプト](secretaryProblem.R)を作成しました。シミュレーション結果にベイジアン最適化を用いて、次のシミュレーションに使う値を求めています。

### Botの投稿時刻によるツイートインプレッションの差を測る

[こちら](postedTime.md)に説明を置きました。

### 全額再投資と複利

投資先の算術平均の期待値が1を上回るだけでは、全額再投資をした場合に期待値が1を下回ることがあります。[Rでシミュレーション](interest.md)してみました。

### 8-bit符号無し整数の十進数表記

8-bit符号無し整数(0..255)を十進数表記にしたときの、百、十、一の位を求めるという話題があったので[求めてみました](binToDigits.md)。

### 単語をx86ニーモニックに変換する

Coffeeのような単語をを[HEXで表現する](https://twitter.com/kuina_ch/status/1017931384998936576)というのはよくある遊びですが、x86の機械語として成立するかどうか調べるスクリプトを作りました。CoffeeはSAR命令だったのですね。

```bash
$ ruby convert_opcode.rb coffee
c0,ff,ee  sar bh,0xee
```

### C++で負の二項分布を作ると、sizeに整数しか指定できないことがある

[こちら](scripts/stock_price/negative_binomial_cpp.md)に説明を置きました。

## C++の一般的な情報源

これらに記載されていることをすべて本ページに書くわけにもいきませんので、自分でC++のコードを書いていて、特に気になることだけを随時上記にまとめています。

* [私が読んだ書籍](https://github.com/zettsu-t/zettsu-t.github.io/wiki/Books)
* [Boost C++ Libraries](http://www.boost.org/)
* [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
* [More C++ Idioms](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms)
* [Intel 64 and IA-32 Architectures Software Developer Manuals](https://software.intel.com/en-us/articles/intel-sdm) and [Intel 64 and IA-32 Architectures Optimization Reference Manual](http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-optimization-manual.html)
* [Use reentrant functions for safer signal handling](https://www.ibm.com/developerworks/library/l-reent/index.html)
* http://stackoverflow.com/ などの各記事

## Rの一般的な情報源

[Pythonプログラマが30分で分かるR](https://github.com/zettsu-t/Rin30minutes) を執筆中です。当レポジトリに置いてある元のファイル (r_in_30min.Rmd) はこれ以上更新しません。

* みんなのR 第2版 (Jared P. Lander 著/高柳慎一, 津田真樹, 牧山幸史, 松村杏子, 簑田高志 監修, 2018/12, マイナビ出版)
* パーフェクトR (Rサポーターズ 著, 2017/03, 技術評論社)
* 再現可能性のすゝめ ―RStudioによるデータ解析とレポート作成― (Wonderful R 3) (高橋康介 著/石田基広 監修/市川太祐, 高橋康介, 高柳慎一, 福島真太朗, 松浦健太郎 編, 2018/05, 共立出版)
* Rクックブック 第2版 (J.D. Long, Paul Teetor 著/大橋真也 監訳/木下哲也 訳, 2020/01, オライリー・ジャパン)
* [Rの公式マニュアル](https://cran.r-project.org/manuals.html)
* [R for Data Science by Garrett Grolemund and Hadley Wickham](https://r4ds.had.co.nz/)
* [Advanced R by Hadley Wickham](https://adv-r.hadley.nz/)
* [R packages by Hadley Wickham](http://r-pkgs.had.co.nz/)

## ライセンス

本レポジトリのライセンスは、[MITライセンス](LICENSE.txt)です。
