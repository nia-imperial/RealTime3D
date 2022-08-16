//
// Created by Nic on 23/02/2022.
//

#ifndef REALTIME3D_DEMGENERATION_H
#define REALTIME3D_DEMGENERATION_H

#include <QWidget>
#include <QLineEdit>
#include <QSettings>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QThread>
#include <QProgressDialog>
#include "../frame_shift_viewer/FrameShiftModule.h"
#include "imagecutter.h"
#include "../utility/scriptlauncher.h"
#include "../utility/CameraWatchdog.hpp"
#include "../utility/DirectoryWatchdog.hpp"
#include "../main_window/WatchdogIndicator.h"

/// Class for handling video processes on a separate thread
class VideoThread : public QThread {
Q_OBJECT
    /// the path to the video file
    QString m_videoFilePath;
    /// the directory where frames will be placed
    QString m_framesDir;
    /// the number of frames to collect per second of video
    double m_frameRate;

    void run() override;

public:
    explicit VideoThread(QObject *parent, QString videoFilePath, QString framesDir, double frameRate);

Q_SIGNALS:

    /// signal indicating that the video has been processed
    void resultReady(const QString &s);
};

/// Enum for data source formats
enum class FormatMode {
    FILES, ///< the format source comes from files on disk
    VIDEO, ///< the format source is a video
    CAMERA, ///< the format source is a camera
    FOLDER ///< the format source is a folder that should be monitored
};

//enum SortOrder{
//    OLDEST = QDir::Time | QDir::Reversed,
//    NEWEST = QDir::Time,
//    ASCENDING = QDir::Name,
//    DESCENDING = QDir::QDir::Name | QDir::Reversed
//};

/// Creates a list of paired images
/// @param values a list of images to be paired
template<class T>
QList<T> pairwise(const T &values);

/// Returns only the part of testPath that doesnt match the `base` or
/// if no match found then return testPath. The DEM generation process will attempt to prefix
/// all files paths with C:/TIGER/Data
/// @param base the base file path
/// @param testPath the path used to test against `base`
std::string isSubPath(const fs::path &base, const fs::path &testPath);

QT_BEGIN_NAMESPACE
namespace Ui { class DemGeneration; }
QT_END_NAMESPACE

/// The main class/widget for handling DEM generation processes
class DemGeneration : public QWidget {
Q_OBJECT

public:
    explicit DemGeneration(QWidget *parent = nullptr, QFileSystemModel *parent_fsModel = nullptr);

    ~DemGeneration() override;

    /// Returns the directory used for cropped images
    static QString getCropDir();

    /// Returns the supported image mimetypes
    static QStringList imageMimes();

    WatchdogIndicatorWidget *getChooseCameraWidget();

    WatchdogIndicatorWidget *getWatchDirWidget();

    QPointer<FrameShiftModule> frameShiftModule;

Q_SIGNALS:

    /// signal indicating the image width has changed
    void imageWidthChanged(int i);

    /// signal indicating the image height has changed
    void imageHeightChanged(int i);

    /// signal indicating the flight altitude has changed
    void flightAltitudeChanged(double d);

    /// signal indicating the camera baseline has changed
    void baselineChanged(double d);

    /// single indicating that a camera has been found
    void cameraFound(const QString &cameraName);

    /// signal indicating the input path has been edited
    void inputPathEdited(const QString &inputPath);

    /// signal indicating the output path has been edited
    void outputPathEdited(const QString &outputPath);

public Q_SLOTS:

    ///@param path the input path string
    void setInputPath(const QString &path);

    ///@param path the output path string
    void setOutputPath(const QString &path);

    /// launch the frame shift view window
    void launchFrameShiftView();

    /// set the flight altitude
    ///@param m distance above ground in metres
    void setFlightAltitude(double m);

    /// set the camera baseline distance
    ///@param m distance between cameras/images in metres
    void setCameraBaseline(double m);

    /// set the image width
    ///@param px the width in pixels
    void setImageWidth(double px);

    /// set the image height
    ///@param px the height in pixels
    void setImageHeight(double px);

    /// set the data source format mode
    void setFormatMode(FormatMode mode);

    /// get the current source format mode
    FormatMode getFormatMode();

private:
    /// the number of image pairs
    int imagePairsCount;
    /// the height of the image in px
    int imageHeight;
    /// the width of the image in px
    int imageWidth;
    /// the current source format mode
    FormatMode formatMode;

    static void setIOPath(QLineEdit *pathField, const QFileInfo &info);

    Ui::DemGeneration *ui;
    QPointer<ImageCutter> imageCutter;
    QPointer<ScriptLauncher> scriptLauncher;
    QPointer<CameraWatchdog> cameraWatchdog;
    QPointer<DirectoryWatchdog> directoryWatchdog;
    QPointer<WatchdogIndicatorWidget> setupDirWatchWidget;
    QPointer<WatchdogIndicatorWidget> chooseCamWidget;
    QPointer<QProgressDialog> pd;
    QCompleter *completer;
    //    QPointer<VideoConverter> videoConverter;

private Q_SLOTS:

    /// handle the run button being clicked
    void runClicked();

    /// handle the cancel button being clicked
    void cancelClicked();

    /// handle the stop button being clicked
    void stopClicked();

    /// handle the select input button being clicked
    void selectInput();

    /// handle the select output button being clicked
    void selectOutput();

    /// handle the select camera button being clicked
    void selectCamera();

    /// creates the camera watchdog module
    void createCameraWatchdog();

    /// creates the directory watchdog module
    void createDirectoryWatchdog();

    /// creates the standard options used for the DemGeneration.exe process
    QStringList getStdOpts();

    /// creates the additional options used for the DemGeneration.exe process
    QStringList getExtraOpts();

    /// creates the string for printing to the log
    QString getLogString();

    /// creates the string for printing to stdout
    QString getOutString();

    /// save the project parameters to to a json file
    bool saveParameters();

    bool loadParameters(const QString &paramFilePath = "");

    /// automatically loads the project parameters from .json files if available
    void autoLoadParams();

    /// checks that the DemGeneration.exe file exists
    static bool checkDemExe();

    /// gets the order for images to be sorted in
    QFlags<QDir::SortFlag> getSortOrder();

    /// resets the class and UI after processing
    void resetOperation();

    /// processes the image file pairs
    bool processFiles();

    /// process the frames of the video as image pairs
    bool processVideo();

    /// process the incoming images from a camera
    bool processCamera();

    /// process a pair of images
    ///@param refImage the reference image
    ///@param secondaryImage the secondary image that is matched to the reference
    void processImagePair(const QString &refImage, const QString &secondaryImage);

    /// checks to see if watchdog is active or if processing is finished
    void checkResults();

    /// setup a watchdog on the directory to process new incoming files
    bool processFolder();

    /// check if the input is a video file
    void checkIfVideo();

    /// create the progress bar dialog
    void createProgressDialog();
};


#endif //REALTIME3D_DEMGENERATION_H
