#!/usr/bin/python
# coding: utf-8

import sys
import Levenshtein
import numpy as np

assert len(sys.argv) > 1

with open(sys.argv[1], 'r', encoding='utf-8') as file:
    lines = file.readlines()

n_lines = len(lines)
distances = np.zeros((n_lines, n_lines), dtype=int)
messages = []

for x in range(n_lines):
    for y in range(x + 1, n_lines):
        if x != y:
            value = Levenshtein.distance(lines[x], lines[y])
            distances[x,y] = value
            if value < 5:
                message = "lines {} and {} look similar\n{}{}\n".format(x, y, lines[x], lines[y])
                messages.append(message)

for message in messages:
    print(message)
