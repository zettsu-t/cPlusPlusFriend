#!/usr/bin/python3
# coding: utf-8

'''
Compare memory footprints. Based on
https://twitter.com/uuyr112/status/1160375090090876930
'''

from memory_profiler import profile
import numpy as np

@profile
def hold_a_list(n):
    '''
    Hold candidates as a list
    '''
    l = [i for i in range(1,n) if i % 2 == 0]
    return sum(l)

@profile
def use_a_generator(n):
    '''
    Use generators to avoid holding many candidates on memory
    '''
    l = (i for i in range(1,n) if i % 2 == 0)
    return sum(l)

@profile
def use_a_np_array(n):
    '''
    Hold a NumPy array
    '''
    a = np.arange(1,n)
    f = np.frompyfunc(lambda s, x: ((s + x) if (x % 2 == 0) else s), 2, 1)
    return f.reduce(a, initial=0, dtype=np.int)

if __name__ == "__main__":
    n=1000001
    print((1 + ((n - 1) // 2)) * ((n - 1) // 2))
    print(hold_a_list(n))
    print(use_a_generator(n))
    print(use_a_np_array(n))
