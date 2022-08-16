//
// Created by Nic on 22/02/2022.
//

#ifndef REALTIME3D_MAIN_WINDOW_H
#define REALTIME3D_MAIN_WINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QMenu>
#include <QFileDialog>
#include <QDockWidget>
#include <QSocketNotifier>
#include "../settings/settingsmenu.h"
#include "../dem_generation/demgeneration.h"
#include "../device_profiles/overviewprofile.h"
#include "../flight_parameters/FlightParameters.h"
#include "../navigation/maininterface.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QPlainTextEdit *consoleWidget, QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

public:

    ~MainWindow() override;

    QFileSystemModel *fileModel;
    QString projectInput;
    QString projectOutput;

private:
    Ui::MainWindow *ui;
    SettingsMenu *settingsMenu;
    OverviewProfile *overviewProfile;
    QPointer<DemGeneration> demGeneration;
    QPointer<FlightParameters> parameterCalculator;
    QPointer<MainInterface> navigationApp;
    QFileDialog fileOrDirDialog;

    QDockWidget *overviewDock;
    QDockWidget *aircraftDock;
    QDockWidget *cameraDock;
    QDockWidget *lensDock;
    QDockWidget *consoleDock;

private Q_SLOTS:

    void createNewProject(const QString &newName);

    void openProject(const QString &filePath = QString());

    void setDemGenIO(const QString &input = QString(),
                     const QString &output = QString());

    static void showInFolder(const QString &path);

    void setupProfiles();

    void launchDemGeneration();

    void showContextMenu(const QPoint &pos);

    void setFileRoot();

    void runDat2Tiff(const QString &input);

    void launchNavigation();

    [[maybe_unused]] void runNewCameraCalibration();

    void runGeometricCorrection(const QString &input = QString());

    void createNewCameraProfile();

    void createNewLensProfile();

};


#endif //REALTIME3D_MAIN_WINDOW_H
