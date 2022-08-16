//
// Created by Nic on 13/03/2022.
//

#ifndef REALTIME3D_WAYPOINTS_HPP
#define REALTIME3D_WAYPOINTS_HPP

#include <QList>
#include <QVector3D>
#include <QFileDialog>
#include <QTextStream>

namespace waypointer::io {
    QList<QVector3D> readWaypoints(const QString &path);
};


#endif //REALTIME3D_WAYPOINTS_HPP
