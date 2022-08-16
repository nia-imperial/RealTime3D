//
// Created by Nic on 23/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DEMBehaviour.h" resolved

#include "dembehaviour.h"
#include "ui_DEMBehaviour.h"
#include <QDebug>

DemBehaviour::DemBehaviour(QWidget *parent) :
        SettingsForm(desc, parent),
        ui(new Ui::DemBehaviour) {
    ui->setupUi(this);

    allCheckBoxes = findChildren<QCheckBox *>();
    for (auto box: allCheckBoxes) {
        connect(box, &QCheckBox::toggled, this, &DemBehaviour::reportChanges);
    }

}

SettingDescriptor DemBehaviour::desc{ // NOLINT(cert-err58-cpp)
        QStringLiteral(u"settings/dem"), QStringLiteral(u"DEM Generation")};

DemBehaviour::~DemBehaviour() {
    delete ui;
}

void DemBehaviour::readSettings() {
    for (auto box: allCheckBoxes) {
        box->setChecked(settings.value(box->objectName(), true).toBool());
    }
}

void DemBehaviour::writeSettings() {

    for (auto box: allCheckBoxes) {
        settings.setValue(box->objectName(), box->isChecked());
    }
}

bool DemBehaviour::allow_autoLoadSettings() {
    return getSettingValue(DemBehaviour::desc, QLatin1String("autoLoadSettings"), true).toBool();
}

bool DemBehaviour::allow_createOutputPaths() {
    return getSettingValue(DemBehaviour::desc, QLatin1String("createOutputPaths"), true).toBool();
}

bool DemBehaviour::allow_logPrefixAsParent() {
    return getSettingValue(DemBehaviour::desc, QLatin1String("logPrefixAsParent"), true).toBool();
}

bool DemBehaviour::allow_outputPrefixAsParent() {
    return getSettingValue(DemBehaviour::desc, QLatin1String("outputPrefixAsParent"), true).toBool();
}

bool DemBehaviour::allow_persistentSettings() {
    return getSettingValue(DemBehaviour::desc, QLatin1String("persistentSettings"), true).toBool();
}

void DemBehaviour::resetToDefault() {
    for (auto box: allCheckBoxes) {
        box->setChecked(settings.value(box->objectName(), true).toBool());
    }
}

