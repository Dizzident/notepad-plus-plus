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
    class TreeView;
}

namespace Tests {

class TreeViewTest : public QObject {
    Q_OBJECT

public:
    TreeViewTest();
    ~TreeViewTest();

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
    void testRemoveItem();
    void testClear();
    void testSetItemText();
    void testGetItemText();
    void testSetItemData();
    void testGetItemData();

    // Selection
    void testGetSelectedItem();
    void testSetSelectedItem();
    void testClearSelection();

    // Expansion
    void testExpand();
    void testCollapse();
    void testExpandAll();
    void testCollapseAll();
    void testIsExpanded();

    // Checkboxes
    void testSetCheckable();
    void testIsCheckable();
    void testIsItemChecked();
    void testSetItemChecked();
    void testSetCheckState();
    void testGetCheckState();

    // Columns
    void testSetColumnCount();
    void testGetColumnCount();
    void testSetColumnText();
    void testGetColumnText();

    // Navigation
    void testGetRootItem();
    void testGetParentItem();
    void testGetChildItem();
    void testGetNextSibling();
    void testGetPrevSibling();

    // Sorting
    void testSortItems();
    void testSetSortingEnabled();

    // Search
    void testFindItem();
    void testFindItemByData();

    // State persistence
    void testRestoreFoldingStateFrom();
    void testRetrieveFoldingStateTo();

private:
    std::unique_ptr<QWidget> _parentWidget;
    std::unique_ptr<QtControls::TreeView> _treeView;
};

} // namespace Tests
