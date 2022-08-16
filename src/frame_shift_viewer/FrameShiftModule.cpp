//
// Created by Nic on 15/02/2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_frameShiftView.h" resolved

#include "FrameShiftModule.h"
#include "ui_FrameShiftModule.h"
#include "FrameShiftScene.h"
#include "OpenGlViewport.h"
#include "../settings/SettingsForm.h"
#include "../settings/general_settings/generalsettings.h"
#include <QDebug>
#include <QTimeLine>
#include <QSurfaceFormat>
#include <QTimer>
#include <QtWidgets>


FrameShiftModule::FrameShiftModule(QWidget *parent, int w, int h) :
        QDialog(parent),
        ui(new Ui::FrameShiftModule),
        scene(new FrameShiftScene(this, w, h)),
        numScheduledScalings(0),
        scaleLevel(0) {
    ui->setupUi(this);
    ui->pushButton->setHidden(true);
    setMouseTracking(true);

    if (GeneralSettings::getOpenGLValue()) {
        auto *gl = new OpenGLViewport();
        ui->view->setViewport(gl);
        qInfo() << "Using OpenGL";
    }

    ui->view->viewport()->installEventFilter(this);
    ui->view->setTransformationAnchor(QGraphicsView::NoAnchor);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);


    ui->view->setRenderHints(
            QPainter::Antialiasing |
            QPainter::SmoothPixmapTransform |
            QPainter::TextAntialiasing
    );

    ui->view->setScene(scene);

    setImageWidth(w);
    setImageHeight(h);

    connect(ui->imageWidth, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int i) {
                scene->secRect->setRectWidth(i);
                scene->refRect->setRectWidth(i);
                updateOverlap();
                Q_EMIT imageWidthChanged(i);
                QTimer::singleShot(0, this, [this]() { scene->secRect->adjustFontSize(); });
                QTimer::singleShot(0, this, [this]() { scene->refRect->adjustFontSize(); });
            });

    connect(ui->imageHeight, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int i) {
                scene->secRect->setRectHeight(i);
                scene->refRect->setRectHeight(i);
                updateOverlap();
                Q_EMIT imageHeightChanged(i);
                QTimer::singleShot(0, this, [this]() { scene->secRect->adjustFontSize(); });
                QTimer::singleShot(0, this, [this]() { scene->refRect->adjustFontSize(); });
            });

    // set initial field value of X
    ui->secXSpin->setValue(int(scene->secRect->x()));
    // connect spin for sec. X to change the sec. rectangle
    connect(ui->secXSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int i) {
                scene->secRect->setX(i);
                updateOverlap();
                Q_EMIT xChanged(i);
            });
    // connect changes to sec. rectangle to update sec. X spin
    connect(scene, &FrameShiftScene::overlapChanged,
            this, [this]() {
                ui->secXSpin->setValue(int(scene->secRect->x()));
            });

    // set initial field value of Y
    ui->secYSpin->setValue(int(scene->secRect->y()));
    // connect spin for sec. Y to change the sec. rectangle
    connect(ui->secYSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int i) {
                scene->secRect->setY(i);
                updateOverlap();
                Q_EMIT yChanged(i);
            });
    // connect changes to sec. rectangle to update sec. Y spin
    connect(scene, &FrameShiftScene::overlapChanged,
            this, [this]() {
                ui->secYSpin->setValue(int(scene->secRect->y()));
            });

    // set initial value of overlap spin field value
    ui->overlapSpin->setValue(scene->calculateOverlap());
    // connect changes to the overlap to update the spin field value
    connect(scene, &FrameShiftScene::overlapChanged,
            ui->overlapSpin, &QDoubleSpinBox::setValue);

    connect(this, &FrameShiftModule::viewportDisplayed,
            this, &FrameShiftModule::resize);


    connect(scene->refRect, &RectFrame::sizeChanged,
            this, [this]() {
                ui->imageWidth->setValue(int(scene->refRect->width()));
                ui->imageHeight->setValue(int(scene->refRect->height()));
            });

    connect(scene->secRect, &RectFrame::sizeChanged,
            this, [this]() {
                ui->imageWidth->setValue(int(scene->secRect->width()));
                ui->imageHeight->setValue(int(scene->secRect->height()));
            });

    // default secRect to Soft Light
    scene->secRect->setCompositionMode(QPainter::CompositionMode_SoftLight);
    connect(scene->secRect, &RectFrame::imageInserted,
            this, [this]() { ui->pushButton->setHidden(false); });
    connect(ui->pushButton, &QPushButton::released,
            this, [this]() {
                if (ui->pushButton->isChecked()) {
                    ui->pushButton->setText(QLatin1String("Disable Soft Light"));
                    scene->secRect->setCompositionMode(QPainter::CompositionMode_SoftLight);
                } else {
                    ui->pushButton->setText(QLatin1String("Enable Soft Light"));
                    scene->secRect->setCompositionMode(QPainter::CompositionMode_SourceOver);
                }
            });

}

bool FrameShiftModule::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::Wheel) {
        return true;
    }
    return false;
}


FrameShiftModule::~FrameShiftModule() {
    delete ui;
}

void FrameShiftModule::updateOverlap() {
    ui->overlapSpin->setValue(scene->calculateOverlap());
}

int FrameShiftModule::getShiftX() {
    return ui->secXSpin->value();
}

int FrameShiftModule::getShiftY() {
    return ui->secYSpin->value();
}

void FrameShiftModule::resize() {
    ui->view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void FrameShiftModule::wheelEvent(QWheelEvent *event) {
    if (ui->view->underMouse()) {
        event->accept();
        auto numDegrees = event->angleDelta().y() / 8;
        auto numSteps = numDegrees / 15;

        scaleLevel += numSteps;

        numScheduledScalings += numSteps;
        if (numScheduledScalings * numSteps < 0)
            numScheduledScalings = numSteps;
        // if user moved the wheel in another direction, we reset previously scheduled scalings

        auto *anim = new QTimeLine(350, this);
        anim->setUpdateInterval(20);
        targetScenePos = ui->view->mapToScene(event->position().toPoint());
        connect(anim, &QTimeLine::valueChanged, this, &FrameShiftModule::scalingTime);
        connect(anim, &QTimeLine::finished, this, &FrameShiftModule::animFinished);
        anim->start();
    }
}


void FrameShiftModule::scalingTime(qreal x) {

    qreal factor = 1.0 + qreal(numScheduledScalings) / 300.0;

//
    auto oldAnchor = ui->view->transformationAnchor();
    ui->view->setTransformationAnchor(QGraphicsView::NoAnchor);

    auto matrix = ui->view->transform();
    matrix.translate(targetScenePos.x(), targetScenePos.y());
    matrix.scale(factor, factor);
    matrix.translate(-targetScenePos.x(), -targetScenePos.y());
    ui->view->setTransform(matrix);

    ui->view->setTransformationAnchor(oldAnchor);

//    ui->view->scale(factor, factor);

}

void FrameShiftModule::animFinished() {
    if (numScheduledScalings > 0)
        numScheduledScalings--;
    else
        numScheduledScalings++;
    sender()->deleteLater();
}

void FrameShiftModule::showEvent(QShowEvent *event) {
    Q_EMIT viewportDisplayed();
    QDialog::showEvent(event);
}

void FrameShiftModule::setImageWidth(int px) {
    ui->imageWidth->setValue(px);

}

void FrameShiftModule::setImageHeight(int px) {
    ui->imageHeight->setValue(px);
}


void FrameShiftModule::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    if (ui->view->underMouse()) {
        auto delta = targetViewportPos - event->pos();
        if (qAbs(delta.x()) > 5 or qAbs(delta.y()) > 5) {
            targetViewportPos = event->pos();
            targetScenePos = ui->view->mapToScene(event->pos());
        }
    }
}

void FrameShiftModule::setShiftX(int x) {
    ui->secXSpin->setValue(x);
}

void FrameShiftModule::setShiftY(int y) {
    ui->secYSpin->setValue(y);
}




