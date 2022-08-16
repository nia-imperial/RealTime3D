//
// Created by Nic on 15/02/2022.
//

#ifndef REALTIME3D_FRAMESHIFTSCENE_H
#define REALTIME3D_FRAMESHIFTSCENE_H


#include <QGraphicsScene>

#include <QApplication>
#include "RectFrame.h"


class FrameShiftScene : public QGraphicsScene {
Q_OBJECT
public:
    explicit FrameShiftScene(QObject *parent = nullptr, double w = 1920, double h = 1080);

    /// secondary rectangle, representing secondary image.
    RectFrame *secRect;
    /// reference rectangle, representing reference image.
    RectFrame *refRect;

    [[nodiscard]] double calculateOverlap() const;

Q_SIGNALS:

    void overlapChanged(double newValue);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

    void keyReleaseEvent(QKeyEvent *event) override;

private:
    qreal imageW, imageH;
};



#endif //REALTIME3D_FRAMESHIFTSCENE_H
