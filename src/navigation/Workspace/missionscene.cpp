//
// Created by Nic on 13/03/2022.
//

#include "missionscene.h"
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>
#include "toolmode.h"

MissionScene::MissionScene(QObject *parent, ToolModes* parent_toolModes) :
        QGraphicsScene(parent),
        toolMode(parent_toolModes),
        startItem(nullptr),
        newConnection(nullptr){
    setObjectName("MissionScene");
}

void MissionScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        auto ctrlPoint = controlPointAt(event->scenePos().toPoint());
        if (ctrlPoint != nullptr and toolMode->connectMode->isActive()){
            startItem = ctrlPoint;
            newConnection = new ConnectionLine(ctrlPoint,
                                               event->scenePos().toPoint(),
                                               ctrlPoint->color);
            addItem(newConnection);
            event->accept();
            return;
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void MissionScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (newConnection != nullptr) {
        auto item = controlPointAt(event->scenePos());
        QPointF p2;
        if (item != nullptr and item != startItem)
            p2 = item->scenePos();
        else
            p2 = event->scenePos();
        newConnection->setP2(p2);
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void MissionScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (newConnection != nullptr) {
        auto ctrlPoint = controlPointAt(event->scenePos());
        if (ctrlPoint != nullptr and ctrlPoint != startItem) {
            newConnection->setEnd(ctrlPoint);
            if (startItem->addLine(newConnection))
                ctrlPoint->addLine(newConnection);
            else {
                startItem->removeLine(newConnection);
                removeItem(newConnection);
            }
        } else
            removeItem(newConnection);
    }
    startItem = nullptr;
    newConnection = nullptr;

    QGraphicsScene::mouseReleaseEvent(event);
}

QPainterPath MissionScene::makeMask() {
    auto mask = QPainterPath();
    mask.setFillRule(Qt::WindingFill);
    return mask;
}

ControlPoint *MissionScene::controlPointAt(const QPointF &point) {
    auto mask = makeMask();
    for (auto item: items(point)) {
        if (mask.contains(point))
            return nullptr;
        auto ctrlPoint = qgraphicsitem_cast<ControlPoint *>(item);
        if (ctrlPoint != nullptr)
            return ctrlPoint;

        auto connection = qgraphicsitem_cast<ConnectionLine *>(item);
        if (connection == nullptr)
            mask.addPath(item->shape().translated(item->scenePos()));
    }
    return nullptr;
}

Waypoint *MissionScene::waypointAt(const QPointF &point) {
    auto mask = makeMask();
    for (auto item: items(point)) {
        if (mask.contains(point))
            return nullptr;
        auto pWaypoint = qgraphicsitem_cast<Waypoint *>(item);
        if (pWaypoint != nullptr)
            return pWaypoint;

        auto connection = qgraphicsitem_cast<ConnectionLine *>(item);
        if (connection == nullptr)
            mask.addPath(item->shape().translated(item->scenePos()));
    }
    return nullptr;
}
