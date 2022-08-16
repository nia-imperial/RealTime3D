//
// Created by Nic on 13/03/2022.
//

#ifndef REALTIME3D_IMAGES_HPP
#define REALTIME3D_IMAGES_HPP


#include <QPixmap>
#include <QFileDialog>

namespace waypointer::io {
    std::tuple<QPixmap, QString> readPhoto(const QString &path);
}

QString supportedImageFormats();
QString supportedVideoFormats();
QStringList supportedVideoNameFilters();


#endif //REALTIME3D_IMAGES_HPP
