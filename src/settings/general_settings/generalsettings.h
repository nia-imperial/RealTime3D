//
// Created by Nic on 26/02/2022.
//

#ifndef REALTIME3D_GENERALSETTINGS_H
#define REALTIME3D_GENERALSETTINGS_H

#include "../SettingsForm.h"


QT_BEGIN_NAMESPACE
namespace Ui { class GeneralSettings; }
QT_END_NAMESPACE

class GeneralSettings : public SettingsForm {
Q_OBJECT

public:
    explicit GeneralSettings(QWidget *parent = nullptr);

    ~GeneralSettings() override;

    void writeSettings() override;

    void readSettings() override;
    static SettingDescriptor desc;

    static QString openGL_key();
    static bool getOpenGLValue();

    void resetToDefault() override;

private:

    QList<QCheckBox *>allCheckBoxes;
    Ui::GeneralSettings *ui;
};


#endif //REALTIME3D_GENERALSETTINGS_H
