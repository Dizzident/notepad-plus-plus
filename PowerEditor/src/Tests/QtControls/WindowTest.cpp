// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "WindowTest.h"
#include "Window.h"
#include "../Common/TestUtils.h"
#include <QRect>
#include <QWidget>

using namespace QtControls;

// Testable concrete implementation of the abstract Window class
class TestableWindow : public Window {
public:
    TestableWindow(QObject* parent = nullptr) : Window(parent) {}

    void destroy() override {
        if (_widget) {
            _widget->deleteLater();
            _widget = nullptr;
        }
    }

    void init(QWidget* parent) override {
        _parent = parent;
        _widget = new QWidget(parent);
    }
};

namespace Tests {

WindowTest::WindowTest() {}

WindowTest::~WindowTest() {}

void WindowTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void WindowTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void WindowTest::init() {
    _parentWidget = std::make_unique<QWidget>();
    _parentWidget->resize(800, 600);
    _window = std::make_unique<TestableWindow>();
}

void WindowTest::cleanup() {
    _window.reset();
    _parentWidget.reset();
}

// ============================================================================
// Initialization Tests
// ============================================================================
void WindowTest::testInit() {
    QVERIFY(_window != nullptr);
    _window->init(_parentWidget.get());
    QCOMPARE(_window->getParent(), _parentWidget.get());
}

void WindowTest::testDestroy() {
    _window->init(_parentWidget.get());
    _window->destroy();
    // Window should be in destroyed state
    QVERIFY(_window->getWidget() == nullptr);
}

// ============================================================================
// Visibility Tests
// ============================================================================
void WindowTest::testDisplay() {
    if (Tests::WidgetTestUtils::isHeadlessEnvironment()) {
        QSKIP("Skipping visibility test in headless environment");
    }

    _window->init(_parentWidget.get());
    QVERIFY(!_window->isVisible());

    _window->display(true);
    QVERIFY(_window->isVisible());

    _window->display(false);
    QVERIFY(!_window->isVisible());
}

void WindowTest::testIsVisible() {
    if (Tests::WidgetTestUtils::isHeadlessEnvironment()) {
        QSKIP("Skipping visibility test in headless environment");
    }

    _window->init(_parentWidget.get());
    QVERIFY(!_window->isVisible());

    _window->display(true);
    QVERIFY(_window->isVisible());
}

// ============================================================================
// Geometry Tests
// ============================================================================
void WindowTest::testReSizeTo() {
    if (Tests::WidgetTestUtils::isHeadlessEnvironment()) {
        QSKIP("Skipping geometry test in headless environment");
    }

    _window->init(_parentWidget.get());
    _window->display(true);

    QRect newRect(10, 20, 300, 400);
    _window->reSizeTo(newRect);

    QRect clientRect;
    _window->getClientRect(clientRect);
    QCOMPARE(clientRect, newRect);
}

void WindowTest::testReSizeToWH() {
    if (Tests::WidgetTestUtils::isHeadlessEnvironment()) {
        QSKIP("Skipping geometry test in headless environment");
    }

    _window->init(_parentWidget.get());
    _window->display(true);

    QRect newRect(10, 20, 300, 400);
    _window->reSizeToWH(newRect);

    // Should have same geometry
    QRect clientRect;
    _window->getClientRect(clientRect);
    QCOMPARE(clientRect.x(), 10);
    QCOMPARE(clientRect.y(), 20);
    QCOMPARE(clientRect.width(), 300);
    QCOMPARE(clientRect.height(), 400);
}

void WindowTest::testGetClientRect() {
    _window->init(_parentWidget.get());
    _window->display(true);

    QRect rect;
    _window->getClientRect(rect);

    // Should return valid rect
    QVERIFY(rect.width() >= 0);
    QVERIFY(rect.height() >= 0);
}

void WindowTest::testGetWindowRect() {
    _window->init(_parentWidget.get());
    _window->display(true);

    QRect rect;
    _window->getWindowRect(rect);

    // Should return valid rect
    QVERIFY(rect.width() >= 0);
    QVERIFY(rect.height() >= 0);
}

void WindowTest::testGetWidth() {
    _window->init(_parentWidget.get());
    _window->display(true);

    int width = _window->getWidth();
    QVERIFY(width >= 0);
}

void WindowTest::testGetHeight() {
    _window->init(_parentWidget.get());
    _window->display(true);

    int height = _window->getHeight();
    QVERIFY(height >= 0);
}

// ============================================================================
// Redraw Tests
// ============================================================================
void WindowTest::testRedraw() {
    _window->init(_parentWidget.get());
    _window->display(true);

    // Should not crash
    _window->redraw();
    QVERIFY(true);
}

void WindowTest::testRedrawForceUpdate() {
    _window->init(_parentWidget.get());
    _window->display(true);

    // Should not crash
    _window->redraw(true);
    QVERIFY(true);
}

// ============================================================================
// Focus Tests
// ============================================================================
void WindowTest::testGrabFocus() {
    _window->init(_parentWidget.get());
    _window->display(true);

    // Should not crash
    _window->grabFocus();
    QVERIFY(true);
}

// ============================================================================
// Widget Access Tests
// ============================================================================
void WindowTest::testGetWidget() {
    _window->init(_parentWidget.get());
    QVERIFY(_window->getWidget() != nullptr);
}

void WindowTest::testGetParent() {
    _window->init(_parentWidget.get());
    QCOMPARE(_window->getParent(), _parentWidget.get());
}

} // namespace Tests
