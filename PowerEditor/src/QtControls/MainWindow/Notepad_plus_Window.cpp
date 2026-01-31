// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "Notepad_plus_Window.h"

#include "../../menuCmdID.h"
#include "../../resource.h"
#include "../../Notepad_plus.h"
#include "../../Parameters.h"
#include "../../MISC/PluginsManager/Notepad_plus_msgs.h"

// Dialog includes
#include "../FindReplace/FindReplaceDlg.h"
#include "../GoToLine/GoToLineDlg.h"
#include "../Preference/preferenceDlg.h"
#include "../WordStyleDlg/WordStyleDlg.h"
#include "../ShortcutMapper/ShortcutMapper.h"
#include "../RunDlg/RunDlg.h"
#include "../RunMacroDlg/RunMacroDlg.h"
#include "../AboutDlg/AboutDlg.h"
#include "../AboutDlg/CmdLineArgsDlg.h"
#include "../AboutDlg/DebugInfoDlg.h"
#include "../UserDefineDialog/UserDefineDialog.h"
#include "../ScintillaComponent/ScintillaEditView.h"
#include "../ShortcutManager/ShortcutManager.h"
#include "../../WinControls/PluginsAdmin/pluginsAdminRes.h"
#include "../DocTabView/DocTabView.h"

#include <QApplication>
#include <QStyle>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>
#include <QSettings>
#include <QScreen>

#include <iostream>
#include <QActionGroup>
#include <QProcess>
#include <QDesktopServices>
#include <QDebug>
#include <QDateTime>
#include <QClipboard>

namespace QtControls {

namespace MainWindow {

// ============================================================================
// Constructor / Destructor
// ============================================================================

MainWindow::MainWindow()
    : QMainWindow(nullptr)
{
    // Set window properties
    setWindowTitle("Notepad++");
    setAcceptDrops(true);
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

    // Initialize shortcut manager first (before UI setup)
    _shortcutManager = ShortcutManager::getInstance();
    _shortcutManager->setParent(this);

    // Set up command callback for shortcut manager
    _shortcutManager->setCommandCallback([this](int commandId) {
        // Route commands to Notepad_plus core
        if (_pNotepad_plus) {
            // Use the core's command handling via WM_COMMAND-like mechanism
            // This allows shortcuts to trigger the same handlers as menu items
            switch (commandId) {
                // File commands
                case IDM_FILE_NEW: onFileNew(); break;
                case IDM_FILE_OPEN: onFileOpen(); break;
                case IDM_FILE_SAVE: onFileSave(); break;
                case IDM_FILE_SAVEAS: onFileSaveAs(); break;
                case IDM_FILE_SAVEALL: onFileSaveAll(); break;
                case IDM_FILE_CLOSE: onFileClose(); break;
                case IDM_FILE_CLOSEALL: onFileCloseAll(); break;
                case IDM_FILE_EXIT: onFileExit(); break;

                // Edit commands
                case IDM_EDIT_UNDO: onEditUndo(); break;
                case IDM_EDIT_REDO: onEditRedo(); break;
                case IDM_EDIT_CUT: onEditCut(); break;
                case IDM_EDIT_COPY: onEditCopy(); break;
                case IDM_EDIT_PASTE: onEditPaste(); break;
                case IDM_EDIT_DELETE: onEditDelete(); break;
                case IDM_EDIT_SELECTALL: onEditSelectAll(); break;

                // Search commands
                case IDM_SEARCH_FIND: onSearchFind(); break;
                case IDM_SEARCH_REPLACE: onSearchReplace(); break;
                case IDM_SEARCH_FINDNEXT: onSearchFindNext(); break;
                case IDM_SEARCH_FINDPREV: onSearchFindPrev(); break;
                case IDM_SEARCH_GOTOLINE: onSearchGoToLine(); break;

                // View commands
                case IDM_VIEW_FULLSCREENTOGGLE: onViewFullScreen(); break;
                case IDM_VIEW_POSTIT: onViewPostIt(); break;
                case IDM_VIEW_ALWAYSONTOP: onViewAlwaysOnTop(); break;
                case IDM_VIEW_WRAP: onViewWordWrap(); break;
                case IDM_VIEW_TAB_SPACE: onViewShowWhiteSpace(); break;
                case IDM_VIEW_EOL: onViewShowEOL(); break;
                case IDM_VIEW_INDENT_GUIDE: onViewShowIndentGuide(); break;
                case IDM_VIEW_FUNC_LIST: onViewFunctionList(); break;
                case IDM_VIEW_PROJECT_PANEL_1: onViewProjectPanel(); break;
                case IDM_VIEW_DOC_MAP: onViewDocumentMap(); break;
                case IDM_VIEW_FILEBROWSER: onViewFileBrowser(); break;
                case IDM_EDIT_CLIPBOARDHISTORY_PANEL: onViewClipboardHistory(); break;

                // Macro commands
                case IDM_MACRO_STARTRECORDINGMACRO: onMacroStartRecording(); break;
                case IDM_MACRO_STOPRECORDINGMACRO: onMacroStopRecording(); break;
                case IDM_MACRO_PLAYBACKRECORDEDMACRO: onMacroPlayback(); break;
                case IDM_MACRO_RUNMULTIMACRODLG: onMacroRunMultiple(); break;

                // Run commands
                case IDM_EXECUTE: onRunRun(); break;

                // Default: try to handle via core if possible
                default:
                    // For unhandled commands, we could emit a signal or log
                    qDebug() << "Unhandled command ID:" << commandId;
                    break;
            }
        }
    });

    // Setup UI components
    std::cout << "[MainWindow::init] About to setupUI..." << std::endl;
    setupUI();
    std::cout << "[MainWindow::init] setupUI done. About to connectSignals..." << std::endl;
    connectSignals();
    std::cout << "[MainWindow::init] connectSignals done. About to createDockWindows..." << std::endl;
    createDockWindows();
    std::cout << "[MainWindow::init] createDockWindows done. About to loadSettings..." << std::endl;
    loadSettings();
    std::cout << "[MainWindow::init] loadSettings done. About to updateMenuState..." << std::endl;

    // Initialize menu state
    std::cout << "[MainWindow::init] About to call updateMenuState..." << std::endl;
    updateMenuState();
    std::cout << "[MainWindow::init] updateMenuState done." << std::endl;

    // Initialize toolbar state
    std::cout << "[MainWindow::init] About to call updateToolBarState..." << std::endl;
    updateToolBarState();
    std::cout << "[MainWindow::init] updateToolBarState done." << std::endl;

    // Initialize status bar
    std::cout << "[MainWindow::init] About to call updateStatusBar..." << std::endl;
    updateStatusBar();
    std::cout << "[MainWindow::init] updateStatusBar done." << std::endl;

    // Apply shortcuts from NppParameters to registered actions
    std::cout << "[MainWindow::init] About to call applyShortcuts..." << std::endl;
    _shortcutManager->applyShortcuts();
    std::cout << "[MainWindow::init] applyShortcuts done." << std::endl;

    // Connect shortcut manager signals
    std::cout << "[MainWindow::init] About to connect shortcut signals..." << std::endl;
    connect(_shortcutManager, &ShortcutManager::shortcutsReloaded,
            this, &MainWindow::refreshShortcuts);
    std::cout << "[MainWindow::init] Shortcut signals connected." << std::endl;

    // Initialize plugin manager
    std::cout << "[MainWindow::init] About to call initPlugins..." << std::endl;
    initPlugins();
    std::cout << "[MainWindow::init] initPlugins done." << std::endl;

    std::cout << "[MainWindow::init] Initialization complete!" << std::endl;
    return true;
}

void MainWindow::setupUI()
{
    // Validate Notepad_plus pointer
    if (!_pNotepad_plus) {
        std::cerr << "[MainWindow::setupUI] ERROR: _pNotepad_plus is null!" << std::endl;
        return;
    }

    // Create central widget with splitter for editors
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create editor splitter
    _editorSplitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(_editorSplitter);

    std::cout << "[MainWindow::setupUI] Initializing main edit view..." << std::endl;

    // Initialize main edit view - check for null
    ScintillaEditView* mainEditView = _pNotepad_plus->getMainEditView();
    if (!mainEditView) {
        std::cerr << "[MainWindow::setupUI] ERROR: Main edit view is null!" << std::endl;
        return;
    }
    mainEditView->init(_editorSplitter);
    std::cout << "[MainWindow::setupUI] Main edit view widget: " << _pNotepad_plus->getMainEditView()->getWidget() << std::endl;

    // Create container for main view (tab bar + editor)
    auto* mainContainer = new QWidget(_editorSplitter);
    auto* mainVLayout = new QVBoxLayout(mainContainer);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(0);

    // Initialize main doc tab and add to container
    std::cout << "[MainWindow::setupUI] Initializing main doc tab..." << std::endl;
    _mainDocTab = _pNotepad_plus->getMainDocTab();
    if (!_mainDocTab) {
        std::cerr << "[MainWindow::setupUI] ERROR: Main doc tab is null!" << std::endl;
        return;
    }
    _mainDocTab->init(mainContainer, mainEditView);
    mainVLayout->addWidget(_mainDocTab->getWidget());

    // Add main editor to container
    QWidget* mainEditWidget = mainEditView->getWidget();
    if (!mainEditWidget) {
        std::cerr << "[MainWindow::setupUI] ERROR: Main edit widget is null!" << std::endl;
        return;
    }
    std::cout << "[MainWindow::setupUI] Adding main edit widget to layout: " << mainEditWidget << std::endl;
    mainVLayout->addWidget(mainEditWidget, 1);

    // Initialize sub edit view
    std::cout << "[MainWindow::setupUI] Initializing sub edit view..." << std::endl;
    ScintillaEditView* subEditView = _pNotepad_plus->getSubEditView();
    if (!subEditView) {
        std::cerr << "[MainWindow::setupUI] ERROR: Sub edit view is null!" << std::endl;
        return;
    }
    subEditView->init(_editorSplitter);
    auto* subContainer = new QWidget(_editorSplitter);
    auto* subVLayout = new QVBoxLayout(subContainer);
    subVLayout->setContentsMargins(0, 0, 0, 0);
    subVLayout->setSpacing(0);
    _subDocTab = _pNotepad_plus->getSubDocTab();
    if (!_subDocTab) {
        std::cerr << "[MainWindow::setupUI] ERROR: Sub doc tab is null!" << std::endl;
        return;
    }
    _subDocTab->init(subContainer, subEditView);
    subVLayout->addWidget(_subDocTab->getWidget());
    QWidget* subEditWidget = subEditView->getWidget();
    if (!subEditWidget) {
        std::cerr << "[MainWindow::setupUI] ERROR: Sub edit widget is null!" << std::endl;
        return;
    }
    subVLayout->addWidget(subEditWidget, 1);

    // Add containers to splitter and hide sub view
    _editorSplitter->addWidget(mainContainer);
    _editorSplitter->addWidget(subContainer);
    subContainer->hide();  // Only show main view by default

    // Set initial splitter sizes (80% main, 20% sub - but sub is hidden)
    QList<int> sizes;
    sizes << 800 << 200;
    _editorSplitter->setSizes(sizes);

    // Ensure main container and edit widget are visible
    mainContainer->show();
    mainEditWidget->show();

    // Also ensure central widget and splitter are visible
    centralWidget->show();
    _editorSplitter->show();

    std::cout << "[MainWindow::setupUI] Layout setup complete." << std::endl;
    std::cout << "[MainWindow::setupUI] centralWidget visible: " << centralWidget->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] _editorSplitter visible: " << _editorSplitter->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] mainContainer visible after show(): " << mainContainer->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] mainEditWidget visible after show(): " << mainEditWidget->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] mainContainer visible: " << mainContainer->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] mainEditWidget visible: " << mainEditWidget->isVisible() << std::endl;
    std::cout << "[MainWindow::setupUI] mainEditWidget size: " << mainEditWidget->width() << "x" << mainEditWidget->height() << std::endl;
    std::cout << "[MainWindow::setupUI] _editorSplitter sizes: " << _editorSplitter->sizes()[0] << ", " << _editorSplitter->sizes()[1] << std::endl;

    std::cout << "[MainWindow::setupUI] About to init menu bar..." << std::endl;
    // Initialize menu bar
    initMenuBar();
    std::cout << "[MainWindow::setupUI] Menu bar done." << std::endl;

    // Add more debugging for widget hierarchy
    std::cout << "[MainWindow::setupUI] mainContainer parent: " << mainContainer->parentWidget() << std::endl;
    std::cout << "[MainWindow::setupUI] mainEditWidget parent: " << mainEditWidget->parentWidget() << std::endl;
    std::cout << "[MainWindow::setupUI] _editorSplitter parent: " << _editorSplitter->parentWidget() << std::endl;

    std::cout << "[MainWindow::setupUI] About to init tool bar..." << std::endl;
    // Initialize toolbar
    initToolBar();
    std::cout << "[MainWindow::setupUI] Tool bar done." << std::endl;

    std::cout << "[MainWindow::setupUI] About to init status bar..." << std::endl;
    // Initialize status bar
    initStatusBar();
    std::cout << "[MainWindow::setupUI] Status bar done." << std::endl;

    std::cout << "[MainWindow::setupUI] About to create docking manager..." << std::endl;
    // Create docking manager
    _dockingManager = new DockingManager();
    std::cout << "[MainWindow::setupUI] Docking manager created." << std::endl;
    std::cout << "[MainWindow::setupUI] About to init docking manager..." << std::endl;
    _dockingManager->init(this);
    std::cout << "[MainWindow::setupUI] Docking manager init done." << std::endl;

    // Create update timer - DISABLED to prevent crash
    // _updateTimer = new QTimer(this);
    // connect(_updateTimer, &QTimer::timeout, this, [this]() {
    //     updateStatusBar();
    //     updateMenuState();
    // });
    // _updateTimer->start(500); // Update every 500ms
}

void MainWindow::connectSignals()
{
    // Connect tab close signals from DocTabView to MainWindow slots
    if (_mainDocTab) {
        connect(_mainDocTab, &DocTabView::tabCloseRequested,
                this, &MainWindow::onMainTabCloseRequested);
    }
    if (_subDocTab) {
        connect(_subDocTab, &DocTabView::tabCloseRequested,
                this, &MainWindow::onSubTabCloseRequested);
    }
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
// Window Interface
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
    createPluginsMenu();

    // Register all menu actions with shortcut manager
    registerMenuActionsWithShortcutManager();
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
    insertMenu->addAction(tr("Current Date and Time"), this, &MainWindow::onEditInsertDateTime);
    insertMenu->addAction(tr("Full File Path"), this, &MainWindow::onEditInsertFullPath);
    insertMenu->addAction(tr("File Name"), this, &MainWindow::onEditInsertFileName);
    insertMenu->addAction(tr("Current Directory"), this, &MainWindow::onEditInsertDirPath);

    // Copy to Clipboard submenu
    auto* copyToMenu = _editMenu->addMenu(tr("Copy to Clipboard"));
    copyToMenu->addAction(tr("Current Full File Path"), this, &MainWindow::onEditCopyFullPath);
    copyToMenu->addAction(tr("Current File Name"), this, &MainWindow::onEditCopyFileName);
    copyToMenu->addAction(tr("Current Directory Path"), this, &MainWindow::onEditCopyDirPath);

    // Indent submenu
    auto* indentMenu = _editMenu->addMenu(tr("Indent"));
    indentMenu->addAction(tr("Increase Line Indent"), this, &MainWindow::onEditIncreaseIndent);
    indentMenu->addAction(tr("Decrease Line Indent"), this, &MainWindow::onEditDecreaseIndent);

    // Convert submenu
    auto* convertMenu = _editMenu->addMenu(tr("Convert Case to"));
    convertMenu->addAction(tr("Uppercase"), this, &MainWindow::onEditUpperCase);
    convertMenu->addAction(tr("Lowercase"), this, &MainWindow::onEditLowerCase);
    convertMenu->addAction(tr("Title Case"), this, &MainWindow::onEditTitleCase);
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
    viewModeMenu->addAction(tr("Distraction &Free Mode"), this, &MainWindow::onViewDistractionFreeMode);

    _viewMenu->addSeparator();

    // Always on Top
    _viewMenu->addAction(tr("Always on &Top"), this, &MainWindow::onViewAlwaysOnTop);

    _viewMenu->addSeparator();

    // Word Wrap
    _wordWrapAction = _viewMenu->addAction(tr("Word &Wrap"), this, &MainWindow::onViewWordWrap);
    _wordWrapAction->setCheckable(true);

    // Show Symbols submenu
    auto* symbolsMenu = _viewMenu->addMenu(tr("Show Symbol"));
    _showWhiteSpaceAction = symbolsMenu->addAction(tr("Show White Space and TAB"), this, &MainWindow::onViewShowWhiteSpace);
    _showWhiteSpaceAction->setCheckable(true);

    _showEOLAction = symbolsMenu->addAction(tr("Show End of Line"), this, &MainWindow::onViewShowEOL);
    _showEOLAction->setCheckable(true);

    _showIndentGuideAction = symbolsMenu->addAction(tr("Show Indent Guide"), this, &MainWindow::onViewShowIndentGuide);
    _showIndentGuideAction->setCheckable(true);

    _viewMenu->addSeparator();

    // Zoom submenu
    auto* zoomMenu = _viewMenu->addMenu(tr("Zoom"));
    auto* zoomInAction = zoomMenu->addAction(tr("Zoom &In"), this, [this]() {
        if (_pNotepad_plus) {
            ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
            if (view) view->execute(SCI_ZOOMIN);
        }
    });
    zoomInAction->setShortcut(QKeySequence::ZoomIn);

    auto* zoomOutAction = zoomMenu->addAction(tr("Zoom &Out"), this, [this]() {
        if (_pNotepad_plus) {
            ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
            if (view) view->execute(SCI_ZOOMOUT);
        }
    });
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);

    zoomMenu->addAction(tr("Restore Default Zoom"), this, [this]() {
        if (_pNotepad_plus) {
            ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
            if (view) view->execute(SCI_SETZOOM, 0);
        }
    });

    _viewMenu->addSeparator();

    // Panels submenu
    auto* panelsMenu = _viewMenu->addMenu(tr("Panel"));
    auto* functionListAction = panelsMenu->addAction(tr("Function &List"), this, &MainWindow::onViewFunctionList);
    functionListAction->setCheckable(true);
    auto* projectPanelAction = panelsMenu->addAction(tr("&Project Panel"), this, &MainWindow::onViewProjectPanel);
    projectPanelAction->setCheckable(true);
    auto* documentMapAction = panelsMenu->addAction(tr("&Document Map"), this, &MainWindow::onViewDocumentMap);
    documentMapAction->setCheckable(true);
    auto* clipboardHistoryAction = panelsMenu->addAction(tr("&Clipboard History"), this, &MainWindow::onViewClipboardHistory);
    clipboardHistoryAction->setCheckable(true);
    auto* fileBrowserAction = panelsMenu->addAction(tr("Folder as &Workspace"), this, &MainWindow::onViewFileBrowser);
    fileBrowserAction->setCheckable(true);

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
    _languageMenu->addAction(tr("Define your language..."), this, &MainWindow::onLanguageDefineUserLang);
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

    _settingsMenu->addSeparator();

    // Plugin Manager
    _settingsMenu->addAction(tr("Plugins &Admin..."), this, &MainWindow::onSettingsPluginManager);
}

void MainWindow::createPluginsMenu()
{
    // Plugins menu is created dynamically after plugins are loaded
    // This will be populated by initPlugins()
    _pluginsMenu = nullptr;
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
    _windowMenu->addAction(tr("Window List"), this, &MainWindow::onWindowList);
}

void MainWindow::createHelpMenu()
{
    _helpMenu = _menuBar->addMenu(tr("&Help"));

    // About Notepad++
    _helpMenu->addAction(tr("&About Notepad++"), this, &MainWindow::onHelpAbout);

    _helpMenu->addSeparator();

    // Command Line Arguments
    _helpMenu->addAction(tr("Command Line Arguments..."), this, &MainWindow::onHelpCmdLineArgs);

    // Debug Info
    _helpMenu->addAction(tr("Debug Info..."), this, &MainWindow::onHelpDebugInfo);
}

void MainWindow::updateMenuState()
{
    // Update menu items based on current document state
    if (!_pNotepad_plus) {
        return;
    }

    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();

    if (!view || !buffer) {
        return;
    }

    // Update Edit menu actions based on document state
    bool canUndo = view->execute(SCI_CANUNDO) != 0;
    bool canRedo = view->execute(SCI_CANREDO) != 0;
    bool hasSelection = view->hasSelection();
    bool isReadOnly = buffer->isReadOnly();

    // Find and update Edit menu actions
    if (_editMenu) {
        for (QAction* action : _editMenu->actions()) {
            QString text = action->text();
            if (text.contains("Undo")) {
                action->setEnabled(canUndo && !isReadOnly);
            } else if (text.contains("Redo")) {
                action->setEnabled(canRedo && !isReadOnly);
            } else if (text.contains("Cut")) {
                action->setEnabled(hasSelection && !isReadOnly);
            } else if (text.contains("Copy")) {
                action->setEnabled(hasSelection);
            } else if (text.contains("Paste")) {
                action->setEnabled(view->execute(SCI_CANPASTE) != 0 && !isReadOnly);
            } else if (text.contains("Delete")) {
                action->setEnabled(!isReadOnly);
            }
        }
    }

    // Update View menu check states
    if (_wordWrapAction) {
        _wordWrapAction->setChecked(view->isWrap());
    }
    if (_showWhiteSpaceAction) {
        _showWhiteSpaceAction->setChecked(view->isShownSpaceAndTab());
    }
    if (_showEOLAction) {
        _showEOLAction->setChecked(view->isShownEol());
    }
    if (_showIndentGuideAction) {
        _showIndentGuideAction->setChecked(view->isShownIndentGuide());
    }

    // Update panel visibility check states in View menu
    // This is done by finding the Panel submenu and updating actions
    if (_viewMenu) {
        for (QAction* action : _viewMenu->actions()) {
            if (action->menu() && action->text() == tr("Panel")) {
                QMenu* panelsMenu = action->menu();
                for (QAction* panelAction : panelsMenu->actions()) {
                    QString panelText = panelAction->text();
                    if (panelText.contains("Function")) {
                        panelAction->setChecked(isPanelVisible("functionList"));
                    } else if (panelText.contains("Project")) {
                        panelAction->setChecked(isPanelVisible("projectPanel"));
                    } else if (panelText.contains("Document Map")) {
                        panelAction->setChecked(isPanelVisible("documentMap"));
                    } else if (panelText.contains("Clipboard")) {
                        panelAction->setChecked(isPanelVisible("clipboardHistory"));
                    } else if (panelText.contains("Workspace") || panelText.contains("Folder")) {
                        panelAction->setChecked(isPanelVisible("fileBrowser"));
                    }
                }
                break;
            }
        }
    }

    // Update Language menu - check the current language
    if (_languageMenu && buffer) {
        LangType currentLang = buffer->getLangType();
        for (QAction* action : _languageMenu->actions()) {
            if (action->isCheckable()) {
                QString langName = action->text();
                LangType actionLang = L_TEXT;

                // Map language name to LangType
                if (langName == "Normal Text") actionLang = L_TEXT;
                else if (langName == "C") actionLang = L_C;
                else if (langName == "C++") actionLang = L_CPP;
                else if (langName == "C#") actionLang = L_CS;
                else if (langName == "Java") actionLang = L_JAVA;
                else if (langName == "Python") actionLang = L_PYTHON;
                else if (langName == "JavaScript") actionLang = L_JAVASCRIPT;
                else if (langName == "HTML") actionLang = L_HTML;
                else if (langName == "CSS") actionLang = L_CSS;
                else if (langName == "XML") actionLang = L_XML;
                else if (langName == "JSON") actionLang = L_JSON;
                else if (langName == "SQL") actionLang = L_SQL;
                else if (langName == "PHP") actionLang = L_PHP;
                else if (langName == "Ruby") actionLang = L_RUBY;
                else if (langName == "Go") actionLang = L_GOLANG;
                else if (langName == "Rust") actionLang = L_RUST;
                else if (langName == "TypeScript") actionLang = L_TYPESCRIPT;
                else if (langName == "Shell" || langName == "Bash") actionLang = L_BASH;
                else if (langName == "PowerShell") actionLang = L_POWERSHELL;
                else if (langName == "Batch") actionLang = L_BATCH;
                else if (langName == "Makefile") actionLang = L_MAKEFILE;
                else if (langName == "CMake") actionLang = L_CMAKE;
                else if (langName == "YAML") actionLang = L_YAML;
                else if (langName == "Lua") actionLang = L_LUA;
                else if (langName == "Perl") actionLang = L_PERL;
                else if (langName == "R") actionLang = L_R;
                else if (langName == "Swift") actionLang = L_SWIFT;
                else if (langName == "VB") actionLang = L_VB;
                else if (langName == "ActionScript") actionLang = L_FLASH;
                else if (langName == "CoffeeScript") actionLang = L_COFFEESCRIPT;
                else if (langName == "Erlang") actionLang = L_ERLANG;
                else if (langName == "Fortran") actionLang = L_FORTRAN;
                else if (langName == "Haskell") actionLang = L_HASKELL;
                else if (langName == "Lisp") actionLang = L_LISP;
                else if (langName == "MATLAB") actionLang = L_MATLAB;
                else if (langName == "Objective-C") actionLang = L_OBJC;
                else if (langName == "Pascal") actionLang = L_PASCAL;
                else if (langName == "Raku") actionLang = L_RAKU;
                else if (langName == "Tcl") actionLang = L_TCL;
                else if (langName == "Verilog") actionLang = L_VERILOG;
                else if (langName == "VHDL") actionLang = L_VHDL;

                action->setChecked(currentLang == actionLang);
            }
        }
    }
}

// ============================================================================
// Toolbar Operations
// ============================================================================

void MainWindow::initToolBar()
{
    _mainToolBar = new ToolBar();

    // Define toolbar buttons - ToolBarButtonUnit has 10 int fields:
    // _cmdID, _defaultIcon, _grayIcon, _defaultIcon2, _grayIcon2,
    // _defaultDarkModeIcon, _grayDarkModeIcon, _defaultDarkModeIcon2, _grayDarkModeIcon2, _stdIcon
    static const ToolBarButtonUnit toolBarButtons[] = {
        // File operations
        { IDM_FILE_NEW, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_FILE_OPEN, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_FILE_SAVE, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_FILE_SAVEALL, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Separator
        // Edit operations
        { IDM_EDIT_CUT, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_EDIT_COPY, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_EDIT_PASTE, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Separator
        { IDM_EDIT_UNDO, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_EDIT_REDO, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Separator
        // Search operations
        { IDM_SEARCH_FIND, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_SEARCH_REPLACE, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Separator
        // View operations
        { IDM_VIEW_ZOOMIN, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_VIEW_ZOOMOUT, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Separator
        // Macro operations
        { IDM_MACRO_STARTRECORDINGMACRO, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_MACRO_STOPRECORDINGMACRO, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { IDM_MACRO_PLAYBACKRECORDEDMACRO, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    };

    _mainToolBar->init(this, TB_STANDARD, toolBarButtons,
                       sizeof(toolBarButtons) / sizeof(ToolBarButtonUnit));

    // Add toolbar to main window
    addToolBar(_mainToolBar->getToolBar());
}

void MainWindow::updateToolBarState()
{
    // Update toolbar button states based on current document state
    if (!_mainToolBar || !_pNotepad_plus) {
        return;
    }

    ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();

    if (!view || !buffer) {
        return;
    }

    // Update toolbar button states
    bool canUndo = view->execute(SCI_CANUNDO) != 0;
    bool canRedo = view->execute(SCI_CANREDO) != 0;
    bool hasSelection = view->hasSelection();
    bool isReadOnly = buffer->isReadOnly();

    // Enable/disable toolbar buttons based on state
    // Note: Toolbar button enabling/disabling would be implemented
    // in the ToolBar class. This is a placeholder for that logic.
    (void)canUndo;
    (void)canRedo;
    (void)hasSelection;
    (void)isReadOnly;
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

    // Update status bar information from the current editor
    ScintillaEditView* view = nullptr;
    Buffer* buffer = nullptr;

    if (_pNotepad_plus) {
        view = _pNotepad_plus->getCurrentEditView();
        buffer = _pNotepad_plus->getCurrentBuffer();
    }

    if (view && buffer) {
        // Get cursor position
        intptr_t pos = view->execute(SCI_GETCURRENTPOS);
        intptr_t line = view->execute(SCI_LINEFROMPOSITION, pos);
        intptr_t col = view->execute(SCI_GETCOLUMN, pos);

        // Line and column (1-based)
        _statusBar->setText(tr("Ln %1, Col %2").arg(line + 1).arg(col + 1), 4);

        // Selection info
        intptr_t selStart = view->execute(SCI_GETSELECTIONSTART);
        intptr_t selEnd = view->execute(SCI_GETSELECTIONEND);
        intptr_t selLength = selEnd - selStart;
        intptr_t selLines = (selLength > 0) ?
            (view->execute(SCI_LINEFROMPOSITION, selEnd) - view->execute(SCI_LINEFROMPOSITION, selStart) + 1) : 0;

        if (selLength > 0) {
            _statusBar->setText(tr("Sel %1 | %2").arg(selLength).arg(selLines), 5);
        } else {
            _statusBar->setText(tr("Sel 0 | 0"), 5);
        }

        // Document type / Language
        LangType langType = buffer->getLangType();
        const wchar_t* langNameC = NppParameters::getInstance().getLangExtFromLangType(langType);
        QString langName = (langNameC && langNameC[0]) ? QString::fromStdWString(langNameC) : tr("Normal text file");
        _statusBar->setText(langName, 3);

        // Encoding
        UniMode encoding = buffer->getUnicodeMode();
        QString encodingStr;
        switch (encoding) {
            case uniUTF8: encodingStr = "UTF-8 BOM"; break;
            case uniUTF8_NoBOM: encodingStr = "UTF-8"; break;
            case uni16BE: encodingStr = "UTF-16 BE"; break;
            case uni16LE: encodingStr = "UTF-16 LE"; break;
            case uni16BE_NoBOM: encodingStr = "UTF-16 BE"; break;
            case uni16LE_NoBOM: encodingStr = "UTF-16 LE"; break;
            case uni7Bit: encodingStr = "UTF-8"; break;
            default: encodingStr = "ANSI"; break;
        }
        _statusBar->setText(encodingStr, 2);

        // EOL format
        auto eol = buffer->getEolFormat();
        QString eolStr;
        switch (eol) {
            case Buffer::eolWindows: eolStr = "Windows (CRLF)"; break;
            case Buffer::eolMac: eolStr = "Macintosh (CR)"; break;
            case Buffer::eolUnix: eolStr = "Unix (LF)"; break;
            default: eolStr = "Windows (CRLF)"; break;
        }
        _statusBar->setText(eolStr, 1);

        // Zoom level
        int zoom = static_cast<int>(view->execute(SCI_GETZOOM));
        int zoomPercent = 100 + (zoom * 10);  // Each zoom step is 10%
        _statusBar->setText(tr("%1%").arg(zoomPercent), 6);

        // Update document modification indicator in title
        updateTitle();
    } else {
        // Default values when no document is open
        _statusBar->setText(tr("Ln 1, Col 1"), 4);
        _statusBar->setText(tr("Sel 0 | 0"), 5);
        _statusBar->setText(tr("100%"), 6);
    }
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

bool MainWindow::isTrayIconSupported() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

bool MainWindow::shouldMinimizeToTray() const
{
    if (!isTrayIconSupported()) {
        return false;
    }

    NppParameters& nppParam = NppParameters::getInstance();
    const NppGUI& nppGUI = nppParam.getNppGUI();
    int trayAction = nppGUI._isMinimizedToTray;

    return (trayAction == sta_minimize || trayAction == sta_minimize_close);
}

bool MainWindow::shouldCloseToTray() const
{
    if (!isTrayIconSupported()) {
        return false;
    }

    NppParameters& nppParam = NppParameters::getInstance();
    const NppGUI& nppGUI = nppParam.getNppGUI();
    int trayAction = nppGUI._isMinimizedToTray;

    return (trayAction == sta_close || trayAction == sta_minimize_close);
}

void MainWindow::createTrayIconMenu()
{
    if (!_trayIconMenu) {
        _trayIconMenu = new QMenu(this);

        _trayIconShowAction = new QAction(tr("Show Notepad++"), this);
        connect(_trayIconShowAction, &QAction::triggered,
                this, &MainWindow::onTrayIconShowTriggered);
        _trayIconMenu->addAction(_trayIconShowAction);

        _trayIconMenu->addSeparator();

        _trayIconExitAction = new QAction(tr("Exit"), this);
        connect(_trayIconExitAction, &QAction::triggered,
                this, &MainWindow::onTrayIconExitTriggered);
        _trayIconMenu->addAction(_trayIconExitAction);

        if (_trayIcon) {
            _trayIcon->setContextMenu(_trayIconMenu);
        }
    }
}

void MainWindow::minimizeToTray()
{
    if (!isTrayIconSupported()) {
        // Tray not supported, just minimize normally
        showMinimized();
        return;
    }

    if (!_trayIcon) {
        _trayIcon = new QSystemTrayIcon(this);

        // Try to load icon from theme or use application icon
        QIcon trayIcon = QIcon::fromTheme("notepad++");
        if (trayIcon.isNull()) {
            // Use the application window icon
            trayIcon = windowIcon();
        }
        if (trayIcon.isNull()) {
            // Fallback to a standard icon
            trayIcon = QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);
        }

        _trayIcon->setIcon(trayIcon);
        _trayIcon->setToolTip("Notepad++");

        connect(_trayIcon, &QSystemTrayIcon::activated,
                this, &MainWindow::onTrayIconActivated);

        createTrayIconMenu();
    }

    _isMinimizedToTray = true;
    _trayIcon->show();
    hide();
}

void MainWindow::restoreFromTray()
{
    _isMinimizedToTray = false;

    show();
    raise();
    activateWindow();

    if (_trayIcon) {
        _trayIcon->hide();
    }
}

void MainWindow::onTrayIconShowTriggered()
{
    restoreFromTray();
}

void MainWindow::onTrayIconExitTriggered()
{
    // Properly close the application
    _isMinimizedToTray = false;
    QApplication::quit();
}

// ============================================================================
// Event Handlers
// ============================================================================

void MainWindow::closeEvent(QCloseEvent* event)
{
    // Check if we should minimize to tray instead of closing
    if (shouldCloseToTray() && !_isMinimizedToTray) {
        minimizeToTray();
        event->ignore();
        return;
    }

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
            // Window was minimized - check if we should minimize to tray
            if (shouldMinimizeToTray() && !_isMinimizedToTray) {
                // Use a single-shot timer to allow the minimize animation to complete
                QTimer::singleShot(0, this, [this]() {
                    minimizeToTray();
                });
            }
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
    if (_pNotepad_plus) {
        _pNotepad_plus->fileNew();
    }
}

void MainWindow::onFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), QString(),
        tr("All Files (*);;Text Files (*.txt)"));

    if (!fileName.isEmpty() && _pNotepad_plus) {
        _pNotepad_plus->doOpen(fileName.toStdWString());
    }
}

void MainWindow::onFileSave()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSave();
    }
}

void MainWindow::onFileSaveAs()
{
    // Delegate to Notepad_plus which handles the save dialog and operation
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSaveAs(BUFFER_INVALID, false);
    }
}

void MainWindow::onFileSaveAll()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSaveAll();
    }
}

void MainWindow::onFileClose()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->fileClose();
    }
}

void MainWindow::onFileCloseAll()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->fileCloseAll(true, false);
    }
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
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_UNDO);
        }
    }
}

void MainWindow::onEditRedo()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_REDO);
        }
    }
}

void MainWindow::onEditCut()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_CUT);
        }
    }
}

void MainWindow::onEditCopy()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_COPY);
        }
    }
}

void MainWindow::onEditPaste()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_PASTE);
        }
    }
}

void MainWindow::onEditDelete()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_CLEAR);
        }
    }
}

void MainWindow::onEditSelectAll()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_SELECTALL);
        }
    }
}

void MainWindow::onEditInsertDateTime()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            // Get current date and time
            QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(dateTime.toUtf8().constData()));
        }
    }
}

void MainWindow::onEditInsertFullPath()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (buffer && view) {
            std::wstring path = buffer->getFullPathName();
            QString qPath = QString::fromStdWString(path);
            view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(qPath.toUtf8().constData()));
        }
    }
}

void MainWindow::onEditInsertFileName()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (buffer && view) {
            std::wstring fileName = buffer->getFileName();
            QString qFileName = QString::fromStdWString(fileName);
            view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(qFileName.toUtf8().constData()));
        }
    }
}

void MainWindow::onEditInsertDirPath()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (buffer && view) {
            std::wstring path = buffer->getFullPathName();
            size_t lastSlash = path.find_last_of(L"/\\");
            if (lastSlash != std::wstring::npos) {
                std::wstring dir = path.substr(0, lastSlash);
                QString qDir = QString::fromStdWString(dir);
                view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(qDir.toUtf8().constData()));
            }
        }
    }
}

void MainWindow::onEditCopyFullPath()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        if (buffer) {
            std::wstring path = buffer->getFullPathName();
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(QString::fromStdWString(path));
        }
    }
}

void MainWindow::onEditCopyFileName()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        if (buffer) {
            std::wstring fileName = buffer->getFileName();
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(QString::fromStdWString(fileName));
        }
    }
}

void MainWindow::onEditCopyDirPath()
{
    if (_pNotepad_plus) {
        Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
        if (buffer) {
            std::wstring path = buffer->getFullPathName();
            size_t lastSlash = path.find_last_of(L"/\\");
            if (lastSlash != std::wstring::npos) {
                std::wstring dir = path.substr(0, lastSlash);
                QClipboard* clipboard = QApplication::clipboard();
                clipboard->setText(QString::fromStdWString(dir));
            }
        }
    }
}

void MainWindow::onEditIncreaseIndent()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_TAB);
        }
    }
}

void MainWindow::onEditDecreaseIndent()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->execute(SCI_BACKTAB);
        }
    }
}

void MainWindow::onEditUpperCase()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->convertSelectedTextToUpperCase();
        }
    }
}

void MainWindow::onEditLowerCase()
{
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            view->convertSelectedTextToLowerCase();
        }
    }
}

void MainWindow::onEditTitleCase()
{
    // Title case conversion - convert first letter of each word to uppercase
    if (_pNotepad_plus) {
        ScintillaEditView* view = _pNotepad_plus->getCurrentEditView();
        if (view) {
            // Get selected text
            QByteArray selectedText;
            size_t selStart = view->execute(SCI_GETSELECTIONSTART);
            size_t selEnd = view->execute(SCI_GETSELECTIONEND);
            if (selEnd > selStart) {
                size_t len = selEnd - selStart;
                selectedText.resize(static_cast<int>(len) + 1);
                view->execute(SCI_GETSELTEXT, 0, reinterpret_cast<sptr_t>(selectedText.data()));
                QString text = QString::fromUtf8(selectedText);

                // Convert to title case
                bool newWord = true;
                for (int i = 0; i < text.length(); ++i) {
                    if (text[i].isLetter()) {
                        if (newWord) {
                            text[i] = text[i].toUpper();
                            newWord = false;
                        } else {
                            text[i] = text[i].toLower();
                        }
                    } else {
                        newWord = true;
                    }
                }

                // Replace selection
                view->execute(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(text.toUtf8().constData()));
            }
        }
    }
}

// ============================================================================
// Slot Implementations - Search Menu
// ============================================================================

void MainWindow::onSearchFind()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->showFindReplaceDlg(FIND_DLG);
    }
}

void MainWindow::onSearchReplace()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->showFindReplaceDlg(REPLACE_DLG);
    }
}

void MainWindow::onSearchFindNext()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->findNext(1); // 1 = forward direction
    }
}

void MainWindow::onSearchFindPrev()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->findNext(-1); // -1 = backward direction
    }
}

void MainWindow::onSearchGoToLine()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->showGoToLineDlg();
    }
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

void MainWindow::onViewDistractionFreeMode()
{
    toggleDistractionFreeMode();
}

void MainWindow::onViewAlwaysOnTop()
{
    setAlwaysOnTop(!isAlwaysOnTop());
}

void MainWindow::onViewWordWrap()
{
    if (!_pNotepad_plus) return;
    bool enabled = _wordWrapAction ? _wordWrapAction->isChecked() : false;
    _pNotepad_plus->wrapAllEditors(enabled);
    updateMenuState();
}

void MainWindow::onViewShowWhiteSpace()
{
    if (!_pNotepad_plus) return;
    bool enabled = _showWhiteSpaceAction ? _showWhiteSpaceAction->isChecked() : false;
    _pNotepad_plus->showWhiteSpace(enabled);
    updateMenuState();
}

void MainWindow::onViewShowEOL()
{
    if (!_pNotepad_plus) return;
    bool enabled = _showEOLAction ? _showEOLAction->isChecked() : false;
    _pNotepad_plus->showEOL(enabled);
    updateMenuState();
}

void MainWindow::onViewShowIndentGuide()
{
    if (!_pNotepad_plus) return;
    bool enabled = _showIndentGuideAction ? _showIndentGuideAction->isChecked() : false;
    _pNotepad_plus->showIndentGuide(enabled);
    updateMenuState();
}

void MainWindow::onViewFunctionList()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleFunctionList();
        updateMenuState();
    }
}

void MainWindow::onViewProjectPanel()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleProjectPanel(0);
        updateMenuState();
    }
}

void MainWindow::onViewDocumentMap()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleDocumentMap();
        updateMenuState();
    }
}

void MainWindow::onViewClipboardHistory()
{
    if (isPanelVisible("clipboardHistory")) {
        showPanel("clipboardHistory", false);
    } else {
        showPanel("clipboardHistory", true);
    }
    updateMenuState();
}

void MainWindow::onViewFileBrowser()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleFileBrowser();
        updateMenuState();
    }
}

// ============================================================================
// Slot Implementations - Encoding Menu
// ============================================================================

void MainWindow::onEncodingANSI()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->setEncoding(IDM_FORMAT_AS_UTF_8);
    }
}

void MainWindow::onEncodingUTF8()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->setEncoding(IDM_FORMAT_UTF_8);
    }
}

void MainWindow::onEncodingUTF8BOM()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->setEncoding(IDM_FORMAT_AS_UTF_8);
    }
}

void MainWindow::onEncodingUTF16BE()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->setEncoding(IDM_FORMAT_UTF_16BE);
    }
}

void MainWindow::onEncodingUTF16LE()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->setEncoding(IDM_FORMAT_UTF_16LE);
    }
}

// ============================================================================
// Slot Implementations - Language Menu
// ============================================================================

void MainWindow::onLanguageSelected(QAction* action)
{
    if (!action || !_pNotepad_plus) {
        return;
    }

    QString langName = action->text();
    // Convert language name to LangType and set it
    LangType langType = L_TEXT;
    if (langName == "C") langType = L_C;
    else if (langName == "C++") langType = L_CPP;
    else if (langName == "C#") langType = L_CS;
    else if (langName == "Java") langType = L_JAVA;
    else if (langName == "Python") langType = L_PYTHON;
    else if (langName == "JavaScript") langType = L_JAVASCRIPT;
    else if (langName == "HTML") langType = L_HTML;
    else if (langName == "CSS") langType = L_CSS;
    else if (langName == "XML") langType = L_XML;
    else if (langName == "JSON") langType = L_JSON;
    else if (langName == "SQL") langType = L_SQL;
    else if (langName == "PHP") langType = L_PHP;
    else if (langName == "Ruby") langType = L_RUBY;
    else if (langName == "Go") langType = L_GOLANG;
    else if (langName == "Rust") langType = L_RUST;
    else if (langName == "TypeScript") langType = L_TYPESCRIPT;
    else if (langName == "Shell" || langName == "Bash") langType = L_BASH;
    else if (langName == "PowerShell") langType = L_POWERSHELL;
    else if (langName == "Batch") langType = L_BATCH;
    else if (langName == "Makefile") langType = L_MAKEFILE;
    else if (langName == "CMake") langType = L_CMAKE;
    else if (langName == "Markdown") langType = L_TEXT;  // No markdown lexer yet
    else if (langName == "YAML") langType = L_YAML;
    else if (langName == "Lua") langType = L_LUA;
    else if (langName == "Perl") langType = L_PERL;
    else if (langName == "R") langType = L_R;
    else if (langName == "Swift") langType = L_SWIFT;
    else if (langName == "Kotlin") langType = L_TEXT;  // No kotlin lexer yet
    else if (langName == "Scala") langType = L_TEXT;  // No scala lexer yet
    else if (langName == "Groovy") langType = L_TEXT;  // No groovy lexer yet
    else if (langName == "VB") langType = L_VB;
    else if (langName == "VBScript") langType = L_VB;
    else if (langName == "ActionScript") langType = L_FLASH;
    else if (langName == "CoffeeScript") langType = L_COFFEESCRIPT;
    else if (langName == "Dart") langType = L_TEXT;  // No dart lexer yet
    else if (langName == "Elixir") langType = L_TEXT;  // No elixir lexer yet
    else if (langName == "Erlang") langType = L_ERLANG;
    else if (langName == "Fortran") langType = L_FORTRAN;
    else if (langName == "Haskell") langType = L_HASKELL;
    else if (langName == "Julia") langType = L_TEXT;  // No julia lexer yet
    else if (langName == "Lisp") langType = L_LISP;
    else if (langName == "MATLAB") langType = L_MATLAB;
    else if (langName == "Objective-C") langType = L_OBJC;
    else if (langName == "Pascal") langType = L_PASCAL;
    else if (langName == "Raku") langType = L_RAKU;
    else if (langName == "Tcl") langType = L_TCL;
    else if (langName == "Verilog") langType = L_VERILOG;
    else if (langName == "VHDL") langType = L_VHDL;

    // Set the language through the core
    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
    if (buffer) {
        buffer->setLangType(langType);
        // Update status bar to reflect language change
        updateStatusBar();
    }
}

void MainWindow::onLanguageDefineUserLang()
{
    if (!_userDefineDialog) {
        _userDefineDialog = new QtControls::UserDefineDialog(this);
        // Initialize with the main edit view
        ScintillaEditView* mainView = getMainEditView();
        ScintillaEditView** ppEditView = &mainView;
        _userDefineDialog->init(ppEditView);
    }
    _userDefineDialog->doDialog();
}

// ============================================================================
// Slot Implementations - Settings Menu
// ============================================================================

void MainWindow::onSettingsPreferences()
{
    if (!_preferenceDlg) {
        _preferenceDlg = new QtControls::PreferenceDlg(this);
    }
    _preferenceDlg->doDialog();
}

void MainWindow::onSettingsStyleConfigurator()
{
    if (!_wordStyleDlg) {
        _wordStyleDlg = new QtControls::WordStyleDlg(this);
        _wordStyleDlg->init();
    }
    _wordStyleDlg->doDialog();
}

void MainWindow::onSettingsShortcutMapper()
{
    if (!_shortcutMapper) {
        _shortcutMapper = new QtControls::ShortcutMapper::ShortcutMapper(this);
    }
    _shortcutMapper->doDialog();
}

void MainWindow::onSettingsPluginManager()
{
    if (!_pluginsAdminDlg) {
        _pluginsAdminDlg = new PluginsAdminDlg(this);
        _pluginsAdminDlg->create(IDD_PLUGINSADMIN_DLG, false);
        _pluginsAdminDlg->setPluginsManager(&_pluginsManager);
    }
    _pluginsAdminDlg->doDialog(false);
}

// ============================================================================
// Plugin Management
// ============================================================================

void MainWindow::initPlugins()
{
    if (!_pNotepad_plus) {
        return;
    }

    // Initialize NppData for plugins
    // Note: Scintilla handles are void* - plugins use these as identifiers
    NppData nppData;
    nppData._nppHandle = reinterpret_cast<HWND>(this);
    // Use the edit view pointers as handles - plugins treat these as opaque handles
    ScintillaEditView* mainView = _pNotepad_plus->getMainEditView();
    ScintillaEditView* subView = _pNotepad_plus->getSubEditView();
    nppData._scintillaMainHandle = reinterpret_cast<HWND>(mainView ? mainView->getHSelf() : nullptr);
    nppData._scintillaSecondHandle = reinterpret_cast<HWND>(subView ? subView->getHSelf() : nullptr);

    // Initialize plugin manager
    _pluginsManager.init(nppData);

    // Load plugins from the plugins directory
    NppParameters& nppParam = NppParameters::getInstance();
    std::wstring pluginDir = nppParam.getPluginRootDir();

    // Load plugins (without plugin admin list for now)
    _pluginsManager.loadPlugins(pluginDir.c_str(), nullptr, nullptr);

    // Initialize plugin menu - this registers commands but doesn't create Qt menus
    _pluginsManager.initMenu(nullptr, false);

    // Create Qt plugins menu if plugins were loaded
    if (_pluginsManager.hasPlugins()) {
        populatePluginsMenu();
    }
}

void MainWindow::populatePluginsMenu()
{
    // Create the Plugins menu before the Window menu
    if (!_menuBar) {
        return;
    }

    // Find the Window menu position to insert before it
    int windowMenuIndex = -1;
    for (int i = 0; i < _menuBar->actions().size(); ++i) {
        if (_menuBar->actions()[i]->menu() == _windowMenu) {
            windowMenuIndex = i;
            break;
        }
    }

    // Create Plugins menu
    _pluginsMenu = new QMenu(tr("&Plugins"), this);

    // Add "Plugins Admin..." at the top
    _pluginsMenu->addAction(tr("Plugins Admin..."), this, &MainWindow::onSettingsPluginManager);
    _pluginsMenu->addSeparator();

    // Add plugin commands
    size_t pluginCount = _pluginsManager.getPluginCount();
    for (size_t i = 0; i < pluginCount; ++i) {
        const PluginInfo* pluginInfo = _pluginsManager.getPluginInfo(i);
        if (!pluginInfo) continue;

        // Create submenu for this plugin
        QString pluginName = QString::fromStdWString(pluginInfo->_funcName);
        QMenu* pluginSubMenu = _pluginsMenu->addMenu(pluginName);

        // Add plugin commands
        for (int j = 0; j < pluginInfo->_nbFuncItem; ++j) {
            const FuncItem& funcItem = pluginInfo->_funcItems[j];

            if (funcItem._pFunc == nullptr) {
                // Separator
                pluginSubMenu->addSeparator();
            } else {
                QString itemName = QString::fromWCharArray(funcItem._itemName);
                QAction* action = pluginSubMenu->addAction(itemName, this, &MainWindow::onPluginCommandTriggered);
                // Store command ID in the action's data
                action->setData(funcItem._cmdID);

                // Set shortcut if provided
                if (funcItem._pShKey) {
                    QKeySequence shortcut;
                    QString seq;
                    if (funcItem._pShKey->_isCtrl) seq += "Ctrl+";
                    if (funcItem._pShKey->_isAlt) seq += "Alt+";
                    if (funcItem._pShKey->_isShift) seq += "Shift+";
                    seq += QKeySequence(funcItem._pShKey->_key).toString();
                    action->setShortcut(QKeySequence(seq));
                }

                // Set checkable state
                if (funcItem._init2Check) {
                    action->setCheckable(true);
                    action->setChecked(true);
                }
            }
        }
    }

    // Insert the Plugins menu before the Window menu
    if (windowMenuIndex >= 0) {
        _menuBar->insertMenu(_menuBar->actions()[windowMenuIndex], _pluginsMenu);
    } else {
        _menuBar->addMenu(_pluginsMenu);
    }
}

void MainWindow::onPluginCommandTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;

    int cmdID = action->data().toInt();
    if (cmdID <= 0) return;

    // Find and execute the plugin command
    // The command ID is ID_PLUGINS_CMD + index into _pluginsCommands
    int commandIndex = cmdID - ID_PLUGINS_CMD;
    if (commandIndex >= 0) {
        _pluginsManager.runPluginCommand(static_cast<size_t>(commandIndex));
    }
}

// ============================================================================
// Slot Implementations - Macro Menu
// ============================================================================

void MainWindow::onMacroStartRecording()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->startMacroRecording();
    }
}

void MainWindow::onMacroStopRecording()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->stopMacroRecording();
    }
}

void MainWindow::onMacroPlayback()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->macroPlayback();
    }
}

void MainWindow::onMacroRunMultiple()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->showRunMacroDlg();
    }
}

// ============================================================================
// Slot Implementations - Run Menu
// ============================================================================

void MainWindow::onRunRun()
{
    if (_pNotepad_plus) { _pNotepad_plus->showRunDlg(); }
}

void MainWindow::onRunLaunchInBrowser()
{
    if (!_pNotepad_plus) return;
    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
    if (!buffer) return;
    std::wstring filePath = buffer->getFullPathName();
    if (filePath.empty()) return;
    QString url = QString::fromStdWString(filePath);
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = QUrl::fromLocalFile(url).toString();
    }
    QDesktopServices::openUrl(QUrl(url));
}

// ============================================================================
// Slot Implementations - Window Menu
// ============================================================================

void MainWindow::onWindowNewInstance()
{
    // Launch a new Notepad++ instance
    QString appPath = QApplication::applicationFilePath();
    QProcess::startDetached(appPath, QStringList());
}

void MainWindow::onWindowSplit()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->otherView();
    }
}

void MainWindow::onWindowCloneToOtherView()
{
    if (_pNotepad_plus) {
        // Clone current document to other view
        _pNotepad_plus->otherView();
    }
}

void MainWindow::onWindowList()
{
    // Show window list dialog
    // TODO: Implement window list dialog
    if (_pNotepad_plus) {
        // Could show a dialog listing all open documents
    }
}

// ============================================================================
// Slot Implementations - Help Menu
// ============================================================================

void MainWindow::onHelpAbout()
{
    if (!_aboutDlg) {
        _aboutDlg = new QtControls::AboutDlg(this);
    }
    _aboutDlg->doDialog();
}

void MainWindow::onHelpCmdLineArgs()
{
    // Create and show command line arguments dialog
    if (!_cmdLineArgsDlg) {
        _cmdLineArgsDlg = new QtControls::CmdLineArgsDlg(this);
    }
    _cmdLineArgsDlg->doDialog();
}

void MainWindow::onHelpDebugInfo()
{
    // Create and show debug info dialog
    if (!_debugInfoDlg) {
        _debugInfoDlg = new QtControls::DebugInfoDlg(this);
    }
    _debugInfoDlg->doDialog();
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

void MainWindow::onMainTabCloseRequested(int index)
{
    if (!_pNotepad_plus || !_mainDocTab) {
        return;
    }

    // Get the buffer ID for the tab being closed
    QtControls::BufferID bufferId = _mainDocTab->getBufferByIndex(static_cast<size_t>(index));
    if (bufferId != QtControls::BUFFER_INVALID) {
        // Activate the buffer first (required for proper closing)
        _mainDocTab->activateBuffer(bufferId);
        // Call fileClose with the specific buffer ID and view
        _pNotepad_plus->fileClose(bufferId, MAIN_VIEW);
    }
}

void MainWindow::onSubTabCloseRequested(int index)
{
    if (!_pNotepad_plus || !_subDocTab) {
        return;
    }

    // Get the buffer ID for the tab being closed
    QtControls::BufferID bufferId = _subDocTab->getBufferByIndex(static_cast<size_t>(index));
    if (bufferId != QtControls::BUFFER_INVALID) {
        // Activate the buffer first (required for proper closing)
        _subDocTab->activateBuffer(bufferId);
        // Call fileClose with the specific buffer ID and view
        _pNotepad_plus->fileClose(bufferId, SUB_VIEW);
    }
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
    switch (reason) {
        case QSystemTrayIcon::DoubleClick:
            // Double-click restores the window
            restoreFromTray();
            break;

        case QSystemTrayIcon::Trigger:
            // Single click also restores for better UX
            restoreFromTray();
            break;

        case QSystemTrayIcon::Context:
            // Right-click shows context menu (handled automatically by setContextMenu)
            break;

        default:
            break;
    }
}

// ============================================================================
// Getters
// ============================================================================

ScintillaEditView* MainWindow::getMainEditView() const
{
    if (_pNotepad_plus) {
        return _pNotepad_plus->getMainEditView();
    }
    return nullptr;
}

ScintillaEditView* MainWindow::getSubEditView() const
{
    if (_pNotepad_plus) {
        return _pNotepad_plus->getSubEditView();
    }
    return nullptr;
}

void MainWindow::updateTitle()
{
    if (!_pNotepad_plus) {
        setWindowTitle("Notepad++");
        return;
    }

    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
    if (!buffer) {
        setWindowTitle("Notepad++");
        return;
    }

    QString title;

    // File name - use QString directly to avoid null pointer issues
    title = buffer->getFileNameQString();

    // Add modification indicator
    if (buffer->isDirty()) {
        title = "*" + title;
    }

    // Add read-only indicator
    if (buffer->isReadOnly()) {
        title = title + " [Read Only]";
    }

    // Add application name
    title = title + " - Notepad++";

    // Add admin indicator if running as admin (Linux doesn't typically have this concept)
    // but we could add it if needed

    setWindowTitle(title);
}

void MainWindow::updateDocumentState()
{
    // Update document-related UI state
    updateMenuState();
    updateToolBarState();
    updateStatusBar();
    updateTitle();
}

void MainWindow::refreshShortcuts()
{
    if (_shortcutManager) {
        _shortcutManager->applyShortcuts();
    }
}

// ============================================================================
// Shortcut Management
// ============================================================================

void MainWindow::registerMenuActionsWithShortcutManager()
{
    if (!_shortcutManager) {
        return;
    }

    // File menu
    if (_fileMenu) {
        for (QAction* action : _fileMenu->actions()) {
            if (action->menu()) continue; // Skip submenus for now
            QString text = action->text();
            if (text.contains("New") && !text.contains("Restore")) {
                action->setProperty("commandId", IDM_FILE_NEW);
                _shortcutManager->registerAction(IDM_FILE_NEW, action, QString("File"));
            } else if (text.contains("Open...")) {
                action->setProperty("commandId", IDM_FILE_OPEN);
                _shortcutManager->registerAction(IDM_FILE_OPEN, action, QString("File"));
            } else if (text.contains("Save") && !text.contains("As") && !text.contains("All")) {
                action->setProperty("commandId", IDM_FILE_SAVE);
                _shortcutManager->registerAction(IDM_FILE_SAVE, action, QString("File"));
            } else if (text.contains("Save As...")) {
                action->setProperty("commandId", IDM_FILE_SAVEAS);
                _shortcutManager->registerAction(IDM_FILE_SAVEAS, action, QString("File"));
            } else if (text.contains("Save All")) {
                action->setProperty("commandId", IDM_FILE_SAVEALL);
                _shortcutManager->registerAction(IDM_FILE_SAVEALL, action, QString("File"));
            } else if (text.contains("Close") && !text.contains("All")) {
                action->setProperty("commandId", IDM_FILE_CLOSE);
                _shortcutManager->registerAction(IDM_FILE_CLOSE, action, QString("File"));
            } else if (text.contains("Close All")) {
                action->setProperty("commandId", IDM_FILE_CLOSEALL);
                _shortcutManager->registerAction(IDM_FILE_CLOSEALL, action, QString("File"));
            } else if (text.contains("Exit")) {
                action->setProperty("commandId", IDM_FILE_EXIT);
                _shortcutManager->registerAction(IDM_FILE_EXIT, action, QString("File"));
            }
        }
    }

    // Edit menu
    if (_editMenu) {
        for (QAction* action : _editMenu->actions()) {
            if (action->menu()) continue;
            QString text = action->text();
            if (text.contains("Undo")) {
                action->setProperty("commandId", IDM_EDIT_UNDO);
                _shortcutManager->registerAction(IDM_EDIT_UNDO, action, QString("Edit"));
            } else if (text.contains("Redo")) {
                action->setProperty("commandId", IDM_EDIT_REDO);
                _shortcutManager->registerAction(IDM_EDIT_REDO, action, QString("Edit"));
            } else if (text.contains("Cut")) {
                action->setProperty("commandId", IDM_EDIT_CUT);
                _shortcutManager->registerAction(IDM_EDIT_CUT, action, QString("Edit"));
            } else if (text.contains("Copy")) {
                action->setProperty("commandId", IDM_EDIT_COPY);
                _shortcutManager->registerAction(IDM_EDIT_COPY, action, QString("Edit"));
            } else if (text.contains("Paste")) {
                action->setProperty("commandId", IDM_EDIT_PASTE);
                _shortcutManager->registerAction(IDM_EDIT_PASTE, action, QString("Edit"));
            } else if (text.contains("Delete")) {
                action->setProperty("commandId", IDM_EDIT_DELETE);
                _shortcutManager->registerAction(IDM_EDIT_DELETE, action, QString("Edit"));
            } else if (text.contains("Select All")) {
                action->setProperty("commandId", IDM_EDIT_SELECTALL);
                _shortcutManager->registerAction(IDM_EDIT_SELECTALL, action, QString("Edit"));
            }
        }
    }

    // Search menu
    if (_searchMenu) {
        for (QAction* action : _searchMenu->actions()) {
            if (action->menu()) continue;
            QString text = action->text();
            if (text.contains("Find...")) {
                action->setProperty("commandId", IDM_SEARCH_FIND);
                _shortcutManager->registerAction(IDM_SEARCH_FIND, action, QString("Search"));
            } else if (text.contains("Find Next")) {
                action->setProperty("commandId", IDM_SEARCH_FINDNEXT);
                _shortcutManager->registerAction(IDM_SEARCH_FINDNEXT, action, QString("Search"));
            } else if (text.contains("Find Previous")) {
                action->setProperty("commandId", IDM_SEARCH_FINDPREV);
                _shortcutManager->registerAction(IDM_SEARCH_FINDPREV, action, QString("Search"));
            } else if (text.contains("Replace...")) {
                action->setProperty("commandId", IDM_SEARCH_REPLACE);
                _shortcutManager->registerAction(IDM_SEARCH_REPLACE, action, QString("Search"));
            } else if (text.contains("Go To...")) {
                action->setProperty("commandId", IDM_SEARCH_GOTOLINE);
                _shortcutManager->registerAction(IDM_SEARCH_GOTOLINE, action, QString("Search"));
            }
        }
    }

    // View menu - register view mode actions
    if (_viewMenu) {
        for (QAction* action : _viewMenu->actions()) {
            if (action->menu()) {
                // Handle submenus
                QMenu* subMenu = action->menu();
                QString subMenuText = action->text();
                if (subMenuText == tr("View Mode")) {
                    for (QAction* viewAction : subMenu->actions()) {
                        QString viewText = viewAction->text();
                        if (viewText.contains("Full Screen")) {
                            viewAction->setProperty("commandId", IDM_VIEW_FULLSCREENTOGGLE);
                            _shortcutManager->registerAction(IDM_VIEW_FULLSCREENTOGGLE, viewAction, QString("View"));
                        } else if (viewText.contains("Post-it")) {
                            viewAction->setProperty("commandId", IDM_VIEW_POSTIT);
                            _shortcutManager->registerAction(IDM_VIEW_POSTIT, viewAction, QString("View"));
                        } else if (viewText.contains("Distraction")) {
                            viewAction->setProperty("commandId", IDM_VIEW_DISTRACTIONFREE);
                            _shortcutManager->registerAction(IDM_VIEW_DISTRACTIONFREE, viewAction, QString("View"));
                        }
                    }
                } else if (subMenuText == tr("Show Symbol")) {
                    for (QAction* symbolAction : subMenu->actions()) {
                        QString symbolText = symbolAction->text();
                        if (symbolText.contains("White Space")) {
                            symbolAction->setProperty("commandId", IDM_VIEW_TAB_SPACE);
                            _shortcutManager->registerAction(IDM_VIEW_TAB_SPACE, symbolAction, QString("View"));
                        } else if (symbolText.contains("End of Line")) {
                            symbolAction->setProperty("commandId", IDM_VIEW_EOL);
                            _shortcutManager->registerAction(IDM_VIEW_EOL, symbolAction, QString("View"));
                        } else if (symbolText.contains("Indent Guide")) {
                            symbolAction->setProperty("commandId", IDM_VIEW_INDENT_GUIDE);
                            _shortcutManager->registerAction(IDM_VIEW_INDENT_GUIDE, symbolAction, QString("View"));
                        }
                    }
                } else if (subMenuText == tr("Panel")) {
                    for (QAction* panelAction : subMenu->actions()) {
                        QString panelText = panelAction->text();
                        if (panelText.contains("Function")) {
                            panelAction->setProperty("commandId", IDM_VIEW_FUNC_LIST);
                            _shortcutManager->registerAction(IDM_VIEW_FUNC_LIST, panelAction, QString("View"));
                        } else if (panelText.contains("Project")) {
                            panelAction->setProperty("commandId", IDM_VIEW_PROJECT_PANEL_1);
                            _shortcutManager->registerAction(IDM_VIEW_PROJECT_PANEL_1, panelAction, QString("View"));
                        } else if (panelText.contains("Document Map")) {
                            panelAction->setProperty("commandId", IDM_VIEW_DOC_MAP);
                            _shortcutManager->registerAction(IDM_VIEW_DOC_MAP, panelAction, QString("View"));
                        } else if (panelText.contains("Clipboard")) {
                            panelAction->setProperty("commandId", IDM_EDIT_CLIPBOARDHISTORY_PANEL);
                            _shortcutManager->registerAction(IDM_EDIT_CLIPBOARDHISTORY_PANEL, panelAction, QString("View"));
                        } else if (panelText.contains("Workspace") || panelText.contains("Folder")) {
                            panelAction->setProperty("commandId", IDM_VIEW_FILEBROWSER);
                            _shortcutManager->registerAction(IDM_VIEW_FILEBROWSER, panelAction, QString("View"));
                        }
                    }
                }
            } else {
                QString text = action->text();
                if (text.contains("Always on Top")) {
                    action->setProperty("commandId", IDM_VIEW_ALWAYSONTOP);
                    _shortcutManager->registerAction(IDM_VIEW_ALWAYSONTOP, action, QString("View"));
                } else if (text.contains("Word Wrap")) {
                    action->setProperty("commandId", IDM_VIEW_WRAP);
                    _shortcutManager->registerAction(IDM_VIEW_WRAP, action, QString("View"));
                }
            }
        }
    }

    // Macro menu
    if (_macroMenu) {
        for (QAction* action : _macroMenu->actions()) {
            QString text = action->text();
            if (text.contains("Start Recording")) {
                action->setProperty("commandId", IDM_MACRO_STARTRECORDINGMACRO);
                _shortcutManager->registerAction(IDM_MACRO_STARTRECORDINGMACRO, action, QString("Macro"));
            } else if (text.contains("Stop Recording")) {
                action->setProperty("commandId", IDM_MACRO_STOPRECORDINGMACRO);
                _shortcutManager->registerAction(IDM_MACRO_STOPRECORDINGMACRO, action, QString("Macro"));
            } else if (text.contains("Playback")) {
                action->setProperty("commandId", IDM_MACRO_PLAYBACKRECORDEDMACRO);
                _shortcutManager->registerAction(IDM_MACRO_PLAYBACKRECORDEDMACRO, action, QString("Macro"));
            } else if (text.contains("Run a Macro Multiple")) {
                action->setProperty("commandId", IDM_MACRO_RUNMULTIMACRODLG);
                _shortcutManager->registerAction(IDM_MACRO_RUNMULTIMACRODLG, action, QString("Macro"));
            } else if (text.contains("Save Current Recorded Macro")) {
                action->setProperty("commandId", IDM_MACRO_SAVECURRENTMACRO);
                _shortcutManager->registerAction(IDM_MACRO_SAVECURRENTMACRO, action, QString("Macro"));
            }
        }
    }

    // Run menu
    if (_runMenu) {
        for (QAction* action : _runMenu->actions()) {
            QString text = action->text();
            if (text.contains("Run...")) {
                action->setProperty("commandId", IDM_EXECUTE);
                _shortcutManager->registerAction(IDM_EXECUTE, action, QString("Run"));
            }
        }
    }

    // Apply shortcuts from NppParameters
    _shortcutManager->applyShortcuts();
}

} // namespace MainWindow

} // namespace QtControls
