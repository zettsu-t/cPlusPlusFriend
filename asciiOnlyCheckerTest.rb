#!/usr/bin/ruby
# -*- coding: utf-8 -*-

require_relative './asciiOnlyCheckerImpl.rb'
require 'open3'
require 'tempfile'
require 'tmpdir'
require 'test/unit'

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

  def puts(line)
    @str += line
    @str += "\n" if line == line.chomp
  end
end

class TestParagraph < Test::Unit::TestCase
  def test_singleLine
    paragraph = Paragraph.new("foo.cpp", 456, "comment")
    expected = "foo.cpp : 456\ncomment"
    assert_equal(expected, paragraph.instance_variable_get(:@str))

    os = MockOutputStream.new
    assert_false(paragraph.print(os))
    assert_equal(expected + "\n\n", os.str)
  end

  def test_multipleLines
    paragraph = Paragraph.new("foo.cpp", 456, "first")
    paragraph.add("second")
    expected = "foo.cpp : 456\nfirst second"
    assert_equal(expected, paragraph.instance_variable_get(:@str))

    os = MockOutputStream.new
    assert_false(paragraph.print(os))
    assert_equal(expected + "\n\n", os.str)
  end

  data(
    'head'   => [["一行目", "second", "third"],  "一行目 second third"],
    'middle' => [["first",  "二行目", "third"],  "first 二行目 third"],
    'last'   => [["first",  "second", "三行目"], "first second 三行目"])
  def test_nonAscii(data)
    lines, expected = data
    paragraph = Paragraph.new("foo.cpp", 456, lines.shift)
    lines.each { |line| paragraph.add(line) }
    header = "Non-ASCII characters found in foo.cpp : 456\n"

    os = MockOutputStream.new
    assert_true(paragraph.print(os))
    assert_equal(header + expected + "\n\n", os.str)
  end
end

class TestPlainExtractor < Test::Unit::TestCase
  data(
    'no spaces' => ["line",  ["line"]],
    'left' =>      [" line", ["line"]],
    'right' =>     ["line ", ["line"]],
    'both' =>      [" line ", ["line"]],
    'zenkaku 1' => [ " 　 ",  ["　"]],
    'zenkaku 2' => [ " 全角 ", ["全角"]])
  def test_plainExtractor(data)
    line, expected = data
    assert_equal(expected, PlainExtractor.new.extract(line))
  end
end

class TestCppCommentExtractor < Test::Unit::TestCase
  data(
    'cpp comment 1' => ["//comment",  ["comment"]],
    'cpp comment 2' => ["// comment", ["comment"]],
    'cpp comment 3' => ["//コメント",  ["コメント"]],
    'cpp comment 4' => ["// コメント", ["コメント"]],
    'cpp comment 5' => ["// comment // コメント", ["comment // コメント"]],
    'c directive' => ["#define", [""]],
    'code' => ["int a = 0;", [""]],
    'code and comment 1' => ["int a = 0; // comment", ["", "comment"]],
    'code and comment 2' => ["int a = 0; //コメント //comment", ["", "コメント //comment"]],
  )
  def test_cppComment(data)
    line, expected = data
    assert_equal(expected, CppCommentExtractor.new.extract(line))
  end
end

class TestScriptCommentExtractor < Test::Unit::TestCase
  data(
    'comment 1' => ["#comment",  ["comment"]],
    'comment 2' => ["# comment", ["comment"]],
    'comment 3' => ["#コメント",  ["コメント"]],
    'comment 4' => ["# コメント", ["コメント"]],
    'comment 5' => ["# comment # コメント", ["comment # コメント"]],
    'cpp comment' => ["//comment",  [""]],
    'code' => ["a = 0;", [""]],
    'code and comment 1' => ["a = 0; # comment", ["", "comment"]],
    'code and comment 2' => ["a = 0; #コメント #comment", ["", "コメント #comment"]]
  )
  def test_scriptComment(data)
    line, expected = data
    assert_equal(expected, ScriptCommentExtractor.new.extract(line))
  end
end

class TestInputFile < Test::Unit::TestCase
  data(
    'none'   => [["first",  "second", "third"],  false, "foo.txt : 1\nfirst second third\n\n"],
    'head'   => [["一行目", "second", "third"],  true,  "Non-ASCII characters found in foo.txt : 1\n一行目 second third\n\n"],
    'middle' => [["first",  "二行目", "third"],  true,  "Non-ASCII characters found in foo.txt : 1\nfirst 二行目 third\n\n"],
    'last'   => [["first",  "second", "三行目"], true,  "Non-ASCII characters found in foo.txt : 1\nfirst second 三行目\n\n"])
  def test_plain(data)
    lines, expectedResult, expectedStr = data
    os = MockOutputStream.new
    file = InputFile.new("", nil, os)
    is = MockInputStream.new(lines)
    assert_equal(expectedResult, file.parse("foo.txt", PlainExtractor.new, os, is))
    assert_equal(expectedStr, os.str)
  end

  data(
    'comment' =>     [["// 一行目", "// second", "# third", "4"], true, "Non-ASCII characters found in foo.txt : 1\n一行目 second\n\n"],
    'non comment' => [["// first",  "二行目", "//third"], false, "foo.txt : 1\nfirst\n\nfoo.txt : 3\nthird\n\n"])
  def test_cpp(data)
    lines, expectedResult, expectedStr = data
    os = MockOutputStream.new
    file = InputFile.new("", nil, os)
    is = MockInputStream.new(lines)
    assert_equal(expectedResult, file.parse("foo.txt", CppCommentExtractor.new, os, is))
    assert_equal(expectedStr, os.str)
  end


  data(
    'members 1' => [["int a; // 一行目", "int b; // second", "int c; // third"], true,
                    "Non-ASCII characters found in foo.txt : 1\n一行目\n\nfoo.txt : 2\nsecond\n\nfoo.txt : 3\nthird\n\n"],
    'members 2' => [["int a; // first", "int b;", "int c; // 三行目"], true,
                    "foo.txt : 1\nfirst\n\nNon-ASCII characters found in foo.txt : 3\n三行目\n\n"]
  )
  def test_cppCode(data)
    lines, expectedResult, expectedStr = data
    os = MockOutputStream.new
    file = InputFile.new("", nil, os)
    is = MockInputStream.new(lines)
    assert_equal(expectedResult, file.parse("foo.txt", CppCommentExtractor.new, os, is))
    assert_equal(expectedStr, os.str)
  end

  data(
    'comment' =>     [["# 一行目", "# second", "// third", "4"], true, "Non-ASCII characters found in foo.txt : 1\n一行目 second\n\n"],
    'non comment' => [["# first",  "二行目", "#third"], false, "foo.txt : 1\nfirst\n\nfoo.txt : 3\nthird\n\n"])
  def test_script(data)
    lines, expectedResult, expectedStr = data
    os = MockOutputStream.new
    file = InputFile.new("", nil, os)
    is = MockInputStream.new(lines)
    assert_equal(expectedResult, file.parse("foo.txt", ScriptCommentExtractor.new, os, is))
    assert_equal(expectedStr, os.str)
  end

  data(
    'members 1' =>   [["a # 一行目", "b # second", "c # third"], true,
                      "Non-ASCII characters found in foo.txt : 1\n一行目\n\nfoo.txt : 2\nsecond\n\nfoo.txt : 3\nthird\n\n"],
    'members 2' =>   [["a #first", "b #二行目", "c"], true,
                      "foo.txt : 1\nfirst\n\nNon-ASCII characters found in foo.txt : 2\n二行目\n\n"]
  )
  def test_scriptCode(data)
    lines, expectedResult, expectedStr = data
    os = MockOutputStream.new
    file = InputFile.new("", nil, os)
    is = MockInputStream.new(lines)
    assert_equal(expectedResult, file.parse("foo.txt", ScriptCommentExtractor.new, os, is))
    assert_equal(expectedStr, os.str)
  end

  data(
    'ascii'     => ["first", false, "foo.cpp : 457\nfirst\n\n"],
    'non ascii' => ["一行目", true, "Non-ASCII characters found in foo.cpp : 457\n一行目\n\n"])
  def test_checkSingleLine(data)
    line, expectedResult, expectedStr = data
    os = MockOutputStream.new
    is = MockInputStream.new([])
    file = InputFile.new("", nil, os)
    filename = "foo.cpp"
    lineNumber = 456

    # 空行を入れる
    nextp, result = file.check(filename, lineNumber, "", nil, os)
    assert_nil(nextp)
    assert_false(result)

    # 空行以外を入れる
    lineNumber += 1
    paragraph, result = file.check(filename, lineNumber, line, nil, os)
    assert_not_nil(paragraph)
    assert_false(result)

    # 空行を入れる
    lineNumber += 1
    nextp, result = file.check(filename, lineNumber, "", paragraph, os)
    assert_nil(nextp)
    assert_equal(expectedResult, result)
    assert_equal(expectedStr, os.str)
  end

  data(
    'ascii only'  => [["first", "second", "third"], false, "foo.cpp : 456\nfirst second third\n\n"],
    'non ascii 2-1' => [["一行目", "second"], true, "Non-ASCII characters found in foo.cpp : 456\n一行目 second\n\n"],
    'non ascii 2-2' => [["first", "二行目"],  true, "Non-ASCII characters found in foo.cpp : 456\nfirst 二行目\n\n"],
    'non ascii 3-1' => [["一行目", "second", "third"], true, "Non-ASCII characters found in foo.cpp : 456\n一行目 second third\n\n"],
    'non ascii 3-2' => [["first", "二行目",  "third"], true, "Non-ASCII characters found in foo.cpp : 456\nfirst 二行目 third\n\n"],
    'non ascii 3-3' => [["first", "second", "三行目"], true, "Non-ASCII characters found in foo.cpp : 456\nfirst second 三行目\n\n"])
  def test_checkMultiLines(data)
    lines, expectedResult, expectedStr = data
    os = MockOutputStream.new
    is = MockInputStream.new([])
    file = InputFile.new("", nil, os)
    filename = "foo.cpp"
    lineNumber = 456
    paragraph = nil

    lines.each do |line|
      paragraph, result = file.check(filename, lineNumber, line, paragraph, os)
      assert_not_nil(paragraph)
      lineNumber += 1
    end

    # 空行を入れる
    lineNumber += 1
    nextp, result = file.check(filename, lineNumber, "", paragraph, os)
    assert_nil(nextp)
    assert_equal(expectedResult, result)
    assert_equal(expectedStr, os.str)
  end
end

class TestInputFileSet < Test::Unit::TestCase
  def test_getExtractor
    fileSet = InputFileSet.new([])
    assert_equal(0, fileSet.exitStatus)

    ["c", "h", "cpp", "hpp", "cc"].each do |suffix|
      fileSet.getExtractor("base." + suffix).instance_of?(CppCommentExtractor)
    end

    ["sh", "pl", "rb", "s"].each do |suffix|
      fileSet.getExtractor("base." + suffix).instance_of?(ScriptCommentExtractor)
    end

    ["README.md", "LICENSE.txt", "Makefile"].each do |filename|
      fileSet.getExtractor(filename).instance_of?(PlainExtractor)
    end

    ["Makefile", "Makefile.mk", "Makefile.in", "Makefile_var", "dir/Makefile"].each do |filename|
      fileSet.getExtractor(filename).instance_of?(ScriptCommentExtractor)
    end
  end
end

class TestActualInputFile < Test::Unit::TestCase
  data(
    'one line' => [["a"], " : 1\na\n\n"],
    'one paragraph'  => [["a", "b"], " : 1\na b\n\n"],
    'paragraphs 1' => [["", "b", "", "d", "e", "", "g", ""], " : 2\nb\n\n : 4\nd e\n\n : 7\ng\n\n"],
    'paragraphs 2'  => [["", "b", "", "d", "e", "", "g", "h"], " : 2\nb\n\n : 4\nd e\n\n : 7\ng h\n\n"])
  def test_plainFile(data)
    lines, expectedStr = data
    inTmpfile = Tempfile.new("input")
    inFilename = inTmpfile.path
    lines.each { |s| inTmpfile.puts s }
    inTmpfile.close

    os = MockOutputStream.new
    file = InputFile.new(inFilename, PlainExtractor.new, os)
    assert_false(file.failed)
    str = os.str.gsub(/\/\S+/, "")
    assert_equal(expectedStr, str)
  end

  data(
    'one line' => [["カナ"], "Non-ASCII characters found in  : 1\nカナ\n\n"],
    'one paragraph'  => [["a", "カナ"],
                         "Non-ASCII characters found in  : 1\na カナ\n\n"],
    'paragraphs 1' => [["", "b", "", "d", "カナ", "", "g", ""],
                       " : 2\nb\n\nNon-ASCII characters found in  : 4\nd カナ\n\n : 7\ng\n\n"],
    'paragraphs 2'  => [["", "b", "", "d", "e", "", "g", "カナ"],
                        " : 2\nb\n\n : 4\nd e\n\nNon-ASCII characters found in  : 7\ng カナ\n\n"],
    'paragraphs 3'  => [["", "b", "", "かな", "e", "", "g", "カナ"],
                        " : 2\nb\n\nNon-ASCII characters found in  : 4\nかな e\n\nNon-ASCII characters found in  : 7\ng カナ\n\n"])
  def test_plainKanaFile(data)
    lines, expectedStr = data
    inTmpfile = Tempfile.new("input")
    inFilename = inTmpfile.path
    lines.each { |s| inTmpfile.puts s }
    inTmpfile.close

    os = MockOutputStream.new
    file = InputFile.new(inFilename, PlainExtractor.new, os)
    assert_true(file.failed)
    str = os.str.gsub(/\/\S+/, "")
    assert_equal(expectedStr, str)
  end

  data(
    'paragraphs 1'  => [["", "// b", "", "かな", "e", "", "//g ", "カナ"], false,
                        " : 2\nb\n\n : 7\ng\n\n"],
    'paragraphs 2'  => [["", "// カナ", "", "# d", "e", "", "g", "//カナ"], true,
                        "Non-ASCII characters found in  : 2\nカナ\n\nNon-ASCII characters found in  : 8\nカナ\n\n"])
  def test_cppFile(data)
    lines, expectedResult, expectedStr = data
    inTmpfile = Tempfile.new("input")
    inFilename = inTmpfile.path
    mark = "#"
    lines.each { |s| inTmpfile.puts s }
    inTmpfile.close

    os = MockOutputStream.new
    file = InputFile.new(inFilename, CppCommentExtractor.new, os)
    assert_equal(expectedResult, file.failed)
    str = os.str.gsub(/\/\S+/, "")
    assert_equal(expectedStr, str)
  end

  data(
    'paragraphs 1'  => [["", "# b", "", "かな", "e", "", "#g ", "カナ"], false,
                        " : 2\nb\n\n : 7\ng\n\n"],
    'paragraphs 2'  => [["", "# カナ", "", "// d", "e", "", "# g", "# カナ"], true,
                        "Non-ASCII characters found in  : 2\nカナ\n\nNon-ASCII characters found in  : 7\ng カナ\n\n"])
  def test_scriptFile(data)
    lines, expectedResult, expectedStr = data
    inTmpfile = Tempfile.new("input")
    inFilename = inTmpfile.path
    lines.each { |s| inTmpfile.puts s }
    inTmpfile.close

    os = MockOutputStream.new
    file = InputFile.new(inFilename, ScriptCommentExtractor.new, os)
    assert_equal(expectedResult, file.failed)
    str = os.str.gsub(/\/\S+/, "")
    assert_equal(expectedStr, str)
  end

  def test_fileNotExist
    assert_equal(0, InputFileSet.new(["__file_does_not_exist__"]).exitStatus)
  end

  def test_dir
    Dir.mktmpdir("ascii_only_testing") { |dir|
      assert_equal(0, InputFileSet.new([dir]).exitStatus)
    }
  end
end
