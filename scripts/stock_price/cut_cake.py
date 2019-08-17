#!/usr/bin/python3
# coding: utf-8

import math
import sys
import numpy as np
import matplotlib.patches as patches
import matplotlib.pyplot as plt
import matplotlib.animation as animation

def main():
    use_arc = False
    if len(sys.argv) > 1:
        use_arc = sys.argv[1] == 'arc'

    radius = 1.0
    n_arcs = 5
    n_segments = 10
    chart_size = (2.2 if use_arc else 1.02) * radius

    n_frames = n_arcs * n_segments
    interval_msec = 66
    theta = 360.0 / n_arcs
    theta_margin = 1
    theta_sin = math.sin(math.pi * theta / 180)
    theta_cos = math.cos(math.pi * theta / 180)
    mat = np.matrix([[theta_cos, -theta_sin], [theta_sin, theta_cos]])

    def plot(frame_number):
        plt.cla()
        ex = patches.Circle(xy=(0, 0), radius=radius, linewidth=1.0, edgecolor='black', facecolor='khaki')
        ax.add_patch(ex)

        theta_base = theta if use_arc else 0.0
        outer_pos = np.matrix([[radius * (1 + theta_cos)], [radius * theta_sin]])
        circle_pos = np.matrix([[radius], [0.0]])

        n_parts = min(frame_number, n_frames)
        while n_parts > 0:
            segment = min(n_parts, n_segments)
            next_circle_pos = mat.dot(circle_pos)

            if use_arc:
                theta_end_pos = 540.0 + theta_base + theta_margin
                theta_end = theta_end_pos - math.floor(theta_end_pos / 360.0) * 360.0
                theta_diff_pos = (theta + 2.0 * theta_margin) * ((1.0 * segment) / n_segments)
                theta_start_pos = theta_end_pos - theta_diff_pos
                theta_start = theta_start_pos - math.floor(theta_start_pos / 360.0) * 360.0

                pc = patches.Arc(xy=(outer_pos[0,0], outer_pos[1,0]), width=2.0, height=2.0, angle=0, theta1=0, theta2=360, linewidth=1.0, color='gray')
                pt = patches.Arc(xy=(outer_pos[0,0], outer_pos[1,0]), width=2.0, height=2.0, angle=0, theta1=theta_start, theta2=theta_end, linewidth=2.0)
                ax.add_patch(pc)
                ax.add_patch(pt)
            else:
                diff = next_circle_pos - circle_pos
                diff = diff * ((1.0 * segment) / n_segments)
                ax.arrow(x=circle_pos[0,0], y=circle_pos[1,0], dx=diff[0,0], dy=diff[1,0], width=0.02, head_width=0.08, head_length=0.1, length_includes_head=True, color='black')

            outer_pos = mat.dot(outer_pos)
            circle_pos = next_circle_pos
            theta_base += theta
            n_parts -= n_segments

        ax.set_xlim(-chart_size, chart_size)
        ax.set_ylim(-chart_size, chart_size)
        ax.axis('off')

    figsize = 2.56
    fig_animation = plt.figure(figsize=(figsize, figsize))
    ax = fig_animation.add_subplot(111)
    ax.set_aspect('equal')
    anim = animation.FuncAnimation(
        fig_animation, plot, interval=interval_msec,
        frames=range(1, n_frames + 10),
        repeat=True,
    )
    anim.save('cut_cake.gif', writer='pillow')

if __name__ == '__main__':
    main()
