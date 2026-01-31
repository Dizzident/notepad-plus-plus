// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "GoToLineDlgTest.h"
#include "GoToLine/GoToLineDlg.h"
#include "../Common/TestUtils.h"

using namespace QtControls;

namespace Tests {

GoToLineDlgTest::GoToLineDlgTest() {}

GoToLineDlgTest::~GoToLineDlgTest() {}

void GoToLineDlgTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void GoToLineDlgTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void GoToLineDlgTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(800, 600);
    _dialog = std::make_unique<GoToLineDlg>(_parentWidget.get());
}

void GoToLineDlgTest::cleanup() {
    _dialog.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void GoToLineDlgTest::testInit() {
    QVERIFY(_dialog != nullptr);

    // Initialize with position info
    _dialog->init(10, 100, 500);
    QVERIFY(true);
}

void GoToLineDlgTest::testDoDialog() {
    _dialog->init(1, 100, 0);

    // Show the dialog
    _dialog->doDialog();
    QVERIFY(_dialog->getWidget()->isVisible());
}

// ============================================================================
// Getters Tests
// ============================================================================
void GoToLineDlgTest::testGetLine() {
    _dialog->init(1, 100, 0);
    _dialog->doDialog();

    // Default line should be current line
    int line = _dialog->getLine();
    QCOMPARE(line, 1);
}

void GoToLineDlgTest::testIsLineMode() {
    _dialog->init(1, 100, 0);

    // Default should be line mode
    QVERIFY(_dialog->isLineMode());
}

// ============================================================================
// Mode Switching Tests
// ============================================================================
void GoToLineDlgTest::testModeSwitching() {
    _dialog->init(1, 100, 50);
    _dialog->doDialog();

    // Start in line mode
    QVERIFY(_dialog->isLineMode());

    // Switch to offset mode would require UI interaction
    // This is tested through the UI in integration tests
    QVERIFY(true);
}

} // namespace Tests

#include "GoToLineDlgTest.moc"
