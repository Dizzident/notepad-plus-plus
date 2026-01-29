// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "FunctionListPanel.h"
#include "../TreeView/TreeView.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QTreeWidget>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTextBlock>
#include <QApplication>
#include <QHeaderView>
#include <QDebug>
#include <QFileInfo>

namespace QtControls {

// ============================================================================
// FunctionParserManager Implementation
// ============================================================================
FunctionParserManager::FunctionParserManager()
{
    registerDefaultParsers();
}

void FunctionParserManager::registerDefaultParsers()
{
    registerParser(std::make_unique<CppFunctionParser>());
    registerParser(std::make_unique<PythonFunctionParser>());
    registerParser(std::make_unique<JavaScriptFunctionParser>());
    registerParser(std::make_unique<JavaFunctionParser>());
    registerParser(std::make_unique<CSharpFunctionParser>());
}

void FunctionParserManager::registerParser(std::unique_ptr<FunctionParser> parser)
{
    _parsers.push_back(std::move(parser));
}

FunctionParser* FunctionParserManager::getParser(const QString& language) const
{
    for (const auto& parser : _parsers) {
        if (parser->getLanguageId().compare(language, Qt::CaseInsensitive) == 0 ||
            parser->getDisplayName().compare(language, Qt::CaseInsensitive) == 0) {
            return parser.get();
        }
    }
    return nullptr;
}

FunctionParser* FunctionParserManager::getParserForExtension(const QString& ext) const
{
    static const QHash<QString, QString> extToLang = {
        {"cpp", "cpp"}, {"c", "cpp"}, {"h", "cpp"}, {"hpp", "cpp"},
        {"cc", "cpp"}, {"cxx", "cpp"},
        {"py", "python"}, {"pyw", "python"},
        {"js", "javascript"}, {"jsx", "javascript"}, {"ts", "javascript"}, {"tsx", "javascript"},
        {"java", "java"},
        {"cs", "csharp"}
    };

    QString lang = extToLang.value(ext.toLower(), QString());
    if (!lang.isEmpty()) {
        return getParser(lang);
    }
    return nullptr;
}

QStringList FunctionParserManager::getAvailableParsers() const
{
    QStringList names;
    for (const auto& parser : _parsers) {
        names.append(parser->getDisplayName());
    }
    return names;
}

// ============================================================================
// CppFunctionParser Implementation
// ============================================================================
std::vector<FunctionInfo> CppFunctionParser::parse(const QString& content)
{
    std::vector<FunctionInfo> results;

    // Pattern for C++ class/struct definitions
    QRegularExpression classRegex(
        R"((class|struct)\s+(\w+)\s*(?::\s*(?:public|protected|private)\s+\w+\s*)?\{)",
        QRegularExpression::MultilineOption);

    // Pattern for C++ function definitions
    // Matches: return_type function_name(parameters) {
    QRegularExpression funcRegex(
        R"((\w[\w\s\*\&\:]+)\s+(\w+)\s*\(([^)]*)\)\s*(?:const\s*)?\{)",
        QRegularExpression::MultilineOption);

    // Pattern for constructors/destructors
    QRegularExpression ctorRegex(
        R"((\w+)\s*\(([^)]*)\)\s*(?::\s*\w+\([^)]*\)\s*)*\{)",
        QRegularExpression::MultilineOption);

    QStringList lines = content.split('\n');

    // Find classes first
    QRegularExpressionMatchIterator classIt = classRegex.globalMatch(content);
    while (classIt.hasNext()) {
        QRegularExpressionMatch match = classIt.next();
        FunctionInfo info;
        info.type = match.captured(1); // "class" or "struct"
        info.name = match.captured(2);
        info.displayName = info.name;
        info.pos = match.capturedStart();

        // Calculate line number
        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    // Find functions
    QRegularExpressionMatchIterator funcIt = funcRegex.globalMatch(content);
    while (funcIt.hasNext()) {
        QRegularExpressionMatch match = funcIt.next();
        FunctionInfo info;
        info.returnType = match.captured(1).trimmed();
        info.name = match.captured(2);
        info.parameters = match.captured(3).trimmed();
        info.displayName = info.name + "(" + info.parameters + ")";
        info.type = "function";
        info.pos = match.capturedStart();

        // Skip if it looks like a control statement
        if (info.name == "if" || info.name == "while" || info.name == "for" ||
            info.name == "switch" || info.name == "catch") {
            continue;
        }

        // Calculate line number
        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    return results;
}

// ============================================================================
// PythonFunctionParser Implementation
// ============================================================================
std::vector<FunctionInfo> PythonFunctionParser::parse(const QString& content)
{
    std::vector<FunctionInfo> results;

    // Pattern for Python function definitions
    QRegularExpression funcRegex(
        R"(def\s+(\w+)\s*\(([^)]*)\)\s*(?:->\s*([^:]+))?:)",
        QRegularExpression::MultilineOption);

    // Pattern for Python class definitions
    QRegularExpression classRegex(
        R"(class\s+(\w+)\s*(?:\(([^)]*)\))?:)",
        QRegularExpression::MultilineOption);

    QStringList lines = content.split('\n');

    // Find classes
    QRegularExpressionMatchIterator classIt = classRegex.globalMatch(content);
    while (classIt.hasNext()) {
        QRegularExpressionMatch match = classIt.next();
        FunctionInfo info;
        info.type = "class";
        info.name = match.captured(1);
        info.displayName = info.name;
        info.pos = match.capturedStart();

        // Calculate line number
        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    // Find functions
    QRegularExpressionMatchIterator funcIt = funcRegex.globalMatch(content);
    while (funcIt.hasNext()) {
        QRegularExpressionMatch match = funcIt.next();
        FunctionInfo info;
        info.type = "function";
        info.name = match.captured(1);
        info.parameters = match.captured(2).trimmed();
        info.returnType = match.captured(3).trimmed();
        info.displayName = info.name + "(" + info.parameters + ")";
        info.pos = match.capturedStart();

        // Calculate line number
        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    return results;
}

// ============================================================================
// JavaScriptFunctionParser Implementation
// ============================================================================
std::vector<FunctionInfo> JavaScriptFunctionParser::parse(const QString& content)
{
    std::vector<FunctionInfo> results;

    // Pattern for function declarations: function name(params) {
    QRegularExpression funcDeclRegex(
        R"(function\s+(\w+)\s*\(([^)]*)\)\s*\{)",
        QRegularExpression::MultilineOption);

    // Pattern for arrow functions with const: const name = (params) =>
    QRegularExpression arrowFuncRegex(
        R"((?:const|let|var)\s+(\w+)\s*=\s*(?:\(([^)]*)\)|(\w+))\s*=>)",
        QRegularExpression::MultilineOption);

    // Pattern for method definitions: name(params) {
    QRegularExpression methodRegex(
        R"((\w+)\s*\(([^)]*)\)\s*\{)",
        QRegularExpression::MultilineOption);

    // Pattern for class definitions
    QRegularExpression classRegex(
        R"(class\s+(\w+)\s*(?:extends\s+\w+\s*)?\{)",
        QRegularExpression::MultilineOption);

    QStringList lines = content.split('\n');

    // Find classes
    QRegularExpressionMatchIterator classIt = classRegex.globalMatch(content);
    while (classIt.hasNext()) {
        QRegularExpressionMatch match = classIt.next();
        FunctionInfo info;
        info.type = "class";
        info.name = match.captured(1);
        info.displayName = info.name;
        info.pos = match.capturedStart();

        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    // Find function declarations
    QRegularExpressionMatchIterator funcIt = funcDeclRegex.globalMatch(content);
    while (funcIt.hasNext()) {
        QRegularExpressionMatch match = funcIt.next();
        FunctionInfo info;
        info.type = "function";
        info.name = match.captured(1);
        info.parameters = match.captured(2).trimmed();
        info.displayName = info.name + "(" + info.parameters + ")";
        info.pos = match.capturedStart();

        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    // Find arrow functions
    QRegularExpressionMatchIterator arrowIt = arrowFuncRegex.globalMatch(content);
    while (arrowIt.hasNext()) {
        QRegularExpressionMatch match = arrowIt.next();
        FunctionInfo info;
        info.type = "function";
        info.name = match.captured(1);
        info.parameters = match.captured(2).isEmpty() ? match.captured(3) : match.captured(2);
        info.displayName = info.name + "(" + info.parameters + ")";
        info.pos = match.capturedStart();

        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    return results;
}

// ============================================================================
// JavaFunctionParser Implementation
// ============================================================================
std::vector<FunctionInfo> JavaFunctionParser::parse(const QString& content)
{
    std::vector<FunctionInfo> results;

    // Pattern for Java class definitions
    QRegularExpression classRegex(
        R"((?:public\s+|private\s+|protected\s+)?(?:abstract\s+|final\s+)?class\s+(\w+))",
        QRegularExpression::MultilineOption);

    // Pattern for Java method definitions
    QRegularExpression methodRegex(
        R"((?:public|private|protected|static|\s)+(?:\w[\w\s\<\>\[\]]+)\s+(\w+)\s*\(([^)]*)\)\s*(?:throws\s+\w+(?:\s*,\s*\w+)*)?\s*\{)",
        QRegularExpression::MultilineOption);

    QStringList lines = content.split('\n');

    // Find classes
    QRegularExpressionMatchIterator classIt = classRegex.globalMatch(content);
    while (classIt.hasNext()) {
        QRegularExpressionMatch match = classIt.next();
        FunctionInfo info;
        info.type = "class";
        info.name = match.captured(1);
        info.displayName = info.name;
        info.pos = match.capturedStart();

        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    // Find methods
    QRegularExpressionMatchIterator methodIt = methodRegex.globalMatch(content);
    while (methodIt.hasNext()) {
        QRegularExpressionMatch match = methodIt.next();
        FunctionInfo info;
        info.type = "method";
        info.name = match.captured(1);
        info.parameters = match.captured(2).trimmed();
        info.displayName = info.name + "(" + info.parameters + ")";
        info.pos = match.capturedStart();

        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    return results;
}

// ============================================================================
// CSharpFunctionParser Implementation
// ============================================================================
std::vector<FunctionInfo> CSharpFunctionParser::parse(const QString& content)
{
    std::vector<FunctionInfo> results;

    // Pattern for C# class definitions
    QRegularExpression classRegex(
        R"((?:public|private|protected|internal|static|abstract|sealed|partial\s+)*class\s+(\w+))",
        QRegularExpression::MultilineOption);

    // Pattern for C# method definitions
    QRegularExpression methodRegex(
        R"((?:public|private|protected|internal|static|virtual|abstract|override|sealed|async\s+)+\s*(?:\w[\w\s\<\>\[\]\?]*)\s+(\w+)\s*\(([^)]*)\)\s*(?:where\s+\w+\s*:\s*\w+)?\s*\{?)",
        QRegularExpression::MultilineOption);

    QStringList lines = content.split('\n');

    // Find classes
    QRegularExpressionMatchIterator classIt = classRegex.globalMatch(content);
    while (classIt.hasNext()) {
        QRegularExpressionMatch match = classIt.next();
        FunctionInfo info;
        info.type = "class";
        info.name = match.captured(1);
        info.displayName = info.name;
        info.pos = match.capturedStart();

        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    // Find methods
    QRegularExpressionMatchIterator methodIt = methodRegex.globalMatch(content);
    while (methodIt.hasNext()) {
        QRegularExpressionMatch match = methodIt.next();
        FunctionInfo info;
        info.type = "method";
        info.name = match.captured(1);
        info.parameters = match.captured(2).trimmed();
        info.displayName = info.name + "(" + info.parameters + ")";
        info.pos = match.capturedStart();

        int lineNum = 1;
        for (int i = 0; i < info.pos && i < content.length(); ++i) {
            if (content[i] == '\n') lineNum++;
        }
        info.lineNumber = lineNum;

        results.push_back(info);
    }

    return results;
}

// ============================================================================
// FunctionListPanel Implementation
// ============================================================================
FunctionListPanel::FunctionListPanel(QWidget* parent)
    : StaticDialog(parent)
    , _parserMgr(std::make_unique<FunctionParserManager>())
{
}

FunctionListPanel::~FunctionListPanel() = default;

void FunctionListPanel::init(ScintillaEditView** ppEditView)
{
    _ppEditView = ppEditView;
    setupUI();
    connectSignals();
}

void FunctionListPanel::setupUI()
{
    // Create main widget
    _widget = new QWidget(this);
    _widget->setObjectName("FunctionListPanel");

    QVBoxLayout* mainLayout = new QVBoxLayout(_widget);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // Toolbar with buttons
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(4);

    // Filter/search box
    _filterEdit = new QLineEdit(_widget);
    _filterEdit->setPlaceholderText(tr("Search functions..."));
    _filterEdit->setClearButtonEnabled(true);
    toolbarLayout->addWidget(_filterEdit, 1);

    // Refresh button
    _refreshBtn = new QPushButton(_widget);
    _refreshBtn->setText(tr("Refresh"));
    _refreshBtn->setToolTip(tr("Refresh function list"));
    toolbarLayout->addWidget(_refreshBtn);

    // Sort button
    _sortBtn = new QPushButton(_widget);
    _sortBtn->setText(tr("Sort"));
    _sortBtn->setToolTip(tr("Sort alphabetically"));
    _sortBtn->setCheckable(true);
    toolbarLayout->addWidget(_sortBtn);

    // Expand button
    _expandBtn = new QPushButton(_widget);
    _expandBtn->setText(tr("Expand"));
    _expandBtn->setToolTip(tr("Expand all"));
    toolbarLayout->addWidget(_expandBtn);

    // Collapse button
    _collapseBtn = new QPushButton(_widget);
    _collapseBtn->setText(tr("Collapse"));
    _collapseBtn->setToolTip(tr("Collapse all"));
    toolbarLayout->addWidget(_collapseBtn);

    mainLayout->addLayout(toolbarLayout);

    // Language selector
    QHBoxLayout* langLayout = new QHBoxLayout();
    QLabel* langLabel = new QLabel(tr("Language:"), _widget);
    langLayout->addWidget(langLabel);

    _langCombo = new QComboBox(_widget);
    populateLanguageCombo();
    _langCombo->setCurrentText(tr("Auto"));
    langLayout->addWidget(_langCombo, 1);

    mainLayout->addLayout(langLayout);

    // Tree view
    _treeView = new TreeView();
    _treeView->init(_widget);
    _treeView->makeLabelEditable(false);

    QTreeWidget* treeWidget = _treeView->getTreeWidget();
    if (treeWidget) {
        treeWidget->setHeaderHidden(true);
        treeWidget->setColumnCount(1);
        mainLayout->addWidget(treeWidget, 1);
    }

    _widget->setLayout(mainLayout);
}

void FunctionListPanel::connectSignals()
{
    if (_refreshBtn) {
        connect(_refreshBtn, &QPushButton::clicked, this, &FunctionListPanel::onRefreshClicked);
    }
    if (_sortBtn) {
        connect(_sortBtn, &QPushButton::toggled, this, &FunctionListPanel::onSortClicked);
    }
    if (_expandBtn) {
        connect(_expandBtn, &QPushButton::clicked, this, &FunctionListPanel::onExpandAllClicked);
    }
    if (_collapseBtn) {
        connect(_collapseBtn, &QPushButton::clicked, this, &FunctionListPanel::onCollapseAllClicked);
    }
    if (_filterEdit) {
        connect(_filterEdit, &QLineEdit::textChanged, this, &FunctionListPanel::onFilterChanged);
    }
    if (_langCombo) {
        connect(_langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this](int) { onLanguageChanged(_langCombo->currentText()); });
    }
    if (_treeView) {
        connect(_treeView, &TreeView::itemClicked, this, &FunctionListPanel::onItemClicked);
        connect(_treeView, &TreeView::itemDoubleClicked, this, &FunctionListPanel::onItemDoubleClicked);
    }
}

void FunctionListPanel::doDialog()
{
    if (_widget) {
        _widget->show();
        parseCurrentDocument();
    }
}

bool FunctionListPanel::run_dlgProc(QEvent* event)
{
    (void)event;
    return false;
}

void FunctionListPanel::parseCurrentDocument()
{
    if (!_ppEditView || !*_ppEditView) {
        return;
    }

    parseDocument();
}

void FunctionListPanel::parseDocument()
{
    if (!_ppEditView || !*_ppEditView) {
        return;
    }

    // Get document content using Scintilla API
    ScintillaEditView* pEditView = *_ppEditView;
    auto length = pEditView->execute(SCI_GETTEXTLENGTH);
    std::string buffer(length + 1, '\0');
    pEditView->execute(SCI_GETTEXT, length + 1, reinterpret_cast<LPARAM>(buffer.data()));
    QString content = QString::fromUtf8(buffer.data(), static_cast<int>(length));

    // Detect language
    QString lang = detectLanguage();
    if (_langCombo && _langCombo->currentText() != tr("Auto")) {
        lang = _langCombo->currentText();
    }

    // Get appropriate parser
    FunctionParser* parser = _parserMgr->getParser(lang);
    if (!parser) {
        // Try by file extension
#ifdef NPP_LINUX
        QString fileName = (*_ppEditView)->getCurrentBuffer()->getFileName();
#else
        QString fileName = QString::fromWCharArray((*_ppEditView)->getCurrentBuffer()->getFileName());
#endif
        QString ext = QFileInfo(fileName).suffix();
        parser = _parserMgr->getParserForExtension(ext);
    }

    _functions.clear();

    if (parser) {
        _functions = parser->parse(content);
    }

    rebuildTree();
}

void FunctionListPanel::rebuildTree()
{
    if (!_treeView) {
        return;
    }

    // Save current tree state before clearing
    saveTreeState();

    _treeView->clear();

    if (_functions.empty()) {
        return;
    }

    // Add root node with file name
    QString fileName;
    if (_ppEditView && *_ppEditView) {
#ifdef NPP_LINUX
        fileName = (*_ppEditView)->getCurrentBuffer()->getFileName();
#else
        fileName = QString::fromWCharArray((*_ppEditView)->getCurrentBuffer()->getFileName());
#endif
    } else {
        fileName = tr("Functions");
    }

    int rootId = _treeView->addItem(fileName, -1, INDEX_ROOT);

    // Sort if needed
    if (_sortAlpha) {
        sortFunctions();
    }

    // Add functions to tree
    for (const auto& func : _functions) {
        if (matchesFilter(func)) {
            addFunctionToTreeRecursive(func, rootId);
        }
    }

    // Expand root
    _treeView->expand(rootId);

    // Restore tree state
    restoreTreeState();
}

void FunctionListPanel::addFunctionToTree(const FunctionInfo& func)
{
    if (!_treeView) {
        return;
    }

    int rootId = _treeView->getRootItem();
    if (rootId < 0) {
        return;
    }

    addFunctionToTreeRecursive(func, rootId);
}

void FunctionListPanel::addFunctionToTreeRecursive(const FunctionInfo& func, int parentId)
{
    if (!_treeView) {
        return;
    }

    // Determine icon based on type
    int iconIndex = INDEX_LEAF;
    if (func.type == "class" || func.type == "struct") {
        iconIndex = INDEX_NODE;
    }

    // Create item
    int itemId = _treeView->addItem(func.displayName, parentId, iconIndex);

    // Store function info in item data
    QVariant data;
    data.setValue(func.lineNumber);
    _treeView->setItemData(itemId, data);

    // Store position for navigation
    QVariant posData;
    posData.setValue(static_cast<qint64>(func.pos));
    _treeView->setItemData(itemId, posData);
}

void FunctionListPanel::sortFunctions()
{
    std::sort(_functions.begin(), _functions.end());
}

void FunctionListPanel::sortTreeItems(int parentId)
{
    if (!_treeView) {
        return;
    }

    _treeView->sortItems(0, Qt::AscendingOrder);
}

bool FunctionListPanel::matchesFilter(const FunctionInfo& func) const
{
    if (_currentFilter.isEmpty()) {
        return true;
    }

    return func.name.contains(_currentFilter, Qt::CaseInsensitive) ||
           func.displayName.contains(_currentFilter, Qt::CaseInsensitive);
}

void FunctionListPanel::applyFilter()
{
    rebuildTree();
}

void FunctionListPanel::saveTreeState()
{
    if (!_treeView || _isRestoringState) {
        return;
    }

    int rootId = _treeView->getRootItem();
    if (rootId < 0) {
        return;
    }

    _savedTreeState = TreeState();
    saveTreeStateRecursive(rootId, _savedTreeState);
}

void FunctionListPanel::saveTreeStateRecursive(int itemId, TreeState& state)
{
    if (!_treeView) {
        return;
    }

    state.label = _treeView->getItemText(itemId);
    state.isExpanded = _treeView->isExpanded(itemId);

    // Save children
    int childId = _treeView->getChildItem(itemId);
    while (childId >= 0) {
        TreeState childState;
        saveTreeStateRecursive(childId, childState);
        state.children.push_back(childState);
        childId = _treeView->getNextSibling(childId);
    }
}

void FunctionListPanel::restoreTreeState()
{
    if (!_treeView || _savedTreeState.label.isEmpty()) {
        return;
    }

    _isRestoringState = true;

    int rootId = _treeView->getRootItem();
    if (rootId >= 0) {
        restoreTreeStateRecursive(rootId, _savedTreeState);
    }

    _isRestoringState = false;
}

void FunctionListPanel::restoreTreeStateRecursive(int itemId, const TreeState& state)
{
    if (!_treeView) {
        return;
    }

    QString itemText = _treeView->getItemText(itemId);
    if (itemText != state.label) {
        return;
    }

    if (state.isExpanded) {
        _treeView->expand(itemId);
    } else {
        _treeView->collapse(itemId);
    }

    // Restore children
    int childId = _treeView->getChildItem(itemId);
    size_t childIndex = 0;
    while (childId >= 0 && childIndex < state.children.size()) {
        restoreTreeStateRecursive(childId, state.children[childIndex]);
        childId = _treeView->getNextSibling(childId);
        childIndex++;
    }
}

QString FunctionListPanel::detectLanguage() const
{
    if (!_ppEditView || !*_ppEditView) {
        return QString();
    }

#ifdef NPP_LINUX
    QString fileName = (*_ppEditView)->getCurrentBuffer()->getFileName();
#else
    QString fileName = QString::fromWCharArray((*_ppEditView)->getCurrentBuffer()->getFileName());
#endif
    QString ext = QFileInfo(fileName).suffix().toLower();

    // Map extension to language
    if (ext == "cpp" || ext == "c" || ext == "h" || ext == "hpp" || ext == "cc" || ext == "cxx") {
        return "C/C++";
    } else if (ext == "py" || ext == "pyw") {
        return "Python";
    } else if (ext == "js" || ext == "jsx" || ext == "ts" || ext == "tsx") {
        return "JavaScript";
    } else if (ext == "java") {
        return "Java";
    } else if (ext == "cs") {
        return "C#";
    }

    return QString();
}

void FunctionListPanel::populateLanguageCombo()
{
    if (!_langCombo) {
        return;
    }

    _langCombo->clear();
    _langCombo->addItem(tr("Auto"));

    QStringList parsers = _parserMgr->getAvailableParsers();
    for (const QString& name : parsers) {
        _langCombo->addItem(name);
    }
}

void FunctionListPanel::navigateToLine(int line)
{
    if (!_ppEditView || !*_ppEditView || line < 0) {
        return;
    }

    // Convert to 0-based line number for Scintilla
    intptr_t linePos = (*_ppEditView)->execute(SCI_POSITIONFROMLINE, line - 1);
    (*_ppEditView)->execute(SCI_GOTOPOS, linePos);
    (*_ppEditView)->execute(SCI_ENSUREVISIBLE, line - 1);
    (*_ppEditView)->scrollPosToCenter(linePos);
}

void FunctionListPanel::navigateToPosition(intptr_t pos)
{
    if (!_ppEditView || !*_ppEditView || pos < 0) {
        return;
    }

    (*_ppEditView)->execute(SCI_GOTOPOS, pos);
    (*_ppEditView)->execute(SCI_ENSUREVISIBLE,
        (*_ppEditView)->execute(SCI_LINEFROMPOSITION, pos));
    (*_ppEditView)->scrollPosToCenter(pos);
}

void FunctionListPanel::clearTree()
{
    if (_treeView) {
        _treeView->clear();
    }
}

void FunctionListPanel::refresh()
{
    parseCurrentDocument();
}

void FunctionListPanel::setSortAlphabetically(bool sort)
{
    _sortAlpha = sort;
    if (_sortBtn) {
        _sortBtn->setChecked(sort);
    }
    rebuildTree();
}

void FunctionListPanel::markEntry()
{
    if (!_ppEditView || !*_ppEditView || !_treeView) {
        return;
    }

    // Get current line
    intptr_t currentLine = (*_ppEditView)->getCurrentLineNumber();

    // Find the function that contains this line
    int bestMatch = -1;
    int bestLine = -1;

    for (size_t i = 0; i < _functions.size(); ++i) {
        int funcLine = _functions[i].lineNumber;
        if (funcLine <= currentLine && funcLine > bestLine) {
            bestLine = funcLine;
            bestMatch = static_cast<int>(i);
        }
    }

    if (bestMatch >= 0) {
        // Find and select the corresponding tree item
        // This is simplified - in a full implementation we'd map functions to tree items
        updateCurrentItemMarker();
    }
}

void FunctionListPanel::updateCurrentItemMarker()
{
    // Update visual indicator for current function
    // Implementation depends on how we want to highlight
}

void FunctionListPanel::setBackgroundColor(const QColor& color)
{
    if (_treeView && _treeView->getTreeWidget()) {
        _treeView->getTreeWidget()->setStyleSheet(
            QString("QTreeWidget { background-color: %1; }").arg(color.name()));
    }
}

void FunctionListPanel::setForegroundColor(const QColor& color)
{
    if (_treeView && _treeView->getTreeWidget()) {
        // Append to existing stylesheet or create new one
        QString currentSheet = _treeView->getTreeWidget()->styleSheet();
        _treeView->getTreeWidget()->setStyleSheet(
            currentSheet + QString(" QTreeWidget { color: %1; }").arg(color.name()));
    }
}

// ============================================================================
// Slot implementations
// ============================================================================
void FunctionListPanel::onItemClicked(int itemId)
{
    (void)itemId;
    // Single click just selects, doesn't navigate
}

void FunctionListPanel::onItemDoubleClicked(int itemId)
{
    if (!_treeView) {
        return;
    }

    // Get the line number from item data
    QVariant data = _treeView->getItemData(itemId);
    int lineNumber = data.toInt();

    if (lineNumber > 0) {
        navigateToLine(lineNumber);
    }
}

void FunctionListPanel::onRefreshClicked()
{
    parseCurrentDocument();
}

void FunctionListPanel::onSortClicked()
{
    _sortAlpha = !_sortAlpha;
    setSortAlphabetically(_sortAlpha);
}

void FunctionListPanel::onExpandAllClicked()
{
    if (_treeView) {
        _treeView->expandAll();
    }
}

void FunctionListPanel::onCollapseAllClicked()
{
    if (_treeView) {
        _treeView->collapseAll();
    }
}

void FunctionListPanel::onFilterChanged(const QString& text)
{
    _currentFilter = text;
    applyFilter();
}

void FunctionListPanel::onLanguageChanged(const QString& lang)
{
    (void)lang;
    parseCurrentDocument();
}

} // namespace QtControls
