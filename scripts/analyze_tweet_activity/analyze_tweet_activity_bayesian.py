#!/usr/bin/python3
# coding: utf-8

'''
Prefilter tweets to a bayesian analysis
Usage:
$ python analyze_tweet_activity_bayesian.py [--hour_span N] [input-csv-filename [output-csv-filename]]
- --hour_span N indicates there are N hours between tweets
- It extracts columns for the analysis and convert column names of an input CSV
- It removes tweet texts because they are possibly contain characters
  which R cannot handle such as U+10000 emojis
- It adds time_index and autopost columns
 + time_index is
  - -1 for tweets posted manually
  - 0 for tweets posted 7:00 automatically (the first of a day),
  - 1 for second (7:00 + N hours), 2 for third (7:00 + 2*N hours), and so on
 + autopost indicates whether each tweet are posted automatically
'''

import pandas
import re
import sys
import warnings
from collections import OrderedDict
from optparse import OptionParser

# Suppress warnings to assign new columns as df['A'] = value
pandas.options.mode.chained_assignment = None

COLUMN_NAME_FROM_TO = OrderedDict({'時間': 'time',
                                   'インプレッション': 'impressions',
                                   'リツイート': 'retweets',
                                   '返信': 'replies',
                                   'いいね': 'likes'})

DEFAULT_INFILENAME  = 'data/in2.csv' # input data filename
DEFAULT_OUTFILENAME = 'data/converted2.csv' # output data filename
DEFAULT_HOUR_SPAN = 4
EXIT_STATUS_SUCCESS = 0

# Convert 7:00 JST (the first posting of a day) to PST
POSINTG_FIRST_HOUR = 7
POSINTG_LAST_HOUR = 23
TIMEZONE_HOUR_OFFSET = 15
HOURS_PER_DAY = 24

class TweetImpressions(object):
    '''Filter a tweet CSV file and write it to another'''

    def __init__(self, infilename, outfilename, hour_span):
        # UTC Time
        pattern = '.* ' + self.make_hour_set(hour_span) + ':0[0-3] '
        self.autopost_regex = re.compile(pattern)
        tweets = self.parse(infilename, hour_span)
        tweets.to_csv(outfilename, index_label='index')

    @staticmethod
    def make_hour_set(hour_span):
        '''Make a string to extract hours by a regexp'''
        hour_set = map(lambda x: '{:02d}'.format((x + TIMEZONE_HOUR_OFFSET) % HOURS_PER_DAY),
                       range(POSINTG_FIRST_HOUR, POSINTG_LAST_HOUR + hour_span, hour_span))
        return '({})'.format('|'.join(hour_set))

    def parse(self, infilename, hour_span):
        '''Parses UTF-8 CSV lines'''
        in_tweets = pandas.read_csv(infilename)
        in_tweets.rename(columns=COLUMN_NAME_FROM_TO, inplace=True)
        column_names = list(COLUMN_NAME_FROM_TO.values())

        out_tweets = in_tweets[column_names]
        out_tweets['time_index'] = out_tweets['time'].apply(lambda x: self.get_time_index(x, hour_span))
        out_tweets['autopost'] = out_tweets['time_index'].apply(lambda x: x >= 0)
        return out_tweets

    def get_time_index(self, time_str, hour_span):
        '''Get time index if a tweet is automatically posted otherwise -1'''

        matched = self.autopost_regex.match(time_str)
        time_index = -1
        if matched is not None:
            # Convert JST 07:00, 11:00, 15:00, 19:00, 23:00 to 0..4
            try:
                timestamp = int(matched[1]) + HOURS_PER_DAY * 2 - TIMEZONE_HOUR_OFFSET - POSINTG_FIRST_HOUR
                time_index = (timestamp % HOURS_PER_DAY) // hour_span
            except ValueError:
                # failed to parse as a number
                pass

        return time_index


def main():
    exit_status_code = EXIT_STATUS_SUCCESS
    in_filename = DEFAULT_INFILENAME
    out_filename = DEFAULT_OUTFILENAME

    parser = OptionParser()
    parser.add_option("-s", "--hour_span", dest="span", type="int",
                      help="Hours between tweets")
    (options, args) = parser.parse_args()
    hour_span = DEFAULT_HOUR_SPAN if options.span is None else options.span

    if len(args) > 0:
        in_filename = args[0]
    if len(args) > 1:
        out_filename = args[1]

    TweetImpressions(in_filename, out_filename, hour_span)
    return exit_status_code

if __name__ == "__main__":
    sys.exit(main())
