#!/usr/bin/ruby
# coding: utf-8

require 'test/unit'

def parseHourString(timestamp)
# timestamp.sub(/(\D*)(\d+)/){ t=$2.to_i; ( t + ((!$1.include?("午後") || (t > 12)) ? 0 : 12)).to_s }.to_i
  timestamp.sub(/(\D*)(\d+)/){t=$2.to_i;(t+((!$1.include?("午後")||(t>12))?0:12)).to_s}.to_i
end

class TestSeatMap < Test::Unit::TestCase
  def createStringSet(n, prefix)
    numberSet = [n.to_s, sprintf("%02d", n)]
    spaceSet = ["", " "]
    postfixSet = ["", "時"]
    numberSet.product(spaceSet).product(postfixSet).map { |a| prefix + a.join("") }
  end

  data(
    'zero'       => [0,  "", ["0", "0時", "0 ", "0 時", "00", "00時", "00 ", "00 時"]],
    'one digit'  => [1,  "", ["1", "1時", "1 ", "1 時", "01", "01時", "01 ", "01 時"]],
    'two digits' => [10, "", ["10", "10時", "10 ", "10 時", "10", "10時", "10 ", "10 時"]],
    'prefix'     => [12, "午前", ["午前12", "午前12時", "午前12 ", "午前12 時",
                                  "午前12", "午前12時", "午前12 ", "午前12 時"]])
  def test_input(data)
    n, prefix, expected = data
    assert_equal(expected, createStringSet(n, prefix))
  end

  # 24時制
  def test_hour24
    0.upto(24) do |n|
      createStringSet(n, "").each do |str|
        actual = parseHourString(str)
        assert_equal(n, actual)
      end
    end
  end

  # 午前
  # 正午は午前12時ちょうど = 午後0時ちょうど
  # https://www.nao.ac.jp/faq/a0401.html
  def test_am
    0.upto(12) do |n|
      createStringSet(n, "午前").each do |str|
        actual = parseHourString(str)
        assert_equal(n, actual)
      end
    end
  end

  # 午後
  def test_pm
    0.upto(24) do |n|
      createStringSet(n, "午後").each do |str|
        expected = (n <= 12) ? (n + 12) : n;
        actual = parseHourString(str)
        assert_equal(expected, actual)
      end
    end
  end
end
