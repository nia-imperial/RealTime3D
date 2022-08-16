//
// Created by arnoldn on 21/03/2022.
//
#include "pyscriptcaller.h"
#include <QPixmap>
#include <QDebug>
#include <pybind11/embed.h> // everything needed for embedding

namespace py = pybind11;
using namespace py::literals;

std::string pycall::perspectiveMatching(const std::string &outputDir, const std::string &nadirImagePath,
                                        const std::string &obliqueImagePath, bool saveImages) {
    py::gil_scoped_acquire acquire;

    try {
        auto match_images = py::module_::import("scripts.image_matching_v2").attr("match_images");
        auto matchedImagePath = match_images(outputDir, nadirImagePath, obliqueImagePath);
        return py::cast<std::string>(matchedImagePath);
    } catch (py::error_already_set &e) {
        qDebug() << e.what();
    }
    return {};
}

std::string pycall::makeFramesDir(const std::string &videoFilePath) {
    py::gil_scoped_acquire acquire;
    try {
        auto framesDir = py::module_::import("scripts.video2frames").attr("make_frames_dir");
        return py::cast<std::string>(framesDir(videoFilePath));
    } catch (py::error_already_set &e) {
        qDebug() << e.what();
    }
    return {};
}

void pycall::video2frames(const std::string &videoFilePath, const std::string &framesDir, double frameRate) {
    py::gil_scoped_acquire acquire;
    try {
        auto video_2_frames = py::module_::import("scripts.video2frames").attr("video2frames");
        video_2_frames(videoFilePath, framesDir, frameRate);
    } catch (py::error_already_set &e) {
        qDebug() << e.what();
    }
}

void pycall::video2frames(const std::string &videoFilePath,
                          const std::string &framesDir,
                          double frameRate, int maximum) {
    py::gil_scoped_acquire acquire;
    try {
        auto video_2_frames = py::module_::import("scripts.video2frames").attr("video2frames");
        video_2_frames(videoFilePath, framesDir, frameRate, maximum);
    } catch (py::error_already_set &e) {
        qDebug() << e.what();
    }
}

// extra: pass image data directly
std::tuple<int, int>
pycall::matchAerialToMap(const std::string &aerialImage, const std::string &baseMap, int x, int y) {
    py::gil_scoped_acquire acquire;

    try {
        auto aerialMatching = py::module_::import("scripts.image_processing").attr("match_aerial_to_map");
        py::tuple pos = aerialMatching(aerialImage, baseMap, x, y);
        return {py::cast<int>(pos[0]), py::cast<int>(pos[1])};
    } catch (py::error_already_set &e) {
        qDebug() << e.what();
    }

    return {};
}

bool pycall::dat2tiff_dir(const std::string &dirname) {
    py::gil_scoped_acquire acquire;
    qDebug() << "Converting .dat files in Folder to .tiff";
    try {
        auto dat2tiff_dir = py::module_::import("scripts.dat2tiff").attr("dir_dat2tiff");
        dat2tiff_dir(dirname);
        return true;
    } catch (py::error_already_set &e) {
        qDebug() << "dir_dat2tiff from dat2tiff.pyc could not load." << e.what();
        return false;
    }
}

bool pycall::dat2tiff(const std::string &filename) {
    py::gil_scoped_acquire acquire;
    qInfo() << "Converting .dat to .tiff";
    try {
        auto dat2tiff = py::module_::import("scripts.dat2tiff").attr("dat2tiff");
        dat2tiff(filename);
        return true;
    } catch (py::error_already_set &e) {
        qDebug() << "dat2tiff from dat2tiff.pyc could not load." << e.what();
        return false;
    }
}

void
pycall::correctDistortion(const std::string &imagePath,
                          const std::string &cameraMaker,
                          const std::string &cameraModel,
                          const std::string &lensMaker,
                          const std::string &lensModel,
                          double focalLength,
                          double aperture,
                          double distanceToSubject,
                          int interMethod) {
    py::gil_scoped_acquire acquire;

    try {
        auto correctDistortion = py::module_::import("scripts.lens_correction").attr("correct_distortion");
        correctDistortion(imagePath, cameraMaker, cameraModel, lensMaker, lensModel,
                          focalLength, aperture, distanceToSubject, interMethod);

    } catch (py::error_already_set &e) {
        qDebug() << e.what();
    }
}


