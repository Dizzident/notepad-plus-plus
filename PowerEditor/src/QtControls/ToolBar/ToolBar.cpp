// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ToolBar.h"
#include <QToolButton>
#include <QStyle>
#include <QMenu>

namespace QtControls {

bool ToolBar::init(QWidget* parent, toolBarStatusType type,
                   const ToolBarButtonUnit* buttonUnitArray, int arraySize)
{
    if (!parent) return false;

    _parent = parent;
    _widget = new QToolBar(parent);
    _state = type;

    QToolBar* toolbar = getToolBar();
    if (!toolbar) return false;

    toolbar->setMovable(true);
    toolbar->setFloatable(true);

    setupIcons(type);
    fillToolbar();

    // Add buttons from array
    if (buttonUnitArray && arraySize > 0) {
        for (int i = 0; i < arraySize; ++i) {
            const ToolBarButtonUnit& unit = buttonUnitArray[i];

            if (unit.idCommand == 0) {
                // Separator
                toolbar->addSeparator();
            } else {
                QAction* action = new QAction(unit.tooltip, toolbar);
                action->setData(unit.idCommand);

                if (unit.style & 0x02) { // TBSTYLE_CHECK
                    action->setCheckable(true);
                }

                toolbar->addAction(action);
                _actions.push_back(action);
            }
        }
        _nbButtons = arraySize;
    }

    return true;
}

void ToolBar::destroy()
{
    _actions.clear();
    if (_widget) {
        delete _widget;
        _widget = nullptr;
    }
}

void ToolBar::enable(int cmdID, bool doEnable) const
{
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return;

    for (QAction* action : toolbar->actions()) {
        if (action->data().toInt() == cmdID) {
            action->setEnabled(doEnable);
            return;
        }
    }
}

int ToolBar::getWidth() const
{
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return 0;
    return toolbar->width();
}

int ToolBar::getHeight() const
{
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return 0;
    return toolbar->height();
}

void ToolBar::reduce()
{
    _state = TB_SMALL;
    setupIcons(_state);
    resizeIconsDpi(_dpi);
}

void ToolBar::enlarge()
{
    _state = TB_LARGE;
    setupIcons(_state);
    resizeIconsDpi(_dpi);
}

void ToolBar::reduceToSet2()
{
    _state = TB_SMALL2;
    setupIcons(_state);
    resizeIconsDpi(_dpi);
}

void ToolBar::enlargeToSet2()
{
    _state = TB_LARGE2;
    setupIcons(_state);
    resizeIconsDpi(_dpi);
}

void ToolBar::setToBmpIcons()
{
    // Qt handles icons natively
    // This method is for Windows-specific bitmap handling
}

bool ToolBar::getCheckState(int ID2Check) const
{
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return false;

    for (QAction* action : toolbar->actions()) {
        if (action->data().toInt() == ID2Check) {
            return action->isChecked();
        }
    }
    return false;
}

void ToolBar::setCheck(int ID2Check, bool willBeChecked) const
{
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return;

    for (QAction* action : toolbar->actions()) {
        if (action->data().toInt() == ID2Check) {
            action->setChecked(willBeChecked);
            return;
        }
    }
}

void ToolBar::registerDynBtn(int messageId, void* iconHandles)
{
    (void)messageId;
    (void)iconHandles;
    // Dynamic button registration - implement as needed
}

void ToolBar::doPopup(QPoint chevPoint)
{
    (void)chevPoint;
    // Show popup menu for hidden buttons
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return;

    QMenu menu;
    for (QAction* action : toolbar->actions()) {
        if (!action->isVisible()) {
            menu.addAction(action);
        }
    }
    if (!menu.isEmpty()) {
        menu.exec(chevPoint);
    }
}

void ToolBar::addToRebar(ReBar* rebar)
{
    (void)rebar;
    // Qt handles toolbar docking natively
    // This method is for Windows-specific ReBar integration
}

void ToolBar::resizeIconsDpi(int dpi)
{
    _dpi = dpi;
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return;

    int iconSize = 16;
    if (_state == TB_LARGE || _state == TB_LARGE2) {
        iconSize = 32;
    }

    // Scale by DPI
    iconSize = iconSize * dpi / 96;
    toolbar->setIconSize(QSize(iconSize, iconSize));
}

void ToolBar::setupIcons(toolBarStatusType type)
{
    (void)type;
    // Icon setup - load appropriate icon set based on type
}

void ToolBar::fillToolbar()
{
    // Fill toolbar with default buttons
}

// ============================================================================
// ReBar implementation
// ============================================================================

void ReBar::init(QWidget* parent)
{
    (void)parent;
    // Qt handles toolbar docking natively
    // ReBar is a Windows-specific container
}

void ReBar::addToolBar(ToolBar* toolBar)
{
    (void)toolBar;
    // Qt handles this through QMainWindow
}

void ReBar::reNew(int id, int width, int height)
{
    (void)id;
    (void)width;
    (void)height;
}

void ReBar::resize(QWidget parent)
{
    (void)parent;
}

} // namespace QtControls
