#!/usr/bin/python3
# coding: utf-8

'''
Parse and draw results of item_response_theory.py
'''

from collections import namedtuple
import sys
import matplotlib.pyplot as plt
import numpy as np
import pandas
import pickle

DEFAULT_INPUT_FILENAME = 'images/summary.csv'

class QuestionAndAbilityResult(object):
    '''Draw reesults of the item response theory script'''
    def __init__(self, argv):
        in_filename = argv[1] if len(argv) > 1 else DEFAULT_INPUT_FILENAME
        self.out_filename = in_filename.replace('.', '_') + '.png'

        self.df = None
        if '.csv' in in_filename:
            self.df = pandas.read_csv(in_filename)
        else:
            with open(in_filename, 'rb') as f:
                self.df = pickle.load(f)

        self.df_success = self.df.loc[self.df['converged'] == True]
        self.df_failure = self.df.loc[self.df['converged'] == False]
        print('Successful trials', self.df_success['trial'].reshape(-1))

    @staticmethod
    def add_jitter(xs):
        return xs * np.random.normal(loc=1.0, scale=0.025, size=len(xs))

    @staticmethod
    def point_size(df_target):
        return np.clip(a=[40.0 * np.log1p(df_target['trial'])], a_min=40.0, a_max=200.0)

    def draw_scatter(self, ax, df, label, color, marker, alpha):
        xs = self.add_jitter(df['hmc_stepsize'])
        ys = self.add_jitter(df['hmc_leapfrog_steps'])
        ss = self.point_size(df)
        ax.scatter(x=xs, y=ys, s=ss, label=label, c=color, marker=marker, alpha=alpha)
        return ax

    def draw(self):
        fig = plt.figure()
        ax = fig.add_subplot(1,1,1)
        self.draw_scatter(ax, self.df_failure, 'Failure', 'royalblue', 'x', 0.8)
        self.draw_scatter(ax, self.df_success, 'Success', 'darkorchid', 'o', 0.3)
        ax.set_xscale('log')
        ax.set_yscale('log')
        ax.set_title('Result')
        ax.set_xlabel('HMC stepsize')
        ax.set_ylabel('HMC leapfrog steps')
        ax.tick_params(direction='out', which='major', length=10, width=1)
        ax.tick_params(direction='out', which='minor', length=8, width=1)
        ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), ncol=2)

        for i, hmc_stepsize in enumerate(self.df_success['hmc_stepsize']):
            hmc_leapfrog_steps = self.df_success.iloc[i]['hmc_leapfrog_steps']
##          txt = '{0:.3e}, {1}'.format(hmc_stepsize, int(hmc_leapfrog_steps))
##          ax.annotate(txt, (hmc_stepsize, hmc_leapfrog_steps))
        plt.savefig(self.out_filename, dpi=120, bbox_inches='tight')
        plt.close()

if __name__ == '__main__':
    QuestionAndAbilityResult(sys.argv).draw()
