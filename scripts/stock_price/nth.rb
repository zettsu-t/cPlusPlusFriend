#!/usr/bin/ruby
require 'test/unit'

class TestNth < Test::Unit::TestCase
  def test_all
    0.upto(1000) do |i|
      expected = "#{i}th"
      if ((i % 100) / 10) != 1
        case i % 10
        when 1
          expected = "#{i}st"
        when 2
          expected = "#{i}nd"
        when 3
          expected = "#{i}rd"
        end
      end

      actual = i.to_s+['st','nd','rd','th'][[3,([89,(i+80)%100].min+9)%10].min]
      puts i, actual
      assert_equal(expected, actual)
    end
  end
end
