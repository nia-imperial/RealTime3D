//
// Created by Nic on 13/03/2022.
//

#ifndef REALTIME3D_SYSTEMVIEWER_H
#define REALTIME3D_SYSTEMVIEWER_H

#include <QPointer>
#include <QGraphicsView>
#include <QMouseEvent>
#include "toolmode.h"

class LayerPanel;

class LayerData;

class CoordinatePanel;

class Waypoint;

enum class PointType;

class SystemViewer : public QGraphicsView {
Q_OBJECT

    LayerPanel *layerPanel;
    CoordinatePanel *coordinatePanel;
    ToolModes *toolMode;
    int zoom;
    double factor;
    bool empty;

public:
    explicit SystemViewer(QWidget *parent,
                          ToolModes *parent_toolModes,
                          LayerPanel *parent_layerPanel,
                          CoordinatePanel *parent_coordinatePanel);

    QCursor originalCursor;
    bool hasStoredOriginalCursor;
    bool useLastMouseEvent;
    QMouseEvent *lastMouseEvent;
    bool handScrolling;
    int handScrollingMotions;
    QGraphicsView::DragMode m_dragMode;

    QPointF lastMouseMoveScenePoint;
    QPointF lastMouseMoveScreenPoint;

    Waypoint *lastWaypointPlaced;
    Waypoint *lastWaypointSelected;

Q_SIGNALS:

    void sendImageLocation(const QPoint &point);

    void waypointAdded(Waypoint *waypoint);

    void targetPointAdded(Waypoint *waypoint);

public Q_SLOTS:

    void fitInView(bool rescale = true);

    void handleToolChange(ToolMode *currentTool, ToolMode *previous_tool);

    void setViewportCursor(const QCursor &cursor);

    void unsetViewportCursor();

    DragMode dragMode() const;

    void setDragMode(DragMode mode);

    Waypoint *addWaypoint(QVector3D point3d);

    Waypoint *addTargetpoint(QVector3D point3d);

    Waypoint *addMapPoint(QVector3D point3d, PointType pointType);

    bool removeMapPoint(Waypoint *waypoint);

    [[nodiscard]] bool hasPhoto() const;

    void setBasemap(const QPixmap &pixmap);

    void setBasemap(const QPixmap &pixmap, const QString &filePath);

    void addPhoto(LayerData *imageLayerData, QPointF waypointPosition, bool connectPrev = true);

    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void enterEvent(QEvent *event) override;

private:
    void storeMouseEvent(QMouseEvent *event);

};


#endif //REALTIME3D_SYSTEMVIEWER_H
