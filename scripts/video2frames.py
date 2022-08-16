from pathlib import Path
from cv2 import cv2


def make_frames_dir(video_path):
    video_path = Path(video_path)
    frame_dir = video_path.with_name(video_path.stem + '_frames')
    frame_dir.mkdir(exist_ok=True)
    return str(frame_dir)


def video2frames(video_path, frame_dir, frame_rate=0.5, maximum=-1):
    video_path = Path(video_path)
    frame_dir = Path(frame_dir)
    vidcap = cv2.VideoCapture(video_path.as_posix())

    def get_frame(sec_, count_):
        vidcap.set(cv2.CAP_PROP_POS_MSEC, sec_ * 1000)
        has_frames, image = vidcap.read()

        if has_frames:
            frame_path = frame_dir / f"image_{count_}.jpg"
            cv2.imwrite(frame_path.as_posix(), image)  # save frame as JPG file
        return has_frames

    sec = 0
    count = 1
    success = get_frame(sec, count)
    while success and count != maximum:
        count = count + 1
        sec = sec + frame_rate
        sec = round(sec, 2)
        success = get_frame(sec, count)
