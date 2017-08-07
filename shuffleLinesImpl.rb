#!/usr/bin/ruby
# coding: utf-8

# 要素数がそれぞれm, n, ... 個の配列要素が、ほどよく入り混じるように並べ替える
# ほどよく = 要素数が最大でないものについては、連続して出ないことを保証する
class ArrayInterleaver
  def initialize
  end

  # 引数   : 各配列の要素数を並べた配列 [整数]
  # 返り値 : 何番目の配列を取り出すか、を順に並べた配列
  # 例えば [4,2] を入力すると、 [0,0,1,0,1,0] を返す。何を返すかはランダムである。
  def makeSequence(argIndexSizeArray)
    return [] if argIndexSizeArray.empty?

    # 要素数が多い物から順に並べ替えるときに、引数では何番目だったかを保存する
    indexArray = []
    argIndexSizeArray.each_with_index do |size, i|
      indexArray << Struct.new(:size, :index).new(size, i)
    end

    sortedIndexArray = indexArray.sort_by(&:size).reverse
    maxSize = sortedIndexArray[0].size

    sortedIndexArray.map do |s|
      makeIndexSequence(maxSize, s.size, s.index)
    end.shuffle.transpose.flatten.compact
  end

  # witdh個の配列の中に、elementSize個のelementを分布させる
  def makeIndexSequence(width, elementSize, element)
    return Array.new(width, element) if width == elementSize

    sequence = Array.new(width, nil)
    sequenceCount = width
    elementCount = elementSize
    0.upto(width - 1) do |i|
      if Random.rand(sequenceCount) < elementCount
        sequence[i] = element
        elementCount -= 1
      end
      sequenceCount -= 1
    end

    sequence
  end
end
