//
// Created by Nic on 28/02/2022.
//

#ifndef REALTIME3D_DEVICEPROFILE_H
#define REALTIME3D_DEVICEPROFILE_H

#include <QWidget>
#include <lensfun/lensfun.h>
#include <QStringListModel>

class DeviceProfile : public QWidget {
Q_OBJECT
public:
    explicit DeviceProfile(const std::shared_ptr<lfDatabase>& ltd, QWidget *parent = nullptr);

    ~DeviceProfile() override = default;

Q_SIGNALS:
    void profileChanged();

public Q_SLOTS:

    virtual void loadProfile() = 0;

    virtual void populateProfileList() = 0;


protected:
    /// has value of "Choose Profile"
    QString def_index1;
    std::weak_ptr<lfDatabase> lensfunDB;

    const lfCamera ** findCameras(const QString &cameraName);
    const lfCamera ** findCameras(const QString &cameraName, const QString &cameraMaker);
    const lfCamera * findCamera(const QString &cameraName);
    const lfCamera * findCamera(const QString &cameraName, const QString &cameraMaker);

    const lfLens ** findLenses(const QString &lensName, const QString &lensMaker);
    const lfLens ** findLenses(const lfCamera *camera);
    const lfLens *findLens(const QString &lensName, const QString &lensMaker);
    const lfLens *findLens(const lfCamera *camera);
};


#endif //REALTIME3D_DEVICEPROFILE_H
