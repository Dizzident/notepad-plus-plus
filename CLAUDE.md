# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Notepad++ is a free source code editor and Notepad replacement. This repository contains:
- **Windows**: Native Win32 API implementation (original)
- **Linux**: Native Qt6 port using a Platform Abstraction Layer (in progress)

## Build Commands

### Linux (Qt6 Port)

```bash
# Basic build
mkdir build && cd build
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Debug build
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run the application
./notepad-plus-plus
./notepad-plus-plus /path/to/file.txt
```

### Windows

Visual Studio 2022 (v17.5+):
```bash
# Open solution
PowerEditor/visual.net/notepadPlus.sln

# Build configurations: Debug/Release x64/Win32/ARM64
```

MinGW-w64/MSYS2:
```bash
cd PowerEditor/gcc
mingw32-make
```

## Testing

### Linux Tests

```bash
# Build with tests
cmake ../PowerEditor/src -DBUILD_TESTING=ON
make -j$(nproc)

# Run all tests
./run_tests.sh

# Run specific test categories
./run_tests.sh --platform-only      # Platform abstraction tests
./run_tests.sh --qtcontrols-only    # UI control tests
./run_tests.sh --integration-only   # Integration tests

# Or use ctest
ctest --output-on-failure
```

### Windows Tests

Function list parser tests are in `PowerEditor/Test/`. Run with Python.

## Architecture

### Platform Abstraction Layer (PAL)

Located in `PowerEditor/src/Platform/`, the PAL enables cross-platform support:

| Interface | Purpose | Windows | Linux |
|-----------|---------|---------|-------|
| `IFileSystem` | File operations | Win32 API | std::filesystem + POSIX |
| `ISettings` | Configuration | Registry + INI | XDG directories + INI |
| `IProcess` | Process management | CreateProcess | fork() + exec() |
| `IFileWatcher` | File monitoring | ReadDirectoryChangesW | inotify |
| `IClipboard` | Clipboard | Win32 API | Qt Clipboard |
| `IThreading` | Thread utilities | Win32 Threads | std::thread |
| `IDialogs` | Common dialogs | Win32 Dialogs | Qt Dialogs |

Usage pattern:
```cpp
auto& fs = Platform::IFileSystem::getInstance();
fs.fileExists(path);
```

### UI Control Layers

**Windows**: `PowerEditor/src/WinControls/` - Native Win32 controls
**Linux**: `PowerEditor/src/QtControls/` - Qt6-based implementations

The QtControls framework mirrors Windows control hierarchy to minimize application layer changes. Key mappings:
- `HWND` → `QWidget*`
- `StaticDialog` → `QtControls::StaticDialog`
- `ToolBar` → `QtControls::ToolBar`

### Key Source Files

| File | Purpose |
|------|---------|
| `Notepad_plus.cpp` | Main application class (~369K) |
| `Parameters.cpp` | Configuration management (~364K) |
| `NppCommands.cpp` | Command handling (~134K) |
| `NppBigSwitch.cpp` | Message handling (~130K) |
| `NppDarkMode.cpp` | Theming (~115K) |
| `NppIO.cpp` | File I/O operations (~90K) |
| `FindReplaceDlg.cpp` | Search/replace (~228K) |
| `ScintillaEditView.cpp` | Editor component wrapper (~179K) |
| `Buffer.cpp` | Document buffer management |

### Linux Port Status

The Linux port is functional but incomplete. See `REMAINING_WORK.md` for current status. Key missing implementations:
- `QtCore::FileManager::loadFile()` - File loading
- `QtCore::Buffer::getFullPathName()` - Path retrieval
- `QtCore::Buffer::setUnsync()` - Sync tracking

## Coding Standards

From `CONTRIBUTING.md`:

### Style
- **Bracing**: Allman style (not Java-style)
- **Indentation**: Tabs (4-space equivalent)
- **Language**: C++20

### Naming
- Classes: `PascalCase`
- Methods/parameters: `camelCase`
- Member variables: prefixed with underscore (`_memberName`)

### Key Rules
- Use `!`, `&&`, `||` (not `not`, `and`, `or`)
- Prefer C++ casts over C-style casts
- Use `empty()` for string emptiness checks
- Initialize with curly braces: `MyClass instance{10.4}`
- Prefer pre-increment (`++i`) over post-increment
- Avoid `using namespace` in headers
- Use `unique_ptr` over `shared_ptr` when possible

### Platform-Specific Code

Use conditional compilation:
```cpp
#ifdef NPP_LINUX
    // Linux-specific code
#else
    // Windows code
#endif
```

CMake handles source file selection based on platform.

## Contribution Guidelines

- **First rule**: You do not ask for permission to contribute
- **Second rule**: You DO NOT ask for permission to contribute
- Create unique branch names (add dates: `feature_20240129`)
- Single feature/fix per PR
- PRs need attachment to issues (except translations/docs)
- Issues need `Accepted` label from administrators
- Respect existing coding style
- Do not reformat source code in PRs
- Test changes before submitting

## Documentation

- `README.md` - Project overview
- `BUILD.md` - Windows build instructions
- `docs/BUILD.md` - Linux build instructions
- `docs/ARCHITECTURE.md` - Detailed architecture
- `docs/PORTING.md` - Porting guide
- `REMAINING_WORK.md` - Linux port pending tasks
- `CONTRIBUTING.md` - Full contribution guidelines
