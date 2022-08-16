//
// Created by arnoldn on 20/05/2021.
//

#ifndef MYDSTL_CAMERAWATCHDOG_HPP
#define MYDSTL_CAMERAWATCHDOG_HPP


#include <QObject>
#include <QWidget>
#include <QPointer>
#include <filesystem>
#include <queue>
#include "../../libs/wia/wiaaut.h"

namespace fs = std::filesystem;
typedef QPointer<WIA::CommonDialog> WiaCommonDialogPtr;
typedef QPointer<WIA::DeviceManager> WiaDeviceManagerPtr;
typedef QPointer<WIA::Device> WiaDevicePtr;


class CameraWatchdog : public QWidget {
    Q_OBJECT

    const QString wiaEventItemCreated = QStringLiteral("{4C8F4EF5-E14F-11D2-B326-00C04F68CE61}");
    const QString wiaEventItemDeleted = QStringLiteral("{1D22A559-E14F-11D2-B326-00C04F68CE61}");
    const QString wiaEventDeviceConnected = QStringLiteral("{A28BBADE-64B6-11D2-A231-00C04FA31809}");
    const QString wiaEventDeviceDisconnected = QStringLiteral("{143E4E83-6497-11D2-A231-00C04FA31809}");

    const QString wiaFormatBMP = QStringLiteral("{B96B3CAB-0728-11D3-9D7B-0000F81EF32E}"); // FormatID for the Windows BMP format.
    const QString wiaFormatPNG = QStringLiteral("{B96B3CAF-0728-11D3-9D7B-0000F81EF32E}"); // FormatID for the PNG format.
    const QString wiaFormatGIF = QStringLiteral("{B96B3CB0-0728-11D3-9D7B-0000F81EF32E}"); // FormatID for the GIF format.
    const QString wiaFormatJPEG = QStringLiteral("{B96B3CAE-0728-11D3-9D7B-0000F81EF32E}"); // FormatID for the JPEG format.
    const QString wiaFormatTIFF = QStringLiteral("{B96B3CB1-0728-11D3-9D7B-0000F81EF32E}"); // FormatID for the TIFF format.

    WiaCommonDialogPtr commonDialog;
    WiaDeviceManagerPtr deviceManager;
    WiaDeviceManagerPtr imageListener;
    WiaDevicePtr camera;

    QString cameraDeviceID;
    QString cameraDeviceName;
    std::queue<std::string> cached_files;

    fs::path destination;
    bool testCameraImages();
    bool active;

private Q_SLOTS:
    void handleImageCreated(const QString &itemID);
    void onDeviceEvent(const QString&, const QString&, const QString&);
    static void wiaException(int code, const QString &source, const QString &description, const QString &help);

public:
    explicit CameraWatchdog(QWidget *parent = nullptr);
    CameraWatchdog(QWidget *parent, const fs::path &dest);
    CameraWatchdog(QWidget *parent, const QString &dest);
    ~CameraWatchdog() override;

    QString getCameraName();
    void connectToCamera(const QString &deviceID);

public Q_SLOTS:
    void setDestination(const QString &dest);
    void startWatch();
    void stopWatch();
    bool chooseCamera();
    [[nodiscard]] bool isActive() const;
    bool hasCamera();
    void shutDownCamera();

Q_SIGNALS:
    void cameraChosen(const QString &name);
    void destinationSet(const QString &dest);
    void watchStarted();
    void watchStopped();
    void watchingCamera(bool);
    void imageReceived(const QString &img1, const QString &img2);

    void itemCreated(const QString &item);
    void itemDeleted();
    void deviceConnected();
    void deviceDisconnected();
};


#endif //MYDSTL_CAMERAWATCHDOG_HPP
