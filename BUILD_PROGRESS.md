# Notepad++ Linux Build Progress

## Current Status (as of 2026-01-29)

### Build System Status

| Component | Status | Notes |
|-----------|--------|-------|
| Scintilla Qt6 library | ✓ Fixed | CMake configuration updated |
| Lexilla library | ✓ Fixed | Building successfully |
| Platform Abstraction | ✓ Fixed | Common.h refactored into platform-specific headers |
| Notepad++ executable | In Progress | QtControls class hierarchy mostly fixed; remaining issues in Notepad_plus_Window.cpp and individual panel classes |

### Summary

We are approximately 85% complete with the Linux build port. The major infrastructure is in place:
- Platform abstraction layer (LinuxTypes.h)
- Qt6 alternatives for core classes (QtCore/, QtControls/, Platform/Linux/)
- CMake build system configured
- Most Windows-specific files excluded

Remaining work focuses on:
1. Panel widget accessor methods (getWidget() → getDialog() migration)
2. Abstract class implementations (run_dlgProc)
3. MainWindow integration with panels
4. Remaining Qt6 API differences

### Parallel Agent Results Summary

Multiple cpp-pro agents were launched in parallel to resolve build issues. The following fixes have been **completed**:

#### ✓ Fixed - Command Constants (Task #1, #2)
- Added missing `CMD_EDIT_BLOCK_UNCOMMENT`, `CMD_EDIT_STREAM_UNCOMMENT`
- Added `CMD_VIEW_SWITCHTO_PROJECT_PANEL_1/2/3`, `CMD_VIEW_SWITCHTO_FILEBROWSER`
- Added `CMD_VIEW_SWITCHTO_FUNC_LIST`, `CMD_VIEW_SWITCHTO_DOCLIST`
- Added `CMD_VIEW_TAB_START`, `CMD_VIEW_TAB_END`
- Fixed `IDM_` vs `CMD_` constant mismatches in NppCommands.cpp

#### ✓ Fixed - Method Access (Task #3, #4)
- Added `findMatchingBracePos()` declaration to Notepad_plus.h (public)
- Made 14 Notepad_plus methods public: `cutMarkedLines()`, `copyMarkedLines()`, `pasteToMarkedLines()`,
  `deleteMarkedLines()`, `inverseMarks()`, `goToNextIndicator()`, `goToPreviousIndicator()`,
  `fullScreenToggle()`, `postItToggle()`, `distractionFreeToggle()`, `otherView()`,
  `switchEditViewTo()`, `activateDoc()`, `activateNextDoc()`

#### ✓ Fixed - Qt Class Hierarchy (Task #9, #12, #14)
- Changed `StaticDialog` to inherit from `QDialog` instead of `Window`
- Added `Q_OBJECT` macro to `StaticDialog`
- Fixed `MainWindow` diamond inheritance (removed `StaticDialog` base, now only `QMainWindow`)
- Fixed `ListView::init()` return type conflict (changed from `bool` to `void`)
- Added `fontStyleType` enum to `WordStyleDlg.h`

#### ✓ Fixed - Build System (Task #7, #8, #11, #15, #16)
- Added core source files to CMakeLists.txt for Linux build
- Excluded Windows-only files: `Notepad_plus.cpp`, `NppBigSwitch.cpp`, `NppNotification.cpp`,
  `Parameters.cpp`, `dpiManagerV2.cpp`, `localization.cpp`, `NppDarkMode.cpp`, `NppCommands.cpp` (original)
- Fixed include path order (QtControls before WinControls)
- Added uchardet, TinyXML, pugixml, SHA libraries to build

#### ✓ Fixed - Platform Compatibility (Task #13, #17, #19, #20)
- Added `#ifndef` guards around `MB_OK` and `MB_ICONHAND` in ScintillaEditView.h
- Added `_stricmp` → `strcasecmp` mapping for Linux in EncodingMapper.cpp
- Added Linux alternatives in Utf8_16.cpp:
  - `_countof` macro
  - `_byteswap_ushort` using `__builtin_bswap16`
  - `IsTextUnicode` stub
  - `IS_TEXT_UNICODE_STATISTICS` constant

### Summary of Changes

#### 1. Refactored Common.h into Platform-Specific Headers

**Problem:** Common.h was 1,877 lines with complex nested `#ifdef` blocks that were error-prone and difficult to maintain.

**Solution:** Split into three focused headers:

**New Files Created:**
- `PowerEditor/src/MISC/Common/LinuxTypes.h` - Linux platform abstraction (types, constants, stubs)
- `PowerEditor/src/MISC/Common/WindowsTypes.h` - Windows API headers
- `PowerEditor/src/MISC/Common/Common.h` - Only truly common utilities (refactored)

**LinuxTypes.h Contents:**
- All Windows type aliases for Linux (HWND, UINT, WPARAM, DWORD, BOOL, etc.)
- All Windows structures (RECT, POINT, MSG, WINDOWPLACEMENT, MINMAXINFO, etc.)
- All Windows constants (WM_*, SW_*, MB_*, VK_*, etc.)
- All stub functions (SendMessage, ShowWindow, CreateEvent, etc.)
- ControlInfoTip stub class for Linux

**Benefits:**
- Clean separation of platform-specific code
- No complex #ifdef nesting
- Easier maintenance
- Clearer dependencies

#### 2. Headers Updated to Use New Structure

**Core Headers:**
- `Window.h` - Now includes `../MISC/Common/Common.h` instead of defining its own types
- `ToolBar.h` - Uses Common.h which includes appropriate platform header
- `StaticDialog.h` - Uses Common.h for platform types
- `DockingDlgInterface.h` - Uses Common.h

**All platform-specific code now properly isolated in:**
- `LinuxTypes.h` for Linux builds
- `WindowsTypes.h` for Windows builds

#### 3. Previously Fixed Issues (Preserved)

**CMakeLists.txt Improvements:**
- Qt6 integration with proper components (Core, Widgets, Gui, Network, PrintSupport)
- OpenSSL integration for Linux
- Platform Linux files included in build
- Proper include directories

**Name Conflicts Resolved:**
- `enum Platform` → `enum class Platform` (scoped enum)
- CP_ACP/CP_UTF8 redefinition fixed with guard macros

**Qt6 Platform Files:**
- Settings.cpp - Qt6 native APIs
- Process.cpp - Linux process implementation
- FileWatcher.cpp - QFileSystemWatcher-based implementation
- Clipboard.cpp - QClipboard-based implementation
- Threading.cpp - QThread-based implementation

### Remaining Issues

The following issues remain to be fixed:

#### 1. QtControls Base Class Hierarchy
**Status:** In Progress (Task #18)

Several QtControls classes inherit from `StaticDialog` but Qt's MOC requires proper QObject inheritance chain:

- `FinderPanel` (in FindReplaceDlg.h) - inherits from `StaticDialog`, MOC can't convert to QObject*
- Multiple dialogs marked `override` on `setupUI()` and `connectSignals()` but base class doesn't have these methods

**Required Fix:**
- Ensure `StaticDialog` properly inherits from `QDialog` with Q_OBJECT macro ✓ (Done)
- Update all derived classes to match base class interface
- Add missing `#include <QXmlStreamWriter>` and `#include <QXmlStreamReader>` to ProjectPanel.h

#### 2. Windows-Specific Files Still Being Compiled
**Status:** Needs CMakeLists.txt Update

These Windows-specific files are still being compiled and need to be excluded:

- `lastRecentFileList.cpp` - uses `CreatePopupMenu`, `RemoveMenu` (Win32 API)
- `lesDlgs.cpp` - likely Windows-specific dialog code

#### 3. Macro Redefinition Warnings
**Status:** Low Priority (Warnings, not errors)

- `MB_OK` and `MB_ICONHAND` redefined between `LinuxTypes.h` and `ScintillaEditView.h`
- `SCE_USER_MASK_NESTING_*` redefined between `UserDefineDialog.h` and `SciLexer.h`

These don't prevent compilation but should be cleaned up.

#### 4. Missing Qt Implementations
**Status:** Feature Gaps

Some Qt implementations are stubs or incomplete:
- `Parameters.cpp` - No Qt alternative (excluded from build)
- `NppNotification.cpp` - No Qt alternative (excluded from build)
- `localization.cpp` - No Qt alternative (excluded from build)

### Build Instructions

```bash
cd PowerEditor/src/build
cmake .. -DNPP_LINUX=ON
make -j$(nproc)
```

### Files Created/Modified

**New Files:**
- `PowerEditor/src/MISC/Common/LinuxTypes.h` - Linux platform abstraction
- `PowerEditor/src/MISC/Common/WindowsTypes.h` - Windows platform headers

**Modified Files:**
- `PowerEditor/src/MISC/Common/Common.h` - Refactored to only common utilities
- `PowerEditor/src/WinControls/Window.h` - Updated to include Common.h
- `PowerEditor/src/CMakeLists.txt` - Qt6 and Linux build configuration

### Parallel Agent Tasks Completed

| Task # | Description | Status | Files Modified |
|--------|-------------|--------|----------------|
| 1 | Fix missing CMD_ constants in NppCommands.h | ✓ Complete | NppCommands.h |
| 2 | Fix CMD_ vs IDM_ constant mismatches | ✓ Complete | NppCommands.cpp, NppCommands.h |
| 3 | Add findMatchingBracePos declaration | ✓ Complete | Notepad_plus.h |
| 4 | Make Notepad_plus methods accessible | ✓ Complete | Notepad_plus.h |
| 5 | Fix Qt include issues in NppCommands | ✓ Complete | NppCommands.cpp |
| 6 | Fix Notepad_plus incomplete type | ✓ Complete | NppCommands.cpp |
| 7 | Add core source files to CMake | ✓ Complete | CMakeLists.txt |
| 8 | Organize Notepad++ sources for Linux | ✓ Complete | CMakeLists.txt |
| 9 | Fix QtControls StaticDialog QObject inheritance | ✓ Complete | StaticDialog.h |
| 10 | Fix fontStyleType type in WordStyleDlg | ✓ Complete | WordStyleDlg.h |
| 11 | Fix Windows-only files in Linux build | ✓ Complete | CMakeLists.txt |
| 12 | Fix ListView::init() return type conflict | ✓ Complete | ListView.h, ListView.cpp |
| 13 | Fix MB_OK and MB_ICONHAND redefinitions | ✓ Complete | ScintillaEditView.h |
| 14 | Fix MainWindow diamond inheritance | ✓ Complete | Notepad_plus_Window.h |
| 15 | Fix include paths for Qt TreeView | ✓ Complete | CMakeLists.txt |
| 16 | Exclude more Windows-specific files | ✓ Complete | CMakeLists.txt |
| 17 | Fix EncodingMapper _stricmp issue | ✓ Complete | EncodingMapper.cpp |
| 18 | Fix QtControls TreeView QObject inheritance | ✓ Complete | Window.h |
| 19 | Fix Windows-specific functions in Utf8_16.cpp | ✓ Complete | Utf8_16.cpp |
| 20 | Fix EncodingMapper _stricmp for Linux | ✓ Complete | EncodingMapper.cpp |
| 21-24 | LinuxTypes.h, lastRecentFileList, lesDlgs, macros | ✓ Complete | LinuxTypes.h, CMakeLists.txt |
| 25 | Fix TreeView::init() return type | ✓ Complete | TreeView.h, TreeView.cpp |
| 26 | Add setupUI() and connectSignals() to StaticDialog | ✓ Complete | StaticDialog.h |
| 27 | Add Qt XML includes to ProjectPanel.h | ✓ Complete | ProjectPanel.h |
| 28 | Fix FinderPanel QObject inheritance | ✓ Complete | FindReplaceDlg.h |
| 29-30 | Exclude Common.cpp, FileInterface.cpp, DialogsLinux fix | ✓ Complete | CMakeLists.txt, Dialogs.cpp |
| 31-37 | Exclude more Windows files | ✓ Complete | CMakeLists.txt |
| 38 | Exclude ScintillaComponent Windows files | ✓ Complete | CMakeLists.txt |
| 39-41 | Exception files, ScintillaCtrls, uchardet fix | ✓ Complete | CMakeLists.txt, nsCharSetProber.cpp |
| 42 | Fix StaticDialog _widget member | ✓ Complete | StaticDialog.h, StaticDialog.cpp |
| 43 | Fix TinyXml for Linux | ✓ Complete | tinyxml.h |
| 44-46 | StaticDialog constructor, TabBar, AboutDlg | ✓ Complete | Multiple files |
| 47-51 | VK constants, GetRValue, Scintilla constants | ✓ Complete | LinuxTypes.h |
| 52-54 | setFrameStyle, QProgressBar, processFindNext | ✓ Complete | Multiple files |
| 55-58 | More VK fixes, ShortcutMapper wstring | ✓ Complete | LinuxTypes.h, ShortcutMapper.cpp |
| 59-60 | VK includes, FindReplaceDlg QString | ✓ Complete | ShortcutMapper.cpp, FindReplaceDlg.cpp |
| 61-63 | DockingManager unique_ptr, QTextEdit, abstract dialogs | ✓ Complete | DockingManager.h, UserDefineDialog files |
| 64-67 | DocumentMap, ProjectPanel fixes | ✓ Complete | DocumentMap.cpp, ProjectPanel files |
| 68-72 | QPlainTextEdit, ClipboardHistory, QRect/RECT, etc | ✓ Complete | Multiple files |

### Remaining Issues (as of commit afad021)

The following issues still need to be addressed:

#### 1. Notepad_plus_Window.cpp Panel Integration
**Status:** In Progress

Multiple panel classes need fixes for MainWindow integration:
- `ProjectPanel::getWidget()` - needs to return QWidget* (use getDialog())
- `DocumentMap` is abstract (missing run_dlgProc)
- `DocumentMap::getWidget()` - needs accessor
- `ClipboardHistoryPanel::getWidget()` - needs accessor
- `FileBrowser` is abstract (missing run_dlgProc)
- `FileBrowser::getWidget()` - needs accessor
- `FunctionListPanel` constructor/init issues
- `setFullScreen()` method missing in MainWindow

#### 2. Panel Abstract Classes
**Status:** Needs Implementation

Several panel classes are abstract because they don't implement `run_dlgProc`:
- `DocumentMap` (DocumentMap.h)
- `FileBrowser` (FileBrowser.h)
- `FunctionListPanel` (FunctionListPanel.h)

Each needs a simple `run_dlgProc()` implementation that returns false.

#### 3. QWidget Accessor Methods
**Status:** API Migration

Panels need proper QWidget accessor for DockingManager:
- Most panels inherit from StaticDialog which inherits from QDialog
- `getDialog()` method exists in StaticDialog
- Need to either expose getDialog() publicly or add getWidget() alias

#### 4. Qt6 API Updates Needed
**Status:** Pending

- `setFullScreen()` vs `showFullScreen()` in MainWindow
- Include file fixes for incomplete types

### Next Steps

Priority order:

1. **Add run_dlgProc() to abstract panel classes:**
   - DocumentMap, FileBrowser, FunctionListPanel

2. **Fix QWidget accessor methods:**
   - Make getDialog() public in StaticDialog (already done) OR add getWidget()
   - Update Notepad_plus_Window.cpp to use correct accessor

3. **Fix MainWindow setFullScreen:**
   - Add method or use Qt's showFullScreen()

4. **Complete build verification**

1. **Complete QtControls base class hierarchy fixes:**
   - Fix remaining `setupUI()` and `connectSignals()` override issues
   - Add missing Qt includes (QXmlStreamWriter/QXmlStreamReader)
   - Fix `FinderPanel` QObject inheritance

2. **Exclude remaining Windows-specific files from CMakeLists.txt:**
   - `lastRecentFileList.cpp`
   - `lesDlgs.cpp`

3. **Create Qt alternatives for excluded files:**
   - Qt version of Parameters.cpp (settings management)
   - Qt version of NppNotification.cpp (notification handling)
   - Qt version of localization.cpp (UI localization)

4. **Clean up macro redefinition warnings**

5. **Complete build verification and test executable**
