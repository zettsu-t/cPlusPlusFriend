#!/usr/bin/ruby
# coding: utf-8
# 円周率の各桁が乱数とみなせるなら、7zで圧縮したときに、
# 乱数と同程度のサイズになるのではないか?

class RandomDigits
    attr_reader :n, :a, :s

    def initialize(argv)
      # 第一引数(もしあれば) : 小数点以下の桁数
      @n = 1000000
      @n = argv[0].strip.to_i if argv.size > 0

      # 0..9の一様乱数を作るなら、引数は10未満にする
      @n_digits = 10

      # 整数.をつける
      @s = rand(@n_digits).to_s
      @s += "."

      # 小数点以下n桁を作る
      @a = (1..@n).map{ |x| rand(@n_digits) }
      @s += @a.map(&:to_s).join('')
    end
end

# 第一引数(もしあれば) : testならテストする
testing = ((ARGV.size) > 0 && ARGV[0] == "test")

if testing
  require 'test/unit'

  class TestAll < Test::Unit::TestCase
    def test_length
      [123, 456].each do |n|
        argv = [n.to_s]
        tested = RandomDigits.new(argv)
        assert_equal(n, tested.a.size)
        assert_equal(n + 2, tested.s.size)
        assert_match(/\A\d\.\d+\z/, tested.s)
      end
    end

    def test_count_digits
      n = 10000
      tested = RandomDigits.new([n.to_s])
      -2.upto(11) do |i|
        actual = tested.a.count(i)
        if i >= 0 && i < 10
          assert_true(actual > 0)
          assert_true(actual < n)
        else
          assert_equal(0, actual)
        end
      end
    end

    def test_distribution
      n = 1000000
      p = 0.1
      ## 二項分布 : 期待値=np, 分散=np(1-p)
      mu = n * p
      sigma = Math.sqrt(n * p * (1.0 - p))

      # 管理限界
      width = 6.0
      lower_limit = mu - width * sigma
      upper_limit = mu + width * sigma

      tested = RandomDigits.new([n.to_s])
      0.upto(9) do |i|
        actual = tested.a.count(i)
        assert_true(actual > lower_limit)
        assert_true(actual < upper_limit)
      end
    end

  end
else
  puts RandomDigits.new(ARGV).s
end
