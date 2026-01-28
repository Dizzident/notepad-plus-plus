// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QString>
#include <QSignalSpy>
#include <memory>

namespace Platform {
    class IFileWatcher;
}

namespace Tests {

class FileWatcherTest : public QObject {
    Q_OBJECT

public:
    FileWatcherTest();
    ~FileWatcherTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Directory watching
    void testWatchDirectory();
    void testUnwatchDirectory();
    void testWatchDirectoryWithOptions();

    // File watching
    void testWatchFile();
    void testUnwatchFile();

    // Event processing
    void testProcessEvents();
    void testHasPendingEvents();

    // Watch utilities
    void testSetWatchEnabled();
    void testUpdateWatchOptions();
    void testIsWatchValid();
    void testGetWatchPath();

    // Utility functions
    void testChangeTypeToString();
    void testIsContentModified();

    // DirectoryWatcher class
    void testDirectoryWatcher();

    // FileWatcher class
    void testFileWatcherClass();

    // Integration test
    void testFileChangeDetection();

private:
    Platform::IFileWatcher* _watcher = nullptr;
    std::unique_ptr<QTemporaryDir> _tempDir;
    QString _tempPath;

    // Helper methods
    QString createTestFile(const QString& fileName, const QString& content);
    QString getTestPath(const QString& relativePath);
    void waitForEvents(int timeoutMs = 500);
};

} // namespace Tests
