// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../../Platform/Settings.h"

#include <QWidget>
#include <QString>
#include <QHash>

// Forward declarations
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QPushButton;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QSlider;
class QLabel;
class QGroupBox;
class QRadioButton;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTextEdit;
class QTabWidget;

namespace QtControls {

// ============================================================================
// Forward declarations for all sub-dialogs
// ============================================================================
class GeneralSubDlg;
class EditingSubDlg;
class NewDocumentSubDlg;
class DefaultDirectorySubDlg;
class RecentFilesHistorySubDlg;
class LanguageSubDlg;
class HighlightingSubDlg;
class PrintSubDlg;
class SearchingSubDlg;
class BackupSubDlg;
class AutoCompletionSubDlg;
class MultiInstanceSubDlg;
class DelimiterSubDlg;
class CloudLinkSubDlg;
class SearchEngineSubDlg;
class MISCSubDlg;

// ============================================================================
// Category Info Structure
// ============================================================================
struct CategoryInfo {
    QString name;
    QString internalName;
    QWidget* page = nullptr;

    CategoryInfo() = default;
    CategoryInfo(const QString& n, const QString& internal, QWidget* p)
        : name(n), internalName(internal), page(p) {}
};

// ============================================================================
// GeneralSubDlg - General settings (language, toolbar, status bar)
// ============================================================================
class GeneralSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit GeneralSubDlg(QWidget* parent = nullptr);
    ~GeneralSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onStatusBarToggled(bool checked);
    void onMenuBarToggled(bool checked);
    void onHideMenuShortcutsToggled(bool checked);
    void onLanguageChanged(int index);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _generalGroup = nullptr;
    QCheckBox* _hideStatusBarCheck = nullptr;
    QCheckBox* _hideMenuBarCheck = nullptr;
    QCheckBox* _hideMenuShortcutsCheck = nullptr;

    QGroupBox* _localizationGroup = nullptr;
    QComboBox* _languageCombo = nullptr;
    QLabel* _languageLabel = nullptr;

    // Settings reference
    bool _statusBarShow = true;
    bool _menuBarShow = true;
    bool _hideMenuRightShortcuts = false;
    QString _currentLanguage = "English";
};

// ============================================================================
// EditingSubDlg - Editing settings (line numbering, indentation, caret)
// ============================================================================
class EditingSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit EditingSubDlg(QWidget* parent = nullptr);
    ~EditingSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onLineNumberToggled(bool checked);
    void onLineNumberDynamicWidthToggled(bool checked);
    void onCurrentLineHighlightChanged(int index);
    void onCaretBlinkRateChanged(int value);
    void onCaretWidthChanged(int index);
    void onSmoothFontToggled(bool checked);
    void onVirtualSpaceToggled(bool checked);
    void onScrollBeyondLastLineToggled(bool checked);
    void onRightClickKeepsSelectionToggled(bool checked);
    void onLineCopyCutWithoutSelectionToggled(bool checked);
    void onLineWrapMethodChanged(int index);

private:
    void setupUI();
    void connectSignals();
    void initScintParam();

    // UI Components - Line Numbering
    QGroupBox* _lineNumberGroup = nullptr;
    QCheckBox* _lineNumberCheck = nullptr;
    QCheckBox* _lineNumberDynamicWidthCheck = nullptr;

    // UI Components - Current Line
    QGroupBox* _currentLineGroup = nullptr;
    QComboBox* _currentLineHighlightCombo = nullptr;
    QLabel* _currentLineLabel = nullptr;

    // UI Components - Caret
    QGroupBox* _caretGroup = nullptr;
    QLabel* _caretBlinkRateLabel = nullptr;
    QSlider* _caretBlinkRateSlider = nullptr;
    QLabel* _caretBlinkRateValue = nullptr;
    QLabel* _caretWidthLabel = nullptr;
    QComboBox* _caretWidthCombo = nullptr;

    // UI Components - Scintilla Options
    QGroupBox* _scintillaGroup = nullptr;
    QCheckBox* _smoothFontCheck = nullptr;
    QCheckBox* _virtualSpaceCheck = nullptr;
    QCheckBox* _scrollBeyondLastLineCheck = nullptr;
    QCheckBox* _rightClickKeepsSelectionCheck = nullptr;
    QCheckBox* _lineCopyCutWithoutSelectionCheck = nullptr;

    // UI Components - Line Wrap
    QGroupBox* _lineWrapGroup = nullptr;
    QComboBox* _lineWrapCombo = nullptr;

    // Settings
    bool _lineNumberShow = true;
    bool _lineNumberDynamicWidth = true;
    int _currentLineHighlightMode = 1; // 0=none, 1=hilite, 2=frame
    int _caretBlinkRate = 600;
    int _caretWidth = 1;
    bool _doSmoothFont = false;
    bool _virtualSpace = false;
    bool _scrollBeyondLastLine = true;
    bool _rightClickKeepsSelection = false;
    bool _lineCopyCutWithoutSelection = false;
    int _lineWrapMethod = 0; // 0=default, 1=aligned, 2=indent
};

// ============================================================================
// NewDocumentSubDlg - New document defaults
// ============================================================================
class NewDocumentSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit NewDocumentSubDlg(QWidget* parent = nullptr);
    ~NewDocumentSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onEncodingChanged(int index);
    void onFormatChanged(int index);
    void onLanguageChanged(int index);
    void onAnsiAsUtf8Toggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _encodingGroup = nullptr;
    QComboBox* _encodingCombo = nullptr;
    QCheckBox* _ansiAsUtf8Check = nullptr;

    QGroupBox* _formatGroup = nullptr;
    QComboBox* _formatCombo = nullptr;

    QGroupBox* _languageGroup = nullptr;
    QComboBox* _languageCombo = nullptr;

    QGroupBox* _defaultsGroup = nullptr;
    QCheckBox* _applyToOpenedAnsiFilesCheck = nullptr;

    // Settings
    int _defaultEncoding = 0; // UTF-8
    int _defaultFormat = 0; // Windows (CR LF)
    int _defaultLanguage = 0; // Text
    bool _openAnsiAsUtf8 = true;
};

// ============================================================================
// DefaultDirectorySubDlg - Default file paths
// ============================================================================
class DefaultDirectorySubDlg : public QWidget {
    Q_OBJECT

public:
    explicit DefaultDirectorySubDlg(QWidget* parent = nullptr);
    ~DefaultDirectorySubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onDirectoryTypeChanged(int index);
    void onBrowseClicked();
    void onCustomPathChanged(const QString& path);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _directoryGroup = nullptr;
    QComboBox* _directoryTypeCombo = nullptr;
    QLineEdit* _customPathEdit = nullptr;
    QPushButton* _browseButton = nullptr;

    // Settings
    int _defaultDirectoryType = 0; // 0=follow current doc, 1=last used, 2=custom
    QString _customDefaultDirectory;
};

// ============================================================================
// RecentFilesHistorySubDlg - Recent files settings
// ============================================================================
class RecentFilesHistorySubDlg : public QWidget {
    Q_OBJECT

public:
    explicit RecentFilesHistorySubDlg(QWidget* parent = nullptr);
    ~RecentFilesHistorySubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onMaxFilesChanged(int value);
    void onCustomLengthChanged(int value);
    void onDontCheckAtStartupToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _recentFilesGroup = nullptr;
    QLabel* _maxFilesLabel = nullptr;
    QSpinBox* _maxFilesSpin = nullptr;

    QLabel* _customLengthLabel = nullptr;
    QSpinBox* _customLengthSpin = nullptr;

    QCheckBox* _dontCheckAtStartupCheck = nullptr;

    // Settings
    int _maxRecentFiles = 10;
    int _customLength = 0;
    bool _dontCheckAtStartup = false;
};

// ============================================================================
// LanguageSubDlg - Language-specific settings
// ============================================================================
class LanguageSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit LanguageSubDlg(QWidget* parent = nullptr);
    ~LanguageSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onLanguageSelected(int index);
    void onTabSizeChanged(int value);
    void onReplaceBySpaceToggled(bool checked);

private:
    void setupUI();
    void connectSignals();
    void populateLanguageList();

    // UI Components
    QGroupBox* _languageListGroup = nullptr;
    QListWidget* _languageList = nullptr;

    QGroupBox* _indentationGroup = nullptr;
    QLabel* _tabSizeLabel = nullptr;
    QSpinBox* _tabSizeSpin = nullptr;
    QCheckBox* _replaceBySpaceCheck = nullptr;

    // Settings
    int _selectedLanguage = 0;
    int _tabSize = 4;
    bool _replaceBySpace = false;
};

// ============================================================================
// HighlightingSubDlg - Smart highlighting, matching
// ============================================================================
class HighlightingSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit HighlightingSubDlg(QWidget* parent = nullptr);
    ~HighlightingSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onSmartHighlightingToggled(bool checked);
    void onMatchCaseToggled(bool checked);
    void onWholeWordToggled(bool checked);
    void onMatchSelectionToggled(bool checked);
    void onBraceHighlightingToggled(bool checked);
    void onTagHighlightingToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components - Smart Highlighting
    QGroupBox* _smartHighlightGroup = nullptr;
    QCheckBox* _enableSmartHighlightCheck = nullptr;
    QCheckBox* _matchCaseCheck = nullptr;
    QCheckBox* _wholeWordCheck = nullptr;
    QCheckBox* _matchSelectionCheck = nullptr;

    // UI Components - Brace/Tag Matching
    QGroupBox* _matchingGroup = nullptr;
    QCheckBox* _braceHighlightCheck = nullptr;
    QCheckBox* _tagHighlightCheck = nullptr;

    // Settings
    bool _enableSmartHighlight = true;
    bool _smartHighlightMatchCase = false;
    bool _smartHighlightWholeWord = true;
    bool _smartHighlightUseSelection = false;
    bool _enableBraceHighlight = true;
    bool _enableTagHighlight = true;
};

// ============================================================================
// PrintSubDlg - Print settings
// ============================================================================
class PrintSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit PrintSubDlg(QWidget* parent = nullptr);
    ~PrintSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onPrintLineNumberToggled(bool checked);
    void onPrintSelectionToggled(bool checked);
    void onNoBackgroundToggled(bool checked);
    void onHeaderFooterToggled(bool checked);
    void onHeaderLeftChanged(const QString& text);
    void onHeaderCenterChanged(const QString& text);
    void onHeaderRightChanged(const QString& text);
    void onFooterLeftChanged(const QString& text);
    void onFooterCenterChanged(const QString& text);
    void onFooterRightChanged(const QString& text);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _printOptionsGroup = nullptr;
    QCheckBox* _printLineNumberCheck = nullptr;
    QCheckBox* _printSelectionCheck = nullptr;
    QCheckBox* _noBackgroundCheck = nullptr;

    QGroupBox* _headerFooterGroup = nullptr;
    QCheckBox* _enableHeaderFooterCheck = nullptr;

    QLabel* _headerLeftLabel = nullptr;
    QLineEdit* _headerLeftEdit = nullptr;
    QLabel* _headerCenterLabel = nullptr;
    QLineEdit* _headerCenterEdit = nullptr;
    QLabel* _headerRightLabel = nullptr;
    QLineEdit* _headerRightEdit = nullptr;

    QLabel* _footerLeftLabel = nullptr;
    QLineEdit* _footerLeftEdit = nullptr;
    QLabel* _footerCenterLabel = nullptr;
    QLineEdit* _footerCenterEdit = nullptr;
    QLabel* _footerRightLabel = nullptr;
    QLineEdit* _footerRightEdit = nullptr;

    // Settings
    bool _printLineNumber = true;
    bool _printSelection = false;
    bool _noBackground = true;
    bool _headerFooterEnabled = true;
    QString _headerLeft;
    QString _headerCenter;
    QString _headerRight;
    QString _footerLeft;
    QString _footerCenter;
    QString _footerRight;
};

// ============================================================================
// SearchingSubDlg - Search behavior
// ============================================================================
class SearchingSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit SearchingSubDlg(QWidget* parent = nullptr);
    ~SearchingSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onStopAtFirstMatchToggled(bool checked);
    void onWrapAroundToggled(bool checked);
    void onMatchWholeWordToggled(bool checked);
    void onMatchCaseToggled(bool checked);
    void onInSelectionThresholdChanged(int value);
    void onFillFindWhatToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _searchOptionsGroup = nullptr;
    QCheckBox* _stopAtFirstMatchCheck = nullptr;
    QCheckBox* _wrapAroundCheck = nullptr;
    QCheckBox* _matchWholeWordCheck = nullptr;
    QCheckBox* _matchCaseCheck = nullptr;

    QGroupBox* _advancedGroup = nullptr;
    QLabel* _inSelectionThresholdLabel = nullptr;
    QSpinBox* _inSelectionThresholdSpin = nullptr;
    QCheckBox* _fillFindWhatCheck = nullptr;

    // Settings
    bool _stopAtFirstMatch = false;
    bool _wrapAround = true;
    bool _matchWholeWord = false;
    bool _matchCase = false;
    int _inSelectionThreshold = 1024;
    bool _fillFindWhatWithSelection = true;
};

// ============================================================================
// BackupSubDlg - Auto-backup settings
// ============================================================================
class BackupSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit BackupSubDlg(QWidget* parent = nullptr);
    ~BackupSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onBackupModeChanged(int index);
    void onBackupDirectoryTypeChanged(int index);
    void onCustomDirectoryChanged(const QString& path);
    void onBrowseDirectoryClicked();
    void onSessionSnapshotToggled(bool checked);

private:
    void setupUI();
    void connectSignals();
    void updateBackupUI();

    // UI Components
    QGroupBox* _backupGroup = nullptr;
    QComboBox* _backupModeCombo = nullptr;
    QLabel* _backupModeLabel = nullptr;

    QGroupBox* _backupDirectoryGroup = nullptr;
    QComboBox* _backupDirectoryCombo = nullptr;
    QLineEdit* _customDirectoryEdit = nullptr;
    QPushButton* _browseButton = nullptr;

    QGroupBox* _sessionGroup = nullptr;
    QCheckBox* _sessionSnapshotCheck = nullptr;
    QLabel* _snapshotIntervalLabel = nullptr;
    QSpinBox* _snapshotIntervalSpin = nullptr;

    // Settings
    int _backupMode = 0; // 0=none, 1=simple, 2=verbose
    int _backupDirectoryType = 0; // 0=same dir, 1=custom
    QString _customBackupDirectory;
    bool _sessionSnapshot = true;
    int _snapshotInterval = 7;
};

// ============================================================================
// AutoCompletionSubDlg - Auto-completion settings
// ============================================================================
class AutoCompletionSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit AutoCompletionSubDlg(QWidget* parent = nullptr);
    ~AutoCompletionSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onAutoCompletionToggled(bool checked);
    void onAutoInsertBracketsToggled(bool checked);
    void onAutoInsertQuotesToggled(bool checked);
    void onAutoIndentToggled(bool checked);
    void onAutoCompletionIgnoreNumbersToggled(bool checked);
    void onAutoCompletionFromChanged(int index);
    void onAutoCompletionThresholdChanged(int value);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _autoCompletionGroup = nullptr;
    QCheckBox* _enableAutoCompletionCheck = nullptr;
    QLabel* _thresholdLabel = nullptr;
    QSpinBox* _thresholdSpin = nullptr;

    QGroupBox* _autoInsertGroup = nullptr;
    QCheckBox* _autoInsertBracketsCheck = nullptr;
    QCheckBox* _autoInsertQuotesCheck = nullptr;

    QGroupBox* _autoIndentGroup = nullptr;
    QCheckBox* _enableAutoIndentCheck = nullptr;

    QGroupBox* _completionSourceGroup = nullptr;
    QComboBox* _completionSourceCombo = nullptr;
    QCheckBox* _ignoreNumbersCheck = nullptr;

    // Settings
    bool _enableAutoCompletion = true;
    int _autoCompletionThreshold = 1;
    bool _autoInsertBrackets = true;
    bool _autoInsertQuotes = true;
    bool _enableAutoIndent = true;
    int _autoCompletionSource = 0; // 0=all, 1=words, 2=functions, 3=words+functions
    bool _autoCompletionIgnoreNumbers = false;
};

// ============================================================================
// MultiInstanceSubDlg - Multi-instance behavior
// ============================================================================
class MultiInstanceSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit MultiInstanceSubDlg(QWidget* parent = nullptr);
    ~MultiInstanceSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onMultiInstanceModeChanged(int index);
    void onDateTimeFormatChanged(const QString& format);
    void onUseCustomDateTimeToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _multiInstanceGroup = nullptr;
    QComboBox* _multiInstanceCombo = nullptr;
    QLabel* _multiInstanceLabel = nullptr;

    QGroupBox* _dateTimeFormatGroup = nullptr;
    QCheckBox* _useCustomDateTimeCheck = nullptr;
    QLineEdit* _dateTimeFormatEdit = nullptr;
    QLabel* _dateTimePreviewLabel = nullptr;

    // Settings
    int _multiInstanceMode = 0; // 0=mono, 1=multi, 2=session
    bool _useCustomDateTime = false;
    QString _dateTimeFormat;
};

// ============================================================================
// DelimiterSubDlg - Word delimiter settings
// ============================================================================
class DelimiterSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit DelimiterSubDlg(QWidget* parent = nullptr);
    ~DelimiterSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onDelimiterListChanged();
    void onWordCharListChanged();
    void onDefaultWordCharsToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _delimiterGroup = nullptr;
    QLabel* _delimiterListLabel = nullptr;
    QLineEdit* _delimiterListEdit = nullptr;

    QGroupBox* _wordCharGroup = nullptr;
    QRadioButton* _defaultWordCharsRadio = nullptr;
    QRadioButton* _customWordCharsRadio = nullptr;
    QLineEdit* _customWordCharsEdit = nullptr;

    // Settings
    QString _delimiterList;
    bool _useDefaultWordChars = true;
    QString _customWordChars;
};

// ============================================================================
// CloudLinkSubDlg - Cloud/OneDrive settings
// ============================================================================
class CloudLinkSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit CloudLinkSubDlg(QWidget* parent = nullptr);
    ~CloudLinkSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onCloudPathChanged(const QString& path);
    void onBrowseCloudPathClicked();
    void onCloudSyncToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _cloudGroup = nullptr;
    QCheckBox* _enableCloudSyncCheck = nullptr;
    QLabel* _cloudPathLabel = nullptr;
    QLineEdit* _cloudPathEdit = nullptr;
    QPushButton* _browseButton = nullptr;

    // Settings
    bool _cloudSyncEnabled = false;
    QString _cloudPath;
};

// ============================================================================
// SearchEngineSubDlg - Search engine preferences
// ============================================================================
class SearchEngineSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit SearchEngineSubDlg(QWidget* parent = nullptr);
    ~SearchEngineSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onSearchEngineChanged(int index);
    void onCustomUrlChanged(const QString& url);
    void onSetDefaultClicked();

private:
    void setupUI();
    void connectSignals();
    void populateSearchEngines();

    // UI Components
    QGroupBox* _searchEngineGroup = nullptr;
    QComboBox* _searchEngineCombo = nullptr;
    QLabel* _customUrlLabel = nullptr;
    QLineEdit* _customUrlEdit = nullptr;
    QPushButton* _setDefaultButton = nullptr;

    // Settings
    int _selectedSearchEngine = 0; // 0=Google, 1=Bing, 2=DuckDuckGo, 3=custom
    QString _customSearchUrl;
};

// ============================================================================
// MISCSubDlg - Miscellaneous settings
// ============================================================================
class MISCSubDlg : public QWidget {
    Q_OBJECT

public:
    explicit MISCSubDlg(QWidget* parent = nullptr);
    ~MISCSubDlg() override = default;

    void loadSettings();
    void saveSettings();
    bool applySettings();

private slots:
    void onMinimizeToTrayToggled(bool checked);
    void onAutoUpdateToggled(bool checked);
    void onEnableNotificationsToggled(bool checked);
    void onFileAutoDetectionChanged(int index);
    void onMuteSoundsToggled(bool checked);
    void onConfirmExitToggled(bool checked);
    void onConfirmDeleteToggled(bool checked);

private:
    void setupUI();
    void connectSignals();

    // UI Components
    QGroupBox* _generalMiscGroup = nullptr;
    QCheckBox* _minimizeToTrayCheck = nullptr;
    QCheckBox* _autoUpdateCheck = nullptr;
    QCheckBox* _enableNotificationsCheck = nullptr;
    QCheckBox* _muteSoundsCheck = nullptr;
    QCheckBox* _confirmExitCheck = nullptr;
    QCheckBox* _confirmDeleteCheck = nullptr;

    QGroupBox* _fileDetectionGroup = nullptr;
    QComboBox* _fileAutoDetectionCombo = nullptr;
    QLabel* _fileAutoDetectionLabel = nullptr;

    // Settings
    bool _minimizeToTray = false;
    bool _autoUpdate = true;
    bool _enableNotifications = true;
    int _fileAutoDetection = 1; // 0=disable, 1=enable, 2=update silently
    bool _muteSounds = false;
    bool _confirmExit = true;
    bool _confirmDelete = true;
};

// ============================================================================
// PreferenceDlg - Main Preferences Dialog
// ============================================================================
class PreferenceDlg : public StaticDialog {
    Q_OBJECT

public:
    explicit PreferenceDlg(QWidget* parent = nullptr);
    ~PreferenceDlg() override;

    void doDialog();
    void doDialog(bool isRTL);

    // Navigation methods
    void showPage(const QString& internalName);
    void showPage(int index);
    int getCurrentPageIndex() const;

private slots:
    void onCategoryChanged(int index);
    void onCategoryItemClicked(QListWidgetItem* item);
    void onOKClicked();
    void onCancelClicked();
    void onApplyClicked();

private:
    void setupUI();
    void createSubPages();
    void loadSettings();
    bool saveSettings();
    void connectSignals();
    bool run_dlgProc(QEvent* event) override;

    // UI Components
    QWidget* _mainWidget = nullptr;
    QListWidget* _categoryList = nullptr;
    QStackedWidget* _pagesStack = nullptr;

    QPushButton* _okButton = nullptr;
    QPushButton* _cancelButton = nullptr;
    QPushButton* _applyButton = nullptr;

    // Sub-page instances
    GeneralSubDlg* _generalPage = nullptr;
    EditingSubDlg* _editingPage = nullptr;
    NewDocumentSubDlg* _newDocumentPage = nullptr;
    DefaultDirectorySubDlg* _defaultDirectoryPage = nullptr;
    RecentFilesHistorySubDlg* _recentFilesHistoryPage = nullptr;
    LanguageSubDlg* _languagePage = nullptr;
    HighlightingSubDlg* _highlightingPage = nullptr;
    PrintSubDlg* _printPage = nullptr;
    SearchingSubDlg* _searchingPage = nullptr;
    BackupSubDlg* _backupPage = nullptr;
    AutoCompletionSubDlg* _autoCompletionPage = nullptr;
    MultiInstanceSubDlg* _multiInstancePage = nullptr;
    DelimiterSubDlg* _delimiterPage = nullptr;
    CloudLinkSubDlg* _cloudLinkPage = nullptr;
    SearchEngineSubDlg* _searchEnginePage = nullptr;
    MISCSubDlg* _miscPage = nullptr;

    // Category list
    QVector<CategoryInfo> _categories;

    // Settings interface
    PlatformLayer::ISettings* _settings = nullptr;
};

} // namespace QtControls
