//
// Created by Nic on 20/03/2022.
// Kept for reference only, project was switched to use the opencv-python implementation
//

#ifndef REALTIME3D_IMAGEPROCESSING_H
#define REALTIME3D_IMAGEPROCESSING_H

#include <opencv2/opencv.hpp>
#include <QString>
#include <QPixmap>

class ImageMatcher {
    cv::Ptr<cv::FlannBasedMatcher> matcher;
    cv::Ptr<cv::SIFT> sift;
    QString outputDir;
    double confidence;
    bool saveImages;
    int minMatches;
    cv::InterpolationFlags interMethod;

    // aesthetic settings
    cv::ColorConversionCodes colorConverter;
    int borderLineColor;
    cv::Scalar matchLineColor;

public:

    explicit ImageMatcher(QString outputDir = "",
                          bool saveImages = false,
                          cv::InterpolationFlags method = cv::INTER_CUBIC,
                          int minMatches = 10,
                          double confidence = 0.99);

    cv::Mat readImage(const QString &path);

    QString writeImage(const cv::Mat& img, const QString &filename);

    cv::Mat match(const QString &nadirPath, const QString &obliquePath);

    QPixmap cvImageToQPixmap(cv::Mat cvImage);


};


#endif //REALTIME3D_IMAGEPROCESSING_H
