# Notepad++ Linux Port - Remaining Work

## Current Status (as of 2026-01-29)

The Linux Qt6 port has made significant progress with a successful build system and many core components implemented. However, there are still several critical components that need to be completed before the application is fully functional.

## Build Status

- **CMake Configuration**: ✓ Complete
- **Lexilla Library**: ✓ Building
- **Scintilla Qt6**: ✓ Building
- **Main Executable**: ⚠️ Compiling with ongoing implementation work
- **Buffer/FileManager Core**: ✓ Core methods implemented

## Critical Missing Methods

### 1. QtCore::Buffer Class

Location: `PowerEditor/src/QtCore/Buffer.h` and `Buffer.cpp`

| Method | Status | Description |
|--------|--------|-------------|
| `setUnsync(bool)` | ✓ **IMPLEMENTED** | Marks buffer as synced/unsynced with file |
| `getFullPathName()` | ✓ **IMPLEMENTED** | Returns full file path as wchar_t* |
| `isUnsync()` | ✓ **IMPLEMENTED** | Returns buffer sync status |

**Implementation Notes:**
- `getFullPathName()` returns `_filePath.toStdWString().c_str()` with caching
- `setUnsync()` updates internal `_isUnsync` flag with thread safety (QMutexLocker)

### 2. QtCore::FileManager Class

Location: `PowerEditor/src/QtCore/Buffer.cpp` (FileManager implementation)

| Method | Status | Description |
|--------|--------|-------------|
| `loadFile(const wchar_t*, Document, int)` | ✓ **IMPLEMENTED** | Loads file from disk into buffer |
| `reloadBuffer(BufferID)` | ✓ **IMPLEMENTED** | Reloads buffer from disk |
| `getBufferFromName(const wchar_t*)` | ✓ **IMPLEMENTED** | Finds buffer by file path |
| `deleteBufferBackup(BufferID)` | ✓ **IMPLEMENTED** | Deletes backup file for buffer |
| `closeBuffer(BufferID)` | ✓ **IMPLEMENTED** | Closes and removes buffer |
| `getNbBuffers()` | ✓ **IMPLEMENTED** | Returns total buffer count |
| `getNbDirtyBuffers()` | ✓ **IMPLEMENTED** | Returns modified buffer count |
| `getBufferByIndex(size_t)` | ✓ **IMPLEMENTED** | Returns buffer by index |
| `getBufferIndexByID(BufferID)` | ✓ **IMPLEMENTED** | Returns index for buffer ID |

**Implementation Notes:**
- All methods integrated with Qt's QFile for file operations
- FileManager acts as a singleton managing all buffers
- Uses QMutexLocker for thread-safe buffer access
- Path comparison uses canonicalFilePath() for reliability

### 3. Type/Enum Compatibility Issues

| Issue | Location | Description |
|-------|----------|-------------|
| ~~`DocLangType` vs `LangType`~~ | ~~`Buffer.h:1684`~~ | ~~Comparison between different enum types~~ **FIXED** |
| ~~`SavingStatus` enum~~ | ~~`Notepad_plus.cpp:1180`~~ | ~~Not defined in Qt version~~ **FIXED** |
| `BufferID` type | Various | Type alias defined but may need refinement |

**Fixes Applied:**
- `DocLangType` changed from enum to type alias: `using DocLangType = LangType;`
- `SavingStatus` enum added with all Windows-compatible values (SaveOK, SaveOpenFailed, SaveWritingFailed, etc.)
- `BufferID` and `Document` type aliases added for cross-platform compatibility

### 4. Return Variable Issue

**Location:** `QtControls/Notepad_plus.cpp:1180`

**Status:** ✓ **FIXED**

The `doSave()` function already properly declares `res` variable and uses it correctly. The `SavingStatus` enum is now defined and available.

```cpp
// Working implementation:
SavingStatus res = doSave(...);
return res == SavingStatus::SaveOK;
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
- **File loading from disk** (`FileManager::loadFile()`)
- **File reload from disk** (`FileManager::reloadBuffer()`)
- **Buffer lookup by name** (`FileManager::getBufferFromName()`)
- **Backup file deletion** (`FileManager::deleteBufferBackup()`)
- Buffer sync status tracking (`setUnsync()`, `isUnsync()`)
- Full path name retrieval (`getFullPathName()`)

### Pending ❌
- File saving with encoding conversion
- Backup file creation/management (partial - delete implemented)
- File change monitoring (QFileSystemWatcher integration)
- Auto-save functionality

## Build Instructions

```bash
cd PowerEditor/src/build
cmake .. -DNPP_LINUX=ON
make -j4 2>&1 | head -50
```

## Priority Order for Completion

### Completed ✓

1. ~~**Implement FileManager::loadFile()**~~ ✓ **DONE**
2. ~~**Implement Buffer::getFullPathName()**~~ ✓ **DONE**
3. ~~**Fix doSave() return value**~~ ✓ **DONE**
4. ~~**Implement FileManager::reloadBuffer()**~~ ✓ **DONE**
5. ~~**Implement FileManager::getBufferFromName()**~~ ✓ **DONE**
6. ~~**Implement Buffer::setUnsync()**~~ ✓ **DONE**
7. ~~**Implement FileManager::deleteBufferBackup()**~~ ✓ **DONE**
8. ~~**Fix DocLangType vs LangType comparison**~~ ✓ **FIXED**
9. ~~**Define SavingStatus enum**~~ ✓ **DONE**

### High Priority (Remaining blockers)

1. **DOC_UNNAMED scope issues**
   - `DOC_UNNAMED`, `DOC_REGULAR`, etc. not in scope in some files
   - Need to add proper namespace prefixes or using declarations

2. **BufferID type mismatch**
   - `buffer->getID()` returns `int` but `BufferID` is `Buffer*`
   - Need to reconcile type differences

### Medium Priority

3. **Complete MainWindow integration**
   - Notepad_plus core initialization
   - Connect MainWindow signals to Notepad_plus slots
   - Initialize editor views with Scintilla

4. **ScintillaEditView Integration**
   - Proper Scintilla document creation
   - Editor view initialization
   - Buffer activation in editor

### Lower Priority

5. **File change monitoring**
   - QFileSystemWatcher integration for external file changes

6. **Auto-save functionality**
   - Backup file creation and management

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

2026-01-29 (Updated after Buffer/FileManager implementation)

---

**Next Milestone:** Fix remaining compilation issues (DOC_UNNAMED scope, BufferID type) and successfully open a text file in the editor.
