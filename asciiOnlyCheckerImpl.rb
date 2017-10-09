#!/usr/bin/ruby
# -*- coding: utf-8 -*-
# This script checks whether files have US-ASCII characters only.

# これらがファイル名の末尾についていたら調べない
FILENAME_ENDED_IGNORED = ["~", "bak"]

# 接頭辞
PREFIX_SET_SCRIPT = ["Makefile"]

# 拡張子(.を除く)
SUFFIX_SET_CPP = ["c", "h", "cpp", "hpp", "cc"]
SUFFIX_SET_SCRIPT = ["sh", "pl", "rb", "s", "mk", "in"]

class Paragraph
  def initialize(filename, lineNumber, line)
    @str = "#{filename} : #{lineNumber}\n#{line}"
  end

  def add(line)
    @str += " "
    @str += line
    self
  end

  def print(outStream)
    failed = false
    unless @str.empty?
      failed = !@str.ascii_only?
      prefix = failed ? "Non-ASCII characters found in " : ""
      outStream.puts "#{prefix}#{@str}\n\n"
    end
    failed
  end
end

class PlainExtractor
  def initialize
  end

  # Extractorによっては、複数行に分割できる
  def extract(line)
    [line.strip]
  end
end

class CppCommentExtractor
  def initialize
  end

  def extract(line)
    # 最初のコメント開始記号に一致させるのは、greedyではなくreluctantにする
    result = [""]
    if md = line.strip.match(/\A(.*?)\/\/\s*(.+)\z/)
      # コードのある行は独立した段落にする
      result = [] if md[1].strip.empty?
      result << md[2]
    end

    result
  end
end

class ScriptCommentExtractor
  def initialize
  end

  def extract(line)
    # // と同様
    result = [""]
    if md = line.strip.match(/\A(.*?)\#\s*(.+)\z/)
      result = [] if md[1].strip.empty?
      result << md[2]
    end

    result
  end
end

class InputFile
  attr_reader :failed

  def initialize(filename, extractor, outStream)
    @failed = false
    # ユニットテスト専用
    return if filename.empty?

    File.open(filename, "r") { |inStream|
      @failed = parse(filename, extractor, outStream, inStream)
    }
  end

  def parse(filename, extractor, outStream, inStream)
    lineNumber = 1
    paragraph = nil
    result = false

    while line = inStream.gets
      extractor.extract(line.chomp).each do |str|
        paragraph, failed = check(filename, lineNumber, str, paragraph, outStream)
        result |= failed
      end
      lineNumber += 1
    end

    paragraph, failed = check(filename, lineNumber, "", paragraph, outStream)
    result | failed
  end

  def check(filename, lineNumber, line, paragraph, outStream)
    result = [nil, failed]
    if line.empty?
      failed = paragraph ? paragraph.print(outStream) : false
      result = [nil, failed]
    else
      result = [paragraph ? paragraph.add(line) : Paragraph.new(filename, lineNumber, line), false]
    end
    result
  end
end

class InputFileSet
  attr_reader :exitStatus

  def initialize(filenameSet)
    @cppCommentExtractor = CppCommentExtractor.new
    @scriptCommentExtractor = ScriptCommentExtractor.new
    @plainExtractor = PlainExtractor.new

    failed = filenameSet.reduce(false) do |result, filename|
      # シンボリックリンクは未対応
      if FILENAME_ENDED_IGNORED.any? { |str| filename.end_with?(str) } || !File.file?(filename)
        warn "! skip #{filename}"
        next
      end

      extractor = getExtractor(filename)
      result | InputFile.new(filename, extractor, STDOUT).failed
    end
    @exitStatus = failed ? 1 : 0
  end

  def getExtractor(filename)
    name = File.basename(filename)
    return @scriptCommentExtractor if PREFIX_SET_SCRIPT.any? { |str| name.start_with?(str) }

    extractor = @plainExtractor
    md = name.match(/\.([a-zA-Z\d]+)\z/)
    return extractor unless md
    suffix = md[1]

    return @cppCommentExtractor if SUFFIX_SET_CPP.any? { |ext| ext == suffix }
    return @scriptCommentExtractor if SUFFIX_SET_SCRIPT.any? { |ext| ext == suffix }
    extractor
  end
end
