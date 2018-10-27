#!/usr/bin/python3
# coding: utf-8

'''
Example for change point detection
Industrial accident fatalities in Japan since 1958
Data source:
https://www.mhlw.go.jp/stf/houdou/0000165073.html
http://www.aichi-c.ed.jp/digitalcontents/Web_koug/zenpan/anzen/anzen_top.htm

Usage:
$ python3 industrial_accident_fatality.py
It makes samples, analyzes them and draws charts from its result.

$ python3 industrial_accident_fatality.py dump
It loads changepoint_pymc.pickle (the default pickle file name) and draws charts from it.

$ python3 industrial_accident_fatality.py foo.pickle
It loads foo.pickle and draws charts from it.
'''

from collections import namedtuple
import matplotlib
# matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import pandas
import pickle
import pymc3 as pm
import re
import sys

N_WARMUP = 2500
N_BURNED = 5000
N_CHAINS = 3

DEFAULT_DUMP_FILENAME = 'changepoint_pymc.pickle'
CSV_FILENAME = 'industrial_accident_fatality.csv'
LINE_CHART_FILENAME = 'changepoint_pymc.png'
TRACE_FILENAME = 'changepoint_traceplot.png'

Result = namedtuple('Result', ('trace', 'df', 'years', 'fatalities'))

## Based on
## https://docs.pymc.io/notebooks/getting_started.html
## https://cscherrer.github.io/post/bayesian-changepoint/
class YearlyData(object):
    '''Find a change point'''
    def __init__(self, in_filename):
        self.result = None
        if in_filename is None:
            self.result = self.analyze(CSV_FILENAME)
        else:
            dump_filename = DEFAULT_DUMP_FILENAME
            if re.match(in_filename, '\\.pickle') is not None:
                dump_filename = in_filename
            with open(dump_filename, mode='rb') as file:
                self.result = pickle.load(file)

    def analyze(self, in_filename):
        df = pandas.read_csv(in_filename)
        years = df['year'].values
        fatalities = df['fatality'].values

        result = None
        model = pm.Model()
        with model:
            ## Prior and posterior distributions and a switch point
            mu_e = pm.Uniform('mu_e', lower=0, upper=10000)
            mu_l = pm.Uniform('mu_l', lower=0, upper=10000)
            sd_e = pm.Lognormal('sd_e', 7.0)
            sd_l = pm.Lognormal('sd_l', 5.0)
            switchpoint = pm.DiscreteUniform('switchpoint', lower=years.min(), upper=years.max())

            ## Switches distributions before and after the switchpoint
            mu = pm.math.switch(years < switchpoint, mu_e, mu_l)
            sd = pm.math.switch(years < switchpoint, sd_e, sd_l)
            y_obs = pm.Normal('y_obs', mu=mu, sd=sd, observed=fatalities)

            ## Starts sampling and gets traces
            step = pm.Slice()
            trace = pm.sample(draws=N_BURNED, tune=N_WARMUP, step=step, cores=1, chains=N_CHAINS)
            result = Result(trace=trace, df=df, years=years, fatalities=fatalities)
            with open(DEFAULT_DUMP_FILENAME, mode='wb') as file:
                pickle.dump(result, file)

        return result

    def plot(self):
        tau = self.result.trace['switchpoint'].mean()
        cp = int(tau)
        fontsize = 16
        sd_width = 1.96

        xs_e = list(range(self.result.years[0], cp))
        xs_l = list(range(cp, self.result.years[-1]))
        mu_e = self.result.trace['mu_e'].mean()
        mu_l = self.result.trace['mu_l'].mean()
        sd_e = self.result.trace['sd_e'].mean()
        sd_l = self.result.trace['sd_l'].mean()
        ymin_e = np.full(len(xs_e), mu_e) - sd_width * sd_e
        ymax_e = np.full(len(xs_e), mu_e) + sd_width * sd_e
        ymin_l = np.full(len(xs_l), mu_l) - sd_width * sd_l
        ymax_l = np.full(len(xs_l), mu_l) + sd_width * sd_l

        ## Draws the data and estemated changepoint
        plt.figure(figsize=(6, 6))
        plt.fill_between(xs_e, ymin_e, ymax_e, color='lightgray')
        plt.fill_between(xs_l, ymin_l, ymax_l, color='lightgray')
        plt.plot(self.result.years, self.result.fatalities)
        plt.plot([self.result.years[0], tau], [mu_e, mu_e], color='red')
        plt.plot([tau, self.result.years[-1]], [mu_l, mu_l], color='red')
        plt.axvline(x=tau, color='orchid')

        plt.title('Industrial Accident Fatalities (yearly)', fontsize=fontsize)
        plt.ylabel('fatality', fontsize=fontsize)
        plt.xlabel('year', fontsize=fontsize)
        plt.tick_params(labelsize=fontsize)
        plt.text(tau, 300, '{0:4.1f}'.format(tau), color='orchid', fontsize=fontsize)
        plt.tight_layout()
        plt.savefig(LINE_CHART_FILENAME, dpi=160)

        ## Draws the trace
        plt.figure(figsize=(6, 6))
        pm.plots.traceplot(self.result.trace)
        plt.tight_layout()
        plt.savefig(TRACE_FILENAME, dpi=160)
        return 0

if __name__ == '__main__':
    in_filename = None
    if len(sys.argv) > 1:
        in_filename = sys.argv[1]
    exit_status = YearlyData(in_filename).plot()
    sys.exit(exit_status)
