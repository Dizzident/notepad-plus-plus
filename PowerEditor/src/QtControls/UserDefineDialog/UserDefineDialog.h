// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../../ScintillaComponent/ScintillaEditView.h"
#include "../../ScintillaComponent/UserDefineResource.h"
#include <memory>
#include <vector>
#include <unordered_map>

// Forward declarations
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QTabWidget;
class QLabel;
class QGroupBox;
class QRadioButton;
class QPlainTextEdit;
class QSpinBox;
class QColorDialog;

// UDL constants from SciLexer.h
#define SCE_UDL_VERSION_MAJOR       2
#define SCE_UDL_VERSION_MINOR       1
#define SCE_UDL_VERSION_BUILD       0
#define SCE_UDL_VERSION_REVISION    12

// Keyword list indices
#define SCE_USER_KWLIST_COMMENTS                    0
#define SCE_USER_KWLIST_NUMBER_PREFIX1              1
#define SCE_USER_KWLIST_NUMBER_PREFIX2              2
#define SCE_USER_KWLIST_NUMBER_EXTRAS1              3
#define SCE_USER_KWLIST_NUMBER_EXTRAS2              4
#define SCE_USER_KWLIST_NUMBER_SUFFIX1              5
#define SCE_USER_KWLIST_NUMBER_SUFFIX2              6
#define SCE_USER_KWLIST_NUMBER_RANGE                7
#define SCE_USER_KWLIST_OPERATORS1                  8
#define SCE_USER_KWLIST_OPERATORS2                  9
#define SCE_USER_KWLIST_FOLDERS_IN_CODE1_OPEN       10
#define SCE_USER_KWLIST_FOLDERS_IN_CODE1_MIDDLE     11
#define SCE_USER_KWLIST_FOLDERS_IN_CODE1_CLOSE      12
#define SCE_USER_KWLIST_FOLDERS_IN_CODE2_OPEN       13
#define SCE_USER_KWLIST_FOLDERS_IN_CODE2_MIDDLE     14
#define SCE_USER_KWLIST_FOLDERS_IN_CODE2_CLOSE      15
#define SCE_USER_KWLIST_FOLDERS_IN_COMMENT_OPEN     16
#define SCE_USER_KWLIST_FOLDERS_IN_COMMENT_MIDDLE   17
#define SCE_USER_KWLIST_FOLDERS_IN_COMMENT_CLOSE    18
#define SCE_USER_KWLIST_KEYWORDS1                   19
#define SCE_USER_KWLIST_KEYWORDS2                   20
#define SCE_USER_KWLIST_KEYWORDS3                   21
#define SCE_USER_KWLIST_KEYWORDS4                   22
#define SCE_USER_KWLIST_KEYWORDS5                   23
#define SCE_USER_KWLIST_KEYWORDS6                   24
#define SCE_USER_KWLIST_KEYWORDS7                   25
#define SCE_USER_KWLIST_KEYWORDS8                   26
#define SCE_USER_KWLIST_DELIMITERS                  27
#define SCE_USER_KWLIST_TOTAL                       28

// Style indices
#define SCE_USER_STYLE_DEFAULT              0
#define SCE_USER_STYLE_COMMENT              1
#define SCE_USER_STYLE_COMMENTLINE          2
#define SCE_USER_STYLE_NUMBER               3
#define SCE_USER_STYLE_KEYWORD1             4
#define SCE_USER_STYLE_KEYWORD2             5
#define SCE_USER_STYLE_KEYWORD3             6
#define SCE_USER_STYLE_KEYWORD4             7
#define SCE_USER_STYLE_KEYWORD5             8
#define SCE_USER_STYLE_KEYWORD6             9
#define SCE_USER_STYLE_KEYWORD7             10
#define SCE_USER_STYLE_KEYWORD8             11
#define SCE_USER_STYLE_OPERATOR             12
#define SCE_USER_STYLE_FOLDER_IN_CODE1      13
#define SCE_USER_STYLE_FOLDER_IN_CODE2      14
#define SCE_USER_STYLE_FOLDER_IN_COMMENT    15
#define SCE_USER_STYLE_DELIMITER1           16
#define SCE_USER_STYLE_DELIMITER2           17
#define SCE_USER_STYLE_DELIMITER3           18
#define SCE_USER_STYLE_DELIMITER4           19
#define SCE_USER_STYLE_DELIMITER5           20
#define SCE_USER_STYLE_DELIMITER6           21
#define SCE_USER_STYLE_DELIMITER7           22
#define SCE_USER_STYLE_DELIMITER8           23
#define SCE_USER_STYLE_IDENTIFIER           24
#define SCE_USER_STYLE_TOTAL_STYLES         SCE_USER_STYLE_IDENTIFIER

// Nesting masks
#define SCE_USER_MASK_NESTING_NONE          0
#define SCE_USER_MASK_NESTING_DELIMITER1    0x1
#define SCE_USER_MASK_NESTING_DELIMITER2    0x2
#define SCE_USER_MASK_NESTING_DELIMITER3    0x4
#define SCE_USER_MASK_NESTING_DELIMITER4    0x8
#define SCE_USER_MASK_NESTING_DELIMITER5    0x10
#define SCE_USER_MASK_NESTING_DELIMITER6    0x20
#define SCE_USER_MASK_NESTING_DELIMITER7    0x40
#define SCE_USER_MASK_NESTING_DELIMITER8    0x80
#define SCE_USER_MASK_NESTING_COMMENT       0x100
#define SCE_USER_MASK_NESTING_COMMENT_LINE  0x200
#define SCE_USER_MASK_NESTING_KEYWORD1      0x400
#define SCE_USER_MASK_NESTING_KEYWORD2      0x800
#define SCE_USER_MASK_NESTING_KEYWORD3      0x1000
#define SCE_USER_MASK_NESTING_KEYWORD4      0x2000
#define SCE_USER_MASK_NESTING_KEYWORD5      0x4000
#define SCE_USER_MASK_NESTING_KEYWORD6      0x8000
#define SCE_USER_MASK_NESTING_KEYWORD7      0x10000
#define SCE_USER_MASK_NESTING_KEYWORD8      0x20000
#ifndef SCE_USER_MASK_NESTING_OPERATORS1
#define SCE_USER_MASK_NESTING_OPERATORS1    0x40000
#endif
#ifndef SCE_USER_MASK_NESTING_OPERATORS2
#define SCE_USER_MASK_NESTING_OPERATORS2    0x80000
#endif
#ifndef SCE_USER_MASK_NESTING_NUMBERS
#define SCE_USER_MASK_NESTING_NUMBERS       0x100000
#endif

// Font styles
#define FONTSTYLE_NONE      0
#define FONTSTYLE_BOLD      1
#define FONTSTYLE_ITALIC    2
#define FONTSTYLE_UNDERLINE 4

// Color styles
#define COLORSTYLE_FOREGROUND   1
#define COLORSTYLE_BACKGROUND   2

// Other constants
#define PURE_LC_NONE    0
#define PURE_LC_BOL     1
#define PURE_LC_WSP     2
#define DECSEP_DOT      0
#define DECSEP_COMMA    1
#define DECSEP_BOTH     2

// Total keyword groups (8 keywords + operators + folders + delimiters + comments + numbers)
#define SCE_USER_TOTAL_KEYWORD_GROUPS   8

// Forward declarations
class UserDefineDialog;

// ============================================================================
// Style Dialog - For editing individual style properties
// ============================================================================
class StyleDialog : public QtControls::StaticDialog {
    Q_OBJECT

public:
    StyleDialog(int styleIndex, int enabledNesters, QWidget* parent = nullptr);
    ~StyleDialog() override;

    void setStyleData(const QString& name, const QColor& fgColor, const QColor& bgColor,
                      int fontStyle, int fontSize, const QString& fontName,
                      int colorStyle, int nesting);
    void getStyleData(QColor& fgColor, QColor& bgColor, int& fontStyle,
                      int& fontSize, QString& fontName, int& colorStyle, int& nesting) const;

    int exec() override;
    bool run_dlgProc(QEvent* event) override {
        // Handle events or just return false
        (void)event;
        return false;
    }

private slots:
    void onFgColorClicked();
    void onBgColorClicked();
    void onFgTransparentToggled(bool checked);
    void onBgTransparentToggled(bool checked);
    void onFontSizeChanged(int index);
    void onFontNameChanged(int index);
    void onBoldToggled(bool checked);
    void onItalicToggled(bool checked);
    void onUnderlineToggled(bool checked);
    void onNestingChanged();

private:
    void setupUI(const QString& styleName);
    void updatePreview();

    int _styleIndex = 0;
    int _enabledNesters = 0;

    // Style data
    QColor _fgColor;
    QColor _bgColor;
    int _fontStyle = FONTSTYLE_NONE;
    int _fontSize = -1;
    QString _fontName;
    int _colorStyle = COLORSTYLE_FOREGROUND | COLORSTYLE_BACKGROUND;
    int _nesting = SCE_USER_MASK_NESTING_NONE;

    // UI elements
    QPushButton* _fgColorBtn = nullptr;
    QPushButton* _bgColorBtn = nullptr;
    QCheckBox* _fgTransparentCheck = nullptr;
    QCheckBox* _bgTransparentCheck = nullptr;
    QComboBox* _fontSizeCombo = nullptr;
    QComboBox* _fontNameCombo = nullptr;
    QCheckBox* _boldCheck = nullptr;
    QCheckBox* _italicCheck = nullptr;
    QCheckBox* _underlineCheck = nullptr;
    QLabel* _previewLabel = nullptr;

    // Nesting checkboxes
    std::vector<QCheckBox*> _nestingChecks;
};

// ============================================================================
// String Dialog - For entering language names
// ============================================================================
class StringDialog : public QtControls::StaticDialog {
    Q_OBJECT

public:
    StringDialog(const QString& title, const QString& label, const QString& initialText,
                 int maxLength, const QString& restrictedChars = QString(), QWidget* parent = nullptr);
    ~StringDialog() override;

    QString getText() const;
    int exec() override;
    bool run_dlgProc(QEvent* event) override {
        // Handle events or just return false
        (void)event;
        return false;
    }

private slots:
    void onTextChanged(const QString& text);

private:
    void setupUI();
    bool isAllowed(const QString& text) const;

    QString _title;
    QString _label;
    QString _initialText;
    QString _restrictedChars;
    int _maxLength = 0;

    QLineEdit* _lineEdit = nullptr;
    QPushButton* _okButton = nullptr;
};

// ============================================================================
// GlobalMappers - Maps between IDs and names
// ============================================================================
class GlobalMappers {
public:
    GlobalMappers();

    std::unordered_map<std::wstring, int> keywordIdMapper;
    std::unordered_map<int, std::wstring> keywordNameMapper;
    std::unordered_map<std::wstring, int> styleIdMapper;
    std::unordered_map<int, std::wstring> styleNameMapper;
    std::unordered_map<int, int> nestingMapper;
    std::unordered_map<int, int> dialogMapper;
    std::unordered_map<int, std::string> setLexerMapper;

    static GlobalMappers& instance();
};

// ============================================================================
// UserLangContainer - Container for user defined language data
// Qt version - wrapped in namespace to avoid conflict with Parameters.h version
// ============================================================================
namespace QtControls {

// Qt-specific UserLangContainer to avoid conflict with Parameters.h version
struct QtUserLangContainer {
    QtUserLangContainer();
    QtUserLangContainer(const QString& name, const QString& ext, bool isDarkModeTheme, const QString& udlVer);

    QString _name;
    QString _ext;
    bool _isDarkModeTheme = false;
    QString _udlVersion;

    // Keyword lists - each is a space-separated list of keywords
    wchar_t _keywordLists[SCE_USER_KWLIST_TOTAL][1024];

    // Prefix flags for keyword groups
    bool _isPrefix[SCE_USER_TOTAL_KEYWORD_GROUPS] = {false};

    // Options
    bool _isCaseIgnored = false;
    bool _allowFoldOfComments = false;
    int  _forcePureLC = PURE_LC_NONE;
    int _decimalSeparator = DECSEP_DOT;
    bool _foldCompact = false;

    // Styles array - indexed by SCE_USER_STYLE_*
    struct Style {
        QColor _fgColor = Qt::black;
        QColor _bgColor = Qt::white;
        int _fontStyle = FONTSTYLE_NONE;
        int _fontSize = -1;  // -1 means use default
        QString _fontName;
        int _colorStyle = COLORSTYLE_FOREGROUND | COLORSTYLE_BACKGROUND;
        int _nesting = SCE_USER_MASK_NESTING_NONE;
    };
    std::vector<Style> _styles;

    void addStyler(int styleID, const QString& name);
    Style* getStyler(int styleID);
    const Style* getStyler(int styleID) const;
};

} // namespace QtControls

// ============================================================================
// UserDefineDialog - Main UDL dialog
// ============================================================================
class UserDefineDialog : public QtControls::StaticDialog {
    Q_OBJECT

public:
    UserDefineDialog(QWidget* parent = nullptr);
    ~UserDefineDialog() override;

    void init(ScintillaEditView** ppEditView);
    void doDialog();
    void setCurrentLanguage(const QString& langName);

    bool isDocked() const { return _isDocked; }
    void setDockStatus(bool docked) { _isDocked = docked; }

    // Reload language combo from parameters
    void reloadLangCombo();

    // Change between docked and undocked mode
    void changeStyle();

    // Get window handle (compatibility with Windows code)
    HWND getHSelf() const { return reinterpret_cast<HWND>(const_cast<UserDefineDialog*>(this)); }

    // Get handles for sub-dialogs (for localization)
    // In Qt version, these return the main dialog handle since we use tabs instead of separate dialogs
    HWND getFolderHandle() const { return getHSelf(); }
    HWND getKeywordsHandle() const { return getHSelf(); }
    HWND getCommentHandle() const { return getHSelf(); }
    HWND getSymbolHandle() const { return getHSelf(); }

    // Set tab name (for localization)
    void setTabName(int index, const wchar_t* name);

private slots:
    void onLanguageChanged(int index);
    void onKeywordsChanged();
    void onStyleClicked(int styleIndex);
    void onImportClicked();
    void onExportClicked();
    void onNewLangClicked();
    void onRemoveLangClicked();
    void onRenameLangClicked();
    void onSaveAsClicked();
    void onDockClicked();
    void onCloseClicked();
    void onExtChanged(const QString& text);
    void onIgnoreCaseToggled(bool checked);
    void onFoldCompactToggled(bool checked);
    void onAllowFoldCommentsToggled(bool checked);
    void onForcePureLCChanged();
    void onDecimalSeparatorChanged();
    void onPrefixToggled(int group);
    void onTabChanged(int index);

private:
    void setupUI();
    void connectSignals();
    void loadLanguage(int index);
    void saveLanguage();
    void updatePreview();
    void enableLangAndControlsBy(int index);
    void createKeywordsTab();
    void createCommentsTab();
    void createNumbersTab();
    void createOperatorsTab();
    void createDelimitersTab();
    void createFolderTab();
    void updateStyleButtons();

    // Current language data
    std::unique_ptr<QtControls::QtUserLangContainer> _pCurrentUserLang;
    QtControls::QtUserLangContainer* _pUserLang = nullptr;
    ScintillaEditView** _ppEditView = nullptr;

    // Dock status
    bool _isDocked = false;

    // Language selection
    QComboBox* _langCombo = nullptr;
    QLineEdit* _extEdit = nullptr;
    QCheckBox* _ignoreCaseCheck = nullptr;

    // Tab widget
    QTabWidget* _mainTabs = nullptr;

    // Keywords tab
    QPlainTextEdit* _keywordLists[8];
    QCheckBox* _prefixChecks[8];
    QPushButton* _keywordStyleBtns[8];

    // Comments tab
    QLineEdit* _commentLineOpenEdit = nullptr;
    QLineEdit* _commentLineContinueEdit = nullptr;
    QLineEdit* _commentLineCloseEdit = nullptr;
    QLineEdit* _commentOpenEdit = nullptr;
    QLineEdit* _commentCloseEdit = nullptr;
    QCheckBox* _allowFoldCommentsCheck = nullptr;
    QPushButton* _commentLineStyleBtn = nullptr;
    QPushButton* _commentStyleBtn = nullptr;

    // Numbers tab
    QLineEdit* _numberPrefix1Edit = nullptr;
    QLineEdit* _numberPrefix2Edit = nullptr;
    QLineEdit* _numberExtras1Edit = nullptr;
    QLineEdit* _numberExtras2Edit = nullptr;
    QLineEdit* _numberSuffix1Edit = nullptr;
    QLineEdit* _numberSuffix2Edit = nullptr;
    QLineEdit* _numberRangeEdit = nullptr;
    QButtonGroup* _decimalSepGroup = nullptr;
    QRadioButton* _dotRadio = nullptr;
    QRadioButton* _commaRadio = nullptr;
    QRadioButton* _bothRadio = nullptr;
    QPushButton* _numberStyleBtn = nullptr;

    // Operators tab
    QLineEdit* _operators1Edit = nullptr;
    QLineEdit* _operators2Edit = nullptr;
    QPushButton* _operatorStyleBtn = nullptr;

    // Delimiters tab
    QLineEdit* _delimiterOpenEdits[8];
    QLineEdit* _delimiterCloseEdits[8];
    QLineEdit* _delimiterEscapeEdits[8];
    QPushButton* _delimiterStyleBtns[8];

    // Folder tab
    QLineEdit* _folderInCode1OpenEdit = nullptr;
    QLineEdit* _folderInCode1MiddleEdit = nullptr;
    QLineEdit* _folderInCode1CloseEdit = nullptr;
    QLineEdit* _folderInCode2OpenEdit = nullptr;
    QLineEdit* _folderInCode2MiddleEdit = nullptr;
    QLineEdit* _folderInCode2CloseEdit = nullptr;
    QLineEdit* _folderInCommentOpenEdit = nullptr;
    QLineEdit* _folderInCommentMiddleEdit = nullptr;
    QLineEdit* _folderInCommentCloseEdit = nullptr;
    QCheckBox* _foldCompactCheck = nullptr;
    QPushButton* _folderInCode1StyleBtn = nullptr;
    QPushButton* _folderInCode2StyleBtn = nullptr;
    QPushButton* _folderInCommentStyleBtn = nullptr;
    QPushButton* _defaultStyleBtn = nullptr;

    // Force pure LC options
    QButtonGroup* _forcePureLCGroup = nullptr;
    QRadioButton* _allowAnywhereRadio = nullptr;
    QRadioButton* _forceAtBOLRadio = nullptr;
    QRadioButton* _allowWhitespaceRadio = nullptr;

    // Action buttons
    QPushButton* _newLangButton = nullptr;
    QPushButton* _saveAsButton = nullptr;
    QPushButton* _removeLangButton = nullptr;
    QPushButton* _renameLangButton = nullptr;
    QPushButton* _importButton = nullptr;
    QPushButton* _exportButton = nullptr;
    QPushButton* _dockButton = nullptr;
    QPushButton* _closeButton = nullptr;

    // Style buttons array for easy access
    std::vector<QPushButton*> _styleButtons;
};
