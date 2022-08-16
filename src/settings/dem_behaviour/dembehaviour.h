//
// Created by Nic on 23/02/2022.
//

#ifndef REALTIME3D_DEMBEHAVIOUR_H
#define REALTIME3D_DEMBEHAVIOUR_H

#include <QWidget>
#include <QSettings>
#include <QCheckBox>
#include "../SettingsForm.h"


QT_BEGIN_NAMESPACE
namespace Ui { class DemBehaviour; }
QT_END_NAMESPACE

class DemBehaviour : public SettingsForm {
Q_OBJECT

public:
    explicit DemBehaviour(QWidget *parent = nullptr);

    void readSettings() override;

    ~DemBehaviour() override;

    static SettingDescriptor desc;

public Q_SLOTS:
    void writeSettings() override;

    static bool allow_autoLoadSettings();
    static bool allow_createOutputPaths();
    static bool allow_logPrefixAsParent();
    static bool allow_outputPrefixAsParent();
    static bool allow_persistentSettings();

    void resetToDefault() override;


private:
    QList<QCheckBox *> allCheckBoxes;
    Ui::DemBehaviour *ui;
};


#endif //REALTIME3D_DEMBEHAVIOUR_H
