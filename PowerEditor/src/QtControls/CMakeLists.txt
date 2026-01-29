# QtControls CMakeLists.txt
# Qt6-based UI controls for Linux

cmake_minimum_required(VERSION 3.16)

project(QtControls VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# QtControls source files
set(QTCONTROLS_SOURCES
    Window.cpp
    StaticDialog/StaticDialog.cpp
    ToolBar/ToolBar.cpp
    StatusBar/StatusBar.cpp
    TabBar/TabBar.cpp
    DocTabView/DocTabView.cpp
    Splitter/Splitter.cpp
    GoToLine/GoToLineDlg.cpp
    RunDlg/RunDlg.cpp
    RunMacroDlg/RunMacroDlg.cpp
    AboutDlg/AboutDlg.cpp
    TreeView/TreeView.cpp
    ShortcutMapper/ShortcutMapper.cpp
    WordStyleDlg/WordStyleDlg.cpp
    UserDefineDialog/UserDefineDialog.cpp
    Preference/preferenceDlg.cpp
    ListView/ListView.cpp
    ClipboardHistory/ClipboardHistoryPanel.cpp
    DocumentMap/DocumentMap.cpp
    FileBrowser/FileBrowser.cpp
    FunctionList/FunctionListPanel.cpp
    ProjectPanel/ProjectPanel.cpp
    MainWindow/Notepad_plus_Window.cpp
)

# Optional KDE Frameworks integration
option(NPP_ENABLE_KDE "Enable KDE Frameworks integration for native Plasma theming" OFF)
if(NPP_ENABLE_KDE)
    find_package(Qt6 COMPONENTS DBus QUIET)
    if(Qt6DBus_FOUND)
        message(STATUS "Qt6 D-Bus found - enabling KDE style integration")
        list(APPEND QTCONTROLS_SOURCES
            KDEStyle/KDEStyleManager.cpp
            KDEStyle/KDEUtils.cpp
        )
        add_definitions(-DNPP_KDE_AVAILABLE)
        list(APPEND QTCONTROLS_EXTRA_LIBS Qt6::DBus)
    else()
        message(WARNING "Qt6 D-Bus not found - KDE style integration disabled")
    endif()
else()
    message(STATUS "KDE integration disabled (use -DNPP_ENABLE_KDE=ON to enable)")
endif()

set(QTCONTROLS_HEADERS
    Window.h
    StaticDialog/StaticDialog.h
    ToolBar/ToolBar.h
    StatusBar/StatusBar.h
    TabBar/TabBar.h
    DocTabView/DocTabView.h
    Splitter/Splitter.h
    GoToLine/GoToLineDlg.h
    RunDlg/RunDlg.h
    RunMacroDlg/RunMacroDlg.h
    AboutDlg/AboutDlg.h
    TreeView/TreeView.h
    ShortcutMapper/ShortcutMapper.h
    WordStyleDlg/WordStyleDlg.h
    UserDefineDialog/UserDefineDialog.h
    Preference/preferenceDlg.h
    ListView/ListView.h
    ClipboardHistory/ClipboardHistoryPanel.h
    DocumentMap/DocumentMap.h
    FileBrowser/FileBrowser.h
    FunctionList/FunctionListPanel.h
    ProjectPanel/ProjectPanel.h
    MainWindow/Notepad_plus_Window.h
)

# Add KDE headers only if KDE integration is enabled
if(NPP_ENABLE_KDE AND Qt6DBus_FOUND)
    list(APPEND QTCONTROLS_HEADERS
        KDEStyle/KDEStyleManager.h
        KDEStyle/KDEUtils.h
    )
endif()

# Create static library
add_library(QtControls STATIC ${QTCONTROLS_SOURCES} ${QTCONTROLS_HEADERS})

target_include_directories(QtControls PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/StaticDialog
    ${CMAKE_CURRENT_SOURCE_DIR}/ToolBar
    ${CMAKE_CURRENT_SOURCE_DIR}/StatusBar
    ${CMAKE_CURRENT_SOURCE_DIR}/TabBar
    ${CMAKE_CURRENT_SOURCE_DIR}/DocTabView
    ${CMAKE_CURRENT_SOURCE_DIR}/Splitter
    ${CMAKE_CURRENT_SOURCE_DIR}/GoToLine
    ${CMAKE_CURRENT_SOURCE_DIR}/RunDlg
    ${CMAKE_CURRENT_SOURCE_DIR}/RunMacroDlg
    ${CMAKE_CURRENT_SOURCE_DIR}/AboutDlg
    ${CMAKE_CURRENT_SOURCE_DIR}/TreeView
    ${CMAKE_CURRENT_SOURCE_DIR}/ShortcutMapper
    ${CMAKE_CURRENT_SOURCE_DIR}/WordStyleDlg
    ${CMAKE_CURRENT_SOURCE_DIR}/UserDefineDialog
    ${CMAKE_CURRENT_SOURCE_DIR}/Preference
    ${CMAKE_CURRENT_SOURCE_DIR}/ListView
    ${CMAKE_CURRENT_SOURCE_DIR}/ClipboardHistory
    ${CMAKE_CURRENT_SOURCE_DIR}/DocumentMap
    ${CMAKE_CURRENT_SOURCE_DIR}/FileBrowser
    ${CMAKE_CURRENT_SOURCE_DIR}/FunctionList
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectPanel
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow
)

# Add KDE include path only if KDE integration is enabled
if(NPP_ENABLE_KDE AND Qt6DBus_FOUND)
    target_include_directories(QtControls PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/KDEStyle
    )
endif()

target_link_libraries(QtControls PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    ${QTCONTROLS_EXTRA_LIBS}
)
