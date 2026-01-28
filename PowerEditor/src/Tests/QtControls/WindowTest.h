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
    class Window;
}

namespace Tests {

class WindowTest : public QObject {
    Q_OBJECT

public:
    WindowTest();
    ~WindowTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Initialization
    void testInit();
    void testDestroy();

    // Visibility
    void testDisplay();
    void testIsVisible();

    // Geometry
    void testReSizeTo();
    void testReSizeToWH();
    void testGetClientRect();
    void testGetWindowRect();
    void testGetWidth();
    void testGetHeight();

    // Redraw
    void testRedraw();
    void testRedrawForceUpdate();

    // Focus
    void testGrabFocus();

    // Widget access
    void testGetWidget();
    void testGetParent();

private:
    std::unique_ptr<QWidget> _parentWidget;
    std::unique_ptr<QtControls::Window> _window;
};

} // namespace Tests
