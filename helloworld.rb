#!/usr/bin/ruby
# coding: utf-8

require 'test/unit'

PATTERN = /\A((H)|(h))ello,? (?(2)[Ww]|[w])orld!?\z/

class TestSeatMap < Test::Unit::TestCase
  def test_valid_strings
    ['hello world',
     'Hello world',
     'Hello World',
     'hello, world',
     'Hello, world',
     'Hello, World',
     'hello, world!',
     'Hello, world!',
     'Hello, World!',
     'hello world!',
     'Hello world!',
     'Hello World!'].each do |s|
      assert_not_nil(PATTERN.match(s))
    end
  end

  def test_invalid_strings
    ['hello World',
     'hello, World',
     'hello, World!',
     'hello World!',
     ' Hello World',
     'Hello World ',
     'Hello,? world',
     'Hello World!?',
     'Hello World!!',
     'Hello  World!',
     'Hello,World',
     'ello world',
     'Hello orld',
     'Iello world',
     'Hello vorld'
    ].each do |s|
      assert_nil(PATTERN.match(s))
    end
  end
end
