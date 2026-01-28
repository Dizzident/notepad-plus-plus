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
#include <memory>

namespace Platform {
    class IFileSystem;
}

namespace Tests {

class FileSystemTest : public QObject {
    Q_OBJECT

public:
    FileSystemTest();
    ~FileSystemTest();

private Q_SLOTS:
    // Test case setup/cleanup
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // File existence checks
    void testFileExists();
    void testDirectoryExists();
    void testPathExists();

    // File operations
    void testCreateDirectory();
    void testCreateDirectoryRecursive();
    void testDeleteFile();
    void testCopyFile();
    void testMoveFile();
    void testReplaceFile();

    // Path operations
    void testAbsolutePath();
    void testGetFullPathName();
    void testGetTempPath();
    void testGetCurrentDirectory();
    void testSetCurrentDirectory();

    // File attributes
    void testFileAttributes();
    void testGetFileTime();
    void testSetFileTime();
    void testCompareFileTime();

    // Directory enumeration
    void testEnumerateFiles();
    void testEnumerateFilesRecursive();

    // Path manipulation (static methods)
    void testPathAppend();
    void testPathRemoveFileSpec();
    void testGetFileName();
    void testGetDirectoryName();
    void testGetExtension();
    void testChangeExtension();
    void testIsRelativePath();
    void testIsAbsolutePath();

    // Special folders
    void testGetUserConfigDir();
    void testGetUserDataDir();
    void testGetUserCacheDir();
    void testGetDocumentsDir();

    // Disk operations
    void testGetDiskFreeSpace();

    // File I/O class
    void testFileOpen();
    void testFileReadWrite();
    void testFileSeek();
    void testFileFlush();

    // Utility functions
    void testReadFileContent();
    void testWriteFileContent();
    void testEnsureDirectoryExists();
    void testGetTempFilePath();

private:
    Platform::IFileSystem* _fileSystem = nullptr;
    std::unique_ptr<QTemporaryDir> _tempDir;
    QString _tempPath;

    // Helper methods
    QString createTestFile(const QString& fileName, const QString& content);
    QString getTestPath(const QString& relativePath);
};

} // namespace Tests
