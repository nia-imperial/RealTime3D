//
// Created by Nic on 18/03/2022.
//

#ifndef WAYPOINTER_WAYPOINT_H
#define WAYPOINTER_WAYPOINT_H


#include <QObject>
#include <QGraphicsItem>
#include "connectionline.h"
#include "controlpoint.h"
#include <QPersistentModelIndex>
#include <QVector3D>


class WayPointSignals : public QObject {
Q_OBJECT
public:
    explicit WayPointSignals(QObject *parent = nullptr);

Q_SIGNALS:

    void itemMoved(const QModelIndex &rowIndex);

    void itemSelected(const QModelIndex &rowIndex);

    void zUpdated();
};

enum class PointType {
    Waypoint = 1,
    TargetPoint = 2
};

class Waypoint : public QGraphicsItem {
    float w;
    float h;
    PointType pointType;
    int m_z;
public:
    explicit Waypoint(QGraphicsItem *parent, QPersistentModelIndex index);

    explicit Waypoint(QPersistentModelIndex index);

    QColor brushColor;
    QPen pen;
    QBrush brush;
    QBrush controlBrush;
    QRectF rect;
    QPersistentModelIndex index;

    ControlPoint *control;
    WayPointSignals *signals;
    QGraphicsTextItem *label;


    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

public:

    int x();

    int y();

    [[nodiscard]] int z() const;

    void setZ(int z);

    QVector3D coord();

    void setLabel(const QVariant &value) const;

    void setPointType(PointType type);

    [[nodiscard]] PointType getPointType() const;

    void connectTo(Waypoint *other);

    QVector<QPair<ConnectionLine *, Waypoint *>> connectedPoints();

    [[nodiscard]] QRectF boundingRect() const override;


private:

    void alignLabel() const;

};


#endif //WAYPOINTER_WAYPOINT_H
