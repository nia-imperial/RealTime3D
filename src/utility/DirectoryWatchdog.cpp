//
// Created by arnoldn on 20/05/2021.
//

#include "DirectoryWatchdog.hpp"

DirectoryWatchdog::DirectoryWatchdog(QWidget *parent) :
        QObject(parent),
        fileWatcher(std::make_unique<efsw::FileWatcher>()),
        listener(std::make_shared<UpdateListener>()),
        imageReader(new QImageReader()),
        active(false),
        imagePairs(0),
        watchId(), cached_files() {
    setObjectName("DirectoryWatchdog");

    connect(this, &DirectoryWatchdog::watchDirSet, [=](const QString &dir) {
        qInfo() << "Monitoring directory:" << dir;
    });
    connect(this, &DirectoryWatchdog::destinationSet, [=](const QString &dir) {
        qInfo() << "New files in monitored directory will be copied to:" << dir;
    });
}

DirectoryWatchdog::DirectoryWatchdog(QWidget *parent, const QString &watchingDir) : DirectoryWatchdog(parent) {
    setWatchDir(watchingDir);
}

DirectoryWatchdog::DirectoryWatchdog(QWidget *parent, const QString &watchingDir, const QString &destinationDir) :
        DirectoryWatchdog(parent, watchingDir) {
    setDestination(destinationDir);
}

void DirectoryWatchdog::copyFile(const QString &dir, const QString &filename, const QString &oldFileName) {
    if (!isActive()) return;
    if (destinationDirectory.empty()) return;

    auto file_path = fs::path(dir.toStdString()) / filename.toStdString();
    auto new_destination = destinationDirectory / filename.toStdString();
    file_path.make_preferred();
    new_destination.make_preferred();

    qInfo() << "Original file path: " << QString::fromStdString(file_path.string());
    qInfo() << "New destination path: " << QString::fromStdString(new_destination.string());
    std::error_code ec;
    fs::copy_file(file_path, new_destination, fs::copy_options::overwrite_existing, ec);
    if (ec.value() != 0)
        Messages::warning_msg(nullptr, QString("Code %1, %2")
                .arg(ec.value())
                .arg(QString::fromStdString(ec.message())));
}

bool DirectoryWatchdog::isActive() const {
    return active;
}

void DirectoryWatchdog::pushPopFiles(const QString &dir, const QString &filename, const QString &oldFileName) {
    if (!isActive()) return;
    auto file_path = fs::path(dir.toStdString()) / filename.toStdString();
    file_path.make_preferred();

    imageReader->setFileName(QString::fromStdString(file_path.string()));
    if (!imageReader->canRead())
        return;

    if (!cached_files.empty())
        if (file_path.string() == cached_files.back())
            return;

    cached_files.push(file_path.string());
    if (cached_files.size() == 2) {
        auto img1 = cached_files.front();
        cached_files.pop();
        auto img2 = cached_files.front();
        Q_EMIT imagePairReady(QString::fromStdString(img1),
                              QString::fromStdString(img2));
        imagePairs += 1;
    }
}

void DirectoryWatchdog::setWatchDir(const QString &dir) {
    if (dir.isEmpty() or dir.isNull()) return;

    if (!fileWatcher->directories().empty())
        fileWatcher->removeWatch(watchId);

    watchingDirectory = dir.toStdString();

    if (exists(watchingDirectory)) {
        watchId = fileWatcher->addWatch(watchingDirectory.string(), listener.get(), false);
        Q_EMIT watchDirSet(dir);
    }

}

void DirectoryWatchdog::setDestination(const QString &dir) {
    destinationDirectory = dir.toStdString();
    listener->dest = dir.toStdString();
    if (!fs::exists(destinationDirectory))
        fs::create_directory(destinationDirectory);
    connect(&listener->listenerSignals, &ListenerSignals::fileAdded,
            this, &DirectoryWatchdog::copyFile,
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    Q_EMIT destinationSet(dir);
}

void DirectoryWatchdog::startWatch() {
    active = true;
    fileWatcher->watch();
    connect(&listener->listenerSignals, &ListenerSignals::fileAdded, this, &DirectoryWatchdog::pushPopFiles,
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    connect(&listener->listenerSignals, &ListenerSignals::fileModified, this, &DirectoryWatchdog::pushPopFiles,
            static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));

    Q_EMIT watchStarted();
}

void DirectoryWatchdog::stopWatch() {
    fileWatcher->removeWatch(watchId);
    if (!destinationDirectory.empty())
        disconnect();
    active = false;
    Q_EMIT watchStopped();
}

QString DirectoryWatchdog::getWatchedDir() {
    return QString::fromStdString(watchingDirectory.string());
}

UpdateListener::UpdateListener() :
        efsw::FileWatchListener(), listenerSignals(), dest() {

}


void UpdateListener::handleFileAction(efsw::WatchID watchid,
                                      const std::string &dir,
                                      const std::string &filename,
                                      efsw::Action action,
                                      std::string oldFilename) {
    auto file_path = fs::path(dir) / filename;
    auto new_destination = dest / filename;
    file_path.make_preferred();
    new_destination.make_preferred();
    switch (action) {
        case efsw::Actions::Add:
            Q_EMIT listenerSignals.fileAdded(
                    QString::fromStdString(dir),
                    QString::fromStdString(filename),
                    QString::fromStdString(oldFilename)
            );
            break;
        case efsw::Actions::Delete:
            Q_EMIT listenerSignals.fileDeleted(
                    QString::fromStdString(dir),
                    QString::fromStdString(filename),
                    QString::fromStdString(oldFilename)
            );
            break;
        case efsw::Actions::Modified:
            Q_EMIT listenerSignals.fileModified(
                    QString::fromStdString(dir),
                    QString::fromStdString(filename),
                    QString::fromStdString(oldFilename)
            );
            break;
        case efsw::Actions::Moved:
            Q_EMIT listenerSignals.fileMoved(
                    QString::fromStdString(dir),
                    QString::fromStdString(filename),
                    QString::fromStdString(oldFilename)
            );
            break;
        default:
            std::cout << "Should never happen!" << std::endl;
    }
}

