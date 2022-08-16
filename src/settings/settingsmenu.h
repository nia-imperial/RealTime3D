//
// Created by Nic on 23/02/2022.
//

#ifndef REALTIME3D_SETTINGSMENU_H
#define REALTIME3D_SETTINGSMENU_H

#include <QWidget>
#include <QDialog>
#include <QMap>
#include <QListWidgetItem>
#include "path_settings/pathsettings.h"
#include "dem_behaviour/dembehaviour.h"
#include "general_settings/generalsettings.h"
#include "geometric_settings/geometricsettings.h"


QT_BEGIN_NAMESPACE
namespace Ui { class SettingsMenu; }
QT_END_NAMESPACE

class SettingsMenu : public QDialog {
Q_OBJECT

enum SettingsType{Path, DEM};

public:
    explicit SettingsMenu(QWidget *parent = nullptr);

    ~SettingsMenu() override;

    GeneralSettings *generalSettings;
    PathSettings *pathSettings;
    DemBehaviour *demBehaviour;
    GeometricSettings *geometricSettings;
    void setSettingsField(int index);

Q_SIGNALS:
    void applySettings();
    void settingsApplied();
    void settingsShown();

private:
    QMap<int, SettingsForm*> settingsMap;
    bool changesMade;
    Ui::SettingsMenu *ui;
    void reportChanges();
    void setSettingsItem();
    void confirmSettings();
    void clearLayout();

protected:
    void showEvent(QShowEvent *event) override;

    void addSettingsField(SettingsForm* settingsForm);
};


#endif //REALTIME3D_SETTINGSMENU_H
