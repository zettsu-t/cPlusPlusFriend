#!/usr/bin/ruby
# coding: utf-8

require_relative './shuffleLinesImpl.rb'
require 'tempfile'
require 'test/unit'

SIZE_OF_RANDOM_TESTING_SMALL = 1000
SIZE_OF_RANDOM_TESTING_LARGE = 10000

class TestSeatMap < Test::Unit::TestCase
  # ex以外の要素が隣接するときtrue
  def checkIfSuccessive(sequence, ex)
    return false if sequence.size < 2

    e = sequence[0]
    1.upto(sequence.size - 1) do |i|
      nextElement = sequence[i]
      return true if (e == nextElement) && (e != ex)
      e = nextElement
    end

    return false
  end

  def test_empty
    0.upto(2) do |size|
      arg = Array.new(size, 0)
      assert_true(ArrayInterleaver.new.makeSequence(arg).empty?)
    end
  end

  def test_oneMember
    1.upto(3) do |size|
      actual = ArrayInterleaver.new.makeSequence([size])
      assert_equal(size, actual.size)
    end
  end

  def test_uniformMember
    1.upto(3) do |arraySize|
      1.upto(7) do |elementSize|
        arg = Array.new(arraySize, elementSize)
        actual = ArrayInterleaver.new.makeSequence(arg)
        assert_equal(arraySize * elementSize, actual.size)
        assert_false(checkIfSuccessive(actual, nil)) if arraySize > 1

        0.upto(arraySize - 1) do |i|
          assert_equal(elementSize, actual.count(i))
        end
      end
    end
  end

  data(
    'containing zero'  => [2,0,1],
    'ascending'   => [1,2,3],
    'descending'  => [3,2,1],
    'not ordered' => [2,3,1],
    'duplicate 1' => [1,3,3],
    'duplicate 2' => [1,1,3],
    'long 1'      => [1,2,4,8],
    'long 2'      => [4,3,2,1])
  def test_patterns(data)
    arg = data
    totalElements = arg.reduce(0, &:+)
    maxElement = arg.max

    indexArray = []
    arg.each_with_index do |size, i|
      indexArray << Struct.new(:size, :index).new(size, i)
    end

    1.upto(SIZE_OF_RANDOM_TESTING_LARGE) do
      actual = ArrayInterleaver.new.makeSequence(arg)
      assert_equal(totalElements, actual.size)

      # 最大要素数になる配列が複数あるときは、どの配列の要素も隣合わない
      index = (arg.count(maxElement) > 1) ? nil : indexArray.sort_by(&:size)[-1].index
      assert_false(checkIfSuccessive(actual, index))

      arg.each_with_index do |size, i|
        assert_equal(size, actual.count(i))
      end
    end
  end

  def test_isRandom
    arg = [2,3,4,5,6,7,8,9,10,11,12,13,14,15]

    # 偶然前回の結果と一致する確率がないとは言えない
    previous = []
    1.upto(SIZE_OF_RANDOM_TESTING_SMALL) do
      actual = ArrayInterleaver.new.makeSequence(arg)
      assert_not_equal(previous, actual)
      previous = actual
    end
  end
end
