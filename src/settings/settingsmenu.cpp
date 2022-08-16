//
// Created by Nic on 23/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_SettingsMenu.h" resolved

#include "settingsmenu.h"
#include "ui_SettingsMenu.h"
#include <QDebug>


SettingsMenu::SettingsMenu(QWidget *parent) :
        QDialog(parent), ui(new Ui::SettingsMenu), changesMade(false),
        settingsMap(){
    ui->setupUi(this);

    connect(ui->applyButton, &QPushButton::released,
            this, &SettingsMenu::confirmSettings);

    connect(this, &QDialog::accepted,
            this, &SettingsMenu::confirmSettings);

    connect(ui->listWidget, &QListWidget::itemSelectionChanged,
            this, &SettingsMenu::setSettingsItem);

    generalSettings = new GeneralSettings();
    addSettingsField(generalSettings);

    pathSettings = new PathSettings();
    addSettingsField(pathSettings);

    demBehaviour = new DemBehaviour();
    addSettingsField(demBehaviour);

    geometricSettings = new GeometricSettings();
    addSettingsField(geometricSettings);

}

SettingsMenu::~SettingsMenu() {
    delete ui;
}

void SettingsMenu::reportChanges() {
    changesMade = true;
    ui->applyButton->setEnabled(true);
}

void SettingsMenu::setSettingsItem() {
    int row = ui->listWidget->currentRow();
    clearLayout();
    ui->scrollAreaWidgetContents->layout()->addWidget(settingsMap.value(row));
}

void SettingsMenu::setSettingsField(int index) {
    ui->listWidget->setCurrentRow(index);
    setSettingsItem();
}

void SettingsMenu::confirmSettings() {
    if (not changesMade)
        return;

    Q_EMIT applySettings();
    Q_EMIT settingsApplied();
}

void SettingsMenu::clearLayout() {
    QLayoutItem *child;
    auto layout = ui->scrollAreaWidgetContents->layout();
    while ((child = layout->takeAt(0)) != nullptr) {
        child->widget()->setParent(nullptr);
        delete child;   // delete the layout item
    }
}

void SettingsMenu::showEvent(QShowEvent *event) {
    changesMade = false;
    ui->applyButton->setDisabled(true);
    Q_EMIT settingsShown();
    QDialog::showEvent(event);
}

void SettingsMenu::addSettingsField(SettingsForm *settingsForm) {
    auto idx = ui->listWidget->count();
    ui->listWidget->insertItem(idx, settingsForm->settingsLabel);
    settingsMap.insert(idx, settingsForm);

    connect(settingsForm, &SettingsForm::changesMade,
            this, &SettingsMenu::reportChanges);

    connect(this, &SettingsMenu::settingsShown,
            settingsForm, &SettingsForm::init);

    connect(this, &SettingsMenu::applySettings,
            settingsForm,
            [=]() {
                if (settingsForm->hasChanged())
                    settingsForm->writeSettings();
            });
}



