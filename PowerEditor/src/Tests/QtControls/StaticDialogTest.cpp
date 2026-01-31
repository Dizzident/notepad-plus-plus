// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "StaticDialogTest.h"
#include "QtControls/StaticDialog/StaticDialog.h"
#include "../Common/TestUtils.h"

using namespace QtControls;

namespace Tests {

StaticDialogTest::StaticDialogTest() {}

StaticDialogTest::~StaticDialogTest() {}

void StaticDialogTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void StaticDialogTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void StaticDialogTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(800, 600);
}

void StaticDialogTest::cleanup() {
    _dialog.reset();
    _parentWidget.reset();
}

// ============================================================================
// Creation and Lifecycle Tests
// ============================================================================
void StaticDialogTest::testCreate() {
    _dialog = std::make_unique<StaticDialog>();
    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");
    QVERIFY(_dialog->isCreated());
}

void StaticDialogTest::testIsCreated() {
    _dialog = std::make_unique<StaticDialog>();
    QVERIFY(!_dialog->isCreated());

    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");
    QVERIFY(_dialog->isCreated());
}

void StaticDialogTest::testDestroy() {
    _dialog = std::make_unique<StaticDialog>();
    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");
    QVERIFY(_dialog->isCreated());

    _dialog->destroy();
    QVERIFY(!_dialog->isCreated());
}

// ============================================================================
// Display Tests
// ============================================================================
void StaticDialogTest::testDisplay() {
    if (Tests::WidgetTestUtils::isHeadlessEnvironment()) {
        QSKIP("Skipping visibility test in headless environment");
    }

    _dialog = std::make_unique<StaticDialog>();
    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");

    _dialog->display(true);
    QVERIFY(_dialog->getWidget()->isVisible());

    _dialog->display(false);
    QVERIFY(!_dialog->getWidget()->isVisible());
}

void StaticDialogTest::testGoToCenter() {
    _dialog = std::make_unique<StaticDialog>();
    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");

    // Should not crash
    _dialog->goToCenter();
    QVERIFY(true);
}

// ============================================================================
// Checkbox Tests
// ============================================================================
void StaticDialogTest::testIsCheckedOrNot() {
    _dialog = std::make_unique<StaticDialog>();
    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");

    // Test with non-existent checkbox (should return false)
    QVERIFY(!_dialog->isCheckedOrNot("nonExistentCheckBox"));
}

void StaticDialogTest::testSetChecked() {
    _dialog = std::make_unique<StaticDialog>();
    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");

    // Test with non-existent checkbox (should not crash)
    _dialog->setChecked("nonExistentCheckBox", true);
    QVERIFY(true);
}

// ============================================================================
// Position and Sizing Tests
// ============================================================================
void StaticDialogTest::testGetMappedChildRect() {
    _dialog = std::make_unique<StaticDialog>();
    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");

    QRect rect;
    // Test with non-existent child (should return empty rect or not crash)
    _dialog->getMappedChildRect("nonExistentChild", rect);
    QVERIFY(true);
}

void StaticDialogTest::testRedrawDlgItem() {
    _dialog = std::make_unique<StaticDialog>();
    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");

    // Should not crash
    _dialog->redrawDlgItem("nonExistentItem");
    QVERIFY(true);
}

void StaticDialogTest::testGetViewablePositionRect() {
    _dialog = std::make_unique<StaticDialog>();
    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");

    QRect testRect(0, 0, 100, 100);
    QRect result = _dialog->getViewablePositionRect(testRect);

    // Result should be valid
    QVERIFY(result.width() > 0);
    QVERIFY(result.height() > 0);
}

void StaticDialogTest::testGetTopPoint() {
    _dialog = std::make_unique<StaticDialog>();
    _dialog->init(_parentWidget.get());
    _dialog->create("Test Dialog");

    QPoint point = _dialog->getTopPoint(_dialog->getWidget(), true);
    // Point should be valid
    QVERIFY(point.x() >= 0);
}

} // namespace Tests
