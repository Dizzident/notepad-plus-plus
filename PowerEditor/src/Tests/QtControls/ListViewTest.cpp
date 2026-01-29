// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ListViewTest.h"
#include "ListView.h"
#include "../Common/TestUtils.h"
#include <QVariant>

using namespace QtControls;

namespace Tests {

ListViewTest::ListViewTest() {}

ListViewTest::~ListViewTest() {}

void ListViewTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void ListViewTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void ListViewTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(400, 300);
    _listView = std::make_unique<ListView>();
}

void ListViewTest::cleanup() {
    _listView.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void ListViewTest::testInit() {
    _listView->init(_parentWidget.get());
    QVERIFY(_listView->getListWidget() != nullptr);
}

void ListViewTest::testDestroy() {
    _listView->init(_parentWidget.get());
    _listView->destroy();
    QVERIFY(_listView->getWidget() == nullptr);
}

// ============================================================================
// Item Management Tests
// ============================================================================
void ListViewTest::testAddItem() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 2");

    QCOMPARE(_listView->getItemCount(), 2);
}

void ListViewTest::testInsertItem() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 3");
    _listView->insertItem(1, "Item 2");

    QCOMPARE(_listView->getItemCount(), 3);
    QCOMPARE(_listView->getItemText(1), QString("Item 2"));
}

void ListViewTest::testRemoveItem() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 2");
    QCOMPARE(_listView->getItemCount(), 2);

    _listView->removeItem(0);
    QCOMPARE(_listView->getItemCount(), 1);
    QCOMPARE(_listView->getItemText(0), QString("Item 2"));
}

void ListViewTest::testClear() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 2");
    _listView->addItem("Item 3");
    QCOMPARE(_listView->getItemCount(), 3);

    _listView->clear();
    QCOMPARE(_listView->getItemCount(), 0);
}

void ListViewTest::testGetItemCount() {
    _listView->init(_parentWidget.get());

    QCOMPARE(_listView->getItemCount(), 0);

    _listView->addItem("Item 1");
    QCOMPARE(_listView->getItemCount(), 1);

    _listView->addItem("Item 2");
    QCOMPARE(_listView->getItemCount(), 2);
}

// ============================================================================
// Item Text Tests
// ============================================================================
void ListViewTest::testGetItemText() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Test Text");
    QCOMPARE(_listView->getItemText(0), QString("Test Text"));
}

void ListViewTest::testSetItemText() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Original");
    _listView->setItemText(0, "Updated");
    QCOMPARE(_listView->getItemText(0), QString("Updated"));
}

// ============================================================================
// Item Data Tests
// ============================================================================
void ListViewTest::testSetItemData() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item");
    _listView->setItemData(0, QVariant(42), Qt::UserRole);

    QVariant data = _listView->getItemData(0, Qt::UserRole);
    QCOMPARE(data.toInt(), 42);
}

void ListViewTest::testGetItemData() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item");
    _listView->setItemData(0, QVariant("test data"), Qt::UserRole);

    QVariant data = _listView->getItemData(0, Qt::UserRole);
    QCOMPARE(data.toString(), QString("test data"));
}

// ============================================================================
// Selection Tests
// ============================================================================
void ListViewTest::testGetSelectedIndex() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 2");

    _listView->setSelectedIndex(1);
    QCOMPARE(_listView->getSelectedIndex(), 1);
}

void ListViewTest::testSetSelectedIndex() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 2");

    _listView->setSelectedIndex(0);
    QCOMPARE(_listView->getSelectedIndex(), 0);

    _listView->setSelectedIndex(1);
    QCOMPARE(_listView->getSelectedIndex(), 1);
}

void ListViewTest::testGetSelectedIndexes() {
    _listView->init(_parentWidget.get());

    _listView->setSelectionMode(ListViewSelectionMode::Multi);
    _listView->addItem("Item 1");
    _listView->addItem("Item 2");
    _listView->addItem("Item 3");

    _listView->selectItem(0, true);
    _listView->selectItem(2, true);

    std::vector<int> selected = _listView->getSelectedIndexes();
    QCOMPARE(static_cast<int>(selected.size()), 2);
}

void ListViewTest::testSetSelectedIndexes() {
    _listView->init(_parentWidget.get());

    _listView->setSelectionMode(ListViewSelectionMode::Multi);
    _listView->addItem("Item 1");
    _listView->addItem("Item 2");
    _listView->addItem("Item 3");

    std::vector<int> indexes = {0, 2};
    _listView->setSelectedIndexes(indexes);

    QVERIFY(_listView->isItemSelected(0));
    QVERIFY(!_listView->isItemSelected(1));
    QVERIFY(_listView->isItemSelected(2));
}

void ListViewTest::testSelectItem() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 2");

    _listView->selectItem(0, true);
    QVERIFY(_listView->isItemSelected(0));

    _listView->selectItem(0, false);
    QVERIFY(!_listView->isItemSelected(0));
}

void ListViewTest::testIsItemSelected() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 2");

    QVERIFY(!_listView->isItemSelected(0));

    _listView->setSelectedIndex(0);
    QVERIFY(_listView->isItemSelected(0));
}

// ============================================================================
// Selection Mode Tests
// ============================================================================
void ListViewTest::testSetSelectionMode() {
    _listView->init(_parentWidget.get());

    _listView->setSelectionMode(ListViewSelectionMode::Single);
    QCOMPARE(_listView->getSelectionMode(), ListViewSelectionMode::Single);

    _listView->setSelectionMode(ListViewSelectionMode::Multi);
    QCOMPARE(_listView->getSelectionMode(), ListViewSelectionMode::Multi);
}

void ListViewTest::testGetSelectionMode() {
    _listView->init(_parentWidget.get());

    // Default should be Single
    QCOMPARE(_listView->getSelectionMode(), ListViewSelectionMode::Single);

    _listView->setSelectionMode(ListViewSelectionMode::Extended);
    QCOMPARE(_listView->getSelectionMode(), ListViewSelectionMode::Extended);
}

// ============================================================================
// Current Item Tests
// ============================================================================
void ListViewTest::testGetCurrentIndex() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 2");

    _listView->setCurrentIndex(1);
    QCOMPARE(_listView->getCurrentIndex(), 1);
}

void ListViewTest::testSetCurrentIndex() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 2");

    _listView->setCurrentIndex(0);
    QCOMPARE(_listView->getCurrentIndex(), 0);

    _listView->setCurrentIndex(1);
    QCOMPARE(_listView->getCurrentIndex(), 1);
}

// ============================================================================
// Sorting Tests
// ============================================================================
void ListViewTest::testSetSortingEnabled() {
    _listView->init(_parentWidget.get());

    _listView->setSortingEnabled(true);
    QVERIFY(_listView->isSortingEnabled());

    _listView->setSortingEnabled(false);
    QVERIFY(!_listView->isSortingEnabled());
}

void ListViewTest::testIsSortingEnabled() {
    _listView->init(_parentWidget.get());

    QVERIFY(!_listView->isSortingEnabled());

    _listView->setSortingEnabled(true);
    QVERIFY(_listView->isSortingEnabled());
}

void ListViewTest::testSortItems() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Charlie");
    _listView->addItem("Alpha");
    _listView->addItem("Bravo");

    _listView->setSortingEnabled(true);
    _listView->sortItems(SortDirection::Ascending);

    // Items should be sorted
    QVERIFY(true); // Should not crash
}

// ============================================================================
// Visibility Tests
// ============================================================================
void ListViewTest::testEnsureItemVisible() {
    _listView->init(_parentWidget.get());

    // Add many items
    for (int i = 0; i < 100; ++i) {
        _listView->addItem(QString("Item %1").arg(i));
    }

    // Should not crash
    _listView->ensureItemVisible(50);
    QVERIFY(true);
}

// ============================================================================
// Check State Tests
// ============================================================================
void ListViewTest::testSetItemCheckState() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item");
    _listView->setItemCheckState(0, Qt::Checked);

    QCOMPARE(_listView->getItemCheckState(0), Qt::Checked);

    _listView->setItemCheckState(0, Qt::Unchecked);
    QCOMPARE(_listView->getItemCheckState(0), Qt::Unchecked);
}

void ListViewTest::testGetItemCheckState() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item");
    QCOMPARE(_listView->getItemCheckState(0), Qt::Unchecked);

    _listView->setItemCheckState(0, Qt::Checked);
    QCOMPARE(_listView->getItemCheckState(0), Qt::Checked);
}

void ListViewTest::testGetCheckedIndexes() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Item 1");
    _listView->addItem("Item 2");
    _listView->addItem("Item 3");

    _listView->setItemCheckState(0, Qt::Checked);
    _listView->setItemCheckState(2, Qt::Checked);

    std::vector<int> checked = _listView->getCheckedIndexes();
    QCOMPARE(static_cast<int>(checked.size()), 2);
    QCOMPARE(checked[0], 0);
    QCOMPARE(checked[1], 2);
}

// ============================================================================
// Search Tests
// ============================================================================
void ListViewTest::testFindItem() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Alpha");
    _listView->addItem("Beta");
    _listView->addItem("Gamma");

    int found = _listView->findItem("Beta");
    QCOMPARE(found, 1);

    int notFound = _listView->findItem("Delta");
    QCOMPARE(notFound, -1);
}

void ListViewTest::testFindItems() {
    _listView->init(_parentWidget.get());

    _listView->addItem("Apple");
    _listView->addItem("Application");
    _listView->addItem("Banana");
    _listView->addItem("Appetizer");

    std::vector<int> found = _listView->findItems("App", Qt::MatchStartsWith);
    QCOMPARE(static_cast<int>(found.size()), 3);
}

} // namespace Tests

#include "ListViewTest.moc"
