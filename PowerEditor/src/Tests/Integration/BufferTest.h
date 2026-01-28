// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QtTest/QtTest>
#include <memory>

class Buffer;

namespace Tests {

class BufferTest : public QObject {
    Q_OBJECT

public:
    BufferTest();
    ~BufferTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Buffer creation
    void testCreateNewBuffer();
    void testCreateBufferFromFile();

    // File operations
    void testLoadFromFile();
    void testSaveToFile();
    void testReload();

    // Buffer state
    void testIsDirty();
    void testSetDirty();
    void testIsReadOnly();
    void testSetReadOnly();

    // Encoding
    void testGetEncoding();
    void testSetEncoding();
    void testGetBom();
    void testSetBom();

    // Language
    void testGetLangType();
    void testSetLangType();

    // File path
    void testGetFilePath();
    void testSetFilePath();
    void testGetFileName();

    // Document management
    void testGetDocument();
    void testSetDocument();

    // Status
    void testGetStatus();
    void testSetStatus();

private:
    std::unique_ptr<Buffer> _buffer;
};

} // namespace Tests
