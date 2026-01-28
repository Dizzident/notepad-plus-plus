// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <QWidget>
#include <memory>

namespace Platform {
    class FindReplaceDlg;
    class FindIncrementDlg;
}

namespace Tests {

class FindReplaceDlgTest : public QObject {
    Q_OBJECT

public:
    FindReplaceDlgTest();
    ~FindReplaceDlgTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Initialization
    void testInit();
    void testShowDialog();

    // Search text
    void testSetSearchText();
    void testGetSearchText();
    void testGetReplaceText();

    // Options
    void testGetCurrentOptions();
    void testSetOptions();

    // Search operations
    void testFindNext();
    void testFindPrevious();
    void testReplace();
    void testReplaceAll();
    void testCountMatches();

    // Mark operations
    void testMarkAll();
    void testClearMarks();

    // Status
    void testSetStatusMessage();

    // Incremental find
    void testIncrementalFind();

private:
    std::unique_ptr<QWidget> _parentWidget;
    std::unique_ptr<Platform::FindReplaceDlg> _findDlg;
};

} // namespace Tests
