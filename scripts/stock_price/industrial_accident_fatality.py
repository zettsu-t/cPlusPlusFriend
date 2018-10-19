#!/usr/bin/python3
# coding: utf-8

'''
Example for change point detection
Industrial accident fatalities in Japan since 1958
Data source are
https://www.mhlw.go.jp/stf/houdou/0000165073.html
http://www.aichi-c.ed.jp/digitalcontents/Web_koug/zenpan/anzen/anzen_top.htm
'''

import matplotlib.pyplot as plt
import numpy as np
import pandas
import pymc3 as pm

df = pandas.read_csv('industrial_accident_fatality.csv')
years = df['year'].values
fatalities = df['fatality'].values

## Based on
## https://docs.pymc.io/notebooks/getting_started.html
## https://cscherrer.github.io/post/bayesian-changepoint/
model = pm.Model()
with model:
    mu_e = pm.Uniform('mu_e', lower=0, upper=10000)
    mu_l = pm.Uniform('mu_l', lower=0, upper=10000)
    sd_e = pm.Exponential('sd_e', 500)
    sd_l = pm.Exponential('sd_l', 100)
    trend_l = pm.Normal('trend_l', 200)
    switchpoint = pm.DiscreteUniform('switchpoint', lower=years.min(), upper=years.max())

    mu_l_offset = pm.math.switch(switchpoint > years, 0, (years - switchpoint) * trend_l)
    mu = pm.math.switch(years < switchpoint, mu_e, mu_l + mu_l_offset)
    sd = pm.math.switch(years < switchpoint, sd_e, sd_l)
    y_obs = pm.Normal('y_obs', mu=mu, sd=sd, observed=fatalities)
    step = pm.Slice()
    trace = pm.sample(10000, tune=5000, step=step, cores=1, chains=1)
    tau = trace['switchpoint'].mean()

    plt.subplot(111)
    plt.plot(years, fatalities);
    plt.title('Industrial Accident Fatalities (yearly)')
    plt.ylabel('fatality')
    plt.xlabel('year');
    plt.axvline(x=trace['switchpoint'].mean(), color='orchid')
    plt.text(tau, -5, str(tau), color='orchid')
    plt.tight_layout()
    plt.savefig('pymc_changepoint.png', dpi=80)

    plt.subplot(111)
    pm.plots.traceplot(trace)
    plt.vlines(tau, fatalities.min(), fatalities.max())
    plt.tight_layout()
    plt.savefig('traceplot.png')
