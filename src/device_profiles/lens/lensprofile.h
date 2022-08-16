//
// Created by Nic on 28/02/2022.
//

#ifndef REALTIME3D_LENSPROFILE_H
#define REALTIME3D_LENSPROFILE_H

#include <QWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include "../DeviceProfile.h"


QT_BEGIN_NAMESPACE
namespace Ui { class LensProfile; }
QT_END_NAMESPACE

class LensProfile : public DeviceProfile {
Q_OBJECT

public:
    explicit LensProfile(const std::shared_ptr<lfDatabase> &ltd, QWidget *parent = nullptr);

    ~LensProfile() override;

Q_SIGNALS:

    void focalLengthChanged(double mm);

    void apertureChanged(double f);

    void ffLengthChanged(double mm);

    void angleOfViewChanged(double d);

    void profileChosen(const QString &lensName, const QString &lensMaker);

public Q_SLOTS:

    void populateProfileList() override;

    /// Populate the profile list based on the camera passed to it
    void populateProfileList(const QString &cameraName, const QString &cameraMaker);

    void loadProfile() override;

    double getFocalLen();

    void setFocalLen(double d);

    double getAperture();

    void setAperture(double d);

    double get35FocalLen();

    void set35FocalLen(double d);

    double getAOV();

    void setAOV(double d);

    std::tuple<QString, QString> currentTextToLens();

    void setProfileByName(const QString &name);

    static std::tuple<QString, QString> parseLensText(const QString &indexText);

private:
    Ui::LensProfile *ui;


    static QString
    buildDisplayString(const QString& stringOpt1,
                       const QString& stringOpt2,
                       float minValue,
                       float maxValue,
                       QDoubleSpinBox *spinBox);

};


#endif //REALTIME3D_LENSPROFILE_H
