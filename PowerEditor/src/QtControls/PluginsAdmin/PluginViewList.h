// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <vector>
#include <memory>
#include <utility>

// Version is defined in Common.h - include it
#include "../../MISC/Common/Common.h"

// Platform-agnostic plugin update info structure (mirrors Windows version)
struct PluginUpdateInfo {
    std::wstring _fullFilePath;
    std::wstring _folderName;
    std::wstring _displayName;
    Version _version;
    std::pair<Version, Version> _nppCompatibleVersions;
    std::pair<std::pair<Version, Version>, std::pair<Version, Version>> _oldVersionCompatibility;
    std::wstring _homepage;
    std::wstring _sourceUrl;
    std::wstring _description;
    std::wstring _author;
    std::wstring _id;
    std::wstring _repository;
    bool _isVisible = true;
};

enum COLUMN_TYPE { COLUMN_PLUGIN, COLUMN_VERSION };
enum SORT_TYPE { DISPLAY_NAME_ALPHABET_ENCREASE, DISPLAY_NAME_ALPHABET_DECREASE };

namespace QtControls {

// ============================================================================
// PluginListItem - Widget for a single plugin row with checkbox
// ============================================================================
class PluginListItem : public QWidget
{
    Q_OBJECT

public:
    explicit PluginListItem(const QString& displayName, const QString& version, QWidget* parent = nullptr);
    ~PluginListItem() override;

    bool isChecked() const;
    void setChecked(bool checked);

    QString displayName() const;
    QString version() const;

signals:
    void checkedChanged(bool checked);

private:
    QCheckBox* _checkBox = nullptr;
    QLabel* _nameLabel = nullptr;
    QLabel* _versionLabel = nullptr;
};

// ============================================================================
// PluginViewList - Manages the plugin list UI
// ============================================================================
class PluginViewList : public QWidget
{
    Q_OBJECT

public:
    explicit PluginViewList(QWidget* parent = nullptr);
    ~PluginViewList() override;

    // Add a plugin to the list
    void pushBack(PluginUpdateInfo* pi);

    // Get checked items
    std::vector<size_t> getCheckedIndexes() const;
    std::vector<PluginUpdateInfo*> getCheckedPlugins() const;

    // Selection
    long getSelectedIndex() const;
    void setSelection(int index);

    // Get plugin info from UI index
    PluginUpdateInfo* getPluginInfoFromUiIndex(size_t index) const;
    PluginUpdateInfo* findPluginInfoFromFolderName(const std::wstring& folderName, int& index) const;

    // List management
    size_t nbItem() const { return _list.size(); }
    bool removeFromListIndex(size_t index2remove);
    bool hideFromListIndex(size_t index2Hide);
    bool removeFromFolderName(const std::wstring& folderName);
    bool removeFromUiIndex(size_t index2remove);
    bool hideFromPluginInfoPtr(PluginUpdateInfo* pluginInfo2hide);
    bool restore(const std::wstring& folderName);
    bool removeFromPluginInfoPtr(PluginUpdateInfo* pluginInfo2remove);

    // Column management
    void changeColumnName(COLUMN_TYPE index, const wchar_t* name2change);

    // Search
    void filterItems(const QString& searchText, bool searchInNames = true, bool searchInDescs = true);

    // Clear all items
    void clear();

    // Get underlying table widget
    QTableWidget* getTableWidget() const { return _tableWidget; }

    // Get the internal list (for compatibility)
    const std::vector<PluginUpdateInfo*>& getList() const { return _list; }

signals:
    void itemSelectionChanged(int index);
    void itemCheckedChanged(int index, bool checked);
    void itemDoubleClicked(int index);

public slots:
    void onItemSelectionChanged();
    void onItemDoubleClicked(QTableWidgetItem* item);
    void onHeaderClicked(int column);

private:
    void setupUI();
    void updateRow(int row);

    // The list of plugins (synchronized with UI)
    std::vector<PluginUpdateInfo*> _list;

    // UI Components
    QTableWidget* _tableWidget = nullptr;

    // Sorting state
    SORT_TYPE _sortType = DISPLAY_NAME_ALPHABET_ENCREASE;
    int _sortColumn = 0;

    // Hidden items (for filtering)
    std::vector<std::pair<size_t, PluginUpdateInfo*>> _hiddenItems;
};

} // namespace QtControls
