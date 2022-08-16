import numpy as np
from PIL import Image

from .phase_matching_correct import phase_matching_correct

thresh = 0
er_x = 0
er_y = 0


def match_aerial_to_map(aerial_image_path: str, map_image_path: str, x: int, y: int):
    aerial_image = Image.open(aerial_image_path)
    aerial_image = aerial_image.convert('L')

    map_image = Image.open(map_image_path)
    map_image = map_image.convert('L')

    new_pos = image_matching(aerial_image, map_image, np.array([x, y]))
    return new_pos[0], new_pos[1]


def image_matching(img_tmp1, img_src, pos):
    """
    :param img_tmp1: UAV image
    :param img_src: Base map image
    :param pos:
    :return:
    """
    n_true_x = pos[0] + er_x
    n_true_y = pos[1] - er_y

    img_tmp1 = np.array(img_tmp1)  # the current flight location image
    img_src = np.array(img_src)  # the reference image

    nx_pos, ny_pos = phase_matching_correct(img_tmp1, img_src, n_true_x, n_true_y, 3, 50)

    return np.array([nx_pos, ny_pos])
