// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "PluginViewList.h"

// Include Qt headers
#include <QString>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

// Forward declaration
class PluginsManager;
class Version;

// Data structures for plugin management - platform-agnostic definitions
struct NppCurrentStatus {
    bool _isAdminMode = false;
    bool _isInProgramFiles = true;
    bool _isAppDataPluginsAllowed = false;
    std::wstring _nppInstallPath;
    std::wstring _appdataPath;
    bool shouldLaunchInAdmMode() const { return _isInProgramFiles; }
};

enum LIST_TYPE { AVAILABLE_LIST, UPDATES_LIST, INSTALLED_LIST, INCOMPATIBLE_LIST };

// Forward declaration - defined in PluginViewList.h
struct PluginUpdateInfo;

namespace QtControls {

// ============================================================================
// PluginsAdminDlg - Qt6 implementation of the Plugins Admin dialog
// ============================================================================
class PluginsAdminDlg : public StaticDialog
{
    Q_OBJECT

public:
    explicit PluginsAdminDlg(QWidget* parent = nullptr);
    ~PluginsAdminDlg() override;

    // Create and show the dialog
    void create(int dialogID, bool isRTL = false, bool msgDestParent = true);
    void doDialog(bool isRTL = false);

    // Initialization
    bool initFromJson();

    // Tab switching
    void switchDialog(int indexToSwitch);

    // Set the plugins manager
    void setPluginsManager(PluginsManager* pluginsManager);

    // Update the plugin lists
    bool updateList();

    // Admin mode
    void setAdminMode(bool isAdm);

    // Plugin operations
    bool installPlugins();
    bool updatePlugins();
    bool removePlugins();

    // Localization
    void changeTabName(LIST_TYPE index, const wchar_t* name2change);
    void changeColumnName(COLUMN_TYPE index, const wchar_t* name2change);

    // Get plugin list version
    std::wstring getPluginListVerStr() const;

    // Getters for plugin lists
    const PluginViewList& getAvailablePluginUpdateInfoList() const {
        return _availableList;
    }

    PluginViewList& getIncompatibleList() {
        return _incompatibleList;
    }

    // Check if dialog is created
    bool isCreated() const { return _isCreated; }

public slots:
    void onTabChanged(int index);
    void onSearchTextChanged(const QString& text);
    void onSearchNextClicked();
    void onInstallClicked();
    void onUpdateClicked();
    void onRemoveClicked();
    void onCloseClicked();
    void onItemSelectionChanged(int index);
    void onItemDoubleClicked(int index);

protected:
    void setupUI() override;
    void connectSignals() override;
    bool run_dlgProc(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // UI Components
    QTabWidget* _tabWidget = nullptr;

    // Search components
    QLineEdit* _searchEdit = nullptr;
    QPushButton* _searchNextButton = nullptr;
    QLabel* _searchLabel = nullptr;

    // Action buttons
    QPushButton* _installButton = nullptr;
    QPushButton* _updateButton = nullptr;
    QPushButton* _removeButton = nullptr;
    QPushButton* _closeButton = nullptr;

    // Version info
    QLabel* _versionLabel = nullptr;
    QLabel* _versionNumberLabel = nullptr;
    QLabel* _repoLinkLabel = nullptr;

    // Plugin lists for each tab
    PluginViewList _availableList;
    PluginViewList _updateList;
    PluginViewList _installedList;
    PluginViewList _incompatibleList;

    // Container widgets for tabs
    QWidget* _availableTab = nullptr;
    QWidget* _updatesTab = nullptr;
    QWidget* _installedTab = nullptr;
    QWidget* _incompatibleTab = nullptr;

    // Data
    PluginsManager* _pPluginsManager = nullptr;
    NppCurrentStatus* _nppCurrentStatus = nullptr;

    // Paths
    std::wstring _updaterDir;
    std::wstring _updaterFullPath;
    std::wstring _pluginListFullPath;
    std::wstring _pluginListVersion;

    // Search state
    QString _lastSearchText;
    bool _searchInNames = true;
    bool _searchInDescs = false;

    // Operations enum
    enum Operation {
        pa_install = 0,
        pa_update = 1,
        pa_remove = 2
    };

    // Internal methods
    void collectNppCurrentStatusInfos();
    bool searchInPlugins(bool isNextMode);
    bool isFoundInListFromIndex(const PluginViewList& inWhichList, int index, const std::wstring& str2search, bool inWhichPart) const;
    long searchFromCurrentSel(const PluginViewList& inWhichList, const std::wstring& str2search, bool inWhichPart, bool isNextMode) const;
    long searchInNamesFromCurrentSel(const PluginViewList& inWhichList, const std::wstring& str2search, bool isNextMode) const;
    long searchInDescsFromCurrentSel(const PluginViewList& inWhichList, const std::wstring& str2search, bool isNextMode) const;

    bool initAvailablePluginsViewFromList();
    bool initIncompatiblePluginList();
    bool loadFromPluginInfos();
    bool checkUpdates();

    bool exitToInstallRemovePlugins(Operation op, const std::vector<PluginUpdateInfo*>& puis);

    PluginViewList* getCurrentList();
    const PluginViewList* getCurrentList() const;

    void updateButtonStates();
    void createTabs();
    void setupTab(QWidget* tab, PluginViewList& list);
};

} // namespace QtControls
