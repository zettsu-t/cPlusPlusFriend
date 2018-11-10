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
import tensorflow as tf
import tensorflow_probability as tfp
from tensorflow_probability import edward2 as ed

tfd = tfp.distributions

## Size of MCMC iterations
N_RESULTS = 1000
N_BURNIN = 500

## Number of respondents(n) and questions(k)
N = 500
K = 200

## Assuming four-choice questions
CHANCE = 0.25
MU_THETA = 0.5
SIGMA_THETA = 0.17
DIFFICULTY_MIN = 0.1
DIFFICULTY_MAX = 0.9
MU_DISCRIMINATION = 16.0
SIGMA_DISCRIMINATION = 3.0

## Explanatory variable(s)
abilities = tf.clip_by_value(tf.contrib.framework.sort(tf.random.normal(shape=[N], mean=MU_THETA, stddev=SIGMA_THETA)),
                             clip_value_min=0.001, clip_value_max=0.999, name="abilities")
delta = (DIFFICULTY_MAX - DIFFICULTY_MIN) / (K - 1)
difficulties = tf.range(start=DIFFICULTY_MIN, limit=DIFFICULTY_MAX+1e-4, delta=delta, name="difficulties")
discriminations = tf.random.normal(shape=[K], mean=MU_DISCRIMINATION, stddev=SIGMA_DISCRIMINATION, name="discrimination")

## Fills normaized sigmoid parameters with broadcasting
locs = tf.broadcast_to(input=abilities, shape=[K,N])
locs = tf.transpose(locs)
diffs = tf.broadcast_to(input=difficulties, shape=[N,K])
locs = locs - diffs
scales = tf.broadcast_to(input=discriminations, shape=[N,K])
correct_probs = tf.nn.sigmoid(locs * scales)

## Explanatory variables
x_abilities = tf.random_uniform([N], minval=0.0, maxval=1.0)

## Observed data
## https://stackoverflow.com/questions/35487598/sampling-bernoulli-random-variables-in-tensorflow
answers = tf.nn.relu(tf.sign(correct_probs - tf.random_uniform(tf.shape(correct_probs))))

with tf.Session() as sess:
    sess.run(answers)
    plt.figure(figsize=(6, 6))
    plt.title("Abilities and answers")
    plt.xlabel("Rank of abilities")
    plt.ylabel("Number of correct answers")
    plt.scatter(list(range(N)), np.sum(answers.eval(), 1))
    plt.tight_layout()
    plt.savefig("answer_ability.png", dpi=160)

def get_dist():
    abilities = ed.Uniform(low=0.0, high=1.0, sample_shape=N, name="abilities")
    locs = tf.broadcast_to(input=abilities, shape=[K,N])
    locs = tf.transpose(locs)
    diffs = tf.broadcast_to(input=difficulties, shape=[N,K])
    locs = locs - diffs
    scales = tf.broadcast_to(input=discriminations, shape=[N,K])
    logits = locs * scales
    ## Must be float, not int
    answers = ed.Bernoulli(logits=logits, name="answers", dtype=tf.float32)
    return answers

log_joint = ed.make_log_joint_fn(get_dist)

# def target_log_prob_fn(abilities, difficulties, discriminations):
def target_log_prob_fn(abilities):
    return log_joint(abilities=abilities,
                     answers=answers)

hmc_kernel = tfp.mcmc.HamiltonianMonteCarlo(
    target_log_prob_fn=target_log_prob_fn,
    step_size=0.003,
    num_leapfrog_steps=5)

states, kernels_results = tfp.mcmc.sample_chain(
    num_results=N_RESULTS,
    current_state=[x_abilities],
    kernel=hmc_kernel,
    num_burnin_steps=N_BURNIN)

start_time = time.time()
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    states_, results_ = sess.run([states, kernels_results])
    plt.figure(figsize=(6, 6))
    plt.title("Abilities")
    plt.xlabel("Actual")
    plt.ylabel("Estimated")
    plt.scatter(abilities.eval(), np.median(states_[0], 0))
    plt.tight_layout()
    plt.savefig("estimated_ability.png", dpi=160)

elapsed_time = time.time() - start_time
print(elapsed_time)
