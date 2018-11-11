#!/usr/bin/python3
# coding: utf-8

'''
Implementation of the article below with TensorFlow Probability
'Bayesian Methods for Hackers'
https://github.com/CamDavidsonPilon/Probabilistic-Programming-and-Bayesian-Methods-for-Hackers/blob/master/Chapter2_MorePyMC/Ch2_MorePyMC_PyMC3.ipynb

Based on an example of TensorFlow Probability
https://github.com/tensorflow/probability/tree/master/tensorflow_probability/python/edward2
https://www.hellocybernetics.tech/entry/2018/11/09/231817
'''

from collections import namedtuple
from optparse import OptionParser
import time
import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf
import tensorflow_probability as tfp
tfd = tfp.distributions

COIN_PROBABILITY = 0.5
HISTOGRAM_FILANAME = 'tough_question_tfp.png'
ParamSet = namedtuple('ParamSet', ('n_respondents', 'n_trues', 'n_draws', 'n_burnin', 'hmc_stepsize', 'hmc_leapfrog_steps'))

class ToughQuestionTfp(object):
    '''Estimate a true probability of a tough question'''
    def __init__(self, paramSet):
        self.paramSet = paramSet

        ## Explanatory variable(s)
        self.true_prob = tf.random_uniform([], minval=0.0, maxval=1.0)
        ## Observed data
        n_falses = self.paramSet.n_respondents - self.paramSet.n_trues
        self.observations = tf.random.shuffle(
            tf.concat([tf.ones(self.paramSet.n_trues, dtype=tf.int32),
                       tf.zeros(n_falses, dtype=tf.int32)], 0))

    def target_log_prob_fn(self, true_prob):
        '''Returns likelihood'''
        log_prob_parts = [
            tfd.Bernoulli(probs=COIN_PROBABILITY).log_prob(tf.fill([self.paramSet.n_respondents], 1)) +
                tfd.Bernoulli(probs=true_prob).log_prob(self.observations),
            tfd.Bernoulli(probs=COIN_PROBABILITY).log_prob(tf.fill([self.paramSet.n_respondents], 0)) +
                tfd.Bernoulli(probs=COIN_PROBABILITY).log_prob(self.observations)
        ]

        sum_log_prob = tf.reduce_sum(tf.reduce_logsumexp(log_prob_parts, 0))
        return sum_log_prob

    def estimate(self):
        hmc_kernel = tfp.mcmc.HamiltonianMonteCarlo(
            target_log_prob_fn=self.target_log_prob_fn,
            step_size=self.paramSet.hmc_stepsize,
            num_leapfrog_steps=self.paramSet.hmc_leapfrog_steps)

        states, kernels_results = tfp.mcmc.sample_chain(
            num_results=self.paramSet.n_draws,
            current_state=[self.true_prob],
            kernel=hmc_kernel,
            num_burnin_steps=self.paramSet.n_burnin)

        with tf.Session() as sess:
            sess.run(tf.global_variables_initializer())
            states_, results_ = sess.run([states, kernels_results])
            plt.figure(figsize=(6, 6))
            plt.hist(states_[0], bins=50)
            plt.savefig(HISTOGRAM_FILANAME, dpi=160)

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-r', '--respondents', type='int', dest='n_respondents',
                      help='Number of respondents', default=1000)

    parser.add_option('-t', '--trues', type='int', dest='n_trues',
                      help='Number of true responses', default=300)

    parser.add_option('-d', '--draws', type='int', dest='n_draws',
                      help='Number of Markov chain draws', default=500)

    parser.add_option('-b', '--burnin', type='int', dest='n_burnin',
                      help='Number of burnin chain steps', default=250)

    parser.add_option('-s', '--stepsize', type='float', dest='hmc_stepsize',
                      help='HMC stepsize', default=0.01)

    parser.add_option('-l', '--leapfrog', type='int', dest='hmc_leapfrog_steps',
                      help=' Number of HMC leapfrog steps', default=5)

    (options, args) = parser.parse_args()
    paramSet = ParamSet(n_respondents=options.n_respondents,
                        n_trues=options.n_trues,
                        n_draws=options.n_draws,
                        n_burnin=options.n_burnin,
                        hmc_stepsize=options.hmc_stepsize,
                        hmc_leapfrog_steps=options.hmc_leapfrog_steps)

    start_time = time.time()
    ToughQuestionTfp(paramSet).estimate()
    elapsed_time = time.time() - start_time
    print(elapsed_time)
