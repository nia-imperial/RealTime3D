//
// Created by Nic on 15/02/2022.
//

#include "FrameShiftScene.h"
#include <cmath>
#include <QTextEdit>
#include <QPushButton>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsWidget>
#include <QGraphicsLayoutItem>

FrameShiftScene::FrameShiftScene(QObject *parent, double w, double h) :
        QGraphicsScene(parent), imageW(w), imageH(h) {
    refRect = new RectFrame(0, 0,
                            imageW, imageH,
                            "Reference",
                            Qt::darkRed);
    secRect = new RectFrame(0, 0,
                            imageW, imageH,
                            "Secondary"
    );

    secRect->setFlag(QGraphicsItem::ItemIsMovable, true);
    secRect->setFlag(QGraphicsItem::ItemIsFocusable, true);
    secRect->setFocus();

    addItem(refRect);
    addItem(secRect);

    auto initialX = (13. / 100) * imageW;
    auto initialY = (13. / 100) * imageH;

    secRect->setX(initialX);
    secRect->setY(-initialY);

    refRect->setToolTip("Reference");
    secRect->setToolTip("Secondary");

    connect(refRect, &RectFrame::sizeChanged,
            this, [this]() {
                auto overlap = calculateOverlap();
                Q_EMIT overlapChanged(overlap);
            });
    connect(secRect, &RectFrame::sizeChanged,
            this, [this]() {
                auto overlap = calculateOverlap();
                Q_EMIT overlapChanged(overlap);
            });

}

double FrameShiftScene::calculateOverlap() const {
    using std::min;
    using std::max;
    using std::floor;

    double aX1, aX2, aY1, aY2;
    auto rectA = refRect->getAsRectF();
    rectA.getCoords(&aX1, &aY1, &aX2, &aY2);
    auto areaA = rectA.height() * rectA.width();

    double bX1, bY1, bX2, bY2;
    auto rectB = secRect->getAsRectF();
    rectB.getCoords(&bX1, &bY1, &bX2, &bY2);
    auto areaB = rectB.height() * rectB.width();

    // area of intersect
    auto intersect_width = max(.0, min(aX2, bX2) - max(aX1, bX1));
    auto intersect_height = max(.0, min(aY2, bY2) - max(aY1, bY1));
    auto aoi = intersect_width * intersect_height;

    /// area of union
    auto aou = areaA + areaB - aoi;

    /// intersect over union
    auto overlap = aoi / aou;

    return overlap * 100;
}

void FrameShiftScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void FrameShiftScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void FrameShiftScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    auto overlap = calculateOverlap();
    Q_EMIT overlapChanged(overlap);
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}


void FrameShiftScene::keyReleaseEvent(QKeyEvent *event) {
    auto overlap = calculateOverlap();
    Q_EMIT overlapChanged(overlap);
    QGraphicsScene::keyPressEvent(event);
}


