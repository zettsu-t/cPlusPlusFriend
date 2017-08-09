#!/usr/bin/ruby
# coding: utf-8

require_relative './shuffleLinesImpl.rb'
require 'open3'
require 'tempfile'
require 'test/unit'

SIZE_OF_RANDOM_TESTING_SMALL = 500
SIZE_OF_RANDOM_TESTING_LARGE = 5000
SIZE_OF_RANDOM_TESTING_FILE  = 10
TEST_INPUT_FILENAME = "cppFriendsBot.txt"
TEST_COMMAND_NAME = "ruby -Ku shuffleLines.rb"

class MockInputStream
  def initialize(lines)
    @lines = lines.dup
  end

  def gets
    @lines.shift
  end
end

class MockOutputStream
  attr_reader :str

  def initialize
    @str = ""
  end

  def write(line)
    @str += line
  end
end

class TestMyProgramOption < Test::Unit::TestCase
  data(
    'short separated' => ["-i", "name"],
    'short connected' => ["-iname"],
    'long separated'  => ["--input", "name"],
    'long connected'  => ["--input=name"],
    'space' => ["-i name"])
  def test_inFilename(data)
    opt = MyProgramOption.new(data)
    assert_equal("name", opt.inFilename)
    assert_nil(opt.outFilename)
    assert_nil(opt.phrase)
  end

  data(
    'short separated' => ["-o", "name"],
    'short connected' => ["-oname"],
    'long separated'  => ["--output", "name"],
    'long connected'  => ["--output=name"],
    'space' => ["-o name"])
  def test_outFilename(data)
    opt = MyProgramOption.new(data)
    assert_nil(opt.inFilename)
    assert_equal("name", opt.outFilename)
    assert_nil(opt.phrase)
  end

  data(
    'short separated' => ["-p", "name"],
    'short connected' => ["-pname"],
    'long separated'  => ["--phrase", "name"],
    'long connected'  => ["--phrase=name"],
    'space' => ["-p name"])
  def test_phrase(data)
    opt = MyProgramOption.new(data)
    assert_nil(opt.inFilename)
    assert_nil(opt.outFilename)
    assert_equal("name", opt.phrase)
  end

  data(
    'short' => [["-p", "It rains"], "It rains"],
    'short space' => [["-p", " It rains "], "It rains"],
    'long'  => [["--phrase", "わ－い たーのしー"], "わ－い たーのしー"],
    'long space' => [["--phrase", " わ－い  たーのしー  "], "わ－い  たーのしー"])
  def test_sentence(data)
    arg, expected = data
    opt = MyProgramOption.new(arg)
    assert_nil(opt.inFilename)
    assert_nil(opt.outFilename)
    assert_equal(expected, opt.phrase)
  end

  def test_empty
    opt = MyProgramOption.new([])
    assert_nil(opt.inFilename)
    assert_nil(opt.outFilename)
    assert_nil(opt.phrase)
  end

  def test_mixed
    inFilename = "src"
    outFilename = "dst"
    inPhrase = " わ－い  たーのしー  "
    outPhrase = "わ－い  たーのしー"

    [["-i", inFilename,  "--output=#{outFilename}", "--phrase", inPhrase],
     ["-i#{inFilename}", "--output", outFilename,  "-p", inPhrase]].each do |arg|
      opt = MyProgramOption.new(arg)
      assert_equal(inFilename, opt.inFilename)
      assert_equal(outFilename, opt.outFilename)
      assert_equal(outPhrase, opt.phrase)
    end
  end

  def test_missingParameter
    inFilename = "src"
    outFilename = "dst"
    inPhrase = " わ－い  たーのしー  "
    outPhrase = "わ－い  たーのしー"

    [[["-i", inFilename, "--output=#{outFilename}", "--phrase"], inFilename, outFilename, nil],
     [["-i", inFilename, "--output=#{outFilename}", "-p"], inFilename, outFilename, nil],
     [["-i", inFilename, "--output", "--phrase"], inFilename, nil, nil],
     [["-i", inFilename, "-o", "--phrase"], inFilename, nil, nil],
     [["--input", "--output", "--phrase"], nil, nil, nil],
     [["-i", "--output", "-p"], nil, nil, nil],
     [["-i", inFilename, "--output=#{outFilename}", "--phrase="], inFilename, outFilename, ""],
     [["-i", inFilename, "--output=", "--phrase="], inFilename, "", ""],
     [["--input=", "--output=", "--phrase="], "", "", ""]].each do
      |arg, expectedInFilename, expectedOutFilename, expectedPhrase|
      opt = MyProgramOption.new(arg)
      assert_equal(expectedInFilename, opt.inFilename)
      assert_equal(expectedOutFilename, opt.outFilename)
      assert_equal(expectedPhrase, opt.phrase)
    end
  end

  def test_patterns
    assert_equal(LINE_PATTERN_SET, MyProgramOption.new([]).patterns)
  end
end

class TestArrayInterleaver < Test::Unit::TestCase
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

    SIZE_OF_RANDOM_TESTING_LARGE.times do
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
    SIZE_OF_RANDOM_TESTING_SMALL.times do
      actual = ArrayInterleaver.new.makeSequence(arg)
      assert_not_equal(previous, actual)
      previous = actual
    end
  end
end

class TestLineSet < Test::Unit::TestCase
  def test_initialize
    1.upto(3) do |i|
      arg = (1..i).to_a
      lineSet = LineSet.new(arg)
      assert_equal(arg.size + 1, lineSet.instance_variable_get(:@lineArrays).size)
      assert_true(lineSet.instance_variable_get(:@lineArrays).all?(&:empty?))
      assert_equal(arg, lineSet.instance_variable_get(:@patterns))
      assert_true(lineSet.instance_variable_get(:@string).empty?)
    end
  end

  data(
    'first 1'  => ["abc 123", 0],
    'first 2'  => ["123 abc", 0],
    'second 1' => ["def 123", 1],
    'second 2' => ["123 def", 1],
    'other 1' => ["ghi", 2],
    'other 2' => ["abb", 2],
    'other 3' => ["df",  2])
  def test_addLine(data)
    line, expected = data
    patterns = [/abc/, /d.f/]

    lineSet = LineSet.new(patterns)
    lineSet.addLine(line)
    lineSet.instance_variable_get(:@lineArrays).each_with_index do |array, i|
      assert_equal(i != expected, array.empty?)
    end

    lineSet.addLine(line)
    assert_equal(2, lineSet.instance_variable_get(:@lineArrays)[expected].size)
  end

  def test_shuffleShort
    patterns = [/\A\d/, /\A\d/]

    actual = (1..SIZE_OF_RANDOM_TESTING_SMALL).map do
      lineSet = LineSet.new(patterns)
      ["1", "2", "a"].each { |line| lineSet.addLine(line) }
      lineSet.shuffle
      lineSet.to_s
    end.sort.uniq

    assert_equal(["12a", "1a2", "21a", "2a1", "a12", "a21"], actual)
  end

  def test_shuffleLong
    patterns = [/\A\d/, /\A[a-z]/, /\A[A-Z]/]

    (1..SIZE_OF_RANDOM_TESTING_SMALL).map do
      lineSet = LineSet.new(patterns)
      ["1", "2", "a", "b", "A", "B", "C", "D"].each { |line| lineSet.addLine(line) }
      lineSet.shuffle
      str = lineSet.to_s
      assert_not_match(/\d{2}/, str)
      assert_not_match(/[a-z]{2}/, str)
    end.sort.uniq
  end
end

class TestLineSetParser < Test::Unit::TestCase
  def test_parseInputStreamWithoutPhrase
    parser = LineSetParser.new(nil)
    inputLines = ["1", "a", "A", "2"]
    inStream = MockInputStream.new(inputLines)
    patterns = [/\A\d/, /\A[a-x]/, /\A[A-Z]/]
    actual, lines = parser.parseInputStream(patterns, inStream)

    assert_equal(1, actual.size)
    assert_equal(inputLines.sort, lines)
    assert_equal([["1", "2"], ["a"], ["A"], []], actual[0].instance_variable_get(:@lineArrays))
  end

  def test_parseInputStreamWithPhrase
    delimiter = "---END---"
    parser = LineSetParser.new(nil)
    parser.instance_variable_set(:@phrase, "END")

    inputLines = ["1", "a", delimiter, "b", "2"]
    inStream = MockInputStream.new(inputLines)
    patterns = [/\A\d/, /\A[a-x]/]
    actual, lines = parser.parseInputStream(patterns, inStream)

    assert_equal(2, actual.size)
    assert_equal(inputLines.sort, lines)
    assert_equal([["1"], ["a"], [delimiter]], actual[0].instance_variable_get(:@lineArrays))
    assert_equal([["2"], ["b"], []], actual[1].instance_variable_get(:@lineArrays))
  end

  def setUpLineSetQueue
    lineSetA = LineSet.new([])
    lineSetB = LineSet.new([])
    lineSetA.addLine("a")
    lineSetA.addLine("B")
    lineSetB.addLine("c")
    lineSetB.addLine("D")
    [lineSetA, lineSetB]
  end

  def test_shuffle
    actual = (1..SIZE_OF_RANDOM_TESTING_SMALL).map do
      delimiter = "---END---"
      parser = LineSetParser.new(nil)
      lineSetQueue = setUpLineSetQueue
      parser.instance_variable_set(:@lineSetQueue, lineSetQueue)
      parser.shuffle
      lineSetQueue.map(&:to_s).join("")
    end.sort.uniq

    assert_equal(4, actual.size)
  end

  def test_writeToStream
    parser = LineSetParser.new(nil)
    lineSetQueue = setUpLineSetQueue
    parser.instance_variable_set(:@lineSetQueue, lineSetQueue)
    outStream = MockOutputStream.new
    parser.shuffle
    str = parser.writeToStream(outStream)

    assert_equal("BDac", str.split("").sort.join(""))
    assert_equal("BDac", outStream.str.split("").sort.join(""))
  end
end

class TestFileIO < Test::Unit::TestCase
  def parse(options, original, outFilename)
    parser = LineSetParser.new(options)
    expectedLines = original.sort.map { |s| s + "\n" }
    assert_equal(expectedLines, parser.instance_variable_get(:@linesRead))
    parser.shuffle
    parser.write

    lines = []
    File.open(outFilename, "r") { |inStream|
      while line = inStream.gets
        lines << line
      end
    }
    lines
  end

  def test_noParse
    SIZE_OF_RANDOM_TESTING_FILE.times do
      inTmpfile = Tempfile.new("input")
      inFilename = inTmpfile.path
      original = ["a", "b", "c", "d", "1", "2", "3"]
      original.each { |s| inTmpfile.puts s }
      inTmpfile.close

      outTmpfile = Tempfile.new("output")
      outFilename = outTmpfile.path
      outTmpfile.close

      options = MyProgramOption.new(["-i", inFilename, "-o", outFilename])
      options.instance_variable_set(:@patterns, [/\d+/])
      lines = parse(options, original, outFilename)
      assert_equal(original.size, lines.size)
      assert_equal(original.sort, lines.sort.map(&:chomp))
    end
  end

  def test_withParse
    SIZE_OF_RANDOM_TESTING_FILE.times do
      inTmpfile = Tempfile.new("input")
      inFilename = inTmpfile.path
      phrase = "x"
      original = ["a", "b", "c", phrase, "1", "2", "3"]
      original.each { |s| inTmpfile.puts s }
      inTmpfile.close

      outTmpfile = Tempfile.new("output")
      outFilename = outTmpfile.path
      outTmpfile.close

      options = MyProgramOption.new(["-i", inFilename, "-o", outFilename])
      options.instance_variable_set(:@patterns, [/\d+/])
      lines = parse(options, original, outFilename)
      assert_equal(original.size, lines.size)
      assert_equal(original[-3..-1], lines.sort.map(&:chomp)[0..2])
      assert_equal(original[0..3], lines.sort.map(&:chomp)[3..-1])
    end
  end

  data(
    'empty set'   => [[""],   "x", true],
    'not matched' => [["a"],  "x", true],
    'empty set string' => [[], "", true],
    'empty lines' => [["a"], "", false])
  def test_findParse(data)
    lines, phrase, error = data
    inTmpfile = Tempfile.new("input")
    inFilename = inTmpfile.path
    lines.each { |s| inTmpfile.puts s }
    inTmpfile.close

    outTmpfile = Tempfile.new("output")
    outFilename = outTmpfile.path
    outTmpfile.close

    options = MyProgramOption.new(["-i", inFilename, "-o", outFilename, "-p", phrase])
    options.instance_variable_set(:@patterns, [/\d+/])

    if error
      assert_raise(AppInvalidParameter) { LineSetParser.new(options) }
    else
      LineSetParser.new(options)
    end
  end

  def test_identicalFiles
    SIZE_OF_RANDOM_TESTING_FILE.times do
      tmpfile = Tempfile.new("input")
      filename = tmpfile.path
      tmpfile.close

      options = MyProgramOption.new(["-i", filename, "-o", filename])
      assert_raise(AppInvalidParameter) { LineSetParser.new(options) }
    end
  end

  def test_noInputFile
    SIZE_OF_RANDOM_TESTING_FILE.times do
      filename = "__f_i_l_e_not_exist__"
      assert_false(File.exist?(filename))
      options = MyProgramOption.new(["-i", filename])
      assert_raise(AppInvalidParameter) { LineSetParser.new(options) }
    end
  end
end

class TestActualFile < Test::Unit::TestCase
  def setup
    @inputLines = readAndSort(TEST_INPUT_FILENAME)
    assert_false(@inputLines.empty?)
    @convertedLines = @inputLines
  end

  def checkIfIncludeCR(str, includeCR)
    assert_equal(includeCR, str.include?("\r")) unless includeCR.nil?
  end

  def convertFile(outFilename, includeCR, encondingArgs)
    @convertedLines = []
    File.open(outFilename, "w") { |outStream|
      @inputLines.each do |line|
        convertedLine = line.encode(*encondingArgs)
        checkIfIncludeCR(convertedLine, includeCR)
        outStream.write(convertedLine)
        @convertedLines << convertedLine
      end
    }
  end

  def readAndSort(inFilename)
    lines = []
    File.open(inFilename, "r") { |inStream|
      while line = inStream.gets
        lines << line
      end
    }
    lines.sort
  end

  data(
    'file to file'    => "#{TEST_COMMAND_NAME} -i #{TEST_INPUT_FILENAME} -o ",
    'file to stdout'  => "#{TEST_COMMAND_NAME} -i #{TEST_INPUT_FILENAME} >  ",
    'stdin to file'   => "#{TEST_COMMAND_NAME} <  #{TEST_INPUT_FILENAME} -o ",
    'stdin to stdout' => "#{TEST_COMMAND_NAME} <  #{TEST_INPUT_FILENAME} >  ")
  def test_io(data)
    command = data
    outTmpfile = Tempfile.new("output")
    outFilename = outTmpfile.path
    outTmpfile.close
    stdoutstr, stderrstr, status = Open3.capture3(command + outFilename)

    assert_equal("", stdoutstr.chomp)
    assert_equal("", stderrstr.chomp)
    assert_true(status.success?)
    assert_equal(@inputLines, readAndSort(outFilename))
  end

  data(
    'UTF-8 CRLF' => [true,  ["UTF-8", :crlf_newline => true]],
    'UTF-8 LF'   => [false, ["UTF-8", :universal_newline => true]])
  def test_keepLineFeed(data)
    includeCR, encodingArgs = data
    inTmpfile = Tempfile.new("input")
    inFilename = inTmpfile.path
    inTmpfile.close

    outTmpfile = Tempfile.new("output")
    outFilename = outTmpfile.path
    outTmpfile.close

    convertFile(inFilename, includeCR, encodingArgs)
    command = "#{TEST_COMMAND_NAME} -i #{inFilename} -o #{outFilename}"
    stdoutstr, stderrstr, status = Open3.capture3(command)

    assert_equal("", stdoutstr.chomp)
    assert_equal("", stderrstr.chomp)
    assert_true(status.success?)

    lines = readAndSort(outFilename)
    assert_equal(@convertedLines.sort, lines)
    checkIfIncludeCR(lines.join(""), includeCR)
  end

  def test_shiftJIS
    inTmpfile = Tempfile.new("input")
    inFilename = inTmpfile.path
    inTmpfile.close

    outTmpfile = Tempfile.new("output")
    outFilename = outTmpfile.path
    outTmpfile.close

    encodingArgs = ["Shift_JIS", :crlf_newline => true, :undef => :replace, :replace => ""],
    convertFile(inFilename, nil, encodingArgs)
    command = "#{TEST_COMMAND_NAME} -i #{inFilename} -o #{outFilename}"
    stdoutstr, stderrstr, status = Open3.capture3(command)
    assert_true(status.success?)
  end

  data(
    'exist 1' => ["Software Developer Manuals", true],
    'exist 2' => ["われわれはかしこいので", true],
    'not exist' => ["我々は賢いので", false])
  def test_finePhrase(data)
    phrase, success = data
    outTmpfile = Tempfile.new("output")
    outFilename = outTmpfile.path
    outTmpfile.close
    command = "#{TEST_COMMAND_NAME} -i #{TEST_INPUT_FILENAME} -o #{outFilename} -p #{phrase}"
    stdoutstr, stderrstr, status = Open3.capture3(command)
    assert_equal("", stdoutstr.chomp)
    assert_equal(success, status.success?)

    if success
      assert_equal("", stderrstr.chomp)
      assert_equal(@inputLines, readAndSort(outFilename))
    else
      assert_true(stderrstr.include?("Cannot find #{phrase} in the input."))
      assert_equal([], readAndSort(outFilename))
    end
  end
end
