//
// Created by arnoldn on 20/05/2021.
//

#include "CameraWatchdog.hpp"
#include "messages.hpp"
#include <QDebug>
#include <memory>
#include <iostream>
#include <QCoreApplication>

CameraWatchdog::CameraWatchdog(QWidget *parent) :
        QWidget(parent), cached_files(), active(false) {
    setObjectName("CameraWatchdog");

    commonDialog = new WIA::CommonDialog(this);
    commonDialog->setObjectName(QStringLiteral("CommonDialog"));

    deviceManager = new WIA::DeviceManager(this);
    deviceManager->setObjectName(QStringLiteral("DeviceManager"));

    imageListener = new WIA::DeviceManager(this);
    imageListener->setObjectName(QStringLiteral("ImageListener"));

    deviceManager->RegisterEvent(wiaEventDeviceConnected, "*");
    deviceManager->RegisterEvent(wiaEventDeviceDisconnected, "*");
    imageListener->RegisterEvent(wiaEventItemCreated, "*");
    imageListener->RegisterEvent(wiaEventItemDeleted, "*");

    connect(deviceManager, SIGNAL(OnEvent(QString, QString, QString)),
            this, SLOT(onDeviceEvent(QString, QString, QString)), Qt::DirectConnection);

    connect(imageListener, SIGNAL(OnEvent(QString, QString, QString)),
            this, SLOT(onDeviceEvent(QString, QString, QString)), Qt::DirectConnection);


    connect(this, &CameraWatchdog::itemCreated, this, &CameraWatchdog::handleImageCreated);

    connect(this, &CameraWatchdog::cameraChosen, this,
            [=](const QString &string) { qInfo() << "Camera Chosen:" << string; });
    connect(this, &CameraWatchdog::destinationSet, this,
            [=](const QString &dest) { qInfo() << "Camera output destination:" << dest; });
    connect(this, &CameraWatchdog::watchStarted, this,
            [=]() { qInfo() << "Monitoring camera for new images."; });
    connect(this, &CameraWatchdog::watchStopped, this,
            [=]() { qInfo() << "Camera monitoring stopped."; });
    connect(this, &CameraWatchdog::imageReceived, this,
            [=](const QString &img1, const QString &img2) {
        qInfo() << "Images received from camera:\n" << img1 << "\n" << img2; });

}

CameraWatchdog::CameraWatchdog(QWidget *parent, const fs::path &dest) : CameraWatchdog(parent) {
    destination = dest;
}

CameraWatchdog::CameraWatchdog(QWidget *parent, const QString &dest) : CameraWatchdog(parent) {
    destination = fs::path(dest.toStdString());
}


bool CameraWatchdog::chooseCamera() {
    if (not camera.isNull()) return false;
    if (not cameraDeviceID.isNull() or not cameraDeviceID.isEmpty()) return false;
    if (not cameraDeviceName.isNull() or not cameraDeviceName.isEmpty()) return false;

    int num_wiaDevices = deviceManager->DeviceInfos()->Count();
    if (num_wiaDevices <= 0){
        Messages::info_msg(this, "WIA supported cameras not found.");
        return false;
    }

    auto tmp_cam = commonDialog->ShowSelectDevice(
            WIA::WiaDeviceType::UnspecifiedDeviceType, true, false);

    if (tmp_cam) {
        std::unique_ptr<WIA::Device> cam(new WIA::Device(tmp_cam));
        QString camID = cam->DeviceID();

        QVariant name = "Name";
        auto properties = std::make_unique<WIA::Properties>(cam->Properties());
        auto property = std::make_unique<WIA::Property>(properties->Item(name));
        name = property->Value();

        cameraDeviceID = camID;
        cameraDeviceName = name.toString();
        Q_EMIT cameraChosen(cameraDeviceName);
        return true;
    }
    if (camera.isNull())
        Messages::warning_msg(this, "WIA connection could not be established.");
    return false;


}

bool CameraWatchdog::testCameraImages() {
    if (camera.isNull()) return false;

    auto camera_items = std::make_unique<WIA::Items>(camera->Items());
    QVariant property_name = "Item Name";
    QString item_name;
    if (camera_items->Count() > 0) {
        auto camera_item = std::make_unique<WIA::Item>(camera_items->Item(1));
        auto item_properties = std::make_unique<WIA::Properties>(camera_item->Properties());
        auto property = std::make_unique<WIA::Property>(item_properties->Item(property_name));
        item_name = property->Value().toString();
        if (!item_name.isEmpty())
            return true;
        else
            return false;
    }
    return false;
}

void CameraWatchdog::connectToCamera(const QString &deviceID) {
    QVariant DeviceID{deviceID};
    auto deviceInfos = std::make_unique<WIA::DeviceInfos>(deviceManager->DeviceInfos());
    auto item = std::make_unique<WIA::DeviceInfo>(deviceInfos->Item(DeviceID));
    camera = new WIA::Device(item->Connect());
    camera->setParent(this);
    camera->setObjectName(getCameraName());
    connect(camera, &WIA::Device::destroyed, this, [=, this]() {
        if (not camera.isNull()) qDebug() << camera->objectName() << "camera closed."; });
}



QString CameraWatchdog::getCameraName() {
    if (not camera.isNull()) {
        QVariant name = "Name";
        auto properties = std::make_unique<WIA::Properties>(camera->Properties());
        auto property = std::make_unique<WIA::Property>(properties->Item(name));
        name = property->Value();
        return name.toString();
    }
    return {};
}

void CameraWatchdog::wiaException(int code, const QString &source, const QString &description, const QString &help) {
    qDebug() << "Code:" << code;
    qDebug() << "Source:" << source;
    qDebug() << "Description:" << description;
    qDebug() << "Help:";
    qDebug() << help;
}

void CameraWatchdog::onDeviceEvent(const QString &EventID, const QString &DeviceID, const QString &ItemID) {
    if (EventID == wiaEventItemCreated) {
        if (DeviceID == cameraDeviceID)
            handleImageCreated(ItemID);
    } else if (EventID == wiaEventItemDeleted) {
        Q_EMIT itemDeleted();
    } else if (EventID == wiaEventDeviceDisconnected) {
        qWarning() << "Device Disconnected:" << getCameraName() << DeviceID;
        if (DeviceID == cameraDeviceID and not camera.isNull()) {
            shutDownCamera();
            Q_EMIT deviceDisconnected();
            QCoreApplication::processEvents();
        }
    } else if (EventID == wiaEventDeviceConnected) {
        qInfo() << "Device connected:" << getCameraName() << "id:" << DeviceID;
        if (DeviceID == cameraDeviceID)
            connectToCamera(DeviceID);
        Q_EMIT deviceConnected();
    } else
        qWarning() << "Unknown camera (WIA) event:" << EventID;
}


CameraWatchdog::~CameraWatchdog() = default;

void CameraWatchdog::handleImageCreated(const QString &itemID) {
    if (!active)
        return;

    QVariant property_name;

    auto item = std::make_unique<WIA::Item>(camera->GetItem(itemID));
    auto item_properties = std::make_unique<WIA::Properties>(item->Properties());

    property_name = "Item Flags";
    auto item_flags = std::make_unique<WIA::Property>(item_properties->Item(property_name));
    int item_flag = item_flags->Value().toInt();

    property_name = "Item Name";
    auto item_name = std::make_unique<WIA::Property>(item_properties->Item(property_name));
    QString itemName = item_name->Value().toString();

    if (int(item_flag & WIA::WiaItemFlag::FolderItemFlag) == WIA::WiaItemFlag::FolderItemFlag) {
    }
    if (int(item_flag & WIA::WiaItemFlag::TransferItemFlag) == WIA::WiaItemFlag::TransferItemFlag) {
        auto img = commonDialog->ShowTransfer(camera->GetItem(itemID)).value<QAxObject *>();

        auto save_destination = destination / itemName.toStdString();
        save_destination.replace_extension(".jpg");
        img->dynamicCall("SaveFile(const QString&)", QString::fromStdString(save_destination.string()));

        cached_files.push(save_destination.string());
        if (cached_files.size() == 2) {
            auto img1 = cached_files.front();
            cached_files.pop();
            auto img2 = cached_files.front();
            Q_EMIT imageReceived(QString::fromStdString(img1),
                                 QString::fromStdString(img2));
        }
    }
}

void CameraWatchdog::startWatch() {
    if (not (cameraDeviceID.isEmpty() or cameraDeviceID.isNull())) {
        connectToCamera(cameraDeviceID);
        active = true;
        Q_EMIT watchStarted();
    } else {
        active = false;
        Messages::warning_msg(this, "Camera monitoring could not be started.");
    }

}

void CameraWatchdog::stopWatch() {
    active = false;
    Q_EMIT watchStopped();
}

void CameraWatchdog::shutDownCamera(){
    camera->deleteLater();
    cameraDeviceID.clear();
    cameraDeviceName.clear();
}

void CameraWatchdog::setDestination(const QString &dest) {
    destination = dest.toStdString();
    Q_EMIT destinationSet(dest);
}

bool CameraWatchdog::isActive() const {
    return active;
}

bool CameraWatchdog::hasCamera() {
    if (camera.isNull())
        return false;
    if (cameraDeviceID.isEmpty() or cameraDeviceID.isNull())
        return false;
    if (cameraDeviceName.isEmpty() or cameraDeviceName.isNull())
        return false;
    return true;
}
