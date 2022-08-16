//
// Created by Nic on 26/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_GeneralSettings.h" resolved

#include "generalsettings.h"
#include "ui_GeneralSettings.h"


GeneralSettings::GeneralSettings(QWidget *parent) :
        SettingsForm(desc, parent),
        ui(new Ui::GeneralSettings) {
    ui->setupUi(this);

    allCheckBoxes = findChildren<QCheckBox *>();
    for (auto box : allCheckBoxes){
        connect(box, &QCheckBox::toggled,
                this, &SettingsForm::reportChanges);
    }
}

GeneralSettings::~GeneralSettings() {
    delete ui;
}

void GeneralSettings::readSettings() {
    for (auto box : allCheckBoxes){
        box->setChecked(settings.value(box->objectName(), false).toBool());
    }
}

void GeneralSettings::writeSettings() {
    for (auto box : allCheckBoxes){
        settings.setValue(box->objectName(), box->isChecked());
    }
}

SettingDescriptor GeneralSettings::desc = {// NOLINT(cert-err58-cpp)
    QStringLiteral(u"settings/general"), QStringLiteral(u"General Settings")};

QString GeneralSettings::openGL_key() {
    return QStringLiteral(u"opengl");
}

bool GeneralSettings::getOpenGLValue() {
    return getSettingValue(GeneralSettings::desc, openGL_key(), false).toBool();
}

void GeneralSettings::resetToDefault() {
    for (auto box : allCheckBoxes){
        box->setChecked(false);
    }
}



