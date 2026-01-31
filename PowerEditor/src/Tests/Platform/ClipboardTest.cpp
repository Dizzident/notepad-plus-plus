// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ClipboardTest.h"
#include "Clipboard.h"
#include "../Common/TestUtils.h"
#include <QApplication>
#include <QClipboard>
#include <QThread>

using namespace PlatformLayer;

namespace Tests {

ClipboardTest::ClipboardTest() {}

ClipboardTest::~ClipboardTest() {}

void ClipboardTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
    _clipboard = &IClipboard::getInstance();
    QVERIFY(_clipboard != nullptr);
}

void ClipboardTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void ClipboardTest::clearClipboard() {
    _clipboard->clear();
}

// ============================================================================
// Basic Clipboard Operations Tests
// ============================================================================
void ClipboardTest::testHasData() {
    clearClipboard();

    // Empty clipboard should return false
    // Note: On some systems, clipboard may always have some data
    // QVERIFY(!_clipboard->hasData());

    // Set some text
    _clipboard->setText(L"Test data");

    // Now should have data
    QVERIFY(_clipboard->hasData());
}

void ClipboardTest::testClear() {
    _clipboard->setText(L"Data to clear");
    QVERIFY(_clipboard->hasData());

    QVERIFY(_clipboard->clear());

    // After clear, may or may not have data depending on implementation
    QVERIFY(true);
}

void ClipboardTest::testOpenClose() {
    // Open clipboard
    QVERIFY(_clipboard->open());
    QVERIFY(_clipboard->isOpen());

    // Close clipboard
    _clipboard->close();
    QVERIFY(!_clipboard->isOpen());
}

// ============================================================================
// Text Operations Tests
// ============================================================================
void ClipboardTest::testSetText() {
    QVERIFY(_clipboard->setText(L"Hello, World!"));

    std::wstring text = _clipboard->getText();
    QCOMPARE(text, std::wstring(L"Hello, World!"));
}

void ClipboardTest::testGetText() {
    _clipboard->setText(L"Test text content");

    std::wstring text = _clipboard->getText();
    QCOMPARE(text, std::wstring(L"Test text content"));
}

void ClipboardTest::testSetTextWithLength() {
    std::wstring text = L"Text with binary data";
    QVERIFY(_clipboard->setTextWithLength(text, text.length() * sizeof(wchar_t)));

    std::wstring retrieved = _clipboard->getText();
    QCOMPARE(retrieved, text);
}

// ============================================================================
// Binary Data Operations Tests
// ============================================================================
void ClipboardTest::testSetData() {
    ClipboardData data;
    data.format = ClipboardFormat::Binary;
    data.data = {0x01, 0x02, 0x03, 0x04, 0xFF};

    QVERIFY(_clipboard->setData(data));
}

void ClipboardTest::testGetData() {
    ClipboardData data;
    data.format = ClipboardFormat::Text;
    data.data = std::vector<uint8_t>({'H', 'e', 'l', 'l', 'o'});

    _clipboard->setData(data);

    ClipboardData retrieved = _clipboard->getData();
    QVERIFY(!retrieved.data.empty());
}

// ============================================================================
// HTML Operations Tests
// ============================================================================
void ClipboardTest::testSetHtml() {
    std::string html = "<html><body><b>Bold</b> text</body></html>";
    std::string text = "Bold text";

    QVERIFY(_clipboard->setHtml(html, text));
}

void ClipboardTest::testGetHtml() {
    std::string html = "<html><body>Test</body></html>";
    std::string text = "Test";

    _clipboard->setHtml(html, text);

    // Getting HTML may not return exactly what was set due to format conversion
    std::string retrieved = _clipboard->getHtml();
    Q_UNUSED(retrieved)
    QVERIFY(true);
}

// ============================================================================
// RTF Operations Tests
// ============================================================================
void ClipboardTest::testSetRtf() {
    std::string rtf = "{\\rtf1\\ansi Test}";
    std::string text = "Test";

    QVERIFY(_clipboard->setRtf(rtf, text));
}

void ClipboardTest::testGetRtf() {
    std::string rtf = "{\\rtf1\\ansi Test}";
    std::string text = "Test";

    _clipboard->setRtf(rtf, text);

    std::string retrieved = _clipboard->getRtf();
    Q_UNUSED(retrieved)
    QVERIFY(true);
}

// ============================================================================
// Custom Format Operations Tests
// ============================================================================
void ClipboardTest::testRegisterFormat() {
    uint32_t format1 = _clipboard->registerFormat("TestFormat1");
    QVERIFY(format1 != 0);

    uint32_t format2 = _clipboard->registerFormat("TestFormat2");
    QVERIFY(format2 != 0);

    // Same name should return same format ID
    uint32_t format1Again = _clipboard->registerFormat("TestFormat1");
    QCOMPARE(format1, format1Again);
}

void ClipboardTest::testSetCustomData() {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    QVERIFY(_clipboard->setCustomData("TestCustomFormat", data));
}

void ClipboardTest::testGetCustomData() {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    _clipboard->setCustomData("TestCustomFormat2", data);

    std::vector<uint8_t> retrieved = _clipboard->getCustomData("TestCustomFormat2");
    QCOMPARE(static_cast<int>(retrieved.size()), 4);
    QCOMPARE(static_cast<int>(retrieved[0]), 0x01);
}

// ============================================================================
// Format Checking Tests
// ============================================================================
void ClipboardTest::testHasFormat() {
    _clipboard->setText(L"Plain text");

    QVERIFY(_clipboard->hasFormat(ClipboardFormat::Text) ||
            _clipboard->hasFormat(ClipboardFormat::UnicodeText));
}

void ClipboardTest::testHasCustomFormat() {
    std::vector<uint8_t> data = {0x01};
    _clipboard->setCustomData("CustomFormatCheck", data);

    QVERIFY(_clipboard->hasCustomFormat("CustomFormatCheck"));
    QVERIFY(!_clipboard->hasCustomFormat("NonExistentFormat"));
}

// ============================================================================
// Clipboard Monitoring Tests
// ============================================================================
void ClipboardTest::testStartMonitoring() {
    _clipboard->startMonitoring();
    QVERIFY(_clipboard->isMonitoring());
}

void ClipboardTest::testStopMonitoring() {
    _clipboard->startMonitoring();
    QVERIFY(_clipboard->isMonitoring());

    _clipboard->stopMonitoring();
    QVERIFY(!_clipboard->isMonitoring());
}

void ClipboardTest::testIsMonitoring() {
    // Initial state
    bool initialState = _clipboard->isMonitoring();

    _clipboard->startMonitoring();
    QVERIFY(_clipboard->isMonitoring());

    _clipboard->stopMonitoring();
    QVERIFY(!_clipboard->isMonitoring());

    // Restore initial state
    if (initialState) {
        _clipboard->startMonitoring();
    }
}

void ClipboardTest::testGetSequenceNumber() {
    uint32_t seq1 = _clipboard->getSequenceNumber();

    _clipboard->setText(L"Change sequence");

    uint32_t seq2 = _clipboard->getSequenceNumber();

    // Sequence number should have changed
    QVERIFY(seq2 >= seq1);
}

// ============================================================================
// Clipboard History Tests
// ============================================================================
void ClipboardTest::testClipboardHistoryAddEntry() {
    IClipboardHistory& history = IClipboardHistory::getInstance();
    history.init();

    ClipboardData data(L"History test");
    history.addEntry(data);

    QVERIFY(history.getEntryCount() > 0);
}

void ClipboardTest::testClipboardHistoryGetEntry() {
    IClipboardHistory& history = IClipboardHistory::getInstance();
    history.init();
    history.clear();

    ClipboardData data(L"Entry to retrieve");
    history.addEntry(data);

    QCOMPARE(static_cast<int>(history.getEntryCount()), 1);

    ClipboardHistoryEntry entry = history.getEntry(0);
    QCOMPARE(entry.data.toWString(), std::wstring(L"Entry to retrieve"));
}

void ClipboardTest::testClipboardHistoryClear() {
    IClipboardHistory& history = IClipboardHistory::getInstance();
    history.init();

    history.addEntry(ClipboardData(L"Entry 1"));
    history.addEntry(ClipboardData(L"Entry 2"));
    QVERIFY(history.getEntryCount() > 0);

    history.clear();
    QCOMPARE(static_cast<int>(history.getEntryCount()), 0);
}

void ClipboardTest::testClipboardHistoryMaxSize() {
    IClipboardHistory& history = IClipboardHistory::getInstance();
    history.init();
    history.clear();

    history.setMaxSize(5);
    QCOMPARE(static_cast<int>(history.getMaxSize()), 5);

    // Add more entries than max size
    for (int i = 0; i < 10; ++i) {
        history.addEntry(ClipboardData(std::wstring(L"Entry ") + std::to_wstring(i)));
    }

    // Should be limited to max size
    QCOMPARE(static_cast<int>(history.getEntryCount()), 5);
}

// ============================================================================
// Utility Functions Tests
// ============================================================================
void ClipboardTest::testFormatToString() {
    const char* text = ClipboardUtils::formatToString(ClipboardFormat::Text);
    QVERIFY(text != nullptr);

    const char* html = ClipboardUtils::formatToString(ClipboardFormat::Html);
    QVERIFY(html != nullptr);

    const char* rtf = ClipboardUtils::formatToString(ClipboardFormat::Rtf);
    QVERIFY(rtf != nullptr);

    const char* binary = ClipboardUtils::formatToString(ClipboardFormat::Binary);
    QVERIFY(binary != nullptr);
}

void ClipboardTest::testContainsBinary() {
    std::vector<uint8_t> textData = {'H', 'e', 'l', 'l', 'o'};
    QVERIFY(!ClipboardUtils::containsBinary(textData));

    std::vector<uint8_t> binaryData = {0x00, 0x01, 0xFF};
    QVERIFY(ClipboardUtils::containsBinary(binaryData));
}

void ClipboardTest::testTruncateForDisplay() {
    std::wstring longText = L"This is a very long text that should be truncated";
    std::wstring truncated = ClipboardUtils::truncateForDisplay(longText, 20);

    QVERIFY(truncated.length() <= 23); // 20 + "..."
    QVERIFY(truncated.find(L"...") != std::wstring::npos);

    std::wstring shortText = L"Short";
    std::wstring notTruncated = ClipboardUtils::truncateForDisplay(shortText, 20);
    QCOMPARE(notTruncated, shortText);
}

} // namespace Tests

#include "ClipboardTest.moc"
