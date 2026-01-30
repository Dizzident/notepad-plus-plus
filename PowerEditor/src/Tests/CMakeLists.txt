# Notepad++ Tests CMakeLists.txt
# Qt Test-based testing framework for Linux port

cmake_minimum_required(VERSION 3.16)

project(NotepadPlusPlusTests VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt Test
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Test)

# Include directories
# Note: CMAKE_SOURCE_DIR is PowerEditor/src/Tests, so we need to go up to reach src/
# IMPORTANT: QtControls paths must come BEFORE any inherited WinControls paths
set(TEST_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/../Platform
    ${CMAKE_SOURCE_DIR}/../QtControls
    ${CMAKE_SOURCE_DIR}/../QtControls/StaticDialog
    ${CMAKE_SOURCE_DIR}/../QtControls/ToolBar
    ${CMAKE_SOURCE_DIR}/../QtControls/StatusBar
    ${CMAKE_SOURCE_DIR}/../QtControls/TabBar
    ${CMAKE_SOURCE_DIR}/../QtControls/Splitter
    ${CMAKE_SOURCE_DIR}/../QtControls/GoToLine
    ${CMAKE_SOURCE_DIR}/../QtControls/RunDlg
    ${CMAKE_SOURCE_DIR}/../QtControls/AboutDlg
    ${CMAKE_SOURCE_DIR}/../QtControls/TreeView
    ${CMAKE_SOURCE_DIR}/../QtControls/ListView
    ${CMAKE_SOURCE_DIR}/../QtControls/DockingManager
    ${CMAKE_SOURCE_DIR}/../QtControls/FindReplace
    ${CMAKE_SOURCE_DIR}/../QtControls/Preference
    ${CMAKE_SOURCE_DIR}/../QtControls/ShortcutMapper
    ${CMAKE_SOURCE_DIR}/../QtControls/WordStyleDlg
    ${CMAKE_SOURCE_DIR}/../QtControls/UserDefineDialog
    ${CMAKE_SOURCE_DIR}/../QtControls/ClipboardHistory
    ${CMAKE_SOURCE_DIR}/../QtControls/DocumentMap
    ${CMAKE_SOURCE_DIR}/../QtControls/FileBrowser
    ${CMAKE_SOURCE_DIR}/../QtControls/FunctionList
    ${CMAKE_SOURCE_DIR}/../QtControls/ProjectPanel
    ${CMAKE_SOURCE_DIR}/../QtControls/MainWindow
    ${CMAKE_SOURCE_DIR}/../ScintillaComponent
    ${CMAKE_SOURCE_DIR}/../QtCore
    ${CMAKE_SOURCE_DIR}/../../scintilla/include
    ${CMAKE_SOURCE_DIR}/../../lexilla/include
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Test_INCLUDE_DIRS}
)

# Common test utilities
set(COMMON_TEST_SOURCES
    Common/TestUtils.cpp
    Common/TestUtils.h
    Common/TestMain.cpp
)

# Platform Tests
set(PLATFORM_TEST_SOURCES
    Platform/FileSystemTest.cpp
    Platform/FileSystemTest.h
    Platform/SettingsTest.cpp
    Platform/SettingsTest.h
    Platform/ProcessTest.cpp
    Platform/ProcessTest.h
    Platform/FileWatcherTest.cpp
    Platform/FileWatcherTest.h
    Platform/ClipboardTest.cpp
    Platform/ClipboardTest.h
    Platform/DialogsTest.cpp
    Platform/DialogsTest.h
)

# QtControls Tests
set(QTCONTROLS_TEST_SOURCES
    QtControls/WindowTest.cpp
    QtControls/WindowTest.h
    QtControls/StaticDialogTest.cpp
    QtControls/StaticDialogTest.h
    QtControls/TreeViewTest.cpp
    QtControls/TreeViewTest.h
    QtControls/ListViewTest.cpp
    QtControls/ListViewTest.h
    QtControls/DockingManagerTest.cpp
    QtControls/DockingManagerTest.h
    QtControls/FindReplaceDlgTest.cpp
    QtControls/FindReplaceDlgTest.h
    QtControls/GoToLineDlgTest.cpp
    QtControls/GoToLineDlgTest.h
    QtControls/RunDlgTest.cpp
    QtControls/RunDlgTest.h
    QtControls/AboutDlgTest.cpp
    QtControls/AboutDlgTest.h
    QtControls/PreferenceDlgTest.cpp
    QtControls/PreferenceDlgTest.h
    QtControls/ShortcutMapperTest.cpp
    QtControls/ShortcutMapperTest.h
)

# Integration Tests
set(INTEGRATION_TEST_SOURCES
    Integration/MainWindowTest.cpp
    Integration/MainWindowTest.h
    Integration/BufferTest.cpp
    Integration/BufferTest.h
    Integration/IOTest.cpp
    Integration/IOTest.h
    Integration/CommandTest.cpp
    Integration/CommandTest.h
)

# Create individual test executables for each test category

# Platform Tests Executable
add_executable(PlatformTests
    ${COMMON_TEST_SOURCES}
    ${PLATFORM_TEST_SOURCES}
)

target_include_directories(PlatformTests PRIVATE ${TEST_INCLUDE_DIRS})

target_link_libraries(PlatformTests PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Test
)

# Link with Platform implementation
if(UNIX AND NOT APPLE)
    target_sources(PlatformTests PRIVATE
        ${CMAKE_SOURCE_DIR}/Platform/Linux/FileSystem.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Settings.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Process.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/FileWatcher.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Clipboard.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Dialogs.cpp
        ${CMAKE_SOURCE_DIR}/Platform/Linux/Threading.cpp
    )
endif()

# QtControls Tests Executable
add_executable(QtControlsTests
    ${COMMON_TEST_SOURCES}
    ${QTCONTROLS_TEST_SOURCES}
)

target_include_directories(QtControlsTests PRIVATE ${TEST_INCLUDE_DIRS})

target_link_libraries(QtControlsTests PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Test
)

# Integration Tests Executable
add_executable(IntegrationTests
    ${COMMON_TEST_SOURCES}
    ${INTEGRATION_TEST_SOURCES}
)

target_include_directories(IntegrationTests PRIVATE ${TEST_INCLUDE_DIRS})

target_link_libraries(IntegrationTests PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Test
)

# Enable testing
enable_testing()

# Add tests
add_test(NAME PlatformTests COMMAND PlatformTests)
add_test(NAME QtControlsTests COMMAND QtControlsTests)
add_test(NAME IntegrationTests COMMAND IntegrationTests)

# Test runner script
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh.in
    ${CMAKE_BINARY_DIR}/run_tests.sh
    @ONLY
)

# Make test runner executable
file(CHMOD ${CMAKE_BINARY_DIR}/run_tests.sh
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)

# Output directories
set_target_properties(PlatformTests QtControlsTests IntegrationTests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PlatformTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(QtControlsTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(IntegrationTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install targets
install(TARGETS PlatformTests QtControlsTests IntegrationTests
    RUNTIME DESTINATION bin/tests
)
