//
// Created by Nic on 28/02/2022.
//

#ifndef REALTIME3D_OVERVIEWPROFILE_H
#define REALTIME3D_OVERVIEWPROFILE_H

#include <QWidget>
#include <lensfun/lensfun.h>
#include "aircraft/aircraftprofile.h"
#include "camera/cameraprofile.h"
#include "lens/lensprofile.h"


QT_BEGIN_NAMESPACE
namespace Ui { class OverviewProfile; }
QT_END_NAMESPACE

class OverviewProfile : public QWidget {
Q_OBJECT

public:
    AircraftProfile *aircraftProfile;
    CameraProfile *cameraProfile;
    LensProfile *lensProfile;
    explicit OverviewProfile(const QString &dbPath, QWidget *parent = nullptr);

    ~OverviewProfile() override;

Q_SIGNALS:
    void dbPathUpdated();

public Q_SLOTS:
    void setDB_path(const QString& path);

    void saveProjectFile(const QString &saveFilePath, const QString &projectOutputPath = QString());

    QJsonObject loadProjectFile(const QString &filePath = QString());

    bool isAircraftSet();

    bool isCameraSet();

    bool isLensSet();

private:
    std::shared_ptr<lfDatabase> lensfunDB;
    Ui::OverviewProfile *ui;
};


#endif //REALTIME3D_OVERVIEWPROFILE_H
