// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include <QtTest/QtTest>
#include <QApplication>
#include "TestUtils.h"

// Include QtControls test headers
#include "../QtControls/WindowTest.h"
#include "../QtControls/StaticDialogTest.h"
#include "../QtControls/TreeViewTest.h"
#include "../QtControls/ListViewTest.h"
#include "../QtControls/DockingManagerTest.h"
#include "../QtControls/RunDlgTest.h"
#include "../QtControls/AboutDlgTest.h"

// This file provides a main() function for QtControlsTests executable
// that runs all Qt controls tests

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

    // Run all QtControls tests
    result |= QTest::qExec(new Tests::WindowTest(), argc, argv);
    result |= QTest::qExec(new Tests::StaticDialogTest(), argc, argv);
    result |= QTest::qExec(new Tests::TreeViewTest(), argc, argv);
    result |= QTest::qExec(new Tests::ListViewTest(), argc, argv);
    result |= QTest::qExec(new Tests::DockingManagerTest(), argc, argv);
    result |= QTest::qExec(new Tests::RunDlgTest(), argc, argv);
    result |= QTest::qExec(new Tests::AboutDlgTest(), argc, argv);

    Tests::TestEnvironment::getInstance().cleanup();

    return result;
}

#endif // QTEST_CUSTOM_MAIN
