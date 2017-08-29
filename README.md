# やめるのだフェネックで学ぶC++

「やめるのだフェネックで学ぶシリーズ」の慣例に従って、C++およびソフトウェア開発でやっていいことと悪いことをまとめました。

## C++でやっていいこと悪いこと

[cppDosAndDonts.md](cppDosAndDonts.md)にまとめました。この語り口はあくまでネタなので、普段の私はもっと柔らかい口調で話しています、念のため。

### フェネックとアライさんのやり取りを一通り実行する

Cygwin 64bitのターミナルから、

```bash
make
```

を実行すると、一通りテストをビルドして実行します。最後はコンパイルエラーで終わりますが、これはコンパイルエラーを意図的に再現しているものです。

当方の実行環境は以下の通りです。Google Test / Mockは$HOME直下にあると仮定していますので、それ以外の場合はMakefileを変更してください。AVX命令を使用することが前提ですので、サポートしていないプロセッサの場合は、Makefileの _CPPFLAGS_ARCH_ をコメントアウトしてください。

* Windows 10 Creators Update 64bit Edition
* Cygwin 64bit version (2.8.0)
* Google Test / Mock (1.7.0)
* Boost C++ Libraries (1.63.0)
* gcc (6.3.0)
* clang (4.0.1)
* Ruby (2.3.3p222)

RDTSC命令の下の桁に偏りがある、という判定は実行環境によっては失敗するようです。何回か試して失敗するようでしたら、閾値を期待値に寄せるか、諦めてコメントアウトしてください。

### MinGWで実行する

[こちら](upgradeCompiler.md)にまとめました。Cygwinでは実行するが、MinGWでは実行しないテストがあります。コンパイラのバージョンによって動作が異なる件も記載しています。

## その他もろもろ

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

MinGWでは、Boost.Regexの空白文字(\sと[:space:])は、15文字すべてを空白とみなしました。しかしCygwinでは12文字しか空白とみなしませんでした。

### ファイルにUS-ASCII以外の文字が含まれないことを確認する

fileコマンドを使うのが簡単です。ファイルの何行目にUS-ASCII以外の文字があるかを表示したければ、下記のようなRubyのワンライナーを書けばよいです。

```bash
$ ruby -ne '$_.ascii_only? ? 0 : (puts "#{$.} : #{$_}" ; abort)' LICENSE.txt ; echo $?
0
$ ruby -ne '$_.ascii_only? ? 0 : (puts "#{$.} : #{$_}" ; abort)' cppFriends.cpp ; echo $?
31 :     // すごーい! シリアライザはクラスを永続化できるフレンズなんだね
1
```

### switch-caseは整数しか振り分けられない

[こちら](switchCase.md)に説明を書きました。Rubyのcase-whenは便利ですね。

### 複数行のマクロを安全に展開する

Compound statementがあることを教えて頂きました。[こちら](expandMacro.md)に説明を書きました。

### ツイッターbotの投稿順序を並び替える

[こちら](shuffleLines.md)に説明を置きました。

## C++の一般的な情報源

これらに記載されていることをすべて本ページに書くわけにもいきませんので、自分でC++のコードを書いていて、特に気になることだけを随時上記にまとめています。

* [私が読んだ書籍](https://github.com/zettsu-t/zettsu-t.github.io/wiki/Books)
* [Boost C++ Libraries](http://www.boost.org/)
* [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
* [More C++ Idioms](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms)
* [Intel 64 and IA-32 Architectures Software Developer Manuals](https://software.intel.com/en-us/articles/intel-sdm) and [Intel 64 and IA-32 Architectures Optimization Reference Manual](http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-optimization-manual.html)
* [Use reentrant functions for safer signal handling](https://www.ibm.com/developerworks/library/l-reent/index.html)
* http://stackoverflow.com/ などの各記事

「やめるのだフェネック! たとえ英語が嫌いでも、プログラマに英語は必要なのだ! 英語が読み書きできて、語彙が十分でないと、stackoverflow.com で解決策を調べられないのだ! 」

## ライセンス

本レポジトリのライセンスは、[MITライセンス](LICENSE.txt)です。

ちなみに、けものフレンズ公式には[二次創作に関するガイドライン](http://kemono-friends.jp/)があります。私にはあいにく絵心がないです。
