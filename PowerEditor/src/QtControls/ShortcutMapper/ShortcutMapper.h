// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../Shortcut/Shortcut.h"
#include <vector>
#include <memory>

// Forward declarations
class QTabWidget;
class QTableWidget;
class QLineEdit;
class QLabel;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;

namespace QtControls {

namespace ShortcutMapper {

// Grid state matching the Windows version
enum class GridState { STATE_MENU, STATE_MACRO, STATE_USER, STATE_PLUGIN, STATE_SCINTILLA };

// Use global KeyCombo from Shortcut.h
using ::KeyCombo;

// Shortcut data structure for the grid
struct ShortcutData {
    QString name;
    QString shortcut;
    QString category;
    QString pluginName;
    KeyCombo keyCombo;
    bool isEnabled = false;
    size_t originalIndex = 0;
    bool hasConflict = false;
};

// ============================================================================
// ShortcutMapper - Qt implementation for managing keyboard shortcuts
// ============================================================================
class ShortcutMapper : public StaticDialog {
    Q_OBJECT

public:
    explicit ShortcutMapper(QWidget* parent = nullptr);
    ~ShortcutMapper() override;

    // Show the dialog with specified initial tab
    void doDialog(GridState initState = GridState::STATE_MENU);

    // Get client rectangle for the grid area
    void getClientRect(QRect& rc) const;

    // Check for key conflicts
    bool findKeyConflicts(const KeyCombo& combo, size_t itemIndex, QString* conflictLocation);

private slots:
    void onTabChanged(int index);
    void onFilterChanged(const QString& text);
    void onItemDoubleClicked(int row, int column);
    void onItemSelectionChanged();
    void onModifyClicked();
    void onDeleteClicked();
    void onClearClicked();
    void onClearAllClicked();
    void onImportClicked();
    void onExportClicked();
    void onResetAllClicked();
    void onOkClicked();
    void onFilterClearClicked();
    void onContextMenu(const QPoint& pos);

private:
    void setupUI();
    void connectSignals();
    void initGrid();
    void fillGrid();
    bool isFilterValid(const ShortcutData& sc);
    QString getTabString(int index) const;
    bool isConflict(const KeyCombo& lhs, const KeyCombo& rhs) const;
    void updateButtonStates();
    void saveViewState();
    void restoreViewState();
    void showConflictInfo(const ShortcutData& data);
    void clearConflictInfo();
    QString keyComboToString(const KeyCombo& combo) const;

    // UI Components
    QTabWidget* _tabWidget = nullptr;
    QTableWidget* _grid = nullptr;
    QLineEdit* _filterEdit = nullptr;
    QLabel* _filterLabel = nullptr;
    QLabel* _conflictLabel = nullptr;
    QPushButton* _modifyButton = nullptr;
    QPushButton* _deleteButton = nullptr;
    QPushButton* _clearButton = nullptr;
    QPushButton* _clearAllButton = nullptr;
    QPushButton* _importButton = nullptr;
    QPushButton* _exportButton = nullptr;
    QPushButton* _resetAllButton = nullptr;
    QPushButton* _okButton = nullptr;
    QPushButton* _filterClearButton = nullptr;

    // State
    GridState _currentState = GridState::STATE_MENU;
    std::vector<std::wstring> _shortcutFilter;
    std::vector<size_t> _shortcutIndex;

    // View state for each tab
    std::vector<int> _lastHomeRow;
    std::vector<int> _lastCursorRow;

    // Tab names
    static const int _nbTab = 5;
    QString _tabNames[_nbTab];

    // Conflict info strings
    QString _conflictInfoOk;
    QString _conflictInfoEditing;

    // All shortcuts data (populated from NppParameters)
    std::vector<ShortcutData> _menuShortcuts;
    std::vector<ShortcutData> _macroShortcuts;
    std::vector<ShortcutData> _userShortcuts;
    std::vector<ShortcutData> _pluginShortcuts;
    std::vector<ShortcutData> _scintillaShortcuts;

    // Load shortcuts from NppParameters
    void loadShortcutsFromParameters();

    // Get command ID for a shortcut at the given index
    int getCommandIdForShortcut(size_t index) const;
};

} // namespace ShortcutMapper

} // namespace QtControls
