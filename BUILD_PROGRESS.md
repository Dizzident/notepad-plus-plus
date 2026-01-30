# Notepad++ Linux Port - Build Progress

**Last Updated:** 2026-01-30

## Overview

This document tracks the build progress of the Notepad++ Linux Qt6 port.

---

## Build Status Summary

| Component | Status | Notes |
|-----------|--------|-------|
| CMake Configuration | ✅ Complete | Qt6 detection, build system configured |
| Lexilla Library | ✅ Building | All lexers compile successfully |
| Scintilla Qt6 | ✅ Building | Qt6 port compiles |
| **Core Backend** | ✅ **Complete** | Buffer, FileManager, ScintillaEditView, Notepad_plus |
| **UI Base Classes** | ✅ **Implemented** | StaticDialog, ToolBar, StatusBar, DockingManager |
| WinControls Dialogs | ❌ Blocking | Headers need Qt port or conditional compilation |
| **Overall Build** | ⚠️ **In Progress** | Linker errors from WinControls headers |

---

## Recent Changes (2026-01-30)

### Committed: Core Buffer, FileManager, and Notepad_plus Implementation

**Files Added/Modified:**
- `QtCore/Buffer.cpp` - Fixed Scintilla API calls to use `execute(SCI_*, ...)`
- `QtCore/Buffer.h` - BufferID type fix
- `QtCore/NppIO.cpp` - SavingStatus compatibility
- `QtCore/NppDarkMode.cpp` - Linux stubs (new file)
- `QtCore/Parameters.cpp` - Fixed Shortcut header path
- `QtControls/Notepad_plus.cpp` - Core implementation
- `QtControls/StaticDialog/` - Qt dialog base class
- `QtControls/ToolBar/` - Qt toolbar implementation
- `QtControls/StatusBar/` - Qt status bar
- `QtControls/SplitterContainer/` - Qt splitter
- `QtControls/DockingManager/` - Qt docking
- `QtControls/Window.h` - Base window class
- `QtControls/ContextMenu/` - Qt context menu
- `QtControls/Shortcut/` - Qt shortcut stubs
- `ScintillaComponent/ScintillaEditView.h` - Linux compatibility
- `WinControls/DockingWnd/DockingCont.h` - Conditional compilation
- `WinControls/shortcut/shortcut.h` - Conditional compilation
- `Notepad_plus.h` - Include Qt versions
- `CMakeLists.txt` - Include order fixes

### Implemented Methods

**FileManager (QtCore/Buffer.cpp):**
- `loadFile()` - Load files with encoding detection
- `reloadBuffer()` - Reload from disk
- `getBufferFromName()` - Find buffer by path
- `deleteBufferBackup()` - Delete backup files
- `closeBuffer()` - Close and cleanup
- `getNbBuffers()` / `getNbDirtyBuffers()`
- `bufferFromDocument()` - Create buffer from Scintilla doc
- `addBufferReference()` - Track view usage
- `saveBuffer()` - Save with encoding conversion

**Buffer Class:**
- `getFullPathName()` - Path retrieval
- `setUnsync()` / `isUnsync()` - Sync tracking

**Notepad_plus (QtControls/Notepad_plus.cpp):**
- Constructor - Initializes auto-complete, plugins, native language
- Destructor - Cleanup panels and resources
- `loadLastSession()` - Load previous session
- `loadSession()` - Full session restoration

**ScintillaEditView (QtCore/ScintillaEditViewQt.cpp):**
- `init()` - Initialize Qt Scintilla
- `attachDefaultDoc()` - Create initial buffer
- `activateBuffer()` - Switch between buffers

**NppDarkMode (QtCore/NppDarkMode.cpp):**
- `isWindowsModeEnabled()`
- `getThemeName()`
- Color functions (stubs)

---

## Current Blocking Issues

### WinControls Headers (Major Blocker)

The build fails because WinControls headers are included and use Windows-specific types:

**Error Pattern:**
```
error: expected class-name before '{' token
error: 'void ClassName::destroy()' marked 'override', but does not override
error: 'Window' has not been declared
error: '_hSelf' was not declared in this scope
```

**Affected Headers:**
- `WinControls/FindCharsInRange/FindCharsInRange.h`
- `WinControls/ColourPicker/ColourPicker.h`
- `WinControls/ColourPicker/WordStyleDlg.h`
- `WinControls/AboutDlg/URLCtrl.h`
- `WinControls/WindowsDlg/SizeableDlg.h`
- `WinControls/WindowsDlg/WindowsDlg.h`
- `WinControls/AnsiCharPanel/ListView.h`
- `WinControls/PluginsAdmin/pluginsAdmin.h`
- `WinControls/TabBar/TabBar.h`
- `WinControls/TabBar/ControlsTab.h`
- `ScintillaComponent/columnEditor.h`
- `ScintillaComponent/UserDefineDialog.h`
- `MISC/md5/md5Dlgs.h`
- `lesDlgs.h`

**Root Cause:**
- Windows types: `HWND`, `HINSTANCE`, `UINT`, `WPARAM`, `LPARAM`
- Windows base classes: `Window`, `StaticDialog` (Windows version)
- Windows APIs: `::DestroyWindow()`, `::MessageBox()`
- Member variables: `_hSelf`, `_hParent`

---

## Next Steps

### Option 1: Port Remaining Dialogs to Qt

Create Qt versions in `QtControls/`:

1. **High Priority:**
   - `FindCharsInRange` → `QtControls/FindCharsInRange/`
   - `ColourPicker` → `QtControls/ColourPicker/`
   - `WordStyleDlg` → Complete existing
   - `AboutDlg` → Complete existing
   - `WindowsDlg` → `QtControls/WindowsDlg/`
   - `ListView` → Complete existing

2. **Medium Priority:**
   - `columnEditor` → `QtControls/ColumnEditor/`
   - `PluginsAdmin` → `QtControls/PluginsAdmin/`
   - `md5Dlgs` → `QtControls/Misc/`

### Option 2: Conditional Compilation (Quick Fix)

Add `#ifdef _WIN32` guards to exclude non-essential dialogs on Linux.

### Option 3: Stub Headers

Create minimal stub headers that provide the class interface without Windows dependencies.

---

## Build Instructions

```bash
cd PowerEditor/src/build
rm -rf *
cmake .. -DBUILD_TESTS=OFF
make -j4 2>&1 | head -100
```

---

## Git Commits

### Commit 1: Core Implementation
```
Linux Port: Implement core Buffer, FileManager, and Notepad_plus methods

- Fix Buffer.cpp Scintilla API calls to use execute(SCI_*, ...) messages
- Implement FileManager::bufferFromDocument(), addBufferReference(), saveBuffer()
- Implement Notepad_plus constructor, destructor, loadSession(), loadLastSession()
- Add NppDarkMode Linux stubs for dark mode functionality
- Define g_nppStartTimePoint and g_pluginsLoadingTime globals
- Fix BufferID type to use Buffer* instead of int
- Update main_linux.cpp with Notepad_plus integration
- Update REMAINING_WORK.md with current status
```

### Commit 2: UI Base Classes
```
Linux Port: Implement UI base classes (StaticDialog, ToolBar, DockingManager)

- Port StaticDialog to Qt with proper virtual function implementations
- Implement ToolBar and ReBar for Qt
- Add SplitterContainer and DockingManager Qt implementations
- Port ScintillaEditView.h to work on Linux with Qt Window class
- Add conditional compilation for DockingCont.h and shortcut.h on Linux
- Fix CMakeLists.txt include order for Qt vs WinControls headers
- Update Window.h with getHSelf() for compatibility
```

### Commit 3: CMake Fixes
```
Linux Port: Fix CMake include order and Shortcut header path

- Add more WinControls paths to REMOVE_ITEM list to prioritize Qt versions
- Fix QtCore/Parameters.cpp to include Shortcut.h (capital S)
- Add QtControls/Shortcut to target include directories
```

---

## Summary

The Linux port has a solid foundation with:
- ✅ Complete core backend (Buffer, FileManager, ScintillaEditView)
- ✅ Complete Notepad_plus initialization and session management
- ✅ Qt UI base classes (StaticDialog, ToolBar, StatusBar, DockingManager)
- ✅ Platform abstraction layer

The remaining work is porting WinControls dialog classes to Qt or adding conditional compilation to exclude them from Linux builds.
