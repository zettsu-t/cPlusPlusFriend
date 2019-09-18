#!/usr/bin/python3
# coding: utf-8

'''
Implementation of the article below with TensorFlow Probability
http://norimune.net/2949

Based on an example of TensorFlow Probability
https://github.com/tensorflow/probability/tree/master/tensorflow_probability/python/edward2
https://www.hellocybernetics.tech/entry/2018/11/09/231817

Usage:
$ mkdir -p images
$ python3 item_response_theory.py [options]
It writes PNG files on ./images/ on the default setting.
'''

from collections import namedtuple
import datetime
import errno
import math
from optparse import OptionParser
import os
import time
import sys
import GPy
import GPyOpt
import matplotlib.pyplot as plt
import numpy as np
import pandas
import pickle
import seaborn
from scipy.stats import rankdata
from sklearn.metrics import mean_squared_error
import tensorflow as tf
import tensorflow_probability as tfp
from tensorflow_probability import edward2 as ed

tfd = tfp.distributions

## Number of respondents and questions
DEFAULT_N_RESPONDENTS = 500
DEFAULT_N_QUESTIONS = 200
DEFAULT_N_ITERATIONS = 250
DEFAULT_LEARNING_RATE = 0.01

## Chart filenames
ANSWER_ABILITY_BASENAME = 'answer_ability'
ACTUAL_ABILITY_BASENAME = 'actual_ability'
ESTIMATED_ABILITY_SCATTER_BASENAME = 'estimated_ability_scatter'
ESTIMATED_ELBO_BASENAME = 'estimated_elbo'

## Assuming four-choice questions
CHANCE = 0.0
SLIM_CHANCE_EPSILON = 1e-7
## Distribution of respondent abilities
MIN_ABILITIY = 0.1
MAX_ABILITIY = 0.999
MU_THETA = 0.5
SIGMA_THETA = 0.17
## Distribution of questions difficulties
DIFFICULTY_MIN = 0.1
DIFFICULTY_MAX = 0.9
MU_DISCRIMINATION = 16.0
SIGMA_DISCRIMINATION = 3.0

## Checks if HCM sampling is converged in correlation coefficient of
## ranks of actual and estimated abilities
MIN_CORRELATION = 0.95
## Exclude outliers to calculate correlation coefficients
MIN_ABILITIY_IN_CORRELATION = 0.001
MAX_ABILITIY_IN_CORRELATION = 1 - MIN_ABILITIY_IN_CORRELATION

ParamSet = namedtuple(
    'ParamSet', (
        'outputdir', 'n_respondents', 'n_questions', 'n_iterations', 'n_learning_rate'))

FittingResult = namedtuple('FittingResult', ('correlation', 'converged', 'distance'))

class QuestionAndAbility(object):
    '''Estimate abilities under the item response theory'''
    def __init__(self, param_set, filename_suffix):
        self.param_set = param_set
        self.filename_suffix = '' if filename_suffix is None else filename_suffix
        self.n_respondents = param_set.n_respondents
        self.n_questions = param_set.n_questions

        ## Explanatory variables
        self.abilities = tf.clip_by_value(tf.contrib.framework.sort(
            tf.random.normal(shape=[self.n_respondents], mean=MU_THETA, stddev=SIGMA_THETA)),
                             clip_value_min=MIN_ABILITIY, clip_value_max=MAX_ABILITIY,
                             name='actual_abilities')
        delta = (DIFFICULTY_MAX - DIFFICULTY_MIN) / (self.n_questions - 1)

        difficulties = np.append(np.arange(DIFFICULTY_MIN, DIFFICULTY_MAX, delta), DIFFICULTY_MAX)
        difficulties = difficulties[(len(difficulties)-self.n_questions):]
        self.difficulties = tf.constant(difficulties, dtype=tf.float32)
        self.discriminations = tf.constant(np.random.normal(
            loc=MU_DISCRIMINATION, scale=SIGMA_DISCRIMINATION, size=[self.n_questions]), dtype=tf.float32)

        ## Makes normalized sigmoid parameters with broadcasting
        locs = tf.transpose(tf.broadcast_to(input=self.abilities, shape=[self.n_questions, self.n_respondents]))
        diffs = tf.broadcast_to(input=self.difficulties, shape=[self.n_respondents, self.n_questions])
        ## Inflection points of sigmoid functions
        locs = locs - diffs
        scales = tf.broadcast_to(input=self.discriminations, shape=[self.n_respondents, self.n_questions])
        probabilities = tf.add(tf.constant(CHANCE, shape=[self.n_respondents, self.n_questions]),
                               tf.multiply(tf.constant(1.0 - CHANCE, shape=[self.n_respondents, self.n_questions]),
                                           tf.nn.sigmoid(locs * scales)))

        ## Observed data
        ## https://stackoverflow.com/questions/35487598/sampling-bernoulli-random-variables-in-tensorflow
        ## Must be float, not int (see get_sample())
        self.y_answers = tf.nn.relu(tf.sign(probabilities - tf.random_uniform(tf.shape(probabilities))))
        ## Explanatory variable(s)
        self.x_abilities = ed.Normal(sample_shape=[self.n_respondents], loc=MU_THETA, scale=SIGMA_THETA, name='abilities')

        self.plot_actual(self.y_answers, self.abilities)

    def get_png_filename(self, prefix):
        '''Makes a PNG filename for a trial'''
        return os.path.join(self.param_set.outputdir, prefix + '_' + self.filename_suffix + '.png')

    def plot_actual(self, answers, abilities):
       '''Plots the synthetic input data and we can check they are correct'''
       with tf.Session() as sess:
           sess.run(answers)
           plt.figure(figsize=(6, 6))
           plt.hist(abilities.eval(), bins=25, color='royalblue')
           plt.savefig(self.get_png_filename(ACTUAL_ABILITY_BASENAME), dpi=160)
           plt.close()

           n_correct_answers = np.sum(answers.eval(), 1)
           plt.figure(figsize=(6, 6))
           plt.title('Abilities and answers')
           plt.xlabel('Rank of abilities')
           plt.ylabel('Number of correct answers')
           plt.scatter(list(range(n_correct_answers.shape[0])), n_correct_answers, color='mediumblue', alpha=0.7)
           plt.savefig(self.get_png_filename(ANSWER_ABILITY_BASENAME), dpi=160)
           plt.close()

    def get_logit_odds(self, abilities):
        ## Same as the input data
        locs_base = tf.transpose(tf.broadcast_to(input=abilities, shape=[self.n_questions, self.n_respondents]))
        diffs = tf.broadcast_to(input=self.difficulties, shape=[self.n_respondents, self.n_questions])
        locs = tf.subtract(locs_base, diffs)
        scales = tf.broadcast_to(input=self.discriminations, shape=[self.n_respondents, self.n_questions])
        logits = tf.multiply(locs, scales)
        return logits

    def get_sample(self, abilities):
        '''Returns an observation as an MCMC sample'''
        ## Same as the actual distribution
        ## abilities = ed.Normal(loc=MU_THETA, scale=SIGMA_THETA, sample_shape=self.n_respondents, name='abilities')
        logits = self.get_logit_odds(abilities)

        ## The support of Bernoulli distributions takes 0 or 1 but
        ## this function must return float not int to differentiate
        if CHANCE < SLIM_CHANCE_EPSILON:
            answers = ed.Bernoulli(logits=logits, name='answers', dtype=tf.float32)
        else:
            ## tfd.Bernoulli(probs=probs) does not work
            ## You can use broadcasting instead of tf.ops and tf.constant
            probs = tf.add(tf.constant(CHANCE, shape=[self.n_respondents, self.n_questions]),
                           tf.multiply(tf.constant(1.0 - CHANCE, shape=[self.n_respondents, self.n_questions]),
                                       tf.nn.sigmoid(logits)))
            odds = tf.divide(probs, tf.subtract(tf.constant(1.0, shape=[self.n_respondents, self.n_questions],
                                                            dtype=tf.float32), probs))
            answers = ed.Bernoulli(probs=probs, name='answers', dtype=tf.float32)
        return answers, abilities

    def target_log_prob_fn_joint(self, abilities):
        '''Applies observations partially to calculate their log likelihood'''
        log_joint = ed.make_log_joint_fn(self.get_sample)
        return log_joint(abilities=abilities, answers=self.y_answers)

    ## Based on
    ## https://github.com/tensorflow/probability/blob/master/tensorflow_probability/examples/jupyter_notebooks/Probabilistic_PCA.ipynb
    def variational_model(self, qw_mean):
        return ed.Normal(loc=qw_mean, scale=SIGMA_THETA, name="qw")

    def estimate(self):
        '''Estimates the MAP of abilities'''
        self.x_abilities = tf.Variable(np.ones([self.n_respondents]), dtype=tf.float32)
        qw = self.variational_model(qw_mean=self.x_abilities)
        log_q = ed.make_log_joint_fn(self.variational_model)

        def target_q(qw):
            return log_q(qw_mean=self.x_abilities, qw=qw)

        energy = self.target_log_prob_fn_joint(abilities=qw)
        entropy = -target_q(qw)
        elbo = energy + entropy

        optimizer = tf.train.AdamOptimizer(learning_rate=self.param_set.n_learning_rate)
        train = optimizer.minimize(-elbo)
        init = tf.global_variables_initializer()
        t = []

        num_epochs = self.param_set.n_iterations
        with tf.Session() as sess:
            sess.run(init)
            for i in range(num_epochs):
                sess.run(train)
                if i % 5 == 0:
                    cE = sess.run([elbo])
                    t.append(cE)

            ability_estimated = sess.run(self.x_abilities)
            result = self.plot_estimated(ability_estimated)

        x = range(1, num_epochs, 5)
        plt.figure(figsize=(6, 6))
        plt.title('Iterations')
        plt.xlabel('Epoch')
        plt.ylabel('ELBO')
        plt.plot(x, t)
        plt.subplots_adjust(left=0.18)
        plt.savefig(self.get_png_filename(ESTIMATED_ELBO_BASENAME), dpi=160)
        plt.close()

        print(result)
        return result

    def plot_estimated(self, ability_samples):
        '''Plots results'''
        abilities_actual = self.abilities.eval().reshape(-1)
        plt.figure(figsize=(6, 6))
        plt.title('Abilities')
        plt.xlabel('Actual')
        plt.ylabel('Estimated')
        plt.scatter(abilities_actual, ability_samples, color='darkmagenta', alpha=0.7)
        plt.tight_layout()
        plt.savefig(self.get_png_filename(ESTIMATED_ABILITY_SCATTER_BASENAME), dpi=160)
        plt.close()
        return np.corrcoef(ability_samples, abilities_actual)

class QuestionAndAbilityLauncher(object):
    '''Launch QuestionAndAbility'''
    def __init__(self, args):
        (options, args) = self.parse_options(args)
        if not os.path.isdir(options.outputdir):
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), options.outputdir)

        format_trial = datetime.datetime.now().strftime("%Y%m%d%H%M%S")
        format_trial += '_{0:0' + str(len(str(options.n_iterations))) + 'd}'
        self.iterate(options, format_trial)

    def parse_options(self, args):
        parser = OptionParser()
        parser.add_option('-n', '--iterations', type='int', dest='n_iterations',
                          help='Number of trials', default=DEFAULT_N_ITERATIONS)

        parser.add_option('-l', '--learningrate', type='float', dest='n_learning_rate',
                          help='Number of trials', default=DEFAULT_LEARNING_RATE)

        parser.add_option('-o', '--outputdir', type='str', dest='outputdir',
                          help='Directory to write output files', default='images')

        parser.add_option('-r', '--respondents', type='int', dest='n_respondents',
                          help='Number of respondents', default=DEFAULT_N_RESPONDENTS)

        parser.add_option('-q', '--questions', type='int', dest='n_questions',
                          help='Number of questions', default=DEFAULT_N_QUESTIONS)
        return parser.parse_args(args)

    def iterate(self, options, format_trial):
        param_set = ParamSet(outputdir=options.outputdir,
                             n_respondents=options.n_respondents,
                             n_questions=options.n_questions,
                             n_iterations=options.n_iterations,
                             n_learning_rate=options.n_learning_rate)

        trial = 1
        start_time = time.time()
        result = QuestionAndAbility(param_set, format_trial.format(trial)).estimate()
        elapsed_time = time.time() - start_time
        print('{} sec elapsed'.format(elapsed_time))

if __name__ == '__main__':
    QuestionAndAbilityLauncher(sys.argv)
