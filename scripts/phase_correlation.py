import numpy as np
from numpy.fft import fft2, fftshift, ifft2, ifftshift


def phase_correlation(img_tmp, img_src, n_current_x, n_current_y, temp_size):
    A = img_tmp[:temp_size, :temp_size].astype(np.double)
    B = img_src[:temp_size, :temp_size].astype(np.double)

    f = np.hamming(temp_size).reshape(-1, 1)
    f = f * f.conj().T

    a = fftshift(fft2(f * A))
    b = fftshift(fft2(f * B))

    m, temp_size = a.shape
    s = np.abs(a * np.conj(b))
    s[s == 0] = 1

    c = (a * np.conj(b)) / s

    invertfft_c = np.real_if_close(ifftshift(ifft2(c)))
    m, i = np.max(invertfft_c, 0), np.argmax(invertfft_c, 0)
    n, j = np.max(m, 0), np.argmax(m, 0)
    x_shift = j - temp_size / 2
    y_shift = i[j] - temp_size / 2

    nx_pos = n_current_x - x_shift
    ny_pos = n_current_y - y_shift

    return nx_pos, ny_pos, n
