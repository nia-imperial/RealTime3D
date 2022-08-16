//
// Created by Nic on 13/03/2022.
//

#ifndef REALTIME3D_MISSIONSCENE_H
#define REALTIME3D_MISSIONSCENE_H


#include <QGraphicsScene>
#include <QPointer>
#include "waypoint.h"

class ToolModes;

class MissionScene : public QGraphicsScene {
    Q_OBJECT
    ControlPoint *startItem;
    ConnectionLine* newConnection;
    ToolModes *toolMode;
public:
    explicit MissionScene(QObject *parent, ToolModes* pToolModes);

public Q_SLOTS:

    ControlPoint *controlPointAt(const QPointF& point);

    Waypoint *waypointAt(const QPointF& point);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private Q_SLOTS:
    static QPainterPath makeMask();

};


#endif //REALTIME3D_MISSIONSCENE_H
