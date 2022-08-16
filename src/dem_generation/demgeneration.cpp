//
// Created by Nic Arnold 2021-2022.
//

#include "demgeneration.h"
#include "ui_DemGeneration.h"
#include "../utility/GroupFileObjectDialog.hpp"
#include "../settings/path_settings/pathsettings.h"
#include "../settings/dem_behaviour/dembehaviour.h"
#include "../utility/pyscriptcaller.h"
#include <QDir>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileDialog>
#include <QCompleter>
#include <iostream>
#include <filesystem>
#include <QMimeDatabase>
#include <QInputDialog>

/**
 * Camera access protocol
 * - click `DEM Generation`
 *  -- create `connect to camera` button
 * - click `connect to camera`
 *  -- call `selectCamera()`
 *      --- create cameraWatchdog
 *      --- choose camera from options
 *      --- if chosen:
 *          ---- set formatMode to Camera
 *          ---- set indicator to standby
 *          ---- lookup camera name in lensfunDB
 *      --- else:
 *          ---- set formatMode Files
 *          ---- set indicator to disabled
 * **/

DemGeneration::DemGeneration(QWidget *parent, QFileSystemModel *parent_fsModel) :
        QWidget(parent), ui(new Ui::DemGeneration),
        formatMode(FormatMode::FILES),
        imageWidth(0), imageHeight(0), imagePairsCount(0),
        scriptLauncher(new ScriptLauncher(this)),
        imageCutter(new ImageCutter(this)),
        completer(new QCompleter(this)) {
    ui->setupUi(this);
    ui->stopBtn->setDisabled(true);

    ui->frameRate->setDisabled(true);

    // extra: re-enable completer
//    completer->setCompletionMode(QCompleter::PopupCompletion);
//    completer->setCaseSensitivity(Qt::CaseInsensitive);
//    completer->setModel(parent_fsModel);
//
//    ui->inputPathLineEdit->setCompleter(completer);
//    ui->outPrefixLineEdit->setCompleter(completer);

    connect(scriptLauncher, &ScriptLauncher::allFinished,
            this, &DemGeneration::checkResults);

    connect(ui->selectInputBtn, &QPushButton::released, this, &DemGeneration::selectInput);
    connect(ui->selectOutputBtn, &QPushButton::released, this, &DemGeneration::selectOutput);
    connect(ui->runBtn, &QPushButton::released, this, &DemGeneration::runClicked);
    connect(ui->cancelBtn, &QPushButton::released, this, &DemGeneration::cancelClicked);
    connect(ui->stopBtn, &QPushButton::released, this, &DemGeneration::stopClicked);
    connect(ui->saveSettingsBtn, &QPushButton::released, this, &DemGeneration::saveParameters);
    connect(ui->loadSettingsBtn, &QPushButton::released, this, [this]() { loadParameters(); });
    connect(ui->inputPathLineEdit, &QLineEdit::textEdited, this, &DemGeneration::autoLoadParams);
    connect(ui->outPrefixLineEdit, &QLineEdit::textChanged, imageCutter, &ImageCutter::setOutPrefix);
    connect(ui->outputPathLineEdit, &QLineEdit::textChanged, imageCutter, &ImageCutter::setOutPath);

    connect(ui->flightAltSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DemGeneration::flightAltitudeChanged);

    connect(ui->baselineSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DemGeneration::baselineChanged);

    connect(ui->frameShiftBtn, &QPushButton::released,
            this, &DemGeneration::launchFrameShiftView);

    connect(ui->inputPathLineEdit, &QLineEdit::textEdited,
            this, &DemGeneration::inputPathEdited);
    connect(ui->outputPathLineEdit, &QLineEdit::textChanged,
            this, &DemGeneration::outputPathEdited);
}

DemGeneration::~DemGeneration() {
    delete ui;
}

void DemGeneration::setInputPath(const QString &path) {
    QFileInfo info(path);
    setIOPath(ui->inputPathLineEdit, info);
    auto prefix = info.isDir() ? info.fileName() : info.dir().dirName();

    if (DemBehaviour::allow_logPrefixAsParent()) {
        // Set log prefix to parent directory name
        ui->logPrefixLineEdit->setText(prefix);
    }
    if (DemBehaviour::allow_outputPrefixAsParent()) {
        // Set output prefix to parent directory name
        ui->outPrefixLineEdit->setText(prefix);
    }
    autoLoadParams();
    checkIfVideo();
    Q_EMIT inputPathEdited(ui->inputPathLineEdit->text());
}

void DemGeneration::checkIfVideo() {
    auto inputPath = ui->inputPathLineEdit->text();
    QMimeDatabase db;
    auto mime = db.mimeTypeForFile(inputPath, QMimeDatabase::MatchContent);

    if (mime.name().contains(QLatin1String("video"))) {
        qInfo() << "Video file found.";
        setFormatMode(FormatMode::VIDEO);
        ui->frameRate->setDisabled(false);
        ui->sortOrderCmbo->setCurrentIndex(0);
        ui->sortOrderCmbo->setDisabled(true);
        return;
    }

    auto inputDir = QFileInfo(inputPath).isDir() ? QDir(inputPath) : QDir(QFileInfo(inputPath).dir());
    inputDir.setNameFilters(supportedVideoNameFilters());
    auto files = inputDir.entryList();
    for (const auto &f: files) {
        setFormatMode(FormatMode::VIDEO);
        ui->frameRate->setDisabled(false);
        ui->sortOrderCmbo->setCurrentIndex(0);
        ui->sortOrderCmbo->setDisabled(true);
        ui->inputPathLineEdit->setText(inputDir.filePath(f));
        return;
    }

    ui->frameRate->setDisabled(true);
    ui->sortOrderCmbo->setDisabled(false);
}

void DemGeneration::setOutputPath(const QString &path) {
    QFileInfo info(path);
    setIOPath(ui->outputPathLineEdit, info);
    Q_EMIT outputPathEdited(ui->outputPathLineEdit->text());
}

void DemGeneration::setIOPath(QLineEdit *pathField, const QFileInfo &info) {
    pathField->setText(info.absoluteFilePath());
}

void DemGeneration::launchFrameShiftView() {
    if (frameShiftModule != nullptr) frameShiftModule->deleteLater();

    if (imageHeight != 0 and imageWidth != 0)
        frameShiftModule = new FrameShiftModule(this, imageWidth, imageHeight);
    else
        frameShiftModule = new FrameShiftModule(this);

    if (ui->xShift->value() != 0)
        frameShiftModule->setShiftX(ui->xShift->value());

    if (ui->yShift->value() != 0)
        frameShiftModule->setShiftY(ui->yShift->value());

    connect(frameShiftModule, &QDialog::accepted, this,
            [=, this]() {
                ui->xShift->setValue(frameShiftModule->getShiftX());
                ui->yShift->setValue(frameShiftModule->getShiftY());
            }
    );

    connect(frameShiftModule, &FrameShiftModule::imageWidthChanged, this,
            [this](int i) {
                imageWidth = i;
                Q_EMIT imageWidthChanged(i);
            });

    connect(frameShiftModule, &FrameShiftModule::imageHeightChanged, this,
            [this](int i) {
                imageHeight = i;
                Q_EMIT imageHeightChanged(i);
            });

    QDir imageDir;
    if (this->formatMode == FormatMode::VIDEO) {
        auto videoFilePath = ui->inputPathLineEdit->text();
        imageDir.setPath(QString::fromStdString(pycall::makeFramesDir(videoFilePath.toStdString())));
        imageDir.setNameFilters(imageMimes());
        if (imageDir.count() <= 2)
            pycall::video2frames(ui->inputPathLineEdit->text().toStdString(),
                                 imageDir.path().toStdString(),
                                 ui->frameRate->value(),
                                 2);
    } else {
        imageDir.setPath(ui->inputPathLineEdit->text());
    }

    imageDir.setNameFilters(imageMimes());
    auto imageCount = imageDir.count();
    if (imageCount >= 2) {
        auto image_list = imageDir.entryInfoList(QDir::Files, getSortOrder());
        frameShiftModule->scene->refRect->addImage(image_list.at(0).filePath());
        frameShiftModule->scene->secRect->addImage(image_list.at(1).filePath());
    }

    frameShiftModule->exec();

}


void DemGeneration::setFlightAltitude(double m) {
    ui->flightAltSpinBox->setValue(m);
}

void DemGeneration::setCameraBaseline(double m) {
    ui->baselineSpinBox->setValue(m);
}

void DemGeneration::setImageWidth(double px) {
    if (int(px) != imageWidth)
            Q_EMIT imageWidthChanged(int(px));
    imageWidth = int(px);
    if (frameShiftModule == nullptr) return;
    frameShiftModule->setImageWidth(int(px));
}

void DemGeneration::setImageHeight(double px) {
    if (int(px) != imageWidth)
            Q_EMIT imageHeightChanged(int(px));
    imageHeight = int(px);
    if (frameShiftModule == nullptr) return;
    frameShiftModule->setImageHeight(int(px));
}

bool DemGeneration::saveParameters() {
    QJsonObject params;
    params["Window_size"] = ui->winSizeComboBox->currentText();
    params["Flight_altitude"] = ui->flightAltSpinBox->value();
    params["Metres_per_pixel"] = ui->pixelResBox->value();
    params["Phase_correlation_method"] = ui->phaseCorrComboBox->currentText();
    params["Step_size"] = ui->stepSizeSpinBox->value();
    params["Camera_baseline"] = ui->baselineSpinBox->value();
    params["Log_prefix"] = ui->logPrefixLineEdit->text();
    params["Output_prefix"] = ui->outPrefixLineEdit->text();
    auto frame_shift_params = QJsonObject{
            {"Reference_shift_x", 0},
            {"Reference_shift_y", 0},
            {"Secondary_shift_x", ui->xShift->value()},
            {"Secondary_shift_y", ui->yShift->value()}
    };
    params["Frame_shift"] = frame_shift_params;

    auto dirInfo = QFileInfo(ui->inputPathLineEdit->text());
    auto saveFileName = QString("%1_params.json").arg(dirInfo.baseName());

    auto savePath = QFileDialog::getSaveFileName(
            this,
            QStringLiteral("Save Parameters"),
            QDir(dirInfo.path()).filePath(saveFileName),
            "JSON (*.json)"
    );

    if (!savePath.isEmpty() and !savePath.isNull()) {
        QFile saveFile(savePath);
        if (!saveFile.open(QIODevice::WriteOnly)) {
            qWarning() << "Couldn't open file:" << savePath << "for writing.";
            return false;
        }
        qInfo() << "Params written to" << savePath;
        saveFile.write(QJsonDocument(params).toJson(QJsonDocument::Indented));
        return true;
    }
    return false;
}

bool DemGeneration::loadParameters(const QString &paramFilePath) {
    QFile loadFile;
    if ((paramFilePath.isNull() or paramFilePath.isEmpty())) {
        auto paramsPath = QFileDialog::getOpenFileName(
                this,
                QLatin1String("Open Settings file"),
                ui->inputPathLineEdit->text(),
                "JSON (*.json, *.JSON)"
        );
        loadFile.setFileName(QDir::toNativeSeparators(paramsPath));
    } else {
        loadFile.setFileName(QDir::toNativeSeparators(paramFilePath));
    }

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file for reading.");
        return false;
    }
    qInfo() << "Opening settings file:" << loadFile.fileName();

    auto saveData = loadFile.readAll();
    auto params = QJsonDocument::fromJson(saveData).object();

    ui->winSizeComboBox->setCurrentText(params["Window_size"].toString());
    ui->flightAltSpinBox->setValue(params["Flight_altitude"].toDouble());
    ui->pixelResBox->setValue(params["Metres_per_pixel"].toDouble());
    ui->phaseCorrComboBox->setCurrentText(params["Phase_correlation_method"].toString());
    ui->stepSizeSpinBox->setValue(params["Step_size"].toInt());
    ui->baselineSpinBox->setValue(params["Camera_baseline"].toDouble());
    ui->logPrefixLineEdit->setText(params["Log_prefix"].toString());
    ui->outPrefixLineEdit->setText(params["Output_prefix"].toString());

    auto frame_shift_opts = params["Frame_shift"].toObject();
    ui->xShift->setValue(frame_shift_opts["Secondary_shift_x"].toInt());
    ui->yShift->setValue(frame_shift_opts["Secondary_shift_y"].toInt());
    return true;
}

QStringList DemGeneration::getStdOpts() {
    auto method = ui->phaseCorrComboBox->currentIndex();
    return {
            QString("phaseCorrelationMethod=%1").arg(method),
            QString("winSize=%1").arg(ui->winSizeComboBox->currentText()),
            QString("step=%1").arg(ui->stepSizeSpinBox->text()),
            QString("cameraBaseline=%1").arg(ui->baselineSpinBox->value()),
            QString("stereoImageResolution=%1").arg(ui->pixelResBox->value()),
            QString("flightAltitude=%1").arg(ui->flightAltSpinBox->value())
    };
}

QStringList DemGeneration::getExtraOpts() {
    return {
            QString("frameRegistration=%1").arg((int) ui->frameRegChkBox->isChecked()),
            QString("robustRegistration=%1").arg((int) ui->robustRegiChkBox->isChecked()),
            QString("removeBorder=%1").arg((int) ui->rmBorderChkBox->isChecked())
    };
}

QString DemGeneration::getLogString() {
    auto stem_name = QString("%1_%2").arg(imagePairsCount).arg(imagePairsCount + 1);
    auto log_prefix = ui->logPrefixLineEdit->text();
    auto log_name = QString("%1_%2").arg(log_prefix, stem_name);
    auto log_path = fs::path(log_name.toStdString()).replace_extension(".log");
    auto log_string = QString("log=%1").arg(QString::fromStdString(log_path.string()));
    return log_string;
}

QString DemGeneration::getOutString() {
    auto stem_name = QString("%1_%2").arg(imagePairsCount).arg(imagePairsCount + 1);
    auto out_prefix = ui->outPrefixLineEdit->text();
    auto out_name = QString("%1_%2_*").arg(out_prefix, stem_name);
    auto out_path = fs::path(out_name.toStdString());
    out_path = fs::path{ui->outputPathLineEdit->text().toStdString()} / out_path;
    out_path.make_preferred();
    auto out_string = QString("out=%1").arg(QString::fromStdString(out_path.string()));
    return out_string;
}


void DemGeneration::autoLoadParams() {
    if (not DemBehaviour::allow_autoLoadSettings()) return;
//    qDebug() << "Autoload enabled, loading parameters file.";

    fs::path paramFilePath{ui->inputPathLineEdit->text().toStdString()};
    auto paramsName = paramFilePath.stem();
    fs::path paramsPath;
    if (is_directory(paramFilePath)) {
        try {
            paramsPath = (paramFilePath / paramsName).replace_filename(paramsName.stem().string() + "_params.json");
        } catch (std::invalid_argument &e) {
            std::cerr << e.what() << std::endl;
            return;
        }
    } else {
        paramsPath = paramFilePath;
        paramsPath = paramsPath.replace_filename(paramsName.stem().string() + "_params.json");
    }
    if (exists(paramsPath)) {
        loadParameters(QString::fromStdString(paramsPath.string()));
    }
}

void DemGeneration::selectInput() {
    auto inputPath = ui->inputPathLineEdit->text();
    auto dialog = GroupFileObjectDialog(this,
                                        QLatin1String("Select Path to Input"),
                                        inputPath.isEmpty() ? PathSettings::default_tigerInputDir()
                                                            : inputPath);
    if (formatMode == FormatMode::VIDEO) {
        dialog.setFileMode(QFileDialog::ExistingFile);
    }
    if (dialog.exec()) {
        QString imagePath;
        imagePath = dialog.selectedFiles().at(0);
        imagePath = QDir::toNativeSeparators(imagePath);
        setInputPath(imagePath);
    }
}

void DemGeneration::selectOutput() {
    QString outputPath;
    outputPath = QFileDialog::getExistingDirectory(this,
                                                   QLatin1String("Select Output Path"),
                                                   ui->inputPathLineEdit->text().isEmpty()
                                                   ? PathSettings::default_tigerOutputDir()
                                                   : ui->inputPathLineEdit->text());
    outputPath = QDir::toNativeSeparators(outputPath);
    setOutputPath(outputPath);
}

void DemGeneration::createProgressDialog() {
    if (not pd.isNull()) return;
    pd = new QProgressDialog("Processing Image Pairs", "Stop", 0, 1,
                             this, windowFlags() &
                                   ~Qt::WindowContextHelpButtonHint &
                                   ~Qt::WindowMaximizeButtonHint);
    pd->setWindowTitle("Performing DEM Generation");
    pd->setMinimumDuration(0);
    connect(pd, &QProgressDialog::canceled, this, &DemGeneration::stopClicked);

    connect(scriptLauncher, &ScriptLauncher::procFinished,
            pd, [=, this]() {
                pd->setMaximum(imagePairsCount);
                pd->setValue(scriptLauncher->numReturned());
            });

    pd->setValue(pd->minimum());
}

void DemGeneration::runClicked() {
    QDir dir(PathSettings::default_CropPicDir());
    if (not dir.exists())
        dir.mkpath(".");

    if (not checkDemExe()) {
        Messages::warning_msg(this, "DEM Executable could not be found, check path in application settings.");
        return;
    }

    if (QFileInfo(PathSettings::getDemExePath()).fileName() !=
        QFileInfo(PathSettings::default_DemExePath()).fileName()) {
        Messages::warning_msg(this, "Executable must match ExerciseDemGeneration.exe");
        return;
    }
    if (ui->inputPathLineEdit->text().isEmpty()) {
        Messages::warning_msg(this, "Input path must be set.");
        return;
    }
    resetOperation();
    checkIfVideo();

    bool ok;
    switch (formatMode) {
        case FormatMode::FILES:
            ok = processFiles();
            break;
        case FormatMode::VIDEO:
            ok = processVideo();
            break;
        case FormatMode::CAMERA:
            ok = processCamera();
            break;
        case FormatMode::FOLDER:
            ok = processFolder();
            break;
    }
    if (not ok) return;

    // extra handle progress bar
    ui->runBtn->setDisabled(true);
    ui->stopBtn->setDisabled(false);
}

void DemGeneration::checkResults() {
    if (scriptLauncher->numReturned() == imagePairsCount) {

        bool cameraActive = false;
        if (not cameraWatchdog.isNull())
            cameraActive = cameraWatchdog->isActive();

        bool dirWatched = false;
        if (not directoryWatchdog.isNull())
            dirWatched = directoryWatchdog->isActive();

        if (not cameraActive or not dirWatched) {
            if (not pd.isNull()) pd->setValue(pd->maximum());
            Messages::info_msg(this, "DEM Generation Complete");
            resetOperation();
        }
    }
}

bool DemGeneration::processFiles() {
    auto imageDir = QDir(ui->inputPathLineEdit->text());
    imageDir.setNameFilters(imageMimes());
    if (imageDir.count() > 0) {
        createProgressDialog();
        auto image_list = imageDir.entryInfoList(QDir::Files, getSortOrder());
        for (auto &img: image_list) {
            qInfo().nospace() << "Image name: " << img.filePath();
            qInfo() << " FileModificationTime : " << img.fileTime(QFile::FileModificationTime).toString();
        }
        qInfo() << "pairs";
        auto pairwise_image_list = pairwise(image_list);

        for (auto &images: pairwise_image_list) {
            qInfo() << "Image(1):" << images[0].filePath();
            qInfo() << "Image(2):" << images[1].filePath() << "\n";
            processImagePair(images[0].filePath(), images[1].filePath());
        }
        return true;
    }
    return false;
}

bool DemGeneration::processVideo() {
    qInfo() << "Processing video ...";
    auto frameRate = ui->frameRate->value();
    if (frameRate == 0) {
        bool ok;
        double d = QInputDialog::getDouble(this, QLatin1String("Input Frame Rate"),
                                           tr("Frame Rate:"), 0.5,
                                           ui->frameRate->minimum(),
                                           ui->frameRate->maximum(),
                                           ui->frameRate->decimals(),
                                           &ok,
                                           windowFlags() &
                                           ~Qt::WindowContextHelpButtonHint &
                                           ~Qt::WindowMinMaxButtonsHint,
                                           1);
        if (not ok) {
            qDebug() << "Video processing cancelled: No frame rate provided.";
            return false;
        }
        frameRate = d;
        ui->frameRate->setValue(d);
    }
    auto videoFilePath = ui->inputPathLineEdit->text();
    // make the frames directory
    auto framesDir = QString::fromStdString(pycall::makeFramesDir(videoFilePath.toStdString()));
    if (framesDir.isEmpty() or framesDir.isNull()) {
        Messages::warning_msg(this, "Video file could not be processed");
        return false;
    }
    createProgressDialog();
    // create directoryWatchdog to watch the output folder
    if (directoryWatchdog.isNull())
        createDirectoryWatchdog();
    directoryWatchdog->setWatchDir(framesDir);
    directoryWatchdog->startWatch();

//    py::gil_scoped_release release; // add this to release the GIL
    auto videoThread = new VideoThread(this, videoFilePath, framesDir, frameRate);
    connect(videoThread, &VideoThread::resultReady, directoryWatchdog, [this](){
        directoryWatchdog->stopWatch();
    });
    connect(videoThread, &VideoThread::finished, videoThread, &QObject::deleteLater);
    videoThread->start();
    return true;
}

bool DemGeneration::processFolder() {
    if (directoryWatchdog.isNull())
        createDirectoryWatchdog();
    createProgressDialog();
    directoryWatchdog->startWatch();
    qInfo() << "Waiting for new files to come in to:" << directoryWatchdog->getWatchedDir();
    return true;
}

bool DemGeneration::processCamera() {
    if (cameraWatchdog.isNull()) {
        return false;
    }
    createProgressDialog();
    qInfo() << "Running camera watchdog process.";
    cameraWatchdog->startWatch();
    return true;
}

void DemGeneration::processImagePair(const QString &refImage, const QString &secondaryImage) {
    if (not QFileInfo::exists(refImage)) {
        Messages::warning_msg(this, QString("Image file %1 does not exist").arg(refImage));
        return;
    }
    if (not QFileInfo::exists(secondaryImage)) {
        Messages::warning_msg(this, QString("Image file %1 does not exist").arg(refImage));
        return;
    }

    qInfo() << "Processing images " << refImage << " " << secondaryImage;
    // Image cutter will crop the input image and store it in C:\\Tiger\\Data\\Input\\Crop_Pic
    // the input image can be from any input directory because of this
    imageCutter->xShift = ui->xShift->value();
    imageCutter->yShift = ui->yShift->value();
    auto[refPath_cropped, secondaryPath_cropped] = imageCutter->imageCut(refImage, secondaryImage,
                                                                         imagePairsCount, imagePairsCount + 1);
    if (refPath_cropped.empty()) {
        Messages::warning_msg(this, QString("Image file %1 could not be read").arg(refImage));
        return;
    }
    if (secondaryPath_cropped.empty()) {
        Messages::warning_msg(this, QString("Image file %1 could not be read").arg(refImage));
        return;
    }
    auto dataDir = fs::path(PathSettings::default_dataDir().toStdString());
    // remove the prefix C:\\Tiger\Data, as it is prepended automatically by DEM generation process
    fs::path refPath_sub = isSubPath(dataDir, absolute(refPath_cropped));
    fs::path secPath_sub = isSubPath(dataDir, absolute(secondaryPath_cropped));

    QStringList args;
    args = QStringList{
            QString::fromStdString(refPath_sub.string()),
            QString::fromStdString(secPath_sub.string()),
            getLogString(),
            getOutString()
    };


    args += getStdOpts();
    args += getExtraOpts();

    qInfo() << "reference image path:" << QString::fromStdString(secPath_sub.string());
    qInfo() << "secondary image path:" << QString::fromStdString(refPath_sub.string());

    auto working_dir = QString::fromStdString(
            fs::path{PathSettings::getDemExePath().toStdString()}
                    .parent_path()
                    .string()
    );

    scriptLauncher->launchProc(working_dir, PathSettings::getDemExePath(), args);

    imagePairsCount += 1;
}

void DemGeneration::resetOperation() {
    if (not pd.isNull()) pd->deleteLater();
    scriptLauncher->reset();
    imagePairsCount = 0;
    ui->stopBtn->setDisabled(true);
    ui->runBtn->setDisabled(false);
}

void DemGeneration::cancelClicked() {
    // implicitly kill procs on close event
    qInfo() << "Cancelling DEM Generation and closing.";
    parentWidget()->deleteLater();
}

void DemGeneration::stopClicked() {
    scriptLauncher->killProcs();
    resetOperation();
    if (not cameraWatchdog.isNull())
        cameraWatchdog->stopWatch();
    if (not directoryWatchdog.isNull())
        directoryWatchdog->stopWatch();
    qInfo() << "Stopping DEM Generation.";
}

bool DemGeneration::checkDemExe() {

    // check if specified exe is available
    auto exePath = PathSettings::getDemExePath();
    if (exePath.isNull() or exePath.isEmpty()) {
        // extra: check if .exe available at C:/
        return false;
    }
    if (QFileInfo::exists(exePath))
        return true;

    return false;
}

QString DemGeneration::getCropDir() {
    return QStringLiteral(u"C:/Tiger/Data/Input/Crop_Pic");
}

QFlags<QDir::SortFlag> DemGeneration::getSortOrder() {
    QFlags<QDir::SortFlag> sort_type;
    switch (ui->sortOrderCmbo->currentIndex()) {
        case 0: // oldest first
            sort_type = QDir::Time | QDir::Reversed;
            break;
        case 1: // newest first
            sort_type = QDir::Time;
            break;
        case 2: // Name Ascending
            sort_type = QDir::Name;
            break;
        case 3: // Name Descending
            sort_type = QDir::QDir::Name | QDir::Reversed;
            break;
    }
    return sort_type;
}

QStringList DemGeneration::imageMimes() {
    QStringList mimes = {"*.jpg", "*.jpeg", "*.png", "*.tiff", "*.tif", "*.bmp",
                         "*.gif", "*.heic", "*.heif"};
    QStringList capMimes;
    for (const auto &m: mimes) {
        capMimes.append(m.toUpper());
    }
    return mimes + capMimes;
}

void DemGeneration::setFormatMode(FormatMode mode) {
    formatMode = mode;
}

FormatMode DemGeneration::getFormatMode() {
    return formatMode;
}

void DemGeneration::selectCamera() {
    if (cameraWatchdog.isNull())
        createCameraWatchdog();
    cameraWatchdog->chooseCamera();
    // outcomes are handled in `createCameraWatchdog` using signals
}

void DemGeneration::createCameraWatchdog() {
    if (not cameraWatchdog.isNull()) return;
    cameraWatchdog = new CameraWatchdog(this, ui->inputPathLineEdit->text().toStdString());

    connect(ui->inputPathLineEdit, &QLineEdit::textChanged,
            cameraWatchdog, &CameraWatchdog::setDestination);

    connect(cameraWatchdog, &CameraWatchdog::cameraChosen, this,
            [=, this](const QString &name) {
                setFormatMode(FormatMode::CAMERA);
                chooseCamWidget->setWatchType(WatchType::STANDBY);
                chooseCamWidget->connectionBtn->setDisabled(true);
                chooseCamWidget->connectionBtn->setText(QLatin1String("Device Connected"));
                Q_EMIT cameraFound(name);
            });

    connect(cameraWatchdog, &CameraWatchdog::deviceDisconnected, this,
            [=, this]() {
                setFormatMode(FormatMode::FILES);
                chooseCamWidget->setWatchType(WatchType::DISABLED);
                chooseCamWidget->connectionBtn->setDisabled(false);
            });

    connect(cameraWatchdog, &CameraWatchdog::imageReceived,
            this, &DemGeneration::processImagePair);

    connect(cameraWatchdog, &CameraWatchdog::watchStarted,
            this, [this]() { chooseCamWidget->setWatchType(WatchType::ACTIVE); });

    connect(cameraWatchdog, &CameraWatchdog::watchStopped,
            this, [this]() { chooseCamWidget->setWatchType(WatchType::STANDBY); });

    connect(cameraWatchdog, &CameraWatchdog::deviceDisconnected,
            this, [this]() { chooseCamWidget->setWatchType(WatchType::DISABLED); });

}

void DemGeneration::createDirectoryWatchdog() {
    if (not directoryWatchdog.isNull()) return;

    directoryWatchdog = new DirectoryWatchdog(this);
    connect(ui->inputPathLineEdit, &QLineEdit::textChanged,
            directoryWatchdog, &DirectoryWatchdog::setWatchDir);
    connect(ui->inputPathLineEdit, &QLineEdit::textEdited,
            directoryWatchdog, &DirectoryWatchdog::setWatchDir);
    connect(directoryWatchdog, &DirectoryWatchdog::imagePairReady,
            this, &::DemGeneration::processImagePair);
    connect(directoryWatchdog, &DirectoryWatchdog::watchDirSet, this,
            [=, this]() {
                setFormatMode(FormatMode::FOLDER);
                setupDirWatchWidget->setWatchType(WatchType::STANDBY);
            });
    connect(directoryWatchdog, &DirectoryWatchdog::watchStarted, this,
            [=, this]() {
                setupDirWatchWidget->setWatchType(WatchType::ACTIVE);
            });
    connect(directoryWatchdog, &DirectoryWatchdog::watchStopped, this,
            [=, this]() {
                setupDirWatchWidget->setWatchType(WatchType::DISABLED);
                setFormatMode(FormatMode::FILES);
            });
    directoryWatchdog->setWatchDir(ui->inputPathLineEdit->text());
}

WatchdogIndicatorWidget *DemGeneration::getChooseCameraWidget() {
    if (chooseCamWidget.isNull()) {
        chooseCamWidget = new WatchdogIndicatorWidget();
        connect(chooseCamWidget->connectionBtn, &QPushButton::released,
                this, &DemGeneration::selectCamera);
    }

    return chooseCamWidget;
}

WatchdogIndicatorWidget *DemGeneration::getWatchDirWidget() {
    if (setupDirWatchWidget.isNull()) {
        setupDirWatchWidget = new WatchdogIndicatorWidget();
        setupDirWatchWidget->connectionBtn->setText("Watch Folder");
        setupDirWatchWidget->indicator->setToolTip("");
        connect(setupDirWatchWidget->connectionBtn, &QPushButton::released,
                this, &DemGeneration::createDirectoryWatchdog);
    }

    return setupDirWatchWidget;
}

template<class T>
QList<T> pairwise(const T &values) {
    QList<T> pairwiseList;
    if (!values.empty()) {
        auto end = values.cend() - 1;
        for (auto it = values.cbegin(); it != end; ++it) {
            pairwiseList.append(T{*it, *(it + 1)});
        }
    }
    return pairwiseList;
}

std::string isSubPath(const fs::path &base, const fs::path &testPath) {
    auto diff = testPath.lexically_relative(base);
    if (diff.string().size() == 1 || diff.string()[0] != '.' && diff.string()[1] != '.')
        return diff.string();
    else
        return testPath.string();
}

void VideoThread::run() {
    QString result;
    qInfo() << "Running video process.";
    auto framesDir = QDir(m_framesDir);
    framesDir.setFilter(QDir::Files);
    for (const auto &dirFile: framesDir.entryList()) {
        framesDir.remove(dirFile);
    }

    pycall::video2frames(m_videoFilePath.toStdString(),
                         m_framesDir.toStdString(),
                         m_frameRate);
    Q_EMIT resultReady(result);
}

VideoThread::VideoThread(QObject *parent, QString videoFilePath, QString framesDir, double frameRate) :
        QThread(parent),
        m_videoFilePath(std::move(videoFilePath)),
        m_framesDir(std::move(framesDir)),
        m_frameRate(frameRate) {}
