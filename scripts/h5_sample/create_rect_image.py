import numpy as np
from PIL import Image

def save_rect(width, height, color, filename):
    data = np.zeros((height, width, 3), dtype=np.uint8)
    data[0:height, 0:width] = color
    img = Image.fromarray(data, 'RGB')
    img.save(filename)

save_rect(width=128, height=96, color=[65, 105, 255], filename="blue.png")
save_rect(width=64, height=128, color=[255, 165, 0], filename="orange.png")
