//
// Created by Nic on 22/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "maininterface.h"
#include "ui_maininterface.h"
#include "waypointer_io/images.hpp"
#include "waypointer_io/waypoints.hpp"
#include "../utility/messages.hpp"
#include "../settings/path_settings/pathsettings.h"
#include <QDebug>
#include <QScreen>
#include <QMessageBox>

MainInterface::MainInterface(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainInterface),
        waypointFile(""),
        baseItemFile("") {
    ui->setupUi(this);
    qInfo() << "Waypointer created";

    resize(QApplication::primaryScreen()->size() / 1.3);
    setWindowTitle("Waypointer");
    setObjectName("MainInterface");

    // extra: saving features
//    ui->menuSave->menuAction()->setVisible(false);

    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::TabPosition::North);
    splitDockWidget(ui->layerWidget, ui->waypointWidget, Qt::Vertical);

    connect(ui->actionBase_Map, &QAction::triggered,
            this, [this]() { openBasemapPhoto(); });
    connect(ui->actionWaypoints, &QAction::triggered,
            this, [this]() { openWaypoints(); });
    connect(ui->actionSaveWaypoints, &QAction::triggered,
            this, [this](){ saveWaypoints();});
    connect(ui->actionSaveTargets, &QAction::triggered,
            this, [this](){saveTargetPoints();});
    connect(ui->actionAdd_Flight_Image, &QAction::triggered,
            this, [this]() { addFlightImage(); });

    layerPanel = new LayerPanel(this,
                                ui->layerView,
                                ui->opacityBox,
                                ui->layerSettings);

    coordinatePanel = new CoordinatePanel(this,
                                          ui->waypointView,
                                          ui->targetpointView,
                                          ui->addCoordsBtn,
                                          ui->coordLineEdit,
                                          ui->editorTabs);
    ui->addCoordsBtn->setDisabled(true);
    ui->addCoordsBtn->setToolTip(QLatin1String("Load basemap to add waypoints."));

    workspace = new Workspace(this,
                              ui->coordLineEdit,
                              layerPanel,
                              coordinatePanel);
    workspace->toolMode->multiTool->setToolAction(ui->actionNoTool);
    workspace->toolMode->selectionMode->setToolAction(ui->actionToolSelect);
    workspace->toolMode->placeMode->setToolAction(ui->actionToolPlacer);
    workspace->toolMode->connectMode->setToolAction(ui->actionToolConnector);
    workspace->toolMode->panTool->setToolAction(ui->actionPanMovement);
    workspace->toolMode->setMode(ToolType::MultiTool);
    workspace->toolMode->multiTool->activate();

    flightTools = new FlightTools(this,
                                  ui->actionPerspectiveViewMatching,
                                  ui->actionSimulate_Image,
                                  ui->actionSimulate_Flight_Images,
                                  ui->actionConnect_to_Camera,
                                  ui->statusbar,
                                  ui->editorTabs,
                                  workspace,
                                  layerPanel
    );

    connect(coordinatePanel, &CoordinatePanel::requestWaypointAdded,
            workspace, &Workspace::receiveWaypointRequest);

    setCentralWidget(workspace->systemViewer);

    connect(ui->actionWaypoint_Editor, &QAction::triggered,
            ui->waypointWidget, &QDockWidget::setVisible);
    connect(ui->actionLayer_View, &QAction::triggered,
            ui->layerView, &QDockWidget::setVisible);
    ui->menuWindow->addAction(ui->toolBar->toggleViewAction());

    connect(ui->opacityButton, &QPushButton::released,
            this, [this]() {
                auto currentData = layerPanel->currentSelected();
                currentData->setChecked(not currentData->isChecked());
            });
}

void MainInterface::keyPressEvent(QKeyEvent *event) {
    if (underMouse())
        workspace->systemViewer->keyPressEvent(event);
}

void MainInterface::keyReleaseEvent(QKeyEvent *event) {
    if (underMouse())
        workspace->systemViewer->keyReleaseEvent(event);
}

void MainInterface::openBasemapPhoto(const QString &path) {
    auto[image, filePath] = waypointer::io::readPhoto(path);
    if (image.isNull()) return;
    ui->addCoordsBtn->setDisabled(false);
    ui->addCoordsBtn->setToolTip(QLatin1String("Add waypoints to map."));

    baseItemFile = filePath;
    auto newBasemap = layerPanel->addItemToLayer(layerPanel->baseLayer, QFileInfo(filePath).fileName());
    layerPanel->currentBaseMap = LayerPanel::getLayerData(newBasemap);
    workspace->systemViewer->setBasemap(image, filePath);
    Q_EMIT layerPanel->layerModel->layoutChanged();
}

void MainInterface::openWaypoints(const QString &filePath) const {
    if (not flightTools->baseMapItemOk()) {
        QMessageBox::warning(dynamic_cast<QWidget *>(parent()),
                             "Warning", "Base map must be set.", QMessageBox::Ok);
        return;
    }
    auto data = waypointer::io::readWaypoints(filePath);
    for (auto p: data) {
        coordinatePanel->addWaypoint(p, true);
    }
}

void MainInterface::addFlightImage(QString flightImagePath, int x, int y) {
    if (coordinatePanel->waypointsTable->rowCount() < 1) {
        Messages::warning_msg(this, "Waypoints must be added before flight images.");
        return;
    }
    if (flightImagePath.isNull() or flightImagePath.isEmpty()) {
        auto dialog = new FlightImageDialog(this);
        if (dialog->exec()) {
            flightImagePath = dialog->flightFileLine->text();
            flightTools->addFlightImage(flightImagePath, x, y);
        } else {
            return;
        }
    }
}

void MainInterface::setZ(double z) const {
    workspace->setCoordView(QVector3D{0, 0, static_cast<float>(z)});
}


void MainInterface::saveWaypoints() {
    // extra: move to waypointer_io
    auto saveFilePath = QFileDialog::getSaveFileName(this,
                                                     "Save Waypoints",
                                                     QDir(PathSettings::default_tigerDir()).filePath("waypoints.txt"),
                                                     "Waypoint Files (*.txt *.csv *.tsv)"
    );

    auto waypointsTable = coordinatePanel->waypointsTable;
    QFile saveFile(saveFilePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't open file:" << saveFile << "for writing.";
        return;
    }
    QTextStream streamOut(&saveFile);
    for (int i = 0; i < waypointsTable->rowCount(); ++i) {
        auto saveCoord = QString("%1, %2, %3").arg(waypointsTable->item(i, 0)->text(),
                                                   waypointsTable->item(i, 1)->text(),
                                                   waypointsTable->item(i, 2)->text());
        if (QFileInfo(saveFilePath).completeSuffix().contains("tsv"))
            saveCoord.replace(",", "\t");
        streamOut << saveCoord << "\n";
    }
    qInfo() << "Waypoints saved:" << saveFile.fileName();
}

void MainInterface::saveTargetPoints(){
    // extra: move to waypointer_io
    auto saveFilePath = QFileDialog::getSaveFileName(this,
                                                     "Save Target Points",
                                                     QDir(PathSettings::default_tigerDir()).filePath("target_points.txt"),
                                                     "Waypoint Files (*.txt *.csv *.tsv)"
    );

    auto waypointsTable = coordinatePanel->waypointsTable;
    QFile saveFile(saveFilePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't open file:" << saveFile << "for writing.";
        return;
    }
    QTextStream streamOut(&saveFile);
    for (int i = 0; i < waypointsTable->rowCount(); ++i) {
        auto saveCoord = QString("%1, %2, %3").arg(waypointsTable->item(i, 0)->text(),
                                                   waypointsTable->item(i, 1)->text(),
                                                   waypointsTable->item(i, 2)->text());
        if (QFileInfo(saveFilePath).completeSuffix().contains("tsv"))
            saveCoord.replace(",", "\t");
        streamOut << saveCoord << "\n";
    }
    qInfo() << "Target point saved:" << saveFile.fileName();
}
//
//void MainInterface::saveMissionImageAs(){
//    // extra: save > mission image
//}

