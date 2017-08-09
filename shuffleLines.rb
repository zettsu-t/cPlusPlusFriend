#!/usr/bin/ruby
# coding: utf-8

require_relative './shuffleLinesImpl.rb'

LineSetParser.new(MyProgramOption.new(ARGV)).shuffle.write
0
