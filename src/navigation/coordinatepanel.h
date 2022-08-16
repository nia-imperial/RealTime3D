//
// Created by Nic on 19/03/2022.
//

#ifndef WAYPOINTER_COORDINATEPANEL_H
#define WAYPOINTER_COORDINATEPANEL_H


#include <QObject>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>

class Waypoint;

class CoordinatePanel : public QObject {
Q_OBJECT
public:
    explicit CoordinatePanel(QObject *parent,
                             QTableView *ui_waypointView,
                             QTableView *ui_targetView,
                             QPushButton *ui_addCoordsBtn,
                             QLineEdit *ui_coordLineEdit,
                             QTabWidget *ui_editorTabs);

    QStandardItemModel *waypointsTable;
    QStandardItemModel *targetsTable;

    QTableView *waypointView;
    QTableView *targetView;
    QPushButton *addCoordBtn;
    QLineEdit *coordLineEdit;
    QTabWidget *editorTabs;

Q_SIGNALS:
    void requestWaypointAdded(const QVector3D &pos, bool connectPrev);

public Q_SLOTS:


    static void selectWaypoint(const QItemSelection &selected, const QItemSelection &deselected);

    /// Creates row in table, and makes a new Waypoint graphics item
    Waypoint *generateWaypoint(const QVector3D &pos, bool isTargetPoint = false) const;

    Waypoint *retrieveWaypoint(int row, bool fromTargets = false) const;

    /// Creates row in table and makes new Waypoint graphics item,
    /// and adds them to the graphics scene.
    void addWaypoint(const QVector3D &pos, bool connectPrev = false);

    /// Creates row in table and makes new Waypoint graphics item,
    /// and adds them to the graphics scene. Waypoint is the values in LineEdit.
    void getWaypointLine();

private:
    /// Creates the row for waypoint data
    QList<QStandardItem *> createWaypointRow(const QVector3D &pos) const;

    /// Creates the row for target point data
    QList<QStandardItem *> createTargetPointRow(const QVector3D &pos) const;


};


#endif //WAYPOINTER_COORDINATEPANEL_H
