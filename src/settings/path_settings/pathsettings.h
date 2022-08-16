//
// Created by Nic on 23/02/2022.
//

#ifndef REALTIME3D_PATHSETTINGS_H
#define REALTIME3D_PATHSETTINGS_H

#include <QWidget>
#include <QSettings>
#include <QLineEdit>
#include <QString>
#include "../SettingsForm.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PathSettings; }
QT_END_NAMESPACE

class PathSettings : public SettingsForm {
Q_OBJECT

public:
    explicit PathSettings(QWidget *parent = nullptr);

    void resetToDefault() override;

    void readSettings() override;

    ~PathSettings() override;

    /// use with caution, a QApp must be created before use -- kept static for convenience
    static QString default_DemExePath();

    static QString default_tigerDir();

    static QString default_dataDir();

    static QString default_CropPicDir();

    static QString default_tigerInputDir();

    static QString default_tigerOutputDir();

    /// use with caution, a QApp must be created before use -- kept static for convenience
    static QString default_lensfunDir();

    static QString default_logOutputDir();

    static SettingDescriptor desc;

Q_SIGNALS:

    void exePathWritten(const QString &path);

    void tigerPathWritten(const QString &path);

    void camerasPathWritten(const QString &path);

public Q_SLOTS:

    void writeSettings() override;

    void configureTigerDir();

    static void createTigerDir();

    static QStringList defaultPaths();

    static QString getTigerDir();

    static QString getDemExePath();

    static QString getCamerasPath();

    static QString getAircraftProfilesPath();

private:
    static QString dem_exe_key();

    static QString tiger_dir_key();

    static QString lensfun_dir_key();

    QList<QLineEdit *> allLineEdits;

    static void checkPathExists(QLineEdit *lineEdit);

    Ui::PathSettings *ui;

    void selectExe();

    void selectTigerPath();

    void selectCameraPath();


};


#endif //REALTIME3D_PATHSETTINGS_H
