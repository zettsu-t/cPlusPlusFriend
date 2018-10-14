#!/usr/bin/python3
# coding: utf-8

'''
Yokohama population statistics
Data source
http://www.city.yokohama.lg.jp/ex/stat/opendata/suikei01.html
'''

import glob
import os
import re
import numpy as np
import pandas
from collections import OrderedDict

COLUMN_NAME_FROM_TO = OrderedDict({'年月日': 'date',
                                   '全国地方公共団体コード': 'code',
                                   '市区名': 'ward',
                                   '面積[平方キロメートル]': 'area',
                                   '世帯数[世帯]': 'household',
                                   '人口総数[人]': 'population',
                                   '男[人]': 'male',
                                   '女[人]': 'female',
                                   '１世帯当たり人員[人]': 'per_household',
                                   '人口密度[人/平方キロメートル]': 'density',
                                   '届出による前月比増減の世帯数[世帯]': 'diff_household',
                                   '届出による前月比増減の人口[人]': 'diff_population'})

COLUMN_NAME_CHARACTERS =[['［','['], ['］',']'], ['／','/']]

WARDS = OrderedDict({'横浜市': ['All', 'A'],
                     '鶴見区': ['Tsurumi', 'N'],
                     '神奈川区': ['Kanagawa', 'N'],
                     '西区': ['Nishi', 'N'],
                     '中区': ['Naka', 'N'],
                     '南区': ['Minami', 'S'],
                     '港南区': ['Konan', 'S'],
                     '保土ケ谷区': ['Hodogaya', 'S'],
                     '旭区': ['Asahi', 'S'],
                     '磯子区': ['Isogo', 'S'],
                     '金沢区': ['Kanazawa', 'S'],
                     '港北区': ['Kohoku', 'N'],
                     '緑区': ['Midori', 'N'],
                     '青葉区': ['Aoba', 'N'],
                     '都筑区': ['Tsuduki', 'N'],
                     '戸塚区': ['Totsuka', 'S'],
                     '栄区': ['Sakae', 'S'],
                     '泉区': ['Izumi', 'S'],
                     '瀬谷区': ['Seya', 'S']})

column_names = list(COLUMN_NAME_FROM_TO.values())
out_table = None

def format_date(date_str):
    m = re.match(r'(\d+)\D+(\d+)\D(\d+)', date_str.strip())
    year = int(m.group(1))
    month = int(m.group(2))
    day = int(m.group(3))
    return '{0:04d}-{1:02d}-{2:02d}'.format(year, month, day)

for filename in sorted(glob.glob('yokohama/*.csv')):
    df = pandas.read_csv(filename)
    for (from_char, to_char) in COLUMN_NAME_CHARACTERS:
        df.columns = df.columns.str.replace(from_char, to_char)

    df = df.rename(columns=COLUMN_NAME_FROM_TO)
    df['ward_jp'] = df['ward']
    df['date'] = df['date'].apply(format_date)
    df['region'] = df['ward'].apply(lambda x: WARDS[x][1])
    df['ward'] = df['ward'].apply(lambda x: WARDS[x][0])

    if out_table is None:
        out_table = df
        base_population = df.loc[:, 'population']
        df.loc[:,'pop_ratio'] = np.full(df.shape[0], 1.0, dtype=float)
    else:
        df.loc[:,'pop_ratio'] = df.loc[:, 'population'] / base_population
        out_table = pandas.concat([out_table, df], ignore_index=True)

out_table.to_csv('incoming/yokohama.csv', index=None)
