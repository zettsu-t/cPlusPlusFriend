#!/usr/bin/python3
# coding: utf-8

'''
Create a white noise animation like a TV screen
'''

import itertools
import random
import matplotlib.pyplot as plt
import numpy as np
from PIL import Image

width = 256
height = 192
max_value = 255    # brightness
value_center = 64  # mean
value_range = 16   # stddev
n_frames = 10
frame_duration = 100

def create_image():
    image = np.zeros(shape=(height, width, 3), dtype=int)
    for y in range(0, height):
        for x in range(0, width):
            image[y, x] = int(np.random.normal() * value_range + value_center)

    pixels = np.uint8(np.clip(image, 0, max_value))
    return Image.fromarray(pixels), pixels

images, pixels = map(list, zip(*map(lambda _: create_image(), range(0, n_frames))))
images[0].save('out/white_noise.gif',
               save_all=True, append_images=images[1:], optimize=False,
               duration=frame_duration, loop=0)

plt.hist(x=np.array(pixels).reshape(-1), bins=range(0, max_value + 1))
plt.xlabel('value (brightness)')
plt.ylabel('# of pixels')
xticks = list(itertools.takewhile(lambda x: x <= (max_value + 1), itertools.count(0, value_center)))
plt.xticks(xticks)
plt.yticks([])
plt.savefig('out/white_noise_hist.png', dpi=160)
