//
// Created by Nic on 13/03/2022.
//

#include <QScrollBar>
#include <QProgressDialog>
#include "systemviewer.h"
#include "toolmode.h"
#include "waypoint.h"
#include "../layerpanel.h"
#include "../coordinatepanel.h"
#include "missionscene.h"
#include "../../utility/pyscriptcaller.h"
#include <QApplication>
#include <QDebug>
#include <QMessageBox>


bool altModded(int key) {
    return key == Qt::Key_Alt or key == Qt::AltModifier;
}

bool ctrlModded(int key) {
    return key == Qt::Key_Control or key == Qt::ControlModifier;
}

bool shiftModded(int key) {
    return key == Qt::Key_Shift or key == Qt::ShiftModifier;
}


SystemViewer::SystemViewer(QWidget *parent,
                           ToolModes *parent_toolModes,
                           LayerPanel *parent_layerPanel,
                           CoordinatePanel *parent_coordinatePanel) :
        QGraphicsView(parent),
        layerPanel(parent_layerPanel),
        coordinatePanel(parent_coordinatePanel),
        toolMode(parent_toolModes),
        zoom(0), factor(1.25),
        empty(true),
        hasStoredOriginalCursor(false),
        useLastMouseEvent(false),
        handScrolling(false),
        handScrollingMotions(0),
        m_dragMode(NoDrag),
        lastWaypointPlaced(nullptr),
        lastWaypointSelected(nullptr),
        lastMouseEvent(nullptr) {
    setObjectName("System Viewer");
    setMouseTracking(true);

    setBackgroundBrush(QBrush(QColor(30, 30, 30)));
    setFrameShape(QFrame::NoFrame);
    setRenderHint(QPainter::Antialiasing);

    connect(toolMode, &ToolModes::toolChanged,
            this, &SystemViewer::handleToolChange);

    viewport()->setObjectName("System Viewer Viewport");
}

void SystemViewer::fitInView(bool rescale) {
    auto rect = scene()->sceneRect();
    if (not rect.isNull()) {
        setSceneRect(rect);
        if (hasPhoto()) {
            auto unity = transform().mapRect(QRectF(0, 0, 1, 1));
            this->scale(1 / unity.width(), 1 / unity.height());
            auto viewRect = viewport()->rect();
            auto sceneRect = transform().mapRect(rect);
            factor = std::min(viewRect.width() / sceneRect.width(),
                              viewRect.height() / sceneRect.height());
            this->scale(factor, factor);
        }
        zoom = 0;
    }
}

void SystemViewer::keyPressEvent(QKeyEvent *event) {
    auto selectedItems = scene()->selectedItems();
    switch (event->key()) {
        case Qt::Key_Up:
            for (auto item: selectedItems) {
                item->moveBy(0, -1);
                item->ensureVisible(QRectF(), 1, 1);
            }
            break;
        case Qt::Key_Down:
            for (auto item: selectedItems) {
                item->moveBy(0, +1);
                item->ensureVisible(QRectF(), 1, 1);
            }
            break;
        case Qt::Key_Left:
            for (auto item: selectedItems) {
                item->moveBy(-1, 0);
                item->ensureVisible(QRectF(), 1, 1);
            }
            break;
        case Qt::Key_Right:
            for (auto item: selectedItems) {
                item->moveBy(+1, 0);
                item->ensureVisible(QRectF(), 1, 1);
            }
            break;
    }

    if (altModded(event->key()))
        if (toolMode->multiTool->isActive()) {
            toolMode->setMode(ToolType::Placement);
            event->accept();
        }

    if (ctrlModded(event->key()))
        if (toolMode->multiTool->isActive()) {
            toolMode->setMode(ToolType::Connection);
            event->accept();
        }

    if (shiftModded(event->key()))
        if (toolMode->multiTool->isActive()) {
            toolMode->setMode(ToolType::Selection);
            event->accept();
        }

    if (not selectedItems.count())
        QGraphicsView::keyPressEvent(event);
}

void SystemViewer::keyReleaseEvent(QKeyEvent *event) {
    if (altModded(event->key()) or ctrlModded(event->key()) or shiftModded(event->key()))
        if (toolMode->previous()->getType() == toolMode->multiTool->getType())
            toolMode->setMode(toolMode->previous()->getType());

    if ((event->key() == Qt::Key_Delete) or (event->key() == Qt::Key_Backspace)) {
        auto selected = scene()->selectedItems();
        for (auto item: selected) {
            auto wp = qgraphicsitem_cast<Waypoint *>(item);
            if (wp != nullptr)
                removeMapPoint(wp);
        }
        selected = scene()->selectedItems();
        for (auto item: selected) {
            auto conn = qgraphicsitem_cast<ConnectionLine *>(item);
            if (conn != nullptr) {
                for (auto cp: conn->controlPoints())
                    cp->lineItems.removeOne(conn);
                item->scene()->removeItem(item);
            }
        }
    }

    QGraphicsView::keyReleaseEvent(event);
}

void SystemViewer::mouseDoubleClickEvent(QMouseEvent *event) {
    if (auto item = itemAt(event->pos())) {
        auto cp = qgraphicsitem_cast<ControlPoint *>(item);
        auto wp = qgraphicsitem_cast<Waypoint *>(item);
        if (cp or wp) {
            auto lines = cp->lines();
            if (lines.isEmpty()) return;
            for (auto line: cp->lines())
                line->setSelected(true);
            return;
        }
        auto cn = qgraphicsitem_cast<ConnectionLine *>(item);
        if (cn) {
            auto controlPoints = cn->controlPoints();
            if (controlPoints.isEmpty()) return;
            for (auto ctrl: cn->controlPoints())
                ctrl->parentItem()->setSelected(true);
            return;
        }
    }
}

void SystemViewer::mousePressEvent(QMouseEvent *event) {
    storeMouseEvent(event);
    lastMouseEvent->setAccepted(false);

    if (event->button() == Qt::RightButton) {
        setDragMode(NoDrag);
        auto mousePressViewPoint = event->pos();
        auto mousePressScenePoint = mapToScene(mousePressViewPoint);
        auto mousePressScreenPoint = event->globalPos();
        lastMouseMoveScenePoint = mousePressScenePoint;
        lastMouseMoveScreenPoint = mousePressScreenPoint;
        event->accept();
        handScrolling = true;
        handScrollingMotions = 0;
        viewport()->setCursor(Qt::ClosedHandCursor);
    }
    if (event->modifiers() == Qt::AltModifier)
        if (toolMode->multiTool->isActive())
            toolMode->setMode(ToolType::Placement);

    if (event->modifiers() == Qt::ControlModifier)
        if (toolMode->multiTool->isActive())
            toolMode->setMode(ToolType::Connection);

    if (event->modifiers() == Qt::ShiftModifier)
        if (toolMode->multiTool->isActive())
            toolMode->setMode(ToolType::Selection);

    if (event->button() == Qt::LeftButton)
        if (toolMode->placeMode->isActive()) {
            auto m_scene = dynamic_cast<MissionScene *>(scene());
            auto controlPoint = m_scene->controlPointAt(mapToScene(event->pos()));
            if (controlPoint != nullptr) {
                auto waypoint = qgraphicsitem_cast<Waypoint *>(controlPoint->parentItem());
                if (waypoint)
                    lastWaypointPlaced->connectTo(waypoint);
            } else {
                auto baseMapItem = layerPanel->currentBaseMap;
                if (baseMapItem->pixmap().isNull()){
                    QMessageBox::warning(dynamic_cast<QWidget *>(parent()),
                                         "Warning", "Base map must be set.", QMessageBox::Ok);
                    return;
                }
                auto itemPos = baseMapItem->graphicsItem->mapFromScene(mapToScene(event->pos()));
                auto waypoint = addWaypoint(QVector3D(itemPos));
                event->accept();
            }
        }

    QGraphicsView::mousePressEvent(event);
}

void SystemViewer::mouseMoveEvent(QMouseEvent *event) {
    if (handScrolling) {
        auto hBar = horizontalScrollBar();
        auto vBar = verticalScrollBar();
        auto delta = event->pos() - lastMouseEvent->pos();
        auto dx = isRightToLeft() ? delta.x() : -delta.x();
        hBar->setValue(hBar->value() + dx);
        vBar->setValue(vBar->value() - delta.y());
        handScrollingMotions += 1;
        storeMouseEvent(event);
        lastMouseEvent->setAccepted(false);
    }
    auto item = itemAt(event->pos());

    if (event->modifiers() == Qt::AltModifier) {
        try {
            auto baseMapItem = layerPanel->currentBaseMap;
            auto itemPos = baseMapItem->graphicsItem->mapFromScene(mapToScene(event->pos()));
            Q_EMIT sendImageLocation(itemPos.toPoint());
        } catch (...) { /* */ }
    }
    QGraphicsView::mouseMoveEvent(event);
}

void SystemViewer::mouseReleaseEvent(QMouseEvent *event) {
    if (handScrolling and event->button() == Qt::RightButton) {
        handScrolling = false;
        event->accept();
        if (toolMode->selectionMode->isActive())
            setDragMode(RubberBandDrag);
    }
    storeMouseEvent(event);
    setViewportCursor(toolMode->cursor());
    QGraphicsView::mouseReleaseEvent(event);
}

void SystemViewer::wheelEvent(QWheelEvent *event) {
    if (hasPhoto()) {
        if (event->angleDelta().y() > 0) {
            factor = 1.25;
            zoom += 1;
        } else {
            factor = 0.8;
            zoom -= 1;
        }

        if (zoom > 0)
            this->scale(factor, factor);
        else if (zoom == 0)
            SystemViewer::fitInView();
        else
            zoom = 0;

    }
}

void SystemViewer::enterEvent(QEvent *event) {
    setFocus();
    setViewportCursor(toolMode->cursor());
}


void SystemViewer::unsetViewportCursor() {
    auto items = this->items(lastMouseEvent->pos());
    for (auto item: items)
        if (item->isEnabled() and item->hasCursor())
            return;

    if (hasStoredOriginalCursor) {
        hasStoredOriginalCursor = false;
        if (dragMode() == ScrollHandDrag)
            viewport()->setCursor(Qt::OpenHandCursor);
        else
            viewport()->setCursor(originalCursor);
    }
}

QGraphicsView::DragMode SystemViewer::dragMode() const {
    return m_dragMode;
}

void SystemViewer::setDragMode(QGraphicsView::DragMode mode) {
    if (m_dragMode == mode) {
        return QGraphicsView::setDragMode(mode);
    }
    if (mode == RubberBandDrag)
        QGraphicsView::setDragMode(mode);
    else
        QGraphicsView::setDragMode(NoDrag);

    if (m_dragMode == ScrollHandDrag)
        viewport()->unsetCursor();

    if (m_dragMode == ScrollHandDrag and mode == NoDrag and handScrolling)
        handScrolling = false;

    m_dragMode = mode;

    if (m_dragMode == ScrollHandDrag) {
        hasStoredOriginalCursor = false;
        viewport()->setCursor(Qt::OpenHandCursor);
    }

}

Waypoint *SystemViewer::addWaypoint(QVector3D point3d) {
    auto waypoint = addMapPoint(point3d, PointType::Waypoint);
    if (toolMode->placeMode->isActive()) {
        auto focusedItem = qgraphicsitem_cast<Waypoint *>(scene()->focusItem());
        if (focusedItem)
            focusedItem->connectTo(waypoint);
    }

    lastWaypointPlaced = waypoint;
    scene()->setFocusItem(waypoint, Qt::MouseFocusReason);
    return waypoint;
}

Waypoint *SystemViewer::addTargetpoint(QVector3D point3d) {
    auto waypoint = addMapPoint(point3d, PointType::TargetPoint);
    return waypoint;
}

Waypoint *SystemViewer::addMapPoint(QVector3D point3d, PointType pointType) {
    Waypoint *point;
    QString pointTypeStr;
    switch (pointType) {
        case PointType::Waypoint:
            point = coordinatePanel->generateWaypoint(point3d);
            pointTypeStr = QLatin1String("Waypoint");
            break;
        case PointType::TargetPoint:
            point = coordinatePanel->generateWaypoint(point3d, true);
            pointTypeStr = QLatin1String("TargetPoint");
            break;
    }
    Q_EMIT waypointAdded(point);

    auto baseMapItem = layerPanel->currentBaseMap;
    auto scenePos = baseMapItem->graphicsItem->mapToScene(point3d.toPoint());

//    point->setPos(scenePos.toPoint());
    point->setX(point3d.x());
    point->setY(point3d.y());
    point->setZ(qIntCast(point3d.z()));
    scene()->addItem(point);
    Q_EMIT point->signals->itemMoved(point->index);

    qInfo() << QString("%1: %2, %3, %4 added.")
            .arg(pointTypeStr)
            .arg(point->x())
            .arg(point->y())
            .arg(point->z());

    point->setPointType(pointType);
    return point;
}

bool SystemViewer::removeMapPoint(Waypoint *waypoint) {
    QStandardItemModel *pointList;
    switch (waypoint->getPointType()) {
        case PointType::Waypoint:
            pointList = coordinatePanel->waypointsTable;
            break;
        case PointType::TargetPoint:
            pointList = coordinatePanel->targetsTable;
            break;
        default:
            return false;
    }
    if (lastWaypointPlaced && waypoint == lastWaypointPlaced)
        lastWaypointPlaced = nullptr;

    auto connected = waypoint->connectedPoints();
    for (auto connection: connected) {
        auto line = connection.first;
        auto wp = connection.second;
        wp->control->removeLine(line);
    }

    pointList->removeRow(waypoint->index.row());
    waypoint->scene()->removeItem(waypoint);
    return true;
}

bool SystemViewer::hasPhoto() const {
    return not empty;
}

void SystemViewer::setBasemap(const QPixmap &pixmap, const QString &filePath) {
    auto baseMapData = layerPanel->currentBaseMap;
    baseMapData->imagePath = filePath;
    setBasemap(pixmap);
}

void SystemViewer::setBasemap(const QPixmap &pixmap) {
    auto baseMapData = layerPanel->currentBaseMap;
    zoom = 0;
    if (not pixmap.isNull()) {
        empty = false;
        baseMapData->setImage(pixmap, false);
        scene()->addItem(baseMapData->graphicsItem);
    } else {
        empty = true;
        setDragMode(NoDrag);
        baseMapData->setImage(QPixmap());
    }
    fitInView();
}

void SystemViewer::addPhoto(LayerData *imageLayerData, QPointF waypointPosition, bool connectPrev) {
    auto progress = QProgressDialog(QLatin1String("Processing Image"),
                                    nullptr,
                                    0, 4,
                                    this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    progress.setValue(0);
    progress.setValue(1);
    QApplication::processEvents();

    auto baseLayerData = layerPanel->currentBaseMap;

    auto pos = pycall::matchAerialToMap(imageLayerData->imagePath.toStdString(),
                                        baseLayerData->imagePath.toStdString(),
                                        waypointPosition.toPoint().x(), waypointPosition.toPoint().y());

    progress.setValue(2);
    auto newPos = QPoint(std::get<0>(pos), std::get<1>(pos));
    progress.setValue(3);

    auto m_scene = dynamic_cast<MissionScene *>(scene());
    m_scene->addItem(imageLayerData->graphicsItem);
    imageLayerData->graphicsItem->setPos(newPos);
    auto targetPoint = addTargetpoint(QVector3D(newPos));
    progress.setValue(4);
    if (connectPrev) {
        if (coordinatePanel->targetsTable->rowCount() > 1) {
            auto prevPoint = coordinatePanel->retrieveWaypoint(
                    coordinatePanel->targetsTable->rowCount() - 2, true);
            prevPoint->connectTo(targetPoint);
        }
    }
}

void SystemViewer::storeMouseEvent(QMouseEvent *event) {
    useLastMouseEvent = true;
    lastMouseEvent = new QMouseEvent(QEvent::MouseMove,
                                     event->localPos(),
                                     event->windowPos(),
                                     event->screenPos(),
                                     event->button(),
                                     event->buttons(),
                                     event->modifiers());
}

void SystemViewer::handleToolChange(ToolMode *currentTool, ToolMode *previous_tool) {
    setViewportCursor(currentTool->cursor());
    if (currentTool == toolMode->selectionMode)
        setDragMode(RubberBandDrag);
    else
        setDragMode(NoDrag);
}

void SystemViewer::setViewportCursor(const QCursor &cursor) {
    if (not hasStoredOriginalCursor) {
        hasStoredOriginalCursor = true;
        originalCursor = viewport()->cursor();
    }
    viewport()->setCursor(cursor);
}
