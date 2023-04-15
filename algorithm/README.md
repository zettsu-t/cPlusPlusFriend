# スニペット置き場のテンプレート

## 概要

`snipetts/` 以下の .cpp ファイルをビルドして、GoogleTestとリンクしてテストします。競技プログラミングなどのスニペット置き場としてご活用ください。

## ビルド環境を構築して起動する

Dockerイメージを [鉄則本を解いてみた](https://github.com/zettsu-t/kyopro-tessoku-answers) から派生しました。ですのでDockerコンテナ名も変えていません。

```bash
docker-compose build
docker-compose up -d
docker-compose exec kyopro-tessoku-answers /bin/bash
docker-compose down
```

GoogleTestを [こちら](https://github.com/google/googletest) からダウンロードして、`snipetts/googletest/` に置いてから `make` します。 `make run` すると全実行ファイルを実行しますし、特定の実行ファイルを指定して実行してもよいです。

```bash
make
make run
./sample
```

## 縦書きのテキストを横書きにする

テキストファイルの縦横を入れ替えます。成績表(縦軸がコンテスト、横軸が問題)の整形にどうぞ。

```
python3 transpose.py < input.txt
```
