#!/usr/bin/python3
# coding: utf-8

'''
Solving the k-secretary problem with Bayesian Optimization.
This Question is cited from the book
Dennis E. Shasha, "Puzzles for Programmers and Pros", 2007, Wrox
'''

import warnings
from math import ceil
import numpy as np
from skopt import gp_minimize

# Surpress warnings for scikit-optimize
warnings.filterwarnings("ignore", category=DeprecationWarning)
warnings.filterwarnings("ignore", category=UserWarning)

class SecretaryProblem(object):
    '''Simulates the k-secretary problem'''

    def __init__(self, n_items, passes):
        '''
        "passes" defines ranges such that
        [0..passes[0]) which contains the number of passes[0] integers
          ...
        [passes[n-2]..passes[n-1])

        Each element in "passes" are included as left-most (least) members
        and not in right-most (largest) members in each range.
        '''

        self.n_items = n_items
        self.passes = passes

    def solve_once(self):
        '''Solves the problem once'''

        nums = np.arange(1, self.n_items + 1)
        np.random.shuffle(nums)
        return self.find_candidate(self.n_items, self.passes, nums)

    def solve_many(self, n_trial):
        '''Solves the problem n_trial times'''

        count = 0
        for _ in range(n_trial):
            count += 1 if self.solve_once() == self.n_items else 0
        return (count + 0.0) / (n_trial + 0.0)

    @staticmethod
    def find_candidate(n_items, passes, nums):
        '''Solves the problem for a sequence of candidates'''
        pivot = np.max(nums[0:passes[0]])
        index_left = passes[0]
        passes_len = len(passes)
        n_candicates = 0

        for pass_index in range(1, passes_len + 1):
            index_right = n_items if (pass_index >= passes_len) else passes[pass_index]
            index_next_left = index_right
            index = 0
            while (n_candicates < pass_index) and (index_left < index_right):
                sliced_nums = nums[index_left:index_right]
                # Returns 0 if no such index is found
                index = np.argmax(sliced_nums > pivot)
                if (index == 0) and (sliced_nums[0] <= pivot):
                    break
                pivot = sliced_nums[index]
                n_candicates += 1
                index_left += index + 1
            index_left = index_next_left

        return nums[-1] if (n_candicates == 0) else pivot

class ExploreSecretaryProblem(object):
    '''Explores a solution of the k-secretary problem with Bayesian Optimization'''

    def __init__(self, n_items, n_cutoffs, n_trial, n_calls, n_random_starts):
        self.n_items = n_items
        self.n_cutoffs = n_cutoffs
        self.n_trial = n_trial
        self.n_calls = n_calls
        self.n_random_starts = n_random_starts

    def explore(self, verbose=True):
        '''Explores a solution of the problem'''

        def solve(cutoffs):
            '''Solves the problem with floating cutoffs'''

            cutoffs.sort()
            passes = list(map(ceil, cutoffs))
            if verbose:
                print(passes)
            return -1.0 * SecretaryProblem(n_items=100, passes=passes).\
                solve_many(n_trial=self.n_trial)

        dimensions = [(1, self.n_items, "prior")] * self.n_cutoffs
        result = gp_minimize(func=solve, dimensions=dimensions, n_calls=self.n_calls,
                             n_random_starts=self.n_random_starts, verbose=verbose)
        if verbose:
            print(result)
        return result.x, result.fun

if __name__ == "__main__":
    ExploreSecretaryProblem(n_items=100, n_cutoffs=1, n_trial=10000, n_calls=50,
                            n_random_starts=10).explore()
    ExploreSecretaryProblem(n_items=100, n_cutoffs=2, n_trial=10000, n_calls=100,
                            n_random_starts=20).explore()
    ExploreSecretaryProblem(n_items=100, n_cutoffs=3, n_trial=10000, n_calls=300,
                            n_random_starts=150).explore()
