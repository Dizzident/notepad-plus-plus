# Notepad++ Linux Port - Remaining Work

## Current Status (as of 2026-01-29)

The Linux Qt6 port has made significant progress with a successful build system and many core components implemented. However, there are still several critical components that need to be completed before the application is fully functional.

## Build Status

- **CMake Configuration**: ✓ Complete
- **Lexilla Library**: ✓ Building
- **Scintilla Qt6**: ✓ Building
- **Main Executable**: ⚠️ Compiles but with missing method implementations

## Critical Missing Methods

### 1. QtCore::Buffer Class

Location: `PowerEditor/src/QtCore/Buffer.h` and `Buffer.cpp`

| Method | Status | Description |
|--------|--------|-------------|
| `setUnsync(bool)` | ❌ Missing | Marks buffer as synced/unsynced with file |
| `getFullPathName()` | ❌ Missing | Returns full file path as wchar_t* |

**Implementation Notes:**
- `getFullPathName()` should return `_filePath.toStdWString().c_str()` or similar
- `setUnsync()` should update internal sync status flag

### 2. QtCore::FileManager Class

Location: `PowerEditor/src/QtCore/Buffer.cpp` (FileManager implementation)

| Method | Status | Description |
|--------|--------|-------------|
| `reloadBuffer(BufferID)` | ❌ Missing | Reloads buffer from disk |
| `deleteBufferBackup(BufferID)` | ❌ Missing | Deletes backup file for buffer |
| `getBufferFromName(const wchar_t*)` | ❌ Missing | Finds buffer by file path |
| `loadFile(const wchar_t*, Document, int)` | ❌ Missing | Loads file into new buffer |

**Implementation Notes:**
- These methods are called from `Notepad_plus.cpp` for file operations
- FileManager acts as a singleton managing all buffers
- Need to integrate with Qt's QFile for file operations

### 3. Type/Enum Compatibility Issues

| Issue | Location | Description |
|-------|----------|-------------|
| ~~`DocLangType` vs `LangType`~~ | ~~`Buffer.h:1684`~~ | ~~Comparison between different enum types~~ **FIXED** |
| `SavingStatus` enum | `Notepad_plus.cpp:1180` | Not defined in Qt version |

**Fix Required:**
```cpp
// Add to Buffer.h or appropriate header
enum class SavingStatus {
    SaveOK,
    SaveFailed,
    SaveCancelled
};
```

### 4. Return Variable Issue

**Location:** `QtControls/Notepad_plus.cpp:1180`

```cpp
// Current (broken):
return res == SavingStatus::SaveOK;

// Problem: 'res' variable not declared in this scope
// The doSave() method needs to properly return SavingStatus
```

## UI/Integration Issues

### 1. Main Window Integration

**Location:** `main_linux.cpp`

The MainWindow class in `main_linux.cpp` has been updated to use `QtControls::MainWindow::MainWindow`, but:
- Notepad_plus core is not being initialized
- Editor components (ScintillaEditView) not connected
- Menu actions not wired to actual implementations

**Required:**
- Create and initialize `Notepad_plus` instance
- Connect MainWindow signals to Notepad_plus slots
- Initialize editor views with Scintilla

### 2. ScintillaEditView Integration

**Status:** Partially implemented

Missing features:
- Proper Scintilla document creation
- Editor view initialization
- Buffer activation in editor

## File I/O Implementation Status

### Completed ✓
- Basic Buffer class structure
- BufferManager for creating buffers
- File metadata tracking (path, name, modified time)
- Unicode mode support (UTF-8, UTF-16, etc.)
- Line ending support (Windows/Unix/Mac)

### Pending ❌
- Actual file loading from disk
- File saving with encoding conversion
- Backup file management
- File change monitoring (QFileSystemWatcher integration)
- Reload from disk

## Build Instructions

```bash
cd PowerEditor/src/build
cmake .. -DNPP_LINUX=ON
make -j4 2>&1 | head -50
```

## Priority Order for Completion

### High Priority (Required for basic functionality)

1. **Implement FileManager::loadFile()**
   - Required to open any files
   - Integrates with BufferManager

2. **Implement Buffer::getFullPathName()**
   - Simple getter method
   - Used throughout the codebase

3. **Fix doSave() return value**
   - Fix variable scoping issue
   - Define SavingStatus enum

### Medium Priority (Required for full file operations)

4. **Implement FileManager::reloadBuffer()**
   - For file reload functionality

5. **Implement FileManager::getBufferFromName()**
   - For finding existing buffers

6. **Implement Buffer::setUnsync()**
   - For tracking sync status

### Lower Priority (Nice to have)

7. **Implement FileManager::deleteBufferBackup()**
   - For backup management

8. ~~**Fix DocLangType vs LangType comparison**~~ **FIXED**
   - ~~Add proper type conversion~~
   - Changed `DocLangType` from a separate enum to a type alias (`using DocLangType = LangType;`)

9. **Complete MainWindow integration**
   - Wire up all UI actions
   - Initialize Notepad_plus core

## Testing Plan

Once methods are implemented:

1. **Basic Launch Test**
   ```bash
   ./notepad-plus-plus
   # Should show main window with menus and toolbar
   ```

2. **File Open Test**
   ```bash
   ./notepad-plus-plus /path/to/test.txt
   # Should open and display file content
   ```

3. **File Save Test**
   - Edit a file
   - Save with Ctrl+S
   - Verify changes written to disk

4. **Multiple Files Test**
   - Open multiple files
   - Switch between tabs
   - Close individual tabs

## Contributing

When implementing missing methods:

1. Look at Windows implementation in `ScintillaComponent/Buffer.cpp` for reference
2. Use Qt equivalents for Windows API calls
3. Maintain compatibility with existing Notepad_plus method signatures
4. Add proper error handling with Qt error reporting
5. Test thoroughly before committing

## Files Requiring Attention

### Core Implementation Files
- `QtCore/Buffer.h` / `Buffer.cpp` - Buffer and FileManager
- `QtCore/NppIO.cpp` - File I/O operations
- `QtControls/Notepad_plus.cpp` - Main application logic

### Integration Files
- `main_linux.cpp` - Entry point and MainWindow
- `QtControls/MainWindow/Notepad_plus_Window.cpp` - Qt MainWindow

### Supporting Files
- `QtCore/ScintillaEditViewQt.cpp` - Editor view
- `QtControls/DocTabView/DocTabView.cpp` - Tab management

## Notes

- The Windows version uses `wchar_t*` extensively for file paths
- Qt uses `QString` natively - conversion needed at API boundaries
- Scintilla document model needs to be properly initialized
- QFileSystemWatcher can replace Windows file change notification
- Qt's encoding support (QStringEncoder/Decoder) can replace Windows codepage functions

## Last Updated

2026-01-29

---

**Next Milestone:** Successfully open and display a text file in the editor.
