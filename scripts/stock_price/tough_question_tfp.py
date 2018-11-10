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

import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf
import tensorflow_probability as tfp
## from tensorflow_probability import edward2 as ed

tfd = tfp.distributions

N = 1000
X = 300
N_RESULTS = 2000
N_BURNIN = 1000

## Explanatory variable(s)
true_prob = tf.random_uniform([], minval=0.0, maxval=1.0)
## Observed data
observations = tf.random.shuffle(tf.concat([tf.ones(X, dtype=tf.int32), tf.zeros(N-X, dtype=tf.int32)], 0))

def target_log_prob_fn(true_prob):
    log_prob_parts = [
        tfd.Bernoulli(probs=0.5).log_prob(tf.fill([N], 1)) + tfd.Bernoulli(probs=true_prob).log_prob(observations),
        tfd.Bernoulli(probs=0.5).log_prob(tf.fill([N], 0)) + tfd.Bernoulli(probs=0.5).log_prob(observations)
    ]
    sum_log_prob = tf.reduce_sum(tf.reduce_logsumexp(log_prob_parts, 0))
    return sum_log_prob

hmc_kernel = tfp.mcmc.HamiltonianMonteCarlo(
    target_log_prob_fn=target_log_prob_fn,
    step_size=0.01,
    num_leapfrog_steps=5)

states, kernels_results = tfp.mcmc.sample_chain(
    num_results=N_RESULTS,
    current_state=[true_prob],
    kernel=hmc_kernel,
    num_burnin_steps=N_BURNIN)

with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    states_, results_ = sess.run([states, kernels_results])
    plt.hist(states_[0], bins=50)
    plt.show()
