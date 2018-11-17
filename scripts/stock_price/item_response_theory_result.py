#!/usr/bin/python3
# coding: utf-8

'''
Parse results of item_response_theory.py
'''

from collections import namedtuple
import sys
import matplotlib.pyplot as plt
import numpy as np
import pandas
import pickle

in_filename = sys.argv[1] if len(sys.argv) > 1 else 'images/summary_all.pickle'
out_filename = in_filename.replace('.', '_') + '.png'

with open(in_filename, 'rb') as f:
    df = pickle.load(f)

df_success = df.loc[df['converged'] == True]
df_failed = df.loc[df['converged'] == False]
print(df_success['trial'])

fig = plt.figure()
ax = fig.add_subplot(1,1,1)
ax.scatter(df_failed['hmc_stepsize'], df_failed['hmc_leapfrog_steps'], label='Failed', c='royalblue', marker='.', alpha=0.8)
ax.set_xscale('log')
ax.set_yscale('log')
ax.scatter(df_success['hmc_stepsize'], df_success['hmc_leapfrog_steps'], label='Success',  marker='o', c='violet')
ax.set_xscale('log')
ax.set_yscale('log')
ax.set_title('Result')
ax.set_xlabel('HMC stepsize')
ax.set_ylabel('HMC leapfrog steps')
ax.legend(loc='upper right')

for i, hmc_stepsize in enumerate(df_success['hmc_stepsize']):
    hmc_leapfrog_steps = df_success.iloc[i]['hmc_leapfrog_steps']
    txt = '{0:.3e}, {1}'.format(hmc_stepsize, int(hmc_leapfrog_steps))
    ax.annotate(txt, (hmc_stepsize, hmc_leapfrog_steps))

plt.savefig(out_filename, dpi=120)
