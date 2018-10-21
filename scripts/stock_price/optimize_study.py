#!/usr/bin/python3
# coding: utf-8

'''
Optimize a total score of final exems
Based on
https://stackoverflow.com/questions/21765794/python-constrained-non-linear-optimization?rq=1
'''

import numbers
from collections import namedtuple
import numpy as np
import matplotlib.pyplot as plt
from scipy.special import expit, logit
from mystic.symbolic import generate_constraint, generate_solvers, simplify
from mystic.symbolic import generate_penalty, generate_conditions
from mystic.solvers import diffev2
from mystic.math import almostEqual
from mystic.monitors import VerboseMonitor

Subject = namedtuple('Subject', ['name', 'difficulty', 'slope', 'low_score', 'high_score', 'min_score', 'color'])

class ScoreSet(object):
    def __init__(self):
        self.subjects = [Subject(name = 'Japanese',    difficulty = 0.15, slope = 13.0, low_score = 45.0, high_score = 70.0, min_score = 50.0, color = 'darkgoldenrod'),
                         Subject(name = 'English',     difficulty = 0.50, slope = 18.0, low_score = 30.0, high_score = 67.0, min_score = 50.0, color = 'firebrick'),
                         Subject(name = 'Geography',   difficulty = 0.20, slope = 12.0, low_score = 20.0, high_score = 80.0, min_score = 50.0, color = 'dimgray'),
                         Subject(name = 'Mathematics', difficulty = 0.65, slope = 25.0, low_score = 20.0, high_score = 85.0, min_score = 50.0, color = 'navy'),
                         Subject(name = 'Physics',     difficulty = 0.73, slope = 33.0, low_score =  0.0, high_score = 99.0, min_score = 50.0, color = 'royalblue'),
                         Subject(name = 'Chemistry',   difficulty = 0.43, slope = 22.0, low_score = 10.0, high_score = 95.0, min_score = 50.0, color = 'lightskyblue')]

        self.equations_pre = ' + '.join(['x{}'.format(i) for i in range(len(self.subjects))])
        self.equations_post = ''
        self.min_time = 0.0
        self.min_score = 0.0
        for index, subject in enumerate(self.subjects):
            min_x = max(0.0, self.get_time(subject.min_score, subject))
            self.min_time += min_x
            self.min_score += subject.min_score
            self.equations_post += 'x{} >= {}\n'.format(index, min_x)

    @staticmethod
    def get_score(abilitiy, subject):
        width = subject.slope * (abilitiy - subject.difficulty)
        return subject.low_score + expit(width) * (subject.high_score - subject.low_score)

    @staticmethod
    def get_time(score, subject):
        height = (score - subject.low_score) / (subject.high_score - subject.low_score)
        width = logit(height) / subject.slope
        return subject.difficulty + width

    def objective(self, abilities):
        total = 0.0
        for index, abilitiy in enumerate(abilities):
            score = self.get_score(abilitiy, self.subjects[index])
            total += score
        return -1.0 * total

    def solve(self, arg_n_time_range, time_step):
        if isinstance(time_step, numbers.Number):
            n_time_range = arg_n_time_range
            time_set = [self.min_time + (i + 1) * time_step for i in range(n_time_range)]
        elif isinstance(time_step, list):
            n_time_range = len(time_step)
            time_set = self.min_time + np.array(time_step)
        else:
            raise NotImplementedError

        n_chart = len(time_set)
        half_n_chart = (n_chart + 1) // 2
        fig = plt.figure(figsize=(16, half_n_chart * 4))

        for index, limit_time in enumerate(time_set):
            equations = self.equations_pre + ' <= {}\n'.format(limit_time) + self.equations_post
            result = self.find_mininum(equations)
            score = -1.0 * result[1]
            title = 'Mininum score{:0.1f}@time={:0.2f}, Total score={:0.1f}@time={:0.2f}'.format(self.min_score, self.min_time, score, limit_time)
            ax = fig.add_subplot(half_n_chart, 2, index + 1)
            self.plot(ax, title, result, index + 1 == n_chart)
        plt.tight_layout(pad=0.4, h_pad=3.0)
        plt.savefig('optimize_time', dpi=80)

    def find_mininum(self, equations):
        mon = VerboseMonitor(10)
        bounds = [(0.0, 1.0)] * len(self.subjects)
        cf = generate_constraint(generate_solvers(simplify(equations)))
        pf = generate_penalty(generate_conditions(equations), k=1e12)
        result = diffev2(self.objective, x0=bounds, bounds=bounds, constraints=cf, penalty=pf,
                         npop=40, gtol=50, disp=False, full_output=True, itermon=mon)
        return result

    def plot(self, ax, title, result, last_chart):
        xticks = np.arange(0.0, 1.0, 0.001)
        for index, subject in enumerate(self.subjects):
            scores = self.get_score(xticks, subject)
            ax.plot(xticks, scores, label=subject.name, color=subject.color)
            min_x = max(0.0, self.get_time(subject.min_score, subject))
            ax.axvline(x=min_x, color=subject.color)
            ax.axhline(y=subject.min_score, color='red')
            if result is not None:
                opt_x = (result[0])[index]
                ax.axvline(x=opt_x, color=subject.color, linestyle='dashed')
        ax.set_title(title)
        ax.set_ylabel('score')
        if last_chart:
            ax.set_xlabel('time')
        ax.legend()

s = ScoreSet()
s.solve(8, [0.1, 0.2, 0.3, 0.5, 0.7, 1.0, 1.5, 2.0])
