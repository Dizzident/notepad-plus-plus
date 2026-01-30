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
#include <QMainWindow>
#include <QDebug>

namespace QtControls {

// ============================================================================
// ToolBar implementation
// ============================================================================

ToolBar::ToolBar()
    : _state(TB_SMALL)
    , _nbButtons(0)
    , _nbDynButtons(0)
    , _nbTotalButtons(0)
    , _nbCurrentButtons(0)
    , _pRebar(nullptr)
    , _dpi(96)
{
}

ToolBar::~ToolBar()
{
    destroy();
}

void ToolBar::initTheme(void* toolIconsDocRoot)
{
    // TODO: Implement theme initialization from XML
    // For now, this is a placeholder matching the Windows interface
    (void)toolIconsDocRoot;
}

void ToolBar::initHideButtonsConf(void* toolButtonsDocRoot, const ToolBarButtonUnit* buttonUnitArray, int arraySize)
{
    // TODO: Implement button hiding configuration from XML
    // For now, initialize all buttons as visible
    (void)toolButtonsDocRoot;
    (void)buttonUnitArray;

    if (arraySize > 0) {
        _toolbarStdButtonsConfArray = std::make_unique<bool[]>(arraySize);
        for (int i = 0; i < arraySize; ++i) {
            _toolbarStdButtonsConfArray[i] = true;
        }
    }
}

bool ToolBar::init(QWidget* parent, toolBarStatusType type,
                   const ToolBarButtonUnit* buttonUnitArray, int arraySize)
{
    if (!parent) return false;

    _parent = parent;
    _state = type;
    _dpi = 96; // Default DPI

    // Create the toolbar widget
    _widget = new QToolBar(parent);
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return false;

    // Configure toolbar appearance
    toolbar->setMovable(true);
    toolbar->setFloatable(true);
    toolbar->setIconSize(QSize(16, 16));

    // Setup icon lists
    setupIcons(type);

    // Create button array
    _nbButtons = static_cast<size_t>(arraySize);
    _nbDynButtons = _vDynBtnReg.size();
    _nbTotalButtons = _nbButtons + (_nbDynButtons > 0 ? _nbDynButtons + 1 : 0);

    _pTBB.clear();
    if (buttonUnitArray && arraySize > 0) {
        for (int i = 0; i < arraySize; ++i) {
            auto unit = std::make_unique<ToolBarButtonUnit>();
            *unit = buttonUnitArray[i];
            _pTBB.push_back(std::move(unit));
        }
    }

    // Fill toolbar with buttons
    reset(true);

    return true;
}

void ToolBar::destroy()
{
    if (_pRebar) {
        _pRebar->removeBand(_rbBand.wID);
        _pRebar = nullptr;
    }

    _actions.clear();
    _cmdToAction.clear();
    _pTBB.clear();
    _toolbarStdButtonsConfArray.reset();

    if (_widget) {
        delete _widget;
        _widget = nullptr;
    }
}

void ToolBar::enable(int cmdID, bool doEnable) const
{
    auto it = _cmdToAction.find(cmdID);
    if (it != _cmdToAction.end() && it->second) {
        it->second->setEnabled(doEnable);
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
    reset(true);
    redraw();
}

void ToolBar::enlarge()
{
    _state = TB_LARGE;
    setupIcons(_state);
    reset(true);
    redraw();
}

void ToolBar::reduceToSet2()
{
    _state = TB_SMALL2;
    setupIcons(_state);
    reset(true);
    redraw();
}

void ToolBar::enlargeToSet2()
{
    _state = TB_LARGE2;
    setupIcons(_state);
    reset(true);
    redraw();
}

void ToolBar::setToBmpIcons()
{
    _state = TB_STANDARD;
    reset(true);
    redraw();
}

bool ToolBar::getCheckState(int ID2Check) const
{
    auto it = _cmdToAction.find(ID2Check);
    if (it != _cmdToAction.end() && it->second) {
        return it->second->isChecked();
    }
    return false;
}

void ToolBar::setCheck(int ID2Check, bool willBeChecked) const
{
    auto it = _cmdToAction.find(ID2Check);
    if (it != _cmdToAction.end() && it->second) {
        it->second->setChecked(willBeChecked);
    }
}

bool ToolBar::change2CustomIconsIfAny()
{
    if (_customIconVect.empty()) return false;

    // TODO: Implement custom icon changing
    return true;
}

bool ToolBar::changeIcons(size_t whichLst, size_t iconIndex, const wchar_t* iconLocation) const
{
    // TODO: Implement icon replacement
    (void)whichLst;
    (void)iconIndex;
    (void)iconLocation;
    return false;
}

void ToolBar::registerDynBtn(unsigned int message, void* iconHandles, void* absentIco)
{
    // Register dynamic button (for plugins)
    // Note: Only possible before init!
    if (_widget || message == 0) return;

    // TODO: Convert iconHandles to QIcon
    DynamicCmdIcoBmp dynBtn;
    dynBtn._message = static_cast<int>(message);
    (void)iconHandles;
    (void)absentIco;

    _vDynBtnReg.push_back(dynBtn);
    _nbDynButtons = _vDynBtnReg.size();
}

void ToolBar::registerDynBtnDM(unsigned int message, void* iconHandles)
{
    // Register dynamic button with dark mode support
    if (_widget || message == 0) return;

    DynamicCmdIcoBmp dynBtn;
    dynBtn._message = static_cast<int>(message);
    (void)iconHandles;

    _vDynBtnReg.push_back(dynBtn);
    _nbDynButtons = _vDynBtnReg.size();
}

void ToolBar::doPopup(QPoint chevPoint)
{
    // Show popup menu for overflow buttons
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return;

    QMenu menu;
    bool hasVisible = false;

    for (QAction* action : toolbar->actions()) {
        // In Qt, we can't easily detect "hidden" buttons due to overflow
        // So we show all actions in the popup as a fallback
        if (action && !action->isSeparator()) {
            menu.addAction(action);
            hasVisible = true;
        }
    }

    if (hasVisible) {
        menu.exec(chevPoint);
    }
}

void ToolBar::addToRebar(ReBar* rebar)
{
    if (!rebar || _pRebar) return;

    _pRebar = rebar;

    // Initialize band info
    _rbBand = ReBarBandInfo();
    _rbBand.wID = REBAR_BAR_TOOLBAR;
    _rbBand.hwndChild = _widget;
    _rbBand.cxMinChild = 0;
    _rbBand.cyMinChild = getHeight();
    _rbBand.cyMaxChild = getHeight();
    _rbBand.cxIdeal = getWidth();
    _rbBand.cx = getWidth();

    _pRebar->addBand(&_rbBand, true);
}

void ToolBar::resizeIconsDpi(int dpi)
{
    _dpi = dpi;

    int iconSize = 16;
    if (_state == TB_LARGE || _state == TB_LARGE2) {
        iconSize = 32;
    }

    // Scale by DPI
    iconSize = iconSize * dpi / 96;

    QToolBar* toolbar = getToolBar();
    if (toolbar) {
        toolbar->setIconSize(QSize(iconSize, iconSize));
    }
}

void ToolBar::reset(bool create)
{
    QToolBar* toolbar = getToolBar();
    if (!toolbar) return;

    if (create) {
        // Clear existing actions
        toolbar->clear();
        _actions.clear();
        _cmdToAction.clear();

        // Add standard buttons
        for (size_t i = 0; i < _nbButtons && i < _pTBB.size(); ++i) {
            const auto& unit = _pTBB[i];
            if (!unit) continue;

            if (unit->_cmdID == 0) {
                // Separator
                toolbar->addSeparator();
            } else {
                // Button
                QAction* action = new QAction(toolbar);
                action->setData(unit->_cmdID);

                // Set icon based on current state
                // TODO: Load actual icons

                toolbar->addAction(action);
                _actions.push_back(action);
                _cmdToAction[unit->_cmdID] = action;
            }
        }

        // Add separator and dynamic buttons if any
        if (_nbDynButtons > 0) {
            toolbar->addSeparator();

            for (const auto& dynBtn : _vDynBtnReg) {
                QAction* action = new QAction(toolbar);
                action->setData(dynBtn._message);
                action->setIcon(dynBtn._icon);
                action->setText(dynBtn._tooltip);

                toolbar->addAction(action);
                _actions.push_back(action);
                _cmdToAction[dynBtn._message] = action;
            }
        }

        _nbCurrentButtons = _nbTotalButtons;
    }

    // Update band info if attached to rebar
    if (_pRebar) {
        _rbBand.hwndChild = _widget;
        _rbBand.cxMinChild = 0;
        _rbBand.cyMinChild = getHeight();
        _rbBand.cyMaxChild = getHeight();
        _rbBand.cxIdeal = getWidth();
        _rbBand.cx = getWidth();

        _pRebar->reNew(REBAR_BAR_TOOLBAR, &_rbBand);
    }
}

void ToolBar::setState(toolBarStatusType state)
{
    _state = state;
    // TODO: Notify parent window of icon change
    // HWND hRoot = ::GetAncestor(_hSelf, GA_ROOTOWNER);
    // ::SendMessage(hRoot, NPPM_INTERNAL_TOOLBARICONSCHANGED, 0, 0);
}

void ToolBar::setDefaultImageList()
{
    updateButtonImages();
}

void ToolBar::setDisableImageList()
{
    // TODO: Set disabled icons
}

void ToolBar::setDefaultImageList2()
{
    updateButtonImages();
}

void ToolBar::setDisableImageList2()
{
    // TODO: Set disabled icons
}

void ToolBar::setDefaultImageListDM()
{
    updateButtonImages();
}

void ToolBar::setDisableImageListDM()
{
    // TODO: Set disabled icons for dark mode
}

void ToolBar::setDefaultImageListDM2()
{
    updateButtonImages();
}

void ToolBar::setDisableImageListDM2()
{
    // TODO: Set disabled icons for dark mode
}

void ToolBar::setupIcons(toolBarStatusType type)
{
    (void)type;
    // TODO: Load appropriate icon set based on type and DPI
}

void ToolBar::fillToolbar()
{
    // Default toolbar filling - called by reset
}

void ToolBar::updateButtonImages()
{
    // Update action icons based on current state
    // TODO: Implement icon switching based on _state
}

// ============================================================================
// ReBar implementation
// ============================================================================

ReBar::ReBar()
{
    _usedIDs.clear();
}

ReBar::~ReBar()
{
    destroy();
}

void ReBar::destroy()
{
    _bands.clear();
    _bandWidgets.clear();
    _usedIDs.clear();

    if (_widget) {
        delete _widget;
        _widget = nullptr;
    }
}

void ReBar::init(QWidget* parent)
{
    _parent = parent;

    // In Qt, ReBar functionality is handled by QMainWindow's toolbar areas
    // We create a container widget to hold the toolbars
    _widget = new QWidget(parent);
    _widget->setObjectName("ReBarContainer");
}

bool ReBar::addBand(ReBarBandInfo* rBand, bool useID)
{
    if (!rBand) return false;

    // Assign ID if needed
    if (useID) {
        if (isIDTaken(rBand->wID)) {
            return false;
        }
    } else {
        rBand->wID = getNewID();
    }

    // Store band info
    _bands[rBand->wID] = *rBand;

    // If band has a child widget, track it
    if (rBand->hwndChild) {
        _bandWidgets[rBand->wID] = rBand->hwndChild;
    }

    return true;
}

void ReBar::reNew(int id, ReBarBandInfo* rBand)
{
    if (!rBand) return;

    auto it = _bands.find(id);
    if (it != _bands.end()) {
        it->second = *rBand;

        if (rBand->hwndChild) {
            _bandWidgets[id] = rBand->hwndChild;
        }
    }
}

void ReBar::removeBand(int id)
{
    _bands.erase(id);
    _bandWidgets.erase(id);

    if (id >= REBAR_BAR_EXTERNAL) {
        releaseID(id);
    }
}

void ReBar::setIDVisible(int id, bool show)
{
    auto it = _bandWidgets.find(id);
    if (it != _bandWidgets.end() && it->second) {
        it->second->setVisible(show);
    }

    auto bandIt = _bands.find(id);
    if (bandIt != _bands.end()) {
        if (show) {
            bandIt->second.fStyle &= ~0x08; // Clear hidden flag (RBBS_HIDDEN = 0x08)
        } else {
            bandIt->second.fStyle |= 0x08;  // Set hidden flag
        }
    }
}

bool ReBar::getIDVisible(int id)
{
    auto bandIt = _bands.find(id);
    if (bandIt != _bands.end()) {
        return (bandIt->second.fStyle & 0x08) == 0; // Check RBBS_HIDDEN
    }
    return false;
}

void ReBar::setGrayBackground(int id)
{
    // TODO: Set gray background for band
    (void)id;
}

int ReBar::getNewID()
{
    int idToUse = REBAR_BAR_EXTERNAL;

    for (int usedID : _usedIDs) {
        if (usedID < idToUse) {
            continue;
        } else if (usedID == idToUse) {
            ++idToUse;
        } else {
            break; // Found gap
        }
    }

    _usedIDs.push_back(idToUse);
    return idToUse;
}

void ReBar::releaseID(int id)
{
    auto it = std::find(_usedIDs.begin(), _usedIDs.end(), id);
    if (it != _usedIDs.end()) {
        _usedIDs.erase(it);
    }
}

bool ReBar::isIDTaken(int id)
{
    return std::find(_usedIDs.begin(), _usedIDs.end(), id) != _usedIDs.end();
}

} // namespace QtControls
