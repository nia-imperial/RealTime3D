//
// Created by arnoldn on 26/02/2022.
//

#include "SettingsForm.h"

SettingsForm::SettingsForm(const SettingDescriptor &descriptor, QWidget *parent) :
        QWidget(parent),
        desc(descriptor),
        settingsLabel(new QListWidgetItem),
        settings(QSettings::UserScope),
        changed(false) {
    settingsLabel->setText(desc.label);
    settings.beginGroup(desc.group);

    // connect the init signal to reset the status of changed
    connect(this, &SettingsForm::init, this,
            [this]() {
                changed = false;
                readSettings();
            });

}

SettingsForm::~SettingsForm() = default;


void SettingsForm::reportChanges() {
    changed = true;
    Q_EMIT changesMade();
}

void SettingsForm::closeEvent(QCloseEvent *event) {
    settings.endGroup();
    QWidget::closeEvent(event);
}

bool SettingsForm::hasChanged() const {
    return changed;
}


QVariant SettingsForm::getSettingValue(const SettingDescriptor& m_desc, const QString& key, const QVariant& defValue, QSettings::Scope scope) {
    auto setting_ = QSettings(scope);
    setting_.beginGroup(m_desc.group);
    QVariant val = setting_.value(key, defValue);
    return val;
}

QVariant SettingsForm::getSettingValue(const SettingDescriptor& m_desc, const QString& key, const QVariant& defValue) {
    return getSettingValue(m_desc, key, defValue, QSettings::UserScope);
}

QVariant SettingsForm::getSettingValue(const SettingDescriptor& m_desc, const QString& key) {
    return getSettingValue(m_desc, key, QLatin1String(""), QSettings::UserScope);
}


QVariant SettingsForm::getValue(const SettingDescriptor &m_desc, const QString &key) {
    return settings.value(key);
}




