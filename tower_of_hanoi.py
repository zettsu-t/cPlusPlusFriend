#!/usr/bin/python3
# coding: utf-8

import numpy as np
from PIL import Image, ImageDraw

class TowerOfHanoi:
    def __init__(self, n_rods, n_disks):
        self.n_rods = n_rods
        self.n_disks = n_disks
        self.count = 0
        ## Rods * disks
        self.rods = np.zeros(shape=(self.n_rods, n_disks), dtype=np.int)
        self.rods[0,] = np.arange(start=1, stop=n_disks + 1)

        self.fast_mode = self.n_rods > self.n_disks
        self.n_moves = (self.n_disks * 2 - 1) if self.fast_mode else (2 ** self.n_disks)

    @staticmethod
    def check(rods):
        n_disks = rods.shape[1]
        expected_sum = (n_disks * (n_disks + 1)) // 2

        def compare(rod):
            checked_rods = rod.copy()
            checked_rods[checked_rods == 0] = n_disks
            return np.all(checked_rods == np.sort(checked_rods))

        ordered = np.all([compare(rods[i,]) for i in range(rods.shape[0])])
        disk_sum = (np.sum(rods, axis=(0,1)) == expected_sum)
        return ordered & disk_sum

    def move(self):
        if self.count < self.n_moves:
            (from_rod, to_rod) = self.move_fast(self.count) if self.fast_mode else self.move_slow()
            from_index = np.argmax(self.rods[from_rod,])
            disk = self.rods[from_rod, from_index]
            self.rods[from_rod, from_index] = 0
            self.rods[to_rod, np.argmax(self.rods[to_rod,] == 0)] = disk
            self.count += 1
            result = self.check(self.rods)
            assert result, "Found a wrong move!"

        return self.count >= self.n_moves

    def move_fast(self, count):
        from_rod = 0
        to_rod = count + 1
        if count >= self.n_disks:
            from_rod =  + self.n_disks * 2 - 1 - count
            to_rod = self.n_disks
        return (from_rod, to_rod)

    ## https://en.wikipedia.org/wiki/Tower_of_Hanoi
    def move_slow(self):
        from_rod = (self.count & (self.count - 1)) % self.n_rods
        to_rod = ((self.count | (self.count - 1)) + 1) % self.n_rods
        return (from_rod, to_rod)

    def to_string(self):
        def to_string_rod(rod):
            return str(rod)
        return "\n".join([to_string_rod(self.rods[i,]) for i in range(self.rods.shape[0])]) + "\n\n"

class TowerOfHanoiAnimetion:
    def __init__(self, n_rods, n_disks):
        self.n_rods = n_rods
        self.n_disks = n_disks

    def make_images(self, width, height, still_filename, animation_filename):
        tower = TowerOfHanoi(self.n_rods, self.n_disks)
        images = []
        count = 2
        while(count > 0):
            im = Image.new('RGB', (width, height), (0, 0, 0))
            draw = ImageDraw.Draw(im)
            self.plot(tower, width, height, draw)
            images.append(im)
            count -= 1 if tower.move() else 0
            print(tower.to_string())

        images[0].save(still_filename)
        images[0].save(animation_filename, save_all=True, append_images=images[1:], optimize=True, duration=66, loop=True)

    def plot(self, tower, sceeen_width, sceeen_height, draw):
        rods = tower.rods
        n_rods = rods.shape[0]
        n_disks = rods.shape[1]
        disk_width = sceeen_width / n_rods
        disk_height = sceeen_height / n_disks

        x_ratio = 0.8
        y_ratio = 0.99

        for rod in range(n_rods):
            x_center = disk_width * (rod + 0.5)
            for y in range(n_disks):
                disk = rods[rod, y]
                if disk <= 0:
                    continue

                width = disk_width * x_ratio * (n_disks + 1 - disk) / (2.0 * n_disks)
                left = x_center - width
                right = x_center + width

                y_center = disk_height * (n_disks - y - 0.5)
                height = disk_height * y_ratio / 2.0
                top = y_center - height
                bottom = y_center + height

                value = max(0, min(255, int(255 - (128 * disk) / n_disks)))
                draw.rectangle([(left, top), (right, bottom)], fill=(value, value, value), outline=(0, 0, 0))

TowerOfHanoiAnimetion(3, 9).make_images(320, 160, 'slow.png', 'slow.gif')
TowerOfHanoiAnimetion(10, 9).make_images(320, 160, 'fast.png', 'fast.gif')
