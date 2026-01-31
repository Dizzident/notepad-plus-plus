// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "SettingsTest.h"
#include "Settings.h"
#include "../Common/TestUtils.h"
#include <QDir>
#include <QFile>
#include <QTextStream>

using namespace PlatformLayer;

namespace Tests {

SettingsTest::SettingsTest() {}

SettingsTest::~SettingsTest() {}

void SettingsTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
}

void SettingsTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void SettingsTest::init() {
    _tempDir = std::make_unique<QTemporaryDir>();
    QVERIFY(_tempDir->isValid());
    _tempPath = _tempDir->path();

    _settings = &ISettings::getInstance();
    QVERIFY(_settings != nullptr);

    setupTestConfig();
}

void SettingsTest::cleanup() {
    _tempDir.reset();
}

void SettingsTest::setupTestConfig() {
    // Create a minimal test config structure
    QDir configDir(_tempPath);
    configDir.mkpath(".");
}

// ============================================================================
// Initialization Tests
// ============================================================================
void SettingsTest::testInit() {
    QVERIFY(_settings->init());
}

void SettingsTest::testGetConfigPath() {
    std::wstring configPath = _settings->getConfigPath();
    QVERIFY(!configPath.empty());
}

void SettingsTest::testGetSettingsDir() {
    std::wstring settingsDir = _settings->getSettingsDir();
    QVERIFY(!settingsDir.empty());
}

// ============================================================================
// Basic Settings Tests (INI-style)
// ============================================================================
void SettingsTest::testWriteInt() {
    QVERIFY(_settings->writeInt(L"TestSection", L"IntKey", 42));
}

void SettingsTest::testReadInt() {
    _settings->writeInt(L"TestSection", L"IntKey", 42);

    int value = _settings->readInt(L"TestSection", L"IntKey", 0);
    QCOMPARE(value, 42);
}

void SettingsTest::testWriteString() {
    QVERIFY(_settings->writeString(L"TestSection", L"StringKey", L"TestValue"));
}

void SettingsTest::testReadString() {
    _settings->writeString(L"TestSection", L"StringKey", L"TestValue");

    std::wstring value = _settings->readString(L"TestSection", L"StringKey", L"");
    QCOMPARE(value, std::wstring(L"TestValue"));
}

void SettingsTest::testWriteBool() {
    QVERIFY(_settings->writeBool(L"TestSection", L"BoolKey", true));
    QVERIFY(_settings->writeBool(L"TestSection", L"BoolKey2", false));
}

void SettingsTest::testReadBool() {
    _settings->writeBool(L"TestSection", L"BoolKey", true);
    _settings->writeBool(L"TestSection", L"BoolKey2", false);

    QVERIFY(_settings->readBool(L"TestSection", L"BoolKey", false));
    QVERIFY(!_settings->readBool(L"TestSection", L"BoolKey2", true));
}

void SettingsTest::testWriteBinary() {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0xFF};
    QVERIFY(_settings->writeBinary(L"TestSection", L"BinaryKey", data.data(), data.size()));
}

void SettingsTest::testReadBinary() {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0xFF};
    _settings->writeBinary(L"TestSection", L"BinaryKey", data.data(), data.size());

    std::vector<uint8_t> readData = _settings->readBinary(L"TestSection", L"BinaryKey");
    QCOMPARE(static_cast<int>(readData.size()), 5);
    QCOMPARE(static_cast<int>(readData[0]), 0x01);
    QCOMPARE(static_cast<int>(readData[4]), 0xFF);
}

// ============================================================================
// XML Settings Tests
// ============================================================================
void SettingsTest::testSaveConfig() {
    QVERIFY(_settings->saveConfig());
}

void SettingsTest::testLoadConfig() {
    QVERIFY(_settings->loadConfig());
}

void SettingsTest::testSetXmlValue() {
    QVERIFY(_settings->setXmlValue(L"/NotepadPlus/GuiConfig/ToolBar", L"Standard"));
}

void SettingsTest::testGetXmlValue() {
    _settings->setXmlValue(L"/NotepadPlus/GuiConfig/ToolBar", L"Standard");

    std::wstring value = _settings->getXmlValue(L"/NotepadPlus/GuiConfig/ToolBar", L"");
    QCOMPARE(value, std::wstring(L"Standard"));
}

void SettingsTest::testSetXmlValueInt() {
    QVERIFY(_settings->setXmlValueInt(L"/NotepadPlus/GuiConfig/TabBar", 1));
}

void SettingsTest::testGetXmlValueInt() {
    _settings->setXmlValueInt(L"/NotepadPlus/GuiConfig/TabBar", 42);

    int value = _settings->getXmlValueInt(L"/NotepadPlus/GuiConfig/TabBar", 0);
    QCOMPARE(value, 42);
}

void SettingsTest::testSetXmlValueBool() {
    QVERIFY(_settings->setXmlValueBool(L"/NotepadPlus/GuiConfig/StatusBar", true));
}

void SettingsTest::testGetXmlValueBool() {
    _settings->setXmlValueBool(L"/NotepadPlus/GuiConfig/StatusBar", true);

    bool value = _settings->getXmlValueBool(L"/NotepadPlus/GuiConfig/StatusBar", false);
    QVERIFY(value);
}

// ============================================================================
// Session Management Tests
// ============================================================================
void SettingsTest::testSaveSession() {
    SessionInfo session;
    session.files.push_back(SessionFileInfo(L"/path/to/file1.txt"));
    session.files.push_back(SessionFileInfo(L"/path/to/file2.cpp"));
    session.activeIndex = 0;

    QVERIFY(_settings->saveSession(session));
}

void SettingsTest::testLoadSession() {
    // First save a session
    SessionInfo saveSession;
    saveSession.files.push_back(SessionFileInfo(L"/path/to/file1.txt"));
    saveSession.files.push_back(SessionFileInfo(L"/path/to/file2.cpp"));
    saveSession.activeIndex = 1;
    _settings->saveSession(saveSession);

    // Then load it
    SessionInfo loadSession;
    QVERIFY(_settings->loadSession(loadSession));

    QCOMPARE(static_cast<int>(loadSession.files.size()), 2);
    QCOMPARE(loadSession.activeIndex, static_cast<intptr_t>(1));
}

// ============================================================================
// Recent Files Tests
// ============================================================================
void SettingsTest::testAddToRecentFiles() {
    _settings->addToRecentFiles(L"/path/to/file1.txt");
    _settings->addToRecentFiles(L"/path/to/file2.cpp");

    std::vector<std::wstring> recent = _settings->getRecentFiles();
    QVERIFY(!recent.empty());
}

void SettingsTest::testGetRecentFiles() {
    // Clear first
    _settings->clearRecentFiles();

    _settings->addToRecentFiles(L"/path/to/file1.txt");
    _settings->addToRecentFiles(L"/path/to/file2.cpp");
    _settings->addToRecentFiles(L"/path/to/file3.h");

    std::vector<std::wstring> recent = _settings->getRecentFiles();
    QCOMPARE(static_cast<int>(recent.size()), 3);

    // Check order (most recent first)
    QCOMPARE(recent[0], std::wstring(L"/path/to/file3.h"));
    QCOMPARE(recent[1], std::wstring(L"/path/to/file2.cpp"));
    QCOMPARE(recent[2], std::wstring(L"/path/to/file1.txt"));
}

void SettingsTest::testClearRecentFiles() {
    _settings->addToRecentFiles(L"/path/to/file1.txt");
    QVERIFY(!_settings->getRecentFiles().empty());

    _settings->clearRecentFiles();
    QVERIFY(_settings->getRecentFiles().empty());
}

// ============================================================================
// Plugin Settings Tests
// ============================================================================
void SettingsTest::testWritePluginSetting() {
    QVERIFY(_settings->writePluginSetting(L"TestPlugin", L"SettingKey", L"SettingValue"));
}

void SettingsTest::testReadPluginSetting() {
    _settings->writePluginSetting(L"TestPlugin", L"SettingKey", L"SettingValue");

    std::wstring value = _settings->readPluginSetting(L"TestPlugin", L"SettingKey", L"");
    QCOMPARE(value, std::wstring(L"SettingValue"));
}

// ============================================================================
// Default Values Tests
// ============================================================================
void SettingsTest::testDefaultValues() {
    // Reading non-existent keys should return default values
    int intValue = _settings->readInt(L"NonExistent", L"NonExistent", 123);
    QCOMPARE(intValue, 123);

    std::wstring stringValue = _settings->readString(L"NonExistent", L"NonExistent", L"Default");
    QCOMPARE(stringValue, std::wstring(L"Default"));

    bool boolValue = _settings->readBool(L"NonExistent", L"NonExistent", true);
    QVERIFY(boolValue);
}

void SettingsTest::testOverwriteExisting() {
    // Write initial value
    _settings->writeInt(L"Overwrite", L"Key", 100);
    QCOMPARE(_settings->readInt(L"Overwrite", L"Key", 0), 100);

    // Overwrite with new value
    _settings->writeInt(L"Overwrite", L"Key", 200);
    QCOMPARE(_settings->readInt(L"Overwrite", L"Key", 0), 200);
}

} // namespace Tests

#include "SettingsTest.moc"
