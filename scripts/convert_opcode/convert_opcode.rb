#!/usr/bin/ruby
# coding: utf-8

# Converting HEX strings to x86 mnemonics
#
# Usage: pass a HEX word to the first argument
# $ ruby convert_opcode.rb coffee
# > c0,ff,ee  sar bh,0xee
#
# This script
# - treats o as 0 and i,l as 1 and applies less common rules
# - writes convert_opcode.out and does not delete it automatically

require 'open3'

INPUT_C_FILENAME = 'convert_opcode.c'
OUTPUT_EXE_FILENAME = 'convert_opcode.out'

class HexspeakToOpcodes
  def initialize(phrase)
    title, code_seq_set = get_code_seq_set(phrase)
    puts(title)
    code_seq_set.each do |code_seq|
      mnemonics = get_mnemonic(code_seq)
      puts(mnemonics, "---")
    end
  end

  # Converts "abc" to [[0x0a, 0xbc], [0xab, 0xc0]]
  def get_code_seq_set(arg)
    # Common rules
    phrase = arg.strip().downcase().tr("ilo", "110")
    # Less common rules
    phrase = phrase.strip().tr("gstz", "6572")
    phrase = phrase.strip().gsub("r", "12")
    title = "#{phrase.upcase()} as #{arg.upcase()}"

    matched = phrase.match(/\A([\da-f])+\z/)
    unless matched
      warn("#{arg} is not a HEX number")
      exit(1)
    end

    hex_digit_strs = phrase.size.even?() ? [phrase] : ['0' + phrase, phrase + '0']
    code_seq_set = hex_digit_strs.map{ |digit_str| get_byte_seq(digit_str) }
    return title, code_seq_set
  end

  # Converts "0abc" to [0x0a, 0xbc]
  def get_byte_seq(hex_digit_str)
    return hex_digit_str.split('').each_slice(2).map do |hex_digits|
      "0x" + hex_digits.join("")
    end.join(",")
  end

  # Compiles code_seq and returns opcodes-mnemonic strings
  # which are matched to code_seq
  def get_mnemonic(code_seq)
    found = false
    code_remaining = split_to_bytes(code_seq)

    mnemonics = []
    get_disassembly(code_seq).each_line do |raw_line|
      line = raw_line.downcase().strip()
      if found
        code_remaining, mnemonic = parse_mnemonic(code_remaining, line)
        unless mnemonic.empty?
          mnemonics << mnemonic
        end

        if code_remaining.empty?
          break
        end
      end

      # Find int3
      if line.match(/\A\d+:\s+cc\s+int3\z/)
        found = true
      end
    end

    return mnemonics
  end

  # Converts "0xab,0xc0" to ["ab", "c0"]
  def split_to_bytes(line)
    line.strip().gsub("0x", "").split(/,|\s+/)
  end

  # Returns an opcodes-mnemonic string when bytes in code_seq are
  # fully or partially matched in opcodes of a line
  def parse_mnemonic(code_seq, line)
    code_remaining = []
    mnemonic = ''

    matched = line.match(/\A\d+:\s+(([\da-f]{2}\s+)+)(\S.*)/)
    # Not a mnemonic
    unless matched
      return code_remaining, mnemonic
    end

    matched_asm = matched[3].strip().gsub(/\s+/, " ")
    matched_code = split_to_bytes(matched[1])

    last_index = -1
    code_seq.each_with_index do |code, index|
      if index >= matched_code.size || code != matched_code[index]
        break
      end
      last_index = index
    end

    if last_index < 0
      return code_remaining, mnemonic
    end

    mnemonic = code_seq[0..last_index].join(',') + '  ' + matched_asm
    last_index += 1
    # Partially matched
    if last_index == code_seq.size && last_index < matched_code.size
      mnemonic += " (" + matched_code[last_index..-1].join(',') + " added)"
    end

    # Fully matched and may continue to the following lines
    if last_index < code_seq.size && last_index == matched_code.size
      code_remaining = code_seq[last_index..-1]
    end

    return code_remaining, mnemonic
  end

  # Compiles, links and disassembles with code_seq
  def get_disassembly(code_seq)
    command = "gcc -mavx2 -O0 -g -c -o #{OUTPUT_EXE_FILENAME} #{INPUT_C_FILENAME}"
    command += ' -DMY_INSTRUCTIONS=\\"' + code_seq + '\\"'
    exec(command)
    command = "objdump -d -M intel #{OUTPUT_EXE_FILENAME}"
    exec(command)
  end

  # Executes a command and get its stdout lines
  def exec(command)
    stdoutstr, stderrstr, status = Open3.capture3(command)
    unless status.success?
      warn("#{command} failed")
      exit(1)
    end
    stdoutstr
  end
end

arg = ARGV.empty? ? "coffee" : ARGV[0]
HexspeakToOpcodes.new(arg)
0
