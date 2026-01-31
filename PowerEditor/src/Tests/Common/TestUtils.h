// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <QString>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QTextStream>
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace Tests {

// ============================================================================
// Test Environment Setup
// ============================================================================
class TestEnvironment {
public:
    static TestEnvironment& getInstance();

    // Initialize test environment
    bool init();

    // Cleanup test environment
    void cleanup();

    // Get temporary test directory
    QString getTempDir() const;

    // Create a temporary file with content
    QString createTempFile(const QString& fileName, const QString& content);

    // Create a temporary directory
    QString createTempDir(const QString& dirName);

    // Get test data directory
    QString getTestDataDir() const;

private:
    TestEnvironment() = default;
    ~TestEnvironment() = default;

    std::unique_ptr<QTemporaryDir> _tempDir;
    bool _initialized = false;
};

// ============================================================================
// Test File Utilities
// ============================================================================
namespace FileUtils {

    // Create a file with specified content
    bool createFile(const QString& path, const QString& content);

    // Read file content
    QString readFile(const QString& path);

    // Check if file exists
    bool fileExists(const QString& path);

    // Check if directory exists
    bool dirExists(const QString& path);

    // Copy file
    bool copyFile(const QString& src, const QString& dst);

    // Compare two files
    bool compareFiles(const QString& file1, const QString& file2);

    // Get file size
    qint64 getFileSize(const QString& path);

    // Create directory structure
    bool createDirectoryStructure(const QString& basePath, const QStringList& dirs);

    // Clean up directory recursively
    bool removeDirectoryRecursively(const QString& path);

} // namespace FileUtils

// ============================================================================
// Qt Widget Test Helpers
// ============================================================================
namespace WidgetTestUtils {

    // Check if running in headless/offscreen environment
    bool isHeadlessEnvironment();

    // Wait for widget to be visible
    bool waitForWidgetVisible(QWidget* widget, int timeoutMs = 5000);

    // Wait for widget to be hidden
    bool waitForWidgetHidden(QWidget* widget, int timeoutMs = 5000);

    // Process events until condition is met or timeout
    bool waitForCondition(std::function<bool()> condition, int timeoutMs = 5000);

    // Find child widget by name
    QWidget* findChildByName(QWidget* parent, const QString& name);

    // Simulate key press
    void simulateKeyPress(QWidget* widget, Qt::Key key, Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    // Simulate mouse click
    void simulateMouseClick(QWidget* widget, Qt::MouseButton button = Qt::LeftButton);

    // Get all child widgets of specific type
    template<typename T>
    QList<T*> findChildrenOfType(QWidget* parent) {
        if (!parent) return QList<T*>();
        return parent->findChildren<T*>();
    }

} // namespace WidgetTestUtils

// ============================================================================
// Test Macros and Helpers
// ============================================================================

// Compare QString with std::wstring
#define QCOMPARE_WSTRING(actual, expected) \
    QCOMPARE(QString::fromStdWString(actual), QString::fromStdWString(expected))

// Verify file exists
#define VERIFY_FILE_EXISTS(path) \
    QVERIFY2(QFile::exists(path), qPrintable(QString("File does not exist: %1").arg(path)))

// Verify directory exists
#define VERIFY_DIR_EXISTS(path) \
    QVERIFY2(QDir(path).exists(), qPrintable(QString("Directory does not exist: %1").arg(path)))

// Wait for signal with timeout
#define WAIT_FOR_SIGNAL(signal, timeout) \
    QTest::qWaitFor([&]() { return signal; }, timeout)

// ============================================================================
// Benchmark Helpers
// ============================================================================
class Benchmark {
public:
    explicit Benchmark(const QString& name);
    ~Benchmark();

    void start();
    void stop();
    void report();

    qint64 elapsedMs() const;

private:
    QString _name;
    QElapsedTimer _timer;
    qint64 _elapsedMs = 0;
    bool _running = false;
};

// ============================================================================
// Mock Objects for Testing
// ============================================================================

// Mock dialog result provider
class MockDialogProvider {
public:
    void setNextResult(int result);
    int getNextResult();

    void setNextFileName(const QString& fileName);
    QString getNextFileName();

    void reset();

private:
    QList<int> _results;
    QList<QString> _fileNames;
    int _resultIndex = 0;
    int _fileNameIndex = 0;
};

// ============================================================================
// Test Data Generators
// ============================================================================
namespace TestData {

    // Generate random string
    QString randomString(int length);

    // Generate random text content
    QString randomText(int lines, int wordsPerLine);

    // Generate test file paths
    QStringList generateTestFilePaths(int count);

    // Sample text content for testing
    QString sampleText();
    QString sampleCode();
    QString sampleXml();
    QString sampleJson();

} // namespace TestData

} // namespace Tests

// ============================================================================
// QTest Extensions
// ============================================================================

// Custom comparison for std::wstring
inline bool operator==(const std::wstring& lhs, const QString& rhs) {
    return QString::fromStdWString(lhs) == rhs;
}

inline bool operator==(const QString& lhs, const std::wstring& rhs) {
    return lhs == QString::fromStdWString(rhs);
}
