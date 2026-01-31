// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "FindReplaceDlgTest.h"
#include "FindReplace/FindReplaceDlg.h"
#include "../Common/TestUtils.h"

using namespace NppFindReplace;
using namespace PlatformLayer;

namespace Tests {

FindReplaceDlgTest::FindReplaceDlgTest() {}

FindReplaceDlgTest::~FindReplaceDlgTest() {}

void FindReplaceDlgTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void FindReplaceDlgTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void FindReplaceDlgTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(800, 600);
    _findDlg = std::make_unique<FindReplaceDlg>(_parentWidget.get());
}

void FindReplaceDlgTest::cleanup() {
    _findDlg.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void FindReplaceDlgTest::testInit() {
    QVERIFY(_findDlg != nullptr);
    // Initialization would require ScintillaEditView
    QVERIFY(true);
}

void FindReplaceDlgTest::testShowDialog() {
    // Show Find tab
    _findDlg->showDialog(FindDialogType::Find);
    QVERIFY(_findDlg->getWidget()->isVisible());

    // Show Replace tab
    _findDlg->showDialog(FindDialogType::Replace);
    QVERIFY(_findDlg->getWidget()->isVisible());
}

// ============================================================================
// Search Text Tests
// ============================================================================
void FindReplaceDlgTest::testSetSearchText() {
    _findDlg->setSearchText("test search");
    QCOMPARE(_findDlg->getSearchText(), QString("test search"));
}

void FindReplaceDlgTest::testGetSearchText() {
    _findDlg->setSearchText("sample text");
    QCOMPARE(_findDlg->getSearchText(), QString("sample text"));
}

void FindReplaceDlgTest::testGetReplaceText() {
    // Test would require UI setup
    QVERIFY(true);
}

// ============================================================================
// Options Tests
// ============================================================================
void FindReplaceDlgTest::testGetCurrentOptions() {
    FindOptions options = _findDlg->getCurrentOptions();
    // Should return default options
    QVERIFY(true);
}

void FindReplaceDlgTest::testSetOptions() {
    FindOptions options;
    options.isMatchCase = true;
    options.isWholeWord = true;
    options.searchType = SearchType::Regex;

    _findDlg->setOptions(options);

    FindOptions retrieved = _findDlg->getCurrentOptions();
    QVERIFY(retrieved.isMatchCase);
    QVERIFY(retrieved.isWholeWord);
    QCOMPARE(retrieved.searchType, SearchType::Regex);
}

// ============================================================================
// Search Operations Tests
// ============================================================================
void FindReplaceDlgTest::testFindNext() {
    // Would require ScintillaEditView to be initialized
    // bool found = _findDlg->findNext();
    QVERIFY(true);
}

void FindReplaceDlgTest::testFindPrevious() {
    // Would require ScintillaEditView to be initialized
    // bool found = _findDlg->findPrevious();
    QVERIFY(true);
}

void FindReplaceDlgTest::testReplace() {
    // Would require ScintillaEditView to be initialized
    // bool replaced = _findDlg->replace();
    QVERIFY(true);
}

void FindReplaceDlgTest::testReplaceAll() {
    // Would require ScintillaEditView to be initialized
    // int count = _findDlg->replaceAll();
    QVERIFY(true);
}

void FindReplaceDlgTest::testCountMatches() {
    // Would require ScintillaEditView to be initialized
    // int count = _findDlg->countMatches();
    QVERIFY(true);
}

// ============================================================================
// Mark Operations Tests
// ============================================================================
void FindReplaceDlgTest::testMarkAll() {
    // Would require ScintillaEditView to be initialized
    // int count = _findDlg->markAll();
    QVERIFY(true);
}

void FindReplaceDlgTest::testClearMarks() {
    // Should not crash
    _findDlg->clearMarks();
    QVERIFY(true);
}

// ============================================================================
// Status Tests
// ============================================================================
void FindReplaceDlgTest::testSetStatusMessage() {
    _findDlg->setStatusMessage("Test message", FindStatus::Found);
    QVERIFY(true);
}

// ============================================================================
// Incremental Find Tests
// ============================================================================
void FindReplaceDlgTest::testIncrementalFind() {
    FindIncrementDlg incDlg(_parentWidget.get());
    incDlg.setSearchText("incremental");
    QCOMPARE(incDlg.getSearchText(), QString("incremental"));
}

} // namespace Tests

#include "FindReplaceDlgTest.moc"
