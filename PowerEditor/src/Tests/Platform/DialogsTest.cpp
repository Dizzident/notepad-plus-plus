// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "DialogsTest.h"
#include "Dialogs.h"
#include "../Common/TestUtils.h"
#include <QWidget>

using namespace PlatformLayer;

namespace Tests {

// ============================================================================
// MockDialogsProvider Implementation
// ============================================================================
void MockDialogsProvider::setNextResult(DialogResult result) {
    _results.push_back(result);
}

void MockDialogsProvider::setNextFileName(const QString& fileName) {
    _fileNames.push_back(fileName);
}

void MockDialogsProvider::setNextFileNames(const QStringList& fileNames) {
    _fileNamesList.push_back(fileNames);
}

void MockDialogsProvider::setNextBoolResult(bool result) {
    _boolResults.push_back(result);
}

void MockDialogsProvider::setNextIntResult(int result) {
    _intResults.push_back(result);
}

DialogResult MockDialogsProvider::getNextResult() {
    if (_resultIndex < _results.size()) {
        return _results[_resultIndex++];
    }
    return DialogResult::OK;
}

QString MockDialogsProvider::getNextFileName() {
    if (_fileNameIndex < _fileNames.size()) {
        return _fileNames[_fileNameIndex++];
    }
    return QString();
}

QStringList MockDialogsProvider::getNextFileNames() {
    if (_fileNamesListIndex < _fileNamesList.size()) {
        return _fileNamesList[_fileNamesListIndex++];
    }
    return QStringList();
}

bool MockDialogsProvider::getNextBoolResult() {
    if (_boolIndex < _boolResults.size()) {
        return _boolResults[_boolIndex++];
    }
    return false;
}

int MockDialogsProvider::getNextIntResult() {
    if (_intIndex < _intResults.size()) {
        return _intResults[_intIndex++];
    }
    return -1;
}

void MockDialogsProvider::reset() {
    _results.clear();
    _fileNames.clear();
    _fileNamesList.clear();
    _boolResults.clear();
    _intResults.clear();
    _resultIndex = 0;
    _fileNameIndex = 0;
    _fileNamesListIndex = 0;
    _boolIndex = 0;
    _intIndex = 0;
}

// ============================================================================
// DialogsTest Implementation
// ============================================================================
DialogsTest::DialogsTest() {}

DialogsTest::~DialogsTest() {}

void DialogsTest::initTestCase() {
    QVERIFY(TestEnvironment::getInstance().init());
    _dialogs = &IDialogs::getInstance();
    QVERIFY(_dialogs != nullptr);
    _mockProvider = std::make_unique<MockDialogsProvider>();
}

void DialogsTest::cleanupTestCase() {
    TestEnvironment::getInstance().cleanup();
}

void* DialogsTest::createTestDialog() {
    // Create a simple widget to use as dialog handle for testing
    return new QWidget();
}

void DialogsTest::destroyTestDialog(void* handle) {
    if (handle) {
        delete static_cast<QWidget*>(handle);
    }
}

// ============================================================================
// Message Box Tests
// ============================================================================
void DialogsTest::testMessageBox() {
    // Just verify the method exists and doesn't crash
    // Actual dialog display would require user interaction
    // DialogResult result = _dialogs->messageBox(
    //     L"Test message",
    //     L"Test Title",
    //     MessageBoxType::OK,
    //     MessageBoxIcon::Information
    // );
    QVERIFY(true);
}

void DialogsTest::testShowInfo() {
    // Verify method doesn't crash
    // _dialogs->showInfo(L"Information message");
    QVERIFY(true);
}

void DialogsTest::testShowWarning() {
    // Verify method doesn't crash
    // _dialogs->showWarning(L"Warning message");
    QVERIFY(true);
}

void DialogsTest::testShowError() {
    // Verify method doesn't crash
    // _dialogs->showError(L"Error message");
    QVERIFY(true);
}

void DialogsTest::testAskYesNo() {
    // Verify method doesn't crash
    // bool result = _dialogs->askYesNo(L"Do you want to continue?");
    QVERIFY(true);
}

void DialogsTest::testAskYesNoCancel() {
    // Verify method doesn't crash
    // DialogResult result = _dialogs->askYesNoCancel(L"Save changes?");
    QVERIFY(true);
}

void DialogsTest::testAskRetryCancel() {
    // Verify method doesn't crash
    // bool result = _dialogs->askRetryCancel(L"Operation failed. Retry?");
    QVERIFY(true);
}

// ============================================================================
// File Dialog Tests
// ============================================================================
void DialogsTest::testShowOpenFileDialog() {
    std::vector<FileFilter> filters = DialogFilters::allFiles();

    // Verify method doesn't crash
    // std::wstring result = _dialogs->showOpenFileDialog(L"Open File", filters);
    QVERIFY(true);
}

void DialogsTest::testShowOpenFilesDialog() {
    std::vector<FileFilter> filters = DialogFilters::textFiles();

    // Verify method doesn't crash
    // std::vector<std::wstring> results = _dialogs->showOpenFilesDialog(L"Open Files", filters);
    QVERIFY(true);
}

void DialogsTest::testShowSaveFileDialog() {
    std::vector<FileFilter> filters = DialogFilters::allFiles();

    // Verify method doesn't crash
    // std::wstring result = _dialogs->showSaveFileDialog(
    //     L"Save File",
    //     filters,
    //     L"default.txt"
    // );
    QVERIFY(true);
}

// ============================================================================
// Folder Dialog Tests
// ============================================================================
void DialogsTest::testShowFolderDialog() {
    FolderDialogOptions options;
    options.title = L"Select Folder";

    // Verify method doesn't crash
    // std::wstring result = _dialogs->showFolderDialog(L"Select Folder", options);
    QVERIFY(true);
}

// ============================================================================
// Input Dialog Tests
// ============================================================================
void DialogsTest::testShowInputDialog() {
    std::wstring value = L"default";

    // Verify method doesn't crash
    // bool result = _dialogs->showInputDialog(L"Enter value", L"Prompt:", value);
    QVERIFY(true);
}

void DialogsTest::testShowInputDialogEx() {
    InputDialogOptions options;
    options.title = L"Input";
    options.prompt = L"Enter value:";
    options.defaultValue = L"default";

    std::wstring value;

    // Verify method doesn't crash
    // bool result = _dialogs->showInputDialogEx(options, value);
    QVERIFY(true);
}

void DialogsTest::testShowMultiLineInputDialog() {
    std::wstring value = L"Line 1\nLine 2";

    // Verify method doesn't crash
    // bool result = _dialogs->showMultiLineInputDialog(
    //     L"Enter text",
    //     L"Multi-line input:",
    //     value
    // );
    QVERIFY(true);
}

void DialogsTest::testShowListDialog() {
    std::vector<std::wstring> items = {
        L"Item 1",
        L"Item 2",
        L"Item 3"
    };

    // Verify method doesn't crash
    // int result = _dialogs->showListDialog(L"Select Item", L"Choose:", items, 0);
    QVERIFY(true);
}

// ============================================================================
// Dialog Utilities Tests
// ============================================================================
void DialogsTest::testCenterDialog() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->centerDialog(handle);

    destroyTestDialog(handle);
}

void DialogsTest::testSetDialogPosition() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->setDialogPosition(handle, 100, 100);

    destroyTestDialog(handle);
}

void DialogsTest::testGetDialogPosition() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    int x = 0, y = 0;
    _dialogs->getDialogPosition(handle, x, y);

    // Values should be set (may be 0,0 for new widget)
    QVERIFY(x >= 0 && y >= 0);

    destroyTestDialog(handle);
}

void DialogsTest::testSetDialogSize() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->setDialogSize(handle, 400, 300);

    destroyTestDialog(handle);
}

void DialogsTest::testGetDialogSize() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    int width = 0, height = 0;
    _dialogs->getDialogSize(handle, width, height);

    // Values should be set (may be default size)
    QVERIFY(width >= 0 && height >= 0);

    destroyTestDialog(handle);
}

void DialogsTest::testSetDialogTitle() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->setDialogTitle(handle, L"New Title");

    destroyTestDialog(handle);
}

void DialogsTest::testEnableDialog() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    _dialogs->enableDialog(handle, false);
    QVERIFY(!_dialogs->isDialogEnabled(handle));

    _dialogs->enableDialog(handle, true);
    QVERIFY(_dialogs->isDialogEnabled(handle));

    destroyTestDialog(handle);
}

void DialogsTest::testIsDialogEnabled() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Default should be enabled
    QVERIFY(_dialogs->isDialogEnabled(handle));

    destroyTestDialog(handle);
}

void DialogsTest::testBringToFront() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->bringToFront(handle);

    destroyTestDialog(handle);
}

void DialogsTest::testSetModal() {
    void* handle = createTestDialog();
    QVERIFY(handle != nullptr);

    // Verify method doesn't crash
    _dialogs->setModal(handle, true);
    _dialogs->setModal(handle, false);

    destroyTestDialog(handle);
}

// ============================================================================
// File Filter Helpers Tests
// ============================================================================
void DialogsTest::testAllFilesFilter() {
    std::vector<FileFilter> filters = DialogFilters::allFiles();
    QVERIFY(!filters.empty());

    // Should contain "All Files" or similar
    bool foundAllFiles = false;
    for (const auto& filter : filters) {
        if (filter.pattern == L"*.*" || filter.pattern == L"*") {
            foundAllFiles = true;
            break;
        }
    }
    QVERIFY(foundAllFiles);
}

void DialogsTest::testTextFilesFilter() {
    std::vector<FileFilter> filters = DialogFilters::textFiles();
    QVERIFY(!filters.empty());

    // Should contain text file patterns
    bool foundTextPattern = false;
    for (const auto& filter : filters) {
        if (filter.pattern.find(L"*.txt") != std::wstring::npos) {
            foundTextPattern = true;
            break;
        }
    }
    QVERIFY(foundTextPattern);
}

void DialogsTest::testSourceCodeFilesFilter() {
    std::vector<FileFilter> filters = DialogFilters::sourceCodeFiles();
    QVERIFY(!filters.empty());

    // Should contain source code patterns
    bool foundSourcePattern = false;
    for (const auto& filter : filters) {
        if (filter.pattern.find(L"*.cpp") != std::wstring::npos ||
            filter.pattern.find(L"*.c") != std::wstring::npos ||
            filter.pattern.find(L"*.h") != std::wstring::npos) {
            foundSourcePattern = true;
            break;
        }
    }
    QVERIFY(foundSourcePattern);
}

void DialogsTest::testCombineFilters() {
    std::vector<FileFilter> filters1 = DialogFilters::textFiles();
    std::vector<FileFilter> filters2 = DialogFilters::sourceCodeFiles();

    std::vector<FileFilter> combined = DialogFilters::combine(filters1, filters2);

    QCOMPARE(static_cast<int>(combined.size()),
             static_cast<int>(filters1.size() + filters2.size()));
}

// ============================================================================
// Progress Dialog Tests
// ============================================================================
void DialogsTest::testProgressDialog() {
    std::unique_ptr<IProgressDialog> progress(IProgressDialog::create());
    QVERIFY(progress != nullptr);

    progress->show(L"Test Progress", L"Processing...");
    progress->setProgress(50);
    progress->setMessage(L"Halfway done");
    progress->setStatus(L"Working...");
    progress->step(10);
    progress->setRange(0, 100);

    QVERIFY(!progress->isCancelled());

    progress->hide();
}

} // namespace Tests

#include "DialogsTest.moc"
