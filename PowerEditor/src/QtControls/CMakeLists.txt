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
)

set(QTCONTROLS_HEADERS
    Window.h
    StaticDialog/StaticDialog.h
    ToolBar/ToolBar.h
    StatusBar/StatusBar.h
    TabBar/TabBar.h
)

# Create static library
add_library(QtControls STATIC ${QTCONTROLS_SOURCES} ${QTCONTROLS_HEADERS})

target_include_directories(QtControls PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/StaticDialog
    ${CMAKE_CURRENT_SOURCE_DIR}/ToolBar
    ${CMAKE_CURRENT_SOURCE_DIR}/StatusBar
    ${CMAKE_CURRENT_SOURCE_DIR}/TabBar
)

target_link_libraries(QtControls PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)
