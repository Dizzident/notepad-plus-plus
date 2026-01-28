// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <QString>
#include <vector>

namespace Platform {
    class IClipboard;
}

namespace Tests {

class ClipboardTest : public QObject {
    Q_OBJECT

public:
    ClipboardTest();
    ~ClipboardTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    // Basic clipboard operations
    void testHasData();
    void testClear();
    void testOpenClose();

    // Text operations
    void testSetText();
    void testGetText();
    void testSetTextWithLength();

    // Binary data operations
    void testSetData();
    void testGetData();

    // HTML operations
    void testSetHtml();
    void testGetHtml();

    // RTF operations
    void testSetRtf();
    void testGetRtf();

    // Custom format operations
    void testRegisterFormat();
    void testSetCustomData();
    void testGetCustomData();

    // Format checking
    void testHasFormat();
    void testHasCustomFormat();

    // Clipboard monitoring
    void testStartMonitoring();
    void testStopMonitoring();
    void testIsMonitoring();
    void testGetSequenceNumber();

    // Clipboard history
    void testClipboardHistoryAddEntry();
    void testClipboardHistoryGetEntry();
    void testClipboardHistoryClear();
    void testClipboardHistoryMaxSize();

    // Utility functions
    void testFormatToString();
    void testContainsBinary();
    void testTruncateForDisplay();

private:
    Platform::IClipboard* _clipboard = nullptr;

    // Helper to ensure clipboard is in known state
    void clearClipboard();
};

} // namespace Tests
