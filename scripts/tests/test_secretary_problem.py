#!/usr/bin/python3
# coding: utf-8

'''
This script tests secretary_problem.py
Copyright (C) 2017 Zettsu Tatsuya

usage : python3 -m unittest discover tests
'''

import warnings
from unittest import TestCase
import numpy as np
import secretary_problem.secretary_problem as tested


class TestSecretaryProblem(TestCase):
    '''Testing find_candidate'''

    def test_find_first(self):
        '''Find a candidate just after passed candidates'''

        nums = np.array([10, 20, 30, 40, 50])
        n_items = len(nums)
        for pass_count in range(1, n_items):
            passes = [pass_count]
            actual = tested.SecretaryProblem(n_items, passes).find_candidate(n_items, passes, nums)
            self.assertEqual(actual, nums[pass_count])

    def test_find_last(self):
        '''Find a last candidate'''

        nums = np.array([50, 40, 30, 20, 10])
        n_items = len(nums)
        for pass_count in range(1, n_items):
            passes = [pass_count]
            actual = tested.SecretaryProblem(n_items, passes).find_candidate(n_items, passes, nums)
            self.assertEqual(actual, nums[-1])

    def test_find_middle(self):
        '''Find a candidate between passed and last candidates'''

        nums = np.array([30, 20, 10, 50, 40])
        n_items = len(nums)
        for pass_count in range(1, 3):
            passes = [pass_count]
            actual = tested.SecretaryProblem(n_items, passes).find_candidate(n_items, passes, nums)
            self.assertEqual(actual, 50)

    def test_find_two_candidates1(self):
        '''Hold two candidates and the best candidates is placed last'''

        nums = np.array([10, 20, 40, 30, 50, 60, 70])
        n_items = len(nums)
        passes_set = [[[1, 2], 40], [[1, 3], 30], [[1, 4], 50], [[1, 5], 60], [[1, 6], 70],
                      [[2, 3], 50], [[2, 4], 50], [[2, 5], 60], [[2, 6], 70],
                      [[3, 4], 60], [[3, 5], 60], [[3, 6], 70],
                      [[4, 5], 60], [[4, 6], 70],
                      [[5, 6], 70],
                      [[1, 1], 40], [[2, 2], 50], [[3, 3], 60], [[4, 4], 60], [[5, 5], 70],
                      [[6, 6], 70], [[7, 7], 70],
                      [[1, 7], 20], [[2, 7], 40], [[3, 7], 50], [[4, 7], 50], [[5, 7], 60],
                      [[6, 7], 70]]
        for passes, expected in passes_set:
            actual = tested.SecretaryProblem(n_items, passes).find_candidate(n_items, passes, nums)
            self.assertEqual(actual, expected)

    def test_find_two_candidates2(self):
        '''Hold two candidates and the best candidates is placed middle of candidates'''

        nums = np.array([30, 40, 60, 50, 70, 20, 10])
        n_items = len(nums)
        passes_set = [[[1, 2], 60], [[1, 3], 50], [[1, 4], 70], [[1, 5], 40], [[1, 6], 40],
                      [[2, 3], 70], [[2, 4], 70], [[2, 5], 60], [[2, 6], 60],
                      [[3, 4], 70], [[3, 5], 70], [[3, 6], 70],
                      [[4, 5], 70], [[4, 6], 70],
                      [[5, 6], 10],
                      [[1, 1], 60], [[2, 2], 70], [[3, 3], 70], [[4, 4], 70], [[5, 5], 10],
                      [[6, 6], 10], [[7, 7], 10],
                      [[1, 7], 40], [[2, 7], 60], [[3, 7], 70], [[4, 7], 70], [[5, 7], 10],
                      [[6, 7], 10]]
        for passes, expected in passes_set:
            actual = tested.SecretaryProblem(n_items, passes).find_candidate(n_items, passes, nums)
            self.assertEqual(actual, expected)

    def test_find_many_candidates(self):
        '''Hold many candidates'''

        nums = np.array([10, 20, 30, 40, 70, 60, 50, 49, 48, 47])
        n_items = len(nums)
        passes_set = [[[1, 2, 3], 40], [[1, 2, 3, 4], 70], [[1, 2, 3, 4, 5], 70],
                      [[4, 5, 6, 7], 70], [[5, 6, 7, 8], 47]]
        for passes, expected in passes_set:
            actual = tested.SecretaryProblem(n_items, passes).find_candidate(n_items, passes, nums)
            self.assertEqual(actual, expected)


class TestExploreSecretaryProblem(TestCase):
    '''Testing optimization with ExploreSecretaryProblem'''

    def test_explore(self):
        '''Explore a solution of 1-secretary problem'''
        # Surpress warnings for scikit-optimize
        warnings.filterwarnings("ignore", category=DeprecationWarning)
        warnings.filterwarnings("ignore", category=UserWarning)
        cutoffs, value = tested.ExploreSecretaryProblem(
            n_items=100, n_cutoffs=1, n_trial=10000, n_calls=30,
            n_random_starts=20).explore(verbose=False)

        self.assertGreater(cutoffs[0], 35.0)
        self.assertLess(cutoffs[0], 39.0)
        self.assertLess(value, -0.35)
        self.assertGreater(value, -0.39)
