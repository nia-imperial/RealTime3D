//
// Created by arnoldn on 20/05/2021.
//

#ifndef REALTIME3D_IMAGECUTTER_H
#define REALTIME3D_IMAGECUTTER_H


#include <QWidget>
#include <QImageReader>
#include <QFileInfo>
#include <QDir>
#include <filesystem>
#include <QGraphicsScene>

namespace fs = std::filesystem;

/// Class for dynamically cropping images
class ImageCutter : public QObject {
Q_OBJECT
    /// the prefix used for the output
    QString outPrefix;
    /// the output path
    fs::path outPath;
    /// the path for saving crop images
    fs::path cropImaPath;
    /// the directory where cropped images are saved
    QDir cropDir;
    QGraphicsScene scene;

public:
    explicit ImageCutter(QObject *parent = nullptr);
    /// the images to be cropped
    QList<fs::path> cropImgList{};
    /// shift in the x axis
    int xShift{0};
    /// shift in the y axis
    int yShift{0};

public Q_SLOTS:
    /// set the prefix used in the image name
    void setOutPrefix(const QString &prefix);
    /// set the putput path
    void setOutPath(const QString &path);
    /// set the temporary directory for storing cropped images
    void setTmpCropPath(const QString &path);
    /// crop the overlapping areas of the images
    std::tuple<fs::path, fs::path> imageCut(const QString& refImagePath, const QString& secImagePath, int i, int j);

};


#endif //REALTIME3D_IMAGECUTTER_H


