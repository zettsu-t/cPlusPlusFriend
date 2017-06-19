#!/usr/bin/ruby
# coding: utf-8

require 'prime'
require 'test/unit'

# 同じ数は二度判定しない
class PrimeSet
  def initialize(maxNumber)
    @map = {}
  end

  def prime?(n)
    @map[n] = Prime.prime?(n) unless @map.key?(n)
    @map[n]
  end
end

# これ以上大きな数はこなさそうな番号
PRIME_SET = PrimeSet.new(0x17000)
MARK_PRIME = "O".freeze           # 左から右に連結した16進数が素数
MARK_PRIME_REVERSED = "o".freeze  # 右から左に連結した16進数が素数
MARK_PRIME_ANY = ".".freeze       # 16進数で桁を並べ替えたもののいずれかが素数
MARK_OTHER = "_".freeze           # 上記以外
MARK_EMPTY = " ".freeze           # 通路
# 席数の表示順序
MARK_SET = [MARK_PRIME, MARK_PRIME_REVERSED, MARK_PRIME_ANY, MARK_OTHER].freeze

class Seat
  def initialize(carNumber, rowNumber, column)
    @mark = column.empty? ? MARK_EMPTY : mark([carNumber, rowNumber, column].map(&:to_s).join(""))
  end

  def mark(hexStr)
    isPrime = -> str { PRIME_SET.prime?(str.hex) }

    case hexStr
    when isPrime
      MARK_PRIME
    when -> str { isPrime.call(str.reverse) }
      MARK_PRIME_REVERSED
    when -> str { str.chars.permutation.any? { |a| isPrime.call(a.join("")) }}
      MARK_PRIME_ANY
    else
      MARK_OTHER
    end
  end

  def toString(accumMap)
    unless @mark.strip.empty?
      accumMap[@mark] ||= 0
      accumMap[@mark] += 1
    end
    @mark
  end
end

class SeatRow
  def initialize(carNumber, rowNumber, columns)
    @rowNumber = rowNumber
    @seats = columns.map { |column| Seat.new(carNumber, rowNumber, column) }
  end

  def getIndent
    "    "
  end

  def toString(accumMap)
    sprintf("%2d  ", @rowNumber) + @seats.map { |seat| seat.toString(accumMap) }.join + "\n"
  end
end

class Car
  def initialize(carNumber)
    @carNumber = carNumber
    # 号車 => 行の数
    upperClass = { 8=>17, 9=>16, 10=>17 }
    standardClass = { 1=>13, 2=>20, 3=>17, 4=>20, 5=>18, 6=>20, 7=>15,
                      12=>20, 13=>18, 14=>20, 15=>16, 16=>15 }
    others = { 11=>:makeCar11 }

    @rows = []
    case carNumber
    when *upperClass.keys
      columns = ["A", "B", "", "C", "D"]
      @rows = makeCar(carNumber, upperClass[carNumber], columns)
    when *standardClass.keys
      columns = ["A", "B", "C", "", "D", "E"]
      @rows = makeCar(carNumber, standardClass[carNumber], columns)
    when *others.keys
      @rows, columns = send(others[carNumber])
    end

    @columnStr = columns.map { |s| s.empty? ? " " : s }.join("")
  end

  def makeCar(carNumber, maxRowNumber, columns)
    (1..maxRowNumber).map { |rowNumber| SeatRow.new(carNumber, rowNumber, columns) }
  end

  def makeCar11
    carNumber = 11
    columns = ["A", "B", "C", "", "D", "E"]
    rows = (1..11).map { |rowNumber| SeatRow.new(carNumber, rowNumber, columns) }
    rows += (12..13).map { |rowNumber| SeatRow.new(carNumber, rowNumber, ["A", "B", "", "", "D", "E"]) }
    return rows, columns
  end

  def createMarkMap
    markMap = {}
    # 0を登録して順序をつける
    MARK_SET.each { |mark| markMap[mark] = 0 }
    markMap
  end

  def toString(accumMap)
    markMap = createMarkMap
    str = sprintf("=== Car No.%d ===\n", @carNumber)
    str += @rows[0].getIndent + @columnStr + "\n"
    str += @rows.map { |row| row.toString(markMap) }.join("")
    str += markMap.to_s + "\n\n"

    markMap.each do |key, value|
      accumMap[key] ||= 0
      accumMap[key] += value
    end
    str
  end
end

class SeatMap
  def initialize
    @cars = (1..16).map { |i| Car.new(i) }
  end

  def count
    accumMap = @cars[0].createMarkMap
    str = @cars.map { |car| car.toString(accumMap) }.join("")
    str += "Total " + accumMap.to_s + "\n"
    sizeOfSeats = accumMap.values.reduce(0, :+)
    return str, sizeOfSeats
  end
end

class TestSeatMap < Test::Unit::TestCase
  def test_all
    seatMap = SeatMap.new
    str, sizeOfSeats = seatMap.count
    puts str

    assert_equal(16, str.scan(/Car No.\d+/).size)
    assert_equal(1323, sizeOfSeats)
  end

  data(
    '1-4'   => ["1", "4",   ["A", "B", "C", "D", "E"], ["_Oo", "_."]],
    '9-10'  => ["9", "10",  ["A", "B", "C", "D"], ["__", "o."]],
    '16-11' => ["16", "11", ["A", "B", "C", "D", "E"], [".__", "O."]])
  def test_rows(data)
    carStr, rowStr, postfixSet, expected = data
    carNumber = carStr.to_i
    expectedRow = expected.join("")

    seatMap = SeatMap.new
    fullStr, sizeOfSeats = seatMap.count
    rowSet = fullStr.scan(/\s+#{rowStr}\s+(\S+)\s+(\S+)/)

    assert_equal(16, rowSet.size)
    assert_equal(expected, rowSet[carNumber - 1])
    puts

    postfixSet.each_with_index do |postfix, i|
      inputStr = carStr + rowStr + postfix
      if Prime.prime?(inputStr.hex)
        assert_equal(expectedRow[i], "O")
      elsif Prime.prime?(inputStr.reverse.hex)
        assert_equal(expectedRow[i], "o")
      end

      foundPrime = false
      inputStr.chars.permutation.each do |s|
        str = s.join("")
        prime = Prime.prime?(str.hex)
        foundPrime |= prime
        puts str + " is prime = " + prime.to_s
      end

      unless foundPrime
        assert_equal(expectedRow[i], "_")
      end
      puts
    end
  end
end
