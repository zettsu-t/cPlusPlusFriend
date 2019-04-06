#!/usr/bin/ruby
# coding: utf-8

require "open3"

# Cited from
# https://en.cppreference.com/w/cpp/types/type_identity
HEADER = "#include <functional>
#include <type_traits>
template<class T>
struct type_identity {
    using type = T;
};
template<class T>
using type_identity_t = typename type_identity<T>::type;
int main(int argc, char* argv[]) { return 0; }
static_assert(std::is_integral_v<"

def compile(n)
  puts "n=#{n}"
  exefile = "out"
  cppfile = "out.cpp"
  File.open(cppfile, "w") { |ofs|
    [HEADER, "type_identity_t<" * n, "int", ">" * n, ">);\n"].each do |line|
      ofs.write(line)
    end
  }

  os, er, status = Open3.capture3("rm #{exefile}")
  if File.exists?(exefile)
    abort("Cannot delete #{exefile}")
  end
  os, er, status = Open3.capture3("g++ -std=c++17 -o #{exefile} #{cppfile}")
  system("#{exefile}") == true ? true : false
end

n = 1
result = true
while(result) do
  n = n * 2
  result = compile(n)
end

puts
actual = ((n/4)..(n*2)).bsearch { |i| !compile(i) }
puts "Min failed n=#{actual} result=#{compile(actual)}"
actual = actual - 1
puts "Max passed n=#{actual} result=#{compile(actual)}"
