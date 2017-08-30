#!/usr/bin/ruby
# coding: utf-8

# Boost Filesystem で同一ファイルかどうか調べるテストを準備する
# 第一引数 = 動作(create = 生成, delete = 削除)
# 第二引数 = 生成するC++ヘッダファイル名
HELP_MESSAGE = "usage: cppFriendsFile.rb {create outputfile.hpp|delete} "

# 最初に作るファイル
FILENAME_NODE =  "__generated_text_file_.txt"
VARNAME_NODE = "g_cppFriendsNodeFilename"
# 上記へのシンボリックリンク
FILENAME_SYMLINK = "__generated_symlink__.txt"
VARNAME_SYMLINK = "g_cppFriendsSymlinkFilename"
# 上記へのハードリンク
FILENAME_HARDLINK = "__generated_hardlink__.txt"
VARNAME_HARDLINK = "g_cppFriendsHardlinkFilename"
# 生成した全ファイル
FILENAME_SET = [FILENAME_NODE, FILENAME_SYMLINK, FILENAME_HARDLINK]

class AppInvalidParameter < StandardError
end

class FileCreater
  def initialize(cppFilename)
    generateFiles(cppFilename)
  end

  def generateFiles(cppFilename)
    guard = cppFilename.upcase.gsub(/[^[:alnum:]]/,"_").gsub(/_+/,"_")
    guard += "_ALREADY_INCLUDED"

    File.open(cppFilename, "w") { |outStream|
      ["#ifndef #{guard}", "#define #{guard}", "namespace {",
       generateNode(), generateSymlink(), generateHardlink(),
       "}", "#endif"].each { |str| outStream.puts str}
    }
  end

  def generateNode
    filename = FILENAME_NODE
    File.open(filename, "w") { |stream|
      stream.puts "Genereted"
    }

    getString(VARNAME_NODE, filename)
  end

  def generateSymlink
    filename = ""
    begin
      if File.symlink(FILENAME_NODE, FILENAME_SYMLINK) == 0
        filename = FILENAME_SYMLINK
      end
    rescue
      # シンボリックリンクを作れなければファイル名を空文字列にする
    end

    getString(VARNAME_SYMLINK, filename)
  end

  def generateHardlink
    filename = ""
    begin
      if File.link(FILENAME_NODE, FILENAME_HARDLINK) == 0
        filename = FILENAME_HARDLINK
      end
    rescue
      # ハードリンクを作れなければファイル名を空文字列にする
    end

    getString(VARNAME_HARDLINK, filename)
  end

  def getString(varname, filename)
    '    const char* ' + varname + ' = "' + filename + '";'
  end
end

class FileDeleter
  def initialize
    FILENAME_SET.each do |filename|
      begin
        File.delete(filename)
      rescue
      end
    end
  end
end

class Launcher
  def initialize(argv)
    raise AppInvalidParameter, HELP_MESSAGE if argv.size < 1
    op = argv[0].strip.downcase

    case op
    when "create"
      raise AppInvalidParameter, HELP_MESSAGE if argv.size < 2
      FileDeleter.new
      FileCreater.new(argv[1].strip)
    when "delete"
      FileDeleter.new
    else
      raise AppInvalidParameter
    end
  end
end

Launcher.new(ARGV)
exit(0)
