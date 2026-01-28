// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <memory>

namespace Tests {

class CommandTest : public QObject {
    Q_OBJECT

public:
    CommandTest();
    ~CommandTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // File commands
    void testNewFile();
    void testOpenFile();
    void testSaveFile();
    void testCloseFile();

    // Edit commands
    void testUndo();
    void testRedo();
    void testCut();
    void testCopy();
    void testPaste();
    void testDelete();
    void testSelectAll();

    // Search commands
    void testFind();
    void testReplace();
    void testGoToLine();

    // View commands
    void testZoomIn();
    void testZoomOut();
    void testZoomReset();
    void testToggleWordWrap();
    void testToggleLineNumbers();

    // Macro commands
    void testStartRecording();
    void testStopRecording();
    void testPlayback();

    // Run commands
    void testRunCommand();

    // Window commands
    void testNewWindow();
    void testCloseWindow();
    void testSplitView();

private:
    // Helper to execute a command
    bool executeCommand(int commandId);
};

} // namespace Tests
