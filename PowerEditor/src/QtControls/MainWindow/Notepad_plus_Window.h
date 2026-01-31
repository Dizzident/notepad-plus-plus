// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../ToolBar/ToolBar.h"
#include "../StatusBar/StatusBar.h"
#include "../TabBar/TabBar.h"
#include "../Splitter/Splitter.h"
#include "../DockingManager/DockingManager.h"
#include "../FunctionList/FunctionListPanel.h"
#include "../ProjectPanel/ProjectPanel.h"
#include "../DocumentMap/DocumentMap.h"
#include "../ClipboardHistory/ClipboardHistoryPanel.h"
#include "../FileBrowser/FileBrowser.h"
#include "../ShortcutManager/ShortcutManager.h"
#include "../PluginsAdmin/PluginsAdminDlg.h"
#include "../AboutDlg/CmdLineArgsDlg.h"
#include "../AboutDlg/DebugInfoDlg.h"
#include "../../MISC/PluginsManager/PluginsManager.h"

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QSplitter>
#include <QDockWidget>
#include <QToolBar>
#include <QStatusBar>
#include <QCloseEvent>
#include <QTimer>
#include <QSystemTrayIcon>

#include <vector>
#include <memory>

// Forward declaration
class Notepad_plus;
class ScintillaEditView;
namespace QtControls {
class DocTabView;
}

// Forward declarations for dialogs
namespace QtControls {
class PreferenceDlg;
class WordStyleDlg;
class GoToLineDlg;
class AboutDlg;
class UserDefineDialog;
namespace ShortcutMapper { class ShortcutMapper; }
namespace RunDlg { class RunDlg; }
class RunMacroDlg;
}

// Forward declarations for FindReplace dialogs
namespace NppFindReplace {
class FindReplaceDlg;
class FindIncrementDlg;
}

namespace QtControls {

namespace MainWindow {

// ============================================================================
// MainWindow - Qt-based main application window for Notepad++
// ============================================================================
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

    // Initialize with Notepad_plus core
    bool init(Notepad_plus* pNotepad_plus);

    // Window operations
    void destroy();
    void display(bool toShow = true);
    void reSizeTo(QRect& rc);

    // Menu operations
    void initMenuBar();
    void updateMenuState();

    // Toolbar operations
    void initToolBar();
    void updateToolBarState();

    // Status bar operations
    void initStatusBar();
    void updateStatusBar();

    // Panel management
    void showPanel(const QString& panelName, bool show);
    bool isPanelVisible(const QString& panelName);

    // Document management
    void addTab(const QString& title, const QString& filePath);
    void closeTab(int index);
    void switchTab(int index);

    // Getters for core components
    Notepad_plus* getNotepadPlus() const { return _pNotepad_plus; }
    ScintillaEditView* getMainEditView() const;
    ScintillaEditView* getSubEditView() const;
    TabBar* getTabBar() const { return _tabBar; }
    ToolBar* getToolBar() const { return _mainToolBar; }
    StatusBar* getStatusBar() const { return _statusBar; }
    DockingManager* getDockingManager() const { return _dockingManager; }

    // Panel getters
    FunctionListPanel* getFunctionListPanel() const { return _functionListPanel; }
    ProjectPanel* getProjectPanel() const { return _projectPanel; }
    DocumentMap* getDocumentMap() const { return _documentMap; }
    ClipboardHistoryPanel* getClipboardHistoryPanel() const { return _clipboardHistoryPanel; }
    FileBrowser* getFileBrowser() const { return _fileBrowser; }

    // Window state
    void saveWindowState();
    void restoreWindowState();

    // Special view modes
    void toggleFullScreen();
    void setFullScreen(bool fullScreen);
    void togglePostItMode();
    void toggleDistractionFreeMode();
    bool isFullScreen() const { return _isFullScreen; }
    bool isPostItMode() const { return _isPostItMode; }

    // Always on top
    void setAlwaysOnTop(bool alwaysOnTop);
    bool isAlwaysOnTop() const;

    // Tray icon
    void minimizeToTray();
    void restoreFromTray();
    void createTrayIconMenu();
    bool isTrayIconSupported() const;
    bool shouldMinimizeToTray() const;
    bool shouldCloseToTray() const;

    // Shortcut management
    void refreshShortcuts();

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void changeEvent(QEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    // File menu
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onFileSaveAll();
    void onFileClose();
    void onFileCloseAll();
    void onFileExit();

    // Edit menu
    void onEditUndo();
    void onEditRedo();
    void onEditCut();
    void onEditCopy();
    void onEditPaste();
    void onEditDelete();
    void onEditSelectAll();
    void onEditInsertDateTime();
    void onEditInsertFullPath();
    void onEditInsertFileName();
    void onEditInsertDirPath();
    void onEditCopyFullPath();
    void onEditCopyFileName();
    void onEditCopyDirPath();
    void onEditIncreaseIndent();
    void onEditDecreaseIndent();
    void onEditUpperCase();
    void onEditLowerCase();
    void onEditTitleCase();

    // Search menu
    void onSearchFind();
    void onSearchReplace();
    void onSearchFindNext();
    void onSearchFindPrev();
    void onSearchGoToLine();

    // View menu
    void onViewFullScreen();
    void onViewPostIt();
    void onViewDistractionFreeMode();
    void onViewAlwaysOnTop();
    void onViewWordWrap();
    void onViewShowWhiteSpace();
    void onViewShowEOL();
    void onViewShowIndentGuide();

    // Panel menu
    void onViewFunctionList();
    void onViewProjectPanel();
    void onViewDocumentMap();
    void onViewClipboardHistory();
    void onViewFileBrowser();

    // Encoding menu
    void onEncodingANSI();
    void onEncodingUTF8();
    void onEncodingUTF8BOM();
    void onEncodingUTF16BE();
    void onEncodingUTF16LE();

    // Language menu
    void onLanguageSelected(QAction* action);
    void onLanguageDefineUserLang();

    // Settings menu
    void onSettingsPreferences();
    void onSettingsStyleConfigurator();
    void onSettingsShortcutMapper();
    void onSettingsPluginManager();

    // Macro menu
    void onMacroStartRecording();
    void onMacroStopRecording();
    void onMacroPlayback();
    void onMacroRunMultiple();

    // Run menu
    void onRunRun();
    void onRunLaunchInBrowser();

    // Window menu
    void onWindowNewInstance();
    void onWindowSplit();
    void onWindowCloneToOtherView();
    void onWindowList();

    // Help menu
    void onHelpAbout();
    void onHelpCmdLineArgs();
    void onHelpDebugInfo();

    // Tab bar
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void onMainTabCloseRequested(int index);
    void onSubTabCloseRequested(int index);

    // Panel visibility
    void onPanelVisibilityChanged(bool visible);

    // Tray icon
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onTrayIconShowTriggered();
    void onTrayIconExitTriggered();

private:
    void setupUI();
    void connectSignals();
    void createDockWindows();
    void loadSettings();
    void saveSettings();

    // Menu creation helpers
    void createFileMenu();
    void createEditMenu();
    void createSearchMenu();
    void createViewMenu();
    void createEncodingMenu();
    void createLanguageMenu();
    void createSettingsMenu();
    void createMacroMenu();
    void createRunMenu();
    void createWindowMenu();
    void createHelpMenu();

    // Register menu actions with shortcut manager
    void registerMenuActionsWithShortcutManager();

    // Plugin management
    void initPlugins();
    void createPluginsMenu();
    void populatePluginsMenu();
    void onPluginCommandTriggered();

    // Update UI state
    void updateTitle();
    void updateDocumentState();

    // Core components
    Notepad_plus* _pNotepad_plus = nullptr;

    // Editor components
    QSplitter* _editorSplitter = nullptr;
    std::vector<ScintillaEditView*> _editViews;

    // Tab bar
    TabBar* _tabBar = nullptr;
    QTabWidget* _tabWidget = nullptr;

    // DocTabViews (for signal connections)
    DocTabView* _mainDocTab = nullptr;
    DocTabView* _subDocTab = nullptr;

    // Menus
    QMenuBar* _menuBar = nullptr;
    QMenu* _fileMenu = nullptr;
    QMenu* _editMenu = nullptr;
    QMenu* _searchMenu = nullptr;
    QMenu* _viewMenu = nullptr;
    QMenu* _encodingMenu = nullptr;
    QMenu* _languageMenu = nullptr;
    QMenu* _settingsMenu = nullptr;
    QMenu* _macroMenu = nullptr;
    QMenu* _runMenu = nullptr;
    QMenu* _windowMenu = nullptr;
    QMenu* _helpMenu = nullptr;
    QMenu* _pluginsMenu = nullptr;

    // Toolbars
    ToolBar* _mainToolBar = nullptr;
    ReBar* _reBar = nullptr;

    // Status bar
    StatusBar* _statusBar = nullptr;

    // Panels
    FunctionListPanel* _functionListPanel = nullptr;
    ProjectPanel* _projectPanel = nullptr;
    DocumentMap* _documentMap = nullptr;
    ClipboardHistoryPanel* _clipboardHistoryPanel = nullptr;
    FileBrowser* _fileBrowser = nullptr;

    // Dock manager
    DockingManager* _dockingManager = nullptr;

    // Tray icon
    QSystemTrayIcon* _trayIcon = nullptr;
    QMenu* _trayIconMenu = nullptr;
    QAction* _trayIconShowAction = nullptr;
    QAction* _trayIconExitAction = nullptr;
    bool _isMinimizedToTray = false;

    // Window state
    bool _isFullScreen = false;
    bool _isPostItMode = false;
    bool _isDistractionFree = false;
    QByteArray _normalWindowState;
    QByteArray _normalGeometry;

    // Settings
    QString _settingsGroup;

    // Timer for periodic updates
    QTimer* _updateTimer = nullptr;

    // Dialogs
    QtControls::PreferenceDlg* _preferenceDlg = nullptr;
    QtControls::WordStyleDlg* _wordStyleDlg = nullptr;
    QtControls::GoToLineDlg* _goToLineDlg = nullptr;
    QtControls::AboutDlg* _aboutDlg = nullptr;
    QtControls::CmdLineArgsDlg* _cmdLineArgsDlg = nullptr;
    QtControls::DebugInfoDlg* _debugInfoDlg = nullptr;
    QtControls::UserDefineDialog* _userDefineDialog = nullptr;
    QtControls::ShortcutMapper::ShortcutMapper* _shortcutMapper = nullptr;
    QtControls::RunDlg::RunDlg* _runDlg = nullptr;
    QtControls::RunMacroDlg* _runMacroDlg = nullptr;
    NppFindReplace::FindReplaceDlg* _findReplaceDlg = nullptr;

    // View menu actions (for state management)
    QAction* _wordWrapAction = nullptr;
    QAction* _showWhiteSpaceAction = nullptr;
    QAction* _showEOLAction = nullptr;
    QAction* _showIndentGuideAction = nullptr;

    // Shortcut manager
    ShortcutManager* _shortcutManager = nullptr;

    // Plugin manager
    PluginsManager _pluginsManager;
    PluginsAdminDlg* _pluginsAdminDlg = nullptr;
};

} // namespace MainWindow

} // namespace QtControls
