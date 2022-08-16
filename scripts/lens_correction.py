import lensfunpy
from pathlib import Path
from cv2 import cv2 as cv
import imageio


# ALL = -1
# DISTORTION = 8
# GEOMETRY = 16
# SCALE = 32
# TCA = 1
# VIGNETTING = 2

def correct_distortion(image_path,
                       camera_maker, camera_model,
                       lens_maker, lens_model,
                       focal_length, aperture, distance_to_subject,
                       inter_method: int):
    """

    :param image_path: Path to image file
    :param camera_maker: Name of camera maker
    :param camera_model: Name of camera model
    :param lens_maker: Name of lens maker
    :param lens_model: Name of lens model
    :param focal_length: The focal length in mm at which the image was taken.
    :param aperture: The aperture (f-number) at which the image was taken.
    :param distance_to_subject: The approximate focus distance in meters (distance > 0)
    :param inter_method: The method of interpolation used to remap.
    :return:
    """
    db_path = ".\\lensfunDB\\db"
    db_path = Path(db_path)
    if not db_path.exists():
        print('Camera database could not be loaded.')
        return
    xml_paths = [p.as_posix() for p in db_path.glob('*.xml')]
    db = lensfunpy.Database(paths=xml_paths, load_bundled=False)

    try:
        cam = db.find_cameras(camera_maker, camera_model)[0]
    except IndexError as e:
        print("Could not find Camera:", "[maker]", camera_maker, "[model]", camera_model)

    try:
        lens = db.find_lenses(cam, lens_maker, lens_model)[0]
    except IndexError as e:
        print("Could not find lens:", "[maker]", lens_maker, "[model]", lens_model)
        return

    image_path_ = Path(image_path)

    im = imageio.imread(image_path_.as_posix())

    try:
        im = cv.cvtColor(im, cv.COLOR_RGB2BGR)
    except:
        pass

    height, width = im.shape[0], im.shape[1]

    mod = lensfunpy.Modifier(lens, cam.crop_factor, width, height)
    mod.initialize(focal_length, aperture, distance_to_subject)

    print(f"Correcting distortion using: Camera [maker]{cam.maker} [model]{cam.model} "
          f"Lens [maker]{lens.maker} [model]{lens.model}")
    undist_coords = mod.apply_geometry_distortion()

    # bicubic interpolation
    im_undistorted = cv.remap(src=im, map1=undist_coords, map2=None, interpolation=inter_method)
    corrected_image_path = image_path_.with_name(image_path_.stem + '_corrected' + image_path_.suffix)
    if not cv.imwrite(corrected_image_path.as_posix(), im_undistorted):
        print("Correction could not be saved.")


if __name__ == '__main__':
    correct_distortion(
        "C:\\Tiger\\Data\\Input\\kentmere\\DJI_0301.JPG",
        "DJI", "Phantom 4 Pro",
        "DJI", "FC330 (RAW1)",
        focal_length=20,
        aperture=2.8,
        distance_to_subject=120,
        inter_method=cv.INTER_CUBIC
    )

    # nearest neighbor
    # im_undistorted = cv.remap(src=im, map1=undist_coords, map2=None,
    #                            interpolation=cv.INTER_NEAREST)
    # corrected_image_path = image_path.with_name(image_path.stem + '_nn_corrected' + image_path.suffix)
    # cv.imwrite(corrected_image_path.as_posix(), im_undistorted)

    # bilinear interpolation
    # im_undistorted = cv.remap(src=im, map1=undist_coords, map2=None,
    #                           interpolation=cv.INTER_LINEAR)
    # corrected_image_path = image_path.with_name(image_path.stem + '_bilinear_corrected' + image_path.suffix)
    # cv.imwrite(corrected_image_path.as_posix(), im_undistorted)

    # resampling using pixel area relation.
    # It may be a preferred method for image decimation, as it gives moire'-free results.
    # But when the image is zoomed, it is similar to the INTER_NEAREST method.

    # Lanczos interpolation over 8x8 neighborhood
    # im_undistorted = cv.remap(src=im, map1=undist_coords, map2=None,
    #                           interpolation=cv.INTER_LANCZOS4)
    # corrected_image_path = image_path.with_name(image_path.stem + '_lanczos4_corrected' + image_path.suffix)
    # cv.imwrite(corrected_image_path.as_posix(), im_undistorted)

    # Bit exact bilinear interpolation is not supported by remap

    # Bit exact nearest neighbor interpolation.
    # This will produce same results as the nearest neighbor method in PIL, scikit-image or Matlab.
    # im_undistorted = cv.remap(src=im, map1=undist_coords, map2=None,
    #                           interpolation=cv.INTER_NEAREST_EXACT)
    # corrected_image_path = image_path.with_name(image_path.stem + '_nnExact_corrected' + image_path.suffix)
    # cv.imwrite(corrected_image_path.as_posix(), im_undistorted)

    # mask for interpolation codes

    # flag, fills all of the destination image pixels.
    # If some of them correspond to outliers in the source image, they are set to zero
