//
// Created by Nic on 28/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_LensProfile.h" resolved

#include "lensprofile.h"
#include "ui_LensProfile.h"
#include <QDebug>


LensProfile::LensProfile(const std::shared_ptr<lfDatabase> &ltd, QWidget *parent) :
        DeviceProfile(ltd, parent), ui(new Ui::LensProfile) {
    ui->setupUi(this);
    ui->fflenLabel->setHidden(true);
    ui->focalLen_ff->setHidden(true);
    ui->aovLabel->setHidden(true);
    ui->angleOfView->setHidden(true);

    connect(ui->loadProfile, &QPushButton::released,
            this, &LensProfile::loadProfile);

    ui->chooseProfile->insertSeparator(1);
    LensProfile::populateProfileList();

    connect(ui->focalLen, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &LensProfile::focalLengthChanged);
    connect(ui->aperture, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &LensProfile::apertureChanged);

}

LensProfile::~LensProfile() {
    delete ui;
}

QString LensProfile::buildDisplayString(const QString &stringOpt1,
                                        const QString &stringOpt2,
                                        float minValue, float maxValue,
                                        QDoubleSpinBox *spinBox) {
    QString outString;
    if (minValue == maxValue or maxValue == 0) {
        outString = stringOpt1.arg(minValue);
    } else {
        outString = stringOpt2.arg(minValue).arg(maxValue);
    }
    spinBox->setValue(minValue);
    return outString;
}

void LensProfile::loadProfile() {
    if (ui->chooseProfile->currentIndex() == 0) {
        Q_EMIT profileChosen(QLatin1String(""), QLatin1String(""));
        ui->aperture->setValue(0.0);
        ui->focalLen->setValue(0.0);
        ui->apertureMinMax->clear();
        ui->focalLenMinMax->clear();
        return;
    }
    auto[lensName, lensMaker] = currentTextToLens();

    auto lens = findLens(lensName, lensMaker);
    if (lens) {
        auto apertureMinMax = buildDisplayString("F/%1", "F/%1-%2",
                                                 lens->MinAperture, lens->MaxAperture,
                                                 ui->aperture);
        ui->apertureMinMax->setText(apertureMinMax);

        auto focalMinMax = buildDisplayString("%1 mm", "%1-%2 mm",
                                              lens->MinFocal, lens->MaxFocal,
                                              ui->focalLen);
        ui->focalLenMinMax->setText(focalMinMax);
        Q_EMIT profileChosen(lensName, lensMaker);
    } else {
        qDebug() << "No camera found for" << lensName << lensMaker;
    }
}

void LensProfile::populateProfileList() {
    qInfo() << "Lens list populated.";
    ui->chooseProfile->clear();
    if (auto db = lensfunDB.lock()) {
        auto lenses = db->GetLenses();

        for (auto i = 0; lenses[i]; i++) {
            auto lens = lenses[i];
            auto model = QString::fromLatin1(lens->Model);
            auto maker = QString::fromLatin1(lens->Maker);
            ui->chooseProfile->addItem(maker + ": " + model);
            ui->chooseProfile->setItemData(i, model,
                                           Qt::ToolTipRole);

        }
        ui->chooseProfile->model()->sort(0, Qt::AscendingOrder);
    }
    ui->chooseProfile->insertItem(0, def_index1);
    ui->chooseProfile->insertSeparator(1);
    ui->chooseProfile->setCurrentIndex(0);
}

void LensProfile::populateProfileList(const QString &cameraName, const QString &cameraMaker) {
    if ((cameraName.isNull() and cameraMaker.isNull()) or
        (cameraName.isEmpty() and cameraMaker.isEmpty())) {
        qDebug() << "Camera info is empty, repopulating lens list.";
        LensProfile::populateProfileList();
        return;
    }

    ui->chooseProfile->clear();
    if (auto db = lensfunDB.lock()) {
        auto camera = findCamera(cameraName, cameraMaker);
        if (camera == nullptr) {populateProfileList(); return;}
        auto lenses = findLenses(camera);
        if (lenses == nullptr) {populateProfileList(); return;}
        for (auto i = 0; lenses[i]; i++) {
            auto lens = lenses[i];
            auto model = QString::fromLatin1(lens->Model);
            auto maker = QString::fromLatin1(lens->Maker);
            ui->chooseProfile->addItem(maker + ": " + model);
            ui->chooseProfile->setItemData(i, model,
                                           Qt::ToolTipRole);
        }
        ui->chooseProfile->model()->sort(0, Qt::AscendingOrder);
        lf_free(lenses);
    }
    ui->chooseProfile->insertItem(0, def_index1);
    ui->chooseProfile->insertSeparator(1);
    ui->chooseProfile->setCurrentIndex(0);

//    qDebug() << ui->chooseProfile->count();
    if (ui->chooseProfile->count() == 3){
        ui->chooseProfile->setCurrentIndex(2);
        loadProfile();
    } else{
        parentWidget()->raise();
    }
}

std::tuple<QString, QString> LensProfile::currentTextToLens() {
    auto lensString = ui->chooseProfile->currentText();
    return parseLensText(lensString);
}

std::tuple<QString, QString> LensProfile::parseLensText(const QString &indexText){
    auto lensMaker = indexText.section(":", 0, 0);
    auto lensName = indexText.section(":", 1, -1).trimmed();
    return {lensName, lensMaker};
}


void LensProfile::setProfileByName(const QString &name) {

}

void LensProfile::setAOV(double d) {
    ui->angleOfView->setValue(d);
    Q_EMIT angleOfViewChanged(d);
}

double LensProfile::getAOV() {
    return ui->angleOfView->value();
}

void LensProfile::set35FocalLen(double d) {
    ui->focalLen_ff->setValue(d);
    Q_EMIT ffLengthChanged(d);
}

double LensProfile::get35FocalLen() {
    return ui->focalLen_ff->value();
}

double LensProfile::getFocalLen() {
    return ui->focalLen->value();
}

void LensProfile::setFocalLen(double d) {
    ui->focalLen->setValue(d);
    Q_EMIT focalLengthChanged(d);
}

double LensProfile::getAperture() {
    return ui->aperture->value();
}

void LensProfile::setAperture(double d) {
    ui->aperture->setValue(d);
    Q_EMIT apertureChanged(d);
}



