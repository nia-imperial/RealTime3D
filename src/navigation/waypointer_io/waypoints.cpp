//
// Created by Nic on 21/03/2022.
//
#include "waypoints.hpp"
#include "../../settings/path_settings/pathsettings.h"


QList <QVector3D> waypointer::io::readWaypoints(const QString &path) {
    QString file_path;
    if (path.isNull() or path.isEmpty())
        file_path = QFileDialog::getOpenFileName(
                nullptr,
                "Open Waypoints",
                PathSettings::default_tigerDir(),
                "Waypoint Files (*.txt *.csv *.tsv)"
        );
    if (file_path.isEmpty() or file_path.isNull()) return {};

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QList<QVector3D> data;
    QString delimiter(" ");
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains(","))
            delimiter = ",";
        auto parts = line.split(delimiter);
        data.append(QVector3D(parts.value(0).toFloat(),
                              parts.value(1).toFloat(),
                              parts.value(2).toFloat()
        ));
        delimiter = " ";
    }
    return data;
}

