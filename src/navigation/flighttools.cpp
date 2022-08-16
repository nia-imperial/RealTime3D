//
// Created by Nic on 13/03/2022.
//

#include "flighttools.h"
#include "maininterface.h"
#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QProgressDialog>
#include "waypointer_io/images.hpp"
#include <QApplication>
#include "layerpanel.h"
#include "../settings/path_settings/pathsettings.h"
#include "../utility/pyscriptcaller.h"


FlightTools::FlightTools(QWidget *parent,
                         QAction *actionImageViewMatching,
                         QAction *actionSimulateImage,
                         QAction *actionSimulateFlightImages,
                         QAction *actionConnectToCamera,
                         QStatusBar *ui_statusBar,
                         QTabWidget *ui_editorTabs,
                         Workspace *parent_workspace,
                         LayerPanel *parent_layerPanel) :
        QObject(parent),
        workspace(parent_workspace),
        layerPanel(parent_layerPanel),
        editorTabs(ui_editorTabs),
        imageCount(0),
        imagesDirLabel(new QLabel("None")),
        watcher(new QFileSystemWatcher(this)),
        imageDirIsSet(false) {

    connect(actionImageViewMatching, &QAction::triggered,
            this, [this]() { imageViewMatching(); });

    connect(actionConnectToCamera, &QAction::triggered,
            this, &FlightTools::connectToCamera);

    connect(actionSimulateFlightImages, &QAction::triggered,
            this, [this]() { simulateFlightImages(); });
    connect(actionSimulateImage, &QAction::triggered,
            this, [this]() { addSimulatedImage(); });

    ui_statusBar->addPermanentWidget(new QLabel(QLatin1String("Folder Watched")));
    ui_statusBar->addPermanentWidget(imagesDirLabel);

    connect(watcher, &QFileSystemWatcher::fileChanged,
            this, &FlightTools::fn_fileChanged);
    connect(watcher, &QFileSystemWatcher::directoryChanged,
            this, &FlightTools::fn_DirChanged);

}

QStringList FlightTools::NameFilters() {
    return {QLatin1String("*.bmp *.dib "),
            QLatin1String("*.gif "),
            QLatin1String("*.heic *.heif "),
            QLatin1String("*.jpeg *.jpg "),
            QLatin1String("*.png "),
            QLatin1String("*.tiff *.tif"),
            QLatin1String("*.webp"),
            QStringLiteral("*.ARW")};
}

void FlightTools::fn_fileChanged(const QString &path) {
//    qDebug() << "file changed:" << path;
}

void FlightTools::fn_DirChanged(const QString &path) {
    if (not QFileInfo::exists(path)) return;
    qInfo() << "Change in folder:" << path;
    if (imageDirIsSet) {
        auto imagesDir = QDir(imagesDirectory);
        imagesDir.setNameFilters(NameFilters());
        imagesDir.setSorting(QDir::Time);
        auto files = imagesDir.entryList();
        if (files.isEmpty()) return;
        auto newImage = imagesDir.filePath(files.at(0));
        addFlightImage(newImage, 0, 0);
    }
}

bool FlightTools::baseMapItemOk() {
    auto basePixmap = layerPanel->currentBaseMap->pixmap();
    if (basePixmap.isNull())
        return false;
    return true;
}

bool FlightTools::imageDirSet() const {
    if (imagesDirectory.isEmpty() or imagesDirectory.isNull())
        return false;
    return true;
}

void FlightTools::simulateFlightImages(int w, int h) {
    // extra
}

void FlightTools::addFlightImage(const QString &filePath, int xPos, int yPos) {
    if (not baseMapItemOk()) {
        QMessageBox::warning(dynamic_cast<QWidget *>(parent()),
                             "Warning", "Base map must be set.", QMessageBox::Ok);
        return;
    }
    auto [flightImage, imageFilePath] = waypointer::io::readPhoto(filePath);
    if (not flightImage.isNull()) {
        editorTabs->setCurrentIndex(1);
        if (layerPanel->targetLayer == nullptr)
            layerPanel->targetLayer = layerPanel->addLayer("Target Points Layer");

        if (xPos == 0 or yPos == 0) {
            // get waypoints
            auto current_idx = layerPanel->targetLayer->rowCount();
            auto waypointView = parent()->findChild<QTableView *>("waypointView");
            auto waypointModel = dynamic_cast<QStandardItemModel *>(waypointView->model());
            QStandardItem *item;
            if (xPos == 0) {
                item = waypointModel->item(current_idx, 0);
                xPos = item->data(Qt::DisplayRole).toInt();
            }
            if (yPos == 0) {
                item = waypointModel->item(current_idx, 1);
                yPos = item->data(Qt::DisplayRole).toInt();
            }
            auto z = waypointModel->item(current_idx, 2);
        }

        auto imageItem = layerPanel->addItemToLayer(layerPanel->targetLayer, filePath);
        auto imageItemData = LayerPanel::getLayerData(imageItem);
        // extra: maker function to add image and path at once to both layers and scene
        imageItemData->setImage(flightImage);
        imageItemData->imagePath = imageFilePath;

        auto position = QPoint(xPos, yPos);
        workspace->systemViewer->addPhoto(imageItemData, position);

        Q_EMIT layerPanel->layerModel->layoutChanged();
        imageCount += 1;
    }
}

void FlightTools::imageViewMatching(const QString &imageFileName) {
    auto basePixmap = layerPanel->currentBaseMap->pixmap();
    if (basePixmap.isNull()) {
        QMessageBox::warning(dynamic_cast<QWidget *>(parent()),
                             "Warning", "Base map must be set.", QMessageBox::Ok);
        return;
    }
    auto [image, imageFilePath] = waypointer::io::readPhoto(imageFileName);
    if (image.isNull())
        return;

    auto progressDialog = QProgressDialog("Operation in progress.", nullptr, 0, 4,
                                          dynamic_cast<QWidget *>(parent()));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();
    progressDialog.setValue(0);

    progressDialog.setWindowTitle("Generating files...");
    progressDialog.setValue(1);
    QApplication::processEvents();

    auto matchingLayer = layerPanel->matchingLayer;
    auto obliqueImageItem = layerPanel->addItemToLayer(
            matchingLayer,
            QFileInfo(imageFilePath).fileName() + QLatin1String("(Oblique View)")
    );
    auto obliqueImageData = LayerPanel::getLayerData(obliqueImageItem);
    obliqueImageData->setImage(imageFilePath, false);
    workspace->systemViewer->scene()->addItem(obliqueImageData->graphicsItem);
    workspace->systemViewer->fitInView();
    workspace->systemViewer->scene()->update();
    progressDialog.setValue(2);
    QApplication::processEvents();

    progressDialog.setLabelText(QLatin1String("Performing perspective match..."));
    QApplication::processEvents();

    auto matchedImageItem = layerPanel->addItemToLayer(matchingLayer, "Matched Image");
    auto matchedImageData = LayerPanel::getLayerData(matchedImageItem);
    if (outputDir.isNull() or outputDir.isEmpty())
        outputDir = QDir(PathSettings::default_tigerOutputDir()).filePath("PerspectiveMatches");

    auto baseImageFile = layerPanel->currentBaseMap->imagePath;

    auto outImagePath = QString::fromStdString(pycall::perspectiveMatching(
            outputDir.toStdString(),
            baseImageFile.toStdString(),
            imageFilePath.toStdString(),
            false));

    if (outImagePath.isEmpty() or outImagePath.isNull()) return;
    matchedImageData->setImage(outImagePath, false);
    workspace->systemViewer->scene()->addItem(matchedImageData->graphicsItem);
    workspace->systemViewer->fitInView();

    progressDialog.setValue(3);
    QApplication::processEvents();

    progressDialog.setLabelText("Complete.");
    progressDialog.setValue(4);
    QApplication::processEvents();
}

void FlightTools::connectToCamera() {
    auto startDir = QDir::home();
    if (startDir.exists("Pictures"))
        startDir = QDir(startDir.filePath("Pictures"));
    auto imageDir = QFileDialog::getExistingDirectory(dynamic_cast<QWidget *>(parent()),
                                                      "Open Directory",
                                                      startDir.absolutePath(),
                                                      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (not imageDir.isEmpty() and not imageDir.isNull()) {
        imagesDirectory = imageDir;
        imagesDirLabel->setText(imageDir);
        watcher->addPath(imageDir);
        imageDirIsSet = true;
    }
}


void FlightTools::addSimulatedImage() {
    // extra
}
