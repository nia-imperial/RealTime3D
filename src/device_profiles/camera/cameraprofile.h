//
// Created by Nic on 28/02/2022.
//

#ifndef REALTIME3D_CAMERAPROFILE_H
#define REALTIME3D_CAMERAPROFILE_H

#include <QWidget>
#include <QStringListModel>
#include "../DeviceProfile.h"


QT_BEGIN_NAMESPACE
namespace Ui { class CameraProfile; }
QT_END_NAMESPACE

class CameraProfile : public DeviceProfile {
Q_OBJECT

public:
    explicit CameraProfile(const std::shared_ptr<lfDatabase> &ltd, QWidget *parent = nullptr);

    ~CameraProfile() override;

Q_SIGNALS:

    void imageWidthChanged(double px);

    void imageHeightChanged(double px);

    void sensorHeightChanged(double mm);

    void sensorWidthChanged(double mm);

    void intervalChanged(double s);

    void isoChanged(int iso);

    void shutterSpeedChanged(int s);

    void profileIndexChanged(const QString &cameraName,
                             const QString &cameraMaker);

    void profileChosen(const QString &cameraName,
                       const QString &cameraMaker,
                       const QString &cameraMount);

public Q_SLOTS:

    int getImageWidth();

    void setImageWidth(double d);

    int getImageHeight();

    void setImageHeight(double d);

    void populateProfileList() override;

    void loadProfile() override;

    void searchAndSetCamera(const QString &cameraName);

    void setImageInterval(double val);

    double getImageInterval();

    void setShutterSpeed(int val);

    int getShutterSpeed();

    double getSensorWidth();

    void setSensorWidth(double val);

    double getSensorHeight();

    void setSensorHeight(double val);

    double getCropFactor();

    void setCropFactor(double val);

    int getISO();

    void setISO(int val);

    std::tuple<QString, QString> currentTextToCamera();

private:

    static QString createMakerModelStr(const QString &maker, const QString &model, const QString &variant);

    Ui::CameraProfile *ui;

};


#endif //REALTIME3D_CAMERAPROFILE_H
