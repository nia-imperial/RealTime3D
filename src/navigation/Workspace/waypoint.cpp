//
// Created by Nic on 18/03/2022.
//

#include "waypoint.h"
#include <QPainter>
#include <QGraphicsScene>
#include <utility>


Waypoint::Waypoint(QGraphicsItem *parent, QPersistentModelIndex  index) :
        QGraphicsItem(parent),
        w(30), h(30),
        brushColor(31, 176, 224),
        pen(Qt::black, 2),
        brush(brushColor),
        controlBrush(QBrush(QColor(214, 13, 36))),
        rect(-0.5 * w, -0.5 * h, 30, 30),
        control(new ControlPoint(this)),
        index(std::move(index)),
        signals(new WayPointSignals()),
        label(new QGraphicsTextItem(this)),
        m_z(0),
        pointType(PointType::Waypoint) {
    control->color = brushColor;
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);
    setFlag(ItemIsFocusable, true);
    setZValue(2);
    setLabel(this->index.row());
    alignLabel();
}

Waypoint::Waypoint(QPersistentModelIndex index) : Waypoint(nullptr, std::move(index)){

}

int Waypoint::x() {
    return scenePos().toPoint().x();
}

int Waypoint::y() {
    return scenePos().toPoint().y();
}

int Waypoint::z() const {
    return m_z;
}

void Waypoint::setZ(int z) {
    m_z = z;
    Q_EMIT signals->zUpdated();
}

QVector3D Waypoint::coord() {
    return {float(x()),
            float(y()),
            float(z())};
}

void Waypoint::alignLabel() const {
    auto n_pos = boundingRect().topLeft();
    auto x = n_pos.x();
    auto w1 = boundingRect().width();
    auto c = label->boundingRect().width();
    n_pos.setX(((w1 - c) / 2) + x);

    auto y = n_pos.y();
    auto h1 = boundingRect().height();
    c = label->boundingRect().height();
    n_pos.setY(((h1 - c)/2)+y);

    label->setPos(n_pos);
}

void Waypoint::setLabel(const QVariant& value) const {
    auto text = label->toPlainText();
    if (value.toString() == text)
        return;
    label->setPlainText(value.toString());
    label->adjustSize();
}

void Waypoint::setPointType(PointType type) {
    switch (type) {
        case PointType::TargetPoint:
            setFlag(ItemIsMovable, false);
            brushColor = Qt::green;
            brush.setColor(brushColor);
            update();
            break;
        case PointType::Waypoint:
            setFlag(ItemIsMovable, true);
            brushColor = QColor(31, 176, 224);
            brush.setColor(brushColor);
            update();
            break;
    }
    pointType = type;
}

PointType Waypoint::getPointType() const {
    return pointType;
}

QVector<QPair<ConnectionLine *, Waypoint*>> Waypoint::connectedPoints(){
    auto connectedLines = control->lines();
    QVector<QPair<ConnectionLine *, Waypoint *>>connections;
    for (auto line : connectedLines){
        for (auto controlPoint : line->controlPoints()){
            auto waypoint = dynamic_cast<Waypoint *>(controlPoint->parentItem());
            if (waypoint != this){
                connections.append(QPair(line, waypoint));
            }
        }
    }
    return connections;
}

QRectF Waypoint::boundingRect() const {
    auto adjust = pen.width() / 2;
    return rect.adjusted(-adjust, -adjust, adjust, adjust);
}

void Waypoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->save();
    if (isSelected())
        pen.setColor(Qt::yellow);
    else
        pen.setColor(Qt::black);
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawEllipse(rect);
    painter->restore();
    setLabel(index.row());
}

QVariant Waypoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange){
        Q_EMIT signals->itemMoved(index);
    }
    if (change == ItemSelectedChange)
            Q_EMIT signals->itemSelected(index);

    return QGraphicsItem::itemChange(change, value);
}

void Waypoint::connectTo(Waypoint *other) {
    auto otherControl = other->control;
    auto newConnection = new ConnectionLine(control,
                                            scenePos().toPoint(),
                                            brushColor);
    scene()->addItem(newConnection);
    if (control != otherControl){
        newConnection->setEnd(otherControl);
        control->addLine(newConnection);
        otherControl->addLine(newConnection);
    } else
        scene()->removeItem(newConnection);

}

WayPointSignals::WayPointSignals(QObject *parent) : QObject(parent) {

}