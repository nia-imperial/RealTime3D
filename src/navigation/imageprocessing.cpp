//
// Created by Nic on 20/03/2022.
// Kept for reference only, project was switched to use the opencv-python implementation
//

#include "imageprocessing.h"
#include "../settings/path_settings/pathsettings.h"
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>
#include <utility>
#include <QDir>
#include <iostream>
#include <QDebug>

#include <stdexcept>
#include <chrono>

ImageMatcher::ImageMatcher(QString outputDir,
                           bool saveImages,
                           cv::InterpolationFlags method,
                           int minMatches,
                           double confidence) :
        outputDir(std::move(outputDir)),
        sift(cv::SIFT::create()),
        confidence(confidence),
        saveImages(saveImages),
        minMatches(minMatches),
        interMethod(method) {
    if (this->outputDir.isNull() or this->outputDir.isEmpty())
        this->outputDir = QDir(PathSettings::default_tigerOutputDir()).filePath("PerspectiveMatches");
    // extra: add perspective matches to path settings
    QDir(this->outputDir).mkpath(this->outputDir);

    if (not(0 <= this->confidence && this->confidence <= 1))
        this->confidence = 0.99;

    const cv::Ptr<cv::flann::IndexParams> &indexParams = new cv::flann::KDTreeIndexParams(4);
    const cv::Ptr<cv::flann::SearchParams> &searchParams = new cv::flann::SearchParams(50);

    matcher = cv::makePtr<cv::FlannBasedMatcher>(cv::FlannBasedMatcher(indexParams, searchParams));
    colorConverter = cv::COLOR_BGR2GRAY;
    borderLineColor = 255;
    matchLineColor = {0, 255, 0};
}


cv::Mat ImageMatcher::readImage(const QString &path) {
    if (not QFileInfo::exists(path)) return {};

    auto file_path = QDir::toNativeSeparators(path).toStdString();
    auto img = cv::imread(file_path);

    if (img.empty()) {
        throw std::runtime_error("\"!!! image not found\"");
    }

    cv::cvtColor(img, img, colorConverter);

    return img;
}

QString ImageMatcher::writeImage(const cv::Mat &img, const QString &filename) {
    auto writePath = QDir(outputDir).filePath(filename);
    cv::imwrite(writePath.toStdString(), img);
    return writePath;
}

cv::Mat ImageMatcher::match(const QString &nadirPath, const QString &obliquePath) {
    auto start = std::chrono::steady_clock::now();
    /// image 1
    auto imgNadir = readImage(nadirPath);
    /// image 2
    auto imgOblique = readImage(obliquePath);

    auto h = imgOblique.size().height;
    auto w = imgOblique.size().width;
//    auto img2Size = imgOblique.size();

    // resize images
    cv::resize(imgNadir, imgNadir, {w, h}, interMethod);


    // get keypoints and descriptors
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat desc1, desc2;
    sift->detectAndCompute(imgNadir, cv::noArray(), kp1, desc1);
    sift->detectAndCompute(imgOblique, cv::noArray(), kp2, desc2);

    cv::Mat imgKP1;
    cv::Mat imgKP2;
    cv::drawKeypoints(imgNadir, kp1, imgKP1);
    cv::drawKeypoints(imgOblique, kp2, imgKP2);
    if (saveImages) {
        writeImage(imgKP1, "key1.png");
        writeImage(imgKP2, "key2.png");
    }

    std::vector<std::vector<cv::DMatch>> matches;
    matcher->knnMatch(desc1, desc2, matches, 2);

    //-- Filter matches using the Lowe's ratio test
    const float ratio_thresh = 0.7f;
    std::vector<cv::DMatch> good_matches;
    for (auto &match: matches)
        if (match[0].distance < ratio_thresh * match[1].distance)
            good_matches.push_back(match[0]);

    // Check if too few matches are found
    if (good_matches.size() <= minMatches) {
        std::cout << "Error: too few matches were found" << std::endl;
        return {};
    }

    std::vector<cv::Point2f> srcPts, dstPts;
    for (auto &goodMatch: good_matches) {
        // Determine keypoints from the matches
        srcPts.push_back(kp1[goodMatch.queryIdx].pt);
        dstPts.push_back(kp2[goodMatch.trainIdx].pt);
    }

    cv::Mat inlierMask;
    cv::Mat H = findHomography(srcPts, dstPts,
                               cv::RANSAC,
                               5.0,
                               inlierMask,
                               2000,
                               confidence);

    if (saveImages) {
        std::vector<cv::Point2f> obj_corners(4);
        obj_corners[0] = cv::Point2f(0, 0);
        obj_corners[1] = cv::Point2f(0, (float) h - 1);
        obj_corners[2] = cv::Point2f((float) w - 1, (float) h - 1);
        obj_corners[3] = cv::Point2f((float) w - 1, 0);

        std::vector<cv::Point2f> scene_corners(4);
        perspectiveTransform(obj_corners, scene_corners, H);
        auto im2copy = imgOblique.clone();
        std::vector<cv::Point> pt;
        for (auto &scene_corner: scene_corners) {
            pt.push_back(scene_corner);
        }
        cv::polylines(im2copy, pt, true, 255, 3);
        writeImage(im2copy, "oblique_with_borders.png");
    }

    cv::cvtColor(imgNadir, imgNadir, cv::COLOR_BGR2BGRA);

    cv::Mat imgOut;
    cv::warpPerspective(imgNadir, imgOut, H, {w, h});

    cv::resize(imgOut, imgOut, {w, h}, interMethod);

    if (saveImages) {
        // point matched image
        cv::Mat img3;
        cv::drawMatches(imgNadir, kp1, imgOblique, kp2, good_matches,
                        img3,
                        matchLineColor,
                        cv::Scalar::all(-1),
                        inlierMask,
                        cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
        writeImage(img3, "point_matched.png");
    }
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    auto diff_sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    qDebug() << diff_sec.count() << "sec.";
    return imgOut;
}

QPixmap ImageMatcher::cvImageToQPixmap(cv::Mat cvImage) {
    cv::Mat rgbImage;
    cv::cvtColor(cvImage, rgbImage, cv::COLOR_BGR2RGB);
    auto pixmap = QPixmap();
    //extra
    return pixmap;
}

