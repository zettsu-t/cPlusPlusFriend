#!/usr/bin/python3
# coding: utf-8

'''
Create a white noise animation like a TV screen
'''

import numpy as np
from PIL import Image

width = 128
height = 96
n_frames = 10
frame_duration = 100
center_value = 64

def create_image():
    image = np.zeros(shape=(height, width, 3), dtype=int)
    for y in range(0, height):
        for x in range(0, width):
            value = int(np.random.normal() * center_value) + center_value
            image[y, x] = value
    return Image.fromarray(np.uint8(np.clip(image, 0, 255)))

images = list(map(lambda _: create_image(), range(0, n_frames)))
images[0].save('out/white_noise.gif',
               save_all=True, append_images=images[1:], optimize=False,
               duration=frame_duration, loop=0)
