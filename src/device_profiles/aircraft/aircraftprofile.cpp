//
// Created by Nic on 28/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AircraftProfile.h" resolved

#include "aircraftprofile.h"
#include "ui_AircraftProfile.h"
#include "../../settings/path_settings/pathsettings.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QFileDialog>
#include <QDesktopServices>

AircraftProfile::AircraftProfile(const std::shared_ptr<lfDatabase> &ltd, QWidget *parent) :
        DeviceProfile(ltd, parent), ui(new Ui::AircraftProfile) {
    ui->setupUi(this);
    connect(ui->flightHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AircraftProfile::flightHeightChanged);

    connect(ui->flightSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AircraftProfile::flightSpeedChanged);

    connect(ui->loadProfile, &QPushButton::released,
            this, &AircraftProfile::loadProfile);
    AircraftProfile::populateProfileList();

    connect(ui->viewMetadataButton, &QPushButton::released,
            this, [this](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(currentProfilePath));
    });
}

AircraftProfile::~AircraftProfile() {
    delete ui;
}

double AircraftProfile::getFlightSpeed() {
    return ui->flightSpeed->value();
}

void AircraftProfile::setFlightSpeed(double ms) {
    ui->flightSpeed->setValue(ms);
    Q_EMIT flightSpeedChanged(ms);
}

double AircraftProfile::getHeight() {
    return ui->flightHeight->value();
}

void AircraftProfile::setHeight(double m) {
    ui->flightHeight->setValue(m);
    Q_EMIT flightHeightChanged(m);
}

void AircraftProfile::loadProfile() {
    if (ui->chooseProfile->currentIndex() < 2) {
        ui->viewMetadataButton->setDisabled(true);
        currentProfilePath = QString();
        Q_EMIT profileChosen(QString());
        return;
    }
    auto profileLoc = PathSettings::getAircraftProfilesPath();
    auto profileName = ui->chooseProfile->currentText();
    currentProfilePath = QDir(profileLoc).filePath(profileName)+".json";
    auto profile = readProfileFile(currentProfilePath);
    auto profileInfo = profile["aircraft"].toObject();
    ui->flightSpeed->setValue(profileInfo["speed"].toDouble());
    ui->flightHeight->setValue(profileInfo["height"].toDouble());
    qInfo() << "Opening Aircraft profile:" << profileName;
    Q_EMIT profileChosen(profileName);
    ui->viewMetadataButton->setDisabled(false);
}

void AircraftProfile::populateProfileList() {
    qInfo() << "Aircraft list populated.";
    auto oldIndex = ui->chooseProfile->currentText();
    ui->chooseProfile->clear();
    ui->chooseProfile->addItem("Choose Profile");
    ui->chooseProfile->insertSeparator(1);
    auto profileLocDir = QDir(PathSettings::getAircraftProfilesPath());
    auto profiles = profileLocDir.entryList({"*.json"}, QDir::NoFilter, QDir::Name);
    for (const auto& profilePath : profiles){
        auto profile = readProfileFile(profileLocDir.filePath(profilePath));
        if (profile.isEmpty())
            continue;
        auto profileInfo = profile["aircraft"].toObject();
//        qDebug() << profileInfo["name"].toString();
        ui->chooseProfile->addItem(profileInfo["name"].toString());
        ui->chooseProfile->update();
    }
    ui->chooseProfile->setCurrentText(oldIndex);
}

QJsonObject AircraftProfile::readProfileFile(const QString &filePath){
    QFile loadFile;
    loadFile.setFileName(filePath);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open Aircraft profile file:" << loadFile.fileName();
        return{};
    }
    auto saveData = loadFile.readAll();
    auto profile = QJsonDocument::fromJson(saveData).object();
    return profile;
}


void AircraftProfile::saveProfile() {
    ui->chooseProfile->update();
    auto profileLoc = PathSettings::getAircraftProfilesPath();
    if (not QDir(profileLoc).exists())
        QDir().mkpath(profileLoc);
    auto savePath = QFileDialog::getSaveFileName(nativeParentWidget(),
                                                 QLatin1String("Save Aircraft Profile"),
                                                 profileLoc,
                                                 "JSON (*.json)");
    if (savePath.isEmpty() or savePath.isNull()) return;

    QJsonObject profile;
    profile["aircraft"] = QJsonObject{
            {"name", QFileInfo(savePath).baseName()},
            {"speed", ui->flightSpeed->value()},
            {"height", ui->flightHeight->value()},
            {"meta data", QJsonObject()}
    };
    QFile saveFile(savePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't open file:" << saveFile << "for writing.";
        return;
    }
    qInfo() << "Aircraft profile written to" << savePath;
    saveFile.write(QJsonDocument(profile).toJson(QJsonDocument::Indented));
    QTimer::singleShot(0, this, [this]() { populateProfileList();});
}

void AircraftProfile::setProfileByName(const QString &name) {
    if (name.isNull() or name.isEmpty()) return;
    auto idx = ui->chooseProfile->findText(name);
    if (idx == -1){
        qWarning() << "Project's Aircraft profile" << name << "could not be found";
        return;
    }
    ui->chooseProfile->setCurrentIndex(idx);
    loadProfile();
}


