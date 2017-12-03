#!/usr/bin/python3
# coding: utf-8

'''
Search a tweet analysis CSV file for an optimal range of tweet
impressions which fits most likely to a log normal distribution well.
'''

import csv
import codecs
import math
import re
import sys
import warnings
import numpy as np
from scipy import stats
from skopt import gp_minimize

# Surpress warnings for scikit-optimize
warnings.filterwarnings("ignore", category=DeprecationWarning)
warnings.filterwarnings("ignore", category=UserWarning)

DEFAULT_INFILENAME = 'data/in.csv' # input data filename
EXIT_STATUS_SUCCESS = 0

class TweetImpressions(object):
    '''Search for a impression range'''

    def __init__(self, infilename):
        # log scale
        self.impressions = self.parse(infilename)

    def parse(self, infilename):
        '''Parses UTF-8 CSV lines'''

        with codecs.open(infilename, 'r', 'utf-8') as infile:
            reader = csv.reader(infile, quotechar='"', delimiter=',',
                                quoting=csv.QUOTE_ALL)
            # Skip titles
            next(reader)
            impressions = []
            for row in reader:
                impression = self.extract_impression(row)
                if impression is not None:
                    impressions.append(math.log(impression))
            return np.array(impressions)

    @staticmethod
    def extract_impression(row):
        '''Extract the impression in a row'''

        result = None
        # Include auto-posted tweets and exclude manually posted tweets
        if re.match(r'.* \d[02468]:\d[0-5] ', row[3]) is not None:
            try:
                num_str = row[4].replace('"', '')
                result = float(num_str)
            except:
                # failed to parse as a number
                pass

        return result

    def explore(self):
        '''Explore the optimal range'''
        def evaluate(param, tester):
            '''Evaluate likelihood to a normal distribution by the tester'''

            mean, width = param
            impressions_sub = self.impressions[self.impressions > mean - width]
            impressions = impressions_sub[impressions_sub < mean + width]
            # Avoid errors in the optimizer
            if np.size(impressions) < 5:
                return 0.0

            w_value, _ = tester(impressions)
            # Minimize this return value
            return -1.0 * w_value

        def eval_shapiro(param):
            '''Shapiro–Wilk test'''
            def tester(data):
                return stats.shapiro(data)
            return evaluate(param, tester)

#       def eval_ks(param):
#           '''Kolmogorov-Smirnov test'''
#           def tester(data):
#               return stats.kstest(data, 'norm')
#           return evaluate(param, tester)

        full_median = np.median(self.impressions)
        full_mean = np.mean(self.impressions)
        min_mu = min(full_median, full_mean)
        max_mu = min(full_median, full_mean)
        min_mu *= 0.8
        max_mu *= 1.2
        mu_range = np.array([min_mu, max_mu])

        impressions_sub = self.impressions[self.impressions > min_mu]
        impressions = impressions_sub[impressions_sub < max_mu]
        partial_sd = np.std(impressions)
        width_range = np.array([0.5, 2.0]) * partial_sd
        dimensions = [mu_range, width_range]

        estimation = gp_minimize(func=eval_shapiro, dimensions=dimensions,
                                 n_calls=100, n_random_starts=10, verbose=True)
        self.print_estimation(estimation)
        return EXIT_STATUS_SUCCESS

    @staticmethod
    def print_estimation(estimation):
        '''Print estimated mean and stddev'''

        estimated_mu = estimation.x[0]
        estimated_sd = estimation.x[1]
        estimated_min = math.exp(estimated_mu - estimated_sd)
        estimated_max = math.exp(estimated_mu + estimated_sd)
        print(estimated_min, estimated_max)

if __name__ == "__main__":
    in_filename = DEFAULT_INFILENAME
    if len(sys.argv) > 1:
        in_filename = sys.argv[1]
    exit_status = TweetImpressions(in_filename).explore()
    sys.exit(exit_status)
