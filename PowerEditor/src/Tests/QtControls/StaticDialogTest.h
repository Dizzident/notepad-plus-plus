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
    class StaticDialog;
}

namespace Tests {

class StaticDialogTest : public QObject {
    Q_OBJECT

public:
    StaticDialogTest();
    ~StaticDialogTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Creation and lifecycle
    void testCreate();
    void testIsCreated();
    void testDestroy();

    // Display
    void testDisplay();
    void testGoToCenter();

    // Checkboxes
    void testIsCheckedOrNot();
    void testSetChecked();

    // Position and sizing
    void testGetMappedChildRect();
    void testRedrawDlgItem();
    void testGetViewablePositionRect();
    void testGetTopPoint();

private:
    std::unique_ptr<QWidget> _parentWidget;
    std::unique_ptr<QtControls::StaticDialog> _dialog;
};

} // namespace Tests
