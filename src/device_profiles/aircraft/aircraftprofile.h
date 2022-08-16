//
// Created by Nic on 28/02/2022.
//

#ifndef REALTIME3D_AIRCRAFTPROFILE_H
#define REALTIME3D_AIRCRAFTPROFILE_H

#include <QWidget>
#include "../DeviceProfile.h"


QT_BEGIN_NAMESPACE
namespace Ui { class AircraftProfile; }
QT_END_NAMESPACE

class AircraftProfile : public DeviceProfile {
Q_OBJECT

public:
    explicit AircraftProfile(const std::shared_ptr<lfDatabase>& ltd, QWidget *parent = nullptr);

    ~AircraftProfile() override;

    double getFlightSpeed();

    void setFlightSpeed(double ms);

    double getHeight();

    void setHeight(double m);

Q_SIGNALS:
    void flightSpeedChanged(double d);
    void flightHeightChanged(double d);
    void profileChosen(const QString& name);

public Q_SLOTS:
    void populateProfileList() override;

    void loadProfile() override;

    void saveProfile();

    void setProfileByName(const QString &name);


private:
    Ui::AircraftProfile *ui;
    QString currentProfilePath;

    static QJsonObject readProfileFile(const QString &filePath);
};


#endif //REALTIME3D_AIRCRAFTPROFILE_H
