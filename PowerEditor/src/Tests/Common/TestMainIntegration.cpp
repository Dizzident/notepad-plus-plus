// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include <QtTest/QtTest>
#include <QApplication>
#include "TestUtils.h"

// Include Integration test headers
#include "../Integration/IOTest.h"
#include "../Integration/CommandTest.h"

// This file provides a main() function for IntegrationTests executable
// that runs integration tests
// Note: MainWindowTest and BufferTest are excluded due to heavy dependencies

#ifndef QTEST_CUSTOM_MAIN

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Initialize test environment
    if (!Tests::TestEnvironment::getInstance().init()) {
        qWarning() << "Failed to initialize test environment";
        return 1;
    }

    int result = 0;

    // Run integration tests
    result |= QTest::qExec(new Tests::IOTest(), argc, argv);
    result |= QTest::qExec(new Tests::CommandTest(), argc, argv);

    Tests::TestEnvironment::getInstance().cleanup();

    return result;
}

#endif // QTEST_CUSTOM_MAIN
