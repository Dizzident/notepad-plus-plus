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
namespace RunDlg {
    class RunDlg;
}
}

namespace Tests {

class RunDlgTest : public QObject {
    Q_OBJECT

public:
    RunDlgTest();
    ~RunDlgTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Initialization
    void testInit();
    void testDoDialog();

    // Command management
    void testGetCommand();
    void testSetCommand();
    void testGetHistory();
    void testSetHistory();

private:
    std::unique_ptr<QWidget> _parentWidget;
    std::unique_ptr<QtControls::RunDlg::RunDlg> _dialog;
};

} // namespace Tests
