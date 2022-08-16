//
// Created by Nic on 21/01/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_flightParameters.h" resolved

#include <iostream>
#include <QScreen>
#include <QSettings>
#include "FlightParameters.h"
#include "ui_FlightParameters.h"
#include <cmath>

#include "ImageCalculations.hpp"
#include "UnitConversion.hpp"

// extra: integrate with main program and profiles

std::ostream &operator<<(std::ostream &os, FlightParameters::ShotType &u) {
    switch (u) {
        case FlightParameters::ShotType::INTERVAL:
            os << "interval";
            break;
        case FlightParameters::ShotType::BASELINE:
            os << "baseline";
            break;
        default    :
            os.setstate(std::ios_base::failbit);
    }
    return os;
}

QDebug operator<<(QDebug debug, const FlightParameters::ShotType &u) {
    QDebugStateSaver saver(debug);
    switch (u) {
        case FlightParameters::ShotType::INTERVAL:
            debug.nospace() << "interval";
            break;
        case FlightParameters::ShotType::BASELINE:
            debug.nospace() << "baseline";
            break;
    }

    return debug;
}

std::ostream &operator<<(std::ostream &os, FlightParameters::SpeedUnit &u) {
    switch (u) {
        case FlightParameters::SpeedUnit::ms   :
            os << "m/s";
            break;
        case FlightParameters::SpeedUnit::kmh:
            os << "kmh";
            break;
        case FlightParameters::SpeedUnit::fps :
            os << "ft/s";
            break;
        case FlightParameters::SpeedUnit::mph  :
            os << "mph";
            break;
        default    :
            os.setstate(std::ios_base::failbit);
    }
    return os;
}

QDebug operator<<(QDebug debug, const FlightParameters::SpeedUnit &u) {
    QDebugStateSaver saver(debug);
    switch (u) {
        case FlightParameters::SpeedUnit::ms   :
            debug.nospace() << "m/s";
            break;
        case FlightParameters::SpeedUnit::kmh:
            debug.nospace() << "kmh";
            break;
        case FlightParameters::SpeedUnit::fps :
            debug.nospace() << "ft/s";
            break;
        case FlightParameters::SpeedUnit::mph  :
            debug.nospace() << "mph";
            break;
    }

    return debug;
}

std::ostream &operator<<(std::ostream &os, FlightParameters::DistUnit &u) {
    switch (u) {
        case FlightParameters::DistUnit::m   :
            os << "m";
            break;
        case FlightParameters::DistUnit::ft:
            os << "ft";
            break;
        default    :
            os.setstate(std::ios_base::failbit);
    }
    return os;
}

QDebug operator<<(QDebug debug, const FlightParameters::DistUnit &u) {
    QDebugStateSaver saver(debug);
    switch (u) {
        case FlightParameters::DistUnit::m:
            debug.nospace() << "m";
            break;
        case FlightParameters::DistUnit::ft:
            debug.nospace() << "ft";
            break;
    }

    return debug;
}

FlightParameters::FlightParameters(QWidget *parent) : QDialog(parent), ui(new Ui::FlightParameters) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // --- Camera Settings ---
    // - image resolution
    // image width
//    connect(ui->imageWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
//            this, &FlightParameters::handle_imageWidthSpin);

    // image height
//    connect(ui->imageHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged),
//            this, &FlightParameters::handle_imageHeightSpin);

    // - shooting settings
    // shutter speed
//    connect(ui->shutterSpeedSpin, QOverload<int>::of(&QSpinBox::valueChanged),
//            this, &FlightParameters::handle_shutterSpeedSpin);

    // aperture
//    connect(ui->apertureSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_apertureSpin);

    // iso
//    connect(ui->isoSpin, QOverload<int>::of(&QSpinBox::valueChanged),
//            this, &FlightParameters::handle_isoSpin);

    // shooting type
    cameraBaseline = 0;
    imageInterval = 0;
    connect(ui->shootingTypeCmbo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FlightParameters::changeShootingType);

    // - lens parameters
    // focal length
//    connect(ui->focalLengthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_focalLengthSpin);

    // - sensor parameters
    // sensor width
//    connect(ui->sensorWidthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_sensorWidthSpin);

    // sensor height
//    connect(ui->sensorHeightSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_sensorHeightSpin);

    // - uav settings
    // speed
//    uavSpeed = ui->speedSpin->value();
//    connect(ui->speedSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_speedSpin);
//    connect(ui->speedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
//            this, &FlightParameters::changeSpeedUnit);

    // height
//    uavHeight = ui->heightSpin->value();
//    connect(ui->heightSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_heightSpin);
//    connect(ui->heightComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
//            this, &FlightParameters::changeHeightUnit);

    // --- Calculations ---
    // ground px size
//    connect(ui->groundPixelSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_groundPixelSizeSpin);

    // image interval size
//    connect(ui->imageIntvSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_imageIntvSizeSpin);

    // image overlap
//    connect(ui->imageOverlapSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_imageOverlapSpin);
//    connect(ui->imageOverlapMetricCmbo, QOverload<int>::of(&QComboBox::currentIndexChanged),
//            this, &FlightParameters::changeOverlapUnit);

    // - image ground size
    // x
//    connect(ui->imageGroundSizeXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_imageGroundSizeXSpin);

    // y
//    connect(ui->imageGroundSizeYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_imageGroundSizeYSpin);

    // motion blur
//    connect(ui->motionBlurSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &FlightParameters::handle_motionBlurSpin);
//    connect(ui->motionBlurMetricCmbo, QOverload<int>::of(&QComboBox::currentIndexChanged),
//            this, &FlightParameters::changeMotionBlurUnit);

    readSettings();
    calculate();
}

void FlightParameters::calculate() {
    using std::numbers::pi;
    /// diagonal of "full frame" sensor.
    auto diagonal_equiv = std::sqrt(24 * 24 + 36 * 36);

    /// diagonal of image in pixels.
//    auto diagonal_px = std::sqrt(std::pow(ui->imageWidthSpin->value(), 2) + std::pow(ui->imageHeightSpin->value(), 2));

//    if (ui->shutterSpeedSpin->value() == 0)
//        ui->shutterSpeedSpin->setValue(1000);
//    if (ui->shootingTypeSpin->value() == 0)
//        ui->shootingTypeSpin->setValue(10);

    /// diagonal of image in degrees.
//    auto diagonal_degrees = 180 * 2 * std::atan(diagonal_equiv / (2 * ui->focalLengthSpin->value())) / pi;
    /// diagonal of image in metres.
//    auto diagonal_metres = uavHeight * std::tan(pi * diagonal_degrees / (2 * 180));

//    diagonalViewAngle = diagonal_degrees;

    /// distance in metres between each sequential image.
//    auto imageInterval_meters = uavSpeed * ui->shootingTypeSpin->value();
    /// time between each sequential image.
//    auto imageInterval_sec = uavSpeed / imageInterval_meters;
//    switch (ShotType{ui->shootingTypeCmbo->currentIndex()}) {
//        case ShotType::INTERVAL:
//            ui->imageIntvSizeSpin->setValue(imageInterval_meters);
//            break;
//        case ShotType::BASELINE:
//            ui->imageIntvSizeSpin->setValue(imageInterval_sec);
//            break;
//    }
//
//    angularResolution = diagonal_degrees / diagonal_px;

    /// the number of centimetres each pixel represents.
//    auto groundPixelSize_cm = 100 * diagonal_metres / diagonal_px;
//    ui->groundPixelSizeSpin->setValue(groundPixelSize_cm);

    /// amount of motion blur in cm for the given settings.
//    auto motionBlur_cm = 100 * uavSpeed / ui->shutterSpeedSpin->value();
    /// ammount of motion blur in pixels for the given settings.
//    auto motionBlur_pixels = (100 * uavSpeed / ui->shutterSpeedSpin->value()) / groundPixelSize_cm;
//    switch (BlurUnit{ui->motionBlurMetricCmbo->currentIndex()}) {
//        case BlurUnit::cm:
//            ui->motionBlurSpin->setValue(motionBlur_cm);
//            break;
//        case BlurUnit::px:
//            ui->motionBlurSpin->setValue(motionBlur_pixels);
//            break;
//    }

    /// number of metres covered by the height of the image.
//    auto imageSizeY_metres = ui->imageHeightSpin->value() * diagonal_metres / diagonal_px;
//    ui->imageGroundSizeYSpin->setValue(imageSizeY_metres);
    /// number of metres covered by the width of the image.
//    auto imageSizeX_metres = ui->imageWidthSpin->value() * diagonal_metres / diagonal_px;
//    ui->imageGroundSizeXSpin->setValue(imageSizeX_metres);

    /// number of metres overlap between two sequential images.
//    auto imageOverlap_metres = imageSizeY_metres - imageInterval_meters;
    /// percentage overlap between two sequential images.
//    double imageOverlap_percentage;
//    switch (OverlapUnit{ui->imageOverlapMetricCmbo->currentIndex()}) {
//        case OverlapUnit::percent:
//            if (imageOverlap_metres > 0)
//                imageOverlap_percentage = 100 * imageOverlap_metres / imageSizeY_metres;
//            else
//                imageOverlap_percentage = 0;
//            ui->imageOverlapSpin->setValue(imageOverlap_percentage);
//            break;
//        case OverlapUnit::metres:
//            ui->imageOverlapSpin->setValue(imageInterval_meters);
//            break;
//    }

//    rotationBlur1 = (rotationLensAxis / ui->shutterSpeedSpin->value()) * diagonal_px * pi / 360;
//    rotationBlur2 = (rotationOrthogonal / ui->shutterSpeedSpin->value()) / (diagonal_degrees / diagonal_px);

}

FlightParameters::~FlightParameters() {
    delete ui;
}

void FlightParameters::setGroundPixelSize() {
//    auto groundPxSize = image::groundPixelSize::cm(
//            ui->imageWidthSpin->value(),
//            ui->imageHeightSpin->value(),
//            uavHeight,
//            ui->focalLengthSpin->value()
//    );
//    ui->groundPixelSizeSpin->setValue(groundPxSize);
//    calculate();
}

void FlightParameters::setImageIntervalSize() {
    auto u = ShotType{ui->shootingTypeCmbo->currentIndex()};
    double baselineDist;
    switch (u) {
        case ShotType::INTERVAL:
            baselineDist = image::interval::meters(
                    uavSpeed,
                    imageInterval
            );
            break;
        case ShotType::BASELINE:
            baselineDist = cameraBaseline;
            break;
    }

    ui->imageIntvSizeSpin->setValue(baselineDist);
}

void FlightParameters::setImageOverlap() {
//    double overlap;
//    auto u = OverlapUnit{ui->imageOverlapMetricCmbo->currentIndex()};
//    switch (u) {
//        case OverlapUnit::percent:
//            overlap = image::overlap::percent(
//                    ui->imageWidthSpin->value(),
//                    ui->imageHeightSpin->value(),
//                    uavHeight,
//                    ui->focalLengthSpin->value()
//            );
//            break;
//        case OverlapUnit::metres:
//            overlap = image::overlap::meters(
//                    ui->imageWidthSpin->value(),
//                    ui->imageHeightSpin->value(),
//                    uavHeight,
//                    ui->focalLengthSpin->value()
//            );
//            break;
//    }
//    ui->imageOverlapSpin->setValue(overlap);
    calculate();
}

void FlightParameters::setImageGroundSize() {
//    auto gsX = image::groundSize::metres(
//            ui->imageWidthSpin->value(),
//            uavHeight,
//            ui->focalLengthSpin->value()
//    );
//    ui->imageGroundSizeXSpin->setValue(gsX);
//
//    auto gsY = image::groundSize::metres(
//            ui->imageHeightSpin->value(),
//            uavHeight,
//            ui->focalLengthSpin->value()
//    );
//    ui->imageGroundSizeYSpin->setValue(gsY);

}

void FlightParameters::setMotionBlur() {
    calculate();
//    double blur;
//    auto idx = ui->motionBlurMetricCmbo->currentIndex();
//    auto u = BlurUnit{idx};
//    switch (u) {
//        case BlurUnit::cm:
//            blur = image::motionBlur::cm(
//                    uavSpeed,
//                    ui->shutterSpeedSpin->value()
//            );
//            break;
//        case BlurUnit::px:
//            blur = image::motionBlur::pixels(
//                    uavSpeed,
//                    ui->shutterSpeedSpin->value()
//            );
//            break;
//    }
//    ui->motionBlurSpin->setValue(blur);
}

void FlightParameters::changeShootingType(int index) {
    switch (ShotType{index}) {
        case ShotType::INTERVAL:
//            ui->shootingTypeLbl->setText("sec");
//            ui->shootingTypeSpin->setDecimals(0);
//            ui->shootingTypeSpin->setValue(imageInterval);

            ui->imageIntvMetricLbl->setText("m");
            break;
        case ShotType::BASELINE:
//            ui->shootingTypeLbl->setText("m");
//            ui->shootingTypeSpin->setDecimals(2);
//            ui->shootingTypeSpin->setValue(cameraBaseline);

            ui->imageIntvMetricLbl->setText("sec");
            break;
    }
    calculate();
}

void FlightParameters::changeSpeedUnit(int index) {
    double speed;
    auto u = SpeedUnit{index};
//    qDebug() << "speed unit changed to" << u;
    switch (u) {
        case SpeedUnit::ms:
            speed = uavSpeed;
            break;
        case SpeedUnit::kmh:
            using units::metres::per::second::to::kmh;
            speed = kmh(uavSpeed);
            break;
        case SpeedUnit::fps:
            using units::metres::per::second::to::fps;
            speed = fps(uavSpeed);
            break;
        case SpeedUnit::mph:
            using units::metres::per::second::to::mph;
            speed = mph(uavSpeed);
            break;
    }
//    ui->speedSpin->setValue(speed);
}

void FlightParameters::changeHeightUnit(int index) {
    double height;
    auto u = DistUnit{index};
//    qDebug() << "height unit changed to" << u;
    switch (u) {
        case DistUnit::m:
            height = uavHeight;
            break;
        case DistUnit::ft:
            using units::metres::to::feet;
            height = feet(uavHeight);
            break;
    }
//    ui->heightSpin->setValue(height);

}

void FlightParameters::changeOverlapUnit(int index) {
//    setImageOverlap();
    calculate();
}

void FlightParameters::changeMotionBlurUnit(int index) {
    calculate();
}

void FlightParameters::handle_imageWidthSpin(int val) {
    calculate();
}

void FlightParameters::handle_imageHeightSpin(int val) {
    calculate();
}

void FlightParameters::handle_shutterSpeedSpin(int val) {
    calculate();
}

void FlightParameters::handle_apertureSpin(double val) {
    calculate();
}

void FlightParameters::handle_isoSpin(int val) {
    calculate();
}

void FlightParameters::handle_shootingTypeSpin(double val) {
//    auto u = ShotType{ui->shootingTypeCmbo->currentIndex()};
//    switch (u) {
//        case ShotType::INTERVAL:
//            imageInterval = val;
////            using image::interval::meters;
////            cameraBaseline = meters(uavSpeed,imageInterval);
//            break;
//        case ShotType::BASELINE:
//            cameraBaseline = val;
//            break;
//    }
//    setImageIntervalSize();
    calculate();
}

void FlightParameters::handle_sensorWidthSpin(double val) {
    calculate();
}

void FlightParameters::handle_focalLengthSpin(double val) {
    calculate();
}

void FlightParameters::handle_sensorHeightSpin(double val) {
    calculate();
}

void FlightParameters::handle_speedSpin(double val) {
//    auto idx = ui->speedComboBox->currentIndex();
//    auto u = SpeedUnit{idx};
//    switch (u) {
//        case SpeedUnit::ms:
//            uavSpeed = val;
//            break;
//        case SpeedUnit::kmh:
//            using units::metres::per::second::from::kmh;
//            uavSpeed = kmh(val);
//            break;
//        case SpeedUnit::fps:
//            using units::metres::per::second::from::fps;
//            uavSpeed = fps(val);
//            break;
//        case SpeedUnit::mph:
//            using units::metres::per::second::from::mph;
//            uavSpeed = mph(val);
//            break;
//    }

    calculate();
}

void FlightParameters::handle_heightSpin(double val) {
//    auto idx = ui->heightComboBox->currentIndex();
//    auto u = DistUnit{idx};
//    switch (u) {
//        case DistUnit::m:
//            uavHeight = val;
//            break;
//        case DistUnit::ft:
//            using units::metres::from::feet;
//            uavHeight = feet(val);
//            break;
//    }

    calculate();
}

void FlightParameters::handle_groundPixelSizeSpin(double val) {

}

void FlightParameters::handle_imageIntvSizeSpin(double val) {

}

void FlightParameters::handle_imageOverlapSpin(double val) {

}

void FlightParameters::handle_imageGroundSizeXSpin(double val) {

}

void FlightParameters::handle_imageGroundSizeYSpin(double val) {

}

void FlightParameters::handle_motionBlurSpin(double val) {

}

void FlightParameters::writeSettings() {
    QSettings settings;

    settings.beginGroup("FlightParameters"); // ---
    settings.setValue("geometry", saveGeometry());

    settings.beginGroup("CameraSettings"); // --
    settings.beginGroup("ImageResolution"); // -
//    settings.setValue("ImageWidth", ui->imageWidthSpin->value());
//    settings.setValue("ImageHeight", ui->imageHeightSpin->value());
    settings.endGroup(); // -

    settings.beginGroup("ShootingSettings"); // -
//    settings.setValue("ShutterSpeed", ui->shutterSpeedSpin->value());
//    settings.setValue("Aperture", ui->apertureSpin->value());
//    settings.setValue("ISOSpeed", ui->isoSpin->value());
    settings.setValue("ShootingType", ui->shootingTypeCmbo->currentIndex());
    settings.setValue("ImageInterval", imageInterval);
    settings.setValue("CameraBaseline", cameraBaseline);
    settings.endGroup(); // -

    settings.beginGroup("LensProfile"); // -
//    settings.setValue("FocalLength", ui->focalLengthSpin->value());
    settings.endGroup(); // -


    settings.beginGroup("SensorProfile"); // -
//    settings.setValue("SensorWidth", ui->sensorWidthSpin->value());
//    settings.setValue("SensorHeight", ui->sensorHeightSpin->value());
    settings.endGroup(); // -
    settings.endGroup(); // --

    settings.beginGroup("UAVSettings"); // --
    settings.setValue("uavSpeed", uavSpeed);
//    settings.setValue("uavSpeedUnits", ui->speedComboBox->currentIndex());
    settings.setValue("uavHeight", uavHeight);
//    settings.setValue("uavHeightUnits", ui->heightComboBox->currentIndex());
    settings.endGroup(); // --
    settings.endGroup(); // ---

}

void FlightParameters::readSettings() {

    QSettings settings;
    settings.beginGroup("FlightParameters"); // ---
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (not geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    settings.beginGroup("CameraSettings"); // --
    settings.beginGroup("ImageResolution"); // -
    auto imageWidth = settings.value("ImageWidth").toInt();
//    ui->imageWidthSpin->setValue(imageWidth);
    auto imageHeight = settings.value("ImageHeight").toInt();
//    ui->imageHeightSpin->setValue(imageHeight);
    settings.endGroup(); // -

    settings.beginGroup("ShootingSettings"); // -
    auto shutterSpeed = settings.value("ShutterSpeed").toInt();
//    ui->shutterSpeedSpin->setValue(shutterSpeed);
    auto aperture = settings.value("Aperture").toDouble();
//    ui->apertureSpin->setValue(aperture);
    auto isoSpeed = settings.value("ISOSpeed").toInt();
//    ui->isoSpin->setValue(isoSpeed);
    imageInterval = settings.value("ImageInterval").toDouble();
    cameraBaseline = settings.value("CameraBaseline").toDouble();
    auto shootingType = settings.value("ShootingType").toInt();
    ui->shootingTypeCmbo->setCurrentIndex(shootingType);
    settings.endGroup(); // -

    settings.beginGroup("LensProfile"); // -
    auto focalLength = settings.value("FocalLength").toDouble();
//    ui->focalLengthSpin->setValue(focalLength);
    settings.endGroup(); // -

    settings.beginGroup("SensorProfile"); // -
    auto sensorWidth = settings.value("SensorWidth").toDouble();
//    ui->sensorWidthSpin->setValue(sensorWidth);
    auto sensorHeight = settings.value("SensorHeight").toDouble();
//    ui->sensorHeightSpin->setValue(sensorHeight);

    settings.endGroup(); // -
    settings.endGroup(); // --

    settings.beginGroup("UAVSettings"); // --
    uavSpeed = settings.value("uavSpeed").toDouble();
//    ui->speedSpin->setValue(uavSpeed);
    auto droneSpeedUnits = settings.value("uavSpeedUnits").toInt();
//    ui->speedComboBox->setCurrentIndex(droneSpeedUnits);

    uavHeight = settings.value("uavHeight").toDouble();
//    ui->heightSpin->setValue(uavHeight);
    auto droneHeightUnits = settings.value("uavHeightUnits").toInt();
//    ui->heightComboBox->setCurrentIndex(droneHeightUnits);

    settings.endGroup(); // --
    settings.endGroup(); // ---

}

void FlightParameters::closeEvent(QCloseEvent *event) {
    writeSettings();
    QDialog::closeEvent(event);
}
