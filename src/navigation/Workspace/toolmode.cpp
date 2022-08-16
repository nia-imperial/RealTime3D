//
// Created by Nic on 13/03/2022.
//

#include "toolmode.h"

ToolMode::ToolMode(QObject *parent, Qt::CursorShape cursor) :
        QObject(parent), m_cursor(cursor),
        toolType(ToolType::MultiTool), active(false) {

}


void ToolMode::activate() {
    if (active)
        return;
    active = true;
    if (not action.isNull())
        action->setChecked(true);
    Q_EMIT activated();
}

void ToolMode::deactivate() {
    if (not active)
        return;
    active = false;
    if (not action.isNull())
        action->setChecked(false);
    Q_EMIT deactivated();
}

void ToolMode::setActive(bool status) {
    if (status == active)
        return;
    activate();
    Q_EMIT statusChanged(status);
}

bool ToolMode::isActive() const {
    return active;
}

void ToolMode::setToolAction(QAction *pAction) {
    action = pAction;
    connect(action, &QAction::triggered,
            this, &ToolMode::setActive);
}

Qt::CursorShape ToolMode::cursor() {
    return m_cursor;
}

ToolType ToolMode::getType() {
    return toolType;
}

MultiTool::MultiTool(QObject *parent) : ToolMode(parent) {
    toolType = ToolType::MultiTool;
}

PlacementTool::PlacementTool(QObject *parent) : ToolMode(parent, Qt::CrossCursor) {
    toolType = ToolType::Placement;
}

SelectionTool::SelectionTool(QObject *parent) : ToolMode(parent, Qt::ArrowCursor) {
    toolType = ToolType::Selection;
}


ConnectionTool::ConnectionTool(QObject *parent) : ToolMode(parent, Qt::PointingHandCursor) {
    toolType = ToolType::Connection;
}

PanTool::PanTool(QObject *parent) : ToolMode(parent, Qt::PointingHandCursor) {
    toolType = ToolType::Pan;
}


ToolModes::ToolModes(QObject *parent) :
        QObject(parent),
        multiTool(new MultiTool(this)),
        placeMode(new PlacementTool(this)),
        selectionMode(new SelectionTool(this)),
        connectMode(new ConnectionTool(this)),
        panTool(new PanTool(this)),
        m_current(multiTool), m_previous(multiTool){
    auto tools = toolbox();
    for (auto tool : tools){
        connect(tool, &ToolMode::activated,
                this, &ToolModes::handleToolChange);
        tool->setParent(parent);
        for (auto otherTool : tools){
            if (tool != otherTool)
                connect(tool, &ToolMode::activated,
                        otherTool, &ToolMode::deactivate);
        }
    }
}

Qt::CursorShape ToolModes::cursor() {
    return m_current->cursor();
}

ToolMode *ToolModes::current() {
    return m_current;
}

ToolMode *ToolModes::previous() {
    return m_previous;
}

void ToolModes::handleToolChange() {
    auto previousTool = m_current;
    auto currentTool = dynamic_cast<ToolMode *>(sender());
    m_current = currentTool;
    if (previousTool != m_previous)
        m_previous = previousTool;
    Q_EMIT toolChanged(m_current, m_previous);
}

void ToolModes::setMode(ToolType mode) {
    if (m_current->getType() == mode)
        return;

    switch (mode) {
        case ToolType::MultiTool:
            multiTool->activate();
            break;
        case ToolType::Placement:
            placeMode->activate();
            break;
        case ToolType::Selection:
            selectionMode->activate();
            break;
        case ToolType::Connection:
            connectMode->activate();
            break;
        case ToolType::Pan:
            panTool->activate();
            break;
    }

}

QVector<ToolMode *> ToolModes::toolbox() {
    return {multiTool,
            placeMode,
            selectionMode,
            connectMode,
            panTool};
}

