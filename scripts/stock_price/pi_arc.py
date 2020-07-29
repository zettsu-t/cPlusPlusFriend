#!/usr/bin/python3
# coding: utf-8

import math
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

def draw_pi_set(n_half_split, ax):
    n_split = n_half_split * 2
    theta_unit = 360.0 / n_split
    theta_half_unit = theta_unit / 2.0
    theta_half_radian = theta_half_unit * math.pi / 180.0
    width = math.sin(theta_half_radian)
    height = math.cos(theta_half_radian)

    for i in range(n_half_split):
        x = width * 2 * i
        y = 0
        ax.add_patch(mpatches.Arc(xy=[x, y], width=2.0, height=2.0, angle=90, theta1=-theta_half_unit, theta2=theta_half_unit, edgecolor='blue'))
        ax.add_patch(mpatches.Polygon(xy=[(x, y), (x - width, height)], edgecolor='black'))
        x = width * (2 * i + 1)
        y = height
        ax.add_patch(mpatches.Arc(xy=[x, y], width=2.0, height=2.0, angle=270, theta1=-theta_half_unit, theta2=theta_half_unit, edgecolor='blue'))
        ax.add_patch(mpatches.Polygon(xy=[(x, y), (x - width, 0)], edgecolor='black'))

    ax.add_patch(mpatches.Polygon(xy=[(width * n_split, 0), (width * (n_split - 1), height), ], edgecolor='black'))
    ax.axis([-0.5, 3.5, -0.2, 1.2])
    ax.set_xticks([0.0, 3.14])
    ax.set_yticks([0.0, 1.0])
    ax.set_aspect("equal")

def draw_all(n_half_split_set, png_filename):
    n_chart = len(n_half_split_set)
    fig = plt.figure(figsize=(8.0, 2.0 * n_chart))
    for i, n_half_split in zip(range(0, n_chart), n_half_split_set):
        ax = fig.add_subplot(n_chart, 1, i+1)
        draw_pi_set(n_half_split, ax)

    plt.savefig(png_filename)

def main():
    draw_all(n_half_split_set=[4,8,16,32,64], png_filename='pi.png')

if __name__ == '__main__':
    main()
