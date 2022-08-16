//
// Created by Nic on 28/02/2022.
//

#include "DeviceProfile.h"
#include <QDebug>

DeviceProfile::DeviceProfile(const std::shared_ptr<lfDatabase> &ltd, QWidget *parent) :
        lensfunDB(ltd),
        def_index1(QStringLiteral("Choose Profile")) {

}

const lfCamera **DeviceProfile::findCameras(const QString &cameraName) {
    auto db = lensfunDB.lock();
    if (db.use_count() == 0) {
        qDebug() << "Database not available for findCameras(1).";
        return nullptr;
    }

    auto cameras = db->FindCameras(
            nullptr,
            cameraName.toStdString().c_str()
    );
    if (cameras) return cameras;
    lf_free(cameras);
    return nullptr;
}

const lfCamera **DeviceProfile::findCameras(const QString &cameraName, const QString &cameraMaker) {
    auto db = lensfunDB.lock();
    if (db.use_count() == 0) {
        qDebug() << "Database not available for findCameras(2).";
        return nullptr;
    }

    auto cameras = db->FindCameras(
            cameraMaker.toStdString().c_str(),
            cameraName.toStdString().c_str()
    );
    if (cameras) return cameras;
    lf_free(cameras);
    return nullptr;
}

const lfCamera *DeviceProfile::findCamera(const QString &cameraName) {
    if (cameraName.isEmpty() or cameraName.isNull()) return nullptr;
    auto db = lensfunDB.lock();
    if (db.use_count() == 0) {
        qDebug() << "Database not available for findCamera(1).";
        return nullptr;
    }

    auto cameras = findCameras(cameraName);
    if (cameras) {
        auto camera = cameras[0];
        lf_free(cameras);
        return camera;
    }
    lf_free(cameras);
    return nullptr;
}

const lfCamera *DeviceProfile::findCamera(const QString &cameraName, const QString &cameraMaker) {
    auto cameras = findCameras(cameraName, cameraMaker);
    if (cameras) {
        auto camera = cameras[0];
        lf_free(cameras);
        return camera;
    }
    lf_free(cameras);
    return nullptr;
}


const lfLens **DeviceProfile::findLenses(const QString &lensName, const QString &lensMaker) {
    auto db = lensfunDB.lock();
    if (db.use_count() == 0) {
        qDebug() << "Database not available for findLenses(1).";
        return nullptr;
    }

    auto lenses = db->FindLenses(
            nullptr,
            lensMaker.toStdString().c_str(),
            lensName.toStdString().c_str()
    );
    if (lenses) return lenses;
    lf_free(lenses);
    return nullptr;
}

const lfLens **DeviceProfile::findLenses(const lfCamera *camera) {
    auto db = lensfunDB.lock();
    if (db.use_count() == 0) {
        qDebug() << "Database not available for findLenses(1).";
        return nullptr;
    }

    auto lenses = db->FindLenses(camera, nullptr, nullptr);
    if (lenses) return lenses;
    lf_free(lenses);
    return nullptr;
}

const lfLens *DeviceProfile::findLens(const QString &lensName, const QString &lensMaker) {
    auto lenses = findLenses(lensName, lensMaker);
    if (lenses) {
        auto lens = lenses[0];
        lf_free(lenses);
        return lens;
    }
    lf_free(lenses);
    return nullptr;
}

const lfLens *DeviceProfile::findLens(const lfCamera *camera) {
    auto lenses = findLenses(camera);
    if (lenses) {
        auto lens = lenses[0];
        lf_free(lenses);
        return lens;
    }
    lf_free(lenses);
    return nullptr;
}

