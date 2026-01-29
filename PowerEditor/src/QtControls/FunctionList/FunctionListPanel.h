// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../../ScintillaComponent/ScintillaEditView.h"
#include <QObject>
#include <memory>
#include <vector>

// Forward declarations
class QLineEdit;
class QPushButton;
class QComboBox;
class QToolBar;
class QAction;
class QTreeWidgetItem;

namespace QtControls {

class TreeView;

// ============================================================================
// FunctionInfo - Structure to hold function/class information
// ============================================================================
struct FunctionInfo {
    QString name;
    QString displayName;
    int lineNumber = -1;
    QString type;        // "function", "class", "method", "namespace", etc.
    QString returnType;
    QString parameters;
    QString className;   // For methods, the containing class
    intptr_t pos = -1;   // Position in document

    bool operator<(const FunctionInfo& other) const {
        return name < other.name;
    }
};

// ============================================================================
// FunctionParser - Base class for language-specific parsers
// ============================================================================
class FunctionParser {
public:
    virtual ~FunctionParser() = default;

    // Parse content and return list of functions/classes
    virtual std::vector<FunctionInfo> parse(const QString& content) = 0;

    // Get parser display name
    virtual QString getDisplayName() const = 0;

    // Get supported language identifier
    virtual QString getLanguageId() const = 0;
};

// ============================================================================
// CppFunctionParser - C/C++ function parser
// ============================================================================
class CppFunctionParser : public FunctionParser {
public:
    std::vector<FunctionInfo> parse(const QString& content) override;
    QString getDisplayName() const override { return "C/C++"; }
    QString getLanguageId() const override { return "cpp"; }

private:
    void parseClassContent(const QString& content, const QString& className,
                          int classStartLine, std::vector<FunctionInfo>& results);
};

// ============================================================================
// PythonFunctionParser - Python function parser
// ============================================================================
class PythonFunctionParser : public FunctionParser {
public:
    std::vector<FunctionInfo> parse(const QString& content) override;
    QString getDisplayName() const override { return "Python"; }
    QString getLanguageId() const override { return "python"; }
};

// ============================================================================
// JavaScriptFunctionParser - JavaScript/TypeScript function parser
// ============================================================================
class JavaScriptFunctionParser : public FunctionParser {
public:
    std::vector<FunctionInfo> parse(const QString& content) override;
    QString getDisplayName() const override { return "JavaScript"; }
    QString getLanguageId() const override { return "javascript"; }
};

// ============================================================================
// JavaFunctionParser - Java function parser
// ============================================================================
class JavaFunctionParser : public FunctionParser {
public:
    std::vector<FunctionInfo> parse(const QString& content) override;
    QString getDisplayName() const override { return "Java"; }
    QString getLanguageId() const override { return "java"; }
};

// ============================================================================
// CSharpFunctionParser - C# function parser
// ============================================================================
class CSharpFunctionParser : public FunctionParser {
public:
    std::vector<FunctionInfo> parse(const QString& content) override;
    QString getDisplayName() const override { return "C#"; }
    QString getLanguageId() const override { return "csharp"; }
};

// ============================================================================
// FunctionParserManager - Manages parsers for different languages
// ============================================================================
class FunctionParserManager {
public:
    FunctionParserManager();
    ~FunctionParserManager() = default;

    // Get parser for a language
    FunctionParser* getParser(const QString& language) const;

    // Get parser for file extension
    FunctionParser* getParserForExtension(const QString& ext) const;

    // Get all available parser names
    QStringList getAvailableParsers() const;

    // Register a custom parser
    void registerParser(std::unique_ptr<FunctionParser> parser);

private:
    std::vector<std::unique_ptr<FunctionParser>> _parsers;

    void registerDefaultParsers();
};

// ============================================================================
// TreeState - For saving/restoring tree expansion state
// ============================================================================
struct TreeState {
    QString label;
    bool isExpanded = false;
    std::vector<TreeState> children;
};

// ============================================================================
// FunctionListPanel - Dockable panel showing function/class hierarchy
// ============================================================================
class FunctionListPanel : public StaticDialog {
    Q_OBJECT

public:
    explicit FunctionListPanel(QWidget* parent = nullptr);
    ~FunctionListPanel() override;

    // Initialize with Scintilla edit view
    void init(ScintillaEditView** ppEditView);

    // Show the panel
    void doDialog();

    // Event processing override
    bool run_dlgProc(QEvent* event) override;

    // Bring getDialog into public scope for getWidget
    using StaticDialog::getDialog;

    // Get widget for docking integration
    QWidget* getWidget() const { return getDialog(); }

    // Parse current document
    void parseCurrentDocument();

    // Sort control
    void setSortAlphabetically(bool sort);
    bool isSortedAlphabetically() const { return _sortAlpha; }

    // Refresh the function list
    void refresh();

    // Mark current position in tree
    void markEntry();

    // Set background/foreground colors
    void setBackgroundColor(const QColor& color);
    void setForegroundColor(const QColor& color);

public slots:
    void onItemClicked(int itemId);
    void onItemDoubleClicked(int itemId);
    void onRefreshClicked();
    void onSortClicked();
    void onExpandAllClicked();
    void onCollapseAllClicked();
    void onFilterChanged(const QString& text);
    void onLanguageChanged(const QString& lang);

protected:
    void setupUI();
    void connectSignals();

private:
    // UI Components
    TreeView* _treeView = nullptr;
    QLineEdit* _filterEdit = nullptr;
    QPushButton* _refreshBtn = nullptr;
    QPushButton* _sortBtn = nullptr;
    QPushButton* _expandBtn = nullptr;
    QPushButton* _collapseBtn = nullptr;
    QComboBox* _langCombo = nullptr;
    QToolBar* _toolBar = nullptr;

    // Actions for toolbar
    QAction* _sortAction = nullptr;
    QAction* _refreshAction = nullptr;
    QAction* _expandAction = nullptr;
    QAction* _collapseAction = nullptr;

    // Core data
    ScintillaEditView** _ppEditView = nullptr;
    std::unique_ptr<FunctionParserManager> _parserMgr;

    // State
    bool _sortAlpha = false;
    QString _currentFilter;
    QString _currentLanguage;
    std::vector<FunctionInfo> _functions;

    // Tree state preservation
    TreeState _savedTreeState;
    bool _isRestoringState = false;

    // Icon indices
    static constexpr int INDEX_ROOT = 0;
    static constexpr int INDEX_NODE = 1;
    static constexpr int INDEX_LEAF = 2;

    // Internal methods
    void parseDocument();
    void addFunctionToTree(const FunctionInfo& func);
    void addFunctionToTreeRecursive(const FunctionInfo& func, int parentId);
    void navigateToLine(int line);
    void navigateToPosition(intptr_t pos);
    void clearTree();
    void rebuildTree();

    // Sorting
    void sortFunctions();
    void sortTreeItems(int parentId);

    // Filtering
    void applyFilter();
    bool matchesFilter(const FunctionInfo& func) const;

    // Tree state management
    void saveTreeState();
    void restoreTreeState();
    void saveTreeStateRecursive(int itemId, TreeState& state);
    void restoreTreeStateRecursive(int itemId, const TreeState& state);

    // Language detection
    QString detectLanguage() const;
    void populateLanguageCombo();

    // Current item tracking
    int _currentMarkedItem = -1;
    void updateCurrentItemMarker();
};

} // namespace QtControls
