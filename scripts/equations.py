from typing import Tuple, Optional
import math

pi = math.pi
ResolutionType = Tuple[int, int]


class Resolution:
    def __init__(self, width: int, height: int):
        self.width = width
        """image x size pixels"""
        self.height = height
        """image y size pixels"""

    @property
    def x(self):
        return self.width

    @property
    def y(self):
        return self.height

    @property
    def diagonal(self):
        """image diagonal in pixels"""
        return math.hypot(self.width, self.height)


class Camera:
    diag_equiv = math.hypot(24, 36)
    """diagonal of full frame sensor"""

    def __init__(self,
                 focal_length: float,
                 image_intervals: float,
                 shutter_speed: float,
                 resolution: ResolutionType):
        self.focal_length = focal_length
        """camera's lens focal length"""
        self.interval = image_intervals
        """number of seconds between images"""
        self.res = Resolution(*resolution)
        """image resolution of camera"""
        self.shutter_speed = shutter_speed

    @property
    def view_angle(self):  # d_degrees
        """diagonal view angle in degrees"""
        return 180 * 2 * math.atan(self.diag_equiv / (2 * self.focal_length)) / pi

    @property
    def angular_resolution(self):
        return cam.view_angle / cam.res.diagonal


class Aircraft:
    def __init__(self, speed: float, height: float):
        self.speed = speed
        """flight speed"""
        self.height = height
        """height above ground"""
        self.camera: Optional[Camera] = None

    def set_camera(self, camera: Camera):
        self.camera = camera


def calc_diagonal_res_metres(view_angle, uav_height):  # d_metres
    """image diagonal metres"""
    # return (uav_height * Camera.diag_equiv) / (2 * focal_len)
    return uav_height * math.tan(pi * view_angle / (2 * 180))


def calc_interval_metres(speed, camera_interval):
    """Calculate baseline between images"""
    return speed * camera_interval


def calc_interval_seconds(speed, camera_distance):
    """Calculate seconds between images, given the uav speed"""
    return camera_distance / speed


def calc_image_size_x(res_width, focal_len, diag_px, height):
    """number of metres covered by an images width"""
    return res_width * calc_diagonal_res_metres(focal_len, height) / diag_px


def calc_image_size_y(res_height, focal_len, diag_px, height):
    """number of metres covered by an images height"""
    return res_height * calc_diagonal_res_metres(focal_len, height) / diag_px


def calc_image_overlap_metres(res_height, focal_len, diag_px, height, speed, camera_interval):
    """Calculate amount of overlapping ground distance"""
    image_y_size_metres = calc_image_size_y(res_height, focal_len, diag_px, height)
    image_interval_metres = calc_interval_metres(speed, camera_interval)
    return image_y_size_metres - image_interval_metres


def calc_image_overlap_percent(height, focal_len, diag_px, camera_interval):
    """percentage of overlap between two image pairs"""
    f = Camera.diag_equiv
    h = height
    m = focal_len  # l
    d = diag_px
    n = camera_interval
    """seconds between images"""

    # alt1:
    # image_y_size_metres = calc_image_size_y(res_height, focal_len, diag_px, height)
    # image_overlap_metres = calc_image_overlap_metres(res_height, focal_len, diag_px, height, speed, camera_interval)
    # image_overlap_percent = 100 * image_overlap_metres / image_y_size_metres

    # alt2: 50 * (((f * (h ** 2)) / (d * m * n)) - 2)
    return (50 * f * (h ** 2)) / (d * m * n) - 100


def calc_motion_blur_cm(speed, shutter_speed):
    return 100 * speed / shutter_speed


def calc_ground_px_size_cm(height, focal_len, diag_px):
    return 100 * calc_diagonal_res_metres(focal_len, height) / diag_px


def calc_motion_blur_px(speed, shutter_speed, height, focal_len, diag_px):
    # alt 1: (100 * speed / shutter_speed) / calc_ground_px_size_cm(height, focal_len, diag_px)
    return (diag_px * speed) / (shutter_speed * calc_diagonal_res_metres(focal_len, height))


uav = Aircraft(speed=8, height=121)
cam = Camera(focal_length=24.0, image_intervals=5, shutter_speed=1000, resolution=(5472, 3648))
uav.set_camera(cam)

# good
diagonal_view_angle = cam.view_angle
motion_blur_cm = calc_motion_blur_cm(uav.speed, cam.shutter_speed)
image_interval_m = calc_interval_metres(uav.speed, cam.interval)
angular_resolution = cam.angular_resolution

ground_pixel_size_cm = calc_ground_px_size_cm(uav.height, cam.view_angle, cam.res.diagonal)

image_diagonal_degrees = 180 * 2 * math.atan(Camera.diag_equiv / (2 * cam.focal_length)) / pi
image_diagonal_metres = uav.height * math.tan(pi * image_diagonal_degrees / (2 * 180))
ground_pixel_size_cm1 = 100 * image_diagonal_metres / cam.res.diagonal

motion_blur_pixels = calc_motion_blur_px(uav.speed, cam.shutter_speed,
                                         uav.height, cam.focal_length, cam.res.diagonal)
image_y_size_m = calc_image_size_y(cam.res.height, cam.focal_length, cam.res.diagonal, uav.height)
image_x_size_m = calc_image_size_x(cam.res.width, cam.focal_length, cam.res.diagonal, uav.height)  # todo fix
image_overlap_m = calc_image_overlap_metres(cam.res.height, cam.focal_length, cam.res.diagonal,
                                            uav.height, uav.speed, cam.interval)  # todo fix
image_overlap_percent = calc_image_overlap_percent(cam.res.height, cam.focal_length, cam.res.diagonal,
                                                   cam.interval)  # todo fix
