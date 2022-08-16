//
// Created by Nic on 10/03/2022.
//

#ifndef REALTIME3D_WATCHDOGINDICATOR_H
#define REALTIME3D_WATCHDOGINDICATOR_H

#include <QWidget>
#include <QRadioButton>
#include <QWidgetAction>
#include <QHBoxLayout>
#include <QPushButton>


enum class WatchType {
    DISABLED, STANDBY, ACTIVE
};

class WatchdogIndicator : public QRadioButton {
Q_OBJECT
    WatchType watchType;
    const QString disabledStyle;
    const QString standbyStyle;
    const QString activeStyle;
public:
    explicit WatchdogIndicator(QWidget *parent = nullptr);
    WatchType getWatchType();

public Q_SLOTS:
    void setWatchType(const WatchType& type);

};

class WatchdogIndicatorWidget : public QWidget {
Q_OBJECT

    QHBoxLayout *hlayout;

public:
    explicit WatchdogIndicatorWidget(QWidget *parent = nullptr);

    QPushButton *connectionBtn;
    WatchdogIndicator *indicator;

public Q_SLOTS:
    void setWatchType(const WatchType& type) const;

};


#endif //REALTIME3D_WATCHDOGINDICATOR_H
