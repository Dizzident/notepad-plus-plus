// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "preferenceDlg.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFrame>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtGui/QDesktopServices>

namespace QtControls {

// ============================================================================
// GeneralSubDlg Implementation
// ============================================================================
GeneralSubDlg::GeneralSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void GeneralSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // General settings group
    _generalGroup = new QGroupBox(tr("General"), this);
    auto* generalLayout = new QVBoxLayout(_generalGroup);

    _hideStatusBarCheck = new QCheckBox(tr("Hide status bar"), _generalGroup);
    generalLayout->addWidget(_hideStatusBarCheck);

    _hideMenuBarCheck = new QCheckBox(tr("Hide menu bar (use Alt or F10 key to toggle)"), _generalGroup);
    generalLayout->addWidget(_hideMenuBarCheck);

    _hideMenuShortcutsCheck = new QCheckBox(tr("Hide right shortcuts from menu bar (e.g. +F, +O, +S...)"), _generalGroup);
    generalLayout->addWidget(_hideMenuShortcutsCheck);

    mainLayout->addWidget(_generalGroup);

    // Localization group
    _localizationGroup = new QGroupBox(tr("Localization"), this);
    auto* localizationLayout = new QHBoxLayout(_localizationGroup);

    _languageLabel = new QLabel(tr("Language:"), _localizationGroup);
    localizationLayout->addWidget(_languageLabel);

    _languageCombo = new QComboBox(_localizationGroup);
    _languageCombo->setMinimumWidth(200);
    // Add common languages
    QStringList languages;
    languages << "English" << "French" << "German" << "Spanish" << "Italian"
              << "Portuguese" << "Russian" << "Chinese (Simplified)" << "Chinese (Traditional)"
              << "Japanese" << "Korean" << "Arabic" << "Dutch" << "Polish"
              << "Turkish" << "Czech" << "Hungarian" << "Romanian" << "Vietnamese";
    _languageCombo->addItems(languages);
    localizationLayout->addWidget(_languageCombo);
    localizationLayout->addStretch();

    mainLayout->addWidget(_localizationGroup);
    mainLayout->addStretch();
}

void GeneralSubDlg::connectSignals()
{
    connect(_hideStatusBarCheck, &QCheckBox::toggled, this, &GeneralSubDlg::onStatusBarToggled);
    connect(_hideMenuBarCheck, &QCheckBox::toggled, this, &GeneralSubDlg::onMenuBarToggled);
    connect(_hideMenuShortcutsCheck, &QCheckBox::toggled, this, &GeneralSubDlg::onHideMenuShortcutsToggled);
    connect(_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GeneralSubDlg::onLanguageChanged);
}

void GeneralSubDlg::loadSettings()
{
    // Load from PlatformLayer::Settings
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _statusBarShow = settings.readBool(L"General", L"StatusBarShow", true);
    _menuBarShow = settings.readBool(L"General", L"MenuBarShow", true);
    _hideMenuRightShortcuts = settings.readBool(L"General", L"HideMenuRightShortcuts", false);
    _currentLanguage = QString::fromStdWString(settings.readString(L"General", L"Language", L"English"));

    _hideStatusBarCheck->setChecked(!_statusBarShow);
    _hideMenuBarCheck->setChecked(!_menuBarShow);
    _hideMenuShortcutsCheck->setChecked(_hideMenuRightShortcuts);

    int langIndex = _languageCombo->findText(_currentLanguage);
    if (langIndex >= 0) {
        _languageCombo->setCurrentIndex(langIndex);
    }
}

void GeneralSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"General", L"StatusBarShow", _statusBarShow);
    settings.writeBool(L"General", L"MenuBarShow", _menuBarShow);
    settings.writeBool(L"General", L"HideMenuRightShortcuts", _hideMenuRightShortcuts);
    settings.writeString(L"General", L"Language", _currentLanguage.toStdWString());
}

bool GeneralSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void GeneralSubDlg::onStatusBarToggled(bool checked)
{
    _statusBarShow = !checked;
}

void GeneralSubDlg::onMenuBarToggled(bool checked)
{
    _menuBarShow = !checked;
}

void GeneralSubDlg::onHideMenuShortcutsToggled(bool checked)
{
    _hideMenuRightShortcuts = checked;
}

void GeneralSubDlg::onLanguageChanged(int index)
{
    if (index >= 0) {
        _currentLanguage = _languageCombo->currentText();
    }
}

// ============================================================================
// EditingSubDlg Implementation
// ============================================================================
EditingSubDlg::EditingSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void EditingSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Line Numbering group
    _lineNumberGroup = new QGroupBox(tr("Line Numbering"), this);
    auto* lineNumberLayout = new QVBoxLayout(_lineNumberGroup);

    _lineNumberCheck = new QCheckBox(tr("Display line number"), _lineNumberGroup);
    lineNumberLayout->addWidget(_lineNumberCheck);

    _lineNumberDynamicWidthCheck = new QCheckBox(tr("Dynamic line number width"), _lineNumberGroup);
    lineNumberLayout->addWidget(_lineNumberDynamicWidthCheck);

    mainLayout->addWidget(_lineNumberGroup);

    // Current Line group
    _currentLineGroup = new QGroupBox(tr("Current Line"), this);
    auto* currentLineLayout = new QHBoxLayout(_currentLineGroup);

    _currentLineLabel = new QLabel(tr("Highlighting mode:"), _currentLineGroup);
    currentLineLayout->addWidget(_currentLineLabel);

    _currentLineHighlightCombo = new QComboBox(_currentLineGroup);
    _currentLineHighlightCombo->addItem(tr("None"));
    _currentLineHighlightCombo->addItem(tr("Highlight background"));
    _currentLineHighlightCombo->addItem(tr("Frame"));
    currentLineLayout->addWidget(_currentLineHighlightCombo);
    currentLineLayout->addStretch();

    mainLayout->addWidget(_currentLineGroup);

    // Caret group
    _caretGroup = new QGroupBox(tr("Caret (Cursor)"), this);
    auto* caretLayout = new QGridLayout(_caretGroup);

    _caretBlinkRateLabel = new QLabel(tr("Blink rate:"), _caretGroup);
    caretLayout->addWidget(_caretBlinkRateLabel, 0, 0);

    _caretBlinkRateSlider = new QSlider(Qt::Horizontal, _caretGroup);
    _caretBlinkRateSlider->setRange(50, 2500);
    _caretBlinkRateSlider->setSingleStep(50);
    _caretBlinkRateSlider->setValue(600);
    caretLayout->addWidget(_caretBlinkRateSlider, 0, 1);

    _caretBlinkRateValue = new QLabel(tr("600 ms"), _caretGroup);
    caretLayout->addWidget(_caretBlinkRateValue, 0, 2);

    _caretWidthLabel = new QLabel(tr("Width:"), _caretGroup);
    caretLayout->addWidget(_caretWidthLabel, 1, 0);

    _caretWidthCombo = new QComboBox(_caretGroup);
    _caretWidthCombo->addItem(tr("0 - Block"));
    _caretWidthCombo->addItem(tr("1"));
    _caretWidthCombo->addItem(tr("2"));
    _caretWidthCombo->addItem(tr("3"));
    _caretWidthCombo->addItem(tr("Block After"));
    caretLayout->addWidget(_caretWidthCombo, 1, 1, 1, 2);

    mainLayout->addWidget(_caretGroup);

    // Scintilla Options group
    _scintillaGroup = new QGroupBox(tr("Scintilla Options"), this);
    auto* scintillaLayout = new QVBoxLayout(_scintillaGroup);

    _smoothFontCheck = new QCheckBox(tr("Enable smooth font"), _scintillaGroup);
    scintillaLayout->addWidget(_smoothFontCheck);

    _virtualSpaceCheck = new QCheckBox(tr("Enable virtual space"), _scintillaGroup);
    scintillaLayout->addWidget(_virtualSpaceCheck);

    _scrollBeyondLastLineCheck = new QCheckBox(tr("Scroll beyond last line"), _scintillaGroup);
    scintillaLayout->addWidget(_scrollBeyondLastLineCheck);

    _rightClickKeepsSelectionCheck = new QCheckBox(tr("Right click keeps selection"), _scintillaGroup);
    scintillaLayout->addWidget(_rightClickKeepsSelectionCheck);

    _lineCopyCutWithoutSelectionCheck = new QCheckBox(tr("Copy/Cut line without selection"), _scintillaGroup);
    scintillaLayout->addWidget(_lineCopyCutWithoutSelectionCheck);

    mainLayout->addWidget(_scintillaGroup);

    // Line Wrap group
    _lineWrapGroup = new QGroupBox(tr("Line Wrap"), this);
    auto* lineWrapLayout = new QHBoxLayout(_lineWrapGroup);

    _lineWrapCombo = new QComboBox(_lineWrapGroup);
    _lineWrapCombo->addItem(tr("Default"));
    _lineWrapCombo->addItem(tr("Aligned"));
    _lineWrapCombo->addItem(tr("Indent"));
    lineWrapLayout->addWidget(_lineWrapCombo);
    lineWrapLayout->addStretch();

    mainLayout->addWidget(_lineWrapGroup);
    mainLayout->addStretch();
}

void EditingSubDlg::connectSignals()
{
    connect(_lineNumberCheck, &QCheckBox::toggled, this, &EditingSubDlg::onLineNumberToggled);
    connect(_lineNumberDynamicWidthCheck, &QCheckBox::toggled, this, &EditingSubDlg::onLineNumberDynamicWidthToggled);
    connect(_currentLineHighlightCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditingSubDlg::onCurrentLineHighlightChanged);
    connect(_caretBlinkRateSlider, &QSlider::valueChanged, this, &EditingSubDlg::onCaretBlinkRateChanged);
    connect(_caretWidthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditingSubDlg::onCaretWidthChanged);
    connect(_smoothFontCheck, &QCheckBox::toggled, this, &EditingSubDlg::onSmoothFontToggled);
    connect(_virtualSpaceCheck, &QCheckBox::toggled, this, &EditingSubDlg::onVirtualSpaceToggled);
    connect(_scrollBeyondLastLineCheck, &QCheckBox::toggled, this, &EditingSubDlg::onScrollBeyondLastLineToggled);
    connect(_rightClickKeepsSelectionCheck, &QCheckBox::toggled, this, &EditingSubDlg::onRightClickKeepsSelectionToggled);
    connect(_lineCopyCutWithoutSelectionCheck, &QCheckBox::toggled, this, &EditingSubDlg::onLineCopyCutWithoutSelectionToggled);
    connect(_lineWrapCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditingSubDlg::onLineWrapMethodChanged);
}

void EditingSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _lineNumberShow = settings.readBool(L"Scintillas", L"LineNumberShow", true);
    _lineNumberDynamicWidth = settings.readBool(L"Scintillas", L"LineNumberDynamicWidth", true);
    _currentLineHighlightMode = settings.readInt(L"Scintillas", L"CurrentLineHighlightMode", 1);
    _caretBlinkRate = settings.readInt(L"Scintillas", L"CaretBlinkRate", 600);
    _caretWidth = settings.readInt(L"Scintillas", L"CaretWidth", 1);
    _doSmoothFont = settings.readBool(L"Scintillas", L"DoSmoothFont", false);
    _virtualSpace = settings.readBool(L"Scintillas", L"VirtualSpace", false);
    _scrollBeyondLastLine = settings.readBool(L"Scintillas", L"ScrollBeyondLastLine", true);
    _rightClickKeepsSelection = settings.readBool(L"Scintillas", L"RightClickKeepsSelection", false);
    _lineCopyCutWithoutSelection = settings.readBool(L"Scintillas", L"LineCopyCutWithoutSelection", false);
    _lineWrapMethod = settings.readInt(L"Scintillas", L"LineWrapMethod", 0);

    _lineNumberCheck->setChecked(_lineNumberShow);
    _lineNumberDynamicWidthCheck->setChecked(_lineNumberDynamicWidth);
    _currentLineHighlightCombo->setCurrentIndex(_currentLineHighlightMode);
    _caretBlinkRateSlider->setValue(_caretBlinkRate);
    _caretBlinkRateValue->setText(QString::number(_caretBlinkRate) + " ms");
    _caretWidthCombo->setCurrentIndex(_caretWidth);
    _smoothFontCheck->setChecked(_doSmoothFont);
    _virtualSpaceCheck->setChecked(_virtualSpace);
    _scrollBeyondLastLineCheck->setChecked(_scrollBeyondLastLine);
    _rightClickKeepsSelectionCheck->setChecked(_rightClickKeepsSelection);
    _lineCopyCutWithoutSelectionCheck->setChecked(_lineCopyCutWithoutSelection);
    _lineWrapCombo->setCurrentIndex(_lineWrapMethod);
}

void EditingSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Scintillas", L"LineNumberShow", _lineNumberShow);
    settings.writeBool(L"Scintillas", L"LineNumberDynamicWidth", _lineNumberDynamicWidth);
    settings.writeInt(L"Scintillas", L"CurrentLineHighlightMode", _currentLineHighlightMode);
    settings.writeInt(L"Scintillas", L"CaretBlinkRate", _caretBlinkRate);
    settings.writeInt(L"Scintillas", L"CaretWidth", _caretWidth);
    settings.writeBool(L"Scintillas", L"DoSmoothFont", _doSmoothFont);
    settings.writeBool(L"Scintillas", L"VirtualSpace", _virtualSpace);
    settings.writeBool(L"Scintillas", L"ScrollBeyondLastLine", _scrollBeyondLastLine);
    settings.writeBool(L"Scintillas", L"RightClickKeepsSelection", _rightClickKeepsSelection);
    settings.writeBool(L"Scintillas", L"LineCopyCutWithoutSelection", _lineCopyCutWithoutSelection);
    settings.writeInt(L"Scintillas", L"LineWrapMethod", _lineWrapMethod);
}

bool EditingSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void EditingSubDlg::onLineNumberToggled(bool checked)
{
    _lineNumberShow = checked;
}

void EditingSubDlg::onLineNumberDynamicWidthToggled(bool checked)
{
    _lineNumberDynamicWidth = checked;
}

void EditingSubDlg::onCurrentLineHighlightChanged(int index)
{
    _currentLineHighlightMode = index;
}

void EditingSubDlg::onCaretBlinkRateChanged(int value)
{
    _caretBlinkRate = value;
    _caretBlinkRateValue->setText(QString::number(value) + " ms");
}

void EditingSubDlg::onCaretWidthChanged(int index)
{
    _caretWidth = index;
}

void EditingSubDlg::onSmoothFontToggled(bool checked)
{
    _doSmoothFont = checked;
}

void EditingSubDlg::onVirtualSpaceToggled(bool checked)
{
    _virtualSpace = checked;
}

void EditingSubDlg::onScrollBeyondLastLineToggled(bool checked)
{
    _scrollBeyondLastLine = checked;
}

void EditingSubDlg::onRightClickKeepsSelectionToggled(bool checked)
{
    _rightClickKeepsSelection = checked;
}

void EditingSubDlg::onLineCopyCutWithoutSelectionToggled(bool checked)
{
    _lineCopyCutWithoutSelection = checked;
}

void EditingSubDlg::onLineWrapMethodChanged(int index)
{
    _lineWrapMethod = index;
}

void EditingSubDlg::initScintParam()
{
    // Additional initialization if needed
}

// ============================================================================
// NewDocumentSubDlg Implementation
// ============================================================================
NewDocumentSubDlg::NewDocumentSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void NewDocumentSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Encoding group
    _encodingGroup = new QGroupBox(tr("Encoding"), this);
    auto* encodingLayout = new QVBoxLayout(_encodingGroup);

    _encodingCombo = new QComboBox(_encodingGroup);
    _encodingCombo->addItem(tr("UTF-8"));
    _encodingCombo->addItem(tr("UTF-8 BOM"));
    _encodingCombo->addItem(tr("UTF-16 Little Endian"));
    _encodingCombo->addItem(tr("UTF-16 Big Endian"));
    _encodingCombo->addItem(tr("ANSI"));
    _encodingCombo->setMinimumWidth(200);
    encodingLayout->addWidget(_encodingCombo);

    _ansiAsUtf8Check = new QCheckBox(tr("Open ANSI files as UTF-8 (without BOM)"), _encodingGroup);
    encodingLayout->addWidget(_ansiAsUtf8Check);

    mainLayout->addWidget(_encodingGroup);

    // Format group
    _formatGroup = new QGroupBox(tr("Format (Line ending)"), this);
    auto* formatLayout = new QVBoxLayout(_formatGroup);

    _formatCombo = new QComboBox(_formatGroup);
    _formatCombo->addItem(tr("Windows (CR LF)"));
    _formatCombo->addItem(tr("Unix (LF)"));
    _formatCombo->addItem(tr("Macintosh (CR)"));
    formatLayout->addWidget(_formatCombo);

    mainLayout->addWidget(_formatGroup);

    // Language group
    _languageGroup = new QGroupBox(tr("Default Language"), this);
    auto* languageLayout = new QVBoxLayout(_languageGroup);

    _languageCombo = new QComboBox(_languageGroup);
    _languageCombo->addItem(tr("Text"));
    _languageCombo->addItem(tr("C"));
    _languageCombo->addItem(tr("C++"));
    _languageCombo->addItem(tr("Java"));
    _languageCombo->addItem(tr("Python"));
    _languageCombo->addItem(tr("JavaScript"));
    _languageCombo->addItem(tr("HTML"));
    _languageCombo->addItem(tr("XML"));
    _languageCombo->addItem(tr("CSS"));
    _languageCombo->addItem(tr("PHP"));
    languageLayout->addWidget(_languageCombo);

    mainLayout->addWidget(_languageGroup);

    // Defaults group
    _defaultsGroup = new QGroupBox(tr("Apply to Opened ANSI Files"), this);
    auto* defaultsLayout = new QVBoxLayout(_defaultsGroup);

    _applyToOpenedAnsiFilesCheck = new QCheckBox(tr("Apply the above settings to opened ANSI files"), _defaultsGroup);
    defaultsLayout->addWidget(_applyToOpenedAnsiFilesCheck);

    mainLayout->addWidget(_defaultsGroup);
    mainLayout->addStretch();
}

void NewDocumentSubDlg::connectSignals()
{
    connect(_encodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewDocumentSubDlg::onEncodingChanged);
    connect(_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewDocumentSubDlg::onFormatChanged);
    connect(_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewDocumentSubDlg::onLanguageChanged);
    connect(_ansiAsUtf8Check, &QCheckBox::toggled, this, &NewDocumentSubDlg::onAnsiAsUtf8Toggled);
}

void NewDocumentSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _defaultEncoding = settings.readInt(L"NewDoc", L"DefaultEncoding", 0);
    _defaultFormat = settings.readInt(L"NewDoc", L"DefaultFormat", 0);
    _defaultLanguage = settings.readInt(L"NewDoc", L"DefaultLanguage", 0);
    _openAnsiAsUtf8 = settings.readBool(L"NewDoc", L"OpenAnsiAsUtf8", true);

    _encodingCombo->setCurrentIndex(_defaultEncoding);
    _formatCombo->setCurrentIndex(_defaultFormat);
    _languageCombo->setCurrentIndex(_defaultLanguage);
    _ansiAsUtf8Check->setChecked(_openAnsiAsUtf8);
}

void NewDocumentSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"NewDoc", L"DefaultEncoding", _defaultEncoding);
    settings.writeInt(L"NewDoc", L"DefaultFormat", _defaultFormat);
    settings.writeInt(L"NewDoc", L"DefaultLanguage", _defaultLanguage);
    settings.writeBool(L"NewDoc", L"OpenAnsiAsUtf8", _openAnsiAsUtf8);
}

bool NewDocumentSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void NewDocumentSubDlg::onEncodingChanged(int index)
{
    _defaultEncoding = index;
}

void NewDocumentSubDlg::onFormatChanged(int index)
{
    _defaultFormat = index;
}

void NewDocumentSubDlg::onLanguageChanged(int index)
{
    _defaultLanguage = index;
}

void NewDocumentSubDlg::onAnsiAsUtf8Toggled(bool checked)
{
    _openAnsiAsUtf8 = checked;
}

// ============================================================================
// DefaultDirectorySubDlg Implementation
// ============================================================================
DefaultDirectorySubDlg::DefaultDirectorySubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void DefaultDirectorySubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Directory group
    _directoryGroup = new QGroupBox(tr("Default Directory"), this);
    auto* directoryLayout = new QVBoxLayout(_directoryGroup);

    _directoryTypeCombo = new QComboBox(_directoryGroup);
    _directoryTypeCombo->addItem(tr("Follow current document"));
    _directoryTypeCombo->addItem(tr("Remember last used directory"));
    _directoryTypeCombo->addItem(tr("Custom directory:"));
    directoryLayout->addWidget(_directoryTypeCombo);

    auto* customPathLayout = new QHBoxLayout();
    _customPathEdit = new QLineEdit(_directoryGroup);
    _customPathEdit->setPlaceholderText(tr("Enter custom directory path..."));
    customPathLayout->addWidget(_customPathEdit);

    _browseButton = new QPushButton(tr("Browse..."), _directoryGroup);
    customPathLayout->addWidget(_browseButton);

    directoryLayout->addLayout(customPathLayout);
    mainLayout->addWidget(_directoryGroup);
    mainLayout->addStretch();
}

void DefaultDirectorySubDlg::connectSignals()
{
    connect(_directoryTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DefaultDirectorySubDlg::onDirectoryTypeChanged);
    connect(_browseButton, &QPushButton::clicked, this, &DefaultDirectorySubDlg::onBrowseClicked);
    connect(_customPathEdit, &QLineEdit::textChanged, this, &DefaultDirectorySubDlg::onCustomPathChanged);
}

void DefaultDirectorySubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _defaultDirectoryType = settings.readInt(L"DefaultDir", L"DefaultDirectoryType", 0);
    _customDefaultDirectory = QString::fromStdWString(settings.readString(L"DefaultDir", L"CustomDefaultDirectory", L""));

    _directoryTypeCombo->setCurrentIndex(_defaultDirectoryType);
    _customPathEdit->setText(_customDefaultDirectory);
    _customPathEdit->setEnabled(_defaultDirectoryType == 2);
    _browseButton->setEnabled(_defaultDirectoryType == 2);
}

void DefaultDirectorySubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"DefaultDir", L"DefaultDirectoryType", _defaultDirectoryType);
    settings.writeString(L"DefaultDir", L"CustomDefaultDirectory", _customDefaultDirectory.toStdWString());
}

bool DefaultDirectorySubDlg::applySettings()
{
    saveSettings();
    return true;
}

void DefaultDirectorySubDlg::onDirectoryTypeChanged(int index)
{
    _defaultDirectoryType = index;
    bool isCustom = (index == 2);
    _customPathEdit->setEnabled(isCustom);
    _browseButton->setEnabled(isCustom);
}

void DefaultDirectorySubDlg::onBrowseClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Default Directory"),
                                                    _customDefaultDirectory.isEmpty() ? QDir::homePath() : _customDefaultDirectory);
    if (!dir.isEmpty()) {
        _customDefaultDirectory = dir;
        _customPathEdit->setText(dir);
    }
}

void DefaultDirectorySubDlg::onCustomPathChanged(const QString& path)
{
    _customDefaultDirectory = path;
}

// ============================================================================
// RecentFilesHistorySubDlg Implementation
// ============================================================================
RecentFilesHistorySubDlg::RecentFilesHistorySubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void RecentFilesHistorySubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Recent files group
    _recentFilesGroup = new QGroupBox(tr("Recent Files History"), this);
    auto* recentFilesLayout = new QGridLayout(_recentFilesGroup);

    _maxFilesLabel = new QLabel(tr("Max number of recent files:"), _recentFilesGroup);
    recentFilesLayout->addWidget(_maxFilesLabel, 0, 0);

    _maxFilesSpin = new QSpinBox(_recentFilesGroup);
    _maxFilesSpin->setRange(0, 30);
    _maxFilesSpin->setValue(10);
    recentFilesLayout->addWidget(_maxFilesSpin, 0, 1);

    _customLengthLabel = new QLabel(tr("Custom length (0 = unlimited):"), _recentFilesGroup);
    recentFilesLayout->addWidget(_customLengthLabel, 1, 0);

    _customLengthSpin = new QSpinBox(_recentFilesGroup);
    _customLengthSpin->setRange(0, 100);
    _customLengthSpin->setValue(0);
    recentFilesLayout->addWidget(_customLengthSpin, 1, 1);

    recentFilesLayout->setColumnStretch(2, 1);

    _dontCheckAtStartupCheck = new QCheckBox(tr("Don't check at startup"), _recentFilesGroup);
    recentFilesLayout->addWidget(_dontCheckAtStartupCheck, 2, 0, 1, 3);

    mainLayout->addWidget(_recentFilesGroup);
    mainLayout->addStretch();
}

void RecentFilesHistorySubDlg::connectSignals()
{
    connect(_maxFilesSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &RecentFilesHistorySubDlg::onMaxFilesChanged);
    connect(_customLengthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &RecentFilesHistorySubDlg::onCustomLengthChanged);
    connect(_dontCheckAtStartupCheck, &QCheckBox::toggled, this, &RecentFilesHistorySubDlg::onDontCheckAtStartupToggled);
}

void RecentFilesHistorySubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _maxRecentFiles = settings.readInt(L"RecentFilesHistory", L"MaxRecentFiles", 10);
    _customLength = settings.readInt(L"RecentFilesHistory", L"CustomLength", 0);
    _dontCheckAtStartup = settings.readBool(L"RecentFilesHistory", L"DontCheckAtStartup", false);

    _maxFilesSpin->setValue(_maxRecentFiles);
    _customLengthSpin->setValue(_customLength);
    _dontCheckAtStartupCheck->setChecked(_dontCheckAtStartup);
}

void RecentFilesHistorySubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"RecentFilesHistory", L"MaxRecentFiles", _maxRecentFiles);
    settings.writeInt(L"RecentFilesHistory", L"CustomLength", _customLength);
    settings.writeBool(L"RecentFilesHistory", L"DontCheckAtStartup", _dontCheckAtStartup);
}

bool RecentFilesHistorySubDlg::applySettings()
{
    saveSettings();
    return true;
}

void RecentFilesHistorySubDlg::onMaxFilesChanged(int value)
{
    _maxRecentFiles = value;
}

void RecentFilesHistorySubDlg::onCustomLengthChanged(int value)
{
    _customLength = value;
}

void RecentFilesHistorySubDlg::onDontCheckAtStartupToggled(bool checked)
{
    _dontCheckAtStartup = checked;
}

// ============================================================================
// LanguageSubDlg Implementation
// ============================================================================
LanguageSubDlg::LanguageSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void LanguageSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Language list group
    _languageListGroup = new QGroupBox(tr("Language Menu"), this);
    auto* languageListLayout = new QVBoxLayout(_languageListGroup);

    _languageList = new QListWidget(_languageListGroup);
    populateLanguageList();
    languageListLayout->addWidget(_languageList);

    mainLayout->addWidget(_languageListGroup);

    // Indentation group
    _indentationGroup = new QGroupBox(tr("Tab Settings"), this);
    auto* indentationLayout = new QGridLayout(_indentationGroup);

    _tabSizeLabel = new QLabel(tr("Tab size:"), _indentationGroup);
    indentationLayout->addWidget(_tabSizeLabel, 0, 0);

    _tabSizeSpin = new QSpinBox(_indentationGroup);
    _tabSizeSpin->setRange(1, 16);
    _tabSizeSpin->setValue(4);
    indentationLayout->addWidget(_tabSizeSpin, 0, 1);

    _replaceBySpaceCheck = new QCheckBox(tr("Replace by space"), _indentationGroup);
    indentationLayout->addWidget(_replaceBySpaceCheck, 1, 0, 1, 2);

    indentationLayout->setColumnStretch(2, 1);

    mainLayout->addWidget(_indentationGroup);
    mainLayout->addStretch();
}

void LanguageSubDlg::populateLanguageList()
{
    QStringList languages;
    languages << "Text" << "ActionScript" << "Ada" << "ASP" << "Assembly" << "AutoIt"
              << "Batch" << "C" << "C++" << "C#" << "CSS" << "D" << "Diff"
              << "Fortran" << "HTML" << "INI file" << "Java" << "JavaScript"
              << "JSON" << "JSP" << "Lua" << "Makefile" << "Markdown" << "Matlab"
              << "Objective-C" << "Pascal" << "Perl" << "PHP" << "PowerShell"
              << "Python" << "R" << "Ruby" << "Rust" << "Shell" << "SQL"
              << "Tcl" << "TypeScript" << "VB" << "VBScript" << "Verilog"
              << "VHDL" << "XML" << "YAML";
    _languageList->addItems(languages);
}

void LanguageSubDlg::connectSignals()
{
    connect(_languageList, &QListWidget::currentRowChanged, this, &LanguageSubDlg::onLanguageSelected);
    connect(_tabSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &LanguageSubDlg::onTabSizeChanged);
    connect(_replaceBySpaceCheck, &QCheckBox::toggled, this, &LanguageSubDlg::onReplaceBySpaceToggled);
}

void LanguageSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _selectedLanguage = settings.readInt(L"Language", L"SelectedLanguage", 0);
    _tabSize = settings.readInt(L"Language", L"TabSize", 4);
    _replaceBySpace = settings.readBool(L"Language", L"ReplaceBySpace", false);

    if (_selectedLanguage < _languageList->count()) {
        _languageList->setCurrentRow(_selectedLanguage);
    }
    _tabSizeSpin->setValue(_tabSize);
    _replaceBySpaceCheck->setChecked(_replaceBySpace);
}

void LanguageSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"Language", L"SelectedLanguage", _selectedLanguage);
    settings.writeInt(L"Language", L"TabSize", _tabSize);
    settings.writeBool(L"Language", L"ReplaceBySpace", _replaceBySpace);
}

bool LanguageSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void LanguageSubDlg::onLanguageSelected(int index)
{
    _selectedLanguage = index;
}

void LanguageSubDlg::onTabSizeChanged(int value)
{
    _tabSize = value;
}

void LanguageSubDlg::onReplaceBySpaceToggled(bool checked)
{
    _replaceBySpace = checked;
}

// ============================================================================
// HighlightingSubDlg Implementation
// ============================================================================
HighlightingSubDlg::HighlightingSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void HighlightingSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Smart Highlighting group
    _smartHighlightGroup = new QGroupBox(tr("Smart Highlighting"), this);
    auto* smartHighlightLayout = new QVBoxLayout(_smartHighlightGroup);

    _enableSmartHighlightCheck = new QCheckBox(tr("Enable smart highlighting"), _smartHighlightGroup);
    smartHighlightLayout->addWidget(_enableSmartHighlightCheck);

    _matchCaseCheck = new QCheckBox(tr("Match case"), _smartHighlightGroup);
    smartHighlightLayout->addWidget(_matchCaseCheck);

    _wholeWordCheck = new QCheckBox(tr("Match whole word only"), _smartHighlightGroup);
    smartHighlightLayout->addWidget(_wholeWordCheck);

    _matchSelectionCheck = new QCheckBox(tr("Use selection highlighting on another view"), _smartHighlightGroup);
    smartHighlightLayout->addWidget(_matchSelectionCheck);

    mainLayout->addWidget(_smartHighlightGroup);

    // Matching group
    _matchingGroup = new QGroupBox(tr("Brace and Tag Matching"), this);
    auto* matchingLayout = new QVBoxLayout(_matchingGroup);

    _braceHighlightCheck = new QCheckBox(tr("Enable brace matching"), _matchingGroup);
    matchingLayout->addWidget(_braceHighlightCheck);

    _tagHighlightCheck = new QCheckBox(tr("Enable tag attribute matching (HTML/XML)"), _matchingGroup);
    matchingLayout->addWidget(_tagHighlightCheck);

    mainLayout->addWidget(_matchingGroup);
    mainLayout->addStretch();
}

void HighlightingSubDlg::connectSignals()
{
    connect(_enableSmartHighlightCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onSmartHighlightingToggled);
    connect(_matchCaseCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onMatchCaseToggled);
    connect(_wholeWordCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onWholeWordToggled);
    connect(_matchSelectionCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onMatchSelectionToggled);
    connect(_braceHighlightCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onBraceHighlightingToggled);
    connect(_tagHighlightCheck, &QCheckBox::toggled, this, &HighlightingSubDlg::onTagHighlightingToggled);
}

void HighlightingSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _enableSmartHighlight = settings.readBool(L"Highlighting", L"EnableSmartHighlight", true);
    _smartHighlightMatchCase = settings.readBool(L"Highlighting", L"SmartHighlightMatchCase", false);
    _smartHighlightWholeWord = settings.readBool(L"Highlighting", L"SmartHighlightWholeWord", true);
    _smartHighlightUseSelection = settings.readBool(L"Highlighting", L"SmartHighlightUseSelection", false);
    _enableBraceHighlight = settings.readBool(L"Highlighting", L"EnableBraceHighlight", true);
    _enableTagHighlight = settings.readBool(L"Highlighting", L"EnableTagHighlight", true);

    _enableSmartHighlightCheck->setChecked(_enableSmartHighlight);
    _matchCaseCheck->setChecked(_smartHighlightMatchCase);
    _wholeWordCheck->setChecked(_smartHighlightWholeWord);
    _matchSelectionCheck->setChecked(_smartHighlightUseSelection);
    _braceHighlightCheck->setChecked(_enableBraceHighlight);
    _tagHighlightCheck->setChecked(_enableTagHighlight);
}

void HighlightingSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Highlighting", L"EnableSmartHighlight", _enableSmartHighlight);
    settings.writeBool(L"Highlighting", L"SmartHighlightMatchCase", _smartHighlightMatchCase);
    settings.writeBool(L"Highlighting", L"SmartHighlightWholeWord", _smartHighlightWholeWord);
    settings.writeBool(L"Highlighting", L"SmartHighlightUseSelection", _smartHighlightUseSelection);
    settings.writeBool(L"Highlighting", L"EnableBraceHighlight", _enableBraceHighlight);
    settings.writeBool(L"Highlighting", L"EnableTagHighlight", _enableTagHighlight);
}

bool HighlightingSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void HighlightingSubDlg::onSmartHighlightingToggled(bool checked)
{
    _enableSmartHighlight = checked;
    _matchCaseCheck->setEnabled(checked);
    _wholeWordCheck->setEnabled(checked);
    _matchSelectionCheck->setEnabled(checked);
}

void HighlightingSubDlg::onMatchCaseToggled(bool checked)
{
    _smartHighlightMatchCase = checked;
}

void HighlightingSubDlg::onWholeWordToggled(bool checked)
{
    _smartHighlightWholeWord = checked;
}

void HighlightingSubDlg::onMatchSelectionToggled(bool checked)
{
    _smartHighlightUseSelection = checked;
}

void HighlightingSubDlg::onBraceHighlightingToggled(bool checked)
{
    _enableBraceHighlight = checked;
}

void HighlightingSubDlg::onTagHighlightingToggled(bool checked)
{
    _enableTagHighlight = checked;
}

// ============================================================================
// PrintSubDlg Implementation
// ============================================================================
PrintSubDlg::PrintSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void PrintSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Print Options group
    _printOptionsGroup = new QGroupBox(tr("Print Options"), this);
    auto* printOptionsLayout = new QVBoxLayout(_printOptionsGroup);

    _printLineNumberCheck = new QCheckBox(tr("Print line number"), _printOptionsGroup);
    printOptionsLayout->addWidget(_printLineNumberCheck);

    _printSelectionCheck = new QCheckBox(tr("Print selection only (when printing selected text)"), _printOptionsGroup);
    printOptionsLayout->addWidget(_printSelectionCheck);

    _noBackgroundCheck = new QCheckBox(tr("No background color"), _printOptionsGroup);
    printOptionsLayout->addWidget(_noBackgroundCheck);

    mainLayout->addWidget(_printOptionsGroup);

    // Header/Footer group
    _headerFooterGroup = new QGroupBox(tr("Header and Footer"), this);
    auto* headerFooterLayout = new QGridLayout(_headerFooterGroup);

    _enableHeaderFooterCheck = new QCheckBox(tr("Enable header and footer"), _headerFooterGroup);
    headerFooterLayout->addWidget(_enableHeaderFooterCheck, 0, 0, 1, 3);

    // Header
    _headerLeftLabel = new QLabel(tr("Header left:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_headerLeftLabel, 1, 0);
    _headerLeftEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_headerLeftEdit, 1, 1, 1, 2);

    _headerCenterLabel = new QLabel(tr("Header center:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_headerCenterLabel, 2, 0);
    _headerCenterEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_headerCenterEdit, 2, 1, 1, 2);

    _headerRightLabel = new QLabel(tr("Header right:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_headerRightLabel, 3, 0);
    _headerRightEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_headerRightEdit, 3, 1, 1, 2);

    // Footer
    _footerLeftLabel = new QLabel(tr("Footer left:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_footerLeftLabel, 4, 0);
    _footerLeftEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_footerLeftEdit, 4, 1, 1, 2);

    _footerCenterLabel = new QLabel(tr("Footer center:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_footerCenterLabel, 5, 0);
    _footerCenterEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_footerCenterEdit, 5, 1, 1, 2);

    _footerRightLabel = new QLabel(tr("Footer right:"), _headerFooterGroup);
    headerFooterLayout->addWidget(_footerRightLabel, 6, 0);
    _footerRightEdit = new QLineEdit(_headerFooterGroup);
    headerFooterLayout->addWidget(_footerRightEdit, 6, 1, 1, 2);

    headerFooterLayout->setColumnStretch(1, 1);

    mainLayout->addWidget(_headerFooterGroup);
    mainLayout->addStretch();
}

void PrintSubDlg::connectSignals()
{
    connect(_printLineNumberCheck, &QCheckBox::toggled, this, &PrintSubDlg::onPrintLineNumberToggled);
    connect(_printSelectionCheck, &QCheckBox::toggled, this, &PrintSubDlg::onPrintSelectionToggled);
    connect(_noBackgroundCheck, &QCheckBox::toggled, this, &PrintSubDlg::onNoBackgroundToggled);
    connect(_enableHeaderFooterCheck, &QCheckBox::toggled, this, &PrintSubDlg::onHeaderFooterToggled);
    connect(_headerLeftEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onHeaderLeftChanged);
    connect(_headerCenterEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onHeaderCenterChanged);
    connect(_headerRightEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onHeaderRightChanged);
    connect(_footerLeftEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onFooterLeftChanged);
    connect(_footerCenterEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onFooterCenterChanged);
    connect(_footerRightEdit, &QLineEdit::textChanged, this, &PrintSubDlg::onFooterRightChanged);
}

void PrintSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _printLineNumber = settings.readBool(L"Print", L"PrintLineNumber", true);
    _printSelection = settings.readBool(L"Print", L"PrintSelection", false);
    _noBackground = settings.readBool(L"Print", L"NoBackground", true);
    _headerFooterEnabled = settings.readBool(L"Print", L"HeaderFooterEnabled", true);
    _headerLeft = QString::fromStdWString(settings.readString(L"Print", L"HeaderLeft", L""));
    _headerCenter = QString::fromStdWString(settings.readString(L"Print", L"HeaderCenter", L""));
    _headerRight = QString::fromStdWString(settings.readString(L"Print", L"HeaderRight", L""));
    _footerLeft = QString::fromStdWString(settings.readString(L"Print", L"FooterLeft", L""));
    _footerCenter = QString::fromStdWString(settings.readString(L"Print", L"FooterCenter", L""));
    _footerRight = QString::fromStdWString(settings.readString(L"Print", L"FooterRight", L""));

    _printLineNumberCheck->setChecked(_printLineNumber);
    _printSelectionCheck->setChecked(_printSelection);
    _noBackgroundCheck->setChecked(_noBackground);
    _enableHeaderFooterCheck->setChecked(_headerFooterEnabled);
    _headerLeftEdit->setText(_headerLeft);
    _headerCenterEdit->setText(_headerCenter);
    _headerRightEdit->setText(_headerRight);
    _footerLeftEdit->setText(_footerLeft);
    _footerCenterEdit->setText(_footerCenter);
    _footerRightEdit->setText(_footerRight);
}

void PrintSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Print", L"PrintLineNumber", _printLineNumber);
    settings.writeBool(L"Print", L"PrintSelection", _printSelection);
    settings.writeBool(L"Print", L"NoBackground", _noBackground);
    settings.writeBool(L"Print", L"HeaderFooterEnabled", _headerFooterEnabled);
    settings.writeString(L"Print", L"HeaderLeft", _headerLeft.toStdWString());
    settings.writeString(L"Print", L"HeaderCenter", _headerCenter.toStdWString());
    settings.writeString(L"Print", L"HeaderRight", _headerRight.toStdWString());
    settings.writeString(L"Print", L"FooterLeft", _footerLeft.toStdWString());
    settings.writeString(L"Print", L"FooterCenter", _footerCenter.toStdWString());
    settings.writeString(L"Print", L"FooterRight", _footerRight.toStdWString());
}

bool PrintSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void PrintSubDlg::onPrintLineNumberToggled(bool checked)
{
    _printLineNumber = checked;
}

void PrintSubDlg::onPrintSelectionToggled(bool checked)
{
    _printSelection = checked;
}

void PrintSubDlg::onNoBackgroundToggled(bool checked)
{
    _noBackground = checked;
}

void PrintSubDlg::onHeaderFooterToggled(bool checked)
{
    _headerFooterEnabled = checked;
}

void PrintSubDlg::onHeaderLeftChanged(const QString& text)
{
    _headerLeft = text;
}

void PrintSubDlg::onHeaderCenterChanged(const QString& text)
{
    _headerCenter = text;
}

void PrintSubDlg::onHeaderRightChanged(const QString& text)
{
    _headerRight = text;
}

void PrintSubDlg::onFooterLeftChanged(const QString& text)
{
    _footerLeft = text;
}

void PrintSubDlg::onFooterCenterChanged(const QString& text)
{
    _footerCenter = text;
}

void PrintSubDlg::onFooterRightChanged(const QString& text)
{
    _footerRight = text;
}

// ============================================================================
// SearchingSubDlg Implementation
// ============================================================================
SearchingSubDlg::SearchingSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void SearchingSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Search Options group
    _searchOptionsGroup = new QGroupBox(tr("Search Options"), this);
    auto* searchOptionsLayout = new QVBoxLayout(_searchOptionsGroup);

    _stopAtFirstMatchCheck = new QCheckBox(tr("Stop at first match (incremental search)"), _searchOptionsGroup);
    searchOptionsLayout->addWidget(_stopAtFirstMatchCheck);

    _wrapAroundCheck = new QCheckBox(tr("Wrap around"), _searchOptionsGroup);
    searchOptionsLayout->addWidget(_wrapAroundCheck);

    _matchWholeWordCheck = new QCheckBox(tr("Match whole word only (find only)"), _searchOptionsGroup);
    searchOptionsLayout->addWidget(_matchWholeWordCheck);

    _matchCaseCheck = new QCheckBox(tr("Match case (find only)"), _searchOptionsGroup);
    searchOptionsLayout->addWidget(_matchCaseCheck);

    mainLayout->addWidget(_searchOptionsGroup);

    // Advanced group
    _advancedGroup = new QGroupBox(tr("Advanced"), this);
    auto* advancedLayout = new QGridLayout(_advancedGroup);

    _inSelectionThresholdLabel = new QLabel(tr("In-selection threshold (characters):"), _advancedGroup);
    advancedLayout->addWidget(_inSelectionThresholdLabel, 0, 0);

    _inSelectionThresholdSpin = new QSpinBox(_advancedGroup);
    _inSelectionThresholdSpin->setRange(0, 10000);
    _inSelectionThresholdSpin->setSingleStep(128);
    _inSelectionThresholdSpin->setValue(1024);
    advancedLayout->addWidget(_inSelectionThresholdSpin, 0, 1);

    _fillFindWhatCheck = new QCheckBox(tr("Fill Find what field with selected text"), _advancedGroup);
    advancedLayout->addWidget(_fillFindWhatCheck, 1, 0, 1, 2);

    advancedLayout->setColumnStretch(2, 1);

    mainLayout->addWidget(_advancedGroup);
    mainLayout->addStretch();
}

void SearchingSubDlg::connectSignals()
{
    connect(_stopAtFirstMatchCheck, &QCheckBox::toggled, this, &SearchingSubDlg::onStopAtFirstMatchToggled);
    connect(_wrapAroundCheck, &QCheckBox::toggled, this, &SearchingSubDlg::onWrapAroundToggled);
    connect(_matchWholeWordCheck, &QCheckBox::toggled, this, &SearchingSubDlg::onMatchWholeWordToggled);
    connect(_matchCaseCheck, &QCheckBox::toggled, this, &SearchingSubDlg::onMatchCaseToggled);
    connect(_inSelectionThresholdSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SearchingSubDlg::onInSelectionThresholdChanged);
    connect(_fillFindWhatCheck, &QCheckBox::toggled, this, &SearchingSubDlg::onFillFindWhatToggled);
}

void SearchingSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _stopAtFirstMatch = settings.readBool(L"Searching", L"StopAtFirstMatch", false);
    _wrapAround = settings.readBool(L"Searching", L"WrapAround", true);
    _matchWholeWord = settings.readBool(L"Searching", L"MatchWholeWord", false);
    _matchCase = settings.readBool(L"Searching", L"MatchCase", false);
    _inSelectionThreshold = settings.readInt(L"Searching", L"InSelectionThreshold", 1024);
    _fillFindWhatWithSelection = settings.readBool(L"Searching", L"FillFindWhatWithSelection", true);

    _stopAtFirstMatchCheck->setChecked(_stopAtFirstMatch);
    _wrapAroundCheck->setChecked(_wrapAround);
    _matchWholeWordCheck->setChecked(_matchWholeWord);
    _matchCaseCheck->setChecked(_matchCase);
    _inSelectionThresholdSpin->setValue(_inSelectionThreshold);
    _fillFindWhatCheck->setChecked(_fillFindWhatWithSelection);
}

void SearchingSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Searching", L"StopAtFirstMatch", _stopAtFirstMatch);
    settings.writeBool(L"Searching", L"WrapAround", _wrapAround);
    settings.writeBool(L"Searching", L"MatchWholeWord", _matchWholeWord);
    settings.writeBool(L"Searching", L"MatchCase", _matchCase);
    settings.writeInt(L"Searching", L"InSelectionThreshold", _inSelectionThreshold);
    settings.writeBool(L"Searching", L"FillFindWhatWithSelection", _fillFindWhatWithSelection);
}

bool SearchingSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void SearchingSubDlg::onStopAtFirstMatchToggled(bool checked)
{
    _stopAtFirstMatch = checked;
}

void SearchingSubDlg::onWrapAroundToggled(bool checked)
{
    _wrapAround = checked;
}

void SearchingSubDlg::onMatchWholeWordToggled(bool checked)
{
    _matchWholeWord = checked;
}

void SearchingSubDlg::onMatchCaseToggled(bool checked)
{
    _matchCase = checked;
}

void SearchingSubDlg::onInSelectionThresholdChanged(int value)
{
    _inSelectionThreshold = value;
}

void SearchingSubDlg::onFillFindWhatToggled(bool checked)
{
    _fillFindWhatWithSelection = checked;
}

// ============================================================================
// BackupSubDlg Implementation
// ============================================================================
BackupSubDlg::BackupSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void BackupSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Backup group
    _backupGroup = new QGroupBox(tr("Backup"), this);
    auto* backupLayout = new QVBoxLayout(_backupGroup);

    _backupModeLabel = new QLabel(tr("Backup mode:"), _backupGroup);
    backupLayout->addWidget(_backupModeLabel);

    _backupModeCombo = new QComboBox(_backupGroup);
    _backupModeCombo->addItem(tr("None"));
    _backupModeCombo->addItem(tr("Simple backup"));
    _backupModeCombo->addItem(tr("Verbose backup"));
    backupLayout->addWidget(_backupModeCombo);

    mainLayout->addWidget(_backupGroup);

    // Backup Directory group
    _backupDirectoryGroup = new QGroupBox(tr("Backup Directory"), this);
    auto* backupDirectoryLayout = new QVBoxLayout(_backupDirectoryGroup);

    _backupDirectoryCombo = new QComboBox(_backupDirectoryGroup);
    _backupDirectoryCombo->addItem(tr("Same directory as current file"));
    _backupDirectoryCombo->addItem(tr("Custom directory:"));
    backupDirectoryLayout->addWidget(_backupDirectoryCombo);

    auto* customDirLayout = new QHBoxLayout();
    _customDirectoryEdit = new QLineEdit(_backupDirectoryGroup);
    _customDirectoryEdit->setPlaceholderText(tr("Enter custom backup directory..."));
    customDirLayout->addWidget(_customDirectoryEdit);

    _browseButton = new QPushButton(tr("Browse..."), _backupDirectoryGroup);
    customDirLayout->addWidget(_browseButton);

    backupDirectoryLayout->addLayout(customDirLayout);
    mainLayout->addWidget(_backupDirectoryGroup);

    // Session Snapshot group
    _sessionGroup = new QGroupBox(tr("Session Snapshot"), this);
    auto* sessionLayout = new QGridLayout(_sessionGroup);

    _sessionSnapshotCheck = new QCheckBox(tr("Enable session snapshot and periodic backup"), _sessionGroup);
    sessionLayout->addWidget(_sessionSnapshotCheck, 0, 0, 1, 3);

    _snapshotIntervalLabel = new QLabel(tr("Backup interval (seconds):"), _sessionGroup);
    sessionLayout->addWidget(_snapshotIntervalLabel, 1, 0);

    _snapshotIntervalSpin = new QSpinBox(_sessionGroup);
    _snapshotIntervalSpin->setRange(1, 600);
    _snapshotIntervalSpin->setValue(7);
    sessionLayout->addWidget(_snapshotIntervalSpin, 1, 1);

    sessionLayout->setColumnStretch(2, 1);

    mainLayout->addWidget(_sessionGroup);
    mainLayout->addStretch();
}

void BackupSubDlg::connectSignals()
{
    connect(_backupModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BackupSubDlg::onBackupModeChanged);
    connect(_backupDirectoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BackupSubDlg::onBackupDirectoryTypeChanged);
    connect(_customDirectoryEdit, &QLineEdit::textChanged, this, &BackupSubDlg::onCustomDirectoryChanged);
    connect(_browseButton, &QPushButton::clicked, this, &BackupSubDlg::onBrowseDirectoryClicked);
    connect(_sessionSnapshotCheck, &QCheckBox::toggled, this, &BackupSubDlg::onSessionSnapshotToggled);
}

void BackupSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _backupMode = settings.readInt(L"Backup", L"BackupMode", 0);
    _backupDirectoryType = settings.readInt(L"Backup", L"BackupDirectoryType", 0);
    _customBackupDirectory = QString::fromStdWString(settings.readString(L"Backup", L"CustomBackupDirectory", L""));
    _sessionSnapshot = settings.readBool(L"Backup", L"SessionSnapshot", true);
    _snapshotInterval = settings.readInt(L"Backup", L"SnapshotInterval", 7);

    _backupModeCombo->setCurrentIndex(_backupMode);
    _backupDirectoryCombo->setCurrentIndex(_backupDirectoryType);
    _customDirectoryEdit->setText(_customBackupDirectory);
    _sessionSnapshotCheck->setChecked(_sessionSnapshot);
    _snapshotIntervalSpin->setValue(_snapshotInterval);

    updateBackupUI();
}

void BackupSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"Backup", L"BackupMode", _backupMode);
    settings.writeInt(L"Backup", L"BackupDirectoryType", _backupDirectoryType);
    settings.writeString(L"Backup", L"CustomBackupDirectory", _customBackupDirectory.toStdWString());
    settings.writeBool(L"Backup", L"SessionSnapshot", _sessionSnapshot);
    settings.writeInt(L"Backup", L"SnapshotInterval", _snapshotInterval);
}

bool BackupSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void BackupSubDlg::onBackupModeChanged(int index)
{
    _backupMode = index;
    updateBackupUI();
}

void BackupSubDlg::onBackupDirectoryTypeChanged(int index)
{
    _backupDirectoryType = index;
    bool isCustom = (index == 1);
    _customDirectoryEdit->setEnabled(isCustom);
    _browseButton->setEnabled(isCustom);
}

void BackupSubDlg::onCustomDirectoryChanged(const QString& path)
{
    _customBackupDirectory = path;
}

void BackupSubDlg::onBrowseDirectoryClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Backup Directory"),
                                                    _customBackupDirectory.isEmpty() ? QDir::homePath() : _customBackupDirectory);
    if (!dir.isEmpty()) {
        _customBackupDirectory = dir;
        _customDirectoryEdit->setText(dir);
    }
}

void BackupSubDlg::onSessionSnapshotToggled(bool checked)
{
    _sessionSnapshot = checked;
    _snapshotIntervalSpin->setEnabled(checked);
    _snapshotIntervalLabel->setEnabled(checked);
}

void BackupSubDlg::updateBackupUI()
{
    bool isBackupEnabled = (_backupMode != 0);
    _backupDirectoryGroup->setEnabled(isBackupEnabled);
    bool isCustom = (_backupDirectoryType == 1);
    _customDirectoryEdit->setEnabled(isBackupEnabled && isCustom);
    _browseButton->setEnabled(isBackupEnabled && isCustom);
}

// ============================================================================
// AutoCompletionSubDlg Implementation
// ============================================================================
AutoCompletionSubDlg::AutoCompletionSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void AutoCompletionSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Auto-Completion group
    _autoCompletionGroup = new QGroupBox(tr("Auto-Completion"), this);
    auto* autoCompletionLayout = new QGridLayout(_autoCompletionGroup);

    _enableAutoCompletionCheck = new QCheckBox(tr("Enable auto-completion on each input"), _autoCompletionGroup);
    autoCompletionLayout->addWidget(_enableAutoCompletionCheck, 0, 0, 1, 3);

    _thresholdLabel = new QLabel(tr("From the th character:"), _autoCompletionGroup);
    autoCompletionLayout->addWidget(_thresholdLabel, 1, 0);

    _thresholdSpin = new QSpinBox(_autoCompletionGroup);
    _thresholdSpin->setRange(1, 9);
    _thresholdSpin->setValue(1);
    autoCompletionLayout->addWidget(_thresholdSpin, 1, 1);

    autoCompletionLayout->setColumnStretch(2, 1);

    mainLayout->addWidget(_autoCompletionGroup);

    // Auto-Insert group
    _autoInsertGroup = new QGroupBox(tr("Auto-Insert"), this);
    auto* autoInsertLayout = new QVBoxLayout(_autoInsertGroup);

    _autoInsertBracketsCheck = new QCheckBox(tr("Auto-insert matching brackets ( ) [ ] { }"), _autoInsertGroup);
    autoInsertLayout->addWidget(_autoInsertBracketsCheck);

    _autoInsertQuotesCheck = new QCheckBox(tr("Auto-insert matching quotes ' \" `"), _autoInsertGroup);
    autoInsertLayout->addWidget(_autoInsertQuotesCheck);

    mainLayout->addWidget(_autoInsertGroup);

    // Auto-Indent group
    _autoIndentGroup = new QGroupBox(tr("Auto-Indent"), this);
    auto* autoIndentLayout = new QVBoxLayout(_autoIndentGroup);

    _enableAutoIndentCheck = new QCheckBox(tr("Enable auto-indent"), _autoIndentGroup);
    autoIndentLayout->addWidget(_enableAutoIndentCheck);

    mainLayout->addWidget(_autoIndentGroup);

    // Completion Source group
    _completionSourceGroup = new QGroupBox(tr("Completion Source"), this);
    auto* completionSourceLayout = new QVBoxLayout(_completionSourceGroup);

    _completionSourceCombo = new QComboBox(_completionSourceGroup);
    _completionSourceCombo->addItem(tr("Word and function completion"));
    _completionSourceCombo->addItem(tr("Word completion only"));
    _completionSourceCombo->addItem(tr("Function completion only"));
    completionSourceLayout->addWidget(_completionSourceCombo);

    _ignoreNumbersCheck = new QCheckBox(tr("Ignore numbers"), _completionSourceGroup);
    completionSourceLayout->addWidget(_ignoreNumbersCheck);

    mainLayout->addWidget(_completionSourceGroup);
    mainLayout->addStretch();
}

void AutoCompletionSubDlg::connectSignals()
{
    connect(_enableAutoCompletionCheck, &QCheckBox::toggled, this, &AutoCompletionSubDlg::onAutoCompletionToggled);
    connect(_autoInsertBracketsCheck, &QCheckBox::toggled, this, &AutoCompletionSubDlg::onAutoInsertBracketsToggled);
    connect(_autoInsertQuotesCheck, &QCheckBox::toggled, this, &AutoCompletionSubDlg::onAutoInsertQuotesToggled);
    connect(_enableAutoIndentCheck, &QCheckBox::toggled, this, &AutoCompletionSubDlg::onAutoIndentToggled);
    connect(_completionSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AutoCompletionSubDlg::onAutoCompletionFromChanged);
    connect(_thresholdSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &AutoCompletionSubDlg::onAutoCompletionThresholdChanged);
    connect(_ignoreNumbersCheck, &QCheckBox::toggled, this, &AutoCompletionSubDlg::onAutoCompletionIgnoreNumbersToggled);
}

void AutoCompletionSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _enableAutoCompletion = settings.readBool(L"AutoCompletion", L"EnableAutoCompletion", true);
    _autoCompletionThreshold = settings.readInt(L"AutoCompletion", L"AutoCompletionThreshold", 1);
    _autoInsertBrackets = settings.readBool(L"AutoCompletion", L"AutoInsertBrackets", true);
    _autoInsertQuotes = settings.readBool(L"AutoCompletion", L"AutoInsertQuotes", true);
    _enableAutoIndent = settings.readBool(L"AutoCompletion", L"EnableAutoIndent", true);
    _autoCompletionSource = settings.readInt(L"AutoCompletion", L"AutoCompletionSource", 0);
    _autoCompletionIgnoreNumbers = settings.readBool(L"AutoCompletion", L"AutoCompletionIgnoreNumbers", false);

    _enableAutoCompletionCheck->setChecked(_enableAutoCompletion);
    _thresholdSpin->setValue(_autoCompletionThreshold);
    _autoInsertBracketsCheck->setChecked(_autoInsertBrackets);
    _autoInsertQuotesCheck->setChecked(_autoInsertQuotes);
    _enableAutoIndentCheck->setChecked(_enableAutoIndent);
    _completionSourceCombo->setCurrentIndex(_autoCompletionSource);
    _ignoreNumbersCheck->setChecked(_autoCompletionIgnoreNumbers);
}

void AutoCompletionSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"AutoCompletion", L"EnableAutoCompletion", _enableAutoCompletion);
    settings.writeInt(L"AutoCompletion", L"AutoCompletionThreshold", _autoCompletionThreshold);
    settings.writeBool(L"AutoCompletion", L"AutoInsertBrackets", _autoInsertBrackets);
    settings.writeBool(L"AutoCompletion", L"AutoInsertQuotes", _autoInsertQuotes);
    settings.writeBool(L"AutoCompletion", L"EnableAutoIndent", _enableAutoIndent);
    settings.writeInt(L"AutoCompletion", L"AutoCompletionSource", _autoCompletionSource);
    settings.writeBool(L"AutoCompletion", L"AutoCompletionIgnoreNumbers", _autoCompletionIgnoreNumbers);
}

bool AutoCompletionSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void AutoCompletionSubDlg::onAutoCompletionToggled(bool checked)
{
    _enableAutoCompletion = checked;
    _thresholdSpin->setEnabled(checked);
    _thresholdLabel->setEnabled(checked);
}

void AutoCompletionSubDlg::onAutoInsertBracketsToggled(bool checked)
{
    _autoInsertBrackets = checked;
}

void AutoCompletionSubDlg::onAutoInsertQuotesToggled(bool checked)
{
    _autoInsertQuotes = checked;
}

void AutoCompletionSubDlg::onAutoIndentToggled(bool checked)
{
    _enableAutoIndent = checked;
}

void AutoCompletionSubDlg::onAutoCompletionFromChanged(int index)
{
    _autoCompletionSource = index;
}

void AutoCompletionSubDlg::onAutoCompletionThresholdChanged(int value)
{
    _autoCompletionThreshold = value;
}

void AutoCompletionSubDlg::onAutoCompletionIgnoreNumbersToggled(bool checked)
{
    _autoCompletionIgnoreNumbers = checked;
}

// ============================================================================
// MultiInstanceSubDlg Implementation
// ============================================================================
MultiInstanceSubDlg::MultiInstanceSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void MultiInstanceSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Multi-Instance group
    _multiInstanceGroup = new QGroupBox(tr("Multi-Instance"), this);
    auto* multiInstanceLayout = new QVBoxLayout(_multiInstanceGroup);

    _multiInstanceLabel = new QLabel(tr("Open files in:"), _multiInstanceGroup);
    multiInstanceLayout->addWidget(_multiInstanceLabel);

    _multiInstanceCombo = new QComboBox(_multiInstanceGroup);
    _multiInstanceCombo->addItem(tr("Mono-instance (default) - Open all files in same instance"));
    _multiInstanceCombo->addItem(tr("Multi-instance - Each file opens in new instance"));
    _multiInstanceCombo->addItem(tr("Always in multi-instance mode"));
    multiInstanceLayout->addWidget(_multiInstanceCombo);

    mainLayout->addWidget(_multiInstanceGroup);

    // Date/Time Format group
    _dateTimeFormatGroup = new QGroupBox(tr("Date/Time Format"), this);
    auto* dateTimeFormatLayout = new QVBoxLayout(_dateTimeFormatGroup);

    _useCustomDateTimeCheck = new QCheckBox(tr("Use custom date/time format"), _dateTimeFormatGroup);
    dateTimeFormatLayout->addWidget(_useCustomDateTimeCheck);

    _dateTimeFormatEdit = new QLineEdit(_dateTimeFormatGroup);
    _dateTimeFormatEdit->setPlaceholderText(tr("yyyy-MM-dd HH:mm:ss"));
    dateTimeFormatLayout->addWidget(_dateTimeFormatEdit);

    _dateTimePreviewLabel = new QLabel(tr("Preview: "), _dateTimeFormatGroup);
    dateTimeFormatLayout->addWidget(_dateTimePreviewLabel);

    mainLayout->addWidget(_dateTimeFormatGroup);
    mainLayout->addStretch();
}

void MultiInstanceSubDlg::connectSignals()
{
    connect(_multiInstanceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MultiInstanceSubDlg::onMultiInstanceModeChanged);
    connect(_dateTimeFormatEdit, &QLineEdit::textChanged, this, &MultiInstanceSubDlg::onDateTimeFormatChanged);
    connect(_useCustomDateTimeCheck, &QCheckBox::toggled, this, &MultiInstanceSubDlg::onUseCustomDateTimeToggled);
}

void MultiInstanceSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _multiInstanceMode = settings.readInt(L"MultiInstance", L"MultiInstanceMode", 0);
    _useCustomDateTime = settings.readBool(L"MultiInstance", L"UseCustomDateTime", false);
    _dateTimeFormat = QString::fromStdWString(settings.readString(L"MultiInstance", L"DateTimeFormat", L"yyyy-MM-dd HH:mm:ss"));

    _multiInstanceCombo->setCurrentIndex(_multiInstanceMode);
    _useCustomDateTimeCheck->setChecked(_useCustomDateTime);
    _dateTimeFormatEdit->setText(_dateTimeFormat);
    _dateTimeFormatEdit->setEnabled(_useCustomDateTime);
}

void MultiInstanceSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"MultiInstance", L"MultiInstanceMode", _multiInstanceMode);
    settings.writeBool(L"MultiInstance", L"UseCustomDateTime", _useCustomDateTime);
    settings.writeString(L"MultiInstance", L"DateTimeFormat", _dateTimeFormat.toStdWString());
}

bool MultiInstanceSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void MultiInstanceSubDlg::onMultiInstanceModeChanged(int index)
{
    _multiInstanceMode = index;
}

void MultiInstanceSubDlg::onDateTimeFormatChanged(const QString& format)
{
    _dateTimeFormat = format;
    // Update preview
    if (_useCustomDateTime && !format.isEmpty()) {
        _dateTimePreviewLabel->setText(tr("Preview: ") + QDateTime::currentDateTime().toString(format));
    } else {
        _dateTimePreviewLabel->setText(tr("Preview: ") + QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat));
    }
}

void MultiInstanceSubDlg::onUseCustomDateTimeToggled(bool checked)
{
    _useCustomDateTime = checked;
    _dateTimeFormatEdit->setEnabled(checked);
}

// ============================================================================
// DelimiterSubDlg Implementation
// ============================================================================
DelimiterSubDlg::DelimiterSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void DelimiterSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Delimiter group
    _delimiterGroup = new QGroupBox(tr("Word Delimiters"), this);
    auto* delimiterLayout = new QVBoxLayout(_delimiterGroup);

    _delimiterListLabel = new QLabel(tr("Additional delimiters for word selection (double-click selection):"), _delimiterGroup);
    delimiterLayout->addWidget(_delimiterListLabel);

    _delimiterListEdit = new QLineEdit(_delimiterGroup);
    _delimiterListEdit->setPlaceholderText(tr("e.g., -+=~!@#$%^&*()[]{}|;':\",./<>?"));
    delimiterLayout->addWidget(_delimiterListEdit);

    mainLayout->addWidget(_delimiterGroup);

    // Word Character group
    _wordCharGroup = new QGroupBox(tr("Word Characters"), this);
    auto* wordCharLayout = new QVBoxLayout(_wordCharGroup);

    _defaultWordCharsRadio = new QRadioButton(tr("Use default word characters (alphanumeric and underscore)"), _wordCharGroup);
    wordCharLayout->addWidget(_defaultWordCharsRadio);

    _customWordCharsRadio = new QRadioButton(tr("Use custom word characters:"), _wordCharGroup);
    wordCharLayout->addWidget(_customWordCharsRadio);

    _customWordCharsEdit = new QLineEdit(_wordCharGroup);
    _customWordCharsEdit->setPlaceholderText(tr("e.g., _abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
    wordCharLayout->addWidget(_customWordCharsEdit);

    mainLayout->addWidget(_wordCharGroup);
    mainLayout->addStretch();
}

void DelimiterSubDlg::connectSignals()
{
    connect(_delimiterListEdit, &QLineEdit::textChanged, this, &DelimiterSubDlg::onDelimiterListChanged);
    connect(_customWordCharsEdit, &QLineEdit::textChanged, this, &DelimiterSubDlg::onWordCharListChanged);
    connect(_defaultWordCharsRadio, &QRadioButton::toggled, this, &DelimiterSubDlg::onDefaultWordCharsToggled);
}

void DelimiterSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _delimiterList = QString::fromStdWString(settings.readString(L"Delimiter", L"DelimiterList", L""));
    _useDefaultWordChars = settings.readBool(L"Delimiter", L"UseDefaultWordChars", true);
    _customWordChars = QString::fromStdWString(settings.readString(L"Delimiter", L"CustomWordChars", L""));

    _delimiterListEdit->setText(_delimiterList);
    _defaultWordCharsRadio->setChecked(_useDefaultWordChars);
    _customWordCharsRadio->setChecked(!_useDefaultWordChars);
    _customWordCharsEdit->setText(_customWordChars);
    _customWordCharsEdit->setEnabled(!_useDefaultWordChars);
}

void DelimiterSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeString(L"Delimiter", L"DelimiterList", _delimiterList.toStdWString());
    settings.writeBool(L"Delimiter", L"UseDefaultWordChars", _useDefaultWordChars);
    settings.writeString(L"Delimiter", L"CustomWordChars", _customWordChars.toStdWString());
}

bool DelimiterSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void DelimiterSubDlg::onDelimiterListChanged()
{
    _delimiterList = _delimiterListEdit->text();
}

void DelimiterSubDlg::onWordCharListChanged()
{
    _customWordChars = _customWordCharsEdit->text();
}

void DelimiterSubDlg::onDefaultWordCharsToggled(bool checked)
{
    _useDefaultWordChars = checked;
    _customWordCharsEdit->setEnabled(!checked);
}

// ============================================================================
// CloudLinkSubDlg Implementation
// ============================================================================
CloudLinkSubDlg::CloudLinkSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void CloudLinkSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Cloud group
    _cloudGroup = new QGroupBox(tr("Cloud Settings"), this);
    auto* cloudLayout = new QVBoxLayout(_cloudGroup);

    _enableCloudSyncCheck = new QCheckBox(tr("Enable cloud synchronization"), _cloudGroup);
    cloudLayout->addWidget(_enableCloudSyncCheck);

    _cloudPathLabel = new QLabel(tr("Cloud path:"), _cloudGroup);
    cloudLayout->addWidget(_cloudPathLabel);

    auto* cloudPathLayout = new QHBoxLayout();
    _cloudPathEdit = new QLineEdit(_cloudGroup);
    _cloudPathEdit->setPlaceholderText(tr("Path to cloud storage folder (e.g., OneDrive, Dropbox)"));
    cloudPathLayout->addWidget(_cloudPathEdit);

    _browseButton = new QPushButton(tr("Browse..."), _cloudGroup);
    cloudPathLayout->addWidget(_browseButton);

    cloudLayout->addLayout(cloudPathLayout);
    mainLayout->addWidget(_cloudGroup);
    mainLayout->addStretch();
}

void CloudLinkSubDlg::connectSignals()
{
    connect(_cloudPathEdit, &QLineEdit::textChanged, this, &CloudLinkSubDlg::onCloudPathChanged);
    connect(_browseButton, &QPushButton::clicked, this, &CloudLinkSubDlg::onBrowseCloudPathClicked);
    connect(_enableCloudSyncCheck, &QCheckBox::toggled, this, &CloudLinkSubDlg::onCloudSyncToggled);
}

void CloudLinkSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _cloudSyncEnabled = settings.readBool(L"Cloud", L"CloudSyncEnabled", false);
    _cloudPath = QString::fromStdWString(settings.readString(L"Cloud", L"CloudPath", L""));

    _enableCloudSyncCheck->setChecked(_cloudSyncEnabled);
    _cloudPathEdit->setText(_cloudPath);
    _cloudPathEdit->setEnabled(_cloudSyncEnabled);
    _browseButton->setEnabled(_cloudSyncEnabled);
    _cloudPathLabel->setEnabled(_cloudSyncEnabled);
}

void CloudLinkSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"Cloud", L"CloudSyncEnabled", _cloudSyncEnabled);
    settings.writeString(L"Cloud", L"CloudPath", _cloudPath.toStdWString());
}

bool CloudLinkSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void CloudLinkSubDlg::onCloudPathChanged(const QString& path)
{
    _cloudPath = path;
}

void CloudLinkSubDlg::onBrowseCloudPathClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Cloud Directory"),
                                                    _cloudPath.isEmpty() ? QDir::homePath() : _cloudPath);
    if (!dir.isEmpty()) {
        _cloudPath = dir;
        _cloudPathEdit->setText(dir);
    }
}

void CloudLinkSubDlg::onCloudSyncToggled(bool checked)
{
    _cloudSyncEnabled = checked;
    _cloudPathEdit->setEnabled(checked);
    _browseButton->setEnabled(checked);
    _cloudPathLabel->setEnabled(checked);
}

// ============================================================================
// SearchEngineSubDlg Implementation
// ============================================================================
SearchEngineSubDlg::SearchEngineSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void SearchEngineSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Search Engine group
    _searchEngineGroup = new QGroupBox(tr("Search Engine"), this);
    auto* searchEngineLayout = new QVBoxLayout(_searchEngineGroup);

    _searchEngineCombo = new QComboBox(_searchEngineGroup);
    populateSearchEngines();
    searchEngineLayout->addWidget(_searchEngineCombo);

    _customUrlLabel = new QLabel(tr("Custom search URL (use %s for search term):"), _searchEngineGroup);
    searchEngineLayout->addWidget(_customUrlLabel);

    _customUrlEdit = new QLineEdit(_searchEngineGroup);
    _customUrlEdit->setPlaceholderText(tr("https://www.example.com/search?q=%s"));
    searchEngineLayout->addWidget(_customUrlEdit);

    _setDefaultButton = new QPushButton(tr("Set as Default"), _searchEngineGroup);
    searchEngineLayout->addWidget(_setDefaultButton);

    mainLayout->addWidget(_searchEngineGroup);
    mainLayout->addStretch();
}

void SearchEngineSubDlg::populateSearchEngines()
{
    _searchEngineCombo->addItem(tr("Google"), "https://www.google.com/search?q=%s");
    _searchEngineCombo->addItem(tr("Bing"), "https://www.bing.com/search?q=%s");
    _searchEngineCombo->addItem(tr("DuckDuckGo"), "https://duckduckgo.com/?q=%s");
    _searchEngineCombo->addItem(tr("Yahoo"), "https://search.yahoo.com/search?p=%s");
    _searchEngineCombo->addItem(tr("Stack Overflow"), "https://stackoverflow.com/search?q=%s");
    _searchEngineCombo->addItem(tr("GitHub"), "https://github.com/search?q=%s");
    _searchEngineCombo->addItem(tr("Custom..."), "");
}

void SearchEngineSubDlg::connectSignals()
{
    connect(_searchEngineCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SearchEngineSubDlg::onSearchEngineChanged);
    connect(_customUrlEdit, &QLineEdit::textChanged, this, &SearchEngineSubDlg::onCustomUrlChanged);
    connect(_setDefaultButton, &QPushButton::clicked, this, &SearchEngineSubDlg::onSetDefaultClicked);
}

void SearchEngineSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _selectedSearchEngine = settings.readInt(L"SearchEngine", L"SelectedSearchEngine", 0);
    _customSearchUrl = QString::fromStdWString(settings.readString(L"SearchEngine", L"CustomSearchUrl", L""));

    _searchEngineCombo->setCurrentIndex(_selectedSearchEngine);
    _customUrlEdit->setText(_customSearchUrl);
    bool isCustom = (_selectedSearchEngine == _searchEngineCombo->count() - 1);
    _customUrlEdit->setEnabled(isCustom);
    _customUrlLabel->setEnabled(isCustom);
}

void SearchEngineSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeInt(L"SearchEngine", L"SelectedSearchEngine", _selectedSearchEngine);
    settings.writeString(L"SearchEngine", L"CustomSearchUrl", _customSearchUrl.toStdWString());
}

bool SearchEngineSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void SearchEngineSubDlg::onSearchEngineChanged(int index)
{
    _selectedSearchEngine = index;
    bool isCustom = (index == _searchEngineCombo->count() - 1);
    _customUrlEdit->setEnabled(isCustom);
    _customUrlLabel->setEnabled(isCustom);
}

void SearchEngineSubDlg::onCustomUrlChanged(const QString& url)
{
    _customSearchUrl = url;
}

void SearchEngineSubDlg::onSetDefaultClicked()
{
    // Save the current selection as default
    saveSettings();
    QMessageBox::information(this, tr("Search Engine"), tr("Search engine preference saved."));
}

// ============================================================================
// MISCSubDlg Implementation
// ============================================================================
MISCSubDlg::MISCSubDlg(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    loadSettings();
}

void MISCSubDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // General Misc group
    _generalMiscGroup = new QGroupBox(tr("General"), this);
    auto* generalMiscLayout = new QVBoxLayout(_generalMiscGroup);

    _minimizeToTrayCheck = new QCheckBox(tr("Minimize to system tray"), _generalMiscGroup);
    generalMiscLayout->addWidget(_minimizeToTrayCheck);

    _autoUpdateCheck = new QCheckBox(tr("Enable auto-updater"), _generalMiscGroup);
    generalMiscLayout->addWidget(_autoUpdateCheck);

    _enableNotificationsCheck = new QCheckBox(tr("Enable notifications"), _generalMiscGroup);
    generalMiscLayout->addWidget(_enableNotificationsCheck);

    _muteSoundsCheck = new QCheckBox(tr("Mute all sounds"), _generalMiscGroup);
    generalMiscLayout->addWidget(_muteSoundsCheck);

    _confirmExitCheck = new QCheckBox(tr("Confirm exit when there are unsaved changes"), _generalMiscGroup);
    generalMiscLayout->addWidget(_confirmExitCheck);

    _confirmDeleteCheck = new QCheckBox(tr("Confirm move to Recycle Bin"), _generalMiscGroup);
    generalMiscLayout->addWidget(_confirmDeleteCheck);

    mainLayout->addWidget(_generalMiscGroup);

    // File Detection group
    _fileDetectionGroup = new QGroupBox(tr("File Change Detection"), this);
    auto* fileDetectionLayout = new QVBoxLayout(_fileDetectionGroup);

    _fileAutoDetectionLabel = new QLabel(tr("File auto-detection:"), _fileDetectionGroup);
    fileDetectionLayout->addWidget(_fileAutoDetectionLabel);

    _fileAutoDetectionCombo = new QComboBox(_fileDetectionGroup);
    _fileAutoDetectionCombo->addItem(tr("Disable"));
    _fileAutoDetectionCombo->addItem(tr("Enable"));
    _fileAutoDetectionCombo->addItem(tr("Enable and update silently"));
    fileDetectionLayout->addWidget(_fileAutoDetectionCombo);

    mainLayout->addWidget(_fileDetectionGroup);
    mainLayout->addStretch();
}

void MISCSubDlg::connectSignals()
{
    connect(_minimizeToTrayCheck, &QCheckBox::toggled, this, &MISCSubDlg::onMinimizeToTrayToggled);
    connect(_autoUpdateCheck, &QCheckBox::toggled, this, &MISCSubDlg::onAutoUpdateToggled);
    connect(_enableNotificationsCheck, &QCheckBox::toggled, this, &MISCSubDlg::onEnableNotificationsToggled);
    connect(_fileAutoDetectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MISCSubDlg::onFileAutoDetectionChanged);
    connect(_muteSoundsCheck, &QCheckBox::toggled, this, &MISCSubDlg::onMuteSoundsToggled);
    connect(_confirmExitCheck, &QCheckBox::toggled, this, &MISCSubDlg::onConfirmExitToggled);
    connect(_confirmDeleteCheck, &QCheckBox::toggled, this, &MISCSubDlg::onConfirmDeleteToggled);
}

void MISCSubDlg::loadSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    _minimizeToTray = settings.readBool(L"MISC", L"MinimizeToTray", false);
    _autoUpdate = settings.readBool(L"MISC", L"AutoUpdate", true);
    _enableNotifications = settings.readBool(L"MISC", L"EnableNotifications", true);
    _fileAutoDetection = settings.readInt(L"MISC", L"FileAutoDetection", 1);
    _muteSounds = settings.readBool(L"MISC", L"MuteSounds", false);
    _confirmExit = settings.readBool(L"MISC", L"ConfirmExit", true);
    _confirmDelete = settings.readBool(L"MISC", L"ConfirmDelete", true);

    _minimizeToTrayCheck->setChecked(_minimizeToTray);
    _autoUpdateCheck->setChecked(_autoUpdate);
    _enableNotificationsCheck->setChecked(_enableNotifications);
    _fileAutoDetectionCombo->setCurrentIndex(_fileAutoDetection);
    _muteSoundsCheck->setChecked(_muteSounds);
    _confirmExitCheck->setChecked(_confirmExit);
    _confirmDeleteCheck->setChecked(_confirmDelete);
}

void MISCSubDlg::saveSettings()
{
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    settings.writeBool(L"MISC", L"MinimizeToTray", _minimizeToTray);
    settings.writeBool(L"MISC", L"AutoUpdate", _autoUpdate);
    settings.writeBool(L"MISC", L"EnableNotifications", _enableNotifications);
    settings.writeInt(L"MISC", L"FileAutoDetection", _fileAutoDetection);
    settings.writeBool(L"MISC", L"MuteSounds", _muteSounds);
    settings.writeBool(L"MISC", L"ConfirmExit", _confirmExit);
    settings.writeBool(L"MISC", L"ConfirmDelete", _confirmDelete);
}

bool MISCSubDlg::applySettings()
{
    saveSettings();
    return true;
}

void MISCSubDlg::onMinimizeToTrayToggled(bool checked)
{
    _minimizeToTray = checked;
}

void MISCSubDlg::onAutoUpdateToggled(bool checked)
{
    _autoUpdate = checked;
}

void MISCSubDlg::onEnableNotificationsToggled(bool checked)
{
    _enableNotifications = checked;
}

void MISCSubDlg::onFileAutoDetectionChanged(int index)
{
    _fileAutoDetection = index;
}

void MISCSubDlg::onMuteSoundsToggled(bool checked)
{
    _muteSounds = checked;
}

void MISCSubDlg::onConfirmExitToggled(bool checked)
{
    _confirmExit = checked;
}

void MISCSubDlg::onConfirmDeleteToggled(bool checked)
{
    _confirmDelete = checked;
}

// ============================================================================
// PreferenceDlg Implementation
// ============================================================================
PreferenceDlg::PreferenceDlg(QWidget* parent) : StaticDialog(parent)
{
    setupUI();
    _settings = &PlatformLayer::ISettings::getInstance();
}

PreferenceDlg::~PreferenceDlg()
{
    // Sub-pages are owned by the stacked widget and will be deleted automatically
}

void PreferenceDlg::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setWindowTitle(tr("Preferences"));
    dialog->setMinimumSize(700, 500);
    dialog->resize(750, 550);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Create horizontal splitter-like layout
    auto* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(10);

    // Left side - Category list
    _categoryList = new QListWidget(dialog);
    _categoryList->setMaximumWidth(200);
    _categoryList->setMinimumWidth(150);
    contentLayout->addWidget(_categoryList);

    // Right side - Stacked widget for pages
    _pagesStack = new QStackedWidget(dialog);
    contentLayout->addWidget(_pagesStack, 1);

    mainLayout->addLayout(contentLayout);

    // Separator line
    auto* line = new QFrame(dialog);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _okButton = new QPushButton(tr("OK"), dialog);
    _okButton->setDefault(true);
    buttonLayout->addWidget(_okButton);

    _cancelButton = new QPushButton(tr("Cancel"), dialog);
    buttonLayout->addWidget(_cancelButton);

    _applyButton = new QPushButton(tr("Apply"), dialog);
    buttonLayout->addWidget(_applyButton);

    mainLayout->addLayout(buttonLayout);

    // Store initial rect
    _rc = dialog->geometry();
}

void PreferenceDlg::createSubPages()
{
    // Create all sub-pages and add them to the stacked widget
    _generalPage = new GeneralSubDlg(_pagesStack);
    _pagesStack->addWidget(_generalPage);
    _categories.append(CategoryInfo(tr("General"), "General", _generalPage));

    _editingPage = new EditingSubDlg(_pagesStack);
    _pagesStack->addWidget(_editingPage);
    _categories.append(CategoryInfo(tr("Editing"), "Editing", _editingPage));

    _newDocumentPage = new NewDocumentSubDlg(_pagesStack);
    _pagesStack->addWidget(_newDocumentPage);
    _categories.append(CategoryInfo(tr("New Document"), "NewDoc", _newDocumentPage));

    _defaultDirectoryPage = new DefaultDirectorySubDlg(_pagesStack);
    _pagesStack->addWidget(_defaultDirectoryPage);
    _categories.append(CategoryInfo(tr("Default Directory"), "DefaultDir", _defaultDirectoryPage));

    _recentFilesHistoryPage = new RecentFilesHistorySubDlg(_pagesStack);
    _pagesStack->addWidget(_recentFilesHistoryPage);
    _categories.append(CategoryInfo(tr("Recent Files History"), "RecentFilesHistory", _recentFilesHistoryPage));

    _languagePage = new LanguageSubDlg(_pagesStack);
    _pagesStack->addWidget(_languagePage);
    _categories.append(CategoryInfo(tr("Language"), "Language", _languagePage));

    _highlightingPage = new HighlightingSubDlg(_pagesStack);
    _pagesStack->addWidget(_highlightingPage);
    _categories.append(CategoryInfo(tr("Highlighting"), "Highlighting", _highlightingPage));

    _printPage = new PrintSubDlg(_pagesStack);
    _pagesStack->addWidget(_printPage);
    _categories.append(CategoryInfo(tr("Print"), "Print", _printPage));

    _searchingPage = new SearchingSubDlg(_pagesStack);
    _pagesStack->addWidget(_searchingPage);
    _categories.append(CategoryInfo(tr("Searching"), "Searching", _searchingPage));

    _backupPage = new BackupSubDlg(_pagesStack);
    _pagesStack->addWidget(_backupPage);
    _categories.append(CategoryInfo(tr("Backup"), "Backup", _backupPage));

    _autoCompletionPage = new AutoCompletionSubDlg(_pagesStack);
    _pagesStack->addWidget(_autoCompletionPage);
    _categories.append(CategoryInfo(tr("Auto-Completion"), "AutoCompletion", _autoCompletionPage));

    _multiInstancePage = new MultiInstanceSubDlg(_pagesStack);
    _pagesStack->addWidget(_multiInstancePage);
    _categories.append(CategoryInfo(tr("Multi-Instance"), "MultiInstance", _multiInstancePage));

    _delimiterPage = new DelimiterSubDlg(_pagesStack);
    _pagesStack->addWidget(_delimiterPage);
    _categories.append(CategoryInfo(tr("Delimiter"), "Delimiter", _delimiterPage));

    _cloudLinkPage = new CloudLinkSubDlg(_pagesStack);
    _pagesStack->addWidget(_cloudLinkPage);
    _categories.append(CategoryInfo(tr("Cloud & Link"), "Cloud", _cloudLinkPage));

    _searchEnginePage = new SearchEngineSubDlg(_pagesStack);
    _pagesStack->addWidget(_searchEnginePage);
    _categories.append(CategoryInfo(tr("Search Engine"), "SearchEngine", _searchEnginePage));

    _miscPage = new MISCSubDlg(_pagesStack);
    _pagesStack->addWidget(_miscPage);
    _categories.append(CategoryInfo(tr("MISC"), "MISC", _miscPage));

    // Populate category list
    for (const auto& category : _categories) {
        _categoryList->addItem(category.name);
    }

    // Select first item
    if (_categoryList->count() > 0) {
        _categoryList->setCurrentRow(0);
        _pagesStack->setCurrentIndex(0);
    }
}

void PreferenceDlg::loadSettings()
{
    // Load settings for all pages
    if (_generalPage) _generalPage->loadSettings();
    if (_editingPage) _editingPage->loadSettings();
    if (_newDocumentPage) _newDocumentPage->loadSettings();
    if (_defaultDirectoryPage) _defaultDirectoryPage->loadSettings();
    if (_recentFilesHistoryPage) _recentFilesHistoryPage->loadSettings();
    if (_languagePage) _languagePage->loadSettings();
    if (_highlightingPage) _highlightingPage->loadSettings();
    if (_printPage) _printPage->loadSettings();
    if (_searchingPage) _searchingPage->loadSettings();
    if (_backupPage) _backupPage->loadSettings();
    if (_autoCompletionPage) _autoCompletionPage->loadSettings();
    if (_multiInstancePage) _multiInstancePage->loadSettings();
    if (_delimiterPage) _delimiterPage->loadSettings();
    if (_cloudLinkPage) _cloudLinkPage->loadSettings();
    if (_searchEnginePage) _searchEnginePage->loadSettings();
    if (_miscPage) _miscPage->loadSettings();
}

bool PreferenceDlg::saveSettings()
{
    // Apply settings from all pages
    bool success = true;

    if (_generalPage && !_generalPage->applySettings()) success = false;
    if (_editingPage && !_editingPage->applySettings()) success = false;
    if (_newDocumentPage && !_newDocumentPage->applySettings()) success = false;
    if (_defaultDirectoryPage && !_defaultDirectoryPage->applySettings()) success = false;
    if (_recentFilesHistoryPage && !_recentFilesHistoryPage->applySettings()) success = false;
    if (_languagePage && !_languagePage->applySettings()) success = false;
    if (_highlightingPage && !_highlightingPage->applySettings()) success = false;
    if (_printPage && !_printPage->applySettings()) success = false;
    if (_searchingPage && !_searchingPage->applySettings()) success = false;
    if (_backupPage && !_backupPage->applySettings()) success = false;
    if (_autoCompletionPage && !_autoCompletionPage->applySettings()) success = false;
    if (_multiInstancePage && !_multiInstancePage->applySettings()) success = false;
    if (_delimiterPage && !_delimiterPage->applySettings()) success = false;
    if (_cloudLinkPage && !_cloudLinkPage->applySettings()) success = false;
    if (_searchEnginePage && !_searchEnginePage->applySettings()) success = false;
    if (_miscPage && !_miscPage->applySettings()) success = false;

    // Save to persistent storage
    if (_settings) {
        _settings->saveConfig();
    }

    return success;
}

void PreferenceDlg::connectSignals()
{
    connect(_categoryList, &QListWidget::currentRowChanged, this, &PreferenceDlg::onCategoryChanged);
    connect(_okButton, &QPushButton::clicked, this, &PreferenceDlg::onOKClicked);
    connect(_cancelButton, &QPushButton::clicked, this, &PreferenceDlg::onCancelClicked);
    connect(_applyButton, &QPushButton::clicked, this, &PreferenceDlg::onApplyClicked);
}

void PreferenceDlg::doDialog()
{
    doDialog(false);
}

void PreferenceDlg::doDialog(bool /*isRTL*/)
{
    if (!isCreated()) {
        create(tr("Preferences"), false);
        setupUI();
        createSubPages();
        loadSettings();
        connectSignals();
    }

    goToCenter();
    display(true, true);
}

void PreferenceDlg::showPage(const QString& internalName)
{
    for (int i = 0; i < _categories.size(); ++i) {
        if (_categories[i].internalName == internalName) {
            showPage(i);
            return;
        }
    }
}

void PreferenceDlg::showPage(int index)
{
    if (index >= 0 && index < _categoryList->count()) {
        _categoryList->setCurrentRow(index);
        _pagesStack->setCurrentIndex(index);
    }
}

int PreferenceDlg::getCurrentPageIndex() const
{
    return _pagesStack->currentIndex();
}

void PreferenceDlg::onCategoryChanged(int index)
{
    if (index >= 0 && index < _pagesStack->count()) {
        _pagesStack->setCurrentIndex(index);
    }
}

void PreferenceDlg::onCategoryItemClicked(QListWidgetItem* item)
{
    if (!item)
        return;

    int index = _categoryList->row(item);
    if (index >= 0 && index < _pagesStack->count()) {
        _pagesStack->setCurrentIndex(index);
    }
}

void PreferenceDlg::onOKClicked()
{
    saveSettings();
    display(false);
}

void PreferenceDlg::onCancelClicked()
{
    // Discard changes and close
    display(false);
}

void PreferenceDlg::onApplyClicked()
{
    saveSettings();
}

bool PreferenceDlg::run_dlgProc(QEvent* /*event*/)
{
    // Handle any custom event processing here
    // Return true if event was handled, false otherwise
    return false;
}

} // namespace QtControls
