//
// Created by Nic on 28/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_OverviewProfile.h" resolved

#include "overviewprofile.h"
#include "ui_OverviewProfile.h"
#include "../settings/path_settings/pathsettings.h"
#include "../utility/messages.hpp"
#include <QFileInfo>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileDialog>

OverviewProfile::OverviewProfile(const QString &dbPath, QWidget *parent) :
        QWidget(parent), ui(new Ui::OverviewProfile),
        lensfunDB(std::make_shared<lfDatabase>()) {
    ui->setupUi(this);
    setDB_path(dbPath);
    aircraftProfile = new AircraftProfile(lensfunDB);
    cameraProfile = new CameraProfile(lensfunDB);
    lensProfile = new LensProfile(lensfunDB);

    connect(aircraftProfile, &AircraftProfile::profileChosen,
            this, [this](const QString& name){
        if (name.isEmpty() or name.isNull())
            ui->aircraftProfileName->setText(QLatin1String("----"));
        else
            ui->aircraftProfileName->setText(name);
    });

    connect(cameraProfile, &CameraProfile::isoChanged,
            ui->iso, &QSpinBox::setValue);
    connect(cameraProfile, &CameraProfile::shutterSpeedChanged,
            ui->shutterSpeed, &QSpinBox::setValue);
    connect(cameraProfile, &CameraProfile::imageHeightChanged,
            ui->imageDimLength, &QDoubleSpinBox::setValue);
    connect(cameraProfile, &CameraProfile::imageWidthChanged,
            ui->imageDimWidth, &QDoubleSpinBox::setValue);
    connect(cameraProfile, &CameraProfile::profileChosen,
            ui->cameraProfileName, &QLabel::setText);
    connect(cameraProfile, &CameraProfile::profileIndexChanged,
            lensProfile, QOverload<const QString &, const QString &>::of(&LensProfile::populateProfileList));

    connect(lensProfile, &LensProfile::focalLengthChanged,
            ui->focalLen, &QDoubleSpinBox::setValue);
    connect(lensProfile, &LensProfile::apertureChanged,
            ui->aperture, &QDoubleSpinBox::setValue);
    connect(lensProfile, &LensProfile::profileChosen,
            ui->lensProfileName, &QLabel::setText);
    connect(lensProfile, &LensProfile::apertureChanged,
            ui->aperture, &QDoubleSpinBox::setValue);

}

OverviewProfile::~OverviewProfile() {
    delete ui;
}

void OverviewProfile::setDB_path(const QString &path) {
    if (not QFileInfo::exists(path)) return;
//    lensfunDB->Load()
    if (lensfunDB->Load(path.toStdString().c_str()) != LF_NO_ERROR) {
        qWarning() << "ERROR: Database could not be loaded\n";
    } else {
        Q_EMIT dbPathUpdated();
        qInfo() << "Path to camera and lens profiles set.";
    }
}

QJsonObject OverviewProfile::loadProjectFile(const QString &filePath) {
    QFile loadFile;
    if ((filePath.isNull() or filePath.isEmpty())) {
        auto projectChosenDir = QFileDialog::getExistingDirectory(
                this,
                QLatin1String("Load RT3D Project"),
                PathSettings::default_tigerInputDir(),
                QFileDialog::ShowDirsOnly
                | QFileDialog::DontResolveSymlinks
        );
        if (projectChosenDir == PathSettings::default_tigerDir()){return {};}
        if (projectChosenDir == PathSettings::default_dataDir()){return {};}
        if (projectChosenDir == PathSettings::default_tigerInputDir()){return {};}
        if (projectChosenDir == PathSettings::default_tigerOutputDir()){return {};}
        auto projectFilePath = QDir(projectChosenDir).filePath("rt3dProject.json");
        if (not QFileInfo::exists(projectFilePath)){
            Messages::warning_msg(this, "No Project file found in folder.");
            return{};
        }
        loadFile.setFileName(QDir::toNativeSeparators(projectFilePath));
    } else {
        loadFile.setFileName(QDir::toNativeSeparators(filePath));
    }

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open project file:" << loadFile.fileName();
        return {};
    }
    qInfo() << "Loading project:" << loadFile.fileName();
    auto saveData = loadFile.readAll();
    auto project = QJsonDocument::fromJson(saveData).object();
    auto projectInfo = project.value("rt3d project").toObject();

    auto imageInfo = projectInfo.value("image info").toObject();
    cameraProfile->setImageWidth(imageInfo.value("width").toDouble());
    cameraProfile->setImageHeight(imageInfo.value("height").toDouble());

    auto profiles = projectInfo.value("profiles").toObject();
    auto aircraftInfo = profiles.value("aircraft").toObject();
    aircraftProfile->setProfileByName(aircraftInfo.value("name").toString());

    auto cameraInfo = profiles.value("camera").toObject();
    cameraProfile->searchAndSetCamera(cameraInfo.value("name").toString());
    cameraProfile->setISO(cameraInfo.value("iso").toInt(100));
    cameraProfile->setShutterSpeed(cameraInfo.value("shutter speed").toInt(1));
    cameraProfile->setImageInterval(cameraInfo.value("image interval").toDouble());
    auto sensorDimInfo = cameraInfo.value("sensor dim.").toObject();
    cameraProfile->setSensorWidth(sensorDimInfo.value("width").toDouble());
    cameraProfile->setSensorHeight(sensorDimInfo.value("height").toDouble());
    if (sensorDimInfo.contains("crop factor"))
        cameraProfile->setCropFactor(sensorDimInfo.value("crop factor").toDouble());

    auto lensInfo = profiles.value("lens").toObject();
    lensProfile->setFocalLen(lensInfo.value("focal len.").toDouble());
    lensProfile->setAperture(lensInfo.value("aperture").toDouble());
    lensProfile->set35FocalLen(lensInfo.value("35mm focal len.").toDouble());
    lensProfile->setAOV(lensInfo.value("aov").toDouble());

    projectInfo["input path"] = loadFile.fileName();
    return projectInfo;
}

QString checkValidProfileName(const QString &name) {
    if (name == "----")
        return {};
    else
        return name;
}

void OverviewProfile::saveProjectFile(const QString &saveFilePath,
                                      const QString &projectOutputPath) {
    QJsonObject project;
    project["rt3d project"] = QJsonObject{
            {"name",        QFileInfo(saveFilePath).baseName()},
            {"output path", projectOutputPath},
            {"image info",  QJsonObject{
                    {"width",  ui->imageDimWidth->value()},
                    {"height", ui->imageDimLength->value()}
            }},
            {"profiles",    QJsonObject{
                    {"aircraft", QJsonObject{
                            {"name", checkValidProfileName(ui->aircraftProfileName->text())}
                    }},
                    {"camera",   QJsonObject{
                            {"name",           checkValidProfileName(ui->cameraProfileName->text())},
                            {"iso",            ui->iso->value()},
                            {"shutter speed",  ui->shutterSpeed->value()},
                            {"image interval", cameraProfile->getImageInterval()},
                            {"sensor dim.",    QJsonObject{
                                    {"width",  cameraProfile->getSensorWidth()},
                                    {"height", cameraProfile->getSensorHeight()},
                                    {"crop factor", cameraProfile->getCropFactor()}
                            }}
                    }},
                    {"lens",     QJsonObject{
                            {"name",            checkValidProfileName(ui->lensProfileName->text())},
                            {"focal len.",      ui->focalLen->value()},
                            {"aperture",        ui->aperture->value()},
                            {"35mm focal len.", lensProfile->get35FocalLen()},
                            {"aov",             lensProfile->getAOV()}
                    }}
            }},
    };

    auto savePath = QDir(saveFilePath).filePath("rt3dProject.json");

    if (!savePath.isEmpty() and !savePath.isNull()) {
        QFile saveFile(savePath);
        if (!saveFile.open(QIODevice::WriteOnly)) {
            qWarning() << "Project could not be created at path:" << savePath;
            return;
        }
        saveFile.write(QJsonDocument(project).toJson(QJsonDocument::Indented));
        qInfo() << "Project written to" << savePath;
        return;
    }

}

bool OverviewProfile::isAircraftSet() {
    if (checkValidProfileName(ui->aircraftProfileName->text()).isEmpty())
        return false;
    return true;
}

bool OverviewProfile::isCameraSet() {
    if (checkValidProfileName(ui->cameraProfileName->text()).isEmpty())
        return false;
    return true;
}

bool OverviewProfile::isLensSet() {
    if (checkValidProfileName(ui->lensProfileName->text()).isEmpty())
        return false;
    return true;
}

