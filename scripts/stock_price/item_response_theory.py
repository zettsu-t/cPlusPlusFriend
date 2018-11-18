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

## Size of MCMC iterations
DEFAULT_N_DRAWS = 5000
DEFAULT_N_BURNIN = 10000
## Choose an appropriate step size or states are converged irregularly
DEFAULT_HMC_STEP_SIZE = 0.004
DEFAULT_HMC_LEAPFROG_STEPS = 8

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
MIN_ABILITIY = 0.001
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
        'outputdir', 'n_respondents', 'use_sum', 'n_questions',
        'n_draws', 'n_burnin', 'hmc_stepsize', 'hmc_leapfrog_steps'))

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
                           tf.multiply(tf.constant(1.0 - CHANCE, shape=[self.n_respondents, self.n_questions]),
                                       tf.nn.sigmoid(logits)))
            odds = tf.divide(probs, tf.subtract(tf.constant(1.0, shape=[self.n_respondents, self.n_questions],
                                                            dtype=tf.float32), probs))
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
        target_log_prob_fn = self.target_log_prob_fn_joint
        if self.param_set.use_sum:
            target_log_prob_fn = self.target_log_prob_fn_reduce_sum

        hmc_kernel = tfp.mcmc.HamiltonianMonteCarlo(
            target_log_prob_fn=target_log_prob_fn,
            step_size=self.param_set.hmc_stepsize,
            num_leapfrog_steps=self.param_set.hmc_leapfrog_steps)

        states, kernels_results = tfp.mcmc.sample_chain(
            num_results=self.param_set.n_draws,
            current_state=[self.x_abilities],
            kernel=hmc_kernel,
            num_burnin_steps=self.param_set.n_burnin)

        result = None
        with tf.Session() as sess:
            sess.run(tf.global_variables_initializer())
            states_, results_ = sess.run([states, kernels_results])
            result = self.plot_estimated(states_[0])

        return result

    def plot_estimated(self, ability_samples):
        '''Plots results'''
        medians = np.median(ability_samples, axis=0).reshape(-1)
        abilities_actual = self.abilities.eval().reshape(-1)
        result = self.calculate_results(ability_samples, medians, abilities_actual)
        self.plot_charts_for_estimations(ability_samples, medians, abilities_actual)
        return result

    ## ability_samples:2D, medians:1D, abilities_actual:1D
    def calculate_results(self, ability_samples, medians, abilities_actual):
        ## Correlation coefficient between actual and estimated ranks
        correlation = np.corrcoef(rankdata(medians), rankdata(abilities_actual))[1, 0]
        converged = correlation > MIN_CORRELATION
        distance = mean_squared_error(abilities_actual, medians)

        ## Exclude outliers
        min_set = np.min(ability_samples, axis=0).reshape(-1)
        max_set = np.max(ability_samples, axis=0).reshape(-1)
        ## Extract positions from tuples
        min_pos = np.where(min_set < MIN_ABILITIY_IN_CORRELATION)[0]
        max_pos = np.where(max_set > MAX_ABILITIY_IN_CORRELATION)[0]
        minmax_pos = np.concatenate([min_pos, max_pos], axis=0)
        ## Can be empty
        masks = np.unique(np.sort(minmax_pos))

        if converged and masks.shape[0] < min_set.shape[0]:
            ability_parts = np.delete(abilities_actual, masks)
            medians_parts = np.delete(medians, masks)
            distance = mean_squared_error(ability_parts, medians_parts)

        return FittingResult(correlation=correlation, converged=converged, distance=distance)

    ## ability_samples:2D, medians:1D, abilities_actual:1D
    def plot_charts_for_estimations(self, ability_samples, medians, abilities_actual):
        ## Plot estimated abilities ordered by actual abilities
        plt.figure(figsize=(6, 6))
        seaborn.boxplot(data=pandas.DataFrame(ability_samples), color='magenta', width=0.8)
        plt.title('Estimated abilities with ranges')
        plt.xlabel('Actual rank of abilities')
        plt.ylabel('Estimated ability')
        plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
        plt.savefig(self.get_png_filename(ESTIMATED_ABILITY_BOX1_BASENAME), dpi=160)
        plt.close()

        ## Plot confidential intervals for abilities of the respondents ordered by actual abilities
        plt.figure(figsize=(6, 6))
        data = pandas.DataFrame(ability_samples)
        data.boxplot(positions=abilities_actual, widths=np.full(abilities_actual.shape, 0.02))
        plt.xlim(0.0, 1.0)
        plt.plot([0.0, 1.0], [0.0, 1.0], color='magenta', lw=2, linestyle='--')
        plt.title('Estimated and actual abilities with ranges')
        plt.xlabel('Actual ability')
        plt.ylabel('Estimated ability')
        plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
        plt.savefig(self.get_png_filename(ESTIMATED_ABILITY_BOX2_BASENAME), dpi=160)
        plt.close()

        ## Plot actual and estimated abilities to check whether they are on a diagonal line
        plt.figure(figsize=(6, 6))
        plt.title('Abilities')
        plt.xlabel('Actual')
        plt.ylabel('Estimated')
        plt.scatter(abilities_actual, medians, color='darkmagenta', alpha=0.7)
        plt.tight_layout()
        plt.savefig(self.get_png_filename(ESTIMATED_ABILITY_SCATTER_BASENAME), dpi=160)
        plt.close()

class QuestionAndAbilityLauncher(object):
    '''Launch QuestionAndAbility'''
    def __init__(self, args):
        (options, args) = self.parse_options(args)
        if not os.path.isdir(options.outputdir):
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), options.outputdir)

        format_trial = datetime.datetime.now().strftime("%Y%m%d%H%M%S")
        format_trial += '_{0:0' + str(len(str(options.n_trials))) + 'd}'
        if options.optimize:
            self.optimize(options, format_trial)
        else:
            self.iterate(options, format_trial)

    def parse_options(self, args):
        parser = OptionParser()
        parser.add_option('-z', '--optimize', action="store_true", dest='optimize',
                          help='Use an optimzer to search HMC parameters', default=False)

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
        return parser.parse_args(args)

    @staticmethod
    def get_hmc_leapfrog_steps(x):
        return int(math.pow(2.0, x))

    def merge_write_result(self, format_trial, trial, elapsed_time, param_set, result, input_df):
        print(elapsed_time, "seconds")

        data = {'trial':[trial], 'elapsed_time':[elapsed_time],
                'hmc_stepsize':[param_set.hmc_stepsize], 'hmc_leapfrog_steps':[param_set.hmc_leapfrog_steps],
                'n_draws':[param_set.n_draws], 'n_burnin':[param_set.n_burnin],
                'correlation':[result.correlation], 'converged':[result.converged], 'distance':[result.distance]}

        merged_df = input_df
        new_df = pandas.DataFrame(data)
        if merged_df is None:
            merged_df = new_df
        else:
            merged_df = input_df.append(new_df)
        print(merged_df)

        filename = format_trial.format(trial)
        csv_common_filename = os.path.join(param_set.outputdir, 'summary.csv')
        csv_snapshot_filename = os.path.join(param_set.outputdir, 'summary_' + filename + '.csv')
        dump_snapshot_filename = os.path.join(param_set.outputdir, 'summary_' + filename + '.pickle')
        for out_filename in [csv_common_filename, csv_snapshot_filename]:
            merged_df.to_csv(out_filename)

        with open(dump_snapshot_filename, 'wb') as f:
            pickle.dump(obj=merged_df, file=f)

        return merged_df

    def optimize(self, options, format_trial):
        trial = 0
        df = None

        def fit(args):
            nonlocal options
            nonlocal format_trial
            nonlocal trial
            nonlocal df

            hmc_stepsize = args[0][0]
            hmc_leapfrog_steps = self.get_hmc_leapfrog_steps(args[0][1])
            trial = trial + 1
            n_draws = max(2, (options.n_draws * options.hmc_leapfrog_steps) // hmc_leapfrog_steps)
            n_burnin = max(2, (options.n_burnin * options.hmc_leapfrog_steps) // hmc_leapfrog_steps)
            param_set = ParamSet(outputdir=options.outputdir,
                                 n_respondents=options.n_respondents,
                                 use_sum=options.use_sum,
                                 n_questions=options.n_questions,
                                 n_draws=n_draws,
                                 n_burnin=n_burnin,
                                 hmc_stepsize=hmc_stepsize,
                                 hmc_leapfrog_steps=hmc_leapfrog_steps)

            start_time = time.time()
            result = QuestionAndAbility(param_set, format_trial.format(trial)).estimate()
            elapsed_time = time.time() - start_time
            df = self.merge_write_result(format_trial, trial, elapsed_time, param_set, result, df)
            return result.distance

        bounds = [{'name': 'hmc_stepsize', 'type': 'continuous', 'domain': (0.00008, 0.005)},
                  {'name': 'hmc_leapfrog_steps', 'type': 'continuous', 'domain': (1.0, 7.0)}]
        opt = GPyOpt.methods.BayesianOptimization(f=fit, domain=bounds, initial_design_numdata=10, acquisition_type='MPI')
        opt.run_optimization(max_iter=options.n_trials)

        hmc_stepsize_opt, hmc_leapfrog_steps_opt = opt.x_opt
        hmc_leapfrog_steps_opt = self.get_hmc_leapfrog_steps(hmc_leapfrog_steps_opt)
        print("Optimal stepsize=", hmc_stepsize_opt, ", Optimal leapfrog_steps=", hmc_leapfrog_steps_opt)

    def iterate(self, options, format_trial):
        param_set = ParamSet(outputdir=options.outputdir,
                             n_respondents=options.n_respondents,
                             use_sum=options.use_sum,
                             n_questions=options.n_questions,
                             n_draws=options.n_draws,
                             n_burnin=options.n_burnin,
                             hmc_stepsize=options.hmc_stepsize,
                             hmc_leapfrog_steps=options.hmc_leapfrog_steps)

        df = None
        for trial in range(1, options.n_trials+1):
            start_time = time.time()
            result = QuestionAndAbility(param_set, format_trial.format(trial)).estimate()
            elapsed_time = time.time() - start_time
            df = self.merge_write_result(format_trial, trial, elapsed_time, param_set, result, df)

if __name__ == '__main__':
    QuestionAndAbilityLauncher(sys.argv)
