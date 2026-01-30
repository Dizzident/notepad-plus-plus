// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "DockingManager.h"
#include <QDebug>

namespace QtControls {

DockingManager::DockingManager() = default;

DockingManager::~DockingManager()
{
    destroy();
}

void DockingManager::init(QMainWindow* mainWindow)
{
    _mainWindow = mainWindow;
    if (_mainWindow) {
        _mainWindow->setDockNestingEnabled(true);
        _mainWindow->setDockOptions(QMainWindow::AnimatedDocks |
                                     QMainWindow::AllowNestedDocks |
                                     QMainWindow::AllowTabbedDocks);
    }
    _isInitialized = true;
}

void DockingManager::init(void* hInst, void* hWnd, Window** ppWin)
{
    (void)hInst;
    (void)hWnd;
    _ppWindow = ppWin;
    _ppMainWindow = ppWin;
    _isInitialized = true;
}

void DockingManager::destroy()
{
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        PanelInfo* info = it.value().get();
        if (info && info->dockWidget) {
            if (_mainWindow) {
                _mainWindow->removeDockWidget(info->dockWidget);
            }
            delete info->dockWidget;
        }
    }
    _panels.clear();
    _mainWindow = nullptr;
}

void DockingManager::reSizeTo(QRect& rc)
{
    _rect = rc;
    _rcWork = rc;
    // In Qt, dock widgets are managed by QMainWindow, so we don't need to resize them manually
}

void DockingManager::addPanel(const QString& name, QWidget* widget, DockArea area,
                       const QString& title)
{
    if (!_mainWindow || !widget || name.isEmpty()) {
        return;
    }

    if (_panels.contains(name)) {
        removePanel(name);
    }

    auto info = std::make_shared<PanelInfo>();
    info->name = name;
    info->widget = widget;
    info->area = area;
    info->title = title.isEmpty() ? name : title;
    info->id = _nextId++;
    info->visible = true;

    QDockWidget* dockWidget = new QDockWidget(info->title, _mainWindow);
    dockWidget->setObjectName(name);
    dockWidget->setWidget(widget);

    setupDockWidget(dockWidget, area);

    info->dockWidget = dockWidget;

    if (area == DockArea::Floating) {
        dockWidget->setFloating(true);
        dockWidget->show();
    } else {
        Qt::DockWidgetArea qtArea = dockAreaToQt(area);
        _mainWindow->addDockWidget(qtArea, dockWidget);
    }

    _panels[name] = std::move(info);
}

void DockingManager::removePanel(const QString& name)
{
    if (!_panels.contains(name)) {
        return;
    }

    PanelInfo* info = _panels[name].get();
    if (info && info->dockWidget) {
        if (_mainWindow) {
            _mainWindow->removeDockWidget(info->dockWidget);
        }
        delete info->dockWidget;
    }

    _panels.remove(name);
}

void DockingManager::showPanel(const QString& name)
{
    PanelInfo* info = getPanelInfo(name);
    if (!info || !info->dockWidget) {
        return;
    }

    info->dockWidget->show();
    info->visible = true;

    if (info->dockWidget->isFloating()) {
        info->dockWidget->raise();
        info->dockWidget->activateWindow();
    }
}

void DockingManager::hidePanel(const QString& name)
{
    PanelInfo* info = getPanelInfo(name);
    if (!info || !info->dockWidget) {
        return;
    }

    info->dockWidget->hide();
    info->visible = false;
}

void DockingManager::togglePanel(const QString& name)
{
    if (isPanelVisible(name)) {
        hidePanel(name);
    } else {
        showPanel(name);
    }
}

bool DockingManager::isPanelVisible(const QString& name) const
{
    PanelInfo* info = getPanelInfo(name);
    if (!info || !info->dockWidget) {
        return false;
    }
    return info->dockWidget->isVisible();
}

bool DockingManager::hasPanel(const QString& name) const
{
    return _panels.contains(name);
}

void DockingManager::setPanelArea(const QString& name, DockArea area)
{
    PanelInfo* info = getPanelInfo(name);
    if (!info || !info->dockWidget || !_mainWindow) {
        return;
    }

    if (area == DockArea::Floating) {
        info->dockWidget->setFloating(true);
    } else {
        if (info->dockWidget->isFloating()) {
            info->dockWidget->setFloating(false);
        }
        Qt::DockWidgetArea qtArea = dockAreaToQt(area);
        _mainWindow->addDockWidget(qtArea, info->dockWidget);
    }

    info->area = area;
}

DockingManager::DockArea DockingManager::getPanelArea(const QString& name) const
{
    PanelInfo* info = getPanelInfo(name);
    if (!info) {
        return DockArea::Floating;
    }

    if (info->dockWidget && info->dockWidget->isFloating()) {
        return DockArea::Floating;
    }

    return info->area;
}

void DockingManager::setPanelTitle(const QString& name, const QString& title)
{
    PanelInfo* info = getPanelInfo(name);
    if (!info || !info->dockWidget) {
        return;
    }

    info->title = title;
    info->dockWidget->setWindowTitle(title);
}

QString DockingManager::getPanelTitle(const QString& name) const
{
    PanelInfo* info = getPanelInfo(name);
    if (!info) {
        return QString();
    }
    return info->title;
}

QWidget* DockingManager::getPanelWidget(const QString& name) const
{
    PanelInfo* info = getPanelInfo(name);
    if (!info) {
        return nullptr;
    }
    return info->widget;
}

QDockWidget* DockingManager::getDockWidget(const QString& name) const
{
    PanelInfo* info = getPanelInfo(name);
    if (!info) {
        return nullptr;
    }
    return info->dockWidget;
}

void DockingManager::setTabbedDocking(const QString& name1, const QString& name2)
{
    if (!_mainWindow) {
        return;
    }

    QDockWidget* dock1 = getDockWidget(name1);
    QDockWidget* dock2 = getDockWidget(name2);

    if (!dock1 || !dock2) {
        return;
    }

    _mainWindow->tabifyDockWidget(dock1, dock2);
}

QByteArray DockingManager::saveLayout() const
{
    if (!_mainWindow) {
        return QByteArray();
    }

    return _mainWindow->saveState();
}

void DockingManager::restoreLayout(const QByteArray& layout)
{
    if (!_mainWindow || layout.isEmpty()) {
        return;
    }

    _mainWindow->restoreState(layout);

    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        PanelInfo* info = it.value().get();
        if (info && info->dockWidget) {
            info->visible = info->dockWidget->isVisible();
        }
    }
}

void DockingManager::showAllPanels()
{
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        showPanel(it.key());
    }
}

void DockingManager::hideAllPanels()
{
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        hidePanel(it.key());
    }
}

QStringList DockingManager::getPanelNames() const
{
    return _panels.keys();
}

QStringList DockingManager::getVisiblePanels() const
{
    QStringList visible;
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        if (isPanelVisible(it.key())) {
            visible.append(it.key());
        }
    }
    return visible;
}

void DockingManager::setPanelFeatures(const QString& name, bool closable, bool movable, bool floatable)
{
    QDockWidget* dock = getDockWidget(name);
    if (!dock) {
        return;
    }

    QDockWidget::DockWidgetFeatures features = QDockWidget::NoDockWidgetFeatures;

    if (closable) {
        features |= QDockWidget::DockWidgetClosable;
    }
    if (movable) {
        features |= QDockWidget::DockWidgetMovable;
    }
    if (floatable) {
        features |= QDockWidget::DockWidgetFloatable;
    }

    dock->setFeatures(features);
}

void DockingManager::raisePanel(const QString& name)
{
    QDockWidget* dock = getDockWidget(name);
    if (!dock) {
        return;
    }

    dock->raise();

    if (dock->isFloating()) {
        dock->activateWindow();
    }
}

int DockingManager::getPanelCount() const
{
    return _panels.size();
}

// Windows-compatible methods
int DockingManager::getDockedContSize(int iCont)
{
    // Map container index to dock area and return size
    // This is a simplified implementation
    if (!_mainWindow) {
        return 0;
    }

    // Return a default size - in a full implementation, this would
    // track the actual sizes of dock areas
    switch (iCont) {
        case CONT_LEFT:
        case CONT_RIGHT:
            return 200; // default width
        case CONT_TOP:
        case CONT_BOTTOM:
            return 150; // default height
        default:
            return 0;
    }
}

void DockingManager::setDockedContSize(int iCont, int iSize)
{
    // In Qt, dock widget sizes are managed by QMainWindow
    // We would need to implement custom resizing logic here
    (void)iCont;
    (void)iSize;
}

std::vector<DockingCont*>& DockingManager::getContainerInfo()
{
    // Return dummy container vector for compatibility
    return _vContainer;
}

void DockingManager::resize()
{
    // In Qt, resizing is handled automatically by the layout system
    // This method is provided for API compatibility
}

void DockingManager::showFloatingContainers(bool show)
{
    for (auto it = _panels.begin(); it != _panels.end(); ++it) {
        PanelInfo* info = it.value().get();
        if (info && info->dockWidget && info->dockWidget->isFloating()) {
            if (show) {
                info->dockWidget->show();
            } else {
                info->dockWidget->hide();
            }
        }
    }
}

void DockingManager::updateContainerInfo(void* hClient)
{
    (void)hClient;
    // Compatibility stub
}

void DockingManager::createDockableDlg(tTbData data, int iCont, bool isVisible)
{
    (void)data;
    (void)iCont;
    (void)isVisible;
    // Compatibility stub - would need full implementation
}

void DockingManager::setActiveTab(int iCont, int iItem)
{
    (void)iCont;
    (void)iItem;
    // Compatibility stub
}

void DockingManager::showDockableDlg(void* hDlg, int view)
{
    (void)hDlg;
    (void)view;
    // Compatibility stub
}

void DockingManager::showDockableDlg(const wchar_t* pszName, int view)
{
    (void)pszName;
    (void)view;
    // Compatibility stub
}

Qt::DockWidgetArea DockingManager::dockAreaToQt(DockArea area) const
{
    switch (area) {
        case DockArea::Left:
            return Qt::LeftDockWidgetArea;
        case DockArea::Right:
            return Qt::RightDockWidgetArea;
        case DockArea::Top:
            return Qt::TopDockWidgetArea;
        case DockArea::Bottom:
            return Qt::BottomDockWidgetArea;
        case DockArea::Floating:
        default:
            return Qt::NoDockWidgetArea;
    }
}

DockingManager::DockArea DockingManager::qtToDockArea(Qt::DockWidgetArea area) const
{
    switch (area) {
        case Qt::LeftDockWidgetArea:
            return DockArea::Left;
        case Qt::RightDockWidgetArea:
            return DockArea::Right;
        case Qt::TopDockWidgetArea:
            return DockArea::Top;
        case Qt::BottomDockWidgetArea:
            return DockArea::Bottom;
        default:
            return DockArea::Floating;
    }
}

void DockingManager::setupDockWidget(QDockWidget* dockWidget, DockArea area)
{
    if (!dockWidget) {
        return;
    }

    QDockWidget::DockWidgetFeatures features =
        QDockWidget::DockWidgetClosable |
        QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable;

    dockWidget->setFeatures(features);

    if (area != DockArea::Floating) {
        dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    }
}

DockingManager::PanelInfo* DockingManager::getPanelInfo(const QString& name) const
{
    if (!_panels.contains(name)) {
        return nullptr;
    }
    return _panels[name].get();
}

} // namespace QtControls
