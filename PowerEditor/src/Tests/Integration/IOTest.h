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

class IOTest : public QObject {
    Q_OBJECT

public:
    IOTest();
    ~IOTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // File open operations
    void testOpenExistingFile();
    void testOpenNonExistentFile();
    void testOpenMultipleFiles();
    void testOpenWithEncoding();

    // File save operations
    void testSaveNewFile();
    void testSaveExistingFile();
    void testSaveAs();
    void testSaveAll();

    // File close operations
    void testCloseFile();
    void testCloseAllFiles();
    void testCloseWithUnsavedChanges();

    // Recent files
    void testAddToRecentFiles();
    void testGetRecentFiles();
    void testClearRecentFiles();

    // Session management
    void testSaveSession();
    void testLoadSession();
    void testRestoreSession();

    // Large file handling
    void testOpenLargeFile();
    void testSaveLargeFile();

    // Special file types
    void testOpenBinaryFile();
    void testOpenSymlink();
    void testOpenNetworkFile();

private:
    QString createTestFile(const QString& fileName, const QString& content);
    QString getTestPath(const QString& relativePath);
};

} // namespace Tests
