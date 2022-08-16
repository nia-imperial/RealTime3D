//
// Created by Nic on 18/03/2022.
//

#include "layerpanel.h"
#include <QDebug>
#include <QGraphicsScene>

LayerPanel::LayerPanel(QObject *parent,
                       QTreeView *ui_treeView,
                       QSpinBox *ui_opacityBox,
                       QFrame *ui_layerSettings) :
        QObject(parent),
        layerSettings(ui_layerSettings),
        opacityBox(ui_opacityBox),
        layerView(ui_treeView),
        layerModel(new QStandardItemModel(this)){
    layerView->setModel(layerModel);

    layerView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    auto selector = layerView->selectionModel();
    layerModel->setHorizontalHeaderLabels({"Layers"});

    baseLayer = addLayer("Base Layer");
    currentBaseMap = LayerPanel::getLayerData(baseLayer);
    matchingLayer = addLayer("Matching Layer");
    targetLayer = addLayer("Target Points Layer");

    connect(selector, &QItemSelectionModel::currentChanged, this, &LayerPanel::switchToItem);
    connect(layerModel, &QStandardItemModel::dataChanged, this, &LayerPanel::handleChanged);

}

LayerData *LayerPanel::currentSelected(){
    auto index = layerView->currentIndex();
    auto item = layerModel->itemFromIndex(index);
    auto data = getLayerData(item);
    return data;
}

void LayerPanel::handleChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) const {
    auto topLeftItem = layerModel->itemFromIndex(topLeft);

    auto layerData = LayerPanel::getLayerData(topLeftItem);
    if (roles.contains(Qt::CheckStateRole) and not layerData->progSet)
        layerData->handleToggle(topLeftItem->checkState());

}

LayerData *LayerPanel::getLayerData(QStandardItem *item) {
    return static_cast<LayerData *>(item->data().value<void *>());
}

QStandardItem *LayerPanel::createItem(const QString& name) {
    // create QStandardItem
    auto item = new QStandardItem(name);

    // Create user data widget
    auto itemData = new LayerData(this, item);
    itemData->setObjectName(QString("%1 {layer data}").arg(name));
    item->setData(QVariant::fromValue((void *) itemData));
    item->setCheckable(true);
    item->setAutoTristate(true);
    item->setCheckState(Qt::Checked);

    return item;
}

QStandardItem *LayerPanel::addLayer(QString name) {
    if (name.isNull() or name.isEmpty())
        name = QString("layer_%1").arg(layerModel->rowCount());

    auto item = createItem(name);
    auto root = layerModel->invisibleRootItem();
    root->appendRow(item);
    return item;
}

QStandardItem *LayerPanel::addItemToLayer(QStandardItem *parentLayer, const QString& name) {
    auto item = createItem(name);
    parentLayer->appendRow(item);
    return item;
}

void LayerPanel::switchToItem(const QModelIndex &current, const QModelIndex &previous) const {

    if (previous.isValid()){
        auto prevLayer = layerModel->itemFromIndex(previous);
        auto prevLayerData = getLayerData(prevLayer);

        disconnect(opacityBox, qOverload<int>(&QSpinBox::valueChanged),
                prevLayerData, &LayerData::updateOpacity);

        disconnect(opacityBox, &QSpinBox::editingFinished,
                   prevLayerData, qOverload<>(&LayerData::setOpacity));

        disconnect(prevLayerData, &LayerData::toggled, layerSettings, &QFrame::setEnabled);
        layerSettings->setEnabled(false);
    }

    if (current.isValid()){
        auto currentLayer = layerModel->itemFromIndex(current);
        auto currentLayerData = getLayerData(currentLayer);
        connect(currentLayerData, &LayerData::toggled, layerSettings, &QFrame::setEnabled);

        connect(opacityBox, QOverload<int>::of(&QSpinBox::valueChanged),
                   currentLayerData, &LayerData::updateOpacity);

        connect(opacityBox, &QSpinBox::editingFinished,
                   currentLayerData, qOverload<>(&LayerData::setOpacity));

        opacityBox->setValue((int) currentLayerData->getOpacity());

        if (not currentLayerData->isChecked()){
            layerSettings->setEnabled(false);
            return;
        }
        layerSettings->setEnabled(true);
    }

}


LayerData::LayerData(QObject *parent, QStandardItem *item) :
        QObject(parent),
        layerItem(item),
        graphicsItem(new QGraphicsPixmapItem()),
        opacity(1.0f),
        hidden(false),
        progSet(false){

}

QPixmap LayerData::pixmap() const {
    return graphicsItem->pixmap();
}

void LayerData::updateOpacity(float value) {
    auto newValue = value / 100;
    opacity = newValue;
    Q_EMIT opacityUpdated();
}

void LayerData::setOpacity(const QVariant& value){
    // layer_base
    if (not value.isNull()){
        updateOpacity(value.toFloat());
        Q_EMIT opacityChanged();
    }

    // layer_item
    if (not layerItem->hasChildren() and layerItem->parent() != nullptr)
        if (graphicsItem->opacity() != opacity)
            graphicsItem->setOpacity(opacity);

    // layer_class
    // handle children
    if (layerItem->hasChildren()) {
        for (int i = 0; i < layerItem->rowCount(); ++i) {
            auto child = layerItem->child(i);
            auto childData = LayerPanel::getLayerData(child);
            childData->graphicsItem->setOpacity(this->getOpacity());
        }
    }
}

void LayerData::setOpacity() {
    setOpacity(QVariant());
}

void LayerData::setImage(const QPixmap& image, bool center) const {
    graphicsItem->setPixmap(image);
    if (center){
        auto centerPos = graphicsItem->boundingRect().center();
        graphicsItem->setOffset(-centerPos);
    }
}

void LayerData::setImage(const QString &fileName, bool center) {
    auto pixmap = QPixmap(fileName);
    setImage(pixmap, center);
    imagePath = fileName;
}

void LayerData::handleDeletion() const {
    if (not graphicsItem->pixmap().isNull())
        graphicsItem->scene()->removeItem(graphicsItem);

}

void LayerData::handleToggle(Qt::CheckState checkState) {
    progSet = true;
    // layer_base equiv
    Q_EMIT toggled(checkState);

    // layer_item equiv
    if (not layerItem->hasChildren() and layerItem->parent() != nullptr){
        if (isChecked())
            graphicsItem->setOpacity(opacity);
        else{
            graphicsItem->setOpacity(0);
            auto parentItemData = LayerPanel::getLayerData(layerItem->parent());
            if (parentItemData->isChecked()){
                Q_EMIT layerItem->model()->layoutChanged();
            }
        }
    }

    // layer_class equiv
    // handle children
    if (layerItem->hasChildren()){
        for (int i = 0; i < layerItem->rowCount(); ++i) {
            auto child = layerItem->child(i);
            auto childData = LayerPanel::getLayerData(child);
            Q_EMIT layerItem->model()->layoutChanged();
            if (isChecked()){
                childData->graphicsItem->setOpacity(childData->getOpacity());
                child->setEnabled(true);
            } else{
                child->setEnabled(false);
                childData->graphicsItem->setOpacity(0);
            }
        }
    }
    progSet = false;
}

float LayerData::getOpacity() const {
    return opacity * 100;
}

bool LayerData::isChecked() const {
    if (layerItem->checkState() == Qt::Checked)
        return true;
    else
        return false;
}

void LayerData::setChecked(bool val) const {
    if (val){
        layerItem->setCheckState(Qt::Checked);
    }
    else
        layerItem->setCheckState(Qt::Unchecked);
}





