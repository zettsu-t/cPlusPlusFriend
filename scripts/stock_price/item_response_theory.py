#!/usr/bin/python3
# coding: utf-8

'''
Implementation of the article below with TensorFlow Probability
http://norimune.net/2949

Based on an example of TensorFlow Probability
https://github.com/tensorflow/probability/tree/master/tensorflow_probability/python/edward2
https://www.hellocybernetics.tech/entry/2018/11/09/231817
'''

import time
import matplotlib.pyplot as plt
import numpy as np
import pandas
import seaborn
import tensorflow as tf
import tensorflow_probability as tfp
from tensorflow_probability import edward2 as ed

tfd = tfp.distributions

## Size of MCMC iterations
N_RESULTS = 500
N_BURNIN = 500
## Choose an appropriate step size or states are converged irregularly
HMC_STEP_SIZE = 0.001
HMC_LEAPFROG_STEPS = 10

## Number of respondents and questions
N_RESPONDENTS = 50 #500
N_QUESTIONS = 500

## Chart filenames
ANSWER_ABILITY_FILANAME = 'answer_ability.png'
ACTUAL_ABILITY_FILANAME = 'actual_ability.png'
ESTIMATED_ABILITY_BOX1_FILANAME = 'estimated_ability_box1.png'
ESTIMATED_ABILITY_BOX2_FILANAME = 'estimated_ability_box2.png'
ESTIMATED_ABILITY_SCATTER_FILANAME = 'estimated_ability_scatter.png'

## Assuming four-choice questions
CHANCE = 0.25
## Distribution of respondent abilities
MU_THETA = 0.5
SIGMA_THETA = 0.17
## Distribution of questions difficulties
DIFFICULTY_MIN = 0.1
DIFFICULTY_MAX = 0.9
MU_DISCRIMINATION = 16.0
SIGMA_DISCRIMINATION = 3.0

class QuestionAndAbility(object):
    '''Estimate abilities under the item response theory'''
    def __init__(self, n_respondents, n_questions):
        self.n_respondents = n_respondents
        self.n_questions = n_questions

        ## Explanatory variables
        self.abilities = tf.clip_by_value(tf.contrib.framework.sort(
            tf.random.normal(shape=[self.n_respondents], mean=MU_THETA, stddev=SIGMA_THETA)),
                             clip_value_min=0.001, clip_value_max=0.999, name='actual_abilities')
        delta = (DIFFICULTY_MAX - DIFFICULTY_MIN) / (self.n_questions - 1)
        difficulties = np.append(np.arange(DIFFICULTY_MIN, DIFFICULTY_MAX, delta), DIFFICULTY_MAX)
        difficulties = difficulties[(len(difficulties)-self.n_questions):]
        self.difficulties = tf.constant(difficulties, dtype=tf.float32)
        self.discriminations = tf.constant(np.random.normal(loc=MU_DISCRIMINATION, scale=SIGMA_DISCRIMINATION, size=[self.n_questions]), dtype=tf.float32)

        ## Makes normalized sigmoid parameters with broadcasting
        locs = tf.transpose(tf.broadcast_to(input=self.abilities, shape=[self.n_questions, self.n_respondents]))
        diffs = tf.broadcast_to(input=self.difficulties, shape=[self.n_respondents, self.n_questions])
        ## Inflection points of sigmoid functions
        locs = locs - diffs
        scales = tf.broadcast_to(input=self.discriminations, shape=[self.n_respondents, self.n_questions])
        probabilities = tf.add(tf.constant(CHANCE, shape=[self.n_respondents, self.n_questions]),
                               tf.multiply(tf.constant(1.0-CHANCE, shape=[self.n_respondents, self.n_questions]), tf.nn.sigmoid(locs * scales)))

        ## Observed data
        ## https://stackoverflow.com/questions/35487598/sampling-bernoulli-random-variables-in-tensorflow
        ## Must be float, not int (see get_sample())
        self.y_answers = tf.nn.relu(tf.sign(probabilities - tf.random_uniform(tf.shape(probabilities))))
        ## Explanatory variable(s)
        self.x_abilities = tf.random.normal(shape=[self.n_respondents], mean=MU_THETA, stddev=SIGMA_THETA)

        self.plot_actual(self.y_answers, self.abilities)

    def plot_actual(self, answers, abilities):
       '''Plots the synthetic input data and we can check they are correct'''
       with tf.Session() as sess:
           sess.run(answers)
           plt.figure(figsize=(6, 6))
           plt.hist(abilities.eval(), bins=25, color='royalblue')
           plt.savefig(ACTUAL_ABILITY_FILANAME, dpi=160)

           plt.figure(figsize=(6, 6))
           plt.title('Abilities and answers')
           plt.xlabel('Rank of abilities')
           plt.ylabel('Number of correct answers')
           n_correct_answers = np.sum(answers.eval(), 1)
           plt.scatter(list(range(n_correct_answers.shape[0])), n_correct_answers, color='mediumblue', alpha=0.7)
           plt.savefig(ANSWER_ABILITY_FILANAME, dpi=160)

    def get_sample(self):
        '''Returns an observation as an MCMC sample'''
        ## Relaxed from the actual distribution
##      abilities = ed.Uniform(low=0.0, high=1.0, sample_shape=self.n_respondents, name='abilities')
        ## Same as the actual distribution
        abilities = ed.Normal(loc=MU_THETA, scale=SIGMA_THETA, sample_shape=self.n_respondents, name='abilities')
        ## Same as the input data
        locs = tf.transpose(tf.broadcast_to(input=abilities, shape=[self.n_questions, self.n_respondents]))
        diffs = tf.broadcast_to(input=self.difficulties, shape=[self.n_respondents, self.n_questions])
        locs = locs - diffs
        scales = tf.broadcast_to(input=self.discriminations, shape=[self.n_respondents, self.n_questions])
        logits = locs * scales

        ## The support of Bernoulli distributions takes 0 or 1 but
        ## this function must return float not int to differentiate
        probs = tf.add(tf.constant(CHANCE, shape=[self.n_respondents, self.n_questions]),
                       tf.multiply(tf.constant(1.0-CHANCE, shape=[self.n_respondents, self.n_questions]), tf.nn.sigmoid(logits)))
        odds = tf.divide(probs, tf.subtract(tf.constant(1.0, shape=[self.n_respondents, self.n_questions], dtype=tf.float32), probs))
        answers = ed.Bernoulli(logits=tf.log(odds), name='answers', dtype=tf.float32)
        return answers

    def target_log_prob_fn(self, abilities):
        '''Applies observations partially to calculate their log likelihood'''
        log_joint = ed.make_log_joint_fn(self.get_sample)
        return log_joint(abilities=abilities,
                         answers=self.y_answers)

    def estimate(self):
        '''Estimates abilities by an HMC sampler'''
        hmc_kernel = tfp.mcmc.HamiltonianMonteCarlo(
            target_log_prob_fn=self.target_log_prob_fn,
            step_size=HMC_STEP_SIZE,
            num_leapfrog_steps=HMC_LEAPFROG_STEPS)

        states, kernels_results = tfp.mcmc.sample_chain(
            num_results=N_RESULTS,
            current_state=[self.x_abilities],
            kernel=hmc_kernel,
            num_burnin_steps=N_BURNIN)

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
        plt.savefig(ESTIMATED_ABILITY_BOX1_FILANAME, dpi=160)

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
        plt.savefig(ESTIMATED_ABILITY_BOX2_FILANAME, dpi=160)

        ## Plot actual and estimated abilities to check whether they are on a diagonal line
        plt.figure(figsize=(6, 6))
        plt.title('Abilities')
        plt.xlabel('Actual')
        plt.ylabel('Estimated')
        plt.scatter(abilities, medians, color='darkmagenta', alpha=0.7)
        plt.tight_layout()
        plt.savefig(ESTIMATED_ABILITY_SCATTER_FILANAME, dpi=160)

if __name__ == '__main__':
    start_time = time.time()
    QuestionAndAbility(N_RESPONDENTS, N_QUESTIONS).estimate()
    elapsed_time = time.time() - start_time
    print(elapsed_time)
