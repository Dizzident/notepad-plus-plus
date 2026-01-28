// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../../ScintillaComponent/ScintillaEditView.h"
#include <memory>
#include <vector>
#include <functional>

// Forward declarations
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QTabWidget;
class QLabel;
class QGroupBox;
class QRadioButton;

namespace Platform {

// Search types matching Windows version
enum class SearchType {
    Normal = 0,
    Extended = 1,
    Regex = 2
};

// Search direction
enum class SearchDirection {
    Down = true,
    Up = false
};

// Dialog type (tabs)
enum class FindDialogType {
    Find = 0,
    Replace = 1,
    FindInFiles = 2,
    FindInProjects = 3,
    Mark = 4
};

// Search options structure
struct FindOptions {
    bool isWholeWord = false;
    bool isMatchCase = false;
    bool isWrapAround = true;
    SearchDirection direction = SearchDirection::Down;
    SearchType searchType = SearchType::Normal;
    bool isInSelection = false;
    QString str2Search;
    QString str4Replace;
    QString filters;
    QString directory;
    bool isRecursive = true;
    bool isInHiddenDir = false;
    bool dotMatchesNewline = false;
    bool doPurge = false;
    bool doMarkLine = false;
};

// Find status
enum class FindStatus {
    Found,
    NotFound,
    TopReached,
    EndReached,
    Message,
    NoMessage,
    Warning
};

// ============================================================================
// FindReplaceDlg - Qt implementation
// ============================================================================
class FindReplaceDlg : public StaticDialog {
    Q_OBJECT

public:
    FindReplaceDlg(QWidget* parent = nullptr);
    ~FindReplaceDlg() override;

    // Initialize the dialog
    void init(ScintillaEditView** ppEditView);

    // Show specific tab
    void showDialog(FindDialogType type);

    // Set search text
    void setSearchText(const QString& text);

    // Get current search text
    QString getSearchText() const;

    // Get current replace text
    QString getReplaceText() const;

    // Get current options
    FindOptions getCurrentOptions() const;

    // Set options
    void setOptions(const FindOptions& options);

    // Search operations
    bool findNext();
    bool findPrevious();
    bool replace();
    bool replaceAll();
    bool replaceAllInOpenDocs();
    bool findAllInCurrentDoc();
    bool findAllInOpenDocs();
    bool findAllInFiles();

    // Count occurrences
    int countMatches();

    // Mark all occurrences
    int markAll(int styleId = -1);
    void clearMarks();

    // Set status message
    void setStatusMessage(const QString& msg, FindStatus status);

    // Save/restore position
    void savePosition();
    void restorePosition();

    // Static search options (shared across instances)
    static FindOptions options;
    static FindOptions* env;

public slots:
    void onFindNextClicked();
    void onFindPreviousClicked();
    void onReplaceClicked();
    void onReplaceAllClicked();
    void onReplaceAllInOpenDocsClicked();
    void onFindAllClicked();
    void onCountClicked();
    void onMarkAllClicked();
    void onClearMarksClicked();
    void onCloseClicked();
    void onSwapFindReplaceClicked();

    void onSearchTextChanged(const QString& text);
    void onSearchTextEdited();
    void onTabChanged(int index);

    void onBrowseDirectoryClicked();
    void onDirFromActiveDocClicked();

    void updateControlStates();

protected:
    void setupUI() override;
    void connectSignals() override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // UI Components - Find tab
    QLineEdit* _findWhatEdit = nullptr;
    QComboBox* _findWhatCombo = nullptr;
    QCheckBox* _matchWholeWordCheck = nullptr;
    QCheckBox* _matchCaseCheck = nullptr;
    QCheckBox* _wrapAroundCheck = nullptr;
    QGroupBox* _directionGroup = nullptr;
    QRadioButton* _dirUpRadio = nullptr;
    QRadioButton* _dirDownRadio = nullptr;
    QGroupBox* _searchModeGroup = nullptr;
    QRadioButton* _modeNormalRadio = nullptr;
    QRadioButton* _modeExtendedRadio = nullptr;
    QRadioButton* _modeRegexRadio = nullptr;
    QCheckBox* _dotMatchesNewlineCheck = nullptr;
    QCheckBox* _inSelectionCheck = nullptr;

    // UI Components - Replace tab
    QLineEdit* _replaceWithEdit = nullptr;
    QComboBox* _replaceWithCombo = nullptr;

    // UI Components - Find in Files tab
    QComboBox* _filtersCombo = nullptr;
    QComboBox* _directoryCombo = nullptr;
    QPushButton* _browseButton = nullptr;
    QPushButton* _dirFromActiveDocButton = nullptr;
    QCheckBox* _recursiveCheck = nullptr;
    QCheckBox* _hiddenDirCheck = nullptr;
    QCheckBox* _projectPanel1Check = nullptr;
    QCheckBox* _projectPanel2Check = nullptr;
    QCheckBox* _projectPanel3Check = nullptr;

    // UI Components - Mark tab
    QCheckBox* _bookmarkLineCheck = nullptr;
    QCheckBox* _purgeCheck = nullptr;
    QPushButton* _copyMarkedTextButton = nullptr;

    // UI Components - Common
    QTabWidget* _tabWidget = nullptr;
    QLabel* _statusLabel = nullptr;

    // Buttons
    QPushButton* _findNextButton = nullptr;
    QPushButton* _findPreviousButton = nullptr;
    QPushButton* _replaceButton = nullptr;
    QPushButton* _replaceAllButton = nullptr;
    QPushButton* _replaceAllInOpenDocsButton = nullptr;
    QPushButton* _findAllButton = nullptr;
    QPushButton* _countButton = nullptr;
    QPushButton* _markAllButton = nullptr;
    QPushButton* _clearMarksButton = nullptr;
    QPushButton* _closeButton = nullptr;
    QPushButton* _swapButton = nullptr;

    // State
    ScintillaEditView** _ppEditView = nullptr;
    FindDialogType _currentType = FindDialogType::Find;
    QString _statusMessage;
    FindStatus _findStatus = FindStatus::NoMessage;

    // History
    std::vector<QString> _findHistory;
    std::vector<QString> _replaceHistory;
    std::vector<QString> _filterHistory;
    std::vector<QString> _directoryHistory;

    // Internal methods
    void createFindTab();
    void createReplaceTab();
    void createFindInFilesTab();
    void createMarkTab();

    void loadHistory();
    void saveHistory();
    void addToHistory(const QString& text, std::vector<QString>& history);

    bool processFindNext(const QString& text, const FindOptions& options);
    bool processReplace(const QString& findText, const QString& replaceText, const FindOptions& options);
    int processReplaceAll(const QString& findText, const QString& replaceText, const FindOptions& options);
    int processMarkAll(const QString& findText, int styleId, const FindOptions& options);
    int processCount(const QString& findText, const FindOptions& options);
    int processFindInFiles(const QString& findText, const FindOptions& options);

    void updateComboHistory(QComboBox* combo, const std::vector<QString>& history);
    void saveComboHistory(QComboBox* combo, std::vector<QString>& history, int maxCount);

    void enableFindControls(bool enable);
    void enableReplaceControls(bool enable);
    void enableFindInFilesControls(bool enable);
    void enableMarkControls(bool enable);

    int buildSearchFlags(const FindOptions& options) const;
    QString convertExtendedString(const QString& input);

    void showWarning(const QString& msg);
    void showInfo(const QString& msg);

    // Scintilla interaction
    intptr_t searchInTarget(const QString& text, const FindOptions& options);
    void displaySectionCentered(intptr_t start, intptr_t end, bool isDownwards = true);
};

// ============================================================================
// Incremental Find Dialog
// ============================================================================
class FindIncrementDlg : public StaticDialog {
    Q_OBJECT

public:
    FindIncrementDlg(QWidget* parent = nullptr);
    ~FindIncrementDlg() override;

    void init(FindReplaceDlg* pFRDlg, ScintillaEditView** ppEditView);

    void setSearchText(const QString& text);
    void setFindStatus(FindStatus status, int count = -1);

public slots:
    void onTextChanged(const QString& text);
    void onFindNext();
    void onFindPrevious();
    void onHighlightAll();
    void onCaseSensitiveToggled(bool checked);
    void onWholeWordToggled(bool checked);
    void onRegexToggled(bool checked);

protected:
    void setupUI() override;
    void connectSignals() override;

private:
    QLineEdit* _searchEdit = nullptr;
    QLabel* _statusLabel = nullptr;
    QPushButton* _nextButton = nullptr;
    QPushButton* _prevButton = nullptr;
    QPushButton* _highlightButton = nullptr;
    QCheckBox* _caseSensitiveCheck = nullptr;
    QCheckBox* _wholeWordCheck = nullptr;
    QCheckBox* _regexCheck = nullptr;

    FindReplaceDlg* _pFRDlg = nullptr;
    ScintillaEditView** _ppEditView = nullptr;
    FindStatus _findStatus = FindStatus::Found;
};

// ============================================================================
// Progress Dialog for long operations
// ============================================================================
class FindProgressDlg : public StaticDialog {
    Q_OBJECT

public:
    explicit FindProgressDlg(QWidget* parent = nullptr);
    ~FindProgressDlg() override;

    void setHeader(const QString& header);
    void setPercent(int percent);
    void setInfo(const QString& info);
    void setHitsCount(int count);

    bool isCancelled() const { return _cancelled; }

public slots:
    void onCancel();

protected:
    void setupUI() override;
    void connectSignals() override;
    void closeEvent(QCloseEvent* event) override;

private:
    QLabel* _headerLabel = nullptr;
    QLabel* _infoLabel = nullptr;
    QLabel* _hitsLabel = nullptr;
    class QProgressBar* _progressBar = nullptr;
    QPushButton* _cancelButton = nullptr;
    bool _cancelled = false;
};

// ============================================================================
// Finder Results Panel (dockable)
// ============================================================================
class FinderPanel : public StaticDialog {
    Q_OBJECT

public:
    FinderPanel(QWidget* parent = nullptr);
    ~FinderPanel() override;

    void init(ScintillaEditView** ppEditView);

    void addSearchLine(const QString& searchText);
    void addFileNameTitle(const QString& fileName);
    void addFileHitCount(int count);
    void addSearchResult(const QString& line, int lineNumber, intptr_t start, intptr_t end);

    void beginNewFilesSearch();
    void finishFilesSearch(int count, int searchedCount);

    void removeAll();
    void openAll();
    void copy();
    void copyPathnames();

    void gotoNextResult();
    void gotoPreviousResult();

    void setFinderStyle();

public slots:
    void onResultDoubleClicked(int line);
    void onContextMenu(const QPoint& pos);

protected:
    void setupUI() override;
    void connectSignals() override;

private:
    ScintillaEditView* _scintillaView = nullptr;
    ScintillaEditView** _ppEditView = nullptr;

    struct FoundInfo {
        intptr_t start = 0;
        intptr_t end = 0;
        int lineNumber = 0;
        QString filePath;
    };

    std::vector<FoundInfo> _foundInfos;
    int _currentFileCount = 0;

    void gotoFoundLine(size_t index);
};

} // namespace Platform
