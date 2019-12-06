#!/usr/bin/ruby
# coding: utf-8

require 'optparse'

# 文章を分類するための正規表現(最初に一致するものを使う)
LINE_PATTERN_SET = [/\A\s*やめるのだ\s*フェネック/u, /けものフレンズ/u]

# プロプロセッサ指令およびハッシュタグ(前方一致)
HASHTAG_SET = ["define", "include", "if", "endif", "pragma",
               "やめるのだフェネックで学ぶ", "けものフレンズ",
               "はシャープ", "はハッシュタグ", "の後はコメント",
               "newgame", "imas_cg"]

# UTF-8で扱えない文字を置き換える規則
CHARACTER_ENCODING_PARAMETER_SET = { :invalid => :replace, :replace => " " }

# 実装の異常を見つけたら投げる例外
class AppRuntimeError < StandardError
end

# 指定されたパラメータでは動作できない場合に投げる例外
class AppInvalidParameter < StandardError
end

# コマンドオプション
class MyProgramOption
  attr_reader :patterns, :inFilename, :outFilename, :phrase, :checkTweets

  def initialize(argv)
    @patterns = LINE_PATTERN_SET

    options = {}
    OptionParser.new do |opt|
      opt.on('-i', '--input [filename]', 'UTF-8 input file name (default: stdin)')   { |v| options[:input] = v }
      opt.on('-o', '--output [filename]', 'UTF-8 output file name (default: stdout)') { |v| options[:output] = v }
      opt.on('-p', '--phrase [string]', 'phrase to find the last tweet (optional)')   { |v| options[:phrase] = v }
      opt.on('-c', '--[no-]check', 'check tweets (default: no checking tweets)')   { |v| options[:checkTweets] = v }
      opt.parse!(argv)
    end

    inFilename = options[:input]
    outFilename = options[:output]
    phrase = options[:phrase]
    checkTweets = options[:checkTweets]
    @inFilename = inFilename.nil? ? nil : inFilename.to_s.strip
    @outFilename = outFilename.nil? ? nil : outFilename.to_s.strip
    @phrase = phrase.nil? ? nil : phrase.to_s.strip
    @checkTweets = checkTweets.nil? ? false : checkTweets
  end
end

# 行を検査するインタフェースを持つが何もしない
class NullLineChecker
  def initialize(outStream)
  end

  # 指摘事項があればtrue, なければfalse
  def check(line)
    false
  end
end

# ツイートを検査する
class TweetChecker
  def initialize(outStream)
    @outStream = outStream
  end

  def check(argLine)
    line = argLine.chomp
    [:checkMention, :checkHashtag].reduce(false) { |r, m| r |= send(m, line) }
  end

  def checkMention(line)
    if line.match(/(\A|\s)@[\da-zA-Z]+(\z|\s)/)
      @outStream.puts "Mention(s) found in #{line}"
      true
    else
      false
    end
  end

  def checkHashtag(line)
    unknownHashtags = line.scan(/(\A|\s)\#(\S+)/).map do |tag|
      (tag.size < 2) || HASHTAG_SET.any? do |keyword|
        i = tag[1].strip.index(keyword)
        !i.nil? && i == 0
      end ? nil : tag[1]
    end.compact

    if unknownHashtags.empty?
      false
    else
      str = unknownHashtags.map { |tag| '#' + tag }.join(", ")
      @outStream.puts "Unknown tag(s) #{str} found"
      true
    end
  end
end

# 要素数がそれぞれm, n, ... 個の配列要素が、ほどよく入り混じるように並べ替える
class ArrayInterleaver
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

# 行=ツイートの集合
class LineSet
  def initialize(patterns)
    # 第二引数を[]にすると、配列要素はすべて同一オブジェクトへの参照になってしまう
    @lineArrays = Array.new(patterns.size + 1) { [] }
    @patterns = patterns
    @string = ""
  end

  # 1行=1ツイートを分類して追加する
  def addLine(line)
    i = @patterns.index { |pattern| line.match(pattern) }
    i ||= -1
    @lineArrays[i] << line
    nil
  end

  # 行=ツイートの集合を並べ替える
  def shuffle
    lineArrays = @lineArrays.map(&:shuffle)
    indexSizes = lineArrays.map(&:size)

    ArrayInterleaver.new.makeSequence(indexSizes).each do |i|
      # 改行コードを維持する
      @string += lineArrays[i].shift
    end

    # 残った行を移動する
    lineArrays.flatten.each { |line| @string += line }
    nil
  end

  # 並べ替えてできた文字列表現を返す
  def to_s
    @string
  end
end

# ファイルのすべての行を解析する
class LineSetParser
  def initialize(options)
    # ユニットテストでは、ファイルを開かずにここで終了することができる
    return unless options

    patterns = options.patterns
    inFilename = options.inFilename
    @outFilename = options.outFilename
    @phrase = options.phrase
    checker = options.checkTweets ? TweetChecker.new(STDERR) : NullLineChecker.new(STDERR)

    if !inFilename.nil?
      unless File.exist?(inFilename)
        raise AppInvalidParameter, "#{inFilename} does not exist."
      end

      if !@outFilename.nil? && File.identical?(inFilename, @outFilename)
        raise AppInvalidParameter, "Input and output files are identical."
      end
    end

    if inFilename
      File.open(inFilename, "r") { |inStream|
        @lineSetQueue, @linesRead = parseInputStream(patterns, inStream, checker)
      }
    else
      @lineSetQueue, @linesRead = parseInputStream(patterns, STDIN, checker)
    end
  end

  # ファイルの行を入れ替える
  def shuffle
    @lineSetQueue.each(&:shuffle)
    self
  end

  # ファイルの行を入れ替えたものを出力する
  def write
    @linesWritten = nil

    if @outFilename
      File.open(@outFilename, "w") { |outStream|
        @linesWritten = writeToStream(outStream)
      }
    else
      @linesWritten = writeToStream(STDOUT)
    end

    raise AppRuntimeError if  @linesRead != @linesWritten.lines.sort

    if @outFilename
      lines = []
      File.open(@outFilename, "r") { |inStream|
        while line = inStream.gets
          lines << line
        end
      }
      raise AppRuntimeError if  @linesRead != lines.sort
    end
  end

  def parseInputStream(patterns, inStream, checker)
    lineSet = LineSet.new(patterns)
    lineSetQueue = [lineSet]
    linesSorted = []
    found = @phrase.nil?

    while line = inStream.gets
      linesSorted << line
      lineSet.addLine(line)
      checker.check(line)
      if !@phrase.nil? && line.index(@phrase)
        lineSet = LineSet.new(patterns)
        lineSetQueue << lineSet
        found = true
        @phrase = nil
      end
    end

    raise AppInvalidParameter, "Cannot find #{@phrase} in the input." unless found
    return lineSetQueue, linesSorted.sort
  end

  def writeToStream(outStream)
    @lineSetQueue.reverse.map do |lineSet|
      lines = lineSet.to_s
      outStream.write(lines)
      lines
    end.join("")
  end
end
