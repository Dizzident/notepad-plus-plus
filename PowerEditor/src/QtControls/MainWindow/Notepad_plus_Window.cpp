// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "Notepad_plus_Window.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>
#include <QSettings>
#include <QScreen>
#include <QActionGroup>

namespace QtControls {

namespace MainWindow {

// ============================================================================
// Constructor / Destructor
// ============================================================================

MainWindow::MainWindow()
    : QMainWindow(nullptr)
    , StaticDialog()
{
    // Set window properties
    setWindowTitle("Notepad++");
    setAcceptDrops(true);

    // Create the main widget for StaticDialog interface
    _widget = this;
}

MainWindow::~MainWindow()
{
    destroy();
}

// ============================================================================
// Initialization
// ============================================================================

bool MainWindow::init(Notepad_plus* pNotepad_plus)
{
    if (!pNotepad_plus) {
        return false;
    }

    _pNotepad_plus = pNotepad_plus;

    // Setup UI components
    setupUI();
    connectSignals();
    createDockWindows();
    loadSettings();

    // Initialize menu state
    updateMenuState();

    // Initialize toolbar state
    updateToolBarState();

    // Initialize status bar
    updateStatusBar();

    return true;
}

void MainWindow::setupUI()
{
    // Create central widget with splitter for editors
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create editor splitter
    _editorSplitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(_editorSplitter);

    // Initialize menu bar
    initMenuBar();

    // Initialize toolbar
    initToolBar();

    // Initialize status bar
    initStatusBar();

    // Create docking manager
    _dockingManager = new DockingManager::Manager();
    _dockingManager->init(this);

    // Create update timer
    _updateTimer = new QTimer(this);
    connect(_updateTimer, &QTimer::timeout, this, [this]() {
        updateStatusBar();
    });
    _updateTimer->start(500); // Update every 500ms
}

void MainWindow::connectSignals()
{
    // Tab widget signals will be connected when tab widget is created
}

void MainWindow::createDockWindows()
{
    // Create Function List panel
    _functionListPanel = new FunctionListPanel(this);
    _functionListPanel->init(nullptr); // Will be set properly when core is ready
    _dockingManager->addPanel("functionList", _functionListPanel->getWidget(),
                               DockingManager::DockArea::Left, tr("Function List"));

    // Create Project Panel
    _projectPanel = new ProjectPanel(this);
    _projectPanel->init(nullptr);
    _dockingManager->addPanel("projectPanel", _projectPanel->getWidget(),
                               DockingManager::DockArea::Left, tr("Project"));

    // Create Document Map panel
    _documentMap = new DocumentMap(this);
    _documentMap->init(nullptr);
    _dockingManager->addPanel("documentMap", _documentMap->getWidget(),
                               DockingManager::DockArea::Right, tr("Document Map"));

    // Create Clipboard History panel
    _clipboardHistoryPanel = new ClipboardHistoryPanel(this);
    _clipboardHistoryPanel->init(nullptr);
    _dockingManager->addPanel("clipboardHistory", _clipboardHistoryPanel->getWidget(),
                               DockingManager::DockArea::Right, tr("Clipboard History"));

    // Create File Browser panel
    _fileBrowser = new FileBrowser(this);
    _fileBrowser->init(nullptr);
    _dockingManager->addPanel("fileBrowser", _fileBrowser->getWidget(),
                               DockingManager::DockArea::Left, tr("Folder as Workspace"));

    // Initially hide all panels
    _dockingManager->hidePanel("functionList");
    _dockingManager->hidePanel("projectPanel");
    _dockingManager->hidePanel("documentMap");
    _dockingManager->hidePanel("clipboardHistory");
    _dockingManager->hidePanel("fileBrowser");
}

// ============================================================================
// StaticDialog Interface
// ============================================================================

void MainWindow::destroy()
{
    // Save settings before destroying
    saveSettings();

    // Clean up panels
    if (_functionListPanel) {
        delete _functionListPanel;
        _functionListPanel = nullptr;
    }

    if (_projectPanel) {
        delete _projectPanel;
        _projectPanel = nullptr;
    }

    if (_documentMap) {
        delete _documentMap;
        _documentMap = nullptr;
    }

    if (_clipboardHistoryPanel) {
        delete _clipboardHistoryPanel;
        _clipboardHistoryPanel = nullptr;
    }

    if (_fileBrowser) {
        delete _fileBrowser;
        _fileBrowser = nullptr;
    }

    if (_dockingManager) {
        _dockingManager->destroy();
        delete _dockingManager;
        _dockingManager = nullptr;
    }

    if (_mainToolBar) {
        _mainToolBar->destroy();
        delete _mainToolBar;
        _mainToolBar = nullptr;
    }

    if (_statusBar) {
        _statusBar->destroy();
        delete _statusBar;
        _statusBar = nullptr;
    }

    if (_tabBar) {
        _tabBar->destroy();
        delete _tabBar;
        _tabBar = nullptr;
    }

    // Don't delete _widget (which is 'this') - Qt handles that
    _widget = nullptr;
}

void MainWindow::display(bool toShow)
{
    if (toShow) {
        show();
        raise();
        activateWindow();
    } else {
        hide();
    }
}

void MainWindow::reSizeTo(QRect& rc)
{
    setGeometry(rc);
}

bool MainWindow::run_dlgProc(QEvent* event)
{
    // Handle dialog-specific events
    (void)event;
    return false;
}

// ============================================================================
// Menu Operations
// ============================================================================

void MainWindow::initMenuBar()
{
    _menuBar = new QMenuBar(this);
    setMenuBar(_menuBar);

    createFileMenu();
    createEditMenu();
    createSearchMenu();
    createViewMenu();
    createEncodingMenu();
    createLanguageMenu();
    createSettingsMenu();
    createMacroMenu();
    createRunMenu();
    createWindowMenu();
    createHelpMenu();
}

void MainWindow::createFileMenu()
{
    _fileMenu = _menuBar->addMenu(tr("&File"));

    // New
    auto* newAction = _fileMenu->addAction(tr("&New"), this, &MainWindow::onFileNew);
    newAction->setShortcut(QKeySequence::New);

    // Open
    auto* openAction = _fileMenu->addAction(tr("&Open..."), this, &MainWindow::onFileOpen);
    openAction->setShortcut(QKeySequence::Open);

    _fileMenu->addSeparator();

    // Save
    auto* saveAction = _fileMenu->addAction(tr("&Save"), this, &MainWindow::onFileSave);
    saveAction->setShortcut(QKeySequence::Save);

    // Save As
    auto* saveAsAction = _fileMenu->addAction(tr("Save &As..."), this, &MainWindow::onFileSaveAs);
    saveAsAction->setShortcut(QKeySequence::SaveAs);

    // Save All
    auto* saveAllAction = _fileMenu->addAction(tr("Save A&ll"), this, &MainWindow::onFileSaveAll);

    _fileMenu->addSeparator();

    // Close
    auto* closeAction = _fileMenu->addAction(tr("&Close"), this, &MainWindow::onFileClose);
    closeAction->setShortcut(QKeySequence::Close);

    // Close All
    _fileMenu->addAction(tr("Clos&e All"), this, &MainWindow::onFileCloseAll);

    _fileMenu->addSeparator();

    // Recent files submenu
    auto* recentMenu = _fileMenu->addMenu(tr("Recent Files"));
    recentMenu->addAction(tr("Restore Recent Closed File"));
    recentMenu->addSeparator();
    recentMenu->addAction(tr("Empty"));

    _fileMenu->addSeparator();

    // Exit
    auto* exitAction = _fileMenu->addAction(tr("E&xit"), this, &MainWindow::onFileExit);
    exitAction->setShortcut(QKeySequence::Quit);
}

void MainWindow::createEditMenu()
{
    _editMenu = _menuBar->addMenu(tr("&Edit"));

    // Undo
    auto* undoAction = _editMenu->addAction(tr("&Undo"), this, &MainWindow::onEditUndo);
    undoAction->setShortcut(QKeySequence::Undo);

    // Redo
    auto* redoAction = _editMenu->addAction(tr("&Redo"), this, &MainWindow::onEditRedo);
    redoAction->setShortcut(QKeySequence::Redo);

    _editMenu->addSeparator();

    // Cut
    auto* cutAction = _editMenu->addAction(tr("Cu&t"), this, &MainWindow::onEditCut);
    cutAction->setShortcut(QKeySequence::Cut);

    // Copy
    auto* copyAction = _editMenu->addAction(tr("&Copy"), this, &MainWindow::onEditCopy);
    copyAction->setShortcut(QKeySequence::Copy);

    // Paste
    auto* pasteAction = _editMenu->addAction(tr("&Paste"), this, &MainWindow::onEditPaste);
    pasteAction->setShortcut(QKeySequence::Paste);

    // Delete
    auto* deleteAction = _editMenu->addAction(tr("&Delete"), this, &MainWindow::onEditDelete);
    deleteAction->setShortcut(QKeySequence::Delete);

    _editMenu->addSeparator();

    // Select All
    auto* selectAllAction = _editMenu->addAction(tr("Select &All"), this, &MainWindow::onEditSelectAll);
    selectAllAction->setShortcut(QKeySequence::SelectAll);

    _editMenu->addSeparator();

    // Insert submenu
    auto* insertMenu = _editMenu->addMenu(tr("Insert"));
    insertMenu->addAction(tr("Current Date and Time"));
    insertMenu->addAction(tr("Full File Path"));
    insertMenu->addAction(tr("File Name"));
    insertMenu->addAction(tr("Current Directory"));

    // Copy to Clipboard submenu
    auto* copyToMenu = _editMenu->addMenu(tr("Copy to Clipboard"));
    copyToMenu->addAction(tr("Current Full File Path"));
    copyToMenu->addAction(tr("Current File Name"));
    copyToMenu->addAction(tr("Current Directory Path"));

    // Indent submenu
    auto* indentMenu = _editMenu->addMenu(tr("Indent"));
    indentMenu->addAction(tr("Increase Line Indent"));
    indentMenu->addAction(tr("Decrease Line Indent"));

    // Convert submenu
    auto* convertMenu = _editMenu->addMenu(tr("Convert Case to"));
    convertMenu->addAction(tr("Uppercase"));
    convertMenu->addAction(tr("Lowercase"));
    convertMenu->addAction(tr("Title Case"));
}

void MainWindow::createSearchMenu()
{
    _searchMenu = _menuBar->addMenu(tr("&Search"));

    // Find
    auto* findAction = _searchMenu->addAction(tr("&Find..."), this, &MainWindow::onSearchFind);
    findAction->setShortcut(QKeySequence::Find);

    // Find Next
    auto* findNextAction = _searchMenu->addAction(tr("Find &Next"), this, &MainWindow::onSearchFindNext);
    findNextAction->setShortcut(QKeySequence::FindNext);

    // Find Previous
    auto* findPrevAction = _searchMenu->addAction(tr("Find &Previous"), this, &MainWindow::onSearchFindPrev);
    findPrevAction->setShortcut(QKeySequence::FindPrevious);

    _searchMenu->addSeparator();

    // Replace
    auto* replaceAction = _searchMenu->addAction(tr("&Replace..."), this, &MainWindow::onSearchReplace);
    replaceAction->setShortcut(QKeySequence::Replace);

    _searchMenu->addSeparator();

    // Go To
    auto* gotoAction = _searchMenu->addAction(tr("&Go To..."), this, &MainWindow::onSearchGoToLine);
    gotoAction->setShortcut(QKeySequence("Ctrl+G"));

    _searchMenu->addSeparator();

    // Bookmarks submenu
    auto* bookmarkMenu = _searchMenu->addMenu(tr("Bookmark"));
    bookmarkMenu->addAction(tr("Toggle Bookmark"));
    bookmarkMenu->addAction(tr("Next Bookmark"));
    bookmarkMenu->addAction(tr("Previous Bookmark"));
    bookmarkMenu->addAction(tr("Clear All Bookmarks"));

    // Find in Files
    _searchMenu->addSeparator();
    _searchMenu->addAction(tr("Find in Files..."));
}

void MainWindow::createViewMenu()
{
    _viewMenu = _menuBar->addMenu(tr("&View"));

    // View Mode submenu
    auto* viewModeMenu = _viewMenu->addMenu(tr("View Mode"));
    auto* fullScreenAction = viewModeMenu->addAction(tr("&Full Screen"), this, &MainWindow::onViewFullScreen);
    fullScreenAction->setShortcut(QKeySequence::FullScreen);

    viewModeMenu->addAction(tr("&Post-it"), this, &MainWindow::onViewPostIt);
    viewModeMenu->addAction(tr("Distraction &Free Mode"));

    _viewMenu->addSeparator();

    // Always on Top
    _viewMenu->addAction(tr("Always on &Top"), this, &MainWindow::onViewAlwaysOnTop);

    _viewMenu->addSeparator();

    // Word Wrap
    auto* wordWrapAction = _viewMenu->addAction(tr("Word &Wrap"), this, &MainWindow::onViewWordWrap);
    wordWrapAction->setCheckable(true);

    // Show Symbols submenu
    auto* symbolsMenu = _viewMenu->addMenu(tr("Show Symbol"));
    auto* showWhiteSpaceAction = symbolsMenu->addAction(tr("Show White Space and TAB"), this, &MainWindow::onViewShowWhiteSpace);
    showWhiteSpaceAction->setCheckable(true);

    auto* showEOLAction = symbolsMenu->addAction(tr("Show End of Line"), this, &MainWindow::onViewShowEOL);
    showEOLAction->setCheckable(true);

    auto* showIndentAction = symbolsMenu->addAction(tr("Show Indent Guide"), this, &MainWindow::onViewShowIndentGuide);
    showIndentAction->setCheckable(true);

    _viewMenu->addSeparator();

    // Zoom submenu
    auto* zoomMenu = _viewMenu->addMenu(tr("Zoom"));
    zoomMenu->addAction(tr("Zoom &In"));
    zoomMenu->addAction(tr("Zoom &Out"));
    zoomMenu->addAction(tr("Restore Default Zoom"));

    _viewMenu->addSeparator();

    // Panels submenu
    auto* panelsMenu = _viewMenu->addMenu(tr("Panel"));
    panelsMenu->addAction(tr("Function &List"), this, &MainWindow::onViewFunctionList);
    panelsMenu->addAction(tr("&Project Panel"), this, &MainWindow::onViewProjectPanel);
    panelsMenu->addAction(tr("&Document Map"), this, &MainWindow::onViewDocumentMap);
    panelsMenu->addAction(tr("&Clipboard History"), this, &MainWindow::onViewClipboardHistory);
    panelsMenu->addAction(tr("Folder as &Workspace"), this, &MainWindow::onViewFileBrowser);

    // Tab Bar
    _viewMenu->addSeparator();
    auto* tabBarAction = _viewMenu->addAction(tr("Tab Bar"));
    tabBarAction->setCheckable(true);
    tabBarAction->setChecked(true);

    // Status Bar
    auto* statusBarAction = _viewMenu->addAction(tr("Status Bar"));
    statusBarAction->setCheckable(true);
    statusBarAction->setChecked(true);
}

void MainWindow::createEncodingMenu()
{
    _encodingMenu = _menuBar->addMenu(tr("&Encoding"));

    // ANSI
    _encodingMenu->addAction(tr("Encode in &ANSI"), this, &MainWindow::onEncodingANSI);

    _encodingMenu->addSeparator();

    // UTF-8
    _encodingMenu->addAction(tr("Encode in &UTF-8"), this, &MainWindow::onEncodingUTF8);

    // UTF-8 BOM
    _encodingMenu->addAction(tr("Encode in UTF-8-&BOM"), this, &MainWindow::onEncodingUTF8BOM);

    _encodingMenu->addSeparator();

    // UTF-16
    _encodingMenu->addAction(tr("Encode in &UTF-16 BE"), this, &MainWindow::onEncodingUTF16BE);
    _encodingMenu->addAction(tr("Encode in UTF-16 &LE"), this, &MainWindow::onEncodingUTF16LE);
}

void MainWindow::createLanguageMenu()
{
    _languageMenu = _menuBar->addMenu(tr("&Language"));

    // Common languages
    QStringList languages = {
        "Normal Text",
        "C",
        "C++",
        "C#",
        "Java",
        "Python",
        "JavaScript",
        "HTML",
        "CSS",
        "XML",
        "JSON",
        "SQL",
        "PHP",
        "Ruby",
        "Go",
        "Rust",
        "TypeScript",
        "Shell",
        "PowerShell",
        "Batch",
        "Makefile",
        "CMake",
        "Markdown",
        "YAML",
        "Lua",
        "Perl",
        "R",
        "Swift",
        "Kotlin",
        "Scala",
        "Groovy",
        "VB",
        "VBScript",
        "ActionScript",
        "CoffeeScript",
        "Dart",
        "Elixir",
        "Erlang",
        "Fortran",
        "Haskell",
        "Julia",
        "Lisp",
        "MATLAB",
        "Objective-C",
        "Pascal",
        "Raku",
        "Tcl",
        "Verilog",
        "VHDL"
    };

    auto* langGroup = new QActionGroup(this);
    langGroup->setExclusive(true);

    for (const QString& lang : languages) {
        auto* action = _languageMenu->addAction(lang, this, [this]() {
            onLanguageSelected(qobject_cast<QAction*>(sender()));
        });
        action->setCheckable(true);
        langGroup->addAction(action);
    }

    _languageMenu->addSeparator();
    _languageMenu->addAction(tr("Define your language..."));
    _languageMenu->addAction(tr("User-Defined"));
}

void MainWindow::createSettingsMenu()
{
    _settingsMenu = _menuBar->addMenu(tr("&Settings"));

    // Preferences
    _settingsMenu->addAction(tr("&Preferences..."), this, &MainWindow::onSettingsPreferences);

    _settingsMenu->addSeparator();

    // Style Configurator
    _settingsMenu->addAction(tr("Style &Configurator..."), this, &MainWindow::onSettingsStyleConfigurator);

    // Shortcut Mapper
    _settingsMenu->addAction(tr("Shortcut &Mapper..."), this, &MainWindow::onSettingsShortcutMapper);

    _settingsMenu->addSeparator();

    // Import submenu
    auto* importMenu = _settingsMenu->addMenu(tr("Import"));
    importMenu->addAction(tr("Import plugin(s)..."));
    importMenu->addAction(tr("Import style theme(s)..."));

    // Edit Popup ContextMenu
    _settingsMenu->addAction(tr("Edit Popup ContextMenu"));
}

void MainWindow::createMacroMenu()
{
    _macroMenu = _menuBar->addMenu(tr("&Macro"));

    // Start Recording
    _macroMenu->addAction(tr("Start &Recording"), this, &MainWindow::onMacroStartRecording);

    // Stop Recording
    _macroMenu->addAction(tr("S&top Recording"), this, &MainWindow::onMacroStopRecording);

    _macroMenu->addSeparator();

    // Playback
    auto* playbackAction = _macroMenu->addAction(tr("&Playback"), this, &MainWindow::onMacroPlayback);
    playbackAction->setShortcut(QKeySequence("Shift+Ctrl+P"));

    // Run a Macro Multiple Times
    _macroMenu->addAction(tr("Run a Macro Multiple &Times..."), this, &MainWindow::onMacroRunMultiple);

    _macroMenu->addSeparator();

    // Save Current Recorded Macro
    _macroMenu->addAction(tr("Save Current Recorded Macro..."));
}

void MainWindow::createRunMenu()
{
    _runMenu = _menuBar->addMenu(tr("&Run"));

    // Run...
    auto* runAction = _runMenu->addAction(tr("&Run..."), this, &MainWindow::onRunRun);
    runAction->setShortcut(QKeySequence("F5"));

    _runMenu->addSeparator();

    // Launch in browser actions
    _runMenu->addAction(tr("Launch in &Chrome"), this, &MainWindow::onRunLaunchInBrowser);
    _runMenu->addAction(tr("Launch in &Firefox"));
    _runMenu->addAction(tr("Launch in &IE"));
    _runMenu->addAction(tr("Launch in &Safari"));

    _runMenu->addSeparator();

    // Get PHP help
    _runMenu->addAction(tr("Get PHP help"));

    // Wikipedia Search
    _runMenu->addAction(tr("Wikipedia Search"));

    _runMenu->addSeparator();

    // Modify Shortcut / Delete Command
    _runMenu->addAction(tr("Modify Shortcut / Delete Command..."));
}

void MainWindow::createWindowMenu()
{
    _windowMenu = _menuBar->addMenu(tr("&Window"));

    // New Instance
    _windowMenu->addAction(tr("New &Instance"), this, &MainWindow::onWindowNewInstance);

    _windowMenu->addSeparator();

    // Split/Unsplit
    _windowMenu->addAction(tr("&Split"), this, &MainWindow::onWindowSplit);

    // Clone to Other View
    _windowMenu->addAction(tr("&Clone to Other View"), this, &MainWindow::onWindowCloneToOtherView);

    _windowMenu->addSeparator();

    // Window list
    _windowMenu->addAction(tr("Window List"));
}

void MainWindow::createHelpMenu()
{
    _helpMenu = _menuBar->addMenu(tr("&Help"));

    // About Notepad++
    _helpMenu->addAction(tr("&About Notepad++"), this, &MainWindow::onHelpAbout);

    _helpMenu->addSeparator();

    // Command Line Arguments
    _helpMenu->addAction(tr("Command Line Arguments..."));

    // Debug Info
    _helpMenu->addAction(tr("Debug Info..."));
}

void MainWindow::updateMenuState()
{
    // Update menu items based on current document state
    // This will be called when document state changes
}

// ============================================================================
// Toolbar Operations
// ============================================================================

void MainWindow::initToolBar()
{
    _mainToolBar = new ToolBar();

    // Define toolbar buttons
    static const ToolBarButtonUnit toolBarButtons[] = {
        { 1, 1, 0, tr("New").toUtf8().constData() },
        { 2, 2, 0, tr("Open").toUtf8().constData() },
        { 3, 3, 0, tr("Save").toUtf8().constData() },
        { 4, 4, 0, tr("Save All").toUtf8().constData() },
        { 0, 0, 0, "" }, // Separator
        { 5, 5, 0, tr("Cut").toUtf8().constData() },
        { 6, 6, 0, tr("Copy").toUtf8().constData() },
        { 7, 7, 0, tr("Paste").toUtf8().constData() },
        { 0, 0, 0, "" }, // Separator
        { 8, 8, 0, tr("Undo").toUtf8().constData() },
        { 9, 9, 0, tr("Redo").toUtf8().constData() },
        { 0, 0, 0, "" }, // Separator
        { 10, 10, 0, tr("Find").toUtf8().constData() },
        { 11, 11, 0, tr("Replace").toUtf8().constData() },
        { 0, 0, 0, "" }, // Separator
        { 12, 12, 0, tr("Zoom In").toUtf8().constData() },
        { 13, 13, 0, tr("Zoom Out").toUtf8().constData() },
        { 0, 0, 0, "" }, // Separator
        { 14, 14, 0, tr("Start Recording").toUtf8().constData() },
        { 15, 15, 0, tr("Stop Recording").toUtf8().constData() },
        { 16, 16, 0, tr("Playback").toUtf8().constData() },
    };

    _mainToolBar->init(this, TB_STANDARD, toolBarButtons,
                       sizeof(toolBarButtons) / sizeof(ToolBarButtonUnit));

    // Add toolbar to main window
    addToolBar(_mainToolBar->getToolBar());
}

void MainWindow::updateToolBarState()
{
    // Update toolbar button states based on current document state
}

// ============================================================================
// Status Bar Operations
// ============================================================================

void MainWindow::initStatusBar()
{
    _statusBar = new StatusBar();

    // Define status bar parts
    int parts[] = { 200, 150, 150, 100, 100, 100, -1 }; // -1 for last part (stretch)
    _statusBar->init(this, 7);
    _statusBar->setParts(7, parts);

    // Set initial text
    _statusBar->setText(tr("Ready"), 0);
    _statusBar->setText(tr("Windows (CRLF)"), 1);
    _statusBar->setText(tr("UTF-8"), 2);
    _statusBar->setText(tr("Normal text file"), 3);
    _statusBar->setText(tr("Ln 1, Col 1"), 4);
    _statusBar->setText(tr("Sel 0 | 0"), 5);
    _statusBar->setText(tr("100%"), 6);

    setStatusBar(_statusBar->getStatusBar());
}

void MainWindow::updateStatusBar()
{
    if (!_statusBar) {
        return;
    }

    // Update status bar information
    // This would typically get information from the current ScintillaEditView
    // For now, we'll leave placeholders

    // Line and column
    _statusBar->setText(tr("Ln 1, Col 1"), 4);

    // Selection info
    _statusBar->setText(tr("Sel 0 | 0"), 5);

    // Zoom level
    _statusBar->setText(tr("100%"), 6);
}

// ============================================================================
// Panel Management
// ============================================================================

void MainWindow::showPanel(const QString& panelName, bool show)
{
    if (!_dockingManager) {
        return;
    }

    if (show) {
        _dockingManager->showPanel(panelName);
    } else {
        _dockingManager->hidePanel(panelName);
    }
}

bool MainWindow::isPanelVisible(const QString& panelName)
{
    if (!_dockingManager) {
        return false;
    }

    return _dockingManager->isPanelVisible(panelName);
}

// ============================================================================
// Document Management
// ============================================================================

void MainWindow::addTab(const QString& title, const QString& filePath)
{
    (void)filePath;

    if (_tabBar) {
        _tabBar->insertAtEnd(title);
    }
}

void MainWindow::closeTab(int index)
{
    if (_tabBar) {
        _tabBar->deleteItemAt(static_cast<size_t>(index));
    }
}

void MainWindow::switchTab(int index)
{
    if (_tabWidget) {
        _tabWidget->setCurrentIndex(index);
    }

    if (_tabBar) {
        _tabBar->activateAt(index);
    }
}

// ============================================================================
// Settings
// ============================================================================

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");

    // Restore geometry
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    } else {
        // Default size
        resize(1200, 800);
        move(QGuiApplication::primaryScreen()->geometry().center() - rect().center());
    }

    // Restore window state
    QByteArray state = settings.value("windowState").toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    }

    // Restore docking layout
    if (_dockingManager) {
        QByteArray dockState = settings.value("dockLayout").toByteArray();
        if (!dockState.isEmpty()) {
            _dockingManager->restoreLayout(dockState);
        }
    }

    settings.endGroup();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");

    // Save geometry
    settings.setValue("geometry", saveGeometry());

    // Save window state
    settings.setValue("windowState", saveState());

    // Save docking layout
    if (_dockingManager) {
        settings.setValue("dockLayout", _dockingManager->saveLayout());
    }

    settings.endGroup();
}

// ============================================================================
// Window State
// ============================================================================

void MainWindow::saveWindowState()
{
    _normalWindowState = saveState();
    _normalGeometry = saveGeometry();
}

void MainWindow::restoreWindowState()
{
    if (!_normalWindowState.isEmpty()) {
        restoreState(_normalWindowState);
    }
    if (!_normalGeometry.isEmpty()) {
        restoreGeometry(_normalGeometry);
    }
}

void MainWindow::toggleFullScreen()
{
    if (_isFullScreen) {
        setFullScreen(false);
    } else {
        setFullScreen(true);
    }
}

void MainWindow::setFullScreen(bool fullScreen)
{
    if (fullScreen == _isFullScreen) {
        return;
    }

    if (fullScreen) {
        saveWindowState();
        menuBar()->hide();
        statusBar()->hide();
        showFullScreen();
        _isFullScreen = true;
    } else {
        showNormal();
        restoreWindowState();
        menuBar()->show();
        statusBar()->show();
        _isFullScreen = false;
    }
}

void MainWindow::togglePostItMode()
{
    if (_isPostItMode) {
        // Restore normal mode
        setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);
        showNormal();
        restoreWindowState();
        _isPostItMode = false;
    } else {
        // Enter post-it mode
        saveWindowState();
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        show();
        _isPostItMode = true;
    }
}

void MainWindow::toggleDistractionFreeMode()
{
    // Toggle distraction free mode
    // This hides all UI elements except the editor
    _isDistractionFree = !_isDistractionFree;

    if (_isDistractionFree) {
        menuBar()->hide();
        statusBar()->hide();
        if (_mainToolBar) {
            _mainToolBar->display(false);
        }
        // Hide all panels
        if (_dockingManager) {
            _dockingManager->hideAllPanels();
        }
    } else {
        menuBar()->show();
        statusBar()->show();
        if (_mainToolBar) {
            _mainToolBar->display(true);
        }
    }
}

void MainWindow::setAlwaysOnTop(bool alwaysOnTop)
{
    Qt::WindowFlags flags = windowFlags();
    if (alwaysOnTop) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);
    show();
}

bool MainWindow::isAlwaysOnTop() const
{
    return windowFlags() & Qt::WindowStaysOnTopHint;
}

// ============================================================================
// Tray Icon
// ============================================================================

void MainWindow::minimizeToTray()
{
    if (!_trayIcon) {
        _trayIcon = new QSystemTrayIcon(this);
        _trayIcon->setIcon(QIcon::fromTheme("notepad++", QIcon(":/icons/notepad++.png")));
        _trayIcon->setToolTip("Notepad++");
        connect(_trayIcon, &QSystemTrayIcon::activated,
                this, &MainWindow::onTrayIconActivated);
    }

    _trayIcon->show();
    hide();
}

void MainWindow::restoreFromTray()
{
    show();
    raise();
    activateWindow();

    if (_trayIcon) {
        _trayIcon->hide();
    }
}

// ============================================================================
// Event Handlers
// ============================================================================

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveSettings();

    // Check for unsaved documents
    // TODO: Implement unsaved documents check with Notepad_plus core

    event->accept();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    // Save geometry on resize
    if (!_isFullScreen && !_isPostItMode) {
        // Debounce the save
        QTimer::singleShot(500, this, &MainWindow::saveSettings);
    }
}

void MainWindow::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);

    // Save geometry on move
    if (!_isFullScreen && !_isPostItMode) {
        // Debounce the save
        QTimer::singleShot(500, this, &MainWindow::saveSettings);
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);

    if (event->type() == QEvent::WindowStateChange) {
        if (windowState() & Qt::WindowMinimized) {
            // Window was minimized
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl& url : urls) {
            QString filePath = url.toLocalFile();
            if (!filePath.isEmpty()) {
                // TODO: Open file via Notepad_plus core
                (void)filePath;
            }
        }
    }
}

// ============================================================================
// Slot Implementations - File Menu
// ============================================================================

void MainWindow::onFileNew()
{
    // TODO: Call Notepad_plus::fileNew()
}

void MainWindow::onFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), QString(),
        tr("All Files (*);;Text Files (*.txt)"));

    if (!fileName.isEmpty()) {
        // TODO: Call Notepad_plus::doOpen()
        (void)fileName;
    }
}

void MainWindow::onFileSave()
{
    // TODO: Call Notepad_plus::fileSave()
}

void MainWindow::onFileSaveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save As"), QString(),
        tr("All Files (*);;Text Files (*.txt)"));

    if (!fileName.isEmpty()) {
        // TODO: Call Notepad_plus::fileSaveAs()
        (void)fileName;
    }
}

void MainWindow::onFileSaveAll()
{
    // TODO: Call Notepad_plus::fileSaveAll()
}

void MainWindow::onFileClose()
{
    // TODO: Call Notepad_plus::fileClose()
}

void MainWindow::onFileCloseAll()
{
    // TODO: Call Notepad_plus::fileCloseAll()
}

void MainWindow::onFileExit()
{
    close();
}

// ============================================================================
// Slot Implementations - Edit Menu
// ============================================================================

void MainWindow::onEditUndo()
{
    // TODO: Call ScintillaEditView::execute(SCI_UNDO)
}

void MainWindow::onEditRedo()
{
    // TODO: Call ScintillaEditView::execute(SCI_REDO)
}

void MainWindow::onEditCut()
{
    // TODO: Call ScintillaEditView::execute(SCI_CUT)
}

void MainWindow::onEditCopy()
{
    // TODO: Call ScintillaEditView::execute(SCI_COPY)
}

void MainWindow::onEditPaste()
{
    // TODO: Call ScintillaEditView::execute(SCI_PASTE)
}

void MainWindow::onEditDelete()
{
    // TODO: Call ScintillaEditView::execute(SCI_CLEAR)
}

void MainWindow::onEditSelectAll()
{
    // TODO: Call ScintillaEditView::execute(SCI_SELECTALL)
}

// ============================================================================
// Slot Implementations - Search Menu
// ============================================================================

void MainWindow::onSearchFind()
{
    // TODO: Show FindReplaceDlg
}

void MainWindow::onSearchReplace()
{
    // TODO: Show FindReplaceDlg with replace tab active
}

void MainWindow::onSearchFindNext()
{
    // TODO: Call Notepad_plus::findNext()
}

void MainWindow::onSearchFindPrev()
{
    // TODO: Call Notepad_plus::findPrev()
}

void MainWindow::onSearchGoToLine()
{
    // TODO: Show GoToLineDlg
}

// ============================================================================
// Slot Implementations - View Menu
// ============================================================================

void MainWindow::onViewFullScreen()
{
    toggleFullScreen();
}

void MainWindow::onViewPostIt()
{
    togglePostItMode();
}

void MainWindow::onViewAlwaysOnTop()
{
    setAlwaysOnTop(!isAlwaysOnTop());
}

void MainWindow::onViewWordWrap()
{
    // TODO: Toggle word wrap in current view
}

void MainWindow::onViewShowWhiteSpace()
{
    // TODO: Toggle whitespace visibility
}

void MainWindow::onViewShowEOL()
{
    // TODO: Toggle EOL visibility
}

void MainWindow::onViewShowIndentGuide()
{
    // TODO: Toggle indent guide visibility
}

void MainWindow::onViewFunctionList()
{
    if (isPanelVisible("functionList")) {
        showPanel("functionList", false);
    } else {
        showPanel("functionList", true);
    }
}

void MainWindow::onViewProjectPanel()
{
    if (isPanelVisible("projectPanel")) {
        showPanel("projectPanel", false);
    } else {
        showPanel("projectPanel", true);
    }
}

void MainWindow::onViewDocumentMap()
{
    if (isPanelVisible("documentMap")) {
        showPanel("documentMap", false);
    } else {
        showPanel("documentMap", true);
    }
}

void MainWindow::onViewClipboardHistory()
{
    if (isPanelVisible("clipboardHistory")) {
        showPanel("clipboardHistory", false);
    } else {
        showPanel("clipboardHistory", true);
    }
}

void MainWindow::onViewFileBrowser()
{
    if (isPanelVisible("fileBrowser")) {
        showPanel("fileBrowser", false);
    } else {
        showPanel("fileBrowser", true);
    }
}

// ============================================================================
// Slot Implementations - Encoding Menu
// ============================================================================

void MainWindow::onEncodingANSI()
{
    // TODO: Set encoding to ANSI
}

void MainWindow::onEncodingUTF8()
{
    // TODO: Set encoding to UTF-8
}

void MainWindow::onEncodingUTF8BOM()
{
    // TODO: Set encoding to UTF-8 BOM
}

void MainWindow::onEncodingUTF16BE()
{
    // TODO: Set encoding to UTF-16 BE
}

void MainWindow::onEncodingUTF16LE()
{
    // TODO: Set encoding to UTF-16 LE
}

// ============================================================================
// Slot Implementations - Language Menu
// ============================================================================

void MainWindow::onLanguageSelected(QAction* action)
{
    if (!action) {
        return;
    }

    QString langName = action->text();
    // TODO: Set language via Notepad_plus::setLanguage()
    (void)langName;
}

// ============================================================================
// Slot Implementations - Settings Menu
// ============================================================================

void MainWindow::onSettingsPreferences()
{
    // TODO: Show PreferenceDlg
}

void MainWindow::onSettingsStyleConfigurator()
{
    // TODO: Show WordStyleDlg
}

void MainWindow::onSettingsShortcutMapper()
{
    // TODO: Show ShortcutMapper
}

// ============================================================================
// Slot Implementations - Macro Menu
// ============================================================================

void MainWindow::onMacroStartRecording()
{
    // TODO: Start macro recording
}

void MainWindow::onMacroStopRecording()
{
    // TODO: Stop macro recording
}

void MainWindow::onMacroPlayback()
{
    // TODO: Playback recorded macro
}

void MainWindow::onMacroRunMultiple()
{
    // TODO: Show RunMacroDlg
}

// ============================================================================
// Slot Implementations - Run Menu
// ============================================================================

void MainWindow::onRunRun()
{
    // TODO: Show RunDlg
}

void MainWindow::onRunLaunchInBrowser()
{
    // TODO: Launch current file in browser
}

// ============================================================================
// Slot Implementations - Window Menu
// ============================================================================

void MainWindow::onWindowNewInstance()
{
    // TODO: Launch new Notepad++ instance
}

void MainWindow::onWindowSplit()
{
    // TODO: Split/Unsplit views
}

void MainWindow::onWindowCloneToOtherView()
{
    // TODO: Clone current document to other view
}

// ============================================================================
// Slot Implementations - Help Menu
// ============================================================================

void MainWindow::onHelpAbout()
{
    // TODO: Show AboutDlg
    QMessageBox::about(this, tr("About Notepad++"),
        tr("Notepad++ v8.x\n\n"
           "A free source code editor\n"
           "Based on the powerful editing component Scintilla\n\n"
           "Copyright (C)2024 Notepad++ contributors"));
}

// ============================================================================
// Slot Implementations - Tab Bar
// ============================================================================

void MainWindow::onTabChanged(int index)
{
    switchTab(index);
}

void MainWindow::onTabCloseRequested(int index)
{
    closeTab(index);
}

// ============================================================================
// Slot Implementations - Panel
// ============================================================================

void MainWindow::onPanelVisibilityChanged(bool visible)
{
    // Update menu check states based on panel visibility
    (void)visible;
}

// ============================================================================
// Slot Implementations - Tray Icon
// ============================================================================

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        restoreFromTray();
    }
}

// ============================================================================
// Getters
// ============================================================================

ScintillaEditView* MainWindow::getMainEditView() const
{
    // TODO: Return main edit view from Notepad_plus
    return nullptr;
}

ScintillaEditView* MainWindow::getSubEditView() const
{
    // TODO: Return sub edit view from Notepad_plus
    return nullptr;
}

void MainWindow::updateTitle()
{
    // TODO: Update window title based on current document
}

void MainWindow::updateDocumentState()
{
    // TODO: Update document-related UI state
}

} // namespace MainWindow

} // namespace QtControls
