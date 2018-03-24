#!/usr/bin/python3
# coding: utf-8

'''
Prefilter tweets to a bayesian analysis
'''

import pandas
import re
import sys
import warnings
from collections import OrderedDict

# Suppress warnings to assign new columns as df['A'] = value
pandas.options.mode.chained_assignment = None

COLUMN_NAME_FROM_TO = OrderedDict({'時間': 'time',
                                   'インプレッション': 'impressions',
                                   'リツイート': 'retweets',
                                   '返信': 'replies',
                                   'いいね': 'likes'})

DEFAULT_INFILENAME  = 'data/in2.csv' # input data filename
DEFAULT_OUTFILENAME = 'data/converted2.csv' # output data filename
EXIT_STATUS_SUCCESS = 0

class TweetImpressions(object):
    '''Filter a tweet CSV file and write it to another'''

    def __init__(self, infilename, outfilename):
        # UTC Time
        self.autopost_regex = re.compile('.* (22|02|06|10|14):0[0-3] ')
        tweets = self.parse(infilename)
        tweets.to_csv(outfilename, index_label='index')

    def parse(self, infilename):
        '''Parses UTF-8 CSV lines'''
        in_tweets = pandas.read_csv(infilename)
        in_tweets.rename(columns=COLUMN_NAME_FROM_TO, inplace=True)
        column_names = list(COLUMN_NAME_FROM_TO.values())

        out_tweets = in_tweets[column_names]
        out_tweets['time_index'] = out_tweets['time'].apply(lambda x: self.get_time_index(x))
        out_tweets['autopost'] = out_tweets['time_index'].apply(lambda x: x >= 0)
        return out_tweets

    def get_time_index(self, time_str):
        '''Get time index if a tweet is automatically posted otherwise -1'''

        matched = self.autopost_regex.match(time_str)
        time_index = -1
        if matched is not None:
            # Convert JST 07:00, 11:00, 15:00, 19:00, 23:00 to 0..4
            try:
                time_index = ((int(matched[1]) + 24 + 9 - 7) % 24) // 4
            except ValueError:
                # failed to parse as a number
                pass

        return time_index

if __name__ == "__main__":
    in_filename = DEFAULT_INFILENAME
    out_filename = DEFAULT_OUTFILENAME
    if len(sys.argv) > 1:
        in_filename = sys.argv[1]
    if len(sys.argv) > 2:
        out_filename = sys.argv[n]

    TweetImpressions(in_filename, out_filename)
    sys.exit(EXIT_STATUS_SUCCESS)
