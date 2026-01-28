// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <QWidget>
#include <memory>

namespace QtControls {
    class ListView;
}

namespace Tests {

class ListViewTest : public QObject {
    Q_OBJECT

public:
    ListViewTest();
    ~ListViewTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Initialization
    void testInit();
    void testDestroy();

    // Item management
    void testAddItem();
    void testInsertItem();
    void testRemoveItem();
    void testClear();
    void testGetItemCount();

    // Item text
    void testGetItemText();
    void testSetItemText();

    // Item data
    void testSetItemData();
    void testGetItemData();

    // Selection
    void testGetSelectedIndex();
    void testSetSelectedIndex();
    void testGetSelectedIndexes();
    void testSetSelectedIndexes();
    void testSelectItem();
    void testIsItemSelected();

    // Selection mode
    void testSetSelectionMode();
    void testGetSelectionMode();

    // Current item
    void testGetCurrentIndex();
    void testSetCurrentIndex();

    // Sorting
    void testSetSortingEnabled();
    void testIsSortingEnabled();
    void testSortItems();

    // Visibility
    void testEnsureItemVisible();

    // Check state
    void testSetItemCheckState();
    void testGetItemCheckState();
    void testGetCheckedIndexes();

    // Search
    void testFindItem();
    void testFindItems();

private:
    std::unique_ptr<QWidget> _parentWidget;
    std::unique_ptr<QtControls::ListView> _listView;
};

} // namespace Tests
