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

## やめるのだフェネックで学ぶC++

「やめるのだフェネック」あるいは「フェネックやめるのだ」がどこからきたのかよく分からないのですが、もしかして元ネタは[シェーン、おいやめろ！](http://labaq.com/archives/51829457.html)なのでしょうか。

1. やめるのだフェネック! enumを何でもintにキャストしてはいけないのだ! std::underlying_typeを使うのだ!
1. フェネックやめるのだ! XMMレジスタの値を、素のuint64_t[]に保存してはいけないのだ! 16 bytes アライ~~さん~~ンメントが必要なのだ! 単にreinterpret_castするだけじゃダメなのだ!
1. (そろそろしつこいので以下同文) x64 ABIで、asmコードからCの関数を呼び出すときは、rspレジスタを16 bytes境界に合わせないといけないのだ! そうしないと、C++ライブラリの中で突然クラッシュすることがあるのだ!
1. x86で、intを一度に32ビット以上シフトするのはダメだ! REX.Wがないときのシフト回数は有効桁数が5ビットしかないのだ!
1. ```int64_t v = 1 << 32;``` はint64_tではなくintをシフトしているのだ! 0x100000000ではなく0が入るかもしれないのだ! int64_tの変数に代入してから<<=でシフトするんだ!
1. 配列をループで回すときのインデックスを何でもintにしてはいけないのだ! sizeof(size_t) > sizeof(int)だと4Gあたりで動作がおかしくなることがあるのだ!
1. 固定アドレスの格納先を、uint32_tとuint64_tとで、#ifdefで切り替えるのはやめるのだ! uintptr_tを使うのだ!
1. ポインタの差を、intに入れるのは嫌なのだ! ptrdiff_tを使うのだ! 符号ありだから、printfは"%td"を使うのだ!
1. size_tのビット数が分からないからといって、printfの書式指定に%luと書くのはダメだ! %zuと書くのだ! MinGW-w64 gccだとランライムが%zuを解釈しないからコンパイルになるって、それなら仕方ないのだ...
1. 組み込み32bit CPU向けコードのユニットテストを、x86 Google Testで書くのに、x86_64コンパイラは要らないのだ! size_tの違いを吸収するのは大変なのだ! target=i686だけインストールして、x86_64 multilibはインストールしないのだ!
1. sizeofに型名を入れてはいけないのだ! 変数の型が変わった時オーバランするのだ! sizeof(*pObject)とすれば、ポインタpObjectが指すもののサイズが得られるのだ!
1. sizeofにリテラルを渡すのはやめるのだ! sizeof('a')はCとC++で違うのだ! [(参考)](http://david.tribble.com/text/cdiffs.htm#C99-char-literal)
1. 「どんな型の関数へのポインタでも入る物」として、void*を使うのはやめるのだ! データへのポインタとコードへのポインタは互換ではないのだ!  [(参考)](http://stackoverflow.com/questions/5579835/c-function-pointer-casting-to-void-pointer) (boost::anyなら [Value typeの要件](http://www.boost.org/doc/libs/1_63_0/doc/html/any/reference.html#any.ValueType) は満たしているはずですが)
1. 関数への参照は、&をつけるのとつけないのと使い分けるのだ! テンプレートマッチングに失敗することがあるのだ!
1. 同じx86 CPUだからって、64ビットアプリと32ビットアプリで、浮動小数が同じ計算結果を返すと仮定してはだめなのだ! SSEは内部64ビットだが、x87は内部80ビットで計算しているのだ! [(参考)](http://blog.practical-scheme.net/shiro/20110112-floating-point-pitfall) 数の比較結果が前者は==で後者が!=になることがあるのだ! [(例)](cFriends.c)
1. 浮動小数をprintf("%.0e")して7文字(-1e-308)で収まると、決め打ちするのはやめるのだ! -infinityと表示するときは9文字なのだ!
1. std::wstring_convertによるUTF-8/16変換は、例外を捕捉するのだ! わざと冗長にUTF-8エンコードした文字列を入力すると、std::range_errorが飛ぶことがあるのだ!
1. ビットフィールドを上から下に並べても、MSBから順に並ぶとは限らないのだ! エンディアンとコンパイラの仕様を確認するのだ!
1. unionを使ってstructをuint8_t[]に読み替えるのは、実行時にはできても、constexprでコンパイル時にできるとは限らないのだ!
1. コメントに「この変数は符号なしのはず」とか書いてはいけないのだ! static_assert(std::is_unsigned)を書くのだ!
1. ```do { ... } while(--i >= 0); ```は、iが符号なし整数だと永久に終わらないのだ! プロセスの危機なのだ!
1. マクロに複数の文を入れるときは、```do { ... } while(0) ```で囲むのだ! そうしないと、if文の直後でそのマクロを使ったときに、予想外の動作をすることがあるのだ!
1. 非PODのオブジェクトをmemcpyしてはいけないのだ! memmoveもダメだ! vtableへのポインタもコピーされてしまうのだ! 派生クラスのメンバが切り捨てられて不定値に置き換わってしまうのだ! clang++ 4.0.0は警告してくれるが、g++ 6.3.0は警告しないのだ!
1. クラスにoffsetofを取ると警告が出るのだ! Non standard layout型に対してoffsetofを取る意味があるのか考えるのだ!
1. memsetを使ってbyte単位以外の値でメモリを埋めるのは無理なのだ! std::fillを使うのだ!
1. snprintf(dst,N,"%s")で長さNのときにdstにN文字書き込もうとすると最後はNUL終端されるが、strncpyで長さNのときにN文字コピーしようとすると最後は終端されないのだ! バッファオーバランの危機なのだ!
1. thisと引数が同じオブジェクトかどうか(二つの引数が同じオブジェクトかどうか)確かめずに、片方から他方にメンバをコピーするのはやめるのだ! memcpyで領域が重なっているときのように要素が消滅してしまうのだ! [(NaiveCopy)](cppFriends.cpp)
1. 確かにC99の機能はC++でも使えるが、restrictはコンパイルエラーになることがあるのだ! 本当にrestrictが必要か考えるのだ!
1. 立っているビット数をfor文で数えるのは遅いのだ! コンパイラのマニュアルから __builtin_popcount とかを探すのだ!
1. __builtin_popcountの引数はunsigned intなのだ! long long intを渡すと正しい答えを返さないことがあるのだ! テンプレートではないのだ! __builtin_popcountllとかもみるのだ!
1. ```__attribute__((always_inline))```は常にインライン展開できるとは限らないのだ! 再帰呼び出しはインライン展開できないのだ! そもそもinlineはヒントであって命令ではないのだ!
1. Strict aliasing rule警告の意味が分からないからって無視してはいけないのだ! [(参考)](http://dbp-consulting.com/tutorials/StrictAliasing.html) そもそもエンディアン変換なら、自作しないでntohlとかBoost.Endianとか探すのだ!
1. pragmaで警告を抑止してよいのは、コードレビューで承認されてからだ! -Wall -Werror は必須なのだ!
1. コンパイラの警告を無視した箇所を、静的解析ツールに指摘されるのはやめるのだ! コンパイラが数秒で教えてくれることを、翌朝に教わるのは開発効率が低すぎるのだ!
1. /* Local Variables: c-file-style: "stroustrup" */ を理解できないからって消さないで欲しいのだ! それはEmacs上でソースコードを整形するのに必要なのだ!
1. 2個のオブジェクトを交換するコードを自作してはいけないのだ! std::swapはno throw保証なのだ!
1. 出力ファイルストリームのcloseを、いつでもデストラクタ任せにすると、closeで書き出しに失敗したことを検出できないのだ! デストラクタはnoexceptだから呼び出し元に結果を通知できないのだ!
1. 実行環境を確認せずに、いきなりnoexceptと書かないで欲しいのだ! 例外中立にして欲しいのだ! MinGW32 + pthreadGCE2.dll + clangだと、[pthread_exit](https://github.com/Tieske/pthreads-win32/blob/master/pthreads.2/pthread_exit.c)が例外を投げて、[スレッドエントリ関数](https://github.com/Tieske/pthreads-win32/blob/master/pthreads.2/ptw32_threadStart.c)が拾うまでに、noexcept違反でstd::terminateされてしまうのだ! (pthreadGCE-3.dllではこうならず、スレッドを正常に終了できます)
1. 何もしないデストラクタを{}と定義するのはダメだ! =defaultを使うのだ! 理由はEffective Modern C++ 項目17に書いてあるのだ!
1. ユニットテストが書きにくいからって、#defineでprivateをpublicに置き換えちゃいけないのだ! アクセス指定子を超えたメンバ変数の順序は入れ替わることがあるのだ!  [(参考)](http://en.cppreference.com/w/cpp/language/access) friendを使うのだ!
1. メンバ変数名の目印のアンダースコアは、名前の先頭につけちゃいけないのだ! _で始まり次が英大文字の名前はC++処理系の予約語なのだ!
1. 短絡評価の||を「または」と読まないのだ! 「さもなくば」と読むのだ! &&は「だったら」「なので」と読むのだ!
1. メンバ変数を増やしたとき、複数あるコンストラクタすべてに、そのメンバ変数の初期化を加えるのを忘れちゃいけないのだ! 可能ならメンバ変数の定義と併せて初期化して、それ以外の初期化方法だけコンストラクタに書けばよいのだ!
1. コードにstd::coutを直書きしてはいけないのだ! ユニットテストが書けないでないか! std::ostreamへの参照を渡すのだ! もちろんstd::cinもだ! ユニットテストでキー入力するのは大変なのだ!
1. コンテナの要素の型をソースコードにべた書きしたら、コンテナの型を変えた時に修正が大変なのだ! std::vector::value_type と auto と decltypeがあるじゃないか!
1. 配列の要素数を ```#define arraySizeof(a) (sizeof(a)/sizeof(a[0]))``` で数えるのはやめるのだ! aにポインタを渡すと、エラーにならずに変な値が返ってくるのだ! テンプレートとconstexprを使うのだ!
1. ```#if (sizeof(uintptr_t) > 4)```とは書けないのだ! ```if constexpr (sizeof(uintptr_t) > 4)```が使えるようになるのを待つのだ!
1. いくらマクロより関数テンプレートの方がいいからって、 ```#define WARN(str) printf("%s at %d", str, __LINE__)``` は関数にはできないのだ! WARNを呼び出した場所ではなく、WARNを定義した場所の行番号が表示されてしまうのだ!
1. 関数の動作を```#ifdef COLOR ... #endif```で切り替えると、COLOURと打ったときに```...```が除外されてしまうのだ! if (定数式)が使えるならそうするのだ! コンパイラが綴りの違いを見つけてくれるのだ!  C++17ではif constexprが使える(予定な)のだ!
1. テンプレートマッチングをstd::is_pointerだけで済ましてはいけないのだ! 配列T(&)[SIZE]とstd::is_null_pointerに対するマッチングも必要なのだ!
1. f(uint8_t)とf(BYTETYPE)とf(unsigned char)を同時には定義できないのだ! 関数を再定義してますと言われてしまうのだ! プログラマには違う型に見えても、コンパイラには区別がつかないのだ!
1. 関数の動作を何でもかんでもboolの引数で切り替えたら、呼び出す側のコードを読んでtrueとかfalseとか書いてあっても、何をしたいか分からなくなるのだ! enum classでパラメータに名前を付けるのだ!
1. 「みんながインクルードしているヘッダファイルの定義を書き足したらフルビルド」を避けるのだ! 宣言と定義を分離するのだ! enum classの前方宣言を活用するのだ!
1. タイピングが大変だからって、using namespace std;って書いちゃいけないのだ! boostと衝突したらどうするのだ! もうすぐC++17でstd::anyとstd::optionalがくるんだぞッ!
1. そのboost::regexをstd::regexに置き換えるのはやめるのだ! その再帰正規表現は入れ子になった括弧を、一番外側の括弧ごとに分けるのだが、std::regexは再帰正規表現をまだサポートしていないのだ!
1. boost::anyオブジェクトにchar*型の値を入れたとき、 boost::any_cast<char *> ではなく boost::any_cast<const char *> で取り出すのはやめるのだ! boost::bad_any_castが飛んでくるぞ! typeidとは違って、constを無視しないのだ!
1. 基底クラスと派生クラスのtypeidは異なるのだ! 素直にmarker interfaceを使うのだ!
1. 空の構造体がたくさんあるからって、typedefで全部まとめて一つにするのはやめるのだ! それらはBoost.MultiIndexのタグなのだ!
1. ラムダ式の型をtypeid().name()で取得しようとするのは無駄なのだ! それはコンパイラが一意なものを決めるのだ!
1. 関数の返り値型をautoにしたら、return vec[i]でベクタの要素への参照は返せないのだ! 要素のコピーが値渡しされるのだ! そこはdecltype(auto)が必要なのだ!
1. そのreturn文の括弧は余計なのだ! 返り値がdecltype(auto)のときにreturn (v);すると、ローカル変数vへの参照を返すのだ! 未定義動作の危機なのだ!
1. ```__PRETTY_FUNCTION__``` は「かわいい」関数名じゃないのだ! 「見た目がきれい」な関数名なのだ!
1. 実行権を取れるまでセマフォを待つのはacquireなのだ! Getと書くとセマフォのインスタンスを取得すると読めてしまうのだ! 獲ると取るは違うのだ!
1. 関数名を付けるときは辞書をひいて欲しいのだ! 間違った英単語も困るが、ローマ字はもっと困るのだ!
1. その代入は最適化で削除されるかもしれないのだ! メモリとレジスタに書きにいかないのだ! デバッグビルドでは動作してもリリースビルドでは正しく動かないのだ! volatileをつけるのだ!
1. マルチコア環境では、volatileでスレッド間共有変数の同期は取れないのだ! std::atomicが必要なのだ! 競合動作の危機なのだ! [(Counter)](cppFriends.cpp)
1. std::atomic<int>は明示的に初期化する必要があるのだ! 初期化を忘れてもコンパイラは教えてくれないのだ!
1. autoは便利だが万能ではないのだ! std::atomic<int> aから値を取り出すのに、auto v=a;とは書けないのだ! atomicなオブジェクトをコピーしようとして失敗するのだ! 明示的な型に代入するか、a.load()が必要なのだ!
1. volatile T*へのキャストをどう書く分からないからって、Cキャストを使っちゃいけないのだ! const_castを使うのだ!
1. constメンバ関数からメンバ変数を書き換えたくなったからといって、いきなりmutableとかconst_castとかしちゃいけないのだ! 呼び出し側はスレッドセーフを期待しているのだ!
1. スレッドセーフと再入可能は違うのだ! 複数のスレッドからmallocは呼べるかもしれないが、割り込みハンドラからmallocは呼べないのだ! ヒープ構造の危機なのだ!
1. VLAがスタックに置かれるとは限らないのだ! ヒープに確保する[コンパイラ](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0472k/chr1359124223721.html)もあるのだ! 割り込みハンドラからmallocは呼べないことを思い出して欲しいのだ!
1. 割り込みハンドラで自動変数のオブジェクトを生成するとき、コンストラクタがnewするのは困るのだ! メンバ変数と他のメンバ関数についても確認するのだ!
1. コメントに「このクラスのインスタンスはヒープに置かないでください」と書いて済ますのは嫌なのだ! operator newを=deleteして欲しいのだ!
1. グローバル変数のコンストラクタから、別のグローバル変数のインスタンス関数を呼び出してはいけないのだ! グローバル変数の初期化順序はコンパイラが決めるから、呼び出し先は未初期化かもしれないのだ! Construct on first useイディオムが必要なのだ!
1. static T& GetInstance(void) { static T instance; return instance; } はマルチスレッド環境ではシングルトンにならないことがあるのだ! シングルトンの実装方法をよく確認するのだ!
1. boost/thread/future.hppなどをインクルードする.cppファイルで、Intel Syntaxのインラインアセンブリを使うと、アセンブラがエラーを出すことがあるぞ! Intel Syntaxでインラインアセンブリを記述するなら、その.cppファイルは他と分けた方がいいぞ!
1. CreateInstance()がいつでも生ポインタを返したら、誰がdeleteするかわからなくなって、メモリリークしたり二重解放したりするかもしれないのだ! deleteして欲しければ、std::unique_ptrを返すことを検討するのだ! 生ポインタは所有権を渡さないという意志なのだ!
1. abi::__cxa_demangleが返したものはfreeしないと[メモリリークするのだ!](https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a01696.html) 誰がメモリを解放するか、仕様を確認するのだ!
1. 引数としてconst T* pObjectを渡すと、ポインタpObjectが指すオブジェクトはimmutableとして扱われるが、deleteはできるのだ! deleteされたくなければ、デストラクタを非publicにするのだ!
1. thread_localを使う前によく考えるんだ! 特定のスレッドしか参照しない値は、スレッド起動時の引数で参照できるのだ! アライさんはその辺ばっちりなのだ!
1. pthread_t型の変数を、pthread_createを呼び出す前に初期化はできないのだ! pthread_tの型はopaqueで、CygwinはポインタでMinGWはstructだから、ユーザが設定できる初期値などないのだ!
1. 文字列の集合を連結するのに、```boost::algorithm::join(lines, " ");```で連結しちゃ嫌なのだ! 英語はこれが正しいけど、日本語だと「弁慶がな ぎなたを振り回し」になってしまうのだ! 文字列の末尾がUS-ASCIIかどうかで、空白を入れるか決めて欲しいのだ!
1. ファイルをShift_JISで保存するのはやめるのだ! //コメントが「でゲソ」とか「可能」とかで終わると、次の行もコメント扱いされてコンパイルされなくなってしまうのだ!
1. ユニットテストを書くときは、いきなりテストを成功させてはいけないのだ! でないと、テストに成功したのか、そのテストを実行していないのか、区別がつかなくなるぞ! まずテストを実行して失敗することを確かめるのだ!
1. ユニットテストに共通の初期化は、Ruby test/unitはsetupで、CppUnitはsetUpで、Google TestはSetUpなのだ! 大文字小文字に注意するのだ! overrideをつければコンパイラが間違いに気づいてくれるのだ!
1. インラインアセンブリのシフト命令でCLレジスタを使ったら、破壊レジスタに書かないとダメなのだ! RCXレジスタは第一引数が入っているのだ! Segmentation faultの危機なのだ!
1. .sファイルのアセンブリマクロをテストできないと諦めないのだ! 単にマクロを展開して、C++のインラインアセンブリからcallすればテストできるのだ!
1. x86でNOP命令を「何クロックか待つ」ためには使えないのだ! Z80とは違うのだ! NOPは命令をアラインメントするために使うのだ!
1. アセンブリの細かい処理を高速化するときは、何マイクロ秒経ったかだけでなく、何クロック掛かったも測るのだ! 今どきのCPUは可変クロック周波数なのだ!
1. メンバ関数にYieldという名前は使えないのだ! winbase.hでYieldを「何もしない」マクロと定義しているのだ! 理解不能なエラーメッセージの危機なのだ!
1. .cppから.dファイルを作るときに、自動生成する予定のファイルが見つからない、とエラーが出るのは困るのだ! -MGオプションが必要なのだ!
1. .hppからモックなどを作るときに、自動生成する予定のファイルがインクルードできないのは困るのだ! ```__has_include```はまだ早いのだ!
1. 単に速い処理をリアルタイムと呼んじゃ嫌なのだ! リアルタイムシステムとは、結果の正しさが内容だけでなく、締め切りに間に合ったかどうかにも依存するシステムなのだ! Stop the world GCはいつ終わるか分からないから困るのだ!

この語り口はあくまでネタなので、普段の私はもっと柔らかい口調で話しています、念のため。

## ライセンス

本レポジトリのライセンスは、[MITライセンス](LICENSE.txt)です。

ちなみに、けものフレンズ公式には[二次創作に関するガイドライン](http://kemono-friends.jp/)があります。私にはあいにく絵心がないです。

## C++の一般的な情報源

これらに記載されていることをすべて本ページに書くわけにもいきませんので、自分でC++のコードを書いていて、特に気になることだけを随時上記にまとめています。

* [私が読んだ書籍](https://github.com/zettsu-t/zettsu-t.github.io/wiki/Books)
* [Boost C++ Libraries](http://www.boost.org/)
* [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
* [More C++ Idioms](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms) 
* [Intel 64 and IA-32 Architectures Software Developer Manuals](https://software.intel.com/en-us/articles/intel-sdm) and [Intel 64 and IA-32 Architectures Optimization Reference Manual](http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-optimization-manual.html)
* http://stackoverflow.com/ などの各記事

## おまけ

アニメ「けものフレンズ」の主題歌「ようこそジャパリパークへ」を歌っているのは"どうぶつビスケッツ×PPP"ですが、PPPという単語から思い出すのは、Point-to-Point Protocolでも購買力平価説でもなく、アニメ「アイドルマスターシンデレラガールズ」に出てくる架空のファッションブランドPikaPikaPopだったりします。PikaPikaPopをPPPと略しているシーンはアニメにないはずですが、それ言ったら「フレンズなんだね」も本編になかったはずですし...
