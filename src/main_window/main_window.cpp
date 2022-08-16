//
// Created by Nic on 22/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "main_window.h"
#include "ui_mainwindow.h"
#include "../utility/pyscriptcaller.h"
#include "WatchdogIndicator.h"
#include <QDebug>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QTimer>
#include <QtGlobal>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTemporaryFile>
#include <QFormLayout>


MainWindow::MainWindow(QPlainTextEdit *consoleWidget, QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow),
        fileModel(new QFileSystemModel(this)), fileOrDirDialog(this),
        demGeneration(nullptr), parameterCalculator(nullptr),
        overviewDock(new QDockWidget(QStringLiteral("Profile Overview"), this)),
        aircraftDock(new QDockWidget(QStringLiteral("Aircraft"), this)),
        cameraDock(new QDockWidget(QStringLiteral("Camera"), this)),
        lensDock(new QDockWidget(QStringLiteral("Lens"), this)) {
    ui->setupUi(this);

    // console setup
    consoleDock = new QDockWidget(QStringLiteral("Event Log"));
    consoleDock->setWidget(consoleWidget);
    addDockWidget(Qt::BottomDockWidgetArea, consoleDock);
    auto consoleAction = consoleDock->toggleViewAction();
    consoleAction->setStatusTip(QLatin1String("Activate the event log window"));
    ui->menuView->addAction(consoleAction);
    consoleDock->setHidden(true);

    auto spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->toolBarTop->addWidget(spacer);
    setWindowTitle("RT3D: Real Time 3D");

    QMainWindow::setDockOptions(
            AnimatedDocks |
            AllowTabbedDocks |
            GroupedDragging |
            VerticalTabs |
            AllowNestedDocks
    );
    QMainWindow::setDockNestingEnabled(true);

    ui->projectDock->setTitleBarWidget(ui->frame);

    settingsMenu = new SettingsMenu();
    fileModel->setReadOnly(false);

    overviewProfile = new OverviewProfile(PathSettings::getCamerasPath());

    setupProfiles();

    connect(settingsMenu->pathSettings, &PathSettings::camerasPathWritten,
            overviewProfile, &OverviewProfile::setDB_path);


    ui->fileView->setModel(fileModel);

    setFileRoot();

    for (int i = 1; i < fileModel->columnCount(); ++i)
        ui->fileView->hideColumn(i);

    ui->fileView->header()->hide();

    ui->fileView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->fileView, &QTreeView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);

    connect(ui->actionSettings, &QAction::triggered,
            this, [this](bool checked) {
                if (settingsMenu->isVisible())
                    return;
                settingsMenu->show();
            });

    connect(ui->actionParameters_Calculator, &QAction::triggered,
            this, [this](bool checked) {
                if (not parameterCalculator)
                    parameterCalculator = new FlightParameters();
                if (parameterCalculator->isVisible())
                    return;
                parameterCalculator->show();
            });

    connect(ui->actionNewProject, &QAction::triggered,
            this, [this]() { createNewProject(QString()); });

    connect(ui->actionDEM_Generation, &QAction::triggered,
            this, &MainWindow::launchDemGeneration);

    connect(settingsMenu, &SettingsMenu::settingsApplied,
            this, &MainWindow::setFileRoot);

    connect(ui->actionDAT_to_Tiff, &QAction::triggered,
            this, [this]() {
                runDat2Tiff("");
            });

    connect(&fileOrDirDialog, &QFileDialog::currentChanged,
            this, [&](const QString &str) {
                QFileInfo info(str);
                if (info.isFile())
                    fileOrDirDialog.setFileMode(QFileDialog::ExistingFile);
                else if (info.isDir())
                    fileOrDirDialog.setFileMode(QFileDialog::Directory);
            });

    connect(ui->actionNavigation, &QAction::triggered,
            this, &MainWindow::launchNavigation);

    connect(ui->actionNewAircraft_Profile, &QAction::triggered,
            this, [this]() { overviewProfile->aircraftProfile->saveProfile(); });

    connect(ui->actionOpen_Project, &QAction::triggered,
            this, [this]() { openProject(); });

    connect(ui->actionSaveProject, &QAction::triggered,
            this, [this]() {
                if (projectInput.isEmpty() or projectInput.isNull()) {
                    Messages::info_msg(this, QLatin1String("No project currently loaded."));
                    return;
                }
                overviewProfile->saveProjectFile(projectInput, projectOutput);
                statusBar()->showMessage(QLatin1String("Project Saved!"), 3000);
            });

    connect(ui->actionCorrect_Distortion, &QAction::triggered,
            this, [this]() { runGeometricCorrection(); });

    connect(ui->actionChange_Settings, &QAction::triggered,
            this, [this]() {
                if (settingsMenu->isVisible())
                    return;
                settingsMenu->show();
                settingsMenu->setSettingsField(3);
            });

    connect(ui->actionEditProject_Config, &QAction::triggered,
            this, [this]() {
                if (projectInput.isEmpty() or projectInput.isNull()) {
                    Messages::info_msg(this, QLatin1String("No project currently loaded."));
                    return;
                }
                auto projectFile = QDir(projectInput).filePath("rt3dProject.json");
                if (QFileInfo::exists(projectFile))
                    QDesktopServices::openUrl(QUrl::fromLocalFile(projectFile));
                else
                    Messages::warning_msg(this, QLatin1String("Project file could not be found"));
            });

    connect(ui->actionEditCamera_DB, &QAction::triggered,
            this, [this]() {
                auto camerasPath = PathSettings::getCamerasPath();
                if (QFileInfo::exists(camerasPath))
                    showInFolder(camerasPath);
                else
                    Messages::warning_msg(this,
                                          QLatin1String("Could not be opened, check path in application settings."));
            });

    connect(ui->actionEditLens_DB, &QAction::triggered,
            this, [this]() {
                auto camerasPath = PathSettings::getCamerasPath();
                if (QFileInfo::exists(camerasPath))
                    showInFolder(camerasPath);
                else
                    Messages::warning_msg(this,
                                          QLatin1String("Could not be opened, check path in application settings."));
            });

    connect(ui->actionNewCamera_Profile, &QAction::triggered,
            this, [this]() { createNewCameraProfile(); });

    connect(ui->actionNewLens_Profile, &QAction::triggered,
            this, [this]() { createNewLensProfile(); });

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::showContextMenu(const QPoint &pos) {
    QModelIndex index = ui->fileView->indexAt(pos);
    auto file_path = fileModel->filePath(index);

    auto currentIndex = ui->fileView->currentIndex();
    auto current_path = fileModel->filePath(currentIndex);

    auto contextMenu = new QMenu(ui->fileView);

    if (index.isValid()) {
        if (file_path.contains(PathSettings::default_tigerInputDir())) {
            if (file_path != PathSettings::default_tigerInputDir()) {
                if (QFileInfo(file_path).isDir()) {
                    auto projectFile = QDir(file_path).filePath("rt3dProject.json");
                    if (QFileInfo::exists(projectFile)) {
                        contextMenu->addAction(
                                QLatin1String("Open Project"),
                                [=, this]() { openProject(projectFile); }
                        );
                    } else
                        contextMenu->addAction(
                                QLatin1String("Create Project"),
                                [=, this]() { createNewProject(QFileInfo(file_path).baseName()); }
                        );
                }
                contextMenu->addSeparator();
            }
        }
    }

    // extra: right click on directory and check if .dat files are included,
    // if yes, add option to apply correction

    // ### always available ###
    contextMenu->addAction(
            QLatin1String("New Folder"),
            [=, this]() {
                if (index.isValid()) {
                    fileModel->mkdir(
                            QFileInfo(file_path).isDir() ? index : index.parent(), QStringLiteral("New Folder"));
                } else if (currentIndex.isValid()) {
                    fileModel->mkdir(currentIndex, QStringLiteral("New Folder"));
                } else {
                    fileModel->mkdir(ui->fileView->rootIndex(), QStringLiteral("New Folder"));
                }
            }
    );
    contextMenu->addAction(
            QStringLiteral("Open in new window"),
            [=, this]() {
                if (index.isValid()) {
                    showInFolder(file_path);
                } else if (currentIndex.isValid()) {
                    showInFolder(current_path);
                } else {
                    showInFolder(fileModel->rootPath());
                }

            }
    );

    // ### only available if on specific item ###
    if (index.isValid()) {
        if (not PathSettings::defaultPaths().contains(file_path))
            contextMenu->addAction(
                    QStringLiteral("Delete"),
                    [=]() {
                        QFile(file_path).moveToTrash();
                    }
            );

        // connections into DEM generation
        contextMenu->addSeparator();
        contextMenu->addAction(
                QStringLiteral("Add as DEM generation input"),
                [=, this]() {
                    setDemGenIO(file_path, "");
                }
        );
        contextMenu->addAction(
                QStringLiteral("Add as DEM generation output"),
                [=, this]() {
                    setDemGenIO("", file_path);
                }
        );
        contextMenu->addSeparator();
        // editing and tool connections
        auto suffix = QFileInfo(file_path).suffix().toLower();
        if (suffix == "dat") {
            contextMenu->addAction(
                    QStringLiteral("Create tiff"),
                    [=, this]() {
                        runDat2Tiff(file_path);
                    }
            );
        }
        if (suffix == "dat" or
            suffix == "tiff" or
            suffix == "tif" or
            suffix == "jpg" or
            suffix == "jpeg" or
            suffix == "png") {
            contextMenu->addAction(QStringLiteral("Apply Correction"),
                                   [=, this]() {
                                       runGeometricCorrection(file_path);
                                   });
        }

    }

    contextMenu->exec(ui->fileView->viewport()->mapToGlobal(pos));
}

void MainWindow::setFileRoot() {
    QSettings settings(QSettings::UserScope);
    auto tiger_dir = PathSettings::getTigerDir();
    if (QFileInfo::exists(tiger_dir)) {
        fileModel->setRootPath(tiger_dir);
        ui->fileView->setRootIndex(fileModel->index(tiger_dir));
    }
}

void MainWindow::launchDemGeneration() {
    if (demGeneration) {
        demGeneration->parentWidget()->raise();
        return;
    }
    settingsMenu->pathSettings->configureTigerDir();

    auto dock = new QDockWidget(QStringLiteral("DEM Generation"), this);
    dock->setAttribute(Qt::WA_DeleteOnClose);

    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    demGeneration = new DemGeneration(nullptr, fileModel);
    demGeneration->setImageHeight(overviewProfile->cameraProfile->getImageHeight());
    demGeneration->setImageWidth(overviewProfile->cameraProfile->getImageWidth());
    dock->setWidget(demGeneration);
    auto setupDirWatchAction = ui->toolBarTop->addWidget(
            demGeneration->getWatchDirWidget()
    );
    auto chooseCameraAction = ui->toolBarTop->addWidget(
            demGeneration->getChooseCameraWidget()
    );

    tabifyDockWidget(ui->projectDock, dock);


    // ### outward signals ###
    connect(overviewProfile->cameraProfile, &CameraProfile::imageHeightChanged,
            demGeneration, &DemGeneration::setImageHeight);

    connect(overviewProfile->cameraProfile, &CameraProfile::imageWidthChanged,
            demGeneration, &DemGeneration::setImageWidth);

    connect(overviewProfile->aircraftProfile, &AircraftProfile::flightHeightChanged,
            demGeneration, &DemGeneration::setFlightAltitude);

    // ### inward signals ###
    connect(demGeneration, &DemGeneration::imageWidthChanged,
            overviewProfile->cameraProfile, &CameraProfile::setImageWidth);

    connect(demGeneration, &DemGeneration::imageHeightChanged,
            overviewProfile->cameraProfile, &CameraProfile::setImageHeight);

    connect(demGeneration, &DemGeneration::flightAltitudeChanged,
            overviewProfile->aircraftProfile, &AircraftProfile::setHeight);

    connect(demGeneration, &DemGeneration::cameraFound,
            overviewProfile->cameraProfile, &CameraProfile::searchAndSetCamera);

    connect(demGeneration, &DemGeneration::destroyed,
            this, [=, this]() {
                ui->toolBarTop->removeAction(chooseCameraAction);
                chooseCameraAction->deleteLater();
                ui->toolBarTop->removeAction(setupDirWatchAction);
                setupDirWatchAction->deleteLater();
            });

    connect(demGeneration, &DemGeneration::inputPathEdited,
            this, [this](const QString &path) { projectInput = path; });
    connect(demGeneration, &DemGeneration::outputPathEdited,
            this, [this](const QString &path) { projectOutput = path; });

    // ### initial values ###
    demGeneration->setFlightAltitude(overviewProfile->aircraftProfile->getHeight());
    demGeneration->setImageHeight(overviewProfile->cameraProfile->getImageHeight());
    demGeneration->setImageWidth(overviewProfile->cameraProfile->getImageWidth());

    if (not projectInput.isEmpty())
        demGeneration->setInputPath(projectInput);
    if (not projectOutput.isEmpty())
        demGeneration->setOutputPath(projectOutput);

    QTimer::singleShot(0, this, [=]() { dock->raise(); });
}

void MainWindow::setupProfiles() {
    overviewDock->setObjectName("OverviewDock");
    overviewDock->setWidget(overviewProfile);
    overviewDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    aircraftDock->setObjectName("AircraftDock");
    aircraftDock->setWidget(overviewProfile->aircraftProfile);
    aircraftDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    cameraDock->setObjectName("CameraDock");
    cameraDock->setWidget(overviewProfile->cameraProfile);
    cameraDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    lensDock->setObjectName("LensDock");
    lensDock->setWidget(overviewProfile->lensProfile);
    lensDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    addDockWidget(Qt::RightDockWidgetArea, overviewDock);
    splitDockWidget(overviewDock, aircraftDock, Qt::Vertical);
    tabifyDockWidget(aircraftDock, cameraDock);
    tabifyDockWidget(cameraDock, lensDock);
    aircraftDock->raise();

}

void MainWindow::setDemGenIO(const QString &input, const QString &output) {
    if (not demGeneration and (not input.isEmpty() or not output.isEmpty()))
        launchDemGeneration();

    if (not input.isEmpty()) {
        projectInput = input;
        demGeneration->setInputPath(input);
    }

    if (not output.isEmpty()) {
        projectOutput = output;
        demGeneration->setOutputPath(output);
    }

}

void MainWindow::showInFolder(const QString &path) {
    QFileInfo info(path);
#if defined(Q_OS_WIN)
    QStringList args;
    if (!info.isDir())
        args << "/select,";
    args << QDir::toNativeSeparators(path);
    if (QProcess::startDetached(QStringLiteral("explorer"), args))
        return;
#endif
    QDesktopServices::openUrl(QUrl::fromLocalFile(info.isDir() ? path : info.path()));
}


void MainWindow::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    QMetaObject::invokeMethod(settingsMenu->pathSettings,
                              &PathSettings::configureTigerDir,
                              Qt::ConnectionType::QueuedConnection);

}

void MainWindow::createNewProject(const QString &newName) {
    auto projectName = newName;

    settingsMenu->pathSettings->configureTigerDir();
    // query project name
    if (projectName.isEmpty() or projectName.isNull()) {
        bool ok;
        projectName = QInputDialog::getText(this,
                                            QStringLiteral("New Project"),
                                            QStringLiteral("Project Name:"),
                                            QLineEdit::Normal,
                                            QLatin1String(),
                                            &ok,
                                            windowFlags() &
                                            ~Qt::WindowContextHelpButtonHint &
                                            ~Qt::WindowMinMaxButtonsHint,
                                            Qt::ImhNone);
        if (projectName.isEmpty() or not ok) return;
    } else
        projectName = newName;
    qInfo() << "New project created:" << projectName;

    auto tigerInput = PathSettings::default_tigerInputDir();
    auto tigerOutput = PathSettings::default_tigerOutputDir();

    QDir(tigerInput).mkdir(projectName);
    QDir(tigerOutput).mkdir(projectName);

    projectInput = QDir::toNativeSeparators(tigerInput + "/" + projectName);
    projectOutput = QDir::toNativeSeparators(tigerOutput + "/" + projectName);

    overviewProfile->saveProjectFile(projectInput, projectOutput);
    openProject(QDir(projectInput).filePath("rt3dProject.json"));
}

void MainWindow::openProject(const QString &filePath) {
    auto projectInfo = overviewProfile->loadProjectFile(filePath);
    if (projectInfo.isEmpty()) {
        return;
    }
    projectInput = QDir::toNativeSeparators(projectInfo["input path"].toString());
    projectOutput = QDir::toNativeSeparators(projectInfo["output path"].toString());
    if (not QFileInfo::exists(projectOutput))
        QDir().mkpath(projectOutput);

    setDemGenIO(QFileInfo(projectInput).path(), projectOutput);

    auto tigerData = PathSettings::default_dataDir();
    auto tigerInput = PathSettings::default_tigerInputDir();
    auto tigerOutput = PathSettings::default_tigerOutputDir();
    ui->fileView->expand(fileModel->index(tigerData));
    ui->fileView->expand(fileModel->index(tigerInput));
    ui->fileView->expand(fileModel->index(tigerOutput));
    ui->fileView->expand(fileModel->index(projectInput));
    ui->fileView->expand(fileModel->index(projectOutput));

}

void MainWindow::runDat2Tiff(const QString &input) {
    QFileInfo inputInfo;
    if (input.isEmpty() or input.isNull()) {
        fileOrDirDialog.setOption(QFileDialog::DontUseNativeDialog, true);
        fileOrDirDialog.setWindowTitle("Select file or folder");
        fileOrDirDialog.setDirectory(fileModel->filePath(ui->fileView->currentIndex()));
        if (fileOrDirDialog.exec()) {
            auto names = fileOrDirDialog.selectedFiles();
            inputInfo.setFile(names.value(0));
        }
    } else
        inputInfo.setFile(input);
    // extra: ask file replacement with context menu
    if (inputInfo.suffix().toLower() == "dat") { // check correct file extension
        pycall::dat2tiff(inputInfo.filePath().toUtf8().toStdString());
    } else if (inputInfo.isDir()) {
        pycall::dat2tiff_dir(inputInfo.filePath().toUtf8().toStdString());
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    auto dockWidgets = findChildren<QDockWidget *>();
    for (auto dock: dockWidgets) {
        dock->close();
        dock->deleteLater();
    }
    if (not demGeneration.isNull()) {
        demGeneration->close();
        demGeneration->deleteLater();
    }
    if (not parameterCalculator.isNull()) {
        parameterCalculator->close();
        parameterCalculator->deleteLater();
    }
    QWidget::closeEvent(event);
}

void MainWindow::launchNavigation() {
//    qDebug() << "launching navigation.";
    if (not navigationApp.isNull()) return;
    navigationApp = new MainInterface();
    navigationApp->setAttribute(Qt::WA_DeleteOnClose);
    navigationApp->show();

    // connect uav height to z value of navigation
    navigationApp->setZ(overviewProfile->aircraftProfile->getHeight());
    connect(overviewProfile->aircraftProfile, &AircraftProfile::flightHeightChanged,
            navigationApp, &MainInterface::setZ);
}

[[maybe_unused]] void MainWindow::runNewCameraCalibration() {
    // extra: integrate camera calibration
    qDebug() << "NOT IMPLEMENTED YET <running camera calibration>.";
}


// apply geometric correction
void MainWindow::runGeometricCorrection(const QString &input) {
    // aircraft must have a height
    auto distanceToSubject = overviewProfile->aircraftProfile->getHeight();
    bool ok = false;
    if (distanceToSubject == 0.0) {
        distanceToSubject = QInputDialog::getDouble(this, QLatin1String("Enter Aircraft height"),
                                                    QLatin1String("Above Ground Height in metres:"),
                                                    0.01, 0.001, 99999, 2, &ok,
                                                    windowFlags() &
                                                    ~Qt::WindowContextHelpButtonHint &
                                                    ~Qt::WindowMinMaxButtonsHint);
        if (not ok) {
            qWarning() << "Aircraft height could not be determined";
            return;
        }
        overviewProfile->aircraftProfile->setHeight(distanceToSubject);
    }
    if (not overviewProfile->isCameraSet()) {
        Messages::warning_msg(this, QLatin1String("A camera profile must be set to apply correction."));
        cameraDock->raise();
        return;
    }
    auto[lensModel, lensMaker] = overviewProfile->lensProfile->currentTextToLens();
    if (not overviewProfile->isLensSet()) {
        Messages::warning_msg(this, QLatin1String("A lens profile must be set to apply correction."));
        lensDock->raise();
        return;
    }
    auto focalLength = overviewProfile->lensProfile->getFocalLen();
    if (focalLength == 0.0) {
        focalLength = QInputDialog::getDouble(this, QLatin1String("Enter Focal Length"),
                                              QLatin1String("Focal length of lens in mm:"),
                                              0.01, 0.001, 99999, 2, &ok,
                                              windowFlags() &
                                              ~Qt::WindowContextHelpButtonHint &
                                              ~Qt::WindowMinMaxButtonsHint);
        if (not ok) {
            qWarning() << "Focal length could not be determined";
            return;
        }
        overviewProfile->lensProfile->setFocalLen(focalLength);
    }
    auto[cameraModel, cameraMaker] = overviewProfile->cameraProfile->currentTextToCamera();

    auto aperture = overviewProfile->lensProfile->getAperture();
    if (aperture == 0.0) {
        aperture = QInputDialog::getDouble(this, QLatin1String("Enter Aperture Size"),
                                           QLatin1String("Size of Aperture as f/:"),
                                           0.01, 0.001, 99999, 2, &ok,
                                           windowFlags() &
                                           ~Qt::WindowContextHelpButtonHint &
                                           ~Qt::WindowMinMaxButtonsHint);
        if (not ok) {
            qWarning() << "Focal length could not be determined";
            return;
        }
        overviewProfile->lensProfile->setAperture(aperture);
    }

    QFileInfo inputInfo;
    if (input.isEmpty() or input.isNull()) {
        fileOrDirDialog.setOption(QFileDialog::DontUseNativeDialog, true);
        fileOrDirDialog.setWindowTitle("Select file or folder");
        fileOrDirDialog.setDirectory(fileModel->filePath(ui->fileView->currentIndex()));
        connect(&fileOrDirDialog, &QFileDialog::currentChanged,
                this, [&](const QString &str) {
                    QFileInfo info(str);
                    if (info.isFile())
                        fileOrDirDialog.setMimeTypeFilters({"image/jpeg",
                                                            "image/png",
                                                            "image/bmp",
                                                            "image/heif",
                                                            "image/tiff",
                                                           });
                    else
                        fileOrDirDialog.setMimeTypeFilters({"application/octet-stream"});
                });
        if (fileOrDirDialog.exec()) {
            auto names = fileOrDirDialog.selectedFiles();
            inputInfo.setFile(names.value(0));
        } else {
//            qDebug() << "File not available, Correction could not be completed.";
            return;
        }
    } else {
        inputInfo.setFile(input);
    }

    QStringList files;
    if (inputInfo.isDir()) {
        auto imageTypes = DemGeneration::imageMimes();
        imageTypes.append(".dat");
        files = QDir(inputInfo.filePath()).entryList(imageTypes);
    } else
        files.append(inputInfo.filePath());

    for (auto f: files) {
        if (QFileInfo(f).suffix() == "dat") {
            runDat2Tiff(f);
            f = QFileInfo(f).path() + "/" + QFileInfo(f).completeBaseName() + ".tiff";
        }
        pycall::correctDistortion(f.toStdString(),
                                  cameraMaker.toStdString(),
                                  cameraModel.toStdString(),
                                  lensMaker.toStdString(),
                                  lensModel.toStdString(),
                                  focalLength,
                                  aperture,
                                  distanceToSubject,
                                  GeometricSettings::getInterMethod());
    }

}

void MainWindow::createNewCameraProfile() {
    auto dialog = QDialog(this, windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog.setWindowTitle("New Camera Profile");
    auto hLayout = new QVBoxLayout;
    auto formLayout = new QFormLayout;
    auto cameraMakerField = new QLineEdit("User");
    formLayout->addRow("Camera Maker:", cameraMakerField);
    auto cameraModelField = new QLineEdit("User Camera");
    formLayout->addRow("Camera Model:", cameraModelField);
    auto cameraMountField = new QLineEdit("userCameraMount");
    formLayout->addRow("Camera Mount:", cameraMountField);
    auto cropFactorField = new QDoubleSpinBox();
    cropFactorField->setValue(overviewProfile->cameraProfile->getCropFactor());
    formLayout->addRow("Crop Factor:", cropFactorField);
    hLayout->addLayout(formLayout);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                          | QDialogButtonBox::Cancel,
                                          Qt::Horizontal);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    hLayout->addWidget(buttonBox);
    dialog.setLayout(hLayout);
    if (not dialog.exec()) return;

    auto header = QString("<!DOCTYPE lensdatabase SYSTEM \"lensfun-database.dtd\">\n");
    auto header1 = QString("<lensdatabase version=\"2\">\n\n");
    auto footer = QString("</lensdatabase>");


    auto camerasPath = PathSettings::getCamerasPath();
    auto userCamerasPath = QDir(camerasPath).filePath("user-cameras.xml");
    if (not QFileInfo::exists(userCamerasPath)) {
        auto userFile = QFile(userCamerasPath);
        if (!userFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            Messages::warning_msg(this, "User camera profile file could not be created.");
            return;
        }
        QTextStream out(&userFile);
        out << header;
        out << header1;
        out << footer;
        userFile.close();
    }

    auto userFile = QFile(userCamerasPath);
    if (!userFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Messages::warning_msg(this, "User camera profile file could not be read.");
        return;
    }
    QString outString;
    QTextStream out(&outString);
    QTextStream in(&userFile);
    while (!in.atEnd()) {
        auto line = in.readLine();
        if (line != footer)
            out << line << "\n";
    }
    userFile.close();

    if (!userFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Messages::warning_msg(this, "User camera profile file could not be written.");
        return;
    }
    out.setDevice(&userFile);
    out << outString;

    auto cropFactor = QString("%1").arg(cropFactorField->value(), 0, 'f', 2);

    out << "<camera>" << "\n";
    out << "    " << "<maker>" << cameraMakerField->text() << "</maker>" << "\n";
    out << "    " << R"(<!-- <maker lang="en">)" << cameraMakerField->text() << "</maker> -->" << "\n";
    out << "    " << "<model>" << cameraModelField->text() << "</model>" << "\n";
    out << "    " << R"(<!-- <model lang="en">)" << cameraModelField->text() << "</model> -->" << "\n";
    out << "    " << "<mount>" << cameraMountField->text() << "</mount>" << "\n";
    out << "    " << "<cropfactor>" << cropFactor << "</cropfactor>" << "\n";
    out << "</camera>" << "\n\n";

    out << footer;
    QDesktopServices::openUrl(QUrl::fromLocalFile(userCamerasPath));
    overviewProfile->cameraProfile->populateProfileList();
}

void MainWindow::createNewLensProfile() {

    auto dialog = QDialog(this, windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog.setWindowTitle("New Lens Profile");
    auto hLayout = new QVBoxLayout;
    auto formLayout = new QFormLayout;

    auto makerField = new QLineEdit("User");
    formLayout->addRow("Lens Maker:", makerField);

    auto modelField = new QLineEdit("User Lens");
    formLayout->addRow("Lens Model:", modelField);

    auto mountField = new QLineEdit("userCameraMount");
    formLayout->addRow("Camera Mount:", mountField);

    auto cropFactorField = new QDoubleSpinBox();
    cropFactorField->setValue(overviewProfile->cameraProfile->getCropFactor());
    formLayout->addRow("Crop Factor:", cropFactorField);

    auto focalLenField = new QDoubleSpinBox();
    focalLenField->setValue(overviewProfile->lensProfile->getFocalLen());
    formLayout->addRow("Focal Length:", focalLenField);

    auto apertureField = new QDoubleSpinBox();
    apertureField->setValue(overviewProfile->lensProfile->getAperture());
    formLayout->addRow("Aperture:", apertureField);

    hLayout->addLayout(formLayout);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                          Qt::Horizontal);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    hLayout->addWidget(buttonBox);
    dialog.setLayout(hLayout);
    if (not dialog.exec()) return;

    auto header = QString("<!DOCTYPE lensdatabase SYSTEM \"lensfun-database.dtd\">\n");
    auto header1 = QString("<lensdatabase version=\"2\">\n\n");
    auto footer = QString("</lensdatabase>");

    auto camerasPath = PathSettings::getCamerasPath();
    auto userLensPath = QDir(camerasPath).filePath("user-lenses.xml");
    if (not QFileInfo::exists(userLensPath)) {
        auto userFile = QFile(userLensPath);
        if (!userFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            Messages::warning_msg(this, "User lens profile file could not be created.");
            return;
        }
        QTextStream out(&userFile);
        out << header;
        out << header1;
        out << footer;
        userFile.close();
    }

    auto userFile = QFile(userLensPath);
    if (!userFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Messages::warning_msg(this, "User lens profile file could not be read.");
        return;
    }
    QString outString;
    QTextStream out(&outString);
    QTextStream in(&userFile);
    while (!in.atEnd()) {
        auto line = in.readLine();
        if (line != footer)
            out << line << "\n";
    }
    userFile.close();

    if (!userFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Messages::warning_msg(this, "User camera profile file could not be written.");
        return;
    }
    out.setDevice(&userFile);
    out << outString;

    auto lensName = QString("%1 %2mm f/%3")
            .arg(modelField->text())
            .arg(focalLenField->value(), 0, 'f', 1)
            .arg(apertureField->value(), 0, 'f', 1);
    auto cropFactor = QString("%1").arg(cropFactorField->value(), 0, 'f', 2);

    out << "<lens>" << "\n";
    out << "    " << "<maker>" << makerField->text() << "</maker>" << "\n";
    out << "    " << R"(<!-- <maker lang="en">)" << makerField->text() << "</maker> -->" << "\n";
    out << "    " << "<model>" << lensName << "</model>" << "\n";
    out << "    " << R"(<!-- <model lang="en">)" << lensName << "</model> -->" << "\n";
    out << "    " << "<mount>" << mountField->text() << "</mount>" << "\n";
    out << "    " << "<!-- <mount>" << mountField->text() << "2" << "</mount> -->" << "\n";
    out << "    " << "<cropfactor>" << cropFactor << "</cropfactor>" << "\n";
    out << "    " << "<calibration>" << "\n";
    out << "    " << "    " << "<!-- Taken with User Camera -->" << "\n";
    out << "    " << "    " << "<!-- WARNING: Calibration must be added! -->" << "\n";
    out << "    " << "</calibration>" << "\n";
    out << "</lens>" << "\n\n";

    out << footer;
    QDesktopServices::openUrl(QUrl::fromLocalFile(userLensPath));
    overviewProfile->lensProfile->populateProfileList();

}
