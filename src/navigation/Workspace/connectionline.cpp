//
// Created by Nic on 18/03/2022.
//

#include "connectionline.h"
#include <QPainter>
#include <utility>
#include "controlpoint.h"
#include "waypoint.h"

ConnectionLine::ConnectionLine(ControlPoint *startControl, const QPoint &p2, QColor lineColor) :
        QGraphicsLineItem(),
        start(startControl),
        end(nullptr),
        lineColor(std::move(lineColor)),
        outlineColor(Qt::black),
        selectedOutlineColor(Qt::yellow),
        outlineWidth(1),
        m_pen(){
    m_pen.setWidth(5);
    m_pen.setCapStyle(Qt::PenCapStyle::RoundCap);
    m_pen.setColor(this->lineColor);
    setPen(m_pen);

    m_line = QLineF(this->start->scenePos(), p2);
    setLine(m_line);

    setFlag(ItemIsSelectable, true);
    setZValue(1);
}

QVector<ControlPoint *> ConnectionLine::controlPoints() {
    return {start, end};
}

void ConnectionLine::setP2(QPointF p2) {
    m_line.setP2(p2);
    setLine(m_line);
}

void ConnectionLine::setStart(ControlPoint *startControl) {
    start = startControl;
    updateLine(start);
}

void ConnectionLine::setEnd(ControlPoint *endControl) {
    end = endControl;
    updateLine(end);
}

void ConnectionLine::updateLine(ControlPoint *source) {
    if (source == start)
        m_line.setP1(source->scenePos());
    else
        m_line.setP2(source->scenePos());
    setLine(m_line);
}

void ConnectionLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto outline = QPainterPath();
    outline.moveTo(m_line.p1());
    outline.lineTo(m_line.p2());

    auto outlinePen = QPen(outlineColor, 2 * outlineWidth + m_pen.widthF());
    outlinePen.setCapStyle(m_pen.capStyle());
    if (isSelected())
        outlinePen.setColor(selectedOutlineColor);

//    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->strokePath(outline, outlinePen);
    painter->setPen(m_pen);
    painter->drawLine(m_line);
//    painter->restore();
}