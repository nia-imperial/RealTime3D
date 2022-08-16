//
// Created by arnoldn on 20/05/2021.
//

#include "imagecutter.h"
#include "../utility/messages.hpp"
#include <QImage>
#include <QPainterPath>
#include <QGraphicsObject>

void ImageCutter::setOutPrefix(const QString &prefix) {
    outPrefix = prefix;
}

std::tuple<fs::path, fs::path>
ImageCutter::imageCut(const QString &refImagePath, const QString &secImagePath, int i, int j) {
    QImageReader reader1(refImagePath);
    QImageReader reader2(secImagePath);

    if (!reader1.canRead()) {
        qWarning() << "Could not read image:" << refImagePath;
        return std::tuple("", "");
    }
    if (!reader2.canRead()) {
        qWarning() << "Could not read image:" << secImagePath;
        return std::tuple("", "");
    }

    cropImaPath.make_preferred();
    if (!cropDir.exists()) {
        create_directories(cropImaPath);
    }

    reader1.setAutoTransform(true);
    reader2.setAutoTransform(true);

    QImage refImage = reader1.read();
    QImage secImage = reader2.read();

    int newWidth, newHeight;
    newWidth = refImage.width() - abs(xShift);
    newHeight = refImage.height() - abs(yShift);
    if (newWidth < 0 || newHeight < 0)
        Messages::warning_msg(nullptr, tr("The frame shift you entered is too large!"));

    auto rectRef = refImage.rect();
    auto rectRefObj = scene.addRect(rectRef);

    auto rectSec = secImage.rect();
    auto rectSecObj = scene.addRect(rectSec);

    rectSec.translate(xShift, yShift);
    rectSecObj->moveBy(xShift, yShift);

    auto newRefRect = rectRef.intersected(rectSec);
    auto newRefRectObj = scene.addRect(newRefRect);

    auto newSecRect = rectSec.intersected(rectRef);
    auto newSecRectObj = scene.addRect(newSecRect);

    newRefRect = newRefRectObj->mapRectToItem(rectRefObj, newRefRectObj->rect()).toRect();
    newSecRect = newSecRectObj->mapRectToItem(rectSecObj, newSecRectObj->rect()).toRect();
    scene.clear();


    fs::path refOutName{QString("%1_%2_%3_Reference").arg(outPrefix).arg(i).arg(j).toStdString()};
    refOutName.replace_extension(".jpg");

    fs::path tarOutName(QString("%1_%2_%3_Secondary").arg(outPrefix).arg(i).arg(j).toStdString());
    tarOutName.replace_extension(".jpg");

    /// Crop_Pic dir path
    auto refTmpPath = cropImaPath / refOutName;
    refTmpPath.make_preferred();
    /// output path
    auto refOutPath = outPath / refOutName;
    refOutPath.make_preferred();

    auto tarTmpPath = cropImaPath / tarOutName;
    tarTmpPath.make_preferred();
    auto tarOutPath = outPath / tarOutName;
    tarOutPath.make_preferred();

    if (not QFileInfo::exists(QString::fromStdString(outPath.string())))
        QDir().mkpath(QString::fromStdString(outPath.string()));

    QImage copyRefImage = refImage.copy(newRefRect);
    copyRefImage.save(QString::fromStdString(refTmpPath.string()));
    if (outPath != refTmpPath)
        copyRefImage.save(QString::fromStdString(refOutPath.string()));
    cropImgList.append(refTmpPath);


    QImage copySecImage = secImage.copy(newSecRect);
    copySecImage.save(QString::fromStdString(tarTmpPath.string()));
    if (outPath != tarTmpPath)
        copySecImage.save(QString::fromStdString(tarOutPath.string()));
    cropImgList.append(tarTmpPath);


    return {refTmpPath, tarTmpPath};
}

void ImageCutter::setOutPath(const QString &path) {
    outPath = path.toStdString();
}

void ImageCutter::setTmpCropPath(const QString &path) {
    cropImaPath = path.toStdString();
    cropDir.setPath(path);
}

ImageCutter::ImageCutter(QObject *parent) : QObject(parent) {
    setObjectName(QLatin1String("ImageCutter"));
    setTmpCropPath(QStringLiteral("/Tiger/Data/Input/Crop_Pic"));
}

