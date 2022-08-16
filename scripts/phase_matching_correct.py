import numpy as np
from .phase_correlation import phase_correlation


# def crop(arr, height, width):
#     s1 = round(h1 / 2 - height / 2)
#     s2 = round(h1 / 2 + height / 2)
#     s3 = round(w1 / 2 - width / 2)
#     s4 = round(w1 / 2 + width / 2)


def phase_matching_correct(img_tmp, img_src,
                           n_current_x,
                           n_current_y,
                           count=3,
                           n_xstep=50):
    """

    :param img_tmp: The current flight image
    :param img_src: The reference image (base map)
    :param n_current_x:
    :param n_current_y:
    :param count:
    :param n_xstep:
    :return:
    """
    height = 450
    width = 450
    temp_size = 300

    h1, w1 = img_tmp.shape

    s1 = round(h1 / 2 - temp_size / 2)
    s2 = round(h1 / 2 + temp_size / 2)
    s3 = round(w1 / 2 - temp_size / 2)
    s4 = round(w1 / 2 + temp_size / 2)
    img_tmp = img_tmp[s1:s2, s3:s4]

    s1 = round(n_current_y - height / 2)
    s2 = round(n_current_y + height / 2)
    s3 = round(n_current_x - width / 2)
    s4 = round(n_current_x + width / 2)
    clip_img = img_src[s1:s2, s3:s4]  # extracted clip image from base map

    n_ystep = n_xstep
    n_xcount = int(np.floor((width - temp_size) / n_xstep))
    n_ycount = int(np.floor((height - temp_size) / n_ystep))

    n_total_count = n_xcount * n_ycount
    phase_pos = np.zeros((n_total_count, 3))
    run = 0

    for m in range(0, n_ycount * n_xcount, n_xstep):
        for n in range(0, n_ycount * n_ystep, n_ystep):
            nx_frm = int(m + temp_size / 2)
            ny_frm = int(n + temp_size / 2)

            s1 = round(ny_frm - temp_size / 2)
            s2 = round(ny_frm + temp_size / 2)
            s3 = round(nx_frm - temp_size / 2)
            s4 = round(nx_frm + temp_size / 2)
            search_img = clip_img[s1:s2, s3:s4]

            nx_pos, ny_pos, n = phase_correlation(img_tmp, search_img, nx_frm, ny_frm, temp_size)

            nx_pos = nx_pos + n_current_x - width / 2
            ny_pos = ny_pos + n_current_y - height / 2

            phase_pos[run, :] = np.array([nx_pos, ny_pos, n])
            run += 1

    n_sort = phase_pos[:, 2]
    i = np.argsort(-n_sort)
    i_max = i[:count]
    new_pos_x = phase_pos[i_max, 0]
    new_pos_y = phase_pos[i_max, 1]

    nx_pos = np.mean(new_pos_x)
    ny_pos = np.mean(new_pos_y)

    return int(nx_pos), int(ny_pos)
