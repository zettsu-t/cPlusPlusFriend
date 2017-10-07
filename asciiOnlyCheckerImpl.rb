#!/usr/bin/ruby
# -*- coding: utf-8 -*-
# This script checks whether files have US-ASCII characters only.

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

  def extract(line)
    line.strip
  end
end

class CppCommentExtractor
  def initialize
    @parent = PlainExtractor.new
  end

  def extract(line)
    md = @parent.extract(line).match(/\/\/\s*(.+)\z/)
    md ? md[1].strip : ""
  end
end

class ScriptCommentExtractor
  def initialize
    @parent = PlainExtractor.new
  end

  def extract(line)
    md = @parent.extract(line).match(/\#(.+)\z/)
    md ? md[1].strip : ""
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
      paragraph, failed = check(filename, lineNumber, extractor.extract(line.chomp), paragraph, outStream)
      result |= failed
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
      extractor = getExtractor(filename)
      result | InputFile.new(filename, extractor, STDOUT).failed
    end
    @exitStatus = failed ? 1 : 0
  end

  def getExtractor(filename)
    extractor = @plainExtractor
    return @scriptCommentExtractor if filename.match(/\AMakefile/)

    md = filename.match(/\.([a-zA-Z\d]+)\z/)
    return extractor unless md
    suffix = md[1]

    return @cppCommentExtractor if ["c", "h", "cpp", "hpp", "cc"].any? { |ext| ext == suffix }
    return @scriptCommentExtractor if ["sh", "pl", "rb", "s", "mk", "in"].any? { |ext| ext == suffix }
    extractor
  end
end
