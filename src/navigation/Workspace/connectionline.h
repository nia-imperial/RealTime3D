//
// Created by Nic on 18/03/2022.
//

#ifndef WAYPOINTER_CONNECTIONLINE_H
#define WAYPOINTER_CONNECTIONLINE_H


#include <QGraphicsLineItem>
#include <QPointer>
#include <QPen>


class ControlPoint;

class ConnectionLine : public QGraphicsLineItem {
public:
    ConnectionLine(ControlPoint *startControl,
                   const QPoint &p2,
                   QColor lineColor);

    ControlPoint *start;
    ControlPoint *end;

    QColor lineColor;
    QColor outlineColor;
    QColor selectedOutlineColor;
    float outlineWidth;

public:

    QVector<ControlPoint *> controlPoints();

    void setP2(QPointF p2);

    void setStart(ControlPoint *startControl);

    void setEnd(ControlPoint *endControl);

    void updateLine(ControlPoint *source = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QPen m_pen;
    QLineF m_line;
};


#endif //WAYPOINTER_CONNECTIONLINE_H
