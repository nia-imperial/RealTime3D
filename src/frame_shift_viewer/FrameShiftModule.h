//
// Created by Nic on 15/02/2022.
//

#ifndef REALTIME3D_FRAMESHIFTMODULE_H
#define REALTIME3D_FRAMESHIFTMODULE_H

#include <QDialog>
#include "FrameShiftScene.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FrameShiftModule; }
QT_END_NAMESPACE

class FrameShiftModule : public QDialog {
Q_OBJECT

public:
    explicit FrameShiftModule(QWidget *parent = nullptr, int w=1920, int h=1080);

protected:
    void showEvent(QShowEvent *event) override;

public:

    ~FrameShiftModule() override;

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

public:
    FrameShiftScene *scene;

Q_SIGNALS:
    void viewportDisplayed();
    void xChanged(int x);
    void yChanged(int y);
    void imageWidthChanged(int i);
    void imageHeightChanged(int i);
    void zoomed();

public Q_SLOTS:
    int getShiftX();
    int getShiftY();
    void resize();

    void setImageHeight(int px);
    void setShiftX(int x);
    void setShiftY(int y);

    void setImageWidth(int px);

private:
    Ui::FrameShiftModule *ui;
    int numScheduledScalings;
    int scaleLevel;
    void updateOverlap();
    QPointF targetScenePos, targetViewportPos;

protected:
    void wheelEvent(QWheelEvent *event) override;

    void scalingTime(qreal x);

    void animFinished();

    bool eventFilter(QObject *object, QEvent *event) override;

};


#endif //REALTIME3D_FRAMESHIFTMODULE_H
