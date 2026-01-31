// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "TestUtils.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QTest>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QThread>
#include <QDirIterator>
#include <QDebug>
#include <QGuiApplication>

namespace Tests {

// ============================================================================
// TestEnvironment Implementation
// ============================================================================
TestEnvironment& TestEnvironment::getInstance() {
    static TestEnvironment instance;
    return instance;
}

bool TestEnvironment::init() {
    if (_initialized) {
        return true;
    }

    _tempDir = std::make_unique<QTemporaryDir>();
    if (!_tempDir->isValid()) {
        qWarning() << "Failed to create temporary directory";
        return false;
    }

    _initialized = true;
    return true;
}

void TestEnvironment::cleanup() {
    _tempDir.reset();
    _initialized = false;
}

QString TestEnvironment::getTempDir() const {
    if (_tempDir && _tempDir->isValid()) {
        return _tempDir->path();
    }
    return QString();
}

QString TestEnvironment::createTempFile(const QString& fileName, const QString& content) {
    QString fullPath = QDir(getTempDir()).filePath(fileName);
    QDir().mkpath(QFileInfo(fullPath).path());

    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream stream(&file);
    stream << content;
    file.close();

    return fullPath;
}

QString TestEnvironment::createTempDir(const QString& dirName) {
    QString fullPath = QDir(getTempDir()).filePath(dirName);
    if (QDir().mkpath(fullPath)) {
        return fullPath;
    }
    return QString();
}

QString TestEnvironment::getTestDataDir() const {
    // Look for test data in standard locations
    QStringList searchPaths = {
        QDir::current().filePath("testdata"),
        QDir::current().filePath("../testdata"),
        QDir::current().filePath("../../testdata"),
    };

    for (const QString& path : searchPaths) {
        if (QDir(path).exists()) {
            return path;
        }
    }

    return QString();
}

// ============================================================================
// FileUtils Implementation
// ============================================================================
namespace FileUtils {

bool createFile(const QString& path, const QString& content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream << content;
    file.close();
    return true;
}

QString readFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream stream(&file);
    QString content = stream.readAll();
    file.close();
    return content;
}

bool fileExists(const QString& path) {
    return QFile::exists(path);
}

bool dirExists(const QString& path) {
    return QDir(path).exists();
}

bool copyFile(const QString& src, const QString& dst) {
    return QFile::copy(src, dst);
}

bool compareFiles(const QString& file1, const QString& file2) {
    QString content1 = readFile(file1);
    QString content2 = readFile(file2);
    return content1 == content2;
}

qint64 getFileSize(const QString& path) {
    QFileInfo info(path);
    return info.size();
}

bool createDirectoryStructure(const QString& basePath, const QStringList& dirs) {
    for (const QString& dir : dirs) {
        QString fullPath = QDir(basePath).filePath(dir);
        if (!QDir().mkpath(fullPath)) {
            return false;
        }
    }
    return true;
}

bool removeDirectoryRecursively(const QString& path) {
    QDir dir(path);
    if (!dir.exists()) {
        return true;
    }

    // Remove all files and subdirectories
    QDirIterator it(path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString itemPath = it.next();
        QFileInfo info(itemPath);
        if (info.isDir()) {
            QDir(itemPath).rmdir(".");
        } else {
            QFile::remove(itemPath);
        }
    }

    return dir.rmdir(".");
}

} // namespace FileUtils

// ============================================================================
// WidgetTestUtils Implementation
// ============================================================================
namespace WidgetTestUtils {

bool isHeadlessEnvironment() {
    // Check for offscreen platform
    const QString platformName = QGuiApplication::platformName();
    if (platformName == "offscreen" || platformName == "minimal") {
        return true;
    }

    // Check for common headless environment indicators
    if (qEnvironmentVariableIsSet("DISPLAY")) {
        const QString display = qEnvironmentVariable("DISPLAY");
        if (display.isEmpty()) {
            return true;
        }
    }

    // Check for CI environment variables that indicate headless
    if (qEnvironmentVariableIsSet("CI") ||
        qEnvironmentVariableIsSet("GITHUB_ACTIONS") ||
        qEnvironmentVariableIsSet("GITLAB_CI") ||
        qEnvironmentVariableIsSet("JENKINS_URL")) {
        return true;
    }

    // Check for explicit headless flag
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        const QString platform = qEnvironmentVariable("QT_QPA_PLATFORM");
        if (platform == "offscreen" || platform == "minimal" || platform == "headless") {
            return true;
        }
    }

    return false;
}

bool waitForWidgetVisible(QWidget* widget, int timeoutMs) {
    if (!widget) return false;

    QElapsedTimer timer;
    timer.start();

    while (!widget->isVisible() && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }

    return widget->isVisible();
}

bool waitForWidgetHidden(QWidget* widget, int timeoutMs) {
    if (!widget) return false;

    QElapsedTimer timer;
    timer.start();

    while (widget->isVisible() && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }

    return !widget->isVisible();
}

bool waitForCondition(std::function<bool()> condition, int timeoutMs) {
    QElapsedTimer timer;
    timer.start();

    while (!condition() && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }

    return condition();
}

QWidget* findChildByName(QWidget* parent, const QString& name) {
    if (!parent) return nullptr;

    QList<QWidget*> children = parent->findChildren<QWidget*>();
    for (QWidget* child : children) {
        if (child->objectName() == name) {
            return child;
        }
    }

    return nullptr;
}

void simulateKeyPress(QWidget* widget, Qt::Key key, Qt::KeyboardModifiers modifiers) {
    if (!widget) return;

    QKeyEvent pressEvent(QEvent::KeyPress, key, modifiers);
    QKeyEvent releaseEvent(QEvent::KeyRelease, key, modifiers);

    QCoreApplication::sendEvent(widget, &pressEvent);
    QCoreApplication::sendEvent(widget, &releaseEvent);
}

void simulateMouseClick(QWidget* widget, Qt::MouseButton button) {
    if (!widget) return;

    QPoint center = widget->rect().center();

    QMouseEvent pressEvent(QEvent::MouseButtonPress, center, button, button, Qt::NoModifier);
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, center, button, button, Qt::NoModifier);

    QCoreApplication::sendEvent(widget, &pressEvent);
    QCoreApplication::sendEvent(widget, &releaseEvent);
}

} // namespace WidgetTestUtils

// ============================================================================
// Benchmark Implementation
// ============================================================================
Benchmark::Benchmark(const QString& name) : _name(name) {}

Benchmark::~Benchmark() {
    if (_running) {
        stop();
    }
    report();
}

void Benchmark::start() {
    _timer.start();
    _running = true;
}

void Benchmark::stop() {
    if (_running) {
        _elapsedMs = _timer.elapsed();
        _running = false;
    }
}

void Benchmark::report() {
    qDebug() << "Benchmark[" << _name << "]:" << _elapsedMs << "ms";
}

qint64 Benchmark::elapsedMs() const {
    return _elapsedMs;
}

// ============================================================================
// MockDialogProvider Implementation
// ============================================================================
void MockDialogProvider::setNextResult(int result) {
    _results.append(result);
}

int MockDialogProvider::getNextResult() {
    if (_resultIndex < _results.size()) {
        return _results[_resultIndex++];
    }
    return 0; // Default result
}

void MockDialogProvider::setNextFileName(const QString& fileName) {
    _fileNames.append(fileName);
}

QString MockDialogProvider::getNextFileName() {
    if (_fileNameIndex < _fileNames.size()) {
        return _fileNames[_fileNameIndex++];
    }
    return QString();
}

void MockDialogProvider::reset() {
    _results.clear();
    _fileNames.clear();
    _resultIndex = 0;
    _fileNameIndex = 0;
}

// ============================================================================
// TestData Implementation
// ============================================================================
namespace TestData {

QString randomString(int length) {
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    QString result;
    result.reserve(length);

    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        result.append(chars[index]);
    }

    return result;
}

QString randomText(int lines, int wordsPerLine) {
    const QStringList words = {
        "lorem", "ipsum", "dolor", "sit", "amet", "consectetur", "adipiscing", "elit",
        "sed", "do", "eiusmod", "tempor", "incididunt", "ut", "labore", "et",
        "dolore", "magna", "aliqua", "enim", "ad", "minim", "veniam", "quis"
    };

    QString result;
    QTextStream stream(&result);

    for (int i = 0; i < lines; ++i) {
        for (int j = 0; j < wordsPerLine; ++j) {
            int index = QRandomGenerator::global()->bounded(words.size());
            stream << words[index] << " ";
        }
        stream << "\n";
    }

    return result;
}

QStringList generateTestFilePaths(int count) {
    QStringList paths;
    for (int i = 0; i < count; ++i) {
        paths.append(QString("test_file_%1.txt").arg(i));
    }
    return paths;
}

QString sampleText() {
    return R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit.
Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris.
Duis aute irure dolor in reprehenderit in voluptate velit esse cillum.
Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia.)";
}

QString sampleCode() {
    return R"(#include <iostream>

int main() {
    // Print hello world
    std::cout << "Hello, World!" << std::endl;
    return 0;
}

class Example {
public:
    void doSomething() {
        // Implementation here
    }
};)";
}

QString sampleXml() {
    return R"(<?xml version="1.0" encoding="UTF-8"?>
<root>
    <item id="1">
        <name>First Item</name>
        <value>100</value>
    </item>
    <item id="2">
        <name>Second Item</name>
        <value>200</value>
    </item>
</root>)";
}

QString sampleJson() {
    return R"({
    "name": "Test Object",
    "version": "1.0",
    "items": [
        {"id": 1, "value": "first"},
        {"id": 2, "value": "second"}
    ],
    "enabled": true
})";
}

} // namespace TestData

} // namespace Tests
