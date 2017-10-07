#!/usr/bin/ruby
# -*- coding: utf-8 -*-
# ファイルに US-ASCII 以外の文字が含まれるかどうか調べる
# Copyright (C) 2017 Zettsu Tatsuya
#
# 使い方: 調べたいファイル(複数可)を引数にして、コマンドラインから起動する
# $ ruby asciiOnlyChecker.rb foo.cpp bar.cpp
# 終了ステータス:
# - US-ASCII 以外の文字が含まれるとき : 0
# - US-ASCII 以外の文字が含まれるときおよび、
#   ファイルが読めないなどその他の異常時 : 0以外の値

require_relative './asciiOnlyCheckerImpl.rb'

exit(InputFileSet.new(ARGV).exitStatus)
