//
// Created by Nic on 10/03/2022.
//
#include "WatchdogIndicator.h"

auto indicator = QStringLiteral(
        "WatchdogIndicator::indicator {width: 10px; height: 10px; border-radius: 7px;}\n");
auto unchecked = QStringLiteral(
        "QRadioButton::indicator:unchecked {background-color: black; border: 2px solid gray;}\n");
auto checkedAmber = QStringLiteral(
        "QRadioButton::indicator::checked {background-color: darkorange; border: 2px solid gray;}\n");
auto checkedGreen = QStringLiteral(
        "QRadioButton::indicator::checked {background-color: lime; border: 2px solid gray;}\n");

WatchdogIndicator::WatchdogIndicator(QWidget *parent) :
        QRadioButton(parent), watchType(WatchType::DISABLED),
        disabledStyle(indicator + unchecked + checkedAmber),
        standbyStyle(indicator + unchecked + checkedAmber),
        activeStyle(indicator + unchecked + checkedGreen) {
    setToolTip(
            R"(<html><head/><body><p>Indicator for Image Watchdog module.</p><p>If a camera is selected, images will be copied to the location specified by Input Path.</p><p>If a camera is not selected, the module monitors the Input Path for new images.</p><p><span style=" color:#55ff00;">Green</span> = Actively watching camera device</p><p><span style=" color:#ff5500;">Amber</span> = Camera is chosen but monitoring is on standby</p><p><span style=" color:#7a7a7a;">Gray</span> = Disabled</p></body></html>)"
    );
    QRadioButton::setStyleSheet(disabledStyle);
    setChecked(false);
    setDisabled(true);

    setObjectName("WatchdogIndicator");
    setText("");

}

WatchType WatchdogIndicator::getWatchType() {
    return watchType;
}

void WatchdogIndicator::setWatchType(const WatchType &type) {
    switch (type) {
        case WatchType::DISABLED:
            setStyleSheet(disabledStyle);
            setChecked(false);
            break;
        case WatchType::STANDBY:
            setStyleSheet(standbyStyle);
            setChecked(true);
            break;
        case WatchType::ACTIVE:
            setStyleSheet(activeStyle);
            setChecked(true);
            break;
    }
    watchType = type;
}

WatchdogIndicatorWidget::WatchdogIndicatorWidget(QWidget *parent) :
        QWidget(parent) {
    hlayout = new QHBoxLayout(this);
    hlayout->setContentsMargins(6, 0, 6, 0);
    hlayout->setSpacing(0);
    setLayout(hlayout);

    connectionBtn = new QPushButton(this);
    connectionBtn->setText(QLatin1String("Connect to Camera"));
    connectionBtn->setFlat(true);
    hlayout->addWidget(connectionBtn);

    indicator = new WatchdogIndicator(this);
    hlayout->addWidget(indicator);

}

void WatchdogIndicatorWidget::setWatchType(const WatchType &type) const {
    indicator->setWatchType(type);
}


