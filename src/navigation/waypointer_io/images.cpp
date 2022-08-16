//
// Created by Nic on 21/03/2022.
//
#include "images.hpp"
#include "../../settings/path_settings/pathsettings.h"

std::tuple<QPixmap, QString> waypointer::io::readPhoto(const QString &path) {
    QString file_path;
    if (path.isNull() or path.isEmpty())
        file_path = QFileDialog::getOpenFileName(
                nullptr,
                QLatin1String("Open Base Image"),
                PathSettings::default_tigerDir(),
                supportedImageFormats()
        );
    else
        file_path = path;
    if (file_path.isEmpty() or file_path.isNull()) return {};
    return {QPixmap(file_path), file_path};
}

QString supportedImageFormats() {
    return QLatin1String("Image Files ("
                         "*.bmp *.dib "
                         "*.gif "
                         "*.heic *.heif "
                         "*.jpeg *.jpg "
                         "*.png "
                         "*.tiff *.tif"
                         "*.webp"
                         ")");
}

QString supportedVideoFormats() {
    return QLatin1String("Video Files ("
                         "*.mp4 *.m4v *.fv4"
                         "*.mpeg *.mpg *.mp2 *.mpe *.vob"
                         "*.qt *.mov *.moov *.qtvr"
                         "*.webm"
                         "*.avi *.avf *.divx"
                         "*.movie"
                         ")");
}

QStringList supportedVideoNameFilters() {
    return {"*.mp4", "*.m4v", "*.fv4",
            "*.mpeg", "*.mpg", "*.mp2", "*.mpe", "*.vob",
            "*.qt", "*.mov", "*.moov", "*.qtvr",
            "*.webm",
            "*.avi", "*.avf", "*.divx",
            "*.movie"};
}
