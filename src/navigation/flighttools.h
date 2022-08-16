//
// Created by Nic on 13/03/2022.
//

#ifndef REALTIME3D_FLIGHTTOOLS_H
#define REALTIME3D_FLIGHTTOOLS_H


#include <QWidget>
#include <QFileSystemWatcher>
#include <QStatusBar>
#include <QDir>
#include <QLabel>
#include <QTabWidget>
#include <QPointer>

class Workspace;

class LayerPanel;

class FlightTools : public QObject {
    Q_OBJECT
    bool imageDirIsSet;
    Workspace *workspace;
    LayerPanel *layerPanel;
    QFileSystemWatcher *watcher;
    QTabWidget *editorTabs;


public:
    explicit FlightTools(QWidget *parent,
                         QAction *actionImageViewMatching,
                         QAction *actionSimulateImage,
                         QAction *actionSimulateFlightImages,
                         QAction *actionConnectToCamera,
                         QStatusBar *ui_statusBar,
                         QTabWidget *ui_editorTabs,
                         Workspace *parent_workspace,
                         LayerPanel *parent_layerPanel);

    static QStringList NameFilters();
    int imageCount;

    QString imagesDirectory;
    QLabel *imagesDirLabel;
    QString outputDir;

public Q_SLOTS:
    static void fn_fileChanged(const QString& path);

    void fn_DirChanged(const QString& path);

    bool baseMapItemOk();

    [[nodiscard]] bool imageDirSet() const;

    void addFlightImage(const QString& filePath, int xPos, int yPos);

    void imageViewMatching(const QString& imageFileName = "");

    void connectToCamera();

private Q_SLOTS:
    void simulateFlightImages(int w=550, int h=500);
    void addSimulatedImage();


};


#endif //REALTIME3D_FLIGHTTOOLS_H
