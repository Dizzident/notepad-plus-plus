// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "UserDefineDialog.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QFrame>
#include <QtGui/QFontDatabase>

// For NppParameters integration
#include "../../Parameters.h"
#include "../../ScintillaComponent/ScintillaEditView.h"
#include <SciLexer.h>

#include <cwchar>
#include <algorithm>

// UDL file operations
#define max_char 1024

// Windows lstrlen is not available on Linux, use wcslen instead
#ifndef _WIN32
#define lstrlen wcslen
#endif

// Helper function to convert text with prefix markers (from Windows implementation)
static void convertTo(wchar_t* dest, int destLen, const wchar_t* toConvert, const wchar_t* prefix)
{
    bool inGroup = false;
    int index = lstrlen(dest);
    if (index > 0)
        dest[index++] = L' ';
    dest[index++] = prefix[0];
    dest[index++] = prefix[1];

    for (size_t i = 0, len = lstrlen(toConvert); i < len && index < destLen - 7; ++i)
    {
        if (i == 0 && toConvert[i] == L'(' && toConvert[i + 1] == L'(')
        {
            inGroup = true;
        }
        else if (toConvert[i] == L' ' && toConvert[i + 1] == L'(' && toConvert[i + 2] == L'(')
        {
            inGroup = true;
            dest[index++] = L' ';
            dest[index++] = prefix[0];
            dest[index++] = prefix[1];
            ++i;    // skip space
        }

        if (inGroup && toConvert[i - 1] == L')' && toConvert[i - 2] == L')')
        {
            inGroup = false;
        }

        if (toConvert[i] == L' ')
        {
            if (toConvert[i + 1] != L' ' && toConvert[i + 1] != L'\0')
            {
                dest[index++] = L' ';
                if (!inGroup)
                {
                    dest[index++] = prefix[0];
                    dest[index++] = prefix[1];
                }
            }
        }
        else
        {
            dest[index++] = toConvert[i];
        }
    }
    dest[index] = L'\0';
}

// Helper function to retrieve text with prefix markers
static void retrieveFromKeywordList(wchar_t* dest, const wchar_t* toRetrieve, const wchar_t* prefix)
{
    int j = 0;
    bool begin2Copy = false;
    bool inGroup = false;

    for (size_t i = 0, len = lstrlen(toRetrieve); i < len; ++i)
    {
        if ((i == 0 || (toRetrieve[i - 1] == L' ')) && (toRetrieve[i] == prefix[0] && toRetrieve[i + 1] == prefix[1]))
        {
            if (j > 0)
                dest[j++] = L' ';

            begin2Copy = true;
            ++i;
            continue;
        }
        if (toRetrieve[i] == L'(' && toRetrieve[i + 1] == L'(' && inGroup == false && begin2Copy == true)
        {
            inGroup = true;
        }
        if (toRetrieve[i] != L')' && toRetrieve[i - 1] == L')' && toRetrieve[i - 2] == L')' && inGroup == true)
        {
            inGroup = false;
        }
        if (toRetrieve[i] == L' ' && begin2Copy == true && !inGroup)
        {
            begin2Copy = false;
        }

        if (begin2Copy || inGroup)
            dest[j++] = toRetrieve[i];
    }
    dest[j++] = L'\0';
}

// ============================================================================
// GlobalMappers Implementation (in QtControls namespace)
// ============================================================================
namespace QtControls {

GlobalMappers::GlobalMappers()
{
    // Keyword name mappings - pre 2.0 compatibility
    temp[L"Operators"]                     = SCE_USER_KWLIST_OPERATORS1;
    temp[L"Folder+"]                       = SCE_USER_KWLIST_FOLDERS_IN_CODE1_OPEN;
    temp[L"Folder-"]                       = SCE_USER_KWLIST_FOLDERS_IN_CODE1_CLOSE;
    temp[L"Words1"]                        = SCE_USER_KWLIST_KEYWORDS1;
    temp[L"Words2"]                        = SCE_USER_KWLIST_KEYWORDS2;
    temp[L"Words3"]                        = SCE_USER_KWLIST_KEYWORDS3;
    temp[L"Words4"]                        = SCE_USER_KWLIST_KEYWORDS4;

    for (iter = temp.begin(); iter != temp.end(); ++iter)
        keywordNameMapper[iter->second] = iter->first;
    keywordIdMapper.insert(temp.begin(), temp.end());
    temp.clear();

    // Keyword name mappings - 2.0
    temp[L"Comments"]                      = SCE_USER_KWLIST_COMMENTS;
    temp[L"Numbers, additional"]           = SCE_USER_KWLIST_NUMBER_RANGE;
    temp[L"Numbers, prefixes"]             = SCE_USER_KWLIST_NUMBER_PREFIX2;
    temp[L"Numbers, extras with prefixes"] = SCE_USER_KWLIST_NUMBER_EXTRAS2;
    temp[L"Numbers, suffixes"]             = SCE_USER_KWLIST_NUMBER_SUFFIX2;
    temp[L"Operators1"]                    = SCE_USER_KWLIST_OPERATORS1;
    temp[L"Operators2"]                    = SCE_USER_KWLIST_OPERATORS2;
    temp[L"Folders in code1, open"]        = SCE_USER_KWLIST_FOLDERS_IN_CODE1_OPEN;
    temp[L"Folders in code1, middle"]      = SCE_USER_KWLIST_FOLDERS_IN_CODE1_MIDDLE;
    temp[L"Folders in code1, close"]       = SCE_USER_KWLIST_FOLDERS_IN_CODE1_CLOSE;
    temp[L"Folders in code2, open"]        = SCE_USER_KWLIST_FOLDERS_IN_CODE2_OPEN;
    temp[L"Folders in code2, middle"]      = SCE_USER_KWLIST_FOLDERS_IN_CODE2_MIDDLE;
    temp[L"Folders in code2, close"]       = SCE_USER_KWLIST_FOLDERS_IN_CODE2_CLOSE;
    temp[L"Folders in comment, open"]      = SCE_USER_KWLIST_FOLDERS_IN_COMMENT_OPEN;
    temp[L"Folders in comment, middle"]    = SCE_USER_KWLIST_FOLDERS_IN_COMMENT_MIDDLE;
    temp[L"Folders in comment, close"]     = SCE_USER_KWLIST_FOLDERS_IN_COMMENT_CLOSE;
    temp[L"Keywords1"]                     = SCE_USER_KWLIST_KEYWORDS1;
    temp[L"Keywords2"]                     = SCE_USER_KWLIST_KEYWORDS2;
    temp[L"Keywords3"]                     = SCE_USER_KWLIST_KEYWORDS3;
    temp[L"Keywords4"]                     = SCE_USER_KWLIST_KEYWORDS4;
    temp[L"Keywords5"]                     = SCE_USER_KWLIST_KEYWORDS5;
    temp[L"Keywords6"]                     = SCE_USER_KWLIST_KEYWORDS6;
    temp[L"Keywords7"]                     = SCE_USER_KWLIST_KEYWORDS7;
    temp[L"Keywords8"]                     = SCE_USER_KWLIST_KEYWORDS8;
    temp[L"Delimiters"]                    = SCE_USER_KWLIST_DELIMITERS;

    for (iter = temp.begin(); iter != temp.end(); ++iter)
        keywordNameMapper[iter->second] = iter->first;
    keywordIdMapper.insert(temp.begin(), temp.end());
    temp.clear();

    // Keyword name mappings - 2.1
    temp[L"Numbers, prefix1"]              = SCE_USER_KWLIST_NUMBER_PREFIX1;
    temp[L"Numbers, prefix2"]              = SCE_USER_KWLIST_NUMBER_PREFIX2;
    temp[L"Numbers, extras1"]              = SCE_USER_KWLIST_NUMBER_EXTRAS1;
    temp[L"Numbers, extras2"]              = SCE_USER_KWLIST_NUMBER_EXTRAS2;
    temp[L"Numbers, suffix1"]              = SCE_USER_KWLIST_NUMBER_SUFFIX1;
    temp[L"Numbers, suffix2"]              = SCE_USER_KWLIST_NUMBER_SUFFIX2;
    temp[L"Numbers, range"]                = SCE_USER_KWLIST_NUMBER_RANGE;

    for (iter = temp.begin(); iter != temp.end(); ++iter)
        keywordNameMapper[iter->second] = iter->first;
    keywordIdMapper.insert(temp.begin(), temp.end());
    temp.clear();

    // Style name mappings - pre 2.0
    temp[L"FOLDEROPEN"]           = SCE_USER_STYLE_FOLDER_IN_CODE1;
    temp[L"FOLDERCLOSE"]          = SCE_USER_STYLE_FOLDER_IN_CODE1;
    temp[L"KEYWORD1"]             = SCE_USER_STYLE_KEYWORD1;
    temp[L"KEYWORD2"]             = SCE_USER_STYLE_KEYWORD2;
    temp[L"KEYWORD3"]             = SCE_USER_STYLE_KEYWORD3;
    temp[L"KEYWORD4"]             = SCE_USER_STYLE_KEYWORD4;
    temp[L"COMMENT"]              = SCE_USER_STYLE_COMMENT;
    temp[L"COMMENT LINE"]         = SCE_USER_STYLE_COMMENTLINE;
    temp[L"NUMBER"]               = SCE_USER_STYLE_NUMBER;
    temp[L"OPERATOR"]             = SCE_USER_STYLE_OPERATOR;
    temp[L"DELIMINER1"]           = SCE_USER_STYLE_DELIMITER1;
    temp[L"DELIMINER2"]           = SCE_USER_STYLE_DELIMITER2;
    temp[L"DELIMINER3"]           = SCE_USER_STYLE_DELIMITER3;

    for (iter = temp.begin(); iter != temp.end(); ++iter)
        styleNameMapper[iter->second] = iter->first;
    styleIdMapper.insert(temp.begin(), temp.end());
    temp.clear();

    // Style name mappings - post 2.0
    temp[L"DEFAULT"]              = SCE_USER_STYLE_DEFAULT;
    temp[L"COMMENTS"]             = SCE_USER_STYLE_COMMENT;
    temp[L"LINE COMMENTS"]        = SCE_USER_STYLE_COMMENTLINE;
    temp[L"NUMBERS"]              = SCE_USER_STYLE_NUMBER;
    temp[L"KEYWORDS1"]            = SCE_USER_STYLE_KEYWORD1;
    temp[L"KEYWORDS2"]            = SCE_USER_STYLE_KEYWORD2;
    temp[L"KEYWORDS3"]            = SCE_USER_STYLE_KEYWORD3;
    temp[L"KEYWORDS4"]            = SCE_USER_STYLE_KEYWORD4;
    temp[L"KEYWORDS5"]            = SCE_USER_STYLE_KEYWORD5;
    temp[L"KEYWORDS6"]            = SCE_USER_STYLE_KEYWORD6;
    temp[L"KEYWORDS7"]            = SCE_USER_STYLE_KEYWORD7;
    temp[L"KEYWORDS8"]            = SCE_USER_STYLE_KEYWORD8;
    temp[L"OPERATORS"]            = SCE_USER_STYLE_OPERATOR;
    temp[L"FOLDER IN CODE1"]      = SCE_USER_STYLE_FOLDER_IN_CODE1;
    temp[L"FOLDER IN CODE2"]      = SCE_USER_STYLE_FOLDER_IN_CODE2;
    temp[L"FOLDER IN COMMENT"]    = SCE_USER_STYLE_FOLDER_IN_COMMENT;
    temp[L"DELIMITERS1"]          = SCE_USER_STYLE_DELIMITER1;
    temp[L"DELIMITERS2"]          = SCE_USER_STYLE_DELIMITER2;
    temp[L"DELIMITERS3"]          = SCE_USER_STYLE_DELIMITER3;
    temp[L"DELIMITERS4"]          = SCE_USER_STYLE_DELIMITER4;
    temp[L"DELIMITERS5"]          = SCE_USER_STYLE_DELIMITER5;
    temp[L"DELIMITERS6"]          = SCE_USER_STYLE_DELIMITER6;
    temp[L"DELIMITERS7"]          = SCE_USER_STYLE_DELIMITER7;
    temp[L"DELIMITERS8"]          = SCE_USER_STYLE_DELIMITER8;

    for (iter = temp.begin(); iter != temp.end(); ++iter)
        styleNameMapper[iter->second] = iter->first;
    styleIdMapper.insert(temp.begin(), temp.end());
    temp.clear();

    // Nesting mapper
    nestingMapper[IDC_STYLER_CHECK_NESTING_DELIMITER1]      = SCE_USER_MASK_NESTING_DELIMITER1;
    nestingMapper[IDC_STYLER_CHECK_NESTING_DELIMITER2]      = SCE_USER_MASK_NESTING_DELIMITER2;
    nestingMapper[IDC_STYLER_CHECK_NESTING_DELIMITER3]      = SCE_USER_MASK_NESTING_DELIMITER3;
    nestingMapper[IDC_STYLER_CHECK_NESTING_DELIMITER4]      = SCE_USER_MASK_NESTING_DELIMITER4;
    nestingMapper[IDC_STYLER_CHECK_NESTING_DELIMITER5]      = SCE_USER_MASK_NESTING_DELIMITER5;
    nestingMapper[IDC_STYLER_CHECK_NESTING_DELIMITER6]      = SCE_USER_MASK_NESTING_DELIMITER6;
    nestingMapper[IDC_STYLER_CHECK_NESTING_DELIMITER7]      = SCE_USER_MASK_NESTING_DELIMITER7;
    nestingMapper[IDC_STYLER_CHECK_NESTING_DELIMITER8]      = SCE_USER_MASK_NESTING_DELIMITER8;
    nestingMapper[IDC_STYLER_CHECK_NESTING_COMMENT]         = SCE_USER_MASK_NESTING_COMMENT;
    nestingMapper[IDC_STYLER_CHECK_NESTING_COMMENT_LINE]    = SCE_USER_MASK_NESTING_COMMENT_LINE;
    nestingMapper[IDC_STYLER_CHECK_NESTING_KEYWORD1]        = SCE_USER_MASK_NESTING_KEYWORD1;
    nestingMapper[IDC_STYLER_CHECK_NESTING_KEYWORD2]        = SCE_USER_MASK_NESTING_KEYWORD2;
    nestingMapper[IDC_STYLER_CHECK_NESTING_KEYWORD3]        = SCE_USER_MASK_NESTING_KEYWORD3;
    nestingMapper[IDC_STYLER_CHECK_NESTING_KEYWORD4]        = SCE_USER_MASK_NESTING_KEYWORD4;
    nestingMapper[IDC_STYLER_CHECK_NESTING_KEYWORD5]        = SCE_USER_MASK_NESTING_KEYWORD5;
    nestingMapper[IDC_STYLER_CHECK_NESTING_KEYWORD6]        = SCE_USER_MASK_NESTING_KEYWORD6;
    nestingMapper[IDC_STYLER_CHECK_NESTING_KEYWORD7]        = SCE_USER_MASK_NESTING_KEYWORD7;
    nestingMapper[IDC_STYLER_CHECK_NESTING_KEYWORD8]        = SCE_USER_MASK_NESTING_KEYWORD8;
    nestingMapper[IDC_STYLER_CHECK_NESTING_OPERATORS1]      = SCE_USER_MASK_NESTING_OPERATORS1;
    nestingMapper[IDC_STYLER_CHECK_NESTING_OPERATORS2]      = SCE_USER_MASK_NESTING_OPERATORS2;
    nestingMapper[IDC_STYLER_CHECK_NESTING_NUMBERS]         = SCE_USER_MASK_NESTING_NUMBERS;

    // Dialog mapper for control IDs
    dialogMapper[IDC_NUMBER_PREFIX1_EDIT]           = SCE_USER_KWLIST_NUMBER_PREFIX1;
    dialogMapper[IDC_NUMBER_PREFIX2_EDIT]           = SCE_USER_KWLIST_NUMBER_PREFIX2;
    dialogMapper[IDC_NUMBER_EXTRAS1_EDIT]           = SCE_USER_KWLIST_NUMBER_EXTRAS1;
    dialogMapper[IDC_NUMBER_EXTRAS2_EDIT]           = SCE_USER_KWLIST_NUMBER_EXTRAS2;
    dialogMapper[IDC_NUMBER_SUFFIX1_EDIT]           = SCE_USER_KWLIST_NUMBER_SUFFIX1;
    dialogMapper[IDC_NUMBER_SUFFIX2_EDIT]           = SCE_USER_KWLIST_NUMBER_SUFFIX2;
    dialogMapper[IDC_NUMBER_RANGE_EDIT]             = SCE_USER_KWLIST_NUMBER_RANGE;

    dialogMapper[IDC_FOLDER_IN_CODE1_OPEN_EDIT]     = SCE_USER_KWLIST_FOLDERS_IN_CODE1_OPEN;
    dialogMapper[IDC_FOLDER_IN_CODE1_MIDDLE_EDIT]   = SCE_USER_KWLIST_FOLDERS_IN_CODE1_MIDDLE;
    dialogMapper[IDC_FOLDER_IN_CODE1_CLOSE_EDIT]    = SCE_USER_KWLIST_FOLDERS_IN_CODE1_CLOSE;
    dialogMapper[IDC_FOLDER_IN_CODE2_OPEN_EDIT]     = SCE_USER_KWLIST_FOLDERS_IN_CODE2_OPEN;
    dialogMapper[IDC_FOLDER_IN_CODE2_MIDDLE_EDIT]   = SCE_USER_KWLIST_FOLDERS_IN_CODE2_MIDDLE;
    dialogMapper[IDC_FOLDER_IN_CODE2_CLOSE_EDIT]    = SCE_USER_KWLIST_FOLDERS_IN_CODE2_CLOSE;
    dialogMapper[IDC_FOLDER_IN_COMMENT_OPEN_EDIT]   = SCE_USER_KWLIST_FOLDERS_IN_COMMENT_OPEN;
    dialogMapper[IDC_FOLDER_IN_COMMENT_MIDDLE_EDIT] = SCE_USER_KWLIST_FOLDERS_IN_COMMENT_MIDDLE;
    dialogMapper[IDC_FOLDER_IN_COMMENT_CLOSE_EDIT]  = SCE_USER_KWLIST_FOLDERS_IN_COMMENT_CLOSE;

    dialogMapper[IDC_KEYWORD1_EDIT]                 = SCE_USER_KWLIST_KEYWORDS1;
    dialogMapper[IDC_KEYWORD2_EDIT]                 = SCE_USER_KWLIST_KEYWORDS2;
    dialogMapper[IDC_KEYWORD3_EDIT]                 = SCE_USER_KWLIST_KEYWORDS3;
    dialogMapper[IDC_KEYWORD4_EDIT]                 = SCE_USER_KWLIST_KEYWORDS4;
    dialogMapper[IDC_KEYWORD5_EDIT]                 = SCE_USER_KWLIST_KEYWORDS5;
    dialogMapper[IDC_KEYWORD6_EDIT]                 = SCE_USER_KWLIST_KEYWORDS6;
    dialogMapper[IDC_KEYWORD7_EDIT]                 = SCE_USER_KWLIST_KEYWORDS7;
    dialogMapper[IDC_KEYWORD8_EDIT]                 = SCE_USER_KWLIST_KEYWORDS8;

    // SetLexer mapper
    setLexerMapper[SCE_USER_KWLIST_COMMENTS]                = "userDefine.comments";
    setLexerMapper[SCE_USER_KWLIST_DELIMITERS]              = "userDefine.delimiters";
    setLexerMapper[SCE_USER_KWLIST_OPERATORS1]              = "userDefine.operators1";
    setLexerMapper[SCE_USER_KWLIST_NUMBER_PREFIX1]          = "userDefine.numberPrefix1";
    setLexerMapper[SCE_USER_KWLIST_NUMBER_PREFIX2]          = "userDefine.numberPrefix2";
    setLexerMapper[SCE_USER_KWLIST_NUMBER_EXTRAS1]          = "userDefine.numberExtras1";
    setLexerMapper[SCE_USER_KWLIST_NUMBER_EXTRAS2]          = "userDefine.numberExtras2";
    setLexerMapper[SCE_USER_KWLIST_NUMBER_SUFFIX1]          = "userDefine.numberSuffix1";
    setLexerMapper[SCE_USER_KWLIST_NUMBER_SUFFIX2]          = "userDefine.numberSuffix2";
    setLexerMapper[SCE_USER_KWLIST_NUMBER_RANGE]            = "userDefine.numberRange";
    setLexerMapper[SCE_USER_KWLIST_FOLDERS_IN_CODE1_OPEN]   = "userDefine.foldersInCode1Open";
    setLexerMapper[SCE_USER_KWLIST_FOLDERS_IN_CODE1_MIDDLE] = "userDefine.foldersInCode1Middle";
    setLexerMapper[SCE_USER_KWLIST_FOLDERS_IN_CODE1_CLOSE]  = "userDefine.foldersInCode1Close";
}

GlobalMappers& GlobalMappers::instance()
{
    static GlobalMappers instance;
    return instance;
}

} // namespace QtControls

// ============================================================================
// QtUserLangContainer Implementation (in QtControls namespace)
// ============================================================================
namespace QtControls {

QtUserLangContainer::QtUserLangContainer()
    : _name("new user define"), _ext(""), _isDarkModeTheme(false), _udlVersion("")
{
    for (int i = 0; i < SCE_USER_KWLIST_TOTAL; ++i) {
        _keywordLists[i][0] = L'\0';
    }
    _styles.resize(SCE_USER_STYLE_TOTAL_STYLES);
}

QtUserLangContainer::QtUserLangContainer(const QString& name, const QString& ext,
                                     bool isDarkModeTheme, const QString& udlVer)
    : _name(name), _ext(ext), _isDarkModeTheme(isDarkModeTheme), _udlVersion(udlVer)
{
    for (int i = 0; i < SCE_USER_KWLIST_TOTAL; ++i) {
        _keywordLists[i][0] = L'\0';
    }
    _styles.resize(SCE_USER_STYLE_TOTAL_STYLES);
}

void QtUserLangContainer::addStyler(int styleID, const QString& name)
{
    if (styleID >= 0 && styleID < SCE_USER_STYLE_TOTAL_STYLES) {
        // Style already exists in vector, just update name if needed
        (void)name;
    }
}

QtUserLangContainer::Style* QtUserLangContainer::getStyler(int styleID)
{
    if (styleID >= 0 && styleID < static_cast<int>(_styles.size())) {
        return &_styles[styleID];
    }
    return nullptr;
}

const QtUserLangContainer::Style* QtUserLangContainer::getStyler(int styleID) const
{
    if (styleID >= 0 && styleID < static_cast<int>(_styles.size())) {
        return &_styles[styleID];
    }
    return nullptr;
}

} // namespace QtControls

namespace QtControls {

// ============================================================================
// StyleDialog Implementation
// ============================================================================
StyleDialog::StyleDialog(int styleIndex, int enabledNesters, QWidget* parent)
    : StaticDialog(parent), _styleIndex(styleIndex), _enabledNesters(enabledNesters)
{
    QString styleName = QString::fromStdWString(
        GlobalMappers::instance().styleNameMapper[styleIndex]);
    setupUI(styleName);
}

StyleDialog::~StyleDialog() = default;

void StyleDialog::setupUI(const QString& styleName)
{
    create(tr("Style Settings - %1").arg(styleName), false);
    QDialog* dialog = getDialog();
    if (!dialog) return;

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Font group
    auto* fontGroup = new QGroupBox(tr("Font Settings"), dialog);
    auto* fontLayout = new QGridLayout(fontGroup);

    // Font name
    fontLayout->addWidget(new QLabel(tr("Font name:")), 0, 0);
    _fontNameCombo = new QComboBox(dialog);
    QFontDatabase fontDb;
    QStringList families = fontDb.families();
    _fontNameCombo->addItem(tr("(Default)"));
    _fontNameCombo->addItems(families);
    fontLayout->addWidget(_fontNameCombo, 0, 1, 1, 2);

    // Font size
    fontLayout->addWidget(new QLabel(tr("Font size:")), 1, 0);
    _fontSizeCombo = new QComboBox(dialog);
    _fontSizeCombo->addItem(tr("(Default)"));
    for (int size = 5; size <= 28; ++size) {
        _fontSizeCombo->addItem(QString::number(size));
    }
    fontLayout->addWidget(_fontSizeCombo, 1, 1);

    // Font style
    _boldCheck = new QCheckBox(tr("Bold"), dialog);
    _italicCheck = new QCheckBox(tr("Italic"), dialog);
    _underlineCheck = new QCheckBox(tr("Underline"), dialog);
    fontLayout->addWidget(_boldCheck, 2, 0);
    fontLayout->addWidget(_italicCheck, 2, 1);
    fontLayout->addWidget(_underlineCheck, 2, 2);

    mainLayout->addWidget(fontGroup);

    // Colors group
    auto* colorGroup = new QGroupBox(tr("Colors"), dialog);
    auto* colorLayout = new QGridLayout(colorGroup);

    // Foreground color
    colorLayout->addWidget(new QLabel(tr("Foreground:")), 0, 0);
    _fgColorBtn = new QPushButton(dialog);
    _fgColorBtn->setFixedSize(60, 25);
    colorLayout->addWidget(_fgColorBtn, 0, 1);
    _fgTransparentCheck = new QCheckBox(tr("Transparent"), dialog);
    colorLayout->addWidget(_fgTransparentCheck, 0, 2);

    // Background color
    colorLayout->addWidget(new QLabel(tr("Background:")), 1, 0);
    _bgColorBtn = new QPushButton(dialog);
    _bgColorBtn->setFixedSize(60, 25);
    colorLayout->addWidget(_bgColorBtn, 1, 1);
    _bgTransparentCheck = new QCheckBox(tr("Transparent"), dialog);
    colorLayout->addWidget(_bgTransparentCheck, 1, 2);

    mainLayout->addWidget(colorGroup);

    // Nesting group (if applicable)
    if (_enabledNesters != SCE_USER_MASK_NESTING_NONE) {
        auto* nestingGroup = new QGroupBox(tr("Nesting"), dialog);
        auto* nestingLayout = new QGridLayout(nestingGroup);

        int row = 0, col = 0;
        auto addNestingCheck = [&](const QString& text, int mask) {
            if (_enabledNesters & mask) {
                auto* check = new QCheckBox(text, dialog);
                _nestingChecks.push_back(check);
                nestingLayout->addWidget(check, row, col);
                if (++col >= 2) { col = 0; ++row; }
            }
        };

        addNestingCheck(tr("Delimiter 1"), SCE_USER_MASK_NESTING_DELIMITER1);
        addNestingCheck(tr("Delimiter 2"), SCE_USER_MASK_NESTING_DELIMITER2);
        addNestingCheck(tr("Delimiter 3"), SCE_USER_MASK_NESTING_DELIMITER3);
        addNestingCheck(tr("Delimiter 4"), SCE_USER_MASK_NESTING_DELIMITER4);
        addNestingCheck(tr("Delimiter 5"), SCE_USER_MASK_NESTING_DELIMITER5);
        addNestingCheck(tr("Delimiter 6"), SCE_USER_MASK_NESTING_DELIMITER6);
        addNestingCheck(tr("Delimiter 7"), SCE_USER_MASK_NESTING_DELIMITER7);
        addNestingCheck(tr("Delimiter 8"), SCE_USER_MASK_NESTING_DELIMITER8);
        addNestingCheck(tr("Comments"), SCE_USER_MASK_NESTING_COMMENT);
        addNestingCheck(tr("Line Comments"), SCE_USER_MASK_NESTING_COMMENT_LINE);

        mainLayout->addWidget(nestingGroup);
    }

    // Preview
    auto* previewGroup = new QGroupBox(tr("Preview"), dialog);
    auto* previewLayout = new QVBoxLayout(previewGroup);
    _previewLabel = new QLabel(tr("Sample Text"), dialog);
    _previewLabel->setAlignment(Qt::AlignCenter);
    _previewLabel->setMinimumHeight(60);
    previewLayout->addWidget(_previewLabel);
    mainLayout->addWidget(previewGroup);

    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    auto* okBtn = new QPushButton(tr("OK"), dialog);
    auto* cancelBtn = new QPushButton(tr("Cancel"), dialog);
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(_fgColorBtn, &QPushButton::clicked, this, &StyleDialog::onFgColorClicked);
    connect(_bgColorBtn, &QPushButton::clicked, this, &StyleDialog::onBgColorClicked);
    connect(_fgTransparentCheck, &QCheckBox::toggled, this, &StyleDialog::onFgTransparentToggled);
    connect(_bgTransparentCheck, &QCheckBox::toggled, this, &StyleDialog::onBgTransparentToggled);
    connect(_fontSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StyleDialog::onFontSizeChanged);
    connect(_fontNameCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StyleDialog::onFontNameChanged);
    connect(_boldCheck, &QCheckBox::toggled, this, &StyleDialog::onBoldToggled);
    connect(_italicCheck, &QCheckBox::toggled, this, &StyleDialog::onItalicToggled);
    connect(_underlineCheck, &QCheckBox::toggled, this, &StyleDialog::onUnderlineToggled);
    connect(okBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);

    dialog->resize(400, 500);
}

void StyleDialog::setStyleData(const QString& name, const QColor& fgColor,
                               const QColor& bgColor, int fontStyle, int fontSize,
                               const QString& fontName, int colorStyle, int nesting)
{
    _fgColor = fgColor;
    _bgColor = bgColor;
    _fontStyle = fontStyle;
    _fontSize = fontSize;
    _fontName = fontName;
    _colorStyle = colorStyle;
    _nesting = nesting;

    // Update UI
    _fgColorBtn->setStyleSheet(QString("background-color: %1").arg(fgColor.name()));
    _bgColorBtn->setStyleSheet(QString("background-color: %1").arg(bgColor.name()));

    _fgTransparentCheck->setChecked(!(colorStyle & COLORSTYLE_FOREGROUND));
    _bgTransparentCheck->setChecked(!(colorStyle & COLORSTYLE_BACKGROUND));

    _boldCheck->setChecked(fontStyle & FONTSTYLE_BOLD);
    _italicCheck->setChecked(fontStyle & FONTSTYLE_ITALIC);
    _underlineCheck->setChecked(fontStyle & FONTSTYLE_UNDERLINE);

    if (fontSize > 0) {
        int idx = _fontSizeCombo->findText(QString::number(fontSize));
        if (idx >= 0) _fontSizeCombo->setCurrentIndex(idx);
    } else {
        _fontSizeCombo->setCurrentIndex(0);
    }

    if (!fontName.isEmpty()) {
        int idx = _fontNameCombo->findText(fontName);
        if (idx >= 0) _fontNameCombo->setCurrentIndex(idx);
    } else {
        _fontNameCombo->setCurrentIndex(0);
    }

    updatePreview();
}

void StyleDialog::getStyleData(QColor& fgColor, QColor& bgColor, int& fontStyle,
                               int& fontSize, QString& fontName, int& colorStyle,
                               int& nesting) const
{
    fgColor = _fgColor;
    bgColor = _bgColor;
    fontStyle = _fontStyle;
    fontSize = _fontSize;
    fontName = _fontName;
    colorStyle = _colorStyle;
    nesting = _nesting;
}

int StyleDialog::exec()
{
    QDialog* dialog = getDialog();
    if (!dialog) return QDialog::Rejected;
    return dialog->exec();
}

void StyleDialog::onFgColorClicked()
{
    QColor color = QColorDialog::getColor(_fgColor, this, tr("Select Foreground Color"));
    if (color.isValid()) {
        _fgColor = color;
        _fgColorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
        _colorStyle |= COLORSTYLE_FOREGROUND;
        updatePreview();
    }
}

void StyleDialog::onBgColorClicked()
{
    QColor color = QColorDialog::getColor(_bgColor, this, tr("Select Background Color"));
    if (color.isValid()) {
        _bgColor = color;
        _bgColorBtn->setStyleSheet(QString("background-color: %1").arg(color.name()));
        _colorStyle |= COLORSTYLE_BACKGROUND;
        updatePreview();
    }
}

void StyleDialog::onFgTransparentToggled(bool checked)
{
    _fgColorBtn->setEnabled(!checked);
    if (checked) {
        _colorStyle &= ~COLORSTYLE_FOREGROUND;
    } else {
        _colorStyle |= COLORSTYLE_FOREGROUND;
    }
    updatePreview();
}

void StyleDialog::onBgTransparentToggled(bool checked)
{
    _bgColorBtn->setEnabled(!checked);
    if (checked) {
        _colorStyle &= ~COLORSTYLE_BACKGROUND;
    } else {
        _colorStyle |= COLORSTYLE_BACKGROUND;
    }
    updatePreview();
}

void StyleDialog::onFontSizeChanged(int index)
{
    if (index <= 0) {
        _fontSize = -1;
    } else {
        _fontSize = _fontSizeCombo->currentText().toInt();
    }
    updatePreview();
}

void StyleDialog::onFontNameChanged(int index)
{
    if (index <= 0) {
        _fontName.clear();
    } else {
        _fontName = _fontNameCombo->currentText();
    }
    updatePreview();
}

void StyleDialog::onBoldToggled(bool checked)
{
    if (checked) {
        _fontStyle |= FONTSTYLE_BOLD;
    } else {
        _fontStyle &= ~FONTSTYLE_BOLD;
    }
    updatePreview();
}

void StyleDialog::onItalicToggled(bool checked)
{
    if (checked) {
        _fontStyle |= FONTSTYLE_ITALIC;
    } else {
        _fontStyle &= ~FONTSTYLE_ITALIC;
    }
    updatePreview();
}

void StyleDialog::onUnderlineToggled(bool checked)
{
    if (checked) {
        _fontStyle |= FONTSTYLE_UNDERLINE;
    } else {
        _fontStyle &= ~FONTSTYLE_UNDERLINE;
    }
    updatePreview();
}

void StyleDialog::onNestingChanged()
{
    _nesting = SCE_USER_MASK_NESTING_NONE;
    // Update based on checkbox states
}

void StyleDialog::updatePreview()
{
    QFont font = _previewLabel->font();
    font.setBold(_fontStyle & FONTSTYLE_BOLD);
    font.setItalic(_fontStyle & FONTSTYLE_ITALIC);
    font.setUnderline(_fontStyle & FONTSTYLE_UNDERLINE);
    if (!_fontName.isEmpty()) {
        font.setFamily(_fontName);
    }
    if (_fontSize > 0) {
        font.setPointSize(_fontSize);
    }
    _previewLabel->setFont(font);

    QString styleSheet;
    if (_colorStyle & COLORSTYLE_FOREGROUND) {
        styleSheet += QString("color: %1; ").arg(_fgColor.name());
    }
    if (_colorStyle & COLORSTYLE_BACKGROUND) {
        styleSheet += QString("background-color: %1; ").arg(_bgColor.name());
    }
    _previewLabel->setStyleSheet(styleSheet);
}

// ============================================================================
// StringDialog Implementation
// ============================================================================
StringDialog::StringDialog(const QString& title, const QString& label,
                           const QString& initialText, int maxLength,
                           const QString& restrictedChars, QWidget* parent)
    : StaticDialog(parent), _title(title), _label(label), _initialText(initialText),
      _maxLength(maxLength), _restrictedChars(restrictedChars)
{
    setupUI();
}

StringDialog::~StringDialog() = default;

void StringDialog::setupUI()
{
    create(_title, false);
    QDialog* dialog = getDialog();
    if (!dialog) return;

    auto* layout = new QVBoxLayout(dialog);
    layout->setSpacing(10);
    layout->setContentsMargins(15, 15, 15, 15);

    layout->addWidget(new QLabel(_label, dialog));

    _lineEdit = new QLineEdit(_initialText, dialog);
    if (_maxLength > 0) {
        _lineEdit->setMaxLength(_maxLength);
    }
    layout->addWidget(_lineEdit);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _okButton = new QPushButton(tr("OK"), dialog);
    auto* cancelBtn = new QPushButton(tr("Cancel"), dialog);
    buttonLayout->addWidget(_okButton);
    buttonLayout->addWidget(cancelBtn);

    layout->addLayout(buttonLayout);

    connect(_lineEdit, &QLineEdit::textChanged, this, &StringDialog::onTextChanged);
    connect(_okButton, &QPushButton::clicked, dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);

    dialog->resize(300, 120);
}

QString StringDialog::getText() const
{
    return _lineEdit ? _lineEdit->text() : QString();
}

int StringDialog::exec()
{
    QDialog* dialog = getDialog();
    if (!dialog) return QDialog::Rejected;
    return dialog->exec();
}

void StringDialog::onTextChanged(const QString& text)
{
    if (_okButton) {
        _okButton->setEnabled(isAllowed(text));
    }
}

bool StringDialog::isAllowed(const QString& text) const
{
    if (_restrictedChars.isEmpty()) return true;
    for (const QChar& c : text) {
        if (_restrictedChars.contains(c)) return false;
    }
    return true;
}

// ============================================================================
// UserDefineDialog Implementation
// ============================================================================
UserDefineDialog::UserDefineDialog(QWidget* parent)
    : StaticDialog(parent)
    , _pCurrentUserLang(std::make_unique<QtControls::QtUserLangContainer>())
{
    // Initialize keyword list pointers
    for (int i = 0; i < 8; ++i) {
        _keywordLists[i] = nullptr;
    }
    for (int i = 0; i < 8; ++i) {
        _delimiterOpenEdits[i] = nullptr;
        _delimiterCloseEdits[i] = nullptr;
        _delimiterEscapeEdits[i] = nullptr;
    }

    // Initialize current language with default styles
    for (int i = 0; i < SCE_USER_STYLE_TOTAL_STYLES; ++i) {
        QString name = QString::fromStdWString(
            GlobalMappers::instance().styleNameMapper[i]);
        _pCurrentUserLang->addStyler(i, name);
    }
}

UserDefineDialog::~UserDefineDialog() = default;

void UserDefineDialog::init(ScintillaEditView** ppEditView)
{
    _ppEditView = ppEditView;
    _pUserLang = _pCurrentUserLang.get();

    setupUI();
    connectSignals();
    reloadLangCombo();
}

void UserDefineDialog::setupUI()
{
    QString title = tr("User Defined Language v.%1.%2.%3.%4")
        .arg(SCE_UDL_VERSION_MAJOR)
        .arg(SCE_UDL_VERSION_MINOR)
        .arg(SCE_UDL_VERSION_BUILD)
        .arg(SCE_UDL_VERSION_REVISION);

    create(title, false);
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setMinimumSize(700, 600);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Top section - Language selection and actions
    auto* topLayout = new QHBoxLayout();

    topLayout->addWidget(new QLabel(tr("Language:"), dialog));
    _langCombo = new QComboBox(dialog);
    _langCombo->setMinimumWidth(200);
    topLayout->addWidget(_langCombo);

    topLayout->addSpacing(20);

    _newLangButton = new QPushButton(tr("Create New..."), dialog);
    topLayout->addWidget(_newLangButton);

    _saveAsButton = new QPushButton(tr("Save As..."), dialog);
    topLayout->addWidget(_saveAsButton);

    _renameLangButton = new QPushButton(tr("Rename"), dialog);
    topLayout->addWidget(_renameLangButton);

    _removeLangButton = new QPushButton(tr("Remove"), dialog);
    topLayout->addWidget(_removeLangButton);

    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    // Extension and options
    auto* extLayout = new QHBoxLayout();
    extLayout->addWidget(new QLabel(tr("Ext.:"), dialog));
    _extEdit = new QLineEdit(dialog);
    _extEdit->setMaximumWidth(150);
    _extEdit->setToolTip(tr("File extensions separated by space (e.g. txt log)"));
    extLayout->addWidget(_extEdit);

    _ignoreCaseCheck = new QCheckBox(tr("Ignore case"), dialog);
    extLayout->addWidget(_ignoreCaseCheck);

    extLayout->addStretch();

    mainLayout->addLayout(extLayout);

    // Tab widget for different sections
    _mainTabs = new QTabWidget(dialog);
    mainLayout->addWidget(_mainTabs, 1);

    // Create tabs
    createKeywordsTab();
    createCommentsTab();
    createNumbersTab();
    createOperatorsTab();
    createDelimitersTab();
    createFolderTab();

    // Import/Export buttons
    auto* importExportLayout = new QHBoxLayout();
    _importButton = new QPushButton(tr("Import..."), dialog);
    _exportButton = new QPushButton(tr("Export..."), dialog);
    importExportLayout->addWidget(_importButton);
    importExportLayout->addWidget(_exportButton);
    importExportLayout->addStretch();

    _dockButton = new QPushButton(tr("Dock"), dialog);
    importExportLayout->addWidget(_dockButton);

    _closeButton = new QPushButton(tr("Close"), dialog);
    importExportLayout->addWidget(_closeButton);

    mainLayout->addLayout(importExportLayout);
}

void UserDefineDialog::createKeywordsTab()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    auto* scrollArea = new QScrollArea(dialog);
    auto* container = new QWidget(scrollArea);
    auto* layout = new QGridLayout(container);
    layout->setSpacing(10);
    layout->setContentsMargins(15, 15, 15, 15);

    QStringList groupNames = {
        tr("Group 1"), tr("Group 2"), tr("Group 3"), tr("Group 4"),
        tr("Group 5"), tr("Group 6"), tr("Group 7"), tr("Group 8")
    };

    for (int i = 0; i < 8; ++i) {
        int row = i / 2;
        int col = (i % 2) * 2;

        auto* groupBox = new QGroupBox(groupNames[i], container);
        auto* groupLayout = new QVBoxLayout(groupBox);

        _keywordLists[i] = new QPlainTextEdit(groupBox);
        _keywordLists[i]->setMaximumBlockCount(1);
        _keywordLists[i]->setPlaceholderText(tr("Enter keywords separated by spaces"));
        groupLayout->addWidget(_keywordLists[i]);

        auto* optionsLayout = new QHBoxLayout();
        _prefixChecks[i] = new QCheckBox(tr("Prefix mode"), groupBox);
        optionsLayout->addWidget(_prefixChecks[i]);

        optionsLayout->addStretch();

        _keywordStyleBtns[i] = new QPushButton(tr("Styler"), groupBox);
        _keywordStyleBtns[i]->setProperty("styleIndex", SCE_USER_STYLE_KEYWORD1 + i);
        optionsLayout->addWidget(_keywordStyleBtns[i]);

        groupLayout->addLayout(optionsLayout);

        layout->addWidget(groupBox, row, col, 1, 2);
    }

    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    layout->setRowStretch(4, 1);

    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);

    _mainTabs->addTab(scrollArea, tr("Keywords"));
}

void UserDefineDialog::createCommentsTab()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    auto* container = new QWidget(dialog);
    auto* layout = new QVBoxLayout(container);
    layout->setSpacing(15);
    layout->setContentsMargins(15, 15, 15, 15);

    // Line comment group
    auto* lineCommentGroup = new QGroupBox(tr("Line Comment"), container);
    auto* lineLayout = new QGridLayout(lineCommentGroup);

    lineLayout->addWidget(new QLabel(tr("Open:"), lineCommentGroup), 0, 0);
    _commentLineOpenEdit = new QLineEdit(lineCommentGroup);
    lineLayout->addWidget(_commentLineOpenEdit, 0, 1);

    lineLayout->addWidget(new QLabel(tr("Continue:"), lineCommentGroup), 1, 0);
    _commentLineContinueEdit = new QLineEdit(lineCommentGroup);
    lineLayout->addWidget(_commentLineContinueEdit, 1, 1);

    lineLayout->addWidget(new QLabel(tr("Close:"), lineCommentGroup), 2, 0);
    _commentLineCloseEdit = new QLineEdit(lineCommentGroup);
    lineLayout->addWidget(_commentLineCloseEdit, 2, 1);

    auto* lineStyleLayout = new QHBoxLayout();
    lineStyleLayout->addStretch();
    _commentLineStyleBtn = new QPushButton(tr("Styler"), lineCommentGroup);
    _commentLineStyleBtn->setProperty("styleIndex", SCE_USER_STYLE_COMMENTLINE);
    lineStyleLayout->addWidget(_commentLineStyleBtn);
    lineLayout->addLayout(lineStyleLayout, 3, 0, 1, 2);

    layout->addWidget(lineCommentGroup);

    // Block comment group
    auto* commentGroup = new QGroupBox(tr("Comment"), container);
    auto* commentLayout = new QGridLayout(commentGroup);

    commentLayout->addWidget(new QLabel(tr("Open:"), commentGroup), 0, 0);
    _commentOpenEdit = new QLineEdit(commentGroup);
    commentLayout->addWidget(_commentOpenEdit, 0, 1);

    commentLayout->addWidget(new QLabel(tr("Close:"), commentGroup), 1, 0);
    _commentCloseEdit = new QLineEdit(commentGroup);
    commentLayout->addWidget(_commentCloseEdit, 1, 1);

    auto* commentStyleLayout = new QHBoxLayout();
    commentStyleLayout->addStretch();
    _commentStyleBtn = new QPushButton(tr("Styler"), commentGroup);
    _commentStyleBtn->setProperty("styleIndex", SCE_USER_STYLE_COMMENT);
    commentStyleLayout->addWidget(_commentStyleBtn);
    commentLayout->addLayout(commentStyleLayout, 2, 0, 1, 2);

    layout->addWidget(commentGroup);

    // Comment options
    auto* optionsGroup = new QGroupBox(tr("Comment Options"), container);
    auto* optionsLayout = new QVBoxLayout(optionsGroup);

    _allowFoldCommentsCheck = new QCheckBox(tr("Allow folding of comments"), optionsGroup);
    optionsLayout->addWidget(_allowFoldCommentsCheck);

    auto* forceLCGroup = new QGroupBox(tr("Force pure line comments at beginning of line"), optionsGroup);
    auto* forceLCLayout = new QVBoxLayout(forceLCGroup);

    _forcePureLCGroup = new QButtonGroup(this);
    _allowAnywhereRadio = new QRadioButton(tr("Allow anywhere"), forceLCGroup);
    _forceAtBOLRadio = new QRadioButton(tr("Force at beginning of line"), forceLCGroup);
    _allowWhitespaceRadio = new QRadioButton(tr("Allow preceeding whitespace"), forceLCGroup);

    _forcePureLCGroup->addButton(_allowAnywhereRadio, PURE_LC_NONE);
    _forcePureLCGroup->addButton(_forceAtBOLRadio, PURE_LC_BOL);
    _forcePureLCGroup->addButton(_allowWhitespaceRadio, PURE_LC_WSP);

    forceLCLayout->addWidget(_allowAnywhereRadio);
    forceLCLayout->addWidget(_forceAtBOLRadio);
    forceLCLayout->addWidget(_allowWhitespaceRadio);

    optionsLayout->addWidget(forceLCGroup);
    layout->addWidget(optionsGroup);

    layout->addStretch();

    _mainTabs->addTab(container, tr("Comments"));
}

void UserDefineDialog::createNumbersTab()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    auto* container = new QWidget(dialog);
    auto* layout = new QGridLayout(container);
    layout->setSpacing(10);
    layout->setContentsMargins(15, 15, 15, 15);

    // Number prefixes
    layout->addWidget(new QLabel(tr("Prefix 1:"), container), 0, 0);
    _numberPrefix1Edit = new QLineEdit(container);
    layout->addWidget(_numberPrefix1Edit, 0, 1);

    layout->addWidget(new QLabel(tr("Prefix 2:"), container), 1, 0);
    _numberPrefix2Edit = new QLineEdit(container);
    layout->addWidget(_numberPrefix2Edit, 1, 1);

    // Number extras
    layout->addWidget(new QLabel(tr("Extras 1:"), container), 2, 0);
    _numberExtras1Edit = new QLineEdit(container);
    layout->addWidget(_numberExtras1Edit, 2, 1);

    layout->addWidget(new QLabel(tr("Extras 2:"), container), 3, 0);
    _numberExtras2Edit = new QLineEdit(container);
    layout->addWidget(_numberExtras2Edit, 3, 1);

    // Number suffixes
    layout->addWidget(new QLabel(tr("Suffix 1:"), container), 4, 0);
    _numberSuffix1Edit = new QLineEdit(container);
    layout->addWidget(_numberSuffix1Edit, 4, 1);

    layout->addWidget(new QLabel(tr("Suffix 2:"), container), 5, 0);
    _numberSuffix2Edit = new QLineEdit(container);
    layout->addWidget(_numberSuffix2Edit, 5, 1);

    // Range
    layout->addWidget(new QLabel(tr("Range:"), container), 6, 0);
    _numberRangeEdit = new QLineEdit(container);
    _numberRangeEdit->setPlaceholderText(tr("Character for range (e.g. . for 1..10)"));
    layout->addWidget(_numberRangeEdit, 6, 1);

    // Decimal separator
    auto* sepGroup = new QGroupBox(tr("Decimal Separator"), container);
    auto* sepLayout = new QHBoxLayout(sepGroup);

    _decimalSepGroup = new QButtonGroup(this);
    _dotRadio = new QRadioButton(tr("Dot (.)"), sepGroup);
    _commaRadio = new QRadioButton(tr("Comma (,)"), sepGroup);
    _bothRadio = new QRadioButton(tr("Both"), sepGroup);

    _decimalSepGroup->addButton(_dotRadio, DECSEP_DOT);
    _decimalSepGroup->addButton(_commaRadio, DECSEP_COMMA);
    _decimalSepGroup->addButton(_bothRadio, DECSEP_BOTH);

    sepLayout->addWidget(_dotRadio);
    sepLayout->addWidget(_commaRadio);
    sepLayout->addWidget(_bothRadio);
    sepLayout->addStretch();

    layout->addWidget(sepGroup, 7, 0, 1, 2);

    // Styler button
    auto* styleLayout = new QHBoxLayout();
    styleLayout->addStretch();
    _numberStyleBtn = new QPushButton(tr("Styler"), container);
    _numberStyleBtn->setProperty("styleIndex", SCE_USER_STYLE_NUMBER);
    styleLayout->addWidget(_numberStyleBtn);
    layout->addLayout(styleLayout, 8, 0, 1, 2);

    layout->setRowStretch(9, 1);
    layout->setColumnStretch(1, 1);

    _mainTabs->addTab(container, tr("Numbers"));
}

void UserDefineDialog::createOperatorsTab()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    auto* container = new QWidget(dialog);
    auto* layout = new QVBoxLayout(container);
    layout->setSpacing(15);
    layout->setContentsMargins(15, 15, 15, 15);

    auto* op1Group = new QGroupBox(tr("Operators 1"), container);
    auto* op1Layout = new QVBoxLayout(op1Group);
    _operators1Edit = new QLineEdit(op1Group);
    _operators1Edit->setPlaceholderText(tr("Enter operators separated by spaces (e.g. + - * /)"));
    op1Layout->addWidget(_operators1Edit);
    layout->addWidget(op1Group);

    auto* op2Group = new QGroupBox(tr("Operators 2"), container);
    auto* op2Layout = new QVBoxLayout(op2Group);
    _operators2Edit = new QLineEdit(op2Group);
    _operators2Edit->setPlaceholderText(tr("Enter operators separated by spaces"));
    op2Layout->addWidget(_operators2Edit);
    layout->addWidget(op2Group);

    auto* styleLayout = new QHBoxLayout();
    styleLayout->addStretch();
    _operatorStyleBtn = new QPushButton(tr("Styler"), container);
    _operatorStyleBtn->setProperty("styleIndex", SCE_USER_STYLE_OPERATOR);
    styleLayout->addWidget(_operatorStyleBtn);
    layout->addLayout(styleLayout);

    layout->addStretch();

    _mainTabs->addTab(container, tr("Operators"));
}

void UserDefineDialog::createDelimitersTab()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    auto* scrollArea = new QScrollArea(dialog);
    auto* container = new QWidget(scrollArea);
    auto* layout = new QGridLayout(container);
    layout->setSpacing(10);
    layout->setContentsMargins(15, 15, 15, 15);

    for (int i = 0; i < 8; ++i) {
        auto* groupBox = new QGroupBox(tr("Delimiter %1").arg(i + 1), container);
        auto* groupLayout = new QGridLayout(groupBox);

        groupLayout->addWidget(new QLabel(tr("Open:"), groupBox), 0, 0);
        _delimiterOpenEdits[i] = new QLineEdit(groupBox);
        groupLayout->addWidget(_delimiterOpenEdits[i], 0, 1);

        groupLayout->addWidget(new QLabel(tr("Close:"), groupBox), 1, 0);
        _delimiterCloseEdits[i] = new QLineEdit(groupBox);
        groupLayout->addWidget(_delimiterCloseEdits[i], 1, 1);

        groupLayout->addWidget(new QLabel(tr("Escape:"), groupBox), 2, 0);
        _delimiterEscapeEdits[i] = new QLineEdit(groupBox);
        groupLayout->addWidget(_delimiterEscapeEdits[i], 2, 1);

        auto* styleLayout = new QHBoxLayout();
        styleLayout->addStretch();
        _delimiterStyleBtns[i] = new QPushButton(tr("Styler"), groupBox);
        _delimiterStyleBtns[i]->setProperty("styleIndex", SCE_USER_STYLE_DELIMITER1 + i);
        styleLayout->addWidget(_delimiterStyleBtns[i]);
        groupLayout->addLayout(styleLayout, 3, 0, 1, 2);

        int row = i / 2;
        int col = i % 2;
        layout->addWidget(groupBox, row, col);
    }

    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);
    layout->setRowStretch(4, 1);

    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);

    _mainTabs->addTab(scrollArea, tr("Delimiters"));
}

void UserDefineDialog::createFolderTab()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    auto* container = new QWidget(dialog);
    auto* layout = new QVBoxLayout(container);
    layout->setSpacing(15);
    layout->setContentsMargins(15, 15, 15, 15);

    // Folder in code 1
    auto* code1Group = new QGroupBox(tr("Folders in Code 1"), container);
    auto* code1Layout = new QGridLayout(code1Group);

    code1Layout->addWidget(new QLabel(tr("Open:"), code1Group), 0, 0);
    _folderInCode1OpenEdit = new QLineEdit(code1Group);
    code1Layout->addWidget(_folderInCode1OpenEdit, 0, 1);

    code1Layout->addWidget(new QLabel(tr("Middle:"), code1Group), 1, 0);
    _folderInCode1MiddleEdit = new QLineEdit(code1Group);
    code1Layout->addWidget(_folderInCode1MiddleEdit, 1, 1);

    code1Layout->addWidget(new QLabel(tr("Close:"), code1Group), 2, 0);
    _folderInCode1CloseEdit = new QLineEdit(code1Group);
    code1Layout->addWidget(_folderInCode1CloseEdit, 2, 1);

    auto* code1StyleLayout = new QHBoxLayout();
    code1StyleLayout->addStretch();
    _folderInCode1StyleBtn = new QPushButton(tr("Styler"), code1Group);
    _folderInCode1StyleBtn->setProperty("styleIndex", SCE_USER_STYLE_FOLDER_IN_CODE1);
    code1StyleLayout->addWidget(_folderInCode1StyleBtn);
    code1Layout->addLayout(code1StyleLayout, 3, 0, 1, 2);

    layout->addWidget(code1Group);

    // Folder in code 2
    auto* code2Group = new QGroupBox(tr("Folders in Code 2"), container);
    auto* code2Layout = new QGridLayout(code2Group);

    code2Layout->addWidget(new QLabel(tr("Open:"), code2Group), 0, 0);
    _folderInCode2OpenEdit = new QLineEdit(code2Group);
    code2Layout->addWidget(_folderInCode2OpenEdit, 0, 1);

    code2Layout->addWidget(new QLabel(tr("Middle:"), code2Group), 1, 0);
    _folderInCode2MiddleEdit = new QLineEdit(code2Group);
    code2Layout->addWidget(_folderInCode2MiddleEdit, 1, 1);

    code2Layout->addWidget(new QLabel(tr("Close:"), code2Group), 2, 0);
    _folderInCode2CloseEdit = new QLineEdit(code2Group);
    code2Layout->addWidget(_folderInCode2CloseEdit, 2, 1);

    auto* code2StyleLayout = new QHBoxLayout();
    code2StyleLayout->addStretch();
    _folderInCode2StyleBtn = new QPushButton(tr("Styler"), code2Group);
    _folderInCode2StyleBtn->setProperty("styleIndex", SCE_USER_STYLE_FOLDER_IN_CODE2);
    code2StyleLayout->addWidget(_folderInCode2StyleBtn);
    code2Layout->addLayout(code2StyleLayout, 3, 0, 1, 2);

    layout->addWidget(code2Group);

    // Folder in comment
    auto* commentGroup = new QGroupBox(tr("Folders in Comment"), container);
    auto* commentLayout = new QGridLayout(commentGroup);

    commentLayout->addWidget(new QLabel(tr("Open:"), commentGroup), 0, 0);
    _folderInCommentOpenEdit = new QLineEdit(commentGroup);
    commentLayout->addWidget(_folderInCommentOpenEdit, 0, 1);

    commentLayout->addWidget(new QLabel(tr("Middle:"), commentGroup), 1, 0);
    _folderInCommentMiddleEdit = new QLineEdit(commentGroup);
    commentLayout->addWidget(_folderInCommentMiddleEdit, 1, 1);

    commentLayout->addWidget(new QLabel(tr("Close:"), commentGroup), 2, 0);
    _folderInCommentCloseEdit = new QLineEdit(commentGroup);
    commentLayout->addWidget(_folderInCommentCloseEdit, 2, 1);

    auto* commentStyleLayout = new QHBoxLayout();
    commentStyleLayout->addStretch();
    _folderInCommentStyleBtn = new QPushButton(tr("Styler"), commentGroup);
    _folderInCommentStyleBtn->setProperty("styleIndex", SCE_USER_STYLE_FOLDER_IN_COMMENT);
    commentStyleLayout->addWidget(_folderInCommentStyleBtn);
    commentLayout->addLayout(commentStyleLayout, 3, 0, 1, 2);

    layout->addWidget(commentGroup);

    // Options
    auto* optionsGroup = new QGroupBox(tr("Folder Options"), container);
    auto* optionsLayout = new QVBoxLayout(optionsGroup);

    _foldCompactCheck = new QCheckBox(tr("Compact folding"), optionsGroup);
    optionsLayout->addWidget(_foldCompactCheck);

    auto* defaultStyleLayout = new QHBoxLayout();
    defaultStyleLayout->addStretch();
    _defaultStyleBtn = new QPushButton(tr("Default Styler"), optionsGroup);
    _defaultStyleBtn->setProperty("styleIndex", SCE_USER_STYLE_DEFAULT);
    defaultStyleLayout->addWidget(_defaultStyleBtn);
    optionsLayout->addLayout(defaultStyleLayout);

    layout->addWidget(optionsGroup);
    layout->addStretch();

    _mainTabs->addTab(container, tr("Folder & Default"));
}

void UserDefineDialog::connectSignals()
{
    // Language selection
    connect(_langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &UserDefineDialog::onLanguageChanged);

    // Action buttons
    connect(_newLangButton, &QPushButton::clicked, this, &UserDefineDialog::onNewLangClicked);
    connect(_saveAsButton, &QPushButton::clicked, this, &UserDefineDialog::onSaveAsClicked);
    connect(_removeLangButton, &QPushButton::clicked, this, &UserDefineDialog::onRemoveLangClicked);
    connect(_renameLangButton, &QPushButton::clicked, this, &UserDefineDialog::onRenameLangClicked);
    connect(_importButton, &QPushButton::clicked, this, &UserDefineDialog::onImportClicked);
    connect(_exportButton, &QPushButton::clicked, this, &UserDefineDialog::onExportClicked);
    connect(_dockButton, &QPushButton::clicked, this, &UserDefineDialog::onDockClicked);
    connect(_closeButton, &QPushButton::clicked, this, &UserDefineDialog::onCloseClicked);

    // Extension
    connect(_extEdit, &QLineEdit::textChanged, this, &UserDefineDialog::onExtChanged);

    // Options
    connect(_ignoreCaseCheck, &QCheckBox::toggled, this, &UserDefineDialog::onIgnoreCaseToggled);
    connect(_foldCompactCheck, &QCheckBox::toggled, this, &UserDefineDialog::onFoldCompactToggled);
    connect(_allowFoldCommentsCheck, &QCheckBox::toggled,
            this, &UserDefineDialog::onAllowFoldCommentsToggled);

    // Radio groups
    connect(_forcePureLCGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, [this](int) { onForcePureLCChanged(); });
    connect(_decimalSepGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, [this](int) { onDecimalSeparatorChanged(); });

    // Style buttons
    auto styleButtons = findChildren<QPushButton*>();
    for (auto* btn : styleButtons) {
        if (btn->property("styleIndex").isValid()) {
            connect(btn, &QPushButton::clicked, this, [this, btn]() {
                int styleIndex = btn->property("styleIndex").toInt();
                onStyleClicked(styleIndex);
            });
        }
    }

    // Keyword text changes
    for (int i = 0; i < 8; ++i) {
        if (_keywordLists[i]) {
            connect(_keywordLists[i], &QPlainTextEdit::textChanged,
                    this, &UserDefineDialog::onKeywordsChanged);
        }
    }

    // Prefix toggles
    for (int i = 0; i < 8; ++i) {
        if (_prefixChecks[i]) {
            connect(_prefixChecks[i], &QCheckBox::toggled, this, [this, i]() {
                onPrefixToggled(i);
            });
        }
    }

    // Comment edits - connect to save language when changed
    auto connectCommentEdit = [this](QLineEdit* edit) {
        if (edit) {
            connect(edit, &QLineEdit::textChanged, this, [this]() {
                saveLanguage();
                updatePreview();
            });
        }
    };
    connectCommentEdit(_commentLineOpenEdit);
    connectCommentEdit(_commentLineContinueEdit);
    connectCommentEdit(_commentLineCloseEdit);
    connectCommentEdit(_commentOpenEdit);
    connectCommentEdit(_commentCloseEdit);

    // Number edits
    auto connectNumberEdit = [this](QLineEdit* edit) {
        if (edit) {
            connect(edit, &QLineEdit::textChanged, this, [this]() {
                if (_pUserLang) {
                    _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_PREFIX1][0] = L'\0';
                    _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_PREFIX2][0] = L'\0';
                    _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_EXTRAS1][0] = L'\0';
                    _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_EXTRAS2][0] = L'\0';
                    _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_SUFFIX1][0] = L'\0';
                    _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_SUFFIX2][0] = L'\0';
                    _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_RANGE][0] = L'\0';

                    if (_numberPrefix1Edit) wcsncpy(_pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_PREFIX1], _numberPrefix1Edit->text().toStdWString().c_str(), 1023);
                    if (_numberPrefix2Edit) wcsncpy(_pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_PREFIX2], _numberPrefix2Edit->text().toStdWString().c_str(), 1023);
                    if (_numberExtras1Edit) wcsncpy(_pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_EXTRAS1], _numberExtras1Edit->text().toStdWString().c_str(), 1023);
                    if (_numberExtras2Edit) wcsncpy(_pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_EXTRAS2], _numberExtras2Edit->text().toStdWString().c_str(), 1023);
                    if (_numberSuffix1Edit) wcsncpy(_pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_SUFFIX1], _numberSuffix1Edit->text().toStdWString().c_str(), 1023);
                    if (_numberSuffix2Edit) wcsncpy(_pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_SUFFIX2], _numberSuffix2Edit->text().toStdWString().c_str(), 1023);
                    if (_numberRangeEdit) wcsncpy(_pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_RANGE], _numberRangeEdit->text().toStdWString().c_str(), 1023);
                }
                updatePreview();
            });
        }
    };
    connectNumberEdit(_numberPrefix1Edit);
    connectNumberEdit(_numberPrefix2Edit);
    connectNumberEdit(_numberExtras1Edit);
    connectNumberEdit(_numberExtras2Edit);
    connectNumberEdit(_numberSuffix1Edit);
    connectNumberEdit(_numberSuffix2Edit);
    connectNumberEdit(_numberRangeEdit);

    // Operator edits
    auto connectOperatorEdit = [this](QLineEdit* edit, int keywordIndex) {
        if (edit) {
            connect(edit, &QLineEdit::textChanged, this, [this, edit, keywordIndex]() {
                if (_pUserLang) {
                    wcsncpy(_pUserLang->_keywordLists[keywordIndex], edit->text().toStdWString().c_str(), 1023);
                    _pUserLang->_keywordLists[keywordIndex][1023] = L'\0';
                }
                updatePreview();
            });
        }
    };
    connectOperatorEdit(_operators1Edit, SCE_USER_KWLIST_OPERATORS1);
    connectOperatorEdit(_operators2Edit, SCE_USER_KWLIST_OPERATORS2);

    // Folder edits
    auto connectFolderEdit = [this](QLineEdit* edit, int keywordIndex) {
        if (edit) {
            connect(edit, &QLineEdit::textChanged, this, [this, edit, keywordIndex]() {
                if (_pUserLang) {
                    wcsncpy(_pUserLang->_keywordLists[keywordIndex], edit->text().toStdWString().c_str(), 1023);
                    _pUserLang->_keywordLists[keywordIndex][1023] = L'\0';
                }
                updatePreview();
            });
        }
    };
    connectFolderEdit(_folderInCode1OpenEdit, SCE_USER_KWLIST_FOLDERS_IN_CODE1_OPEN);
    connectFolderEdit(_folderInCode1MiddleEdit, SCE_USER_KWLIST_FOLDERS_IN_CODE1_MIDDLE);
    connectFolderEdit(_folderInCode1CloseEdit, SCE_USER_KWLIST_FOLDERS_IN_CODE1_CLOSE);
    connectFolderEdit(_folderInCode2OpenEdit, SCE_USER_KWLIST_FOLDERS_IN_CODE2_OPEN);
    connectFolderEdit(_folderInCode2MiddleEdit, SCE_USER_KWLIST_FOLDERS_IN_CODE2_MIDDLE);
    connectFolderEdit(_folderInCode2CloseEdit, SCE_USER_KWLIST_FOLDERS_IN_CODE2_CLOSE);
    connectFolderEdit(_folderInCommentOpenEdit, SCE_USER_KWLIST_FOLDERS_IN_COMMENT_OPEN);
    connectFolderEdit(_folderInCommentMiddleEdit, SCE_USER_KWLIST_FOLDERS_IN_COMMENT_MIDDLE);
    connectFolderEdit(_folderInCommentCloseEdit, SCE_USER_KWLIST_FOLDERS_IN_COMMENT_CLOSE);

    // Delimiter edits - save to keyword list with group syntax
    for (int i = 0; i < 8; ++i) {
        if (_delimiterOpenEdits[i]) {
            connect(_delimiterOpenEdits[i], &QLineEdit::textChanged, this, [this]() {
                saveLanguage();
                updatePreview();
            });
        }
        if (_delimiterEscapeEdits[i]) {
            connect(_delimiterEscapeEdits[i], &QLineEdit::textChanged, this, [this]() {
                saveLanguage();
                updatePreview();
            });
        }
        if (_delimiterCloseEdits[i]) {
            connect(_delimiterCloseEdits[i], &QLineEdit::textChanged, this, [this]() {
                saveLanguage();
                updatePreview();
            });
        }
    }

    // Tab changes
    connect(_mainTabs, &QTabWidget::currentChanged, this, &UserDefineDialog::onTabChanged);
}

void UserDefineDialog::doDialog()
{
    display(true, true);
}

void UserDefineDialog::setCurrentLanguage(const QString& langName)
{
    int index = _langCombo->findText(langName);
    if (index >= 0) {
        _langCombo->setCurrentIndex(index);
    }
}

void UserDefineDialog::reloadLangCombo()
{
    _langCombo->clear();
    _langCombo->addItem(tr("User Defined Language"));

    // Load from NppParameters
    NppParameters& nppParam = NppParameters::getInstance();
    int nbUserLang = nppParam.getNbUserLang();
    for (int i = 0; i < nbUserLang; ++i) {
        const UserLangContainer* userLang = nppParam.getULCFromIndex(i);
        if (userLang) {
            _langCombo->addItem(QString::fromStdWString(userLang->getName()));
        }
    }
}

void UserDefineDialog::changeStyle()
{
    _isDocked = !_isDocked;
    _dockButton->setText(_isDocked ? tr("Undock") : tr("Dock"));

    // TODO: Emit signal or call parent to handle docking
}

void UserDefineDialog::setTabName(int index, const wchar_t* name)
{
    if (_mainTabs && index >= 0 && index < _mainTabs->count()) {
        _mainTabs->setTabText(index, QString::fromWCharArray(name));
    }
}

void UserDefineDialog::onLanguageChanged(int index)
{
    enableLangAndControlsBy(index);
    loadLanguage(index);
}

void UserDefineDialog::onKeywordsChanged()
{
    // Update keyword lists in current language
    if (!_pUserLang) return;

    for (int i = 0; i < 8; ++i) {
        if (_keywordLists[i]) {
            QString text = _keywordLists[i]->toPlainText();
            // Convert to wchar_t array
            std::wstring wtext = text.toStdWString();
            wcsncpy(_pUserLang->_keywordLists[SCE_USER_KWLIST_KEYWORDS1 + i],
                    wtext.c_str(), 1023);
            _pUserLang->_keywordLists[SCE_USER_KWLIST_KEYWORDS1 + i][1023] = L'\0';
        }
    }

    updatePreview();
}

void UserDefineDialog::onStyleClicked(int styleIndex)
{
    if (!_pUserLang) return;

    auto* styler = _pUserLang->getStyler(styleIndex);
    if (!styler) return;

    StyleDialog dlg(styleIndex, SCE_USER_MASK_NESTING_NONE, this);
    dlg.setStyleData(QString::fromStdWString(
                         GlobalMappers::instance().styleNameMapper[styleIndex]),
                     styler->_fgColor, styler->_bgColor,
                     styler->_fontStyle, styler->_fontSize,
                     styler->_fontName, styler->_colorStyle,
                     styler->_nesting);

    if (dlg.exec() == QDialog::Accepted) {
        dlg.getStyleData(styler->_fgColor, styler->_bgColor,
                         styler->_fontStyle, styler->_fontSize,
                         styler->_fontName, styler->_colorStyle,
                         styler->_nesting);
        updatePreview();
    }
}

void UserDefineDialog::onImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Import User Defined Language"),
                                                    QString(),
                                                    tr("UDL Files (*.xml);;All Files (*)"));
    if (fileName.isEmpty()) return;

    NppParameters& nppParam = NppParameters::getInstance();
    std::wstring wFileName = fileName.toStdWString();

    bool isSuccessful = nppParam.importUDLFromFile(wFileName);
    if (isSuccessful) {
        int currentIndex = _langCombo->currentIndex();
        reloadLangCombo();
        _langCombo->setCurrentIndex(currentIndex);

        QMessageBox::information(this, tr("User Defined Language"),
                                 tr("Import successful."));
    } else {
        QMessageBox::warning(this, tr("User Defined Language"),
                             tr("Failed to import."));
    }
}

void UserDefineDialog::onExportClicked()
{
    int currentIndex = _langCombo->currentIndex();
    if (currentIndex <= 0) {
        QMessageBox::warning(this, tr("User Defined Language"),
                             tr("Before exporting, save your language definition by clicking \"Save As...\" button"));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export User Defined Language"),
                                                    QString(),
                                                    tr("UDL Files (*.xml);;All Files (*)"));
    if (fileName.isEmpty()) return;

    NppParameters& nppParam = NppParameters::getInstance();
    std::wstring wFileName = fileName.toStdWString();

    bool isSuccessful = nppParam.exportUDLToFile(currentIndex - 1, wFileName);
    if (isSuccessful) {
        QMessageBox::information(this, tr("User Defined Language"),
                                 tr("Export successful."));
    } else {
        QMessageBox::warning(this, tr("User Defined Language"),
                             tr("Failed to export."));
    }
}

void UserDefineDialog::onNewLangClicked()
{
    NppParameters& nppParam = NppParameters::getInstance();

    StringDialog dlg(tr("Create New Language"), tr("Name:"),
                     tr("new user define"), 64, QString(), this);
    if (dlg.exec() == QDialog::Accepted) {
        QString name = dlg.getText();
        if (!name.isEmpty()) {
            std::wstring wName = name.toStdWString();

            // Check for duplicate names
            if (nppParam.isExistingUserLangName(wName.c_str())) {
                QMessageBox::warning(this, tr("UDL Error"),
                                     tr("This name is used by another language,\nplease give another one."));
                return;
            }

            // Check max limit
            if (nppParam.getNbUserLang() >= NB_MAX_USER_LANG) {
                return;
            }

            // Add current language to NppParameters
            const UserLangContainer* userLang = _pCurrentUserLang.get();
            int newIndex = nppParam.addUserLangToEnd(userLang, wName.c_str());

            // Add to combobox
            _langCombo->addItem(name);
            _langCombo->setCurrentIndex(_langCombo->count() - 1);
        }
    }
}

void UserDefineDialog::onRemoveLangClicked()
{
    int currentIndex = _langCombo->currentIndex();
    if (currentIndex <= 0) return;

    QString langName = _langCombo->currentText();
    int result = QMessageBox::question(this, tr("Remove the current language"),
                                       tr("Are you sure?"),
                                       QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes) {
        NppParameters& nppParam = NppParameters::getInstance();

        // Remove from NppParameters
        nppParam.removeUserLang(currentIndex - 1);

        // Remove from combobox
        _langCombo->removeItem(currentIndex);
        _langCombo->setCurrentIndex(0);

        // Notify parent window to update menu
        // TODO: Emit signal to update language menu
    }
}

void UserDefineDialog::onRenameLangClicked()
{
    int currentIndex = _langCombo->currentIndex();
    if (currentIndex <= 0) return;

    QString currentName = _langCombo->currentText();
    StringDialog dlg(tr("Rename Current Language Name"), tr("Name:"),
                     currentName, 64, QString(), this);
    if (dlg.exec() == QDialog::Accepted) {
        QString newName = dlg.getText();
        if (!newName.isEmpty() && newName != currentName) {
            std::wstring wNewName = newName.toStdWString();
            NppParameters& nppParam = NppParameters::getInstance();

            // Check for duplicate names
            if (nppParam.isExistingUserLangName(wNewName.c_str())) {
                QMessageBox::warning(this, tr("UDL Error"),
                                     tr("This name is used by another language,\nplease give another one."));
                return;
            }

            // Rename in NppParameters
            UserLangContainer* userLangContainer = nppParam.getULCFromIndex(currentIndex - 1);
            if (userLangContainer) {
                userLangContainer->_name = wNewName;
            }

            // Update combobox
            _langCombo->setItemText(currentIndex, newName);

            // TODO: Update language menu
        }
    }
}

void UserDefineDialog::onSaveAsClicked()
{
    int currentIndex = _langCombo->currentIndex();
    QString currentName = (currentIndex > 0) ? _langCombo->currentText() : QString();

    StringDialog dlg(tr("Save Current Language Name As..."), tr("Name:"),
                     currentName, 64, QString(), this);
    if (dlg.exec() == QDialog::Accepted) {
        QString name = dlg.getText();
        if (!name.isEmpty()) {
            std::wstring wName = name.toStdWString();
            NppParameters& nppParam = NppParameters::getInstance();

            // Check for duplicate names
            if (nppParam.isExistingUserLangName(wName.c_str())) {
                QMessageBox::warning(this, tr("UDL Error"),
                                     tr("This name is used by another language,\nplease give another one."));
                return;
            }

            // Check max limit
            if (nppParam.getNbUserLang() >= NB_MAX_USER_LANG) {
                return;
            }

            // Add current language to NppParameters
            const UserLangContainer* userLang = (currentIndex > 0)
                ? nppParam.getULCFromIndex(currentIndex - 1)
                : _pCurrentUserLang.get();

            int newIndex = nppParam.addUserLangToEnd(userLang, wName.c_str());

            // Add to combobox
            _langCombo->addItem(name);
            _langCombo->setCurrentIndex(_langCombo->count() - 1);
        }
    }
}

void UserDefineDialog::onDockClicked()
{
    changeStyle();
}

void UserDefineDialog::onCloseClicked()
{
    display(false);
}

void UserDefineDialog::onExtChanged(const QString& text)
{
    if (_pUserLang) {
        _pUserLang->_ext = text;
    }
}

void UserDefineDialog::onIgnoreCaseToggled(bool checked)
{
    if (_pUserLang) {
        _pUserLang->_isCaseIgnored = checked;
    }
    updatePreview();
}

void UserDefineDialog::onFoldCompactToggled(bool checked)
{
    if (_pUserLang) {
        _pUserLang->_foldCompact = checked;
    }
}

void UserDefineDialog::onAllowFoldCommentsToggled(bool checked)
{
    if (_pUserLang) {
        _pUserLang->_allowFoldOfComments = checked;
    }
}

void UserDefineDialog::onForcePureLCChanged()
{
    if (_pUserLang && _forcePureLCGroup) {
        _pUserLang->_forcePureLC = _forcePureLCGroup->checkedId();
    }
    updatePreview();
}

void UserDefineDialog::onDecimalSeparatorChanged()
{
    if (_pUserLang && _decimalSepGroup) {
        _pUserLang->_decimalSeparator = _decimalSepGroup->checkedId();
    }
    updatePreview();
}

void UserDefineDialog::onPrefixToggled(int group)
{
    if (_pUserLang && group >= 0 && group < 8) {
        _pUserLang->_isPrefix[group] = _prefixChecks[group]->isChecked();
    }
    updatePreview();
}

void UserDefineDialog::onTabChanged(int /*index*/)
{
    // Handle tab change if needed
}

void UserDefineDialog::loadLanguage(int index)
{
    if (index == 0) {
        _pUserLang = _pCurrentUserLang.get();
    } else {
        // Load from NppParameters
        NppParameters& nppParam = NppParameters::getInstance();
        _pUserLang = nppParam.getULCFromIndex(index - 1);
    }

    if (!_pUserLang) return;

    // Update UI with language data
    _extEdit->setText(_pUserLang->_ext);
    _ignoreCaseCheck->setChecked(_pUserLang->_isCaseIgnored);
    _foldCompactCheck->setChecked(_pUserLang->_foldCompact);
    _allowFoldCommentsCheck->setChecked(_pUserLang->_allowFoldOfComments);

    // Force pure LC
    switch (_pUserLang->_forcePureLC) {
    case PURE_LC_BOL:
        _forceAtBOLRadio->setChecked(true);
        break;
    case PURE_LC_WSP:
        _allowWhitespaceRadio->setChecked(true);
        break;
    default:
        _allowAnywhereRadio->setChecked(true);
        break;
    }

    // Decimal separator
    switch (_pUserLang->_decimalSeparator) {
    case DECSEP_COMMA:
        _commaRadio->setChecked(true);
        break;
    case DECSEP_BOTH:
        _bothRadio->setChecked(true);
        break;
    default:
        _dotRadio->setChecked(true);
        break;
    }

    // Keyword lists
    for (int i = 0; i < 8; ++i) {
        if (_keywordLists[i]) {
            _keywordLists[i]->setPlainText(QString::fromWCharArray(
                _pUserLang->_keywordLists[SCE_USER_KWLIST_KEYWORDS1 + i]));
        }
        if (_prefixChecks[i]) {
            _prefixChecks[i]->setChecked(_pUserLang->_isPrefix[i]);
        }
    }

    // Comments - parse with group syntax
    {
        wchar_t buffer[max_char] = {0};
        retrieveFromKeywordList(buffer, _pUserLang->_keywordLists[SCE_USER_KWLIST_COMMENTS], L"00");
        _commentLineOpenEdit->setText(QString::fromWCharArray(buffer));

        buffer[0] = L'\0';
        retrieveFromKeywordList(buffer, _pUserLang->_keywordLists[SCE_USER_KWLIST_COMMENTS], L"01");
        _commentLineContinueEdit->setText(QString::fromWCharArray(buffer));

        buffer[0] = L'\0';
        retrieveFromKeywordList(buffer, _pUserLang->_keywordLists[SCE_USER_KWLIST_COMMENTS], L"02");
        _commentLineCloseEdit->setText(QString::fromWCharArray(buffer));

        buffer[0] = L'\0';
        retrieveFromKeywordList(buffer, _pUserLang->_keywordLists[SCE_USER_KWLIST_COMMENTS], L"03");
        _commentOpenEdit->setText(QString::fromWCharArray(buffer));

        buffer[0] = L'\0';
        retrieveFromKeywordList(buffer, _pUserLang->_keywordLists[SCE_USER_KWLIST_COMMENTS], L"04");
        _commentCloseEdit->setText(QString::fromWCharArray(buffer));
    }

    // Numbers
    _numberPrefix1Edit->setText(QString::fromWCharArray(
                                     _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_PREFIX1]));
    _numberPrefix2Edit->setText(QString::fromWCharArray(
                                     _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_PREFIX2]));
    _numberExtras1Edit->setText(QString::fromWCharArray(
                                     _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_EXTRAS1]));
    _numberExtras2Edit->setText(QString::fromWCharArray(
                                     _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_EXTRAS2]));
    _numberSuffix1Edit->setText(QString::fromWCharArray(
                                     _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_SUFFIX1]));
    _numberSuffix2Edit->setText(QString::fromWCharArray(
                                     _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_SUFFIX2]));
    _numberRangeEdit->setText(QString::fromWCharArray(
                                   _pUserLang->_keywordLists[SCE_USER_KWLIST_NUMBER_RANGE]));

    // Operators
    _operators1Edit->setText(QString::fromWCharArray(
                                  _pUserLang->_keywordLists[SCE_USER_KWLIST_OPERATORS1]));
    _operators2Edit->setText(QString::fromWCharArray(
                                  _pUserLang->_keywordLists[SCE_USER_KWLIST_OPERATORS2]));

    // Delimiters - parse with group syntax
    for (int i = 0; i < 8; ++i) {
        wchar_t intBuffer[10] = {L'0', 0};
        if (i < 10) {
            intBuffer[1] = L'0' + i;
            intBuffer[2] = L'\0';
        } else {
            intBuffer[0] = L'0' + (i / 10);
            intBuffer[1] = L'0' + (i % 10);
            intBuffer[2] = L'\0';
        }

        wchar_t buffer[max_char] = {0};

        // Open
        buffer[0] = L'\0';
        retrieveFromKeywordList(buffer, _pUserLang->_keywordLists[SCE_USER_KWLIST_DELIMITERS], intBuffer);
        if (_delimiterOpenEdits[i]) _delimiterOpenEdits[i]->setText(QString::fromWCharArray(buffer));

        // Escape (next index)
        intBuffer[1]++;
        buffer[0] = L'\0';
        retrieveFromKeywordList(buffer, _pUserLang->_keywordLists[SCE_USER_KWLIST_DELIMITERS], intBuffer);
        if (_delimiterEscapeEdits[i]) _delimiterEscapeEdits[i]->setText(QString::fromWCharArray(buffer));

        // Close (next index)
        intBuffer[1]++;
        buffer[0] = L'\0';
        retrieveFromKeywordList(buffer, _pUserLang->_keywordLists[SCE_USER_KWLIST_DELIMITERS], intBuffer);
        if (_delimiterCloseEdits[i]) _delimiterCloseEdits[i]->setText(QString::fromWCharArray(buffer));
    }

    // Folders - Code 1
    _folderInCode1OpenEdit->setText(QString::fromWCharArray(
                                         _pUserLang->_keywordLists[SCE_USER_KWLIST_FOLDERS_IN_CODE1_OPEN]));
    _folderInCode1MiddleEdit->setText(QString::fromWCharArray(
                                           _pUserLang->_keywordLists[SCE_USER_KWLIST_FOLDERS_IN_CODE1_MIDDLE]));
    _folderInCode1CloseEdit->setText(QString::fromWCharArray(
                                          _pUserLang->_keywordLists[SCE_USER_KWLIST_FOLDERS_IN_CODE1_CLOSE]));

    // Folders - Code 2
    _folderInCode2OpenEdit->setText(QString::fromWCharArray(
                                         _pUserLang->_keywordLists[SCE_USER_KWLIST_FOLDERS_IN_CODE2_OPEN]));
    _folderInCode2MiddleEdit->setText(QString::fromWCharArray(
                                           _pUserLang->_keywordLists[SCE_USER_KWLIST_FOLDERS_IN_CODE2_MIDDLE]));
    _folderInCode2CloseEdit->setText(QString::fromWCharArray(
                                          _pUserLang->_keywordLists[SCE_USER_KWLIST_FOLDERS_IN_CODE2_CLOSE]));

    // Folders - Comment
    _folderInCommentOpenEdit->setText(QString::fromWCharArray(
                                           _pUserLang->_keywordLists[SCE_USER_KWLIST_FOLDERS_IN_COMMENT_OPEN]));
    _folderInCommentMiddleEdit->setText(QString::fromWCharArray(
                                             _pUserLang->_keywordLists[SCE_USER_KWLIST_FOLDERS_IN_COMMENT_MIDDLE]));
    _folderInCommentCloseEdit->setText(QString::fromWCharArray(
                                            _pUserLang->_keywordLists[SCE_USER_KWLIST_FOLDERS_IN_COMMENT_CLOSE]));

    updateStyleButtons();
}

void UserDefineDialog::saveLanguage()
{
    if (!_pUserLang) return;

    // Save comments with group syntax
    {
        wchar_t newList[max_char] = {0};
        wchar_t buffer[max_char] = {0};

        // Line comment open (00)
        QString text = _commentLineOpenEdit->text();
        std::wstring wtext = text.toStdWString();
        buffer[0] = L'0'; buffer[1] = L'0';
        convertTo(newList, max_char, wtext.c_str(), buffer);

        // Line comment continue (01)
        text = _commentLineContinueEdit->text();
        wtext = text.toStdWString();
        buffer[0] = L'0'; buffer[1] = L'1';
        convertTo(newList, max_char, wtext.c_str(), buffer);

        // Line comment close (02)
        text = _commentLineCloseEdit->text();
        wtext = text.toStdWString();
        buffer[0] = L'0'; buffer[1] = L'2';
        convertTo(newList, max_char, wtext.c_str(), buffer);

        // Block comment open (03)
        text = _commentOpenEdit->text();
        wtext = text.toStdWString();
        buffer[0] = L'0'; buffer[1] = L'3';
        convertTo(newList, max_char, wtext.c_str(), buffer);

        // Block comment close (04)
        text = _commentCloseEdit->text();
        wtext = text.toStdWString();
        buffer[0] = L'0'; buffer[1] = L'4';
        convertTo(newList, max_char, wtext.c_str(), buffer);

        wcscpy(_pUserLang->_keywordLists[SCE_USER_KWLIST_COMMENTS], newList);
    }

    // Save delimiters with group syntax
    {
        wchar_t newList[max_char] = {0};
        wchar_t buffer[max_char] = {0};

        for (int i = 0; i < 8; ++i) {
            wchar_t intBuffer[10] = {0};
            if (i < 10) {
                intBuffer[0] = L'0';
                intBuffer[1] = L'0' + (i * 3);
            } else {
                intBuffer[0] = L'0' + ((i * 3) / 10);
                intBuffer[1] = L'0' + ((i * 3) % 10);
            }

            // Open
            if (_delimiterOpenEdits[i]) {
                QString text = _delimiterOpenEdits[i]->text();
                std::wstring wtext = text.toStdWString();
                convertTo(newList, max_char, wtext.c_str(), intBuffer);
            }

            // Escape (next index)
            intBuffer[1]++;
            if (_delimiterEscapeEdits[i]) {
                QString text = _delimiterEscapeEdits[i]->text();
                std::wstring wtext = text.toStdWString();
                convertTo(newList, max_char, wtext.c_str(), intBuffer);
            }

            // Close (next index)
            intBuffer[1]++;
            if (_delimiterCloseEdits[i]) {
                QString text = _delimiterCloseEdits[i]->text();
                std::wstring wtext = text.toStdWString();
                convertTo(newList, max_char, wtext.c_str(), intBuffer);
            }
        }

        wcscpy(_pUserLang->_keywordLists[SCE_USER_KWLIST_DELIMITERS], newList);
    }

    // Mark as dirty in NppParameters if not the default language
    // This will trigger saving when the application exits
}

void UserDefineDialog::updatePreview()
{
    // TODO: Update Scintilla preview if available
    if (_ppEditView && *_ppEditView) {
        // Trigger style update in Scintilla
    }
}

void UserDefineDialog::enableLangAndControlsBy(int index)
{
    bool isUserLang = (index == 0);
    bool hasLang = (index > 0);

    _extEdit->setVisible(hasLang);
    _removeLangButton->setEnabled(hasLang);
    _renameLangButton->setEnabled(hasLang);

    // Enable/disable all input controls based on selection
    bool enabled = (index >= 0);
    for (int i = 0; i < 8; ++i) {
        if (_keywordLists[i]) _keywordLists[i]->setEnabled(enabled);
        if (_prefixChecks[i]) _prefixChecks[i]->setEnabled(enabled);
    }
}

void UserDefineDialog::updateStyleButtons()
{
    // Update style button appearances based on current styles
    auto updateBtn = [this](QPushButton* btn, int styleIndex) {
        if (!btn) return;
        auto* styler = _pUserLang ? _pUserLang->getStyler(styleIndex) : nullptr;
        if (styler) {
            QString style = QString("background-color: %1; color: %2;")
                                .arg(styler->_bgColor.name())
                                .arg(styler->_fgColor.name());
            btn->setStyleSheet(style);
        }
    };

    for (int i = 0; i < 8; ++i) {
        updateBtn(_keywordStyleBtns[i], SCE_USER_STYLE_KEYWORD1 + i);
        updateBtn(_delimiterStyleBtns[i], SCE_USER_STYLE_DELIMITER1 + i);
    }

    updateBtn(_commentLineStyleBtn, SCE_USER_STYLE_COMMENTLINE);
    updateBtn(_commentStyleBtn, SCE_USER_STYLE_COMMENT);
    updateBtn(_numberStyleBtn, SCE_USER_STYLE_NUMBER);
    updateBtn(_operatorStyleBtn, SCE_USER_STYLE_OPERATOR);
    updateBtn(_folderInCode1StyleBtn, SCE_USER_STYLE_FOLDER_IN_CODE1);
    updateBtn(_folderInCode2StyleBtn, SCE_USER_STYLE_FOLDER_IN_CODE2);
    updateBtn(_folderInCommentStyleBtn, SCE_USER_STYLE_FOLDER_IN_COMMENT);
    updateBtn(_defaultStyleBtn, SCE_USER_STYLE_DEFAULT);
}

} // namespace QtControls

