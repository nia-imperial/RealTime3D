//
// Created by Nic on 18/03/2022.
//

#ifndef WAYPOINTER_CONTROLPOINT_H
#define WAYPOINTER_CONTROLPOINT_H


#include <QGraphicsEllipseItem>

class ConnectionLine;

class ControlPoint : public QGraphicsEllipseItem {
public:
    explicit ControlPoint(QGraphicsItem *parent);

    QVector<ConnectionLine *> lineItems;

public:
    QColor color;

    [[nodiscard]] QVector<ConnectionLine *> lines() const;

    bool addLine(ConnectionLine *lineItem);

    bool removeLine(ConnectionLine *lineItem);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;


};


#endif //WAYPOINTER_CONTROLPOINT_H
