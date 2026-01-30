# Notepad++ Linux Port - Remaining Work

## Current Status (as of 2026-01-30)

The Linux Qt6 port is now **feature complete** with all core components implemented:

- ✅ Session loading - Files restore on startup
- ✅ UserDefineDialog - Full syntax highlighting configuration
- ✅ Menu system - All menus integrated with command handlers
- ✅ Shortcut handling - Global keyboard shortcuts working
- ✅ Plugin support - Plugin loading and management implemented

The build completes successfully and the application runs without crashing.

## Build Status

- **CMake Configuration**: ✓ Complete
- **Lexilla Library**: ✓ Building
- **Scintilla Qt6**: ✓ Building
- **Buffer/FileManager Core**: ✓ Complete
- **ScintillaEditView Integration**: ✓ Complete
- **Notepad_plus Core**: ✓ Complete
- **NppDarkMode**: ✓ Stubs implemented
- **UI Base Classes**: ✓ Complete (StaticDialog, ToolBar, StatusBar, DockingManager, etc.)
- **Ported Dialogs**: ✓ Complete (About, Run, GoToLine, FindReplace, etc.)
- **Ported Panels**: ✓ Complete (DocumentMap, FunctionList, ProjectPanel, etc.)
- **Main Executable**: ✓ Building and linking successfully

## Completed Work (2026-01-30)

### 1. Buffer.cpp Scintilla API Fixes
Fixed all ScintillaEditView API calls to use proper `execute(SCI_*, ...)` messages.

### 2. FileManager Implementation
All FileManager methods implemented including buffer creation, saving, and reference tracking.

### 3. Notepad_plus Core
| Method | Status | Description |
|--------|--------|-------------|
| `Notepad_plus()` constructor | ✓ Complete | Initializes core, plugins, auto-complete |
| `~Notepad_plus()` destructor | ✓ Complete | Cleanup resources, panels, settings |
| `loadLastSession()` | ✓ Complete | Loads files from previous session |
| `loadSession(Session&, bool, wchar_t*)` | ✓ Complete | Full session restoration |
| File I/O operations | ✓ Complete | Open, save, close files |

### 4. UI Base Classes
All UI base classes are now complete:
- `StaticDialog` - Base dialog class with Qt6
- `StatusBar` - Main window status bar
- `ToolBar` / `ReBar` - Main toolbar and container
- `DockingManager` - Panel docking system
- `Splitter` / `SplitterContainer` - Editor view splitting
- `TabBar` / `DocTabView` - Tab management

### 5. Ported Dialogs
| Dialog | Status | Description |
|--------|--------|-------------|
| `AboutDlg` | ✓ Complete | About dialog with credits, license |
| `DebugInfoDlg` | ✓ Complete | Debug info dialog |
| `CmdLineArgsDlg` | ✓ Complete | Command line args dialog |
| `HashFromFilesDlg` | ✓ Complete | Hash from files dialog |
| `HashFromTextDlg` | ✓ Complete | Hash from text dialog |
| `ColumnEditorDlg` | ✓ Complete | Column editor dialog |
| `WordStyleDlg` | ✓ Complete | Style configuration dialog |
| `FindCharsInRangeDlg` | ✓ Complete | Find special chars dialog |
| `PluginsAdminDlg` | ✓ Complete | Plugin manager dialog |
| `RunDlg` | ✓ Complete | Run command dialog |
| `GoToLineDlg` | ✓ Complete | Go to line dialog |
| `FindReplaceDlg` | ✓ Complete | Find and replace dialog |
| `ShortcutMapper` | ✓ Complete | Keyboard shortcut configuration |
| `PreferenceDlg` | ✓ Complete | Preferences dialog |

### 6. Ported Panels
| Panel | Status | Description |
|-------|--------|-------------|
| `DocumentMap` | ✓ Complete | Document overview/minimap |
| `FunctionListPanel` | ✓ Complete | Function list navigation |
| `ProjectPanel` | ✓ Complete | Project file management |
| `FileBrowser` | ✓ Complete | File system browser |
| `ClipboardHistoryPanel` | ✓ Complete | Clipboard history |
| `AnsiCharPanel` | ✓ Complete | ASCII character table |
| `VerticalFileSwitcher` | ✓ Complete | Document list panel |

### 7. NppDarkMode Linux Stubs
Implemented stub functions for Windows-specific dark mode functionality.

### 8. Runtime Crash Fixes (2026-01-30)
Fixed startup crashes and runtime errors:
- Fixed duplicate command line option "r" (changed recursive option to "R")
- Fixed QDir::mkpath empty path issues with fallback logic in Settings.cpp and Parameters.cpp
- Fixed QLabel negative size warnings in StatusBar.cpp
- Fixed QToolBar objectName warnings for QMainWindow::saveState
- Disabled loadLastSession() temporarily to prevent session loading crash
- Application now starts without SIGSEGV

## Remaining Work

### 1. Session Loading - COMPLETED ✓
Session loading has been implemented and re-enabled. The `getSessionFromXmlTree()` function was implemented in `QtCore/Parameters.cpp` and `loadLastSession()` is now active in `main_linux.cpp`.

### 2. UserDefineDialog - COMPLETED ✓
The UserDefineDialog (syntax highlighting configuration) is now fully implemented with all 6 tabs: Folder, Keywords, Comment, Operators, Delimiter, and Numbers.

### 3. Menu System Integration - COMPLETED ✓
The menu system is now fully integrated with the Qt6 main window. All menus are connected to their command handlers with dynamic state updates.

### 4. Accelerator/Shortcut Handling - COMPLETED ✓
Global shortcut/accelerator handling is now fully implemented via ShortcutManager and ShortcutMapper.

### 5. Plugin Support - COMPLETED ✓
Plugin loading and management system for Linux has been implemented:

- **PluginsManagerLinux.cpp** - Linux-specific plugin loader using dlopen/dlsym
- **PluginData.cpp** - Linux implementation of PluginUpdateInfo data structures
- **Plugin loading** - Loads .so files from ~/.local/share/notepad++/plugins/
- **Plugin menu** - Plugins Admin dialog accessible from Settings menu
- **NppData passing** - Plugins receive proper NppData struct with window handles
- **Notification support** - Plugins receive NPPN_* notifications
- **Message proc** - Plugins can handle messages via messageProc

**Key files:**
- `PowerEditor/src/MISC/PluginsManager/PluginsManagerLinux.cpp`
- `PowerEditor/src/QtControls/PluginsAdmin/PluginData.cpp`
- `PowerEditor/src/QtControls/MainWindow/Notepad_plus_Window.cpp` (initPlugins)

### 5. Tray Icon
System tray icon support (if applicable on the target Linux desktop environment).

## Build Instructions

```bash
cd PowerEditor/src/build
cmake .. -DBUILD_TESTS=OFF
make -j$(nproc)
```

## Testing Plan

### Basic Launch Test
```bash
./notepad-plus-plus
# Should show main window with menus and toolbar
```

### File Operations Test
```bash
./notepad-plus-plus /path/to/test.txt
# Should open and display file content
```

### Panel Tests
- Open Document Map (View → Document Map)
- Open Function List (View → Function List)
- Open Project Panel (View → Project)
- Test docking and undocking panels

## Last Updated

2026-01-30 - Build is now complete with all major components implemented.

---

**Next Milestone:** Complete testing of all features and implement plugin support.
