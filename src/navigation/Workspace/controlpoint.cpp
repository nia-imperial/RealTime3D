//
// Created by Nic on 18/03/2022.
//

#include "controlpoint.h"
#include <QPen>
#include "connectionline.h"
#include <QGraphicsScene>

ControlPoint::ControlPoint(QGraphicsItem *parent) :
        QGraphicsEllipseItem(-15, -15, 30, 30, parent), lineItems(), color() {
    setFlag(ItemSendsScenePositionChanges, true);
    setPen(Qt::NoPen);
    setZValue(2);
}

QVector<ConnectionLine *> ControlPoint::lines() const {
    return lineItems;
}

bool ControlPoint::addLine(ConnectionLine *lineItem) {
    for (auto existing: lineItems)
        if (existing->controlPoints() == lineItem->controlPoints())
            return false;
    lineItems.append(lineItem);
    return true;
}

bool ControlPoint::removeLine(ConnectionLine *lineItem) {
    for (auto existing: lineItems)
        if (existing->controlPoints() == lineItem->controlPoints()) {
            auto ret = lineItems.removeOne(existing);
            if (ret)
                scene()->removeItem(existing);
            return ret;
        }
    return false;
}

QVariant ControlPoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
    for (auto line: lineItems)
        line->updateLine(this);
    return QGraphicsItem::itemChange(change, value);
}