#!/usr/bin/ruby
# coding: utf-8

require 'test/unit'

class Animal
  attr_reader :count
  def initialize
    @count = 0
  end

  # 本来の===の使い方とは違うが...
  def ===(other)
    # 呼び出された回数を後で返す
    @count += 1
    # 派生クラスは、派生クラスであるかどうかを返す
    instance_of?(other.class)
  end

  def introduceSelf
    "けものだよ"
  end
end

class Cat < Animal
  def introduceSelf
    "ネコ科だよ"
  end
end

class Serval < Cat
  def introduceSelf
    "私はサーバルキャットのサーバル!"
  end
end

class WhiteFacedScopsOwl
  def initialize
    @base   = Animal.new
    @sub    = Cat.new
    @subsub = Serval.new
  end

  def matchUpto(obj)
    # objと一致する型を見つけて、結果を文字列で返す
    case obj
    when @base then
      "けものです"
    when @sub then
      "ネコ科です"
    when @subsub then
      "サーバルです"
    else
      "教えてやるので、おかわりをよこすのです"
    end
  end

  def matchDownto(obj)
    case obj
    when @subsub then
      "Leptailurus serval"
    when @sub then
      "Felidae"
    when @base then
      "Animalia"
    else
      "Give me a second helping, please"
    end
  end

  def getBaseCount
    @base.count
  end

  def getSubCount
    @sub.count
  end

  def getSubSubCount
    @subsub.count
  end
end

class TestCaseWhen < Test::Unit::TestCase
  data(
    'base'   => [:Animal, "けものだよ"],
    'sub'    => [:Cat,    "ネコ科だよ"],
    'subsub' => [:Serval, "私はサーバルキャットのサーバル!"])
  def test_create(data)
    classNameSym, expected = data
    assert_equal(expected, Object.const_get(classNameSym).new.introduceSelf)
  end

  data(
    'base'   => [:Animal, 1, 0, 0, "けものです"],
    'sub'    => [:Cat,    1, 1, 0, "ネコ科です"],
    'subsub' => [:Serval, 1, 1, 1, "サーバルです"],
    'other'  => [:String, 1, 1, 1, "教えてやるので、おかわりをよこすのです"])
  def test_matchUpto(data)
    classNameSym, baseCount, subCount, subSubCount, expected = data
    konoha = WhiteFacedScopsOwl.new
    assert_equal(expected, konoha.matchUpto(Object.const_get(classNameSym).new))
    assert_equal(baseCount, konoha.getBaseCount)
    assert_equal(subCount, konoha.getSubCount)
    assert_equal(subSubCount, konoha.getSubSubCount)
  end

  data(
    'base'   => [:Animal, 1, 1, 1, "Animalia"],
    'sub'    => [:Cat,    0, 1, 1, "Felidae"],
    'subsub' => [:Serval, 0, 0, 1, "Leptailurus serval"],
    'other'  => [:String, 1, 1, 1, "Give me a second helping, please"])
  def test_matchDownto(data)
    classNameSym, baseCount, subCount, subSubCount, expected = data
    konoha = WhiteFacedScopsOwl.new
    assert_equal(expected, konoha.matchDownto(Object.const_get(classNameSym).new))
    assert_equal(baseCount, konoha.getBaseCount)
    assert_equal(subCount, konoha.getSubCount)
    assert_equal(subSubCount, konoha.getSubSubCount)
  end
end
