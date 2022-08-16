//
// Created by Nic on 19/03/2022.
//

#include "coordinatepanel.h"
#include <QTableView>
#include <QPushButton>
#include <QHeaderView>
#include <QLineEdit>
#include <QVector3D>
#include <QApplication>
#include <QMessageBox>
#include "Workspace/waypoint.h"

CoordinatePanel::CoordinatePanel(QObject *parent,
                                 QTableView *ui_waypointView,
                                 QTableView *ui_targetView,
                                 QPushButton *ui_addCoordsBtn,
                                 QLineEdit *ui_coordLineEdit,
                                 QTabWidget *ui_editorTabs) :
        QObject(parent),
        waypointsTable(new QStandardItemModel(0, 3, this)),
        targetsTable(new QStandardItemModel(0, 3, this)),
        waypointView(ui_waypointView),
        targetView(ui_targetView),
        addCoordBtn(ui_addCoordsBtn),
        coordLineEdit(ui_coordLineEdit),
        editorTabs(ui_editorTabs) {

    waypointView->setModel(waypointsTable);
    waypointsTable->setHorizontalHeaderLabels({QLatin1String("X"),
                                               QLatin1String("Y"),
                                               QLatin1String("Z")});
    waypointView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    waypointView->setSelectionBehavior(QAbstractItemView::SelectRows);
    waypointView->setDragDropMode(QAbstractItemView::NoDragDrop);
    waypointView->setDragDropOverwriteMode(false);
    auto selectionModel = waypointView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &CoordinatePanel::selectWaypoint);
    connect(addCoordBtn, &QPushButton::clicked, this, qOverload<>(&CoordinatePanel::getWaypointLine));

    auto header = waypointView->horizontalHeader();
    for (int i = 0; i < header->count(); ++i) {
        header->setSectionResizeMode(i, header->Stretch);
    }

    targetView->setModel(targetsTable);
    targetsTable->setHorizontalHeaderLabels({QLatin1String("X"),
                                             QLatin1String("Y"),
                                             QLatin1String("Z")});
    targetView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    targetView->setSelectionBehavior(QAbstractItemView::SelectRows);
    targetView->setDragDropMode(QAbstractItemView::NoDragDrop);
    targetView->setDragDropOverwriteMode(false);
    selectionModel = targetView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &CoordinatePanel::selectWaypoint);
    header = targetView->horizontalHeader();
    for (int i = 0; i < header->count(); ++i) {
        header->setSectionResizeMode(i, header->Stretch);
    }
}

QList<QStandardItem *> CoordinatePanel::createTargetPointRow(const QVector3D &pos) const {
    editorTabs->setCurrentIndex(1);
    auto xItem = new QStandardItem(QString().setNum(pos.x()));
    auto yItem = new QStandardItem(QString().setNum(pos.y()));
    auto zItem = new QStandardItem(QString().setNum(pos.z()));

    QList<QStandardItem *>itemList{xItem, yItem, zItem};
    for (auto item : itemList){
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setFlags(item->flags() & ~(Qt::ItemIsDropEnabled));
    }
    targetsTable->appendRow({xItem, yItem, zItem});
    return itemList;
}

QList<QStandardItem *> CoordinatePanel::createWaypointRow(const QVector3D &pos) const {
    editorTabs->setCurrentIndex(0);
    auto xItem = new QStandardItem(QString().setNum(pos.x()));
    auto yItem = new QStandardItem(QString().setNum(pos.y()));
    auto zItem = new QStandardItem(QString().setNum(pos.z()));
    QList<QStandardItem *>itemList{xItem, yItem, zItem};
    for (auto item : itemList){
        item->setFlags(item->flags() & ~(Qt::ItemIsDropEnabled));
    }
    waypointsTable->appendRow(itemList);
    return itemList;
}

void CoordinatePanel::addWaypoint(const QVector3D &pos, bool connectPrev) {
//     connect to workspace receiveWaypointRequest
    Q_EMIT requestWaypointAdded(pos, connectPrev);
}

void CoordinatePanel::getWaypointLine() {
    auto text = coordLineEdit->text();
    if (text.isNull() or text.isEmpty()) return;
    auto parts = text.split(",");
    if (parts.count() == 2)
        addWaypoint({parts.at(0).toFloat(),
                     parts.at(1).toFloat(),
                     0});

    else if (parts.count() == 3)
        addWaypoint({parts.at(0).toFloat(),
                     parts.at(1).toFloat(),
                     parts.at(2).toFloat()});

}


Waypoint *CoordinatePanel::generateWaypoint(const QVector3D &pos, bool isTargetPoint) const {
    QList<QStandardItem *> rowItems;
    Waypoint *waypoint;

    if (isTargetPoint) {
        rowItems = createTargetPointRow(pos);
        waypoint = new Waypoint(QPersistentModelIndex(targetsTable->indexFromItem(rowItems.at(0))));
    } else {
        rowItems = createWaypointRow(pos);
        waypoint = new Waypoint(QPersistentModelIndex(waypointsTable->indexFromItem(rowItems.at(0))));
    }

    connect(waypoint->signals, &WayPointSignals::itemMoved, this, [=]() {
        rowItems[0]->setText(QString().setNum(waypoint->x()));
        rowItems[1]->setText(QString().setNum(waypoint->y()));
        rowItems[2]->setText(QString().setNum(waypoint->z()));
    });

    rowItems.at(0)->setData(QVariant::fromValue((void *) waypoint));
    return waypoint;
}

Waypoint *CoordinatePanel::retrieveWaypoint(int row, bool fromTargets) const {
    QStandardItem *leftItem;
    if (fromTargets)
        leftItem = targetsTable->item(row, 0);
    else
        leftItem = waypointsTable->item(row, 0);

    return static_cast<Waypoint *>(leftItem->data().value<void *>());

}


void CoordinatePanel::selectWaypoint(const QItemSelection &selected, const QItemSelection &deselected) {
    if (auto focusedTable = dynamic_cast<QTableView *>(QApplication::focusWidget())) {
        auto model = dynamic_cast<QStandardItemModel *>(focusedTable->model());
        for (auto index: selected.indexes()) {
            if (index.column() == 0) {
                auto item = model->itemFromIndex(index);
                auto wp = static_cast<Waypoint *>(item->data().value<void *>());
                wp->setSelected(true);
            }
        }
        for (auto index: deselected.indexes()) {
            if (index.column() == 0) {
                auto item = model->itemFromIndex(index);
                auto wp = static_cast<Waypoint *>(item->data().value<void *>());
                wp->setSelected(false);
            }
        }
    }
}
