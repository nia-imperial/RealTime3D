//
// Created by Nic on 19/03/2022.
//

#ifndef WAYPOINTER_WORKSPACE_H
#define WAYPOINTER_WORKSPACE_H


#include <QObject>
#include "missionscene.h"
#include "systemviewer.h"
#include <QLineEdit>

class LayerPanel;
class CoordinatePanel;

class Workspace : public QObject {
Q_OBJECT
public:
    explicit Workspace(QWidget *parent,
                       QLineEdit *ui_coordLineEdit,
                       LayerPanel *parent_layerPanel,
                       CoordinatePanel *parent_CoordinatePanel);

    LayerPanel *layerPanel;
    CoordinatePanel *coordinatePanel;

    ToolModes *toolMode;
    MissionScene *missionScene;
    SystemViewer *systemViewer;
    QLineEdit *coordLineEdit;

public Q_SLOTS:
    void setCoordView(const QPoint &pos) const;

    void setCoordView(const QVector3D &pos) const;

    [[nodiscard]] QVector3D getCoordView() const;

    void highlightRow(const QModelIndex &rowIndex) const;

    void receiveWaypointRequest(const QVector3D &pos, bool connectPrev) const;

private Q_SLOTS:
    void connectWaypointSignals(Waypoint *waypoint);

};


#endif //WAYPOINTER_WORKSPACE_H
