#!/usr/bin/ruby
# coding: utf-8
puts 'FEFF82B985BA5FA15F627E417E374ECF306E5EA783D8863F8461'.scan(/.{4}/).map(&:hex).pack("U*") + "これぞ七草"
