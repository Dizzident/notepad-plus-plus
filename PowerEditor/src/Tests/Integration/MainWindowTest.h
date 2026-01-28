// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <memory>

namespace QtControls {
namespace MainWindow {
    class MainWindow;
}
}

class Notepad_plus;

namespace Tests {

class MainWindowTest : public QObject {
    Q_OBJECT

public:
    MainWindowTest();
    ~MainWindowTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Initialization
    void testInit();
    void testDestroy();

    // Window operations
    void testDisplay();
    void testReSizeTo();

    // Menu operations
    void testInitMenuBar();
    void testUpdateMenuState();

    // Toolbar operations
    void testInitToolBar();
    void testUpdateToolBarState();

    // Status bar operations
    void testInitStatusBar();
    void testUpdateStatusBar();

    // Panel management
    void testShowPanel();
    void testIsPanelVisible();

    // Document management
    void testAddTab();
    void testCloseTab();
    void testSwitchTab();

    // Window state
    void testSaveWindowState();
    void testRestoreWindowState();

    // View modes
    void testToggleFullScreen();
    void testTogglePostItMode();

    // Always on top
    void testSetAlwaysOnTop();
    void testIsAlwaysOnTop();

private:
    std::unique_ptr<QtControls::MainWindow::MainWindow> _mainWindow;
    std::unique_ptr<Notepad_plus> _nppCore;
};

} // namespace Tests
