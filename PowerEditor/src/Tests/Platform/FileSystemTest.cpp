// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "FileSystemTest.h"
#include "FileSystem.h"
#include "../Common/TestUtils.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QThread>

using namespace PlatformLayer;

namespace Tests {

FileSystemTest::FileSystemTest() {}

FileSystemTest::~FileSystemTest() {}

void FileSystemTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
    _fileSystem = &IFileSystem::getInstance();
    QVERIFY(_fileSystem != nullptr);
}

void FileSystemTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void FileSystemTest::init() {
    _tempDir = std::make_unique<QTemporaryDir>();
    QVERIFY(_tempDir->isValid());
    _tempPath = _tempDir->path();
}

void FileSystemTest::cleanup() {
    _tempDir.reset();
}

QString FileSystemTest::createTestFile(const QString& fileName, const QString& content) {
    QString fullPath = QDir(_tempPath).filePath(fileName);
    QDir().mkpath(QFileInfo(fullPath).path());

    QFile file(fullPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
        file.close();
    }
    return fullPath;
}

QString FileSystemTest::getTestPath(const QString& relativePath) {
    return QDir(_tempPath).filePath(relativePath);
}

// ============================================================================
// File Existence Tests
// ============================================================================
void FileSystemTest::testFileExists() {
    // Test non-existent file
    QString nonExistent = getTestPath("non_existent.txt");
    QVERIFY(!_fileSystem->fileExists(nonExistent.toStdWString()));

    // Create and test existing file
    QString existingFile = createTestFile("test.txt", "content");
    QVERIFY(_fileSystem->fileExists(existingFile.toStdWString()));

    // Test directory (should return false for fileExists)
    QString dirPath = getTestPath("testdir");
    QDir().mkpath(dirPath);
    QVERIFY(!_fileSystem->fileExists(dirPath.toStdWString()));
}

void FileSystemTest::testDirectoryExists() {
    // Test non-existent directory
    QString nonExistent = getTestPath("non_existent_dir");
    QVERIFY(!_fileSystem->directoryExists(nonExistent.toStdWString()));

    // Create and test existing directory
    QString dirPath = getTestPath("testdir");
    QDir().mkpath(dirPath);
    QVERIFY(_fileSystem->directoryExists(dirPath.toStdWString()));

    // Test file (should return false for directoryExists)
    QString filePath = createTestFile("test.txt", "content");
    QVERIFY(!_fileSystem->directoryExists(filePath.toStdWString()));
}

void FileSystemTest::testPathExists() {
    // Test non-existent path
    QString nonExistent = getTestPath("non_existent");
    QVERIFY(!_fileSystem->pathExists(nonExistent.toStdWString()));

    // Test existing file
    QString filePath = createTestFile("test.txt", "content");
    QVERIFY(_fileSystem->pathExists(filePath.toStdWString()));

    // Test existing directory
    QString dirPath = getTestPath("testdir");
    QDir().mkpath(dirPath);
    QVERIFY(_fileSystem->pathExists(dirPath.toStdWString()));
}

// ============================================================================
// File Operations Tests
// ============================================================================
void FileSystemTest::testCreateDirectory() {
    QString dirPath = getTestPath("newdir");
    QVERIFY(!QDir(dirPath).exists());

    QVERIFY(_fileSystem->createDirectory(dirPath.toStdWString()));
    QVERIFY(QDir(dirPath).exists());

    // Creating same directory again should succeed or fail gracefully
    _fileSystem->createDirectory(dirPath.toStdWString());
}

void FileSystemTest::testCreateDirectoryRecursive() {
    QString nestedPath = getTestPath("a/b/c/d");
    QVERIFY(!QDir(nestedPath).exists());

    QVERIFY(_fileSystem->createDirectoryRecursive(nestedPath.toStdWString()));
    QVERIFY(QDir(nestedPath).exists());
}

void FileSystemTest::testDeleteFile() {
    QString filePath = createTestFile("delete_me.txt", "content");
    QVERIFY(QFile::exists(filePath));

    QVERIFY(_fileSystem->deleteFile(filePath.toStdWString()));
    QVERIFY(!QFile::exists(filePath));

    // Deleting non-existent file should return false
    QVERIFY(!_fileSystem->deleteFile(filePath.toStdWString()));
}

void FileSystemTest::testCopyFile() {
    QString srcPath = createTestFile("source.txt", "Hello, World!");
    QString dstPath = getTestPath("destination.txt");

    QVERIFY(!QFile::exists(dstPath));
    QVERIFY(_fileSystem->copyFile(srcPath.toStdWString(), dstPath.toStdWString()));
    QVERIFY(QFile::exists(dstPath));

    // Verify content
    QString content = FileUtils::readFile(dstPath);
    QCOMPARE(content, QString("Hello, World!"));

    // Test overwrite
    createTestFile("source.txt", "New Content");
    QVERIFY(_fileSystem->copyFile(srcPath.toStdWString(), dstPath.toStdWString(), true));
    content = FileUtils::readFile(dstPath);
    QCOMPARE(content, QString("New Content"));
}

void FileSystemTest::testMoveFile() {
    QString srcPath = createTestFile("move_me.txt", "Move this content");
    QString dstPath = getTestPath("moved.txt");

    QVERIFY(_fileSystem->moveFile(srcPath.toStdWString(), dstPath.toStdWString()));
    QVERIFY(!QFile::exists(srcPath));
    QVERIFY(QFile::exists(dstPath));

    QString content = FileUtils::readFile(dstPath);
    QCOMPARE(content, QString("Move this content"));
}

void FileSystemTest::testReplaceFile() {
    QString replaced = createTestFile("replaced.txt", "old content");
    QString replacement = createTestFile("replacement.txt", "new content");
    QString backup = getTestPath("backup.txt");

    QVERIFY(_fileSystem->replaceFile(replaced.toStdWString(), replacement.toStdWString(), backup.toStdWString()));

    // Original should now have new content
    QString content = FileUtils::readFile(replaced);
    QCOMPARE(content, QString("new content"));

    // Backup should have old content
    QString backupContent = FileUtils::readFile(backup);
    QCOMPARE(backupContent, QString("old content"));
}

// ============================================================================
// Path Operations Tests
// ============================================================================
void FileSystemTest::testAbsolutePath() {
    // Test relative path conversion
    QString relativePath = "relative/path/file.txt";
    std::wstring absolute = _fileSystem->getFullPathName(relativePath.toStdWString());

    QVERIFY(!absolute.empty());
    QVERIFY(absolute[0] == L'/' || (absolute.length() > 1 && absolute[1] == L':'));
}

void FileSystemTest::testGetFullPathName() {
    QString testPath = getTestPath("test.txt");
    createTestFile("test.txt", "content");

    std::wstring fullPath = _fileSystem->getFullPathName(testPath.toStdWString());
    QVERIFY(!fullPath.empty());
    QVERIFY(fullPath.find(L"test.txt") != std::wstring::npos);
}

void FileSystemTest::testGetTempPath() {
    std::wstring tempPath = _fileSystem->getTempPath();
    QVERIFY(!tempPath.empty());
    QVERIFY(QDir(QString::fromStdWString(tempPath)).exists());
}

void FileSystemTest::testGetCurrentDirectory() {
    std::wstring currentDir = _fileSystem->getCurrentDirectory();
    QVERIFY(!currentDir.empty());
    QVERIFY(QDir(QString::fromStdWString(currentDir)).exists());
}

void FileSystemTest::testSetCurrentDirectory() {
    QString originalDir = QString::fromStdWString(_fileSystem->getCurrentDirectory());
    QString newDir = getTestPath("newcwd");
    QDir().mkpath(newDir);

    QVERIFY(_fileSystem->setCurrentDirectory(newDir.toStdWString()));
    QCOMPARE(QString::fromStdWString(_fileSystem->getCurrentDirectory()), newDir);

    // Restore original directory
    _fileSystem->setCurrentDirectory(originalDir.toStdWString());
}

// ============================================================================
// File Attributes Tests
// ============================================================================
void FileSystemTest::testFileAttributes() {
    QString filePath = createTestFile("attr_test.txt", "content");

    FileAttributes attrs;
    QVERIFY(_fileSystem->getFileAttributes(filePath.toStdWString(), attrs));
    QVERIFY(attrs.exists);
    QCOMPARE(attrs.size, static_cast<uint64_t>(7)); // "content" = 7 bytes
}

void FileSystemTest::testGetFileTime() {
    QString filePath = createTestFile("time_test.txt", "content");
    QThread::msleep(100); // Ensure some time passes

    FileTime creation, lastAccess, lastWrite;
    QVERIFY(_fileSystem->getFileTime(filePath.toStdWString(), creation, lastAccess, lastWrite));

    // Times should be non-zero for existing file
    QVERIFY(creation.seconds > 0);
    QVERIFY(lastWrite.seconds > 0);
}

void FileSystemTest::testSetFileTime() {
    QString filePath = createTestFile("time_set_test.txt", "content");

    FileTime newTime;
    newTime.seconds = 1609459200; // 2021-01-01 00:00:00 UTC

    QVERIFY(_fileSystem->setFileTime(filePath.toStdWString(), nullptr, nullptr, &newTime));

    FileTime creation, lastAccess, lastWrite;
    QVERIFY(_fileSystem->getFileTime(filePath.toStdWString(), creation, lastAccess, lastWrite));

    // Allow some tolerance for filesystem precision
    QVERIFY(std::abs(static_cast<int64_t>(lastWrite.seconds - newTime.seconds)) < 2);
}

void FileSystemTest::testCompareFileTime() {
    FileTime t1{1000, 0};
    FileTime t2{2000, 0};
    FileTime t3{1000, 500};

    QCOMPARE(IFileSystem::compareFileTime(t1, t2), -1);
    QCOMPARE(IFileSystem::compareFileTime(t2, t1), 1);
    QCOMPARE(IFileSystem::compareFileTime(t1, t3), -1);

    FileTime t4{1000, 0};
    QCOMPARE(IFileSystem::compareFileTime(t1, t4), 0);
}

// ============================================================================
// Directory Enumeration Tests
// ============================================================================
void FileSystemTest::testEnumerateFiles() {
    // Create test files
    createTestFile("enum/file1.txt", "content1");
    createTestFile("enum/file2.txt", "content2");
    createTestFile("enum/subdir/file3.txt", "content3");

    QString dirPath = getTestPath("enum");
    std::vector<FileInfo> files;

    QVERIFY(_fileSystem->enumerateFiles(dirPath.toStdWString(), L"*.txt", files));
    QCOMPARE(static_cast<int>(files.size()), 2);

    // Check that we found the expected files
    bool foundFile1 = false, foundFile2 = false;
    for (const auto& file : files) {
        if (file.name == L"file1.txt") foundFile1 = true;
        if (file.name == L"file2.txt") foundFile2 = true;
    }
    QVERIFY(foundFile1);
    QVERIFY(foundFile2);
}

void FileSystemTest::testEnumerateFilesRecursive() {
    createTestFile("rec/file1.txt", "content1");
    createTestFile("rec/subdir/file2.txt", "content2");
    createTestFile("rec/subdir/deeper/file3.txt", "content3");

    QString dirPath = getTestPath("rec");
    std::vector<FileInfo> files;

    QVERIFY(_fileSystem->enumerateFilesRecursive(dirPath.toStdWString(), L"*.txt", files));
    QCOMPARE(static_cast<int>(files.size()), 3);
}

// ============================================================================
// Path Manipulation Tests
// ============================================================================
void FileSystemTest::testPathAppend() {
    std::wstring result = IFileSystem::pathAppend(L"/home/user", L"documents");
    QCOMPARE(result, std::wstring(L"/home/user/documents"));

    result = IFileSystem::pathAppend(L"/home/user/", L"documents");
    QCOMPARE(result, std::wstring(L"/home/user/documents"));
}

void FileSystemTest::testPathRemoveFileSpec() {
    std::wstring result = IFileSystem::pathRemoveFileSpec(L"/home/user/file.txt");
    QCOMPARE(result, std::wstring(L"/home/user"));

    result = IFileSystem::pathRemoveFileSpec(L"/home/user/dir/");
    QCOMPARE(result, std::wstring(L"/home/user/dir"));
}

void FileSystemTest::testGetFileName() {
    std::wstring result = IFileSystem::getFileName(L"/home/user/file.txt");
    QCOMPARE(result, std::wstring(L"file.txt"));

    result = IFileSystem::getFileName(L"file.txt");
    QCOMPARE(result, std::wstring(L"file.txt"));
}

void FileSystemTest::testGetDirectoryName() {
    std::wstring result = IFileSystem::getDirectoryName(L"/home/user/file.txt");
    QCOMPARE(result, std::wstring(L"/home/user"));
}

void FileSystemTest::testGetExtension() {
    std::wstring result = IFileSystem::getExtension(L"/home/user/file.txt");
    QCOMPARE(result, std::wstring(L".txt"));

    result = IFileSystem::getExtension(L"/home/user/file");
    QCOMPARE(result, std::wstring());
}

void FileSystemTest::testChangeExtension() {
    std::wstring result = IFileSystem::changeExtension(L"/home/user/file.txt", L".cpp");
    QCOMPARE(result, std::wstring(L"/home/user/file.cpp"));
}

void FileSystemTest::testIsRelativePath() {
    QVERIFY(IFileSystem::isRelativePath(L"relative/path"));
    QVERIFY(!IFileSystem::isRelativePath(L"/absolute/path"));
}

void FileSystemTest::testIsAbsolutePath() {
    QVERIFY(IFileSystem::isAbsolutePath(L"/absolute/path"));
    QVERIFY(!IFileSystem::isAbsolutePath(L"relative/path"));
}

// ============================================================================
// Special Folders Tests
// ============================================================================
void FileSystemTest::testGetUserConfigDir() {
    std::wstring configDir = _fileSystem->getUserConfigDir();
    QVERIFY(!configDir.empty());
    QVERIFY(QDir(QString::fromStdWString(configDir)).exists() ||
            QDir().mkpath(QString::fromStdWString(configDir)));
}

void FileSystemTest::testGetUserDataDir() {
    std::wstring dataDir = _fileSystem->getUserDataDir();
    QVERIFY(!dataDir.empty());
}

void FileSystemTest::testGetUserCacheDir() {
    std::wstring cacheDir = _fileSystem->getUserCacheDir();
    QVERIFY(!cacheDir.empty());
}

void FileSystemTest::testGetDocumentsDir() {
    std::wstring docsDir = _fileSystem->getDocumentsDir();
    QVERIFY(!docsDir.empty());
}

// ============================================================================
// Disk Operations Tests
// ============================================================================
void FileSystemTest::testGetDiskFreeSpace() {
    QString testPath = getTestPath(".");
    uint64_t freeBytes = 0;

    QVERIFY(_fileSystem->getDiskFreeSpace(testPath.toStdWString(), freeBytes));
    QVERIFY(freeBytes > 0);
}

// ============================================================================
// File I/O Class Tests
// ============================================================================
void FileSystemTest::testFileOpen() {
    QString filePath = createTestFile("io_test.txt", "content");

    File file;
    QVERIFY(!file.isOpen());

    QVERIFY(file.open(filePath.toStdWString(), FileMode::Read));
    QVERIFY(file.isOpen());

    file.close();
    QVERIFY(!file.isOpen());
}

void FileSystemTest::testFileReadWrite() {
    QString filePath = getTestPath("read_write.txt");

    // Write
    {
        File file(filePath.toStdWString(), FileMode::Write);
        QVERIFY(file.isOpen());

        std::string data = "Hello, World!";
        QCOMPARE(static_cast<int>(file.write(data.data(), data.size())), static_cast<int>(data.size()));
    }

    // Read
    {
        File file(filePath.toStdWString(), FileMode::Read);
        QVERIFY(file.isOpen());

        char buffer[100] = {};
        size_t bytesRead = file.read(buffer, sizeof(buffer));
        QCOMPARE(static_cast<int>(bytesRead), 13);
        QCOMPARE(std::string(buffer, bytesRead), std::string("Hello, World!"));
    }
}

void FileSystemTest::testFileSeek() {
    QString filePath = createTestFile("seek_test.txt", "ABCDEFGHIJ");

    File file(filePath.toStdWString(), FileMode::Read);
    QVERIFY(file.isOpen());

    // Seek to position 5
    QCOMPARE(file.seek(5, 0), static_cast<int64_t>(5));

    char buffer[10] = {};
    file.read(buffer, 5);
    QCOMPARE(std::string(buffer), std::string("FGHIJ"));
}

void FileSystemTest::testFileFlush() {
    QString filePath = getTestPath("flush_test.txt");

    File file(filePath.toStdWString(), FileMode::Write);
    QVERIFY(file.isOpen());

    file.writeString("Test content");
    QVERIFY(file.flush());
}

// ============================================================================
// Utility Functions Tests
// ============================================================================
void FileSystemTest::testReadFileContent() {
    QString filePath = createTestFile("read_util.txt", "File content for utility test");

    bool failed = false;
    std::string content = FileSystemUtils::readFileContent(filePath.toStdWString(), &failed);

    QVERIFY(!failed);
    QCOMPARE(QString::fromStdString(content), QString("File content for utility test"));
}

void FileSystemTest::testWriteFileContent() {
    QString filePath = getTestPath("write_util.txt");

    QVERIFY(FileSystemUtils::writeFileContent(filePath.toStdWString(), "Written content"));

    QString readContent = FileUtils::readFile(filePath);
    QCOMPARE(readContent, QString("Written content"));
}

void FileSystemTest::testEnsureDirectoryExists() {
    QString dirPath = getTestPath("ensure/dir/path");
    QVERIFY(!QDir(dirPath).exists());

    QVERIFY(FileSystemUtils::ensureDirectoryExists(dirPath.toStdWString()));
    QVERIFY(QDir(dirPath).exists());
}

void FileSystemTest::testGetTempFilePath() {
    std::wstring tempPath = FileSystemUtils::getTempFilePath(L"npp_test");
    QVERIFY(!tempPath.empty());
    QVERIFY(tempPath.find(L"npp_test") != std::wstring::npos);
}

} // namespace Tests

// This would normally be in a separate main file, but included here for completeness
// QTEST_MAIN(Tests::FileSystemTest)
#include "FileSystemTest.moc"
