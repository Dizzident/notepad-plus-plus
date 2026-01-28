// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <QMainWindow>
#include <memory>

namespace QtControls {
namespace DockingManager {
    class Manager;
}
}

namespace Tests {

class DockingManagerTest : public QObject {
    Q_OBJECT

public:
    DockingManagerTest();
    ~DockingManagerTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Initialization
    void testInit();
    void testDestroy();

    // Panel management
    void testAddPanel();
    void testRemovePanel();
    void testHasPanel();

    // Visibility
    void testShowPanel();
    void testHidePanel();
    void testTogglePanel();
    void testIsPanelVisible();

    // Panel properties
    void testSetPanelArea();
    void testGetPanelArea();
    void testSetPanelTitle();
    void testGetPanelTitle();
    void testGetPanelWidget();

    // Layout
    void testSaveLayout();
    void testRestoreLayout();
    void testSetTabbedDocking();

    // Batch operations
    void testShowAllPanels();
    void testHideAllPanels();
    void testGetPanelNames();
    void testGetVisiblePanels();

    // Features
    void testSetPanelFeatures();
    void testRaisePanel();
    void testGetPanelCount();

private:
    std::unique_ptr<QMainWindow> _mainWindow;
    std::unique_ptr<QtControls::DockingManager::Manager> _dockingManager;
};

} // namespace Tests
