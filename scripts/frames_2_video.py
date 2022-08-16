from pathlib import Path

from cv2 import cv2

input_path = Path("C:\\Users\\arnoldn\\Documents\\storage\\kentmere_compiled_corrected\\36")

input_corrected_no_mat = Path("C:\\Users\\arnoldn\\Documents\\storage\\kentmere_corrected_no_mat")
input_corrected_20_mat = Path("C:\\Users\\arnoldn\\Documents\\storage\\kentmere_corrected_20_mat")
input_corrected_60_mat = Path("C:\\Users\\arnoldn\\Documents\\storage\\kentmere_corrected_60_mat")
input_corrected_frames = Path("C:\\Users\\arnoldn\\Documents\\storage\\kentmere_corrected_frames")

input_distorted_no_mat = Path("C:\\Users\\arnoldn\\Documents\\storage\\kentmere_distorted_no_mat")
input_distorted_20_mat = Path("C:\\Users\\arnoldn\\Documents\\storage\\kentmere_distorted_20_mat")
input_distorted_60_mat = Path("C:\\Users\\arnoldn\\Documents\\storage\\kentmere_distorted_60_mat")
input_distorted_frames = Path("C:\\Users\\arnoldn\\Documents\\storage\\kentmere_distorted_frames")

corrected_ = [input_corrected_no_mat, input_corrected_20_mat, input_corrected_60_mat, input_corrected_frames]
distorted_ = [input_distorted_no_mat, input_distorted_20_mat, input_distorted_60_mat, input_distorted_frames]

size = (0, 0)

corrected_roll = []
for j in sorted(input_path.glob("*.jpg")):
    img = cv2.imread(str(j))
    height, width, layers = img.shape
    size = (width, height)
    corrected_roll.append(img)

output_path = input_path.with_name("comparison36" + ".avi")
out = cv2.VideoWriter(output_path.as_posix(), cv2.VideoWriter_fourcc(*'DIVX'), 24, size)
for f in corrected_roll:
    for i in range(24):
        out.write(f)
out.release()

