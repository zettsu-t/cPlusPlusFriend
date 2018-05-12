#!/usr/bin/python3
# coding: utf-8

'''
Paint a green grass heatmap (tweet version)
'''

import numpy as np
import matplotlib.pyplot as plt
import seaborn
plt.figure()
seaborn.heatmap(np.random.poisson(2,364).reshape(7,52),cmap='Greens',cbar=False,xticklabels=False,yticklabels=False,square=True)
plt.show()
