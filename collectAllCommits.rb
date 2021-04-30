#!/usr/bin/ruby
# coding: utf-8
# Collects all commits of a file in the main branch.

require 'fileutils'
require 'open3'
require 'optparse'

STATUS_SUCCESS = 0
STATUS_ERROR = 1

def get_main_branch_name
  stdoutstr, stderrstr, status = Open3.capture3("git branch")
  unless status.success?
    puts stderrstr
    return nil
  end

  pattern = /\s+(master|main)\z/
  branches = stdoutstr.split(/\n+/).select{ |line| line.match?(pattern) }
  if branches.empty?
    return nil
  end

  branches.map { |name| name.match(pattern)[1] }[0]
end

def checkout_branch(branch)
  stdoutstr, stderrstr, status = Open3.capture3("git checkout #{branch}")
  unless status.success?
    puts stderrstr
    return STATUS_ERROR
  end

  STATUS_SUCCESS
end

def checkout_file(commit_id, filename)
  co_command = "git checkout #{commit_id} #{filename}"
  stdoutstr, stderrstr, status = Open3.capture3(co_command)
  unless status.success?
    return STATUS_ERROR
  end

  STATUS_SUCCESS
end

def checkout_files(in_filename, out_dirname)
  unless Dir.exist?(out_dirname)
    Dir.mkdir(out_dirname)
  end

  log_command = "git log --name-only"
  stdoutstr, stderrstr, status = Open3.capture3(log_command)
  unless status.success?
    puts stderrstr
    return STATUS_ERROR
  end

  main_branch = get_main_branch_name()
  if main_branch.nil?
    return STATUS_ERROR
  end

  if checkout_branch(main_branch) != STATUS_SUCCESS
    return STATUS_ERROR
  end

  commits = stdoutstr.split(/\n+/).select{ |line| line.match?(/\Acommit/) }
  commits.each_with_index do |commit, i|
    i_str = sprintf("%06d", i)
    commit_id = commit.strip.sub(/commit\s+/, "")
    if checkout_file(commit_id, in_filename) != STATUS_SUCCESS
      next
    end

    unless File.exist?(in_filename)
      next
    end

    filename = File.basename(in_filename)
    out_filename = filename.sub(/\./, "#{i_str}.")
    out_filename = File.join(out_dirname, out_filename)
    FileUtils.copy(in_filename, out_filename)
  end

  STATUS_SUCCESS
end

options = {}
opt = OptionParser.new
opt.on('-i', '--input [filename]', 'A filename to collect')   { |v| options[:input] = v }
opt.on('-o', '--output [dirname]', 'An output dirname') { |v| options[:outdir] = v }
opt.parse!(ARGV)

in_filename = options[:input]
out_direname = options[:outdir]
if in_filename.nil? || out_direname.nil?
  puts opt.help
  exit(0)
end

checkout_files(in_filename, out_direname)
