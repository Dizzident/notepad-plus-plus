// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <QString>
#include <vector>

namespace Platform {
    class IProcess;
}

namespace Tests {

class ProcessTest : public QObject {
    Q_OBJECT

public:
    ProcessTest();
    ~ProcessTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    // Basic process execution
    void testRunAsync();
    void testRunSync();
    void testRunSyncWithOutput();

    // Shell execution
    void testShellOpen();
    void testShellOpenWith();

    // Process management
    void testWaitForProcess();
    void testTerminateProcess();
    void testIsProcessRunning();
    void testGetExitCode();

    // Process queries
    void testIsProcessRunningByPid();
    void testGetCurrentProcessId();
    void testGetCurrentProcessPath();

    // Elevated execution
    void testIsCurrentProcessElevated();

    // Utility functions
    void testQuoteArgument();
    void testBuildCommandLine();
    void testFindExecutable();

    // ProcessUtils
    void testExecute();
    void testOpenDocument();
    void testOpenUrl();
    void testRunDetached();

private:
    Platform::IProcess* _process = nullptr;

    // Helper to check if a command exists
    bool commandExists(const QString& command);
};

} // namespace Tests
