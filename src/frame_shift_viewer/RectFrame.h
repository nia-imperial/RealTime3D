#ifndef REALTIME3D_RECTFRAME_H
#define REALTIME3D_RECTFRAME_H

#include <QGraphicsSvgItem>
#include <QGraphicsRectItem>
#include <QGraphicsObject>
#include <QKeyEvent>
#include <QPen>
#include <QPalette>
#include <QLabel>
#include <QDebug>
#include <QPainter>
#include <QTextCursor>


class RectFrame : public QGraphicsObject {
    Q_OBJECT
public:
//    QGraphicsTextItem *textItem;

    RectFrame(qreal x, qreal y, qreal w, qreal h,
              QString text,
              const QColor &c = Qt::black,
              QGraphicsItem *parent = nullptr);

    ~RectFrame() override = default;


    QRectF getAsRectF();

    void setRectWidth(int i);

    void setRectHeight(int i);

    void adjustFontSize();

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void setCompositionMode(QPainter::CompositionMode mode);

    int width();

    int height();

    void addImage(const QString &filePath);

Q_SIGNALS:
    void sizeChanged();
    void imageInserted();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void dropEvent(QGraphicsSceneDragDropEvent *event) override;


private:
    QPainter::CompositionMode m_compositionMode;
    QRectF m_rect;
    QImage image;
    QPen m_pen;
    QString objectName;
};


#endif //REALTIME3D_RECTFRAME_H