# Notepad++ Linux Port - Remaining Work

## Current Status (as of 2026-01-30)

The Linux Qt6 port has made significant progress. The text editor is now functional:

- ✅ **Text area working** - Editor widget appears and accepts text input
- ✅ **"New" command works** - File > New and Ctrl+N create new documents
- ✅ Session loading - Implemented and functional
- ✅ UserDefineDialog - Full syntax highlighting configuration
- ✅ Menu system - All menus integrated with command handlers
- ✅ Shortcut handling - Global keyboard shortcuts working
- ✅ Plugin support - Plugin loading and management implemented

**Note:** The application is now usable for basic text editing. Some secondary features may still have issues.

## Critical Issues (RESOLVED - 2026-01-30)

### Issue 1: Text Area Missing ✅ FIXED
**Status**: ✅ RESOLVED - Text editor now functional

**Fix Applied**:
- Fixed `pathAppend` forward declaration mismatch (`void` vs `std::wstring` return type) in `Parameters.cpp:73`
- Added missing `libscintilla_qt` library to Linux link in `CMakeLists.txt`
- Added Version class implementation for Linux in `CommonLinux.cpp`
- Fixed null pointer issues in `getLangExtFromLangType()` and `Buffer::getFileName()`

The text area now initializes correctly and accepts keyboard input.

### Issue 2: "New" Command Fails ✅ FIXED
**Status**: ✅ RESOLVED - Document creation now works

**Fix Applied**: Core initialization issues resolved. The editor properly creates new documents via File > New and Ctrl+N.

### Issue 3: Missing MainWindow Editor Layout ✅ FIXED
**Status**: ✅ RESOLVED - Editor views properly added to UI

**Fix Applied**: `setupUI()` now correctly initializes `ScintillaEditView`, creates `DocTabView` containers, and adds them to the splitter layout.

### Issue 4: Missing Method Implementations ✅ FIXED
**Status**: ✅ RESOLVED - All methods now implemented

**Fix Applied**:
- `ScintillaEditView::isViewWhiteSpace()` → Use `isShownSpaceAndTab()`
- `ScintillaEditView::isViewEOL()` → Use `isShownEol()`
- `NppParameters::getLangFromLangType()` → Use `getLangExtFromLangType()` with null checks
- Fixed UniMode constants (`uniUTF8` vs `uniUTF8BOM`)
- Fixed EolType enum usage (`Buffer::eolWindows`, etc.)

## Build Status

- **CMake Configuration**: ✓ Complete
- **Lexilla Library**: ✓ Building
- **Scintilla Qt6**: ✓ Building
- **Buffer/FileManager Core**: ✓ Complete
- **ScintillaEditView Integration**: ✓ FIXED - Widget creation working
- **Notepad_plus Core**: ✓ Complete - All pointers initialized
- **MainWindow UI**: ✓ FIXED - Editor views properly added
- **NppDarkMode**: ✓ Stubs implemented
- **UI Base Classes**: ✓ Complete (StaticDialog, ToolBar, StatusBar, DockingManager, etc.)
- **Ported Dialogs**: ✓ Complete (About, Run, GoToLine, FindReplace, etc.)
- **Ported Panels**: ✓ Complete (DocumentMap, FunctionList, ProjectPanel, etc.)
- **Main Executable**: ✓ Builds and runs - Text editing functional

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

### 9. Text Area Fix (2026-01-30) ✅
Fixed the missing text editor widget:
- Fixed `pathAppend` forward declaration mismatch in `Parameters.cpp:73` (return type was `void` instead of `std::wstring`)
- Added missing `libscintilla_qt` library to Linux link in `CMakeLists.txt:1051`
- Added Version class implementation for Linux in `CommonLinux.cpp:796-890`
- Fixed null pointer issues in `updateStatusBar()` for `getLangExtFromLangType()`
- Fixed null pointer issue in `updateTitle()` for `Buffer::getFileName()` - switched to `getFileNameQString()`
- Added proper widget visibility and geometry settings in `ScintillaEditView::init()`
- Fixed method name mismatches (`isViewWhiteSpace` → `isShownSpaceAndTab`, `isViewEOL` → `isShownEol`)
- Fixed UniMode enum values (`uniUTF8BOM` → `uniUTF8`, `uniCookie` → `uniUTF8_NoBOM`)
- Fixed EolType enum usage with `Buffer::eolWindows`, `Buffer::eolUnix`, `Buffer::eolMac`

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

2026-01-30 - Text editor is now functional! Core editing features working.

---

**Next Milestone:** Fix remaining secondary issues (update timer, edge cases) and complete full testing.
