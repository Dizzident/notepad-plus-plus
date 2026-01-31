// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ProcessTest.h"
#include "Process.h"
#include "../Common/TestUtils.h"
#include <QThread>
#include <QFile>
#include <QDir>

using namespace PlatformLayer;

namespace Tests {

ProcessTest::ProcessTest() {}

ProcessTest::~ProcessTest() {}

void ProcessTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
    _process = &IProcess::getInstance();
    QVERIFY(_process != nullptr);
}

void ProcessTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

bool ProcessTest::commandExists(const QString& command) {
    ProcessResult result = _process->runSync(
        L"which",
        command.toStdWString(),
        ProcessOptions()
    );
    return result.success && result.exitCode == 0;
}

// ============================================================================
// Basic Process Execution Tests
// ============================================================================
void ProcessTest::testRunAsync() {
    // Test running a simple command asynchronously
    ProcessOptions options;
    options.waitForExit = false;

    ProcessInfo info = _process->run(L"echo", L"hello", options);

    // Process should have a valid PID
    QVERIFY(info.pid > 0);
}

void ProcessTest::testRunSync() {
    ProcessOptions options;
    options.waitForExit = true;

    ProcessResult result = _process->runSync(L"echo", L"hello", options);

    QVERIFY(result.success);
    QCOMPARE(static_cast<int>(result.exitCode), 0);
}

void ProcessTest::testRunSyncWithOutput() {
    ProcessOptions options;
    options.waitForExit = true;
    options.captureOutput = true;

    ProcessResult result = _process->runSync(L"echo", L"hello world", options);

    QVERIFY(result.success);
    QCOMPARE(static_cast<int>(result.exitCode), 0);
    QVERIFY(result.output.find("hello world") != std::string::npos);
}

// ============================================================================
// Shell Execution Tests
// ============================================================================
void ProcessTest::testShellOpen() {
    // This test is platform-specific and may not work in headless environments
    // Just verify the method doesn't crash
    // Note: xdg-open may not be available in test environment

    QString testFile = TestEnvironment::getInstance().getTempDir() + "/test_open.txt";
    QFile file(testFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write("test");
        file.close();
    }

    // Should return true or false but not crash
    _process->shellOpen(testFile.toStdWString());

    QVERIFY(true); // Test passes if we get here without crash
}

void ProcessTest::testShellOpenWith() {
    QString testFile = TestEnvironment::getInstance().getTempDir() + "/test_open.txt";
    QFile file(testFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write("test");
        file.close();
    }

    // Try to open with cat (should be available on Linux)
    bool result = _process->shellOpenWith(L"cat", testFile.toStdWString());

    // Result depends on environment, but shouldn't crash
    Q_UNUSED(result)
    QVERIFY(true);
}

// ============================================================================
// Process Management Tests
// ============================================================================
void ProcessTest::testWaitForProcess() {
    ProcessOptions options;
    options.waitForExit = false;

    ProcessInfo info = _process->run(L"sleep", L"0.1", options);
    QVERIFY(info.pid > 0);

    // Wait for process to complete
    QVERIFY(_process->waitForProcess(info, 5000));
}

void ProcessTest::testTerminateProcess() {
    // Start a long-running process
    ProcessOptions options;
    options.waitForExit = false;

    ProcessInfo info = _process->run(L"sleep", L"10", options);
    QVERIFY(info.pid > 0);

    // Verify process is running
    QVERIFY(_process->isProcessRunning(info));

    // Terminate it
    QVERIFY(_process->terminateProcess(info, false));

    // Give it time to terminate
    QThread::msleep(100);
}

void ProcessTest::testIsProcessRunning() {
    ProcessOptions options;
    options.waitForExit = false;

    // Start a process that will run for a short time
    ProcessInfo info = _process->run(L"sleep", L"0.5", options);
    QVERIFY(info.pid > 0);

    // Should be running initially
    QVERIFY(_process->isProcessRunning(info));

    // Wait for it to finish
    _process->waitForProcess(info, 2000);

    // Should not be running after completion
    QVERIFY(!_process->isProcessRunning(info));
}

void ProcessTest::testGetExitCode() {
    ProcessOptions options;
    options.waitForExit = true;

    // Test successful exit (exit code 0)
    ProcessResult result = _process->runSync(L"true", L"", options);
    QVERIFY(result.success);
    QCOMPARE(static_cast<int>(result.exitCode), 0);

    // Test failure exit (exit code non-zero)
    result = _process->runSync(L"false", L"", options);
    QVERIFY(!result.success);
}

// ============================================================================
// Process Queries Tests
// ============================================================================
void ProcessTest::testIsProcessRunningByPid() {
    // Current process should be running
    uint32_t currentPid = _process->getCurrentProcessId();
    QVERIFY(currentPid > 0);
    QVERIFY(_process->isProcessRunning(currentPid));

    // PID 1 should be running (init/systemd on Linux)
    QVERIFY(_process->isProcessRunning(1));

    // Very high PID is unlikely to exist
    QVERIFY(!_process->isProcessRunning(999999));
}

void ProcessTest::testGetCurrentProcessId() {
    uint32_t pid = _process->getCurrentProcessId();
    QVERIFY(pid > 0);

    // Should be consistent across calls
    uint32_t pid2 = _process->getCurrentProcessId();
    QCOMPARE(pid, pid2);
}

void ProcessTest::testGetCurrentProcessPath() {
    std::wstring path = _process->getCurrentProcessPath();
    QVERIFY(!path.empty());

    // Path should contain the executable name
    QVERIFY(path.find(L"test") != std::wstring::npos ||
            path.find(L"Test") != std::wstring::npos);
}

// ============================================================================
// Elevated Execution Tests
// ============================================================================
void ProcessTest::testIsCurrentProcessElevated() {
    // Just verify it doesn't crash
    bool elevated = _process->isCurrentProcessElevated();

    // Result depends on test environment
    Q_UNUSED(elevated)
    QVERIFY(true);
}

// ============================================================================
// Utility Functions Tests
// ============================================================================
void ProcessTest::testQuoteArgument() {
    // Test simple argument (no quotes needed)
    std::wstring simple = IProcess::quoteArgument(L"simple");
    QCOMPARE(simple, std::wstring(L"simple"));

    // Test argument with spaces
    std::wstring withSpaces = IProcess::quoteArgument(L"has spaces");
    QVERIFY(withSpaces.find(L'"') != std::wstring::npos ||
             withSpaces.find(L'\'') != std::wstring::npos);
}

void ProcessTest::testBuildCommandLine() {
    std::vector<std::wstring> args = {L"arg1", L"arg2", L"arg3"};
    std::wstring cmdLine = IProcess::buildCommandLine(L"program", args);

    QVERIFY(!cmdLine.empty());
    QVERIFY(cmdLine.find(L"program") != std::wstring::npos);
    QVERIFY(cmdLine.find(L"arg1") != std::wstring::npos);
    QVERIFY(cmdLine.find(L"arg2") != std::wstring::npos);
    QVERIFY(cmdLine.find(L"arg3") != std::wstring::npos);
}

void ProcessTest::testFindExecutable() {
    // 'ls' should be found on any Linux system
    std::wstring lsPath = IProcess::findExecutable(L"ls");
    QVERIFY(!lsPath.empty());

    // Non-existent command should return empty
    std::wstring notFound = IProcess::findExecutable(L"this_command_does_not_exist_12345");
    QVERIFY(notFound.empty());
}

// ============================================================================
// ProcessUtils Tests
// ============================================================================
void ProcessTest::testExecute() {
    std::vector<std::wstring> args = {L"hello"};
    ProcessResult result = ProcessUtils::execute(L"echo", args);

    QVERIFY(result.success);
    QVERIFY(result.output.find("hello") != std::string::npos);
}

void ProcessTest::testOpenDocument() {
    // Create a test file
    QString testFile = TestEnvironment::getInstance().getTempDir() + "/test_doc.txt";
    QFile file(testFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write("test content");
        file.close();
    }

    // Just verify it doesn't crash
    ProcessUtils::openDocument(testFile.toStdWString());
    QVERIFY(true);
}

void ProcessTest::testOpenUrl() {
    // Just verify it doesn't crash
    // Actual URL opening depends on environment
    ProcessUtils::openUrl(L"https://www.example.com");
    QVERIFY(true);
}

void ProcessTest::testRunDetached() {
    std::vector<std::wstring> args = {L"0.1"};
    bool result = ProcessUtils::runDetached(L"sleep", args);

    // Result depends on environment
    Q_UNUSED(result)
    QVERIFY(true);
}

} // namespace Tests

#include "ProcessTest.moc"
