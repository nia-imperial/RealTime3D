import numpy as np


def template_match(img_tmp, img_src, n_current_x, n_current_y, thresh):
    win_y, win_x, n_count = np.atleast_3d(np.asarray(img_tmp)).shape

    height = 450
    width = 450
    temp_size = 300

    # rgb to grayscale
    img_src = img_src.convert('L')

    img_tmp = np.array(img_tmp)
    img_src = np.array(img_src)

    # input image
    s1 = int(win_y / 2 - temp_size / 2)
    s2 = int(win_y / 2 + temp_size / 2)
    s3 = int(win_x / 2 - temp_size / 2)
    s4 = int(win_x / 2 + temp_size / 2)
    img_tmp = img_tmp[s1:s2, s3:s4]

    s1 = int(n_current_y - height / 2)
    s2 = int(n_current_y + height / 2)
    s3 = int(n_current_x - width / 2)
    s4 = int(n_current_x + width / 2)
    clip_img = img_src[s1:s2, s3:s4]

    # the grid per step
    nx_step = 50
    ny_step = 50

    # total count in x direction and y direction of this loop
    nx_count = (width - temp_size) // nx_step
    ny_count = (height - temp_size) // ny_step

    n_total_count = nx_count * ny_count

    n_run = 1  # current run

    # ---
    p = np.zeros((nx_count * ny_count))
    pos = np.zeros((nx_count * ny_count, 4))
    for m in range(0, nx_count * nx_step, nx_step):
        for n in range(0, ny_count * ny_step, ny_step):
            nx_frm = m + 1
            ny_frm = n + 1
            nx_to = nx_frm + temp_size - 1
            ny_to = ny_frm + temp_size - 1
            block_img = clip_img[ny_frm:ny_to, nx_frm:nx_to]

            # correlation coefficient
            cf = np.corrcoef(img_tmp.ravel(), block_img.ravel())
            p[n_run - 1] = cf[0, 1]
            pos[n_run - 1, :] = np.array([nx_frm, ny_frm, temp_size, temp_size])
            n_run += 1

    pmax, n = np.max(p), np.flip(np.argmax(p))
    win = pos[n, :]
    nx_frm = win[0]
    ny_frm = win[1]

    nx_pos = nx_frm + temp_size * 0.5 + n_current_x - width / 2
    ny_pos = ny_frm + temp_size * 0.5 + n_current_y - height / 2

    return nx_pos, ny_pos
