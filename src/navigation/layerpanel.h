//
// Created by Nic on 18/03/2022.
//

#ifndef WAYPOINTER_LAYERPANEL_H
#define WAYPOINTER_LAYERPANEL_H


#include <QWidget>
#include <QStandardItemModel>
#include <QTreeView>
#include <QGraphicsPixmapItem>
#include <QSpinBox>

class LayerData : public QObject {
Q_OBJECT
    float opacity;
    bool hidden;

public:
    explicit LayerData(QObject *parent = nullptr,
                       QStandardItem *standardItem = nullptr);

    QGraphicsPixmapItem *graphicsItem;
    QStandardItem *layerItem;
    QString description;
    QString imagePath;

    bool progSet;

    [[nodiscard]] QPixmap pixmap() const;

Q_SIGNALS:

    void toggled(bool checked);

    void opacityChanged();

    void opacityUpdated();


public Q_SLOTS:

    [[nodiscard]] float getOpacity() const;

    void updateOpacity(float value);

    void setOpacity();

    void setOpacity(const QVariant &value);

    void setImage(const QPixmap &image, bool center = true) const;

    void setImage(const QString &fileName, bool center = true);

    void handleDeletion() const;

    void handleToggle(Qt::CheckState checkState);

    /// Set the layer items check box to checked (true) or unchecked (false), controlling if item is displayed or not
    void setChecked(bool val) const;

    [[nodiscard]] bool isChecked() const;

};

class LayerPanel : public QObject {
Q_OBJECT
public:
    explicit LayerPanel(QObject *parent = nullptr,
                        QTreeView *ui_treeView = nullptr,
                        QSpinBox *ui_opacityBox = nullptr,
                        QFrame *ui_layerSettings = nullptr);

    QFrame *layerSettings;
    QSpinBox *opacityBox;
    QTreeView *layerView;
    QStandardItemModel *layerModel;
    QStandardItem *baseLayer;
    LayerData *currentBaseMap;
    QStandardItem *targetLayer;
    QStandardItem *matchingLayer;
    // extra: context menu

    LayerData *currentSelected();

    static LayerData *getLayerData(QStandardItem *item);

    QStandardItem *addLayer(QString name = "");

    QStandardItem *addItemToLayer(QStandardItem *parentLayer, const QString &name);

public Q_SLOTS:

    void handleChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) const;

    /// Changes UI elements that handle opacity to point to the current index
    void switchToItem(const QModelIndex &current, const QModelIndex &previous) const;

private:
    QStandardItem *createItem(const QString &name);

};


#endif //WAYPOINTER_LAYERPANEL_H
