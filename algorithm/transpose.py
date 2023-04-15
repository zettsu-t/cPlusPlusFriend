#!/usr/bin/python3
# python3 transpose.py < input.txt

import sys

max_len = 0
lines = []
reading = False

for line in sys.stdin:
    s = line.strip()
    if s.startswith("ABC"):
        reading = True
    if not reading:
        continue
    if reading and len(s) == 0:
        break

    lines.append(s)
    l = len(line)
    if l > max_len:
        max_len = l

filled_lines = []
for line in lines:
    filled_lines.append(line.ljust(max_len))

n_lines = len(filled_lines)
for i in range(max_len):
    s = ""
    for j in range(n_lines):
        s += filled_lines[j][i]
    print(s)
