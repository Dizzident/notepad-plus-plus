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
    class ShortcutMapper;
}

namespace Tests {

class ShortcutMapperTest : public QObject {
    Q_OBJECT

public:
    ShortcutMapperTest();
    ~ShortcutMapperTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Initialization
    void testInit();
    void testShowDialog();

private:
    std::unique_ptr<QWidget> _parentWidget;
    std::unique_ptr<QtControls::ShortcutMapper> _dialog;
};

} // namespace Tests
