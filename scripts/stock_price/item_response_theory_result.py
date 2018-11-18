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

in_filename = sys.argv[1] if len(sys.argv) > 1 else 'images/summary.csv'
out_filename = in_filename.replace('.', '_') + '.png'

if '.csv' in in_filename:
    df = pandas.read_csv(in_filename)
else:
    with open(in_filename, 'rb') as f:
        df = pickle.load(f)

df_success = df.loc[df['converged'] == True]
df_failed = df.loc[df['converged'] == False]
print('Successful trials', df_success['trial'].reshape(-1))

def point_size(df_target):
    return [40.0 * np.log1p(df_target['trial'])]

fig = plt.figure()
ax = fig.add_subplot(1,1,1)
ax.scatter(x=df_failed['hmc_stepsize'], y=df_failed['hmc_leapfrog_steps'], s=point_size(df_failed),
           label='Failed', c='royalblue', marker='o', alpha=0.6)
ax.scatter(x=df_success['hmc_stepsize'], y=df_success['hmc_leapfrog_steps'], s=point_size(df_success),
           label='Success',  marker='o', c='violet', alpha=0.5)
ax.set_xscale('log')
ax.set_yscale('log')
ax.set_title('Result')
ax.set_xlabel('HMC stepsize')
ax.set_ylabel('HMC leapfrog steps')
ax.tick_params(direction='out', which='major', length=10, width=1)
ax.tick_params(direction='out', which='minor', length=8, width=1)
ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), ncol=2)

for i, hmc_stepsize in enumerate(df_success['hmc_stepsize']):
    hmc_leapfrog_steps = df_success.iloc[i]['hmc_leapfrog_steps']
##  txt = '{0:.3e}, {1}'.format(hmc_stepsize, int(hmc_leapfrog_steps))
##  ax.annotate(txt, (hmc_stepsize, hmc_leapfrog_steps))

plt.savefig(out_filename, dpi=120, bbox_inches='tight')
