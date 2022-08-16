import sys
from pathlib import Path
from typing import Union, Optional
from time import perf_counter
import numpy as np
import cv2 as cv


np.set_printoptions(suppress=True)

# type defs
PathStr = Union[Path, str]


class ImageMatcher:
    matcher: cv.FlannBasedMatcher
    sift: cv.SIFT
    inter_methods = {'nearest': cv.INTER_NEAREST,
                     'bilinear': cv.INTER_LINEAR,
                     'bicubic': cv.INTER_CUBIC}

    def __init__(self, output_dir: Optional[PathStr] = None,
                 interpolation_method='bicubic',
                 min_matches: int = 10,
                 confidence: float = 0.99,
                 save_images=True):

        if output_dir is None:
            output_dir = Path.cwd().expanduser().joinpath('output')
        print(output_dir)
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)

        try:
            self.inter_method = self.inter_methods[interpolation_method]
        except KeyError as e:
            print('Interpolation method', interpolation_method,
                  'not recognised. Supported methods are:', self.inter_method.keys())
            raise e

        self.sift = cv.SIFT_create()

        if not (0 <= confidence <= 1):
            raise ValueError(f'Confidence must be float between 0 and 1.')
        self.confidence = confidence

        self.min_matches = min_matches
        self.matcher = cv.FlannBasedMatcher(dict(algorithm=1, trees=5),
                                            dict(checks=50))
        self.test_ratio = 0.7

        # aesthetic settings
        self.color_converter = cv.COLOR_BGR2GRAY
        self.border_line_color: int = 255
        self.match_line_color: tuple = (0, 255, 0)

        self.save_images = save_images

    def read_image(self, source: PathStr) -> np.ndarray:
        """Read images from source path and convert to grayscale"""
        source = Path(source).expanduser().resolve()
        if not source.exists():
            raise FileExistsError('Could not find file:', source)
        # read image
        img = cv.imread(str(source))
        # convert to grayscale
        img = cv.cvtColor(img, self.color_converter)
        return img

    def match(self, nadir: PathStr, oblique: PathStr) -> np.ndarray:
        """ Matches image with nadir view to image with oblique view"""
        s = perf_counter()
        # nadir view image
        img1 = self.read_image(nadir)
        # oblique view image
        img2 = self.read_image(oblique)

        h, w = img2.shape
        print('Oblique view image size:'.ljust(30), f"{h}x{w}")

        # resize images
        img1 = cv.resize(img1, (w, h), interpolation=self.inter_method)

        # get keypoints and descriptors
        kp1, desc1 = self.sift.detectAndCompute(img1, None)
        kp2, desc2 = self.sift.detectAndCompute(img2, None)

        img_kp1 = cv.drawKeypoints(img1, kp1, img1)
        img_kp2 = cv.drawKeypoints(img2, kp2, img2)

        if self.save_images:
            self.write_image(img_kp1, 'key1.png')
            self.write_image(img_kp2, 'key2.png')

        matches = self.matcher.knnMatch(queryDescriptors=desc1,
                                        trainDescriptors=desc2,
                                        k=2)

        # ratio test (see Lowe's paper)
        good = [m for m, n in matches if m.distance < self.test_ratio * n.distance]

        if len(good) < self.min_matches:
            raise ValueError(f"Not enough matches were found: {len(good)}/{self.min_matches}")

        src_pts = np.float32([kp1[m.queryIdx].pt for m in good]).reshape((-1, 1, 2))
        dst_pts = np.float32([kp2[m.trainIdx].pt for m in good]).reshape((-1, 1, 2))

        retval, inlier_mask = cv.findHomography(src_pts, dst_pts,
                                                method=cv.RANSAC,
                                                ransacReprojThreshold=5.0,
                                                confidence=self.confidence)

        pts = np.array([[0, 0], [0, h - 1], [w - 1, h - 1], [w - 1, 0]]).reshape((-1, 1, 2)).astype(np.float32)
        dst = cv.perspectiveTransform(pts, retval)

        # oblique with borders image
        if self.save_images:
            img2 = cv.polylines(img2.copy(), [dst.astype(np.int32)],
                                isClosed=True,
                                color=255,
                                thickness=3)
            self.write_image(img2, 'oblique_with_borders.png')

        img1 = cv.cvtColor(img1, cv.COLOR_BGR2BGRA)
        img_out = cv.warpPerspective(img1, retval, (w, h),
                                     flags=self.inter_method)

        # nadir to oblique image
        img_out = cv.resize(img_out, (w, h),
                            interpolation=self.inter_method)

        # point matched image
        draw_params = dict(matchColor=self.match_line_color,
                           singlePointColor=None,
                           matchesMask=inlier_mask.ravel(),
                           flags=2)

        if self.save_images:
            img3 = cv.drawMatches(img1, kp1, img2, kp2, good, None, **draw_params)
            self.write_image(img3, 'point_matched.png')

        e = perf_counter()
        print(F'Process took {e - s:0.4f} seconds.', file=sys.stderr)
        return img_out

    def write_image(self, img: np.ndarray, filename: str):
        cv.imwrite(str(self.output_dir.joinpath(filename)), img)


def match_images(output_dir, nadir_image_path, oblique_image_path, save_images=False) -> str:
    image_matcher = ImageMatcher(output_dir=output_dir, save_images=save_images)
    out_image = image_matcher.match(nadir=nadir_image_path, oblique=oblique_image_path)

    out_image_path = Path(oblique_image_path).with_name(Path(oblique_image_path).stem + "matched.png")
    image_matcher.write_image(out_image, out_image_path.as_posix())
    return out_image_path.as_posix()
