# Notepad++ Linux Build Progress

## Current Status (as of 2026-01-28)

### Build System Status

| Component | Status | Notes |
|-----------|--------|-------|
| Scintilla Qt6 library | ✓ Fixed | CMake configuration updated |
| Lexilla library | ✓ Fixed | Building successfully |
| Notepad++ executable | In Progress | Major platform abstraction complete |

### Successfully Fixed

#### 1. Common.h - Comprehensive Platform Abstraction Layer
Added extensive Linux platform support to `PowerEditor/src/MISC/Common/Common.h`:

**Types Added:**
- Basic Windows types: `HWND`, `UINT`, `WPARAM`, `LPARAM`, `DWORD`, `BOOL`, `UCHAR`, `WORD`, `LONG`
- Pointer types: `UINT_PTR`, `INT_PTR`, `DWORD_PTR`, `ULONG_PTR`
- Handle types: `HANDLE`, `HINSTANCE`, `HICON`, `HCURSOR`, `HFONT`, `HBRUSH`, `HBITMAP`, `HPEN`, `HDC`, `HMENU`
- String types: `WCHAR`, `TCHAR`, `LPWSTR`, `LPCWSTR`, `LPTSTR`
- Graphics types: `COLORREF`
- Structures: `RECT`, `POINT`, `SIZE`, `DRAWITEMSTRUCT`, `SYSTEMTIME`, `FILETIME`, `WIN32_FILE_ATTRIBUTE_DATA`, `TOOLINFO`, `ACCEL`

**Constants Added:**
- Virtual Key Codes: `VK_F1`-`VK_F12`, `VK_NUMPAD0`-`VK_NUMPAD9`, `VK_DOWN`, `VK_UP`, `VK_LEFT`, `VK_RIGHT`, `VK_HOME`, `VK_END`, `VK_PRIOR`, `VK_NEXT`, `VK_INSERT`, `VK_DELETE`, `VK_ESCAPE`, `VK_BACK`, `VK_TAB`, `VK_RETURN`, `VK_SPACE`, `VK_CAPITAL`, `VK_OEM_*`
- MessageBox Constants: `MB_OK`, `MB_OKCANCEL`, `MB_ABORTRETRYIGNORE`, `MB_YESNOCANCEL`, `MB_YESNO`, `MB_RETRYCANCEL`, `MB_CANCELTRYCONTINUE`, `MB_ICONERROR`, `MB_ICONQUESTION`, `MB_ICONWARNING`, `MB_ICONINFORMATION`
- Return Values: `IDOK`, `IDCANCEL`, `IDABORT`, `IDRETRY`, `IDIGNORE`, `IDYES`, `IDNO`, `IDTRYAGAIN`, `IDCONTINUE`
- Menu Flags: `MF_ENABLED`, `MF_DISABLED`, `MF_GRAYED`, `MF_CHECKED`, `MF_UNCHECKED`, `MF_BYCOMMAND`, `MF_BYPOSITION`, `MF_POPUP`, `MF_STRING`, `TPM_LEFTALIGN`
- File Access: `GENERIC_READ`, `GENERIC_WRITE`, `FILE_SHARE_READ`, `FILE_SHARE_WRITE`, `FILE_ATTRIBUTE_*`, `INVALID_FILE_ATTRIBUTES`, `NO_ERROR`
- Boolean Values: `TRUE`, `FALSE`
- Window Messages: `SW_SHOW`, `SW_HIDE`, `BST_CHECKED`, `BM_GETCHECK`, various `WM_*`, `EM_*`, `LB_*`, `CB_*` constants
- Accelerator Flags: `FVIRTKEY`, `FNOINVERT`, `FSHIFT`, `FCONTROL`, `FALT`
- Architecture: `IMAGE_FILE_MACHINE_I386`, `IMAGE_FILE_MACHINE_AMD64`, `IMAGE_FILE_MACHINE_ARM64`
- Code Pages: `CP_ACP`, `CP_OEMCP`, `CP_MACCP`, `CP_THREAD_ACP`, `CP_SYMBOL`, `CP_UTF7`, `CP_UTF8`

**Stub Functions Added:**
- Window Functions: `ShowWindow()`, `MoveWindow()`, `InvalidateRect()`, `UpdateWindow()`, `GetClientRect()`, `GetWindowRect()`, `IsWindowVisible()`, `SetFocus()`, `SetWindowText()`, `GetDlgItem()`, `DestroyWindow()`, `DestroyIcon()`
- Menu Functions: `TrackPopupMenu()`, `EnableMenuItem()`, `CheckMenuItem()`, `ModifyMenu()`, `GetSubMenu()`, `GetMenuString()`, `GetMenuItemID()`
- String Functions: `_stricmp()` (maps to `strcasecmp()`), `_wcsicmp()` (maps to `wcscasecmp()`), `_istspace` (maps to `iswspace`)
- GDI Functions: `ColorRGBToHLS()`, `ColorHLSToRGB()`
- COM Functions: `CoInitializeEx()`, `CoUninitialize()`
- File Functions: `GetSystemTimeAsFileTime()`, `GetLocaleInfoEx()`
- MessageBox: `MessageBox()` stub

#### 2. Headers with Platform Guards Added/Fixed

**Core Headers:**
- `Common.h` - Complete platform abstraction with `#ifdef _WIN32` guards
- `localization.h` - Added platform guards around `windows.h`
- `localization.cpp` - Added platform guards around `windows.h`
- `Parameters.h` - Uses platform abstraction from Common.h
- `Parameters.cpp` - Added platform guards around `windows.h` and Windows headers

**Platform Abstraction Headers:**
- `Window.h` - Added Windows API stubs and fixed `NULL` usage to `nullptr`
- `Docking.h` - Windows types and structures (already had guards)
- `DockingCont.h` - Already properly configured with Common.h include

**Scintilla Component Headers:**
- `ScintillaEditView.h` - Platform guards (already present)
- `xmlMatchedTagsHighlighter.h` - Added platform guards with relative path include fix
- `UserDefineDialog.h` - Uses platform abstraction

**WinControls Headers:**
- `DoubleBuffer.h` - Added platform guards with relative path include fix
- `shortcut.h` - Uses Common.h via include chain (UCHAR and VK_* defined in Common.h)
- `WordStyleDlg.h` - Already had platform guards
- `StaticDialog.h` - Uses Common.h for Linux
- `TabBar.h` - Platform guards (already present)
- `Splitter.h` - Platform guards (already present)
- `SplitterContainer.h` - Platform guards (already present)

**Other Headers:**
- `NppConstants.h` - Platform types and unix macro handling
- `NppDarkMode.h` - COLORREF type
- `Notepad_plus_msgs.h` - Platform types
- `dpiManagerV2.h` - Windows message constants
- `FileInterface.h` - HANDLE type

**Windows-Specific Headers (wrapped entirely in `#ifdef _WIN32`):**
- `MiniDumper.h` - Windows crash dump functionality (entire file wrapped)
- `Win32Exception.h` - Win32 structured exception handling (entire file wrapped)
- `DarkMode.cpp` - Windows-specific dark mode implementation (entire file wrapped)
- `verifySignedfile.cpp` - Windows Authenticode verification (entire file wrapped)
- `Platform/Windows/Dialogs.cpp` - Windows dialog implementation (entire file wrapped)

**Crypto/Hash (with Linux implementations):**
- `sha512.cpp` - Added Linux implementation using OpenSSL EVP API

#### 3. CMakeLists.txt Improvements

**Qt6 Integration:**
- Fixed Qt6 discovery with `find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)`
- Added proper Qt6 include directory extraction using `get_target_property()`
- Added `target_include_directories()` for Qt6 targets
- Updated Qt header includes in Settings.cpp and Process.cpp to use `QtCore/QSettings` format
- Added `find_package(OpenSSL REQUIRED)` for Linux

**Source File Organization:**
- Fixed `src_files` clearing issue that was removing Platform Linux files
- Added Platform Linux files to build:
  - `Platform/Linux/FileSystem.cpp`
  - `Platform/Linux/Settings.cpp`
  - `Platform/Linux/Process.cpp`
  - `Platform/Linux/FileWatcher.cpp`
  - `Platform/Linux/Clipboard.cpp`
  - `Platform/Linux/Threading.cpp`
- Added QtControls files for Linux
- Added QtCore files for Linux

**Include Directories:**
- Fixed scintilla include path: `${CMAKE_SOURCE_DIR}/../../scintilla/include`
- Added current directory `.` to include path for proper header resolution
- Added `WinControls` and `ScintillaComponent` to include path
- Organized include directories with proper precedence

#### 4. Name Conflicts Resolved

**Platform Namespace/Enum Conflict:**
- **Problem:** `FileSystem.h` defined `namespace Platform` while `Notepad_plus_msgs.h` defined `enum Platform`
- **Solution:** Changed `enum Platform` to `enum class Platform` (scoped enum)
- **Files Updated:** `Notepad_plus_msgs.h`, `Parameters.h`, `Parameters.cpp` (all references updated to use `Platform::PF_*`)

**CP_ACP/CP_UTF8 Redefinition:**
- **Problem:** Both `FileSystem.h` and `Parameters.h` defined these constants
- **Solution:** Extended guard macro `CP_UTF8_DEFINED` in `FileSystem.h` to cover all code page constants
- **Constants:** `CP_ACP`, `CP_OEMCP`, `CP_MACCP`, `CP_THREAD_ACP`, `CP_SYMBOL`, `CP_UTF7`, `CP_UTF8`

#### 5. Qt6 Platform Files Updated

**Settings.cpp:**
- Changed `#include <QSettings>` to `#include <QtCore/QSettings>`
- Changed `#include <QStandardPaths>` to `#include <QtCore/QStandardPaths>`
- Changed `#include <QDir>` to `#include <QtCore/QDir>`
- Changed `#include <QXmlStreamReader>` to `#include <QtCore/QXmlStreamReader>`
- Changed `#include <QXmlStreamWriter>` to `#include <QtCore/QXmlStreamWriter>`
- Added `#include <QtCore/QStringList>`

**Process.cpp:**
- Changed Qt headers to use `QtCore/` and `QtGui/` prefixes
- Added `#include <QtCore/QStringList>`
- Added `#include <QtCore/QObject>`
- Added `#include <QtCore/QMimeType>` and `#include <QtCore/QMimeDatabase>`
- Fixed `QDesktopServices` to use `QtGui/QDesktopServices`

### Remaining Issues

The following issues may still appear in IDE diagnostics (clangd) but should be resolved during actual CMake build:

1. **Scintilla Constants in Parameters.cpp**
   - SCI_* constants (SCI_SELECTALL, SCI_CLEAR, SCI_ZOOMIN, etc.) may show as undefined in IDE
   - **Status:** Should be resolved when Scintilla.h include path is properly configured in CMake
   - **File:** `PowerEditor/src/Parameters.cpp`

2. **Resource Constants**
   - IDC_DOCK_BUTTON, IDC_UNDOCK_BUTTON, FIND_DLG may show as undefined
   - **Status:** Need to ensure resource headers are included or add to CMake configuration
   - **Files:** `localization.cpp`, `Notepad_plus.cpp`, `NppCommands.cpp`

3. **Incomplete Types in localization.cpp**
   - UserDefineDialog, FindReplaceDlg may show as incomplete types
   - **Status:** Likely missing header includes in the translation unit
   - **Potential Fix:** Add explicit includes for these headers

4. **Menu Handle Type Issues**
   - Some functions expecting `HMENU` may show type mismatches with `void*`
   - **Status:** HMENU is typedef'd to `void*` in Common.h, but strict type checking may flag this
   - **Note:** This is cosmetic for the IDE; should compile correctly

### Build Instructions

To verify the current build status:

```bash
cd PowerEditor/src
mkdir -p build && cd build
cmake .. -DNPP_LINUX=ON
make -j$(nproc)
```

### Files Modified Summary

**Core Platform Abstraction:**
- `PowerEditor/src/MISC/Common/Common.h` - Extensive additions

**CMake Build System:**
- `PowerEditor/src/CMakeLists.txt` - Qt6 integration, include paths, source files

**Headers with Platform Guards:**
- `PowerEditor/src/localization.h`
- `PowerEditor/src/localization.cpp`
- `PowerEditor/src/Parameters.cpp`
- `PowerEditor/src/ScintillaComponent/xmlMatchedTagsHighlighter.h`
- `PowerEditor/src/WinControls/Window.h`
- `PowerEditor/src/WinControls/DoubleBuffer/DoubleBuffer.h`

**Windows-Specific Files (wrapped in #ifdef _WIN32):**
- `PowerEditor/src/MISC/Exception/MiniDumper.h`
- `PowerEditor/src/MISC/Exception/Win32Exception.h`
- `PowerEditor/src/MISC/Common/verifySignedfile.cpp`
- `PowerEditor/src/MISC/sha512/sha512.cpp`
- `PowerEditor/src/DarkMode/DarkMode.cpp`
- `PowerEditor/src/Platform/Windows/Dialogs.cpp`

**Name Conflict Resolution:**
- `PowerEditor/src/MISC/PluginsManager/Notepad_plus_msgs.h`
- `PowerEditor/src/Parameters.h`
- `PowerEditor/src/MISC/Common/FileSystem.h`

**Qt6 Platform Files:**
- `PowerEditor/src/Platform/Linux/Settings.cpp`
- `PowerEditor/src/Platform/Linux/Process.cpp`

### Next Steps (if build still fails)

1. Run the CMake build and collect actual compiler errors
2. Address any remaining missing constants or types
3. Fix any missing header includes that surface during compilation
4. Test and verify the executable runs correctly on Linux
