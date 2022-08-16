//
// Created by Nic on 19/03/2022.
//

#include "workspace.h"
#include "../layerpanel.h"
#include "../coordinatepanel.h"
#include <QApplication>

Workspace::Workspace(QWidget *parent,
                     QLineEdit *ui_coordLineEdit,
                     LayerPanel *parent_layerPanel,
                     CoordinatePanel *parent_CoordinatePanel) :
        QObject(parent),
        coordLineEdit(ui_coordLineEdit),
        layerPanel(parent_layerPanel),
        coordinatePanel(parent_CoordinatePanel),
        toolMode(new ToolModes(this)),
        missionScene(new MissionScene(this, toolMode)),
        systemViewer(new SystemViewer(parent, toolMode, layerPanel, coordinatePanel)) {

    systemViewer->setScene(missionScene);

    connect(systemViewer, &SystemViewer::sendImageLocation,
            this, qOverload<const QPoint &>(&Workspace::setCoordView));

    connect(systemViewer, &SystemViewer::waypointAdded,
            this, &Workspace::connectWaypointSignals);

}

QVector3D Workspace::getCoordView() const {
    QVector3D coordLine;
    auto text = coordLineEdit->text();
    if (text.isNull() or text.isEmpty()) return coordLine;
    auto parts = text.split(",");
    coordLine.setX(parts.at(0).toFloat());
    if (parts.count() >= 2)
        coordLine.setY(parts.at(1).toFloat());
    if (parts.count() >= 3)
        coordLine.setZ(parts.at(2).toFloat());

    return coordLine;
}

void Workspace::setCoordView(const QPoint &pos) const {
    setCoordView(QVector3D{(float) pos.x(), (float) pos.y(), 0});
}

void Workspace::setCoordView(const QVector3D &pos) const {
    auto coords = pos;
    if (coords.x() == 0) {
        coords.setX(getCoordView().x());
    }
    if (coords.y() == 0) {
        coords.setY(getCoordView().y());
    }
    if (coords.z() == 0){
        coords.setZ(getCoordView().z());
    }
    coordLineEdit->setText(QString("%1, %2, %3").arg(coords.x()).arg(coords.y()).arg(coords.z()));
}

void Workspace::connectWaypointSignals(Waypoint *waypoint) {
    connect(waypoint->signals, &WayPointSignals::itemMoved,
            this, [this]() {
                coordinatePanel->waypointView->viewport()->update();
            });
    connect(waypoint->signals, &WayPointSignals::itemMoved,
            this, [this]() { coordinatePanel->targetView->viewport()->update(); });
    connect(waypoint->signals, &WayPointSignals::itemSelected, this, &Workspace::highlightRow);
}

void Workspace::highlightRow(const QModelIndex &rowIndex) const {
    auto focused = QApplication::focusWidget();
    if (focused == systemViewer) {
        QItemSelectionModel *selector;
        if (rowIndex.model()->objectName() == QLatin1String("WaypointList"))
            selector = coordinatePanel->waypointView->selectionModel();
        else
            selector = coordinatePanel->targetView->selectionModel();
        selector->select(rowIndex,
                         QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
    }
}

void Workspace::receiveWaypointRequest(const QVector3D &pos, bool connectPrev) const {
    auto prev_wp = systemViewer->lastWaypointPlaced;
    auto waypoint = systemViewer->addWaypoint(pos);
    if (connectPrev) {
        if (coordinatePanel->waypointsTable->rowCount() > 1) {
            if (prev_wp)
                waypoint->connectTo(prev_wp);
        }
    }

}

