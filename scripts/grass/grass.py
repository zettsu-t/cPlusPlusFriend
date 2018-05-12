#!/usr/bin/python3
# coding: utf-8

'''
Paint a green grass heatmap (full version)
'''

import numpy as np
import matplotlib.pyplot as plt
import seaborn

n_week = 52
n_day = n_week * 7
mean_push_per_day = 2
data = np.random.poisson(mean_push_per_day, n_day)

plt.figure(figsize=(5.2, 1.0), dpi=200)
ax = seaborn.heatmap(data.reshape(-1, n_week),
                     cmap='Greens', cbar=True,
                     xticklabels=False, yticklabels=False, square=True)

ticks = range(0, int(np.ceil(max(data))) + mean_push_per_day - 1, mean_push_per_day)
ax.collections[0].colorbar.set_ticks(ticks)
plt.savefig('green_grass.png')
