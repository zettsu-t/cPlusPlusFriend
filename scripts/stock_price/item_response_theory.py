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
from optparse import OptionParser
import errno
import os
import time
import sys
import matplotlib.pyplot as plt
import numpy as np
import pandas
import seaborn
import tensorflow as tf
import tensorflow_probability as tfp
from tensorflow_probability import edward2 as ed

tfd = tfp.distributions

## Size of MCMC iterations
DEFAULT_N_DRAWS = 5000
DEFAULT_N_BURNIN = 10000
## Choose an appropriate step size or states are converged irregularly
DEFAULT_HMC_STEP_SIZE = 0.001
DEFAULT_HMC_LEAPFROG_STEPS = 10

## Number of respondents and questions
DEFAULT_N_RESPONDENTS = 50 #500
DEFAULT_N_QUESTIONS = 200

## Chart filenames
ANSWER_ABILITY_BASENAME = 'answer_ability'
ACTUAL_ABILITY_BASENAME = 'actual_ability'
ESTIMATED_ABILITY_BOX1_BASENAME = 'estimated_ability_box_a'
ESTIMATED_ABILITY_BOX2_BASENAME = 'estimated_ability_box_b'
ESTIMATED_ABILITY_SCATTER_BASENAME = 'estimated_ability_scatter'

## Assuming four-choice questions
CHANCE = 0.25
SLIM_CHANCE_EPSILON = 1e-7
## Distribution of respondent abilities
MU_THETA = 0.5
SIGMA_THETA = 0.17
## Distribution of questions difficulties
DIFFICULTY_MIN = 0.1
DIFFICULTY_MAX = 0.9
MU_DISCRIMINATION = 16.0
SIGMA_DISCRIMINATION = 3.0

ParamSet = namedtuple(
    'ParamSet', (
        'outputdir', 'n_respondents', 'use_sum', 'n_questions',
        'n_draws', 'n_burnin', 'hmc_stepsize', 'hmc_leapfrog_steps'))

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
                             clip_value_min=0.001, clip_value_max=0.999, name='actual_abilities')
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

           plt.figure(figsize=(6, 6))
           plt.title('Abilities and answers')
           plt.xlabel('Rank of abilities')
           plt.ylabel('Number of correct answers')
           n_correct_answers = np.sum(answers.eval(), 1)
           plt.scatter(list(range(n_correct_answers.shape[0])), n_correct_answers, color='mediumblue', alpha=0.7)
           plt.savefig(self.get_png_filename(ANSWER_ABILITY_BASENAME), dpi=160)

    def get_logit_odds(self, abilities):
        ## Same as the input data
        locs = tf.transpose(tf.broadcast_to(input=abilities, shape=[self.n_questions, self.n_respondents]))
        diffs = tf.broadcast_to(input=self.difficulties, shape=[self.n_respondents, self.n_questions])
        locs = locs - diffs
        scales = tf.broadcast_to(input=self.discriminations, shape=[self.n_respondents, self.n_questions])
        logits = locs * scales
        return logits

    def get_sample(self):
        '''Returns an observation as an MCMC sample'''
        ## Same as the actual distribution
        abilities = ed.Normal(loc=MU_THETA, scale=SIGMA_THETA, sample_shape=self.n_respondents, name='abilities')
        logits = self.get_logit_odds(abilities)

        ## The support of Bernoulli distributions takes 0 or 1 but
        ## this function must return float not int to differentiate
        if CHANCE < SLIM_CHANCE_EPSILON:
            answers = ed.Bernoulli(logits=logits, name='answers', dtype=tf.float32)
        else:
            ## tfd.Bernoulli(probs=probs) does not work
            ## You can use broadcasting instead of tf.ops and tf.constant
            probs = tf.add(tf.constant(CHANCE, shape=[self.n_respondents, self.n_questions]),
                           tf.multiply(tf.constant(1.0 - CHANCE, shape=[self.n_respondents, self.n_questions]), tf.nn.sigmoid(logits)))
            odds = tf.divide(probs, tf.subtract(tf.constant(1.0, shape=[self.n_respondents, self.n_questions], dtype=tf.float32), probs))
            answers = ed.Bernoulli(logits=tf.log(odds), name='answers', dtype=tf.float32)
        return answers

    def target_log_prob_fn_joint(self, abilities):
        '''Applies observations partially to calculate their log likelihood'''
        log_joint = ed.make_log_joint_fn(self.get_sample)
        return log_joint(abilities=abilities, answers=self.y_answers)

    def target_log_prob_fn_reduce_sum(self, abilities):
        '''Applies observations partially to calculate their log likelihood'''
        logits = self.get_logit_odds(abilities)
        log_prob_parts = None

        if CHANCE < SLIM_CHANCE_EPSILON:
            log_prob_parts = tfd.Bernoulli(logits=logits).log_prob(self.y_answers)
        else:
            probs = CHANCE + (1.0 - CHANCE) * tf.nn.sigmoid(logits)
            ## tfd.Bernoulli(probs=probs) does not work
            log_prob_parts = tfd.Bernoulli(logits=tf.log(probs/(1.0-probs))).log_prob(self.y_answers)

        sum_log_prob = tf.reduce_sum(log_prob_parts)
        return sum_log_prob

    def estimate(self):
        '''Estimates abilities by an HMC sampler'''
        target_log_prob_fn = self.target_log_prob_fn_reduce_sum if self.param_set.use_sum else self.target_log_prob_fn_joint

        hmc_kernel = tfp.mcmc.HamiltonianMonteCarlo(
            target_log_prob_fn=target_log_prob_fn,
            step_size=self.param_set.hmc_stepsize,
            num_leapfrog_steps=self.param_set.hmc_leapfrog_steps)

        states, kernels_results = tfp.mcmc.sample_chain(
            num_results=self.param_set.n_draws,
            current_state=[self.x_abilities],
            kernel=hmc_kernel,
            num_burnin_steps=self.param_set.n_burnin)

        with tf.Session() as sess:
            sess.run(tf.global_variables_initializer())
            states_, results_ = sess.run([states, kernels_results])
            self.plot_estimated(states_[0])

    def plot_estimated(self, ability_samples):
        '''Plots results'''
        medians = np.median(ability_samples, 0)
        abilities = self.abilities.eval()

        ## Plot estimated abilities ordered by actual abilities
        plt.figure(figsize=(6, 6))
        seaborn.boxplot(data=pandas.DataFrame(ability_samples), color='magenta', width=0.8)
        plt.title('Estimated abilities with ranges')
        plt.xlabel('Actual rank of abilities')
        plt.ylabel('Estimated ability')
        plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
        plt.savefig(self.get_png_filename(ESTIMATED_ABILITY_BOX1_BASENAME), dpi=160)

        ## Plot confidential intervals for abilities of the respondents ordered by actual abilities
        plt.figure(figsize=(6, 6))
        data=pandas.DataFrame(ability_samples)
        data.boxplot(positions=abilities, widths=np.full(abilities.shape, 0.02))
        plt.xlim(0.0, 1.0)
        plt.plot([0.0, 1.0], [0.0, 1.0], color='magenta', lw=2, linestyle='--')
        plt.title('Estimated and actual abilities with ranges')
        plt.xlabel('Actual ability')
        plt.ylabel('Estimated ability')
        plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
        plt.savefig(self.get_png_filename(ESTIMATED_ABILITY_BOX2_BASENAME), dpi=160)

        ## Plot actual and estimated abilities to check whether they are on a diagonal line
        plt.figure(figsize=(6, 6))
        plt.title('Abilities')
        plt.xlabel('Actual')
        plt.ylabel('Estimated')
        plt.scatter(abilities, medians, color='darkmagenta', alpha=0.7)
        plt.tight_layout()
        plt.savefig(self.get_png_filename(ESTIMATED_ABILITY_SCATTER_BASENAME), dpi=160)

class QuestionAndAbilityLauncher(object):
    '''Launch QuestionAndAbility'''
    def __init__(self, args):
        parser = OptionParser()
        parser.add_option('-n', '--trials', type='int', dest='n_trials',
                          help='Number of trials', default=1)

        parser.add_option('-o', '--outputdir', type='str', dest='outputdir',
                          help='Directory to write output files', default='images')

        parser.add_option('-p', '--sum', action="store_true", dest='use_sum',
                          help='Use sum to return probabilities instead of joint', default=False)

        parser.add_option('-r', '--respondents', type='int', dest='n_respondents',
                          help='Number of respondents', default=DEFAULT_N_RESPONDENTS)

        parser.add_option('-q', '--questions', type='int', dest='n_questions',
                          help='Number of questions', default=DEFAULT_N_QUESTIONS)

        parser.add_option('-d', '--draws', type='int', dest='n_draws',
                          help='Number of Markov chain draws', default=DEFAULT_N_DRAWS)

        parser.add_option('-b', '--burnin', type='int', dest='n_burnin',
                          help='Number of burnin chain steps', default=DEFAULT_N_BURNIN)

        parser.add_option('-s', '--stepsize', type='float', dest='hmc_stepsize',
                          help='HMC stepsize', default=DEFAULT_HMC_STEP_SIZE)

        parser.add_option('-l', '--leapfrog', type='int', dest='hmc_leapfrog_steps',
                          help=' Number of HMC leapfrog steps', default=DEFAULT_HMC_LEAPFROG_STEPS)

        (options, args) = parser.parse_args()

        if not os.path.isdir(options.outputdir):
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), options.outputdir)

        param_set = ParamSet(outputdir=options.outputdir,
                             n_respondents=options.n_respondents,
                             use_sum=options.use_sum,
                             n_questions=options.n_questions,
                             n_draws=options.n_draws,
                             n_burnin=options.n_burnin,
                             hmc_stepsize=options.hmc_stepsize,
                             hmc_leapfrog_steps=options.hmc_leapfrog_steps)

        n_trial = max(1, options.n_trials)
        format_trial = datetime.datetime.now().strftime("%Y%m%d%H%M%S") + '{0:0' + str(len(str(n_trial))) + 'd}'
        for trial in range(n_trial):
            start_time = time.time()
            QuestionAndAbility(param_set, format_trial.format(trial + 1)).estimate()
            elapsed_time = time.time() - start_time
            print(elapsed_time)

if __name__ == '__main__':
    QuestionAndAbilityLauncher(sys.argv)
