// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QString>
#include <memory>

namespace Platform {
    class ISettings;
}

namespace Tests {

class SettingsTest : public QObject {
    Q_OBJECT

public:
    SettingsTest();
    ~SettingsTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Initialization
    void testInit();
    void testGetConfigPath();
    void testGetSettingsDir();

    // Basic settings (INI-style)
    void testWriteInt();
    void testReadInt();
    void testWriteString();
    void testReadString();
    void testWriteBool();
    void testReadBool();
    void testWriteBinary();
    void testReadBinary();

    // XML settings
    void testSaveConfig();
    void testLoadConfig();
    void testSetXmlValue();
    void testGetXmlValue();
    void testSetXmlValueInt();
    void testGetXmlValueInt();
    void testSetXmlValueBool();
    void testGetXmlValueBool();

    // Session management
    void testSaveSession();
    void testLoadSession();

    // Recent files
    void testAddToRecentFiles();
    void testGetRecentFiles();
    void testClearRecentFiles();

    // Plugin settings
    void testWritePluginSetting();
    void testReadPluginSetting();

    // Default values
    void testDefaultValues();
    void testOverwriteExisting();

private:
    Platform::ISettings* _settings = nullptr;
    std::unique_ptr<QTemporaryDir> _tempDir;
    QString _tempPath;

    void setupTestConfig();
};

} // namespace Tests
