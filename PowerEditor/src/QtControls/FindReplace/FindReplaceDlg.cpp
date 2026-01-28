// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "FindReplaceDlg.h"
#include "../../Platform/Settings.h"
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
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QMenu>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QKeyEvent>

namespace Platform {

// Static members
FindOptions FindReplaceDlg::options;
FindOptions* FindReplaceDlg::env = &FindReplaceDlg::options;

FindReplaceDlg::FindReplaceDlg(QWidget* parent)
    : StaticDialog(parent) {
    setWindowTitle(tr("Find / Replace"));
    resize(500, 400);
}

FindReplaceDlg::~FindReplaceDlg() {
    saveHistory();
}

void FindReplaceDlg::init(ScintillaEditView** ppEditView) {
    _ppEditView = ppEditView;
    loadHistory();
    setupUI();
    connectSignals();
    updateControlStates();
}

void FindReplaceDlg::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Tab widget
    _tabWidget = new QTabWidget(this);
    mainLayout->addWidget(_tabWidget);

    // Create tabs
    createFindTab();
    createReplaceTab();
    createFindInFilesTab();
    createMarkTab();

    // Status label
    _statusLabel = new QLabel(this);
    _statusLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    _statusLabel->setMinimumHeight(24);
    mainLayout->addWidget(_statusLabel);

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _closeButton = new QPushButton(tr("Close"), this);
    buttonLayout->addWidget(_closeButton);

    mainLayout->addLayout(buttonLayout);
}

void FindReplaceDlg::createFindTab() {
    auto* findWidget = new QWidget();
    auto* layout = new QGridLayout(findWidget);
    layout->setSpacing(8);

    // Find what
    layout->addWidget(new QLabel(tr("Find what:")), 0, 0);
    _findWhatCombo = new QComboBox(this);
    _findWhatCombo->setEditable(true);
    _findWhatCombo->setMaxCount(20);
    _findWhatEdit = _findWhatCombo->lineEdit();
    layout->addWidget(_findWhatCombo, 0, 1, 1, 2);

    // Swap button
    _swapButton = new QPushButton(tr("⇄"), this);
    _swapButton->setToolTip(tr("Swap Find and Replace text"));
    layout->addWidget(_swapButton, 0, 3);

    // Match options
    _matchWholeWordCheck = new QCheckBox(tr("Match &whole word only"), this);
    layout->addWidget(_matchWholeWordCheck, 1, 0, 1, 2);

    _matchCaseCheck = new QCheckBox(tr("Match &case"), this);
    layout->addWidget(_matchCaseCheck, 2, 0, 1, 2);

    _wrapAroundCheck = new QCheckBox(tr("Wrap ar&ound"), this);
    _wrapAroundCheck->setChecked(true);
    layout->addWidget(_wrapAroundCheck, 3, 0, 1, 2);

    // Direction group
    _directionGroup = new QGroupBox(tr("Direction"), this);
    auto* dirLayout = new QHBoxLayout(_directionGroup);
    _dirUpRadio = new QRadioButton(tr("&Up"), this);
    _dirDownRadio = new QRadioButton(tr("&Down"), this);
    _dirDownRadio->setChecked(true);
    dirLayout->addWidget(_dirUpRadio);
    dirLayout->addWidget(_dirDownRadio);
    dirLayout->addStretch();
    layout->addWidget(_directionGroup, 1, 2, 3, 2);

    // Search mode group
    _searchModeGroup = new QGroupBox(tr("Search Mode"), this);
    auto* modeLayout = new QVBoxLayout(_searchModeGroup);
    _modeNormalRadio = new QRadioButton(tr("&Normal"), this);
    _modeExtendedRadio = new QRadioButton(tr("E&xtended (\\n, \\r, \\t, \\0, \\x...)"), this);
    _modeRegexRadio = new QRadioButton(tr("Re&gular expression"), this);
    _modeNormalRadio->setChecked(true);
    modeLayout->addWidget(_modeNormalRadio);
    modeLayout->addWidget(_modeExtendedRadio);
    modeLayout->addWidget(_modeRegexRadio);

    _dotMatchesNewlineCheck = new QCheckBox(tr(". matches &newline"), this);
    _dotMatchesNewlineCheck->setEnabled(false);
    modeLayout->addWidget(_dotMatchesNewlineCheck);
    modeLayout->addStretch();
    layout->addWidget(_searchModeGroup, 4, 0, 4, 2);

    // Scope
    _inSelectionCheck = new QCheckBox(tr("In select&ion"), this);
    layout->addWidget(_inSelectionCheck, 8, 0, 1, 2);

    // Action buttons
    auto* actionLayout = new QVBoxLayout();

    _findNextButton = new QPushButton(tr("Find &Next"), this);
    _findNextButton->setDefault(true);
    actionLayout->addWidget(_findNextButton);

    _findPreviousButton = new QPushButton(tr("Find &Previous"), this);
    actionLayout->addWidget(_findPreviousButton);

    _countButton = new QPushButton(tr("&Count"), this);
    actionLayout->addWidget(_countButton);

    _findAllButton = new QPushButton(tr("Find All in Current &Document"), this);
    actionLayout->addWidget(_findAllButton);

    actionLayout->addStretch();
    layout->addLayout(actionLayout, 4, 2, 5, 2);

    layout->setRowStretch(9, 1);
    layout->setColumnStretch(1, 1);

    _tabWidget->addTab(findWidget, tr("Find"));
}

void FindReplaceDlg::createReplaceTab() {
    auto* replaceWidget = new QWidget();
    auto* layout = new QGridLayout(replaceWidget);
    layout->setSpacing(8);

    // Find what
    layout->addWidget(new QLabel(tr("Find what:")), 0, 0);
    auto* findCombo = new QComboBox(this);
    findCombo->setEditable(true);
    layout->addWidget(findCombo, 0, 1);

    // Replace with
    layout->addWidget(new QLabel(tr("Replace with:")), 1, 0);
    _replaceWithCombo = new QComboBox(this);
    _replaceWithCombo->setEditable(true);
    _replaceWithCombo->setMaxCount(20);
    _replaceWithEdit = _replaceWithCombo->lineEdit();
    layout->addWidget(_replaceWithCombo, 1, 1);

    // Same options as Find tab (simplified - would share in real implementation)
    _matchWholeWordCheck = new QCheckBox(tr("Match whole word only"), this);
    layout->addWidget(_matchWholeWordCheck, 2, 0, 1, 2);

    _matchCaseCheck = new QCheckBox(tr("Match case"), this);
    layout->addWidget(_matchCaseCheck, 3, 0, 1, 2);

    _wrapAroundCheck = new QCheckBox(tr("Wrap around"), this);
    _wrapAroundCheck->setChecked(true);
    layout->addWidget(_wrapAroundCheck, 4, 0, 1, 2);

    _inSelectionCheck = new QCheckBox(tr("In selection"), this);
    layout->addWidget(_inSelectionCheck, 5, 0, 1, 2);

    // Action buttons
    auto* actionLayout = new QVBoxLayout();

    _findNextButton = new QPushButton(tr("Find Next"), this);
    _findNextButton->setDefault(true);
    actionLayout->addWidget(_findNextButton);

    _replaceButton = new QPushButton(tr("&Replace"), this);
    actionLayout->addWidget(_replaceButton);

    _replaceAllButton = new QPushButton(tr("Replace &All"), this);
    actionLayout->addWidget(_replaceAllButton);

    _replaceAllInOpenDocsButton = new QPushButton(tr("Replace All in &Open Documents"), this);
    actionLayout->addWidget(_replaceAllInOpenDocsButton);

    actionLayout->addStretch();
    layout->addLayout(actionLayout, 0, 2, 6, 1);

    layout->setRowStretch(6, 1);
    layout->setColumnStretch(1, 1);

    _tabWidget->addTab(replaceWidget, tr("Replace"));
}

void FindReplaceDlg::createFindInFilesTab() {
    auto* fifWidget = new QWidget();
    auto* layout = new QGridLayout(fifWidget);
    layout->setSpacing(8);

    // Find what
    layout->addWidget(new QLabel(tr("Find what:")), 0, 0);
    auto* findCombo = new QComboBox(this);
    findCombo->setEditable(true);
    layout->addWidget(findCombo, 0, 1);

    // Filters
    layout->addWidget(new QLabel(tr("Filters:")), 1, 0);
    _filtersCombo = new QComboBox(this);
    _filtersCombo->setEditable(true);
    _filtersCombo->setToolTip(tr("e.g. *.cpp *.h"));
    layout->addWidget(_filtersCombo, 1, 1);

    // Directory
    layout->addWidget(new QLabel(tr("Directory:")), 2, 0);
    _directoryCombo = new QComboBox(this);
    _directoryCombo->setEditable(true);
    layout->addWidget(_directoryCombo, 2, 1);

    auto* dirButtonLayout = new QVBoxLayout();
    _browseButton = new QPushButton(tr("..."), this);
    _browseButton->setToolTip(tr("Browse..."));
    dirButtonLayout->addWidget(_browseButton);

    _dirFromActiveDocButton = new QPushButton(tr("⬇"), this);
    _dirFromActiveDocButton->setToolTip(tr("Get directory from active document"));
    dirButtonLayout->addWidget(_dirFromActiveDocButton);
    dirButtonLayout->addStretch();
    layout->addLayout(dirButtonLayout, 2, 2);

    // Options
    _recursiveCheck = new QCheckBox(tr("Re&cursive"), this);
    _recursiveCheck->setChecked(true);
    layout->addWidget(_recursiveCheck, 3, 0);

    _hiddenDirCheck = new QCheckBox(tr("In &hidden folders"), this);
    layout->addWidget(_hiddenDirCheck, 3, 1);

    _matchCaseCheck = new QCheckBox(tr("Match case"), this);
    layout->addWidget(_matchCaseCheck, 4, 0);

    _matchWholeWordCheck = new QCheckBox(tr("Match whole word only"), this);
    layout->addWidget(_matchWholeWordCheck, 4, 1);

    // Action buttons
    auto* actionLayout = new QVBoxLayout();

    _findAllButton = new QPushButton(tr("Find All"), this);
    _findAllButton->setDefault(true);
    actionLayout->addWidget(_findAllButton);

    _replaceAllButton = new QPushButton(tr("Replace in Files"), this);
    actionLayout->addWidget(_replaceAllButton);

    actionLayout->addStretch();
    layout->addLayout(actionLayout, 0, 3, 5, 1);

    layout->setRowStretch(5, 1);
    layout->setColumnStretch(1, 1);

    _tabWidget->addTab(fifWidget, tr("Find in Files"));
}

void FindReplaceDlg::createMarkTab() {
    auto* markWidget = new QWidget();
    auto* layout = new QGridLayout(markWidget);
    layout->setSpacing(8);

    // Find what
    layout->addWidget(new QLabel(tr("Find what:")), 0, 0);
    auto* findCombo = new QComboBox(this);
    findCombo->setEditable(true);
    layout->addWidget(findCombo, 0, 1, 1, 2);

    // Options
    _bookmarkLineCheck = new QCheckBox(tr("Bookmark &line"), this);
    _bookmarkLineCheck->setChecked(true);
    layout->addWidget(_bookmarkLineCheck, 1, 0);

    _purgeCheck = new QCheckBox(tr("Purge for each search"), this);
    layout->addWidget(_purgeCheck, 1, 1);

    _matchCaseCheck = new QCheckBox(tr("Match case"), this);
    layout->addWidget(_matchCaseCheck, 2, 0);

    _matchWholeWordCheck = new QCheckBox(tr("Match whole word only"), this);
    layout->addWidget(_matchWholeWordCheck, 2, 1);

    // Action buttons
    auto* actionLayout = new QVBoxLayout();

    _markAllButton = new QPushButton(tr("Mark &All"), this);
    _markAllButton->setDefault(true);
    actionLayout->addWidget(_markAllButton);

    _clearMarksButton = new QPushButton(tr("Clear &all marks"), this);
    actionLayout->addWidget(_clearMarksButton);

    _copyMarkedTextButton = new QPushButton(tr("&Copy Marked Text"), this);
    actionLayout->addWidget(_copyMarkedTextButton);

    _findAllButton = new QPushButton(tr("Find All"), this);
    actionLayout->addWidget(_findAllButton);

    actionLayout->addStretch();
    layout->addLayout(actionLayout, 0, 3, 5, 1);

    layout->setRowStretch(5, 1);
    layout->setColumnStretch(1, 1);

    _tabWidget->addTab(markWidget, tr("Mark"));
}

void FindReplaceDlg::connectSignals() {
    // Tab changes
    connect(_tabWidget, &QTabWidget::currentChanged, this, &FindReplaceDlg::onTabChanged);

    // Buttons
    connect(_findNextButton, &QPushButton::clicked, this, &FindReplaceDlg::onFindNextClicked);
    connect(_findPreviousButton, &QPushButton::clicked, this, &FindReplaceDlg::onFindPreviousClicked);
    connect(_replaceButton, &QPushButton::clicked, this, &FindReplaceDlg::onReplaceClicked);
    connect(_replaceAllButton, &QPushButton::clicked, this, &FindReplaceDlg::onReplaceAllClicked);
    connect(_replaceAllInOpenDocsButton, &QPushButton::clicked, this, &FindReplaceDlg::onReplaceAllInOpenDocsClicked);
    connect(_findAllButton, &QPushButton::clicked, this, &FindReplaceDlg::onFindAllClicked);
    connect(_countButton, &QPushButton::clicked, this, &FindReplaceDlg::onCountClicked);
    connect(_markAllButton, &QPushButton::clicked, this, &FindReplaceDlg::onMarkAllClicked);
    connect(_clearMarksButton, &QPushButton::clicked, this, &FindReplaceDlg::onClearMarksClicked);
    connect(_closeButton, &QPushButton::clicked, this, &FindReplaceDlg::onCloseClicked);
    connect(_swapButton, &QPushButton::clicked, this, &FindReplaceDlg::onSwapFindReplaceClicked);

    // Browse
    connect(_browseButton, &QPushButton::clicked, this, &FindReplaceDlg::onBrowseDirectoryClicked);
    connect(_dirFromActiveDocButton, &QPushButton::clicked, this, &FindReplaceDlg::onDirFromActiveDocClicked);

    // Search mode
    connect(_modeRegexRadio, &QRadioButton::toggled, _dotMatchesNewlineCheck, &QWidget::setEnabled);

    // Text changes
    connect(_findWhatEdit, &QLineEdit::textChanged, this, &FindReplaceDlg::onSearchTextChanged);
    connect(_findWhatEdit, &QLineEdit::editingFinished, this, &FindReplaceDlg::onSearchTextEdited);
}

void FindReplaceDlg::showDialog(FindDialogType type) {
    _currentType = type;
    _tabWidget->setCurrentIndex(static_cast<int>(type));
    updateControlStates();
    show();
    raise();
    activateWindow();

    if (_findWhatEdit) {
        _findWhatEdit->setFocus();
        _findWhatEdit->selectAll();
    }
}

void FindReplaceDlg::setSearchText(const QString& text) {
    if (_findWhatEdit) {
        _findWhatEdit->setText(text);
    }
}

QString FindReplaceDlg::getSearchText() const {
    return _findWhatEdit ? _findWhatEdit->text() : QString();
}

QString FindReplaceDlg::getReplaceText() const {
    return _replaceWithEdit ? _replaceWithEdit->text() : QString();
}

FindOptions FindReplaceDlg::getCurrentOptions() const {
    FindOptions opt;

    // Search text
    opt.str2Search = getSearchText().toStdWString();
    opt.str4Replace = getReplaceText().toStdWString();

    // Match options
    opt.isMatchCase = _matchCaseCheck && _matchCaseCheck->isChecked();
    opt.isWholeWord = _matchWholeWordCheck && _matchWholeWordCheck->isChecked();
    opt.isWrapAround = _wrapAroundCheck && _wrapAroundCheck->isChecked();

    // Direction
    if (_dirUpRadio && _dirDownRadio) {
        opt.direction = _dirDownRadio->isChecked() ? SearchDirection::Down : SearchDirection::Up;
    }

    // Search type
    if (_modeNormalRadio && _modeNormalRadio->isChecked()) {
        opt.searchType = SearchType::Normal;
    } else if (_modeExtendedRadio && _modeExtendedRadio->isChecked()) {
        opt.searchType = SearchType::Extended;
    } else if (_modeRegexRadio && _modeRegexRadio->isChecked()) {
        opt.searchType = SearchType::Regex;
    }

    // Other options
    opt.isInSelection = _inSelectionCheck && _inSelectionCheck->isChecked();
    opt.dotMatchesNewline = _dotMatchesNewlineCheck && _dotMatchesNewlineCheck->isChecked();
    opt.isRecursive = _recursiveCheck && _recursiveCheck->isChecked();
    opt.isInHiddenDir = _hiddenDirCheck && _hiddenDirCheck->isChecked();
    opt.doPurge = _purgeCheck && _purgeCheck->isChecked();
    opt.doMarkLine = _bookmarkLineCheck && _bookmarkLineCheck->isChecked();

    // Filters and directory
    if (_filtersCombo) {
        opt.filters = _filtersCombo->currentText().toStdWString();
    }
    if (_directoryCombo) {
        opt.directory = _directoryCombo->currentText().toStdWString();
    }

    return opt;
}

void FindReplaceDlg::setOptions(const FindOptions& options) {
    if (_matchCaseCheck) _matchCaseCheck->setChecked(options.isMatchCase);
    if (_matchWholeWordCheck) _matchWholeWordCheck->setChecked(options.isWholeWord);
    if (_wrapAroundCheck) _wrapAroundCheck->setChecked(options.isWrapAround);
    if (_inSelectionCheck) _inSelectionCheck->setChecked(options.isInSelection);
    if (_dotMatchesNewlineCheck) _dotMatchesNewlineCheck->setChecked(options.dotMatchesNewline);
    if (_recursiveCheck) _recursiveCheck->setChecked(options.isRecursive);
    if (_hiddenDirCheck) _hiddenDirCheck->setChecked(options.isInHiddenDir);

    // Direction
    if (_dirUpRadio && _dirDownRadio) {
        if (options.direction == SearchDirection::Up) {
            _dirUpRadio->setChecked(true);
        } else {
            _dirDownRadio->setChecked(true);
        }
    }

    // Search type
    if (_modeNormalRadio) _modeNormalRadio->setChecked(options.searchType == SearchType::Normal);
    if (_modeExtendedRadio) _modeExtendedRadio->setChecked(options.searchType == SearchType::Extended);
    if (_modeRegexRadio) _modeRegexRadio->setChecked(options.searchType == SearchType::Regex);
}

bool FindReplaceDlg::findNext() {
    return processFindNext(getSearchText(), getCurrentOptions());
}

bool FindReplaceDlg::findPrevious() {
    auto options = getCurrentOptions();
    options.direction = SearchDirection::Up;
    return processFindNext(getSearchText(), options);
}

bool FindReplaceDlg::replace() {
    return processReplace(getSearchText(), getReplaceText(), getCurrentOptions());
}

bool FindReplaceDlg::replaceAll() {
    int count = processReplaceAll(getSearchText(), getReplaceText(), getCurrentOptions());
    return count > 0;
}

void FindReplaceDlg::onFindNextClicked() {
    if (findNext()) {
        setStatusMessage(tr("Match found"), FindStatus::Found);
    } else {
        setStatusMessage(tr("Match not found"), FindStatus::NotFound);
    }
}

void FindReplaceDlg::onFindPreviousClicked() {
    if (findPrevious()) {
        setStatusMessage(tr("Match found"), FindStatus::Found);
    } else {
        setStatusMessage(tr("Match not found"), FindStatus::NotFound);
    }
}

void FindReplaceDlg::onReplaceClicked() {
    if (replace()) {
        setStatusMessage(tr("Replaced"), FindStatus::Found);
    } else {
        setStatusMessage(tr("No match found to replace"), FindStatus::NotFound);
    }
}

void FindReplaceDlg::onReplaceAllClicked() {
    int count = processReplaceAll(getSearchText(), getReplaceText(), getCurrentOptions());
    if (count > 0) {
        setStatusMessage(tr("Replaced %1 occurrence(s)").arg(count), FindStatus::Found);
    } else {
        setStatusMessage(tr("No occurrences found"), FindStatus::NotFound);
    }
}

void FindReplaceDlg::onReplaceAllInOpenDocsClicked() {
    // TODO: Implement
    QMessageBox::information(this, tr("Not Implemented"), tr("Replace All in Open Documents is not yet implemented"));
}

void FindReplaceDlg::onFindAllClicked() {
    // TODO: Implement find all
    QMessageBox::information(this, tr("Not Implemented"), tr("Find All is not yet implemented"));
}

void FindReplaceDlg::onCountClicked() {
    int count = countMatches();
    setStatusMessage(tr("Count: %1 match(es)").arg(count), FindStatus::Message);
}

void FindReplaceDlg::onMarkAllClicked() {
    int count = markAll();
    setStatusMessage(tr("Marked %1 occurrence(s)").arg(count), FindStatus::Found);
}

void FindReplaceDlg::onClearMarksClicked() {
    clearMarks();
    setStatusMessage(tr("Marks cleared"), FindStatus::Message);
}

void FindReplaceDlg::onCloseClicked() {
    hide();
}

void FindReplaceDlg::onSwapFindReplaceClicked() {
    QString findText = getSearchText();
    QString replaceText = getReplaceText();
    setSearchText(replaceText);
    if (_replaceWithEdit) {
        _replaceWithEdit->setText(findText);
    }
}

void FindReplaceDlg::onSearchTextChanged(const QString& text) {
    (void)text;
    // Update button states based on whether search text is empty
    bool hasText = !_findWhatEdit->text().isEmpty();
    if (_findNextButton) _findNextButton->setEnabled(hasText);
    if (_replaceButton) _replaceButton->setEnabled(hasText);
    if (_replaceAllButton) _replaceAllButton->setEnabled(hasText);
}

void FindReplaceDlg::onSearchTextEdited() {
    addToHistory(getSearchText(), _findHistory);
    updateComboHistory(_findWhatCombo, _findHistory);
}

void FindReplaceDlg::onTabChanged(int index) {
    _currentType = static_cast<FindDialogType>(index);
    updateControlStates();
}

void FindReplaceDlg::onBrowseDirectoryClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
                                                    _directoryCombo ? _directoryCombo->currentText() : QString());
    if (!dir.isEmpty() && _directoryCombo) {
        _directoryCombo->setCurrentText(dir);
    }
}

void FindReplaceDlg::onDirFromActiveDocClicked() {
    // TODO: Get directory from active document
    QMessageBox::information(this, tr("Not Implemented"), tr("Get directory from active document is not yet implemented"));
}

void FindReplaceDlg::updateControlStates() {
    // Enable/disable controls based on current tab
    switch (_currentType) {
        case FindDialogType::Find:
            enableFindControls(true);
            break;
        case FindDialogType::Replace:
            enableReplaceControls(true);
            break;
        case FindDialogType::FindInFiles:
            enableFindInFilesControls(true);
            break;
        case FindDialogType::Mark:
            enableMarkControls(true);
            break;
        default:
            break;
    }
}

void FindReplaceDlg::enableFindControls(bool enable) {
    (void)enable;
    // Update UI state for find tab
}

void FindReplaceDlg::enableReplaceControls(bool enable) {
    (void)enable;
    // Update UI state for replace tab
}

void FindReplaceDlg::enableFindInFilesControls(bool enable) {
    (void)enable;
    // Update UI state for find in files tab
}

void FindReplaceDlg::enableMarkControls(bool enable) {
    (void)enable;
    // Update UI state for mark tab
}

void FindReplaceDlg::setStatusMessage(const QString& msg, FindStatus status) {
    _statusMessage = msg;
    _findStatus = status;
    if (_statusLabel) {
        _statusLabel->setText(msg);

        // Set color based on status
        QPalette palette = _statusLabel->palette();
        switch (status) {
            case FindStatus::Found:
                palette.setColor(QPalette::WindowText, Qt::darkGreen);
                break;
            case FindStatus::NotFound:
            case FindStatus::Warning:
                palette.setColor(QPalette::WindowText, Qt::red);
                break;
            default:
                palette.setColor(QPalette::WindowText, palette.color(QPalette::Text));
                break;
        }
        _statusLabel->setPalette(palette);
    }
}

void FindReplaceDlg::loadHistory() {
    // TODO: Load from settings
}

void FindReplaceDlg::saveHistory() {
    // TODO: Save to settings
}

void FindReplaceDlg::addToHistory(const QString& text, std::vector<QString>& history) {
    if (text.isEmpty()) return;

    // Remove if already exists
    auto it = std::find(history.begin(), history.end(), text);
    if (it != history.end()) {
        history.erase(it);
    }

    // Add to front
    history.insert(history.begin(), text);

    // Limit size
    if (history.size() > 20) {
        history.resize(20);
    }
}

void FindReplaceDlg::updateComboHistory(QComboBox* combo, const std::vector<QString>& history) {
    if (!combo) return;

    QString currentText = combo->currentText();
    combo->clear();

    for (const auto& item : history) {
        combo->addItem(item);
    }

    combo->setCurrentText(currentText);
}

bool FindReplaceDlg::processFindNext(const QString& text, const FindOptions& options) {
    if (!_ppEditView || !*_ppEditView) return false;

    auto* view = *_ppEditView;
    if (!view->isVisible()) return false;

    // Build search flags
    int flags = buildSearchFlags(options);

    // Get current position
    intptr_t currentPos = view->execute(SCI_GETCURRENTPOS);
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    // Set search range based on direction
    intptr_t startPos, endPos;
    if (options.direction == SearchDirection::Down) {
        startPos = currentPos;
        endPos = docLength;
    } else {
        startPos = currentPos;
        endPos = 0;
    }

    // Convert text to proper encoding
    QByteArray searchText = text.toUtf8();

    // Set target
    view->execute(SCI_SETSEARCHFLAGS, flags);
    view->execute(SCI_SETTARGETSTART, startPos);
    view->execute(SCI_SETTARGETEND, endPos);

    // Perform search
    intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                    reinterpret_cast<LPARAM>(searchText.constData()));

    if (result == -1) {
        // Not found - try wrap around if enabled
        if (options.isWrapAround) {
            if (options.direction == SearchDirection::Down) {
                startPos = 0;
                endPos = currentPos;
            } else {
                startPos = docLength;
                endPos = currentPos;
            }

            view->execute(SCI_SETTARGETSTART, startPos);
            view->execute(SCI_SETTARGETEND, endPos);

            result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                   reinterpret_cast<LPARAM>(searchText.constData()));
        }
    }

    if (result != -1) {
        // Found - select and show
        intptr_t matchEnd = view->execute(SCI_GETTARGETEND);
        view->execute(SCI_SETSEL, result, matchEnd);
        displaySectionCentered(result, matchEnd, options.direction == SearchDirection::Down);
        return true;
    }

    return false;
}

bool FindReplaceDlg::processReplace(const QString& findText, const QString& replaceText, const FindOptions& options) {
    if (!_ppEditView || !*_ppEditView) return false;

    auto* view = *_ppEditView;

    // Check if there's a selection that matches
    intptr_t selStart = view->execute(SCI_GETSELECTIONSTART);
    intptr_t selEnd = view->execute(SCI_GETSELECTIONEND);

    if (selStart == selEnd) {
        // No selection - find first
        if (!processFindNext(findText, options)) {
            return false;
        }
        selStart = view->execute(SCI_GETSELECTIONSTART);
        selEnd = view->execute(SCI_GETSELECTIONEND);
    }

    // Replace the selection
    QByteArray replacement = replaceText.toUtf8();
    view->execute(SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(replacement.constData()));

    // Find next occurrence
    processFindNext(findText, options);

    return true;
}

int FindReplaceDlg::processReplaceAll(const QString& findText, const QString& replaceText, const FindOptions& options) {
    if (!_ppEditView || !*_ppEditView) return false;

    auto* view = *_ppEditView;
    int flags = buildSearchFlags(options);

    QByteArray searchText = findText.toUtf8();
    QByteArray replacement = replaceText.toUtf8();

    view->execute(SCI_BEGINUNDOACTION);

    int count = 0;
    intptr_t startPos = 0;
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    view->execute(SCI_SETSEARCHFLAGS, flags);

    while (true) {
        view->execute(SCI_SETTARGETSTART, startPos);
        view->execute(SCI_SETTARGETEND, docLength);

        intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                        reinterpret_cast<LPARAM>(searchText.constData()));

        if (result == -1) {
            break;
        }

        // Replace
        view->execute(SCI_REPLACETARGET, replacement.length(),
                      reinterpret_cast<LPARAM>(replacement.constData()));

        count++;

        // Update position for next search
        startPos = result + replacement.length();
        docLength = view->execute(SCI_GETLENGTH);
    }

    view->execute(SCI_ENDUNDOACTION);

    return count;
}

int FindReplaceDlg::countMatches() {
    if (!_ppEditView || !*_ppEditView) return 0;

    auto* view = *_ppEditView;
    auto options = getCurrentOptions();
    int flags = buildSearchFlags(options);

    QByteArray searchText = getSearchText().toUtf8();

    int count = 0;
    intptr_t startPos = 0;
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    view->execute(SCI_SETSEARCHFLAGS, flags);

    while (true) {
        view->execute(SCI_SETTARGETSTART, startPos);
        view->execute(SCI_SETTARGETEND, docLength);

        intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                        reinterpret_cast<LPARAM>(searchText.constData()));

        if (result == -1) {
            break;
        }

        count++;
        startPos = view->execute(SCI_GETTARGETEND);
    }

    return count;
}

int FindReplaceDlg::markAll(int styleId) {
    if (!_ppEditView || !*_ppEditView) return 0;

    auto* view = *_ppEditView;
    auto options = getCurrentOptions();
    int flags = buildSearchFlags(options);

    QByteArray searchText = getSearchText().toUtf8();

    // Use bookmark or indicator
    int marker = (styleId < 0) ? 1 : styleId;  // Default bookmark marker

    int count = 0;
    intptr_t startPos = 0;
    intptr_t docLength = view->execute(SCI_GETLENGTH);

    view->execute(SCI_SETSEARCHFLAGS, flags);

    while (true) {
        view->execute(SCI_SETTARGETSTART, startPos);
        view->execute(SCI_SETTARGETEND, docLength);

        intptr_t result = view->execute(SCI_SEARCHINTARGET, searchText.length(),
                                        reinterpret_cast<LPARAM>(searchText.constData()));

        if (result == -1) {
            break;
        }

        // Mark the line
        intptr_t line = view->execute(SCI_LINEFROMPOSITION, result);
        view->execute(SCI_MARKERADD, line, marker);

        count++;
        startPos = view->execute(SCI_GETTARGETEND);
    }

    return count;
}

void FindReplaceDlg::clearMarks() {
    if (!_ppEditView || !*_ppEditView) return;

    auto* view = *_ppEditView;

    // Clear all markers
    view->execute(SCI_MARKERDELETEALL, -1);
}

int FindReplaceDlg::buildSearchFlags(const FindOptions& options) const {
    int flags = 0;

    if (options.isWholeWord) {
        flags |= SCFIND_WHOLEWORD;
    }
    if (options.isMatchCase) {
        flags |= SCFIND_MATCHCASE;
    }
    if (options.searchType == SearchType::Regex) {
        flags |= SCFIND_REGEXP | SCFIND_POSIX;
        if (options.dotMatchesNewline) {
            flags |= SCFIND_REGEXP_DOTMATCHESNL;
        }
    }

    return flags;
}

void FindReplaceDlg::displaySectionCentered(intptr_t start, intptr_t end, bool isDownwards) {
    (void)end;
    (void)isDownwards;

    if (!_ppEditView || !*_ppEditView) return;

    auto* view = *_ppEditView;

    // Make sure the position is visible
    view->execute(SCI_ENSUREVISIBLE, view->execute(SCI_LINEFROMPOSITION, start));
    view->execute(SCI_SCROLLCARET);
}

void FindReplaceDlg::resizeEvent(QResizeEvent* event) {
    StaticDialog::resizeEvent(event);
    // Adjust layout if needed
}

// ============================================================================
// FindIncrementDlg implementation
// ============================================================================

FindIncrementDlg::FindIncrementDlg(QWidget* parent)
    : StaticDialog(parent) {
    setWindowTitle(tr("Incremental Search"));
}

FindIncrementDlg::~FindIncrementDlg() = default;

void FindIncrementDlg::init(FindReplaceDlg* pFRDlg, ScintillaEditView** ppEditView) {
    _pFRDlg = pFRDlg;
    _ppEditView = ppEditView;
    setupUI();
    connectSignals();
}

void FindIncrementDlg::setupUI() {
    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(4, 2, 4, 2);

    _searchEdit = new QLineEdit(this);
    _searchEdit->setMinimumWidth(200);
    layout->addWidget(_searchEdit);

    _statusLabel = new QLabel(this);
    _statusLabel->setMinimumWidth(80);
    layout->addWidget(_statusLabel);

    _nextButton = new QPushButton(tr("▲"), this);
    _nextButton->setToolTip(tr("Find Next"));
    _nextButton->setFixedSize(24, 24);
    layout->addWidget(_nextButton);

    _prevButton = new QPushButton(tr("▼"), this);
    _prevButton->setToolTip(tr("Find Previous"));
    _prevButton->setFixedSize(24, 24);
    layout->addWidget(_prevButton);

    _highlightButton = new QPushButton(tr("*"), this);
    _highlightButton->setToolTip(tr("Highlight all"));
    _highlightButton->setFixedSize(24, 24);
    _highlightButton->setCheckable(true);
    layout->addWidget(_highlightButton);

    _caseSensitiveCheck = new QCheckBox(tr("Aa"), this);
    _caseSensitiveCheck->setToolTip(tr("Match case"));
    layout->addWidget(_caseSensitiveCheck);

    _wholeWordCheck = new QCheckBox(tr("\u00ab\u00bb"), this);
    _wholeWordCheck->setToolTip(tr("Match whole word only"));
    layout->addWidget(_wholeWordCheck);

    _regexCheck = new QCheckBox(tr(".*"), this);
    _regexCheck->setToolTip(tr("Regular expression"));
    layout->addWidget(_regexCheck);
}

void FindIncrementDlg::connectSignals() {
    connect(_searchEdit, &QLineEdit::textChanged, this, &FindIncrementDlg::onTextChanged);
    connect(_nextButton, &QPushButton::clicked, this, &FindIncrementDlg::onFindNext);
    connect(_prevButton, &QPushButton::clicked, this, &FindIncrementDlg::onFindPrevious);
    connect(_highlightButton, &QPushButton::toggled, this, &FindIncrementDlg::onHighlightAll);
}

void FindIncrementDlg::setSearchText(const QString& text) {
    if (_searchEdit) {
        _searchEdit->setText(text);
    }
}

void FindIncrementDlg::setFindStatus(FindStatus status, int count) {
    _findStatus = status;
    QString msg;
    switch (status) {
        case FindStatus::Found:
            msg = (count > 0) ? tr("%1 matches").arg(count) : tr("Found");
            break;
        case FindStatus::NotFound:
            msg = tr("Not found");
            break;
        case FindStatus::EndReached:
            msg = tr("Wrapped");
            break;
        default:
            msg = QString();
            break;
    }
    if (_statusLabel) {
        _statusLabel->setText(msg);
    }
}

void FindIncrementDlg::onTextChanged(const QString& text) {
    if (text.isEmpty()) {
        setFindStatus(FindStatus::NoMessage, 0);
        return;
    }

    // Perform incremental search
    if (_pFRDlg) {
        FindOptions opt = _pFRDlg->getCurrentOptions();
        opt.isMatchCase = _caseSensitiveCheck->isChecked();
        opt.isWholeWord = _wholeWordCheck->isChecked();
        opt.searchType = _regexCheck->isChecked() ? SearchType::Regex : SearchType::Normal;

        bool found = _pFRDlg->processFindNext(text, opt);
        setFindStatus(found ? FindStatus::Found : FindStatus::NotFound, 0);
    }
}

void FindIncrementDlg::onFindNext() {
    if (_pFRDlg) {
        _pFRDlg->findNext();
    }
}

void FindIncrementDlg::onFindPrevious() {
    if (_pFRDlg) {
        _pFRDlg->findPrevious();
    }
}

void FindIncrementDlg::onHighlightAll() {
    // TODO: Highlight all matches
}

void FindIncrementDlg::onCaseSensitiveToggled(bool checked) {
    (void)checked;
    // Update search
    onTextChanged(_searchEdit->text());
}

void FindIncrementDlg::onWholeWordToggled(bool checked) {
    (void)checked;
    // Update search
    onTextChanged(_searchEdit->text());
}

void FindIncrementDlg::onRegexToggled(bool checked) {
    (void)checked;
    // Update search
    onTextChanged(_searchEdit->text());
}

// ============================================================================
// FindProgressDlg implementation
// ============================================================================

FindProgressDlg::FindProgressDlg(QWidget* parent)
    : StaticDialog(parent) {
    setWindowTitle(tr("Search Progress"));
    setModal(true);
}

FindProgressDlg::~FindProgressDlg() = default;

void FindProgressDlg::setupUI() {
    auto* layout = new QVBoxLayout(this);

    _headerLabel = new QLabel(this);
    _headerLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(_headerLabel);

    _infoLabel = new QLabel(this);
    layout->addWidget(_infoLabel);

    _progressBar = new QProgressBar(this);
    _progressBar->setRange(0, 100);
    layout->addWidget(_progressBar);

    _hitsLabel = new QLabel(tr("Hits: 0"), this);
    layout->addWidget(_hitsLabel);

    _cancelButton = new QPushButton(tr("Cancel"), this);
    layout->addWidget(_cancelButton, 0, Qt::AlignCenter);
}

void FindProgressDlg::connectSignals() {
    connect(_cancelButton, &QPushButton::clicked, this, &FindProgressDlg::onCancel);
}

void FindProgressDlg::setHeader(const QString& header) {
    if (_headerLabel) {
        _headerLabel->setText(header);
    }
}

void FindProgressDlg::setPercent(int percent) {
    if (_progressBar) {
        _progressBar->setValue(percent);
    }
}

void FindProgressDlg::setInfo(const QString& info) {
    if (_infoLabel) {
        _infoLabel->setText(info);
    }
}

void FindProgressDlg::setHitsCount(int count) {
    if (_hitsLabel) {
        _hitsLabel->setText(tr("Hits: %1").arg(count));
    }
}

void FindProgressDlg::onCancel() {
    _cancelled = true;
    hide();
}

void FindProgressDlg::closeEvent(QCloseEvent* event) {
    _cancelled = true;
    StaticDialog::closeEvent(event);
}

// ============================================================================
// FinderPanel implementation
// ============================================================================

FinderPanel::FinderPanel(QWidget* parent)
    : StaticDialog(parent) {
    setWindowTitle(tr("Find result"));
}

FinderPanel::~FinderPanel() = default;

void FinderPanel::init(ScintillaEditView** ppEditView) {
    _ppEditView = ppEditView;
    setupUI();
    connectSignals();
}

void FinderPanel::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // TODO: Create ScintillaEditView for results
    // For now, just a placeholder
    auto* label = new QLabel(tr("Search results will appear here"), this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}

void FinderPanel::connectSignals() {
    // TODO: Connect signals
}

void FinderPanel::addSearchLine(const QString& searchText) {
    (void)searchText;
    // TODO: Add search line to results
}

void FinderPanel::addFileNameTitle(const QString& fileName) {
    (void)fileName;
    // TODO: Add file name
}

void FinderPanel::addFileHitCount(int count) {
    (void)count;
    // TODO: Add hit count
}

void FinderPanel::addSearchResult(const QString& line, int lineNumber, intptr_t start, intptr_t end) {
    (void)line;
    (void)lineNumber;
    (void)start;
    (void)end;
    // TODO: Add result
}

void FinderPanel::beginNewFilesSearch() {
    _foundInfos.clear();
    _currentFileCount = 0;
    // TODO: Clear display
}

void FinderPanel::finishFilesSearch(int count, int searchedCount) {
    (void)count;
    (void)searchedCount;
    // TODO: Show summary
}

void FinderPanel::removeAll() {
    _foundInfos.clear();
    // TODO: Clear display
}

void FinderPanel::openAll() {
    // TODO: Open all files with results
}

void FinderPanel::copy() {
    // TODO: Copy results to clipboard
}

void FinderPanel::copyPathnames() {
    // TODO: Copy file paths
}

void FinderPanel::gotoNextResult() {
    // TODO: Go to next result
}

void FinderPanel::gotoPreviousResult() {
    // TODO: Go to previous result
}

void FinderPanel::setFinderStyle() {
    // TODO: Set syntax highlighting style
}

void FinderPanel::onResultDoubleClicked(int line) {
    (void)line;
    // TODO: Navigate to result
}

void FinderPanel::onContextMenu(const QPoint& pos) {
    (void)pos;
    // TODO: Show context menu
}

void FinderPanel::gotoFoundLine(size_t index) {
    if (index >= _foundInfos.size()) return;

    const auto& info = _foundInfos[index];
    if (!_ppEditView || !*_ppEditView) return;

    auto* view = *_ppEditView;
    view->execute(SCI_ENSUREVISIBLE, info.lineNumber - 1);
    view->execute(SCI_GOTOPOS, info.start);
    view->execute(SCI_SETSEL, info.start, info.end);
}

} // namespace Platform
