#!/usr/bin/python3
# coding: utf-8

import numpy as np

N_PLAYERS = 3;
N_STATES = 2 ** N_PLAYERS
HIT_RATE = np.array([0.3, 0.5, 1.0])
MISS_RATE = 1.0 - np.array([0.3, 0.5, 1.0])

TRANSITION_OPTIMAL = np.array(
    [[[1.0, 0.0, 0.0, 0.0,          0.0, 0.0,          0.0, 0.0],
      [0.0, 1.0, 0.0, HIT_RATE[0],  0.0, HIT_RATE[0],  0.0, 0.0],
      [0.0, 0.0, 1.0, 0.0,          0.0, 0.0,          0.0, 0.0],
      [0.0, 0.0, 0.0, MISS_RATE[0], 0.0, 0.0,          0.0, 0.0],
      [0.0, 0.0, 0.0, 0.0,          1.0, 0.0,          0.0, 0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, MISS_RATE[0], 0.0, 0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0,     0.0,               1.0, 0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0,     0.0,               0.0, 1.0]],
     [[1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 1.0, HIT_RATE[1],  0.0, 0.0, HIT_RATE[1],  0.0],
      [0.0, 0.0, 0.0, MISS_RATE[1], 0.0, 0.0, 0.0,          HIT_RATE[1]],
      [0.0, 0.0, 0.0, 0.0,          1.0, 0.0, 0.0,          0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, 1.0, 0.0,          0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, 0.0, MISS_RATE[1], 0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, 0.0, 0.0,          MISS_RATE[1]]],
     [[1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 0.0, 0.0, 1.0, HIT_RATE[2],  HIT_RATE[2],  0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0, MISS_RATE[2], 0.0,          HIT_RATE[2]],
      [0.0, 0.0, 0.0, 0.0, 0.0, 0.0,          MISS_RATE[2], 0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0, 0.0,          0.0,          MISS_RATE[2]]]]
    ).reshape(N_PLAYERS, N_STATES, N_STATES)

TRANSITION_NAIVE_B = np.array(
    [[[1.0, 0.0, 0.0, 0.0,          0.0, 0.0,          0.0, 0.0],
      [0.0, 1.0, 0.0, HIT_RATE[0],  0.0, HIT_RATE[0],  0.0, 0.0],
      [0.0, 0.0, 1.0, 0.0,          0.0, 0.0,          0.0, 0.0],
      [0.0, 0.0, 0.0, MISS_RATE[0], 0.0, 0.0,          0.0, HIT_RATE[0]],
      [0.0, 0.0, 0.0, 0.0,          1.0, 0.0,          0.0, 0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, MISS_RATE[0], 0.0, 0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0,     0.0,               1.0, 0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0,     0.0,               0.0, MISS_RATE[0]]],
     [[1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 1.0, HIT_RATE[1],  0.0, 0.0, HIT_RATE[1],  0.0],
      [0.0, 0.0, 0.0, MISS_RATE[1], 0.0, 0.0, 0.0,          HIT_RATE[1]],
      [0.0, 0.0, 0.0, 0.0,          1.0, 0.0, 0.0,          0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, 1.0, 0.0,          0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, 0.0, MISS_RATE[1], 0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, 0.0, 0.0,          MISS_RATE[1]]],
     [[1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 0.0, 0.0, 1.0, HIT_RATE[2],  HIT_RATE[2],  0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0, MISS_RATE[2], 0.0,          HIT_RATE[2]],
      [0.0, 0.0, 0.0, 0.0, 0.0, 0.0,          MISS_RATE[2], 0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0, 0.0,          0.0,          MISS_RATE[2]]]]
    ).reshape(N_PLAYERS, N_STATES, N_STATES)

TRANSITION_NAIVE_C = np.array(
    [[[1.0, 0.0, 0.0, 0.0,          0.0, 0.0,          0.0, 0.0],
      [0.0, 1.0, 0.0, HIT_RATE[0],  0.0, HIT_RATE[0],  0.0, 0.0],
      [0.0, 0.0, 1.0, 0.0,          0.0, 0.0,          0.0, 0.0],
      [0.0, 0.0, 0.0, MISS_RATE[0], 0.0, 0.0,          0.0, 0.0],
      [0.0, 0.0, 0.0, 0.0,          1.0, 0.0,          0.0, 0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, MISS_RATE[0], 0.0, HIT_RATE[0]],
      [0.0, 0.0, 0.0, 0.0, 0.0,     0.0,               1.0, 0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0,     0.0,               0.0, MISS_RATE[0]]],
     [[1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 1.0, HIT_RATE[1],  0.0, 0.0, HIT_RATE[1],  0.0],
      [0.0, 0.0, 0.0, MISS_RATE[1], 0.0, 0.0, 0.0,          HIT_RATE[1]],
      [0.0, 0.0, 0.0, 0.0,          1.0, 0.0, 0.0,          0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, 1.0, 0.0,          0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, 0.0, MISS_RATE[1], 0.0],
      [0.0, 0.0, 0.0, 0.0,          0.0, 0.0, 0.0,          MISS_RATE[1]]],
     [[1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0],
      [0.0, 0.0, 0.0, 0.0, 1.0, HIT_RATE[2],  HIT_RATE[2],  0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0, MISS_RATE[2], 0.0,          HIT_RATE[2]],
      [0.0, 0.0, 0.0, 0.0, 0.0, 0.0,          MISS_RATE[2], 0.0],
      [0.0, 0.0, 0.0, 0.0, 0.0, 0.0,          0.0,          MISS_RATE[2]]]]
    ).reshape(N_PLAYERS, N_STATES, N_STATES)

def assert_near_one(all_probs):
    assert (1.0-(1e-5)) < all_probs < (1.0+(1e-5))

def print_probabilites(label, probs):
    assert_near_one(np.sum(probs))
    print('{0}{1:.3f}, {2:.3f}, {3:.3f}'.format(label, *(np.ndarray.tolist(probs))))

def calculate(label, transition):
    probabilites = np.zeros(shape=[N_STATES], dtype=float)
    probabilites[N_STATES - 1] = 1.0

    for player in range(N_PLAYERS):
        for from_state in range(N_STATES):
            total_probability = np.sum(transition[player,:,from_state])
            assert_near_one(total_probability)

    for i in range(40):
        for player in range(N_PLAYERS):
            probabilites = np.dot(transition[player,:,:], probabilites)
    print_probabilites(label, probabilites[[1,2,4]])

print('Probability to win')
print('Player    A      B      C')
calculate('Nobody   ', TRANSITION_OPTIMAL)
calculate('Target=B ', TRANSITION_NAIVE_B)
calculate('Target=C ', TRANSITION_NAIVE_C)
