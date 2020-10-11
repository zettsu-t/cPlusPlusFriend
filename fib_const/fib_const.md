# フィボナッチ数列をコンパイル時計算する

非負の整数$n$について、フィボナッチ数列を以下のように定義する。

$$
\begin{eqnarray}
fib(n)=\left\{ \begin{array}{ll}
0 & (n=0) \\
1 & (n=1) \\
fib(n-1) + fib(n-2) & (n > 1) \\
\end{array} \right.
\end{eqnarray}
$$

最初の10個を求めてみよう。Rだとこう書ける。

```R
library(purrr)
purrr::reduce(.x=1:10, .init=c(), .f=function(v, i) {
    if (NROW(v) == 0) {
        c(0)
    } else if (NROW(v) == 1) {
        c(v, 1)
    } else{
        c(v, sum(tail(v, 2)))
    }
})
```

10番目つまり、fib(9)は34である。

```text
0  1  1  2  3  5  8 13 21 34
```

fib(9)をC++でコンパイル時計算しよう。

```c++
#include <cstdio>
#include <array>
#include <vector>

using LongInt = long long int;
constexpr LongInt fib(LongInt n) {
    if (n == 0) {
        return 0;
    } else if (n == 1) {
        return 1;
    }
    return fib(n-1) + fib(n-2);
}

int main(int argc, char* argv[]) {
    std::printf("%lld\n", fib(9));
    return 0;
}
```

```bash
g++ -std=gnu++2a -O3 -Wall -O3 -o fib_const_base fib_const.cpp
./fib_const_base
34
```

Cygwin g++ 10.2.0でコンパイルした結果の、アセンブリを抜粋して示す。fibの関数定義はなく、fib(9)は定数34 (0x22) に置き換えられる。

```asm
<main>:
sub    rsp,0x28
call   100401090 <__main>
mov    edx,0x22            # fib(9)の結果
lea    rcx,[rip+0x195b]    # printf書式
call   1004010a0 <printf>
xor    eax,eax
add    rsp,0x28
ret
```

さて、fib(-1)を求めるとどうなるだろうか。実行時にfib(-1)を求めようとして segmentation fault で落ちる。

```asm
<main>:
sub    rsp,0x28
call   100401090 <__main>
mov    rcx,0xffffffffffffffff       # -1
call   100401690 <fib(long long)>   # fib(-1)呼び出し
lea    rcx,[rip+0x14d4]             # printf書式
mov    rdx,rax
call   1004010a0 <printf>
xor    eax,eax
add    rsp,0x28
ret
```

gdbで挙動を見よう。どうやら2 Mbytes近いスタック領域を使い果たしたらしい。

```text
$ gdb ./fib_const_bad
(gdb) b main
(gdb) r
(gdb) info registers
rbp            0xffffcce0          0xffffcce0
rsp            0xffffcbd0          0xffffcbd0
(gdb) c
Thread 1 "fib_const_bad" received signal SIGSEGV, Segmentation fault.
0x00000001004017a6 in fib(long long) ()
(gdb) info registers
rbp            0x0                 0x0
rsp            0xffe03fd0          0xffe03fd0
```

つまりコンパイラはfib(9)をコンパイル時に求めるが、fib(-1)をコンパイル時に求められなかったので諦めたようである。

ところでfib(90)=2880067194370816120なので、64-bit整数で表現できるが、コンパイラはどう計算するだろうか。フィボナッチ数列の定義に従った素朴な計算を行うと、いつまで経っても計算が終わらない。これは下記のコードで、 ${fib}(n-2)$ を計算するために、既に計算した ${fib}(n-3)$ を再利用しないために、計算時間が $O(2^n)$ 掛かるからである。

```c++
int main(int argc, char* argv[]) {
    constexpr LongInt n = 90;  // fib(90) = 2880067194370816120
    std::printf("%lld\n", fib(n));
    return 0;
}
```

ところがテンプレートメタプログラミングを行うと、計算結果を覚えているのでfib(90)を実用的な時間でコンパイルできる。ただしテンプレート再帰の深さは制限があるので、深すぎるとコンパイルエラーになる。

```c++
template<LongInt n> constexpr LongInt fibt() {
    return fibt<n-1>() + fibt<n-2>();
}

template<> constexpr LongInt fibt<1>() {
    return 1;
}

template<> constexpr LongInt fibt<0>() {
    return 0;
}

int main(int argc, char* argv[]) {
    std::printf("%lld\n", fibt<n>());
    return 0;
}
```

結果がfib(90)の計算結果の定数に置き換わっているのが分かる。

```asm
<main>:
sub    rsp,0x28
call   1004010a0 <__main>
...
lea    rcx,[rip+0x1866]         # printf書式
movabs rdx,0x27f80ddaa1ba7878   # fib(90) = 2880067194370816120
call   1004010c0 <printf>
```

メモ化する、つまり副作用のない関数については、関数に引数を適用した結果を覚えているプログラミング言語で試すと面白いだろう。

C++20ではコンテナに対してconstexprできるようになった。Cygwin g++ 10.2.0 -std=gnu++2a で以下のコードをコンパイルすると、コンパイル時に計算される。std::arrayの代わりにstd::vectorを使うと、コンパイル時計算されず実行時に計算される。

```c++
template<size_t n> constexpr LongInt fibarray() {
    std::array<LongInt, n+1> v;
    for(decltype(n) i{0}; i<=n; ++i) {
        if (i == 0) {
            v.at(i) = 0;
        } else if (i == 1) {
            v.at(i) = i;
        } else if (i > 1) {
            v.at(i) = v.at(i-1) + v.at(i-2);
        }
    }
    return v.at(n);
}

int main(int argc, char* argv[]) {
    std::printf("%lld\n", fibarray<static_cast<size_t>(n)>());
    return 0;
}
```

```asm
<main>:
...
lea    rcx,[rip+0x1890]         # printf書式
movabs rdx,0x27f80ddaa1ba7878   # fib(90) = 2880067194370816120
call   1004010c0 <printf>
```

-std=gnu++14 で上記をコンパイルすると、普通の関数呼び出しになる。

```asm
<main>:
...
call   1004016b0 <long long fibarray<90ul>()>
lea    rcx,[rip+0x182b]        # printf書式
mov    rdx,rax                 # fib(90)の実行結果
call   1004010c0 <printf>
```
