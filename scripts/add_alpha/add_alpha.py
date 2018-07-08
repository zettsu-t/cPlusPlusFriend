#!/usr/bin/python3
# coding: utf-8

'''
Adding an alpha pixel to an image and writing as a ARGB image file
Copyright (C) 2018 Zettsu Tatsuya
'''

import cv2
from optparse import OptionParser
import numpy as np

def main():
    parser = OptionParser()
    parser.add_option('-i', '--input', dest='in_image_filename', default='in.png',
                      help='Input image filename')
    parser.add_option('-o', '--output', dest='out_image_filename', default='out.png',
                      help='Output image filename')
    (options, args) = parser.parse_args()

    in_img = cv2.imread(options.in_image_filename)
    shape = in_img.shape
    b_channel, g_channel, r_channel = cv2.split(in_img)
    alpha_channel = np.ones((shape[0], shape[1], 1), dtype=b_channel.dtype) * 255

    # Makes the right-bottom pixel transparent
    alpha_channel[shape[0]-1, shape[1]-1] = 0
    out_img = cv2.merge((b_channel, g_channel, r_channel, alpha_channel))
    cv2.imwrite(options.out_image_filename, out_img)

if __name__ == "__main__":
    main()
