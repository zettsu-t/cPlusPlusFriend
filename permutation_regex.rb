#!/usr/bin/ruby
# coding: utf-8

require 'test/unit'

## https://stackoverflow.com/questions/3101366/regex-to-match-all-permutations-of-1-2-3-4-without-repetition
class TestPermutationRegex < Test::Unit::TestCase
  def test_all()
    pattern = /(?:([大石泉すき])(?!.*\1)){5}/
    str_chars = "大石泉すき".chars
    prefix = "abc"
    postfix = "xyz"

    str_chars.permutation do |s|
      word = prefix + s.join + postfix
      assert_match(pattern, word)
    end

    Array.new(str_chars.size - 1, str_chars).inject(str_chars) do |acc, v|
      acc = acc.product(v).map(&:flatten)
    end.each do |s|
      word = prefix + s.join + postfix
      if s.sort.uniq.size == str_chars.size
        assert_equal("きす大泉石", s.sort.join)
        assert_match(pattern, word)
      else
        assert_not_match(pattern, word)
      end
    end
  end
end
