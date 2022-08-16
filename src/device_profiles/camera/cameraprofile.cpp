//
// Created by Nic on 28/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_CameraProfile.h" resolved

#include "cameraprofile.h"
#include "ui_CameraProfile.h"
#include <lensfun/lensfun.h>
#include <QDebug>


CameraProfile::CameraProfile(const std::shared_ptr<lfDatabase> &ltd, QWidget *parent) :
        DeviceProfile(ltd, parent), ui(new Ui::CameraProfile) {
    ui->setupUi(this);

    connect(ui->imageDimWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &CameraProfile::imageWidthChanged);

    connect(ui->imageDimLength, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &CameraProfile::imageHeightChanged);

    connect(ui->loadProfile, &QPushButton::released,
            this, &CameraProfile::loadProfile);

    connect(ui->chooseProfile, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                auto [cameraName, cameraMaker] = currentTextToCamera();
                Q_EMIT profileIndexChanged(cameraName, cameraMaker);
            });

    ui->chooseProfile->insertSeparator(1);
    CameraProfile::populateProfileList();
}

CameraProfile::~CameraProfile() {
    delete ui;
}

int CameraProfile::getImageHeight() {
    return int(ui->imageDimLength->value());
}

int CameraProfile::getImageWidth() {
    return int(ui->imageDimWidth->value());
}

void CameraProfile::setImageWidth(double d) {
    ui->imageDimWidth->setValue(d);
}

void CameraProfile::setImageHeight(double d) {
    ui->imageDimLength->setValue(d);
}

void CameraProfile::populateProfileList() {
    ui->chooseProfile->clear();
    qInfo() << "Camera list populated.";
    if (auto db = lensfunDB.lock()) {
        auto cameras = db->GetCameras();

        for (auto i = 0; cameras[i]; i++) {
            auto camera = cameras[i];
            auto model = QString::fromLatin1(camera->Model);
            auto maker = QString::fromLatin1(camera->Maker);
            auto variant = QString::fromLatin1(camera->Variant);
            ui->chooseProfile->addItem(createMakerModelStr(maker, model, variant));
            ui->chooseProfile->setItemData(i, model, Qt::ToolTipRole);
        }
        ui->chooseProfile->model()->sort(0, Qt::AscendingOrder);
    }

    ui->chooseProfile->insertItem(0, def_index1);
    ui->chooseProfile->insertSeparator(1);
    ui->chooseProfile->setCurrentIndex(0);

}

void CameraProfile::loadProfile() {

    if (ui->chooseProfile->currentIndex() == 0) {
        Q_EMIT profileChosen(QLatin1String(""), QLatin1String(""), QLatin1String(""));
        return;
    }

    auto[cameraName, cameraMaker] = currentTextToCamera();

    auto camera = findCamera(cameraName, cameraMaker);
    if (camera) {
//        qDebug() << camera->Maker;
//        qDebug() << camera->Model;
//        qDebug() << camera->CropFactor;
//        qDebug() << camera->Mount;
//        qDebug() << camera->Variant;
        ui->cropFactor->setValue(camera->CropFactor);
        Q_EMIT profileChosen(cameraName, cameraMaker, camera->Mount);

    } else {
        qDebug() << "No camera profile found for" << cameraName << cameraMaker;
    }

}

std::tuple<QString, QString> CameraProfile::currentTextToCamera() {
    auto cameraString = ui->chooseProfile->currentText();
    auto cameraMaker = cameraString.section(":", 0, 0);
    auto cameraName = cameraString.section(":", 1, -1).trimmed();
    return {cameraName, cameraMaker};
}

void CameraProfile::searchAndSetCamera(const QString &cameraName) {
    if (cameraName.isEmpty() or cameraName.isNull()) return;
    auto camera = findCamera(cameraName);
    if (camera){
        auto idx = ui->chooseProfile->findText(
                createMakerModelStr(camera->Maker, camera->Model, camera->Variant));
        ui->chooseProfile->setCurrentIndex(idx);
        loadProfile();
        Q_EMIT profileChosen(camera->Model, camera->Maker, camera->Mount);
        parentWidget()->raise();
        return;
    } else {
        qDebug() << "No camera profile found for" << cameraName;
    }
}

QString CameraProfile::createMakerModelStr(const QString &maker, const QString &model, const QString &variant) {
    return maker + ": " + model + " " + variant;
}

void CameraProfile::setImageInterval(double val) {
    ui->imageIntervals->setValue(val);
    Q_EMIT intervalChanged(val);
}

double CameraProfile::getImageInterval() {
    return ui->imageIntervals->value();
}

void CameraProfile::setShutterSpeed(int val) {
    ui->shutterSpeed->setValue(val);
    Q_EMIT shutterSpeedChanged(val);
}

int CameraProfile::getShutterSpeed() {
    return ui->shutterSpeed->value();
}

double CameraProfile::getSensorWidth() {
    return ui->sensorDimWidth->value();
}

void CameraProfile::setSensorWidth(double val) {
    ui->sensorDimWidth->setValue(val);
    Q_EMIT sensorWidthChanged(val);
}

double CameraProfile::getSensorHeight() {
    return ui->sensorDimLength->value();
}

void CameraProfile::setSensorHeight(double val) {
    ui->sensorDimLength->setValue(val);
    Q_EMIT sensorHeightChanged(val);
}

int CameraProfile::getISO() {
    return ui->iso->value();
}

void CameraProfile::setISO(int val){
    ui->iso->setValue(val);
    Q_EMIT isoChanged(val);
}

double CameraProfile::getCropFactor() {
    return ui->cropFactor->value();
}

void CameraProfile::setCropFactor(double val) {
    ui->cropFactor->setValue(val);
}
