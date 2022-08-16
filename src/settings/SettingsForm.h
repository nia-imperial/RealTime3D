//
// Created by arnoldn on 26/02/2022.
//

#ifndef REALTIME3D_SETTINGSFORM_H
#define REALTIME3D_SETTINGSFORM_H

#include <QWidget>
#include <QSettings>
#include <QString>
#include <QCheckBox>
#include <QListWidgetItem>

struct SettingDescriptor {
    /// contains the name of the settings group
    QString group;
    /// the name to be shown on the list view
    QString label;

};

class SettingsForm : public QWidget {
Q_OBJECT

public:

    QListWidgetItem *settingsLabel;

    explicit SettingsForm(const SettingDescriptor& descriptor, QWidget *parent = nullptr);

    ~SettingsForm() override;

    /// returns status of if the settings have been changed or not
    [[nodiscard]] bool hasChanged() const;

    QVariant getValue(const SettingDescriptor& m_desc, const QString& key);



Q_SIGNALS:

    void init();

    void changesMade();

public Q_SLOTS:

    virtual void readSettings() = 0;

    /// Changes `changed` to true and emits the `changesMade` signal.
    void reportChanges();

    /// override to specify how settings are written.
    virtual void writeSettings() = 0;

    /// override to specify resetting settings to default.
    virtual void resetToDefault() = 0;

protected:
    void closeEvent(QCloseEvent *event) override;

    /// boolean to determine if changes have been made.
    bool changed;

    /// The QSettings used to hold information about the settings group.
    /// It is initialised to "settings/{desc.group}" upon creation.
    QSettings settings;

    static QVariant getSettingValue(const SettingDescriptor& m_desc, const QString& key, const QVariant& defValue,
                                    QSettings::Scope scope);
    static QVariant getSettingValue(const SettingDescriptor &m_desc, const QString &key, const QVariant& defValue);
    static QVariant getSettingValue(const SettingDescriptor &m_desc, const QString &key);

private:
    const SettingDescriptor& desc;


};


#endif //REALTIME3D_SETTINGSFORM_H
