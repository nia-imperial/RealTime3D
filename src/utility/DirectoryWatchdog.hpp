//
// Created by arnoldn on 20/05/2021.
//

#ifndef REALTIME3D_DIRECTORYWATCHDOG_HPP
#define REALTIME3D_DIRECTORYWATCHDOG_HPP


#include <QString>
#include <QWidget>
#include <filesystem>
#include <efsw/efsw.hpp>
#include <efsw/System.hpp>
#include <efsw/FileSystem.hpp>
#include <iostream>
#include <QDebug>
#include <QImageReader>
#include <QFile>
#include <queue>
#include "messages.hpp"

namespace fs = std::filesystem;

class ListenerSignals : public QObject {
Q_OBJECT

public:
    ListenerSignals() = default;

Q_SIGNALS:

    void fileAdded(const QString &dir, const QString &filename,
                   const QString &oldFilename);

    void fileDeleted(const QString &dir, const QString &filename,
                     const QString &oldFilename);

    void fileModified(const QString &dir, const QString &filename,
                      const QString &oldFilename);

    void fileMoved(const QString &dir, const QString &filename,
                   const QString &oldFilename);

};

/// Inherits from the listener abstract class, and implements the file action handler
class UpdateListener : public efsw::FileWatchListener {

public:
    UpdateListener();

    ListenerSignals listenerSignals;
    fs::path dest;

    void handleFileAction(efsw::WatchID watchid,
                          const std::string &dir,
                          const std::string &filename,
                          efsw::Action action,
                          std::string oldFilename) override;

};

class DirectoryWatchdog : public QObject {
Q_OBJECT
    fs::path destinationDirectory;
    fs::path watchingDirectory;
    std::unique_ptr<efsw::FileWatcher> fileWatcher;
    std::shared_ptr<UpdateListener> listener;
    efsw::WatchID watchId;
    std::queue<std::string> cached_files;
    int imagePairs;
    QImageReader *imageReader;
    bool active;

private Q_SLOTS:

    void copyFile(const QString &dir, const QString &filename, const QString &oldFileName);

    void pushPopFiles(const QString &dir, const QString &filename, const QString &oldFileName);

public:
    explicit DirectoryWatchdog(QWidget *parent = nullptr);

    DirectoryWatchdog(QWidget *parent, const QString &watchingDir);

    DirectoryWatchdog(QWidget *parent, const QString &watchingDir, const QString &destinationDir);

    QString getWatchedDir();

public Q_SLOTS:
    /// Set the directory to watch for files.
    void setWatchDir(const QString &dir);
    /// Set the directory where files will be coped to (optional).
    void setDestination(const QString &dir);

    void startWatch();

    void stopWatch();

    bool isActive() const;

Q_SIGNALS:

    void watchDirSet(const QString &dir);

    void destinationSet(const QString &dir);

    void watchStarted();

    void watchStopped();

    void imagePairReady(const QString &img1, const QString &img2);

};


#endif //REALTIME3D_DIRECTORYWATCHDOG_HPP
