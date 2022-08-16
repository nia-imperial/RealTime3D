#include <QRect>
#include <QGraphicsScene>
#include <QGraphicsLinearLayout>
#include "RectFrame.h"
#include "../utility/messages.hpp"
#include <QGraphicsProxyWidget>
#include <QTimer>
#include <utility>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>

RectFrame::RectFrame(qreal x, qreal y, qreal w, qreal h,
                     QString text, const QColor &c,
                     QGraphicsItem *parent) :
        QGraphicsObject(parent),
        objectName(std::move(text)),
        m_rect(0, 0, w, h),
        m_compositionMode(QPainter::CompositionMode_SourceOver) {
    // rectangle outline
    m_pen = QPen();
    m_pen.setBrush(c);
    m_pen.setWidth(3);
    m_pen.setJoinStyle(Qt::MiterJoin);

    setAcceptDrops(true);

}

void RectFrame::adjustFontSize() {

    static auto dSize = [=, this](const QSize &inner, const QSize &outer) -> double {
        auto dy = inner.height() - outer.height();
        auto dx = inner.width() - outer.width();
        return std::max(dx, dy);
    };

    static auto f = [=, this](qreal fontSize, QGraphicsTextItem *label) -> qreal {
        auto font = label->font();
        font.setPointSizeF(fontSize);
        label->setFont(font);
        auto d = dSize(label->boundingRect().toRect().size(),
                       this->boundingRect().toRect().size());
        return d;
    };

    static auto df = [=, this](qreal fontSize, QGraphicsTextItem *label) -> qreal {
        if (fontSize < 1.0) fontSize = 1.0;
        return f(fontSize + 0.5, label) - f(fontSize - 0.5, label);
    };
}

QRectF RectFrame::getAsRectF() {
    return mapRectToScene(m_rect);
}

void RectFrame::setRectWidth(int i) {
    if (m_rect.width() == i) return;

    auto adjustedRect = m_rect;
    adjustedRect.setWidth(i);
    m_rect = adjustedRect;
}

void RectFrame::setRectHeight(int i) {
    if (m_rect.height() == i) return;

    auto adjustedRect = m_rect;
    adjustedRect.setHeight(i);
    m_rect = adjustedRect;
}

void RectFrame::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Left:
            setX(x() - 1);
            break;
        case Qt::Key_Right:
            setX(x() + 1);
            break;
        case Qt::Key_Up:
            setY(y() - 1);
            break;
        case Qt::Key_Down:
            setY(y() + 1);
            break;
    }
}

QRectF RectFrame::boundingRect() const {
    qreal penWidth = m_pen.widthF();
    return {-penWidth / 2, -penWidth / 2, m_rect.width() + penWidth, m_rect.height() + penWidth};
}


void RectFrame::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto tpen = QPen();
    tpen.setWidth(3);
    painter->setPen(tpen);
    painter->drawRect(boundingRect());
    painter->setPen(m_pen);
    painter->drawRect(m_rect);
    if (not image.isNull()) {
        painter->setCompositionMode(m_compositionMode);
        painter->drawImage(m_rect, image);
    }
}

void RectFrame::dropEvent(QGraphicsSceneDragDropEvent *event) {
    QGraphicsItem::dropEvent(event);
    if (event->mimeData()->hasUrls()) {
        auto url = event->mimeData()->urls().at(0);
        addImage(url.toLocalFile());
    }
}

void RectFrame::addImage(const QString &filePath) {
    if (filePath.isNull() or filePath.isEmpty()) {
        Messages::warning_msg(nullptr, "Received empty image file path in Frame Shift Viewer.");
        return;
    }
    image = QImage(filePath);
    m_rect = image.rect();
    m_pen.setWidth(1);
    update(boundingRect());
    Q_EMIT imageInserted();
    Q_EMIT sizeChanged();
}

void RectFrame::setCompositionMode(QPainter::CompositionMode mode) {
    m_compositionMode = mode;
    update(boundingRect());
}

int RectFrame::width() {
    return qIntCast(m_rect.width());
}

int RectFrame::height() {
    return qIntCast(m_rect.height());
}


