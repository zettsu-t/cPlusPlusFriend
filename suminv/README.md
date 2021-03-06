# 逆数の和をたす

整数 1..N の逆数を順に足して、与えられた数 M 以上になる最小の N を求める。 1/nの積分はlog(n)なので順に足すより速い方法がありそうだが、SIMD命令のプログラミング演習として順に足す。

## ビルド方法

make を引数なしで実行するとビルドして、実行ファイル suminv を生成する。 make に引数 test をつけると、組み込みのテストを実行して、結果を表示する。

```bash
cd /path/to/suminv
make
```

```bash
make test
```

## 実行

suminv の 第一引数は和がいくつ以上になるまで足すか(M)である。以下は10まで足す。解けたら、答え(N)が 12367 であることを表示する。

```bash
./suminv 10
# Answer: 12367
```

suminv の 第二引数は並列度である。0にすると、除算と足し算をSIMD並列化しない。SIMD並列化するより遅い。

```bash
time ./suminv 22 0
# real 0m3.947s
```

第二引数は0から8までの整数を指定できる。整数はソフトウェアパイプライニングの深さで、8にすると除算と足し算をSIMD並列化し、8段パイプラインで実行する。実際には1段もそれ以上の段数も、実行時間はほとんど変わらない。

```bash
time ./suminv 22 8
# real 0m2.357s
```

make の引数に shortrun または longrun を設定すると、固定のMで実行する。実装を改良して、どれだけ速くなったかみるために使う。

```bash
make shortrun
make longrun
```

## SIMD演算

YMMレジスタ1本で、double 4個の除算をまとめてできる。加算は4回掛かる。

|命令|ビット255:192|ビット191:128|ビット127:64|ビット63:0|
|:------|:------|:------|:------|:------|
|vdivpd|1/i|1/(i+1)|1/(i+2)|1/(i+3)|
|vpermq|1/i|1/i|1/i|1/i|
|vaddpd|accum + 1/i|accum + 1/i|accum + 1/i|accum + 1/i|
|vpermq|1/(i+1)|1/(i+1)|1/(i+1)|1/(i+1)|
|vshufpd|0|1/(i+1)|0|1/(i+1)|
|vperm2f128|0|1/(i+1)|1/(i+1)|1/(i+1)|
|vaddpd|accum + 1/i|accum + 1/i + 1/(i+1)|accum + 1/i + 1/(i+1)|accum + 1/i + 1/(i+1)|
|vpermq|1/(i+2)|1/(i+2)|1/(i+2)|1/(i+2)|
|vperm2f128|0|0|1/(i+2)|1/(i+2)|
|vaddpd|accum + 1/i|accum + 1/i + 1/(i+1)|accum + 1/i + 1/(i+1) + 1/(i+2)|accum + 1/i + 1/(i+1) + 1/(i+2)|
|vpermq|1/(i+3)|1/(i+3)|1/(i+3)|1/(i+3)|
|vshufpd|0|1/(i+3)|0|1/(i+3)|
|vperm2f128|0|0|0|1/(i+3)|
|vaddpd|accum + 1/i|accum + 1/i + 1/(i+1)|accum + 1/i + 1/(i+1) + 1/(i+2)|accum + 1/i + 1/(i+1) + 1/(i+2) + 1/(i+3)|
