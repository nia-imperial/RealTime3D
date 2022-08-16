//
// Created by arnoldn on 28/03/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_GeometricSettings.h" resolved

#include "geometricsettings.h"
#include "ui_GeometricSettings.h"


GeometricSettings::GeometricSettings(QWidget *parent) :
        SettingsForm(desc, parent), ui(new Ui::GeometricSettings) {
    ui->setupUi(this);
    addMethod(QLatin1String("Bicubic"), INTER_CUBIC,
              QLatin1String("bicubic interpolation"));
    addMethod(QLatin1String("Nearest Neighbour"), INTER_NEAREST,
              QLatin1String("nearest neighbour interpolation"));
    addMethod(QLatin1String("Bilinear"), INTER_LINEAR,
              QLatin1String("bilinear interpolation"));
    addMethod(QLatin1String("Pixel Area"), INTER_AREA,
              QLatin1String(
                      "resampling using pixel area relation. It may be a preferred method for image decimation, as it gives moire'-free results. But when the image is zoomed, it is similar to the Nearest Neighbour method."));
    addMethod(QLatin1String("Lanczos [8x8]"), INTER_LANCZOS4,
              QLatin1String("Lanczos interpolation over 8x8 neighborhood"));
    addMethod(QLatin1String("Linear [exact]"), INTER_LINEAR_EXACT,
              QLatin1String("Bit exact bilinear interpolation"));
    addMethod(QLatin1String("Nearest Neighbour [exact]"), INTER_NEAREST_EXACT,
              QLatin1String(
                      "Bit exact nearest neighbor interpolation. This will produce same results as the nearest neighbor method in PIL, scikit-image or Matlab."));
    addMethod(QLatin1String("None [Warp]"), WARP_FILL_OUTLIERS,
              QLatin1String("Fills all of the destination image pixels. If some of them correspond to outliers in the source image, they are set to zero."));

    connect(ui->methodSelect, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int index){
                ui->description->setText(methodDesc[methodLookup(index)]);
    });

    ui->methodSelect->setInsertPolicy(QComboBox::InsertAlphabetically);

    setMethod(INTER_CUBIC);
    ui->description->setText(methodDesc[INTER_CUBIC]);
}

void GeometricSettings::addMethod(const QString &text, InterpolationFlags flag, const QString &descript) {
    auto methodSelect = ui->methodSelect;
    methodSelect->addItem(text, flag);
    methodDesc[flag] = descript;
}

GeometricSettings::~GeometricSettings() {
    delete ui;
}

void GeometricSettings::writeSettings() {
    settings.setValue(
            ui->methodSelect->objectName(),
            static_cast<int>(methodLookup(ui->methodSelect->currentIndex()))
            );
}

void GeometricSettings::readSettings() {
    auto defMethod =  QVariant(static_cast<int>(INTER_CUBIC));
    auto savedMethod = settings.value(ui->methodSelect->objectName(), defMethod).toInt();
    setMethod(static_cast<InterpolationFlags>(savedMethod));
}

SettingDescriptor GeometricSettings::desc = {// NOLINT(cert-err58-cpp)
        QStringLiteral(u"settings/geometric"),
        QStringLiteral(u"Warp Controller")
};

void GeometricSettings::resetToDefault() {
    setMethod(INTER_CUBIC);
}

InterpolationFlags GeometricSettings::methodLookup(int idx) {
    return static_cast<InterpolationFlags>(ui->methodSelect->itemData(idx).toInt());
}

int GeometricSettings::getIndex(InterpolationFlags method){
    for (int i = 0; i < ui->methodSelect->count(); ++i) {
        if (methodLookup(i) == method)
            return i;
    }
    return -1;
}

void GeometricSettings::setMethod(InterpolationFlags method) {
    ui->methodSelect->setCurrentIndex(getIndex(method));
}

int GeometricSettings::getInterMethod() {
    auto defMethod =  QVariant(static_cast<int>(INTER_CUBIC));
    return getSettingValue(GeometricSettings::desc, "methodSelect", defMethod).toInt();
}
