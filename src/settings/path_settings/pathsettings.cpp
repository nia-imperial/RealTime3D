//
// Created by Nic on 23/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PathSettings.h" resolved

#include "pathsettings.h"
#include "ui_PathSettings.h"
#include "../../config.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

PathSettings::PathSettings(QWidget *parent) :
        SettingsForm(desc, parent),
        ui(new Ui::PathSettings) {
    ui->setupUi(this);

    allLineEdits = findChildren<QLineEdit *>();
    for (auto lineEdit: allLineEdits) {
        connect(lineEdit, &QLineEdit::textChanged,
                this, [=, this]() {
                    reportChanges();
                    checkPathExists(lineEdit);
                });
    }

    connect(ui->selectExeBtn, &QPushButton::clicked, this, &PathSettings::selectExe);
    connect(ui->selectTigerBtn, &QPushButton::clicked, this, &PathSettings::selectTigerPath);
    connect(ui->selectCameraPathBtn, &QPushButton::clicked, this, &PathSettings::selectCameraPath);

}

SettingDescriptor PathSettings::desc{ // NOLINT(cert-err58-cpp)
        QStringLiteral(u"settings/paths"), QStringLiteral(u"Path Settings")
};

void PathSettings::selectExe() {
    QString exePath = QFileDialog::getOpenFileName(this,
                                                   "Select Executable",
                                                   ui->exePathLineEdit->text(),
                                                   "Executables (*.exe)");
    if (!exePath.isEmpty()) {
        exePath = QDir::toNativeSeparators(exePath);
        ui->exePathLineEdit->setText(exePath);
    }
}

void PathSettings::selectTigerPath() {
    QString exePath = QFileDialog::getExistingDirectory(this,
                                                        "Select Tiger Folder",
                                                        ui->tigerPathLineEdit->text(),
                                                        QFileDialog::ShowDirsOnly);
    if (!exePath.isEmpty() and exePath.contains(QLatin1String("Tiger"))) {
        exePath = QDir::toNativeSeparators(exePath);
        ui->tigerPathLineEdit->setText(exePath);
    }
}

void PathSettings::selectCameraPath() {
    QString exePath = QFileDialog::getExistingDirectory(this,
                                                        "Select Camera database Folder",
                                                        ui->camerasPathLineEdit->text(),
                                                        QFileDialog::ShowDirsOnly);
    if (!exePath.isEmpty()) {
        exePath = QDir::toNativeSeparators(exePath);
        ui->camerasPathLineEdit->setText(exePath);
    }
}

PathSettings::~PathSettings() {
    delete ui;
}

void PathSettings::readSettings() {
    ui->exePathLineEdit->setText(getDemExePath());
    ui->tigerPathLineEdit->setText(getTigerDir());
    ui->camerasPathLineEdit->setText(getCamerasPath());
}

void PathSettings::writeSettings() {
    auto exe_path = ui->exePathLineEdit->text();
    if (QFileInfo::exists(exe_path)) {
        settings.setValue(dem_exe_key(), exe_path);
        Q_EMIT exePathWritten(exe_path);
    }

    auto tiger_path = ui->tigerPathLineEdit->text();
    if (QFileInfo::exists(tiger_path)) {
        settings.setValue(tiger_dir_key(), tiger_path);
        Q_EMIT tigerPathWritten(tiger_path);
    }

    auto camDBpath = ui->camerasPathLineEdit->text();
    if (QFileInfo::exists(camDBpath)) {
        settings.setValue(lensfun_dir_key(), camDBpath);
        Q_EMIT camerasPathWritten(camDBpath);
    }
}


void PathSettings::checkPathExists(QLineEdit *lineEdit) {
    auto path = lineEdit->text();
    auto palette = lineEdit->palette();
    if (not QFileInfo::exists(path)) {
        palette.setColor(QPalette::Text, Qt::darkGray);
    } else {
        palette.setColor(QPalette::Text, Qt::black);
    }
}

void PathSettings::configureTigerDir() {
    QDir tiger_dir(default_tigerDir());
    if (not tiger_dir.exists()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(QStringLiteral(u"RealTime3D"));
        msgBox.setText("<b>C:\\Tiger was not found on system.</b>");
        msgBox.setInformativeText(
                "The <i>Tiger</i> folder is required for this application to operate correctly.<br>"
                "<br>Do you want to create <samp>C:\\Tiger</samp>?<br>"
                "<br><small>Close will end the application.</small>"
        );
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Close);
        msgBox.setDefaultButton(QMessageBox::Ok);
        switch (msgBox.exec()) {
            case QMessageBox::Ok:
                createTigerDir();
                break;
            case QMessageBox::Close:
                QApplication::quit();
                break;
            default:
                // should never be reached
                break;
        }
        return;
    }
    auto default_paths = defaultPaths();
    default_paths.pop_front();
    for (const auto &def_path: default_paths) {
        if (not QFileInfo::exists(def_path))
            tiger_dir.mkpath(def_path);
    }
}

void PathSettings::createTigerDir() {
    auto tiger_dir = QDir(default_tigerDir());
    for (const auto &def_path: defaultPaths()) {
        if (not QFileInfo::exists(def_path))
            tiger_dir.mkpath(def_path);
    }
}

QStringList PathSettings::defaultPaths() {
    return {
            default_tigerDir(),
            default_dataDir(),
            default_CropPicDir(),
            default_tigerInputDir(),
            default_tigerOutputDir(),
            default_lensfunDir(),
            default_DemExePath(),
            default_logOutputDir()
    };
}

QString PathSettings::default_tigerDir() {
    return QStringLiteral(u"C:/Tiger");
}

QString PathSettings::default_dataDir() {
    return QStringLiteral(u"C:/Tiger/Data");
}

QString PathSettings::default_lensfunDir() {
    return QCoreApplication::applicationDirPath() + QStringLiteral(u"/lensfunDB/db");
}


QString PathSettings::default_CropPicDir() {
    return QStringLiteral(u"C:/Tiger/Data/Input/Crop_Pic");
}

QString PathSettings::default_tigerInputDir() {
    return QStringLiteral(u"C:/Tiger/Data/Input");
}

QString PathSettings::default_tigerOutputDir() {
    return QStringLiteral(u"C:/Tiger/Data/Output");
}

QString PathSettings::default_logOutputDir(){
    return QStringLiteral(u"C:/Tiger/Data/Output/SENT02_logs");
}

QString PathSettings::default_DemExePath() {
    return QCoreApplication::applicationDirPath() + QLatin1String(
            "/DemGeneration_software_Licensed/ExerciseDemGeneration.exe");
}

QString PathSettings::dem_exe_key() {
    return QStringLiteral(u"dem_executable");
}

QString PathSettings::tiger_dir_key() {
    return QStringLiteral(u"tiger_dir");
}

QString PathSettings::lensfun_dir_key() {
    return QStringLiteral(u"lensfunDB");
}

QString PathSettings::getTigerDir() {
    auto tigerDir = getSettingValue(PathSettings::desc, tiger_dir_key(), default_tigerDir()).toString();
    return tigerDir;
}

QString PathSettings::getDemExePath() {
    auto demExePath = getSettingValue(PathSettings::desc, dem_exe_key(), default_DemExePath()).toString();
    if (QApplication::applicationDirPath().contains("rt3d_")){
        if (demExePath.contains("rt3d_")){
            if (not demExePath.contains(MOST_VERSION)){
                return default_DemExePath();
            }
        }
    }
    return demExePath;
}

QString PathSettings::getCamerasPath() {
    auto camerasPaths = getSettingValue(PathSettings::desc, lensfun_dir_key(), default_lensfunDir()).toString();
    if (QApplication::applicationDirPath().contains("rt3d_")){
        if (camerasPaths.contains("rt3d_")){
            if (not camerasPaths.contains(MOST_VERSION)){
                return default_lensfunDir();
            }
        }
    }
    return camerasPaths;
}

void PathSettings::resetToDefault() {
    ui->exePathLineEdit->setText(default_DemExePath());
    ui->tigerPathLineEdit->setText(default_tigerDir());
    ui->camerasPathLineEdit->setText(default_lensfunDir());
}

QString PathSettings::getAircraftProfilesPath(){
    return QDir(QApplication::applicationDirPath()).filePath("aircraftDB");
}