//
// Created by Nic on 13/03/2022.
//

#ifndef REALTIME3D_TOOLMODE_H
#define REALTIME3D_TOOLMODE_H


#include <QObject>
#include <QAction>
#include <QPointer>

enum class ToolType {
    MultiTool,
    Placement,
    Selection,
    Connection,
    Pan
};

class ToolMode : public QObject {
Q_OBJECT
    bool active;
public:
    explicit ToolMode(QObject *parent = nullptr, Qt::CursorShape cursor = Qt::ArrowCursor);

Q_SIGNALS:

    void activated();

    void deactivated();

    void statusChanged(bool);

public Q_SLOTS:

    void activate();

    void deactivate();

    void setActive(bool status = true);

    [[nodiscard]] bool isActive() const;

    void setToolAction(QAction *pAction);

    Qt::CursorShape cursor();

    ToolType getType();

protected:
    Qt::CursorShape m_cursor;
    ToolType toolType;
    QPointer<QAction> action;
    QVector<Qt::Key> key_binding;
};

class MultiTool : public ToolMode {
Q_OBJECT
public:
    explicit MultiTool(QObject *parent = nullptr);
};

class PlacementTool : public ToolMode {
Q_OBJECT
public:
    explicit PlacementTool(QObject *parent = nullptr);
};

class SelectionTool : public ToolMode {
Q_OBJECT
public:
    explicit SelectionTool(QObject *parent = nullptr);
};

class ConnectionTool : public ToolMode {
Q_OBJECT
public:
    explicit ConnectionTool(QObject *parent = nullptr);
};

class PanTool : public ToolMode {
Q_OBJECT
public:
    explicit PanTool(QObject *parent = nullptr);
};


class ToolModes : public QObject {
Q_OBJECT
public:
    explicit ToolModes(QObject *parent = nullptr);
    MultiTool *multiTool;
    PlacementTool *placeMode;
    SelectionTool *selectionMode;
    ConnectionTool *connectMode;
    PanTool *panTool;

Q_SIGNALS:
    void toolChanged(ToolMode *, ToolMode *);

public Q_SLOTS:

    QVector<ToolMode *> toolbox();

    Qt::CursorShape cursor();

    ToolMode *current();

    ToolMode *previous();

    void handleToolChange();

    void setMode(ToolType mode);

private:
    QPointer<ToolMode> m_current;
    QPointer<ToolMode> m_previous;
};


#endif //REALTIME3D_TOOLMODE_H
