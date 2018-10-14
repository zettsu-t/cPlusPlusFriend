#!/usr/bin/python3
# coding: utf-8

'''
Analyze a CSV generated by simulate_event.R
'''

import matplotlib.pyplot as plt
import numpy as np
import pandas
import pymc3 as pm

## Use same CSV files as generated by simulate_event.R
in_filename = 'visitors.csv'
df = pandas.read_csv(in_filename)

## https://docs.pymc.io/notebooks/getting_started.html
model = pm.Model()
with model:
    mu_weekday = pm.Uniform('mu_weekday', lower=0, upper=1)
    sd_weekday = pm.Exponential('sd_weekday', 1)
    mu_holiday = pm.Uniform('mu_holiday', lower=0, upper=1)
    sd_holiday = pm.Exponential('sd_holiday', 1)

    df_weekday = df.loc[(df['event']==0) & (df['day_of_week']<=5),:]
    df_holiday = df.loc[(df['event']==0) & (df['day_of_week']>5),:]
    commutor_weekday = np.array(df_weekday.commutor)
    commutor_holiday = np.array(df_holiday.commutor)

    weekday = pm.Normal('weekday', mu=mu_weekday, sd=sd_weekday, observed=commutor_weekday)
    holiday = pm.Normal('holiday', mu=mu_holiday, sd=sd_holiday, observed=commutor_holiday)
    start = pm.find_MAP(model = model)
    trace = pm.sample(5000, tune=1500, start=start, cores=1, chains=4)
    pm.plots.traceplot(trace)
    plt.savefig("event_py.png")
    plt.show()
