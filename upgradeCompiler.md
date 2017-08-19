# コンパイラのバージョンによる違い

## MinGW GCC 7.1.0の制限事項

MinGW-w64 Distro 15.0 (GCC 7.1.0)でmakeできるようにしました。ただし以下の制限事項があります。

- GCCのLTOは実行ファイルを生成できるが、実行できない
- スレッドとネットワーク処理をコンパイル/リンクしない(MinGWでstd::thread, arpa/inet.hが使えないため)
- clang++でC++例外が使えない。Boost C++ LibrariesもC++例外を使えないので、boost::throw_exceptionを定義している。
- snprintfは、文字列がバッファに収まらないとき-1を返す(Cygwinはバッファが十分長ければ収めるはずの文字数を返す)
- TEST_F(TestRegex, ReDos) で複雑な正規表現を処理するのが終了しない(Cygwinは例外を投げて終了する)
- ::isasciiが使えない。代わりに__isasciiを使う。
- boost::multiprecision::cpp_intが使えない(実行時に異常終了する)
- CPU除算例外が発生したとき、処理が先に進まずプロセスが終了しない(Cygwinはプロセスが終了するのでDeathTestできる)

## Cygwin GCC 5.4.0から6.3.0にアップグレード

Cygwin GCC 5.4.0を6.3.0に変更したことに伴い、本レポジトリのソースコードを変更しないと、コンパイルおよびテストに失敗しました。

### C++標準の違い

GCC6.3.0で-stdが無いときは、-std=gnu++14になります。-std=gnu++98にするときは明示する必要があります。

### 定数式の扱い

GCC6.3.0では下記のテンプレートについて、T=uint64_tは定数式になりますが、T=int64_tは定数式になりません。

```c++
template <typename T>
constexpr int MyNumericLimitsDigits10_A(T a, int digits) {
    // 整数オーバフローを意図的に起こす
    auto n = a * 10 + 9;
    return (n > a) ? MyNumericLimitsDigits10_A(n, digits + 1) : digits;
}

template <typename T>
constexpr int MyNumericLimitsDigits10_A(void) {
    return MyNumericLimitsDigits10_A<T>(9,1);
}
```

GCC6.3.0では上段はコンパイルできますが、下段はコンパイルエラーになります。GCC5.4.0ではいずれもコンパイルできます。符号あり整数のオーバフローに対する動作は未定義ですので、GCC6.3.0は定数が求まらないとしてエラーにするようです。

```c++
static_assert(MyNumericLimitsDigits10_A<uint64_t>() == 19, "");
static_assert(MyNumericLimitsDigits10_A<int64_t>() == 18, "");
```

整数オーバフローを発生させないように書き換えるとコンパイルできます。

```c++
template <typename T>
constexpr int MyNumericLimitsDigits10_B(T a, int digits) {
    auto n = a / 10;
    return (n) ? MyNumericLimitsDigits10_B(n, digits + 1) : digits;
}

template <typename T>
constexpr int MyNumericLimitsDigits10_B(void) {
    // std::is_signed
    using BitWidth = decltype(sizeof(T));
    constexpr T zero = 0;
    constexpr T preZero = zero - static_cast<T>(1);
    constexpr BitWidth isSigned = (zero > preZero) ? 1 : 0;

    T maxNumber = 0;
    constexpr BitWidth shift = sizeof(T) * 8 - isSigned;
    for(BitWidth i=0; i<shift; ++i) {
        // maxNumber *= 2 は -Wconversionで警告が出る
        maxNumber = static_cast<T>(maxNumber * 2);
        ++maxNumber;
    }
    return MyNumericLimitsDigits10_B<T>(maxNumber,0);
}
```

### ムーブコンストラクタ/ムーブ代入演算子

ムーブコンストラクタ/ムーブ代入演算子を=defaultするとき、GCC 6.3.0ではprivate, protectedにするとエラーになります。publicにするとエラーになりません。GCC5.4.0ではprivateにしてもコンパイルできます。

```c++
private:
    Train(Train&&) = default;
    Train& operator =(Train&&) = default;
```
