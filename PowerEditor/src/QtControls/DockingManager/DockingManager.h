// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Window.h"
#include "Docking.h"
#include <QMainWindow>
#include <QDockWidget>
#include <QMap>
#include <QString>
#include <QByteArray>
#include <memory>
#include <vector>

namespace QtControls {

// Forward declarations for compatibility
class DockingCont;

// DockingManager class - Qt implementation compatible with Windows version
class DockingManager : public Window
{
    Q_OBJECT

public:
    enum class DockArea {
        Left,
        Right,
        Top,
        Bottom,
        Floating
    };

    struct PanelInfo {
        QString name;
        QString title;
        QWidget* widget = nullptr;
        QDockWidget* dockWidget = nullptr;
        DockArea area = DockArea::Right;
        bool visible = true;
        int id = 0;
    };

    DockingManager();
    ~DockingManager() override;

    // Windows-compatible interface
    void init(void* hInst, void* hWnd, Window** ppWin);
    void reSizeTo(QRect& rc) override;
    void destroy() override;

    void setClientWnd(Window** ppWin) {
        _ppWindow = ppWin;
        _ppMainWindow = ppWin;
    }

    void showFloatingContainers(bool show);

    void updateContainerInfo(void* hClient);
    void createDockableDlg(tTbData data, int iCont = CONT_LEFT, bool isVisible = false);
    void setActiveTab(int iCont, int iItem);
    void showDockableDlg(void* hDlg, int view);
    void showDockableDlg(const wchar_t* pszName, int view);

    // Qt-specific interface
    void init(QMainWindow* mainWindow);

    void addPanel(const QString& name, QWidget* widget, DockArea area,
                  const QString& title = QString());
    void removePanel(const QString& name);

    void showPanel(const QString& name);
    void hidePanel(const QString& name);
    void togglePanel(const QString& name);

    bool isPanelVisible(const QString& name) const;
    bool hasPanel(const QString& name) const;

    void setPanelArea(const QString& name, DockArea area);
    DockArea getPanelArea(const QString& name) const;

    void setPanelTitle(const QString& name, const QString& title);
    QString getPanelTitle(const QString& name) const;

    QWidget* getPanelWidget(const QString& name) const;
    QDockWidget* getDockWidget(const QString& name) const;

    void setTabbedDocking(const QString& name1, const QString& name2);

    QByteArray saveLayout() const;
    void restoreLayout(const QByteArray& layout);

    void showAllPanels();
    void hideAllPanels();

    QStringList getPanelNames() const;
    QStringList getVisiblePanels() const;

    void setPanelFeatures(const QString& name, bool closable, bool movable, bool floatable);

    void raisePanel(const QString& name);

    int getPanelCount() const;

    // Windows-compatible methods
    int getDockedContSize(int iCont);
    void setDockedContSize(int iCont, int iSize);
    std::vector<DockingCont*>& getContainerInfo();
    void resize();

private:
    QMainWindow* _mainWindow = nullptr;
    QMap<QString, std::shared_ptr<PanelInfo>> _panels;
    int _nextId = 1;

    Window** _ppWindow = nullptr;
    Window** _ppMainWindow = nullptr;
    QRect _rcWork;
    QRect _rect;
    bool _isInitialized = false;

    // Dummy container vector for compatibility
    std::vector<DockingCont*> _vContainer;

    Qt::DockWidgetArea dockAreaToQt(DockArea area) const;
    DockArea qtToDockArea(Qt::DockWidgetArea area) const;
    void setupDockWidget(QDockWidget* dockWidget, DockArea area);
    PanelInfo* getPanelInfo(const QString& name) const;
};

} // namespace QtControls
