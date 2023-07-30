# AtCoder Beginner Contest 成績集計

AtCoder Problems Datasets の Estimated Difficulties of the Problems を利用して、問題の難易度別の成績表を作ります。

## 用意するファイル

### 問題の難易度

[AtCoderProblemsのGitHub レポジトリ](https://github.com/kenkoooo/AtCoderProblems/blob/master/doc/api.md) からリンクされている [データセット](https://kenkoooo.com/atcoder/resources/problem-models.json) をダウンロードして、 `incoming_data` ディレクトリに置いてください。何度もダウンロードしないように、解析はローカルPCのファイルを読みます。

### 成績表

以下のようなプレーンテキストです。一行一コンテストを示します。ABC 277-A, 277-B, 312-E は解いていないので空欄です。

```
ABC277:  ++-
ABC312:++++ +
```

書式は、コンテスト、区切り文字 `:` 、A..H問題の成績です。 `+` は解けた、半角空白 ` ` は解いていない、それの文字以外は解けなかったことを示します。区切り文字 `:` の次がA問題で、間に区切り文字や空白は入れません。

## 成績表を作る

### 図を描く

Rで成績表を作ります。 `images/` ディレクトリに散布図とヒストグラムを出力します。ともに横軸は難易度、縦軸は問題を解けたかどうかです。入出力するファイル名は固定ですので、適宜書き換えてください。

```bash
Rscript difficulty.R
```

併せて、次に実行するPythonスクリプト用に、問題の難易度を `incoming_data/difficulty_auto_abc.csv` に出力します。

RのREPLから `difficulty.R` を実行すると、 `execute_all()` の返り値に結果がいろいろ入っています。

### テキストでみる

一行に一コンテスト分の結果を示します。A..H問題について、難易度と解けたかどうかをプレーンテキストで出力します。

```bash
python3 difficulty.py
```

このような出力になります。解いていないか難易度が分からない問題は空欄です。

```
ABC277:       2+ 3+ 3-
ABC312: 1+ 1+ 2+ 3+    4+
```

## 制限事項

問題の難易度はratingを400刻みにしたもので1以上9以下に限定しています。より具体的には $1 + \lfloor max(0, difficulty) / 400 \rfloor$ で、難易度0以下は難易度0(灰)とみなし、難易度10以上(rating 3600)以上は難易度不明とみなします。これは難易度を一桁で表示するために割り切ったのですが、ABCを解いている人が難易度3600以上の問題を解けるはずが無いので実用上問題ないでしょう。
