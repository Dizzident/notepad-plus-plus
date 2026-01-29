// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// This file implements Windows-style FileManager interface for file I/O
// On Linux, this is not used - QtCore::BufferManager provides the implementation
#ifndef NPP_LINUX

#include "NppIO.h"
#include "Buffer.h"
#include "ScintillaEditView.h"
#include "Platform/FileSystem.h"
#include "Platform/FileWatcher.h"
#include "Platform/Settings.h"
#include "uchardet.h"

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringEncoder>
#include <QStringDecoder>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>
#include <QProgressDialog>
#include <QApplication>
#include <QRegularExpression>
#include <QSaveFile>
#include <QDebug>

#include <cstring>
#include <algorithm>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// ============================================================================
// Static Global Instance (outside namespace for global access)
// ============================================================================

static QtIO::NppIO* g_nppIO = nullptr;

QtIO::NppIO* getNppIO() {
    return g_nppIO;
}

void setNppIO(QtIO::NppIO* nppIO) {
    g_nppIO = nppIO;
}

namespace QtIO {

// ============================================================================
// Constants
// ============================================================================

namespace {
    const int MAX_FILE_SIZE_FOR_ENCODING_DETECTION = 1024 * 1024; // 1MB
    const int ENCODING_DETECTION_SAMPLE_SIZE = 64 * 1024; // 64KB
    const qint64 LARGE_FILE_THRESHOLD = 200 * 1024 * 1024; // 200MB
    const int DEFAULT_RECENT_FILES_MAX = 10;
    const char* RECENT_FILES_KEY = "RecentFiles";
    const char* SESSION_FILE_EXT = ".session";
    const char* WORKSPACE_FILE_EXT = ".workspace";
}

// ============================================================================
// NppIO Implementation
// ============================================================================

NppIO::NppIO(QObject* parent)
    : QObject(parent)
    , _maxRecentFiles(DEFAULT_RECENT_FILES_MAX)
{
    // Initialize file watcher
    _fileWatcher = new QFileSystemWatcher(this);
    connect(_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &NppIO::onFileChanged);
    connect(_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, &NppIO::onDirectoryChanged);

    // Initialize auto-save timer
    _autoSaveTimer = new QTimer(this);
    connect(_autoSaveTimer, &QTimer::timeout, this, &NppIO::onAutoSaveTimer);

    // Load recent files from settings
    loadRecentFiles();

    ::setNppIO(this);
}

NppIO::~NppIO() {
    stopFileChangeDetection();
    saveRecentFiles();
    ::setNppIO(nullptr);
}

void NppIO::setEditView(ScintillaEditView* editView) {
    _pEditView = editView;
}

void NppIO::setScratchEditView(ScintillaEditView* scratchView) {
    _pScratchEditView = scratchView;
}

// ============================================================================
// File Operations
// ============================================================================

Buffer* NppIO::fileNew() {
    if (!_pScratchEditView) {
        qWarning() << "NppIO::fileNew: No scratch edit view set";
        return nullptr;
    }

    // Create new buffer via FileManager
    BufferID bufferId = MainFileManager.newEmptyDocument();
    if (bufferId == BUFFER_INVALID) {
        return nullptr;
    }

    Buffer* buffer = MainFileManager.getBufferByID(bufferId);

    // Set up the buffer
    buffer->setTabCreatedTimeStringWithCurrentTime();

    emit fileOpened(QString());
    return buffer;
}

OpenFileResult NppIO::fileOpen(const QString& filePath, bool addToRecent, int encoding) {
    OpenFileResult result;

    if (filePath.isEmpty()) {
        result.status = FileStatus::FileNotFound;
        result.errorMessage = tr("File path is empty");
        return result;
    }

    QString normalizedPath = IOUtils::normalizePath(filePath);

    // Check if file already open
    Buffer* existingBuffer = findBufferByFilePath(normalizedPath);
    if (existingBuffer) {
        result.status = FileStatus::Success;
        result.buffer = existingBuffer;
        return result;
    }

    // Check if file exists
    if (!fileExists(normalizedPath)) {
        // Check if it's a glob pattern or directory
        QFileInfo info(normalizedPath);
        if (info.isDir()) {
            // Open all files in directory
            QDir dir(normalizedPath);
            QStringList files = dir.entryList(QDir::Files);
            for (const QString& file : files) {
                fileOpen(dir.absoluteFilePath(file), addToRecent, encoding);
            }
            result.status = FileStatus::Success;
            return result;
        }

        result.status = FileStatus::FileNotFound;
        result.errorMessage = tr("File not found: %1").arg(normalizedPath);
        return result;
    }

    // Check file size
    qint64 fileSize = getFileSize(normalizedPath);
    if (isLargeFile(fileSize)) {
        // Warn about large file
        int ret = QMessageBox::question(nullptr, tr("Large File"),
            tr("The file is larger than 200MB. Opening it may take several minutes.\n"
               "Do you want to open it?"),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            result.status = FileStatus::Cancelled;
            return result;
        }
    }

    // Load the file
    BufferID bufferId = MainFileManager.loadFile(
        IOUtils::qstringToWstring(normalizedPath).c_str(),
        static_cast<Document>(NULL),
        encoding
    );

    if (bufferId == BUFFER_INVALID) {
        result.status = FileStatus::ReadError;
        result.errorMessage = tr("Failed to load file: %1").arg(normalizedPath);
        return result;
    }

    result.buffer = MainFileManager.getBufferByID(bufferId);
    result.status = FileStatus::Success;

    // Add to recent files
    if (addToRecent) {
        addToRecentFiles(normalizedPath);
    }

    // Watch file for changes
    watchFile(normalizedPath);

    emit fileOpened(normalizedPath);
    return result;
}

OpenFileResult NppIO::fileOpenMultiple(const QStringList& filePaths) {
    OpenFileResult result;
    result.status = FileStatus::Success;

    int total = filePaths.size();
    int current = 0;

    for (const QString& path : filePaths) {
        emit progressUpdated((current * 100) / total, tr("Opening %1...").arg(path));

        OpenFileResult singleResult = fileOpen(path, true);
        if (singleResult.status != FileStatus::Success && result.status == FileStatus::Success) {
            result.status = singleResult.status;
            result.errorMessage = singleResult.errorMessage;
        }
        if (singleResult.buffer) {
            result.buffer = singleResult.buffer;
        }

        ++current;
    }

    emit progressUpdated(100, tr("Done"));
    return result;
}

SaveFileResult NppIO::fileSave(Buffer* buffer) {
    SaveFileResult result;

    if (!buffer) {
        result.status = FileStatus::WriteError;
        result.errorMessage = tr("Invalid buffer");
        return result;
    }

    // Check if read-only
    if (buffer->isReadOnly()) {
        result.status = FileStatus::AccessDenied;
        result.errorMessage = tr("File is read-only");
        return result;
    }

    QString filePath = IOUtils::wstringToQstring(buffer->getFullPathName());

    // If untitled, do Save As
    if (buffer->isUntitled()) {
        return fileSaveAs(buffer);
    }

    // Create backup if enabled
    if (_backupEnabled && !buffer->isLargeFile()) {
        if (!createBackup(buffer)) {
            // Ask user if they want to continue without backup
            int ret = QMessageBox::question(nullptr, tr("Backup Failed"),
                tr("The previous version could not be saved to the backup directory.\n"
                   "Do you want to save the file anyway?"),
                QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes) {
                result.status = FileStatus::Cancelled;
                return result;
            }
        }
    }

    // Save the file
    SavingStatus saveStatus = MainFileManager.saveBuffer(
        buffer->getID(),
        IOUtils::qstringToWstring(filePath).c_str(),
        false
    );

    switch (saveStatus) {
        case SaveOK:
            result.status = FileStatus::Success;
            result.newFilePath = filePath;
            emit fileSaved(filePath);
            break;
        case SaveOpenFailed:
            result.status = FileStatus::AccessDenied;
            result.errorMessage = tr("Failed to open file for writing");
            break;
        case SaveWritingFailed:
            result.status = FileStatus::WriteError;
            result.errorMessage = tr("Failed to write file");
            break;
        case NotEnoughRoom:
            result.status = FileStatus::DiskFull;
            result.errorMessage = tr("Not enough disk space");
            break;
        case FullReadOnlySavingForbidden:
            result.status = FileStatus::AccessDenied;
            result.errorMessage = tr("Saving is forbidden in read-only mode");
            break;
        default:
            result.status = FileStatus::WriteError;
            result.errorMessage = tr("Unknown save error");
            break;
    }

    return result;
}

SaveFileResult NppIO::fileSaveAs(Buffer* buffer, const QString& newPath) {
    SaveFileResult result;

    if (!buffer) {
        result.status = FileStatus::WriteError;
        result.errorMessage = tr("Invalid buffer");
        return result;
    }

    QString targetPath = newPath;
    if (targetPath.isEmpty()) {
        // Show save dialog
        QString defaultName = IOUtils::wstringToQstring(buffer->getFileName());
        QString defaultDir = IOUtils::wstringToQstring(
            buffer->isUntitled() ? L"" : buffer->getFullPathName()
        );
        targetPath = showSaveDialog(defaultName, defaultDir);

        if (targetPath.isEmpty()) {
            result.status = FileStatus::Cancelled;
            return result;
        }
    }

    // Normalize path
    targetPath = IOUtils::normalizePath(targetPath);

    // Check if file already exists
    if (fileExists(targetPath)) {
        int ret = QMessageBox::question(nullptr, tr("Confirm Save"),
            tr("The file already exists. Do you want to overwrite it?"),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            result.status = FileStatus::Cancelled;
            return result;
        }
    }

    // Save to new location
    SavingStatus saveStatus = MainFileManager.saveBuffer(
        buffer->getID(),
        IOUtils::qstringToWstring(targetPath).c_str(),
        false
    );

    if (saveStatus == SaveOK) {
        result.status = FileStatus::Success;
        result.newFilePath = targetPath;

        // Update buffer filename
        buffer->setFileName(IOUtils::qstringToWstring(targetPath).c_str());

        // Add to recent files
        addToRecentFiles(targetPath);

        emit fileSaved(targetPath);
    } else {
        result.status = FileStatus::WriteError;
        result.errorMessage = tr("Failed to save file");
    }

    return result;
}

SaveFileResult NppIO::fileSaveCopyAs(Buffer* buffer, const QString& newPath) {
    SaveFileResult result;

    if (!buffer) {
        result.status = FileStatus::WriteError;
        result.errorMessage = tr("Invalid buffer");
        return result;
    }

    QString targetPath = newPath;
    if (targetPath.isEmpty()) {
        result.status = FileStatus::Cancelled;
        return result;
    }

    // Save a copy (doesn't change buffer state)
    SavingStatus saveStatus = MainFileManager.saveBuffer(
        buffer->getID(),
        IOUtils::qstringToWstring(targetPath).c_str(),
        true // isCopy
    );

    if (saveStatus == SaveOK) {
        result.status = FileStatus::Success;
        result.newFilePath = targetPath;
    } else {
        result.status = FileStatus::WriteError;
        result.errorMessage = tr("Failed to save copy");
    }

    return result;
}

bool NppIO::fileClose(Buffer* buffer, bool promptIfUnsaved) {
    if (!buffer) {
        return false;
    }

    // Check if dirty and prompt
    if (promptIfUnsaved && buffer->isDirty()) {
        QString fileName = IOUtils::wstringToQstring(buffer->getFullPathName());
        int ret = QMessageBox::question(nullptr, tr("Save Changes"),
            tr("The file \"%1\" has unsaved changes. Do you want to save them?").arg(fileName),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Save) {
            SaveFileResult result = fileSave(buffer);
            if (result.status != FileStatus::Success) {
                return false; // Save failed, don't close
            }
        } else if (ret == QMessageBox::Cancel) {
            return false; // User cancelled
        }
    }

    QString filePath = IOUtils::wstringToQstring(buffer->getFullPathName());

    // Close the buffer
    MainFileManager.closeBuffer(buffer->getID(), _pEditView);

    // Unwatch file
    unwatchFile(filePath);

    emit fileClosed(filePath);
    return true;
}

bool NppIO::fileCloseAll(bool promptIfUnsaved) {
    // Get all buffers
    size_t bufferCount = MainFileManager.getNbBuffers();

    // First pass: count dirty buffers
    size_t dirtyCount = MainFileManager.getNbDirtyBuffers();

    if (dirtyCount > 0 && promptIfUnsaved) {
        // Ask about saving all
        QString message;
        if (dirtyCount == 1) {
            message = tr("There is 1 file with unsaved changes. Save all changes?");
        } else {
            message = tr("There are %1 files with unsaved changes. Save all changes?").arg(dirtyCount);
        }

        int ret = QMessageBox::question(nullptr, tr("Save All"), message,
            QMessageBox::SaveAll | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Cancel) {
            return false;
        } else if (ret == QMessageBox::SaveAll) {
            saveAllFiles(false); // Don't prompt again
        }
    }

    // Close all buffers
    for (size_t i = 0; i < bufferCount; ++i) {
        Buffer* buffer = MainFileManager.getBufferByIndex(i);
        if (buffer) {
            QString filePath = IOUtils::wstringToQstring(buffer->getFullPathName());
            MainFileManager.closeBuffer(buffer->getID(), _pEditView);
            unwatchFile(filePath);
            emit fileClosed(filePath);
        }
    }

    return true;
}

bool NppIO::fileCloseAllButCurrent(Buffer* currentBuffer) {
    if (!currentBuffer) {
        return false;
    }

    size_t bufferCount = MainFileManager.getNbBuffers();
    BufferID currentId = currentBuffer->getID();

    // Collect buffers to close
    std::vector<Buffer*> toClose;
    for (size_t i = 0; i < bufferCount; ++i) {
        Buffer* buffer = MainFileManager.getBufferByIndex(i);
        if (buffer && buffer->getID() != currentId) {
            toClose.push_back(buffer);
        }
    }

    // Close them
    for (Buffer* buffer : toClose) {
        if (!fileClose(buffer, true)) {
            return false; // User cancelled
        }
    }

    return true;
}

bool NppIO::fileCloseAllButPinned() {
    size_t bufferCount = MainFileManager.getNbBuffers();

    std::vector<Buffer*> toClose;
    for (size_t i = 0; i < bufferCount; ++i) {
        Buffer* buffer = MainFileManager.getBufferByIndex(i);
        if (buffer && !buffer->isPinned()) {
            toClose.push_back(buffer);
        }
    }

    for (Buffer* buffer : toClose) {
        if (!fileClose(buffer, true)) {
            return false;
        }
    }

    return true;
}

bool NppIO::fileCloseAllToLeft(Buffer* buffer) {
    if (!buffer) return false;

    // Get buffer index
    int index = MainFileManager.getBufferIndexByID(buffer->getID());
    if (index < 0) return false;

    // Close all buffers before this one
    for (int i = index - 1; i >= 0; --i) {
        Buffer* buf = MainFileManager.getBufferByIndex(i);
        if (buf && !fileClose(buf, true)) {
            return false;
        }
    }

    return true;
}

bool NppIO::fileCloseAllToRight(Buffer* buffer) {
    if (!buffer) return false;

    // Get buffer index
    int index = MainFileManager.getBufferIndexByID(buffer->getID());
    if (index < 0) return false;

    size_t total = MainFileManager.getNbBuffers();

    // Close all buffers after this one
    for (size_t i = total - 1; i > static_cast<size_t>(index); --i) {
        Buffer* buf = MainFileManager.getBufferByIndex(i);
        if (buf && !fileClose(buf, true)) {
            return false;
        }
    }

    return true;
}

bool NppIO::fileCloseAllUnchanged() {
    size_t bufferCount = MainFileManager.getNbBuffers();

    std::vector<Buffer*> toClose;
    for (size_t i = 0; i < bufferCount; ++i) {
        Buffer* buffer = MainFileManager.getBufferByIndex(i);
        if (buffer && !buffer->isDirty() && !buffer->isUntitled()) {
            toClose.push_back(buffer);
        }
    }

    for (Buffer* buffer : toClose) {
        if (!fileClose(buffer, false)) { // Don't prompt, we know they're not dirty
            return false;
        }
    }

    return true;
}

// ============================================================================
// File Reloading
// ============================================================================

bool NppIO::fileReload(Buffer* buffer, bool alert) {
    if (!buffer) return false;

    if (alert && buffer->isDirty()) {
        int ret = QMessageBox::question(nullptr, tr("Reload File"),
            tr("Are you sure you want to reload the current file and lose the changes?"),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return false;
        }
    }

    BufferID id = buffer->getID();
    return MainFileManager.reloadBuffer(id);
}

bool NppIO::reloadAllFiles() {
    size_t bufferCount = MainFileManager.getNbBuffers();
    bool allSuccess = true;

    for (size_t i = 0; i < bufferCount; ++i) {
        Buffer* buffer = MainFileManager.getBufferByIndex(i);
        if (buffer && !buffer->isUntitled()) {
            if (!fileReload(buffer, false)) {
                allSuccess = false;
            }
        }
    }

    return allSuccess;
}

// ============================================================================
// Recent Files Management
// ============================================================================

void NppIO::addToRecentFiles(const QString& filePath) {
    if (filePath.isEmpty()) return;

    // Remove if already exists
    _recentFiles.removeAll(filePath);

    // Add to front
    _recentFiles.prepend(filePath);

    // Trim to max
    while (_recentFiles.size() > _maxRecentFiles) {
        _recentFiles.removeLast();
    }

    saveRecentFiles();
    updateRecentFilesMenu();
    emit recentFilesChanged();
}

void NppIO::removeFromRecentFiles(const QString& filePath) {
    _recentFiles.removeAll(filePath);
    saveRecentFiles();
    updateRecentFilesMenu();
    emit recentFilesChanged();
}

void NppIO::clearRecentFiles() {
    _recentFiles.clear();
    saveRecentFiles();
    updateRecentFilesMenu();
    emit recentFilesChanged();
}

QStringList NppIO::getRecentFiles() const {
    return _recentFiles;
}

void NppIO::setMaxRecentFiles(int max) {
    _maxRecentFiles = max;
    while (_recentFiles.size() > _maxRecentFiles) {
        _recentFiles.removeLast();
    }
    saveRecentFiles();
}

void NppIO::updateRecentFilesMenu() {
    // This will be connected to the main window to update the menu
    emit recentFilesChanged();
}

void NppIO::loadRecentFiles() {
    // Load from platform settings
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();
    std::vector<std::wstring> recent = settings.getRecentFiles();

    _recentFiles.clear();
    for (const auto& path : recent) {
        _recentFiles.append(IOUtils::wstringToQstring(path));
    }
}

void NppIO::saveRecentFiles() {
    // Save to platform settings via individual adds
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();
    settings.clearRecentFiles();
    for (const QString& path : _recentFiles) {
        settings.addToRecentFiles(IOUtils::qstringToWstring(path));
    }
}

// ============================================================================
// Encoding Operations
// ============================================================================

EncodingDetectionResult NppIO::detectEncoding(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return EncodingDetectionResult();
    }

    QByteArray data = file.read(ENCODING_DETECTION_SAMPLE_SIZE);
    file.close();

    return detectEncoding(data);
}

EncodingDetectionResult NppIO::detectEncoding(const QByteArray& data) {
    EncodingDetectionResult result;

    if (data.isEmpty()) {
        result.encoding = "UTF-8";
        result.confidence = 100;
        return result;
    }

    // Check for BOM
    if (data.size() >= 3) {
        if (static_cast<unsigned char>(data[0]) == 0xEF &&
            static_cast<unsigned char>(data[1]) == 0xBB &&
            static_cast<unsigned char>(data[2]) == 0xBF) {
            result.encoding = "UTF-8";
            result.hasBOM = true;
            result.confidence = 100;
            return result;
        }
    }

    if (data.size() >= 2) {
        if (static_cast<unsigned char>(data[0]) == 0xFE &&
            static_cast<unsigned char>(data[1]) == 0xFF) {
            result.encoding = "UTF-16 BE";
            result.hasBOM = true;
            result.confidence = 100;
            return result;
        }
        if (static_cast<unsigned char>(data[0]) == 0xFF &&
            static_cast<unsigned char>(data[1]) == 0xFE) {
            result.encoding = "UTF-16 LE";
            result.hasBOM = true;
            result.confidence = 100;
            return result;
        }
    }

    // Use uchardet for detection
    uchardet_t ud = uchardet_new();
    uchardet_handle_data(ud, data.constData(), data.size());
    uchardet_data_end(ud);

    const char* charset = uchardet_get_charset(ud);
    if (charset && strlen(charset) > 0) {
        result.encoding = QString::fromUtf8(charset);
        result.confidence = 80; // uchardet doesn't give confidence directly
    } else {
        result.encoding = "UTF-8"; // Default fallback
        result.confidence = 50;
    }

    uchardet_delete(ud);

    // Check if it's actually ASCII
    bool allAscii = true;
    for (int i = 0; i < qMin(data.size(), 1000); ++i) {
        if (static_cast<unsigned char>(data[i]) > 127) {
            allAscii = false;
            break;
        }
    }
    if (allAscii) {
        result.encoding = "ASCII";
        result.confidence = 100;
    }

    return result;
}

QByteArray NppIO::convertEncoding(const QByteArray& data, const QString& fromEncoding, const QString& toEncoding) {
    if (fromEncoding == toEncoding) {
        return data;
    }

    auto fromEnc = QStringDecoder::encodingForName(fromEncoding.toUtf8().constData());
    auto toEnc = QStringEncoder::encodingForName(toEncoding.toUtf8().constData());

    if (!fromEnc.has_value() || !toEnc.has_value()) {
        return data; // Can't convert
    }

    QStringDecoder decoder(fromEnc.value());
    QStringEncoder encoder(toEnc.value());

    QString unicode = decoder.decode(data);
    return encoder.encode(unicode);
}

QString NppIO::getEncodingName(int encoding) const {
    // Map encoding IDs to names
    switch (encoding) {
        case 0: return "UTF-8";
        case 1: return "UTF-16 LE";
        case 2: return "UTF-16 BE";
        case 3: return "UTF-32 LE";
        case 4: return "UTF-32 BE";
        case 5: return "ISO-8859-1";
        case 6: return "ISO-8859-2";
        case 7: return "ISO-8859-3";
        case 8: return "ISO-8859-4";
        case 9: return "ISO-8859-5";
        case 10: return "ISO-8859-6";
        case 11: return "ISO-8859-7";
        case 12: return "ISO-8859-8";
        case 13: return "ISO-8859-9";
        case 14: return "ISO-8859-10";
        case 15: return "ISO-8859-11";
        case 16: return "ISO-8859-12";
        case 17: return "ISO-8859-13";
        case 18: return "ISO-8859-14";
        case 19: return "ISO-8859-15";
        case 20: return "ISO-8859-16";
        case 21: return "Windows-1250";
        case 22: return "Windows-1251";
        case 23: return "Windows-1252";
        case 24: return "Windows-1253";
        case 25: return "Windows-1254";
        case 26: return "Windows-1255";
        case 27: return "Windows-1256";
        case 28: return "Windows-1257";
        case 29: return "Windows-1258";
        case 30: return "Big5";
        case 31: return "GB2312";
        case 32: return "Shift_JIS";
        case 33: return "EUC-JP";
        case 34: return "EUC-KR";
        default: return "UTF-8";
    }
}

int NppIO::getEncodingFromName(const QString& name) const {
    // Reverse mapping
    if (name == "UTF-8") return 0;
    if (name == "UTF-16 LE") return 1;
    if (name == "UTF-16 BE") return 2;
    if (name == "UTF-32 LE") return 3;
    if (name == "UTF-32 BE") return 4;
    if (name == "ISO-8859-1") return 5;
    if (name == "Windows-1252") return 23;
    if (name == "Windows-1251") return 22;
    if (name == "Big5") return 30;
    if (name == "GB2312") return 31;
    if (name == "Shift_JIS") return 32;
    if (name == "EUC-JP") return 33;
    if (name == "EUC-KR") return 34;
    return 0; // Default to UTF-8
}

// ============================================================================
// Line Ending Operations
// ============================================================================

LineEnding NppIO::detectLineEnding(const QByteArray& data) {
    return detectLineEndingFromContent(data.constData(), data.size());
}

LineEnding NppIO::detectLineEndingFromContent(const char* data, size_t length) {
    if (!data || length == 0) {
        return LineEnding::Unix; // Default
    }

    bool hasCRLF = false;
    bool hasLF = false;
    bool hasCR = false;

    for (size_t i = 0; i < length; ++i) {
        if (data[i] == '\r') {
            if (i + 1 < length && data[i + 1] == '\n') {
                hasCRLF = true;
                ++i; // Skip the LF
            } else {
                hasCR = true;
            }
        } else if (data[i] == '\n') {
            hasLF = true;
        }
    }

    if (hasCRLF && !hasLF && !hasCR) {
        return LineEnding::Windows;
    } else if (hasLF && !hasCRLF && !hasCR) {
        return LineEnding::Unix;
    } else if (hasCR && !hasCRLF && !hasLF) {
        return LineEnding::ClassicMac;
    } else if (hasCRLF || hasLF || hasCR) {
        return LineEnding::Mixed;
    }

    return LineEnding::Unix; // Default
}

QByteArray NppIO::convertLineEnding(const QByteArray& data, LineEnding targetEnding) {
    if (data.isEmpty()) return data;

    QByteArray result;
    result.reserve(data.size());

    const char* crlf = "\r\n";
    const char* lf = "\n";
    const char* cr = "\r";

    const char* target = nullptr;
    int targetLen = 0;

    switch (targetEnding) {
        case LineEnding::Windows:
            target = crlf;
            targetLen = 2;
            break;
        case LineEnding::Unix:
            target = lf;
            targetLen = 1;
            break;
        case LineEnding::ClassicMac:
            target = cr;
            targetLen = 1;
            break;
        default:
            return data; // Mixed - don't convert
    }

    // Normalize to LF first, then convert to target
    for (int i = 0; i < data.size(); ++i) {
        if (data[i] == '\r') {
            if (i + 1 < data.size() && data[i + 1] == '\n') {
                // CRLF -> target
                result.append(target, targetLen);
                ++i; // Skip LF
            } else {
                // CR -> target
                result.append(target, targetLen);
            }
        } else if (data[i] == '\n') {
            // LF -> target
            result.append(target, targetLen);
        } else {
            result.append(data[i]);
        }
    }

    return result;
}

QString NppIO::lineEndingToString(LineEnding ending) const {
    switch (ending) {
        case LineEnding::Windows: return "Windows (CRLF)";
        case LineEnding::Unix: return "Unix (LF)";
        case LineEnding::ClassicMac: return "Classic Mac (CR)";
        case LineEnding::Mixed: return "Mixed";
        default: return "Unknown";
    }
}

LineEnding NppIO::stringToLineEnding(const QString& str) const {
    if (str.contains("Windows") || str.contains("CRLF")) {
        return LineEnding::Windows;
    } else if (str.contains("Classic") || str.contains("CR only")) {
        return LineEnding::ClassicMac;
    } else if (str.contains("Mixed")) {
        return LineEnding::Mixed;
    } else {
        return LineEnding::Unix;
    }
}

// ============================================================================
// File Information
// ============================================================================

FileInfo NppIO::getFileInfo(const QString& filePath) {
    FileInfo info;
    info.filePath = filePath;

    QFileInfo qfileInfo(filePath);
    if (!qfileInfo.exists()) {
        info.exists = false;
        return info;
    }

    info.exists = true;
    info.fileName = qfileInfo.fileName();
    info.fileSize = qfileInfo.size();
    info.modifiedTime = qfileInfo.lastModified();
    info.isReadOnly = !qfileInfo.isWritable();
    info.isHidden = qfileInfo.isHidden();

    // Detect encoding
    EncodingDetectionResult encResult = detectEncoding(filePath);
    info.encoding = encResult.encoding;

    // Detect line ending
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray sample = file.read(4096);
        file.close();
        info.lineEnding = detectLineEnding(sample);
    }

    return info;
}

bool NppIO::fileExists(const QString& filePath) const {
    return QFile::exists(filePath);
}

bool NppIO::isFileReadOnly(const QString& filePath) const {
    QFileInfo info(filePath);
    return info.exists() && !info.isWritable();
}

bool NppIO::isFileHidden(const QString& filePath) const {
    QFileInfo info(filePath);
    return info.isHidden();
}

qint64 NppIO::getFileSize(const QString& filePath) const {
    QFileInfo info(filePath);
    return info.size();
}

QDateTime NppIO::getFileModifiedTime(const QString& filePath) const {
    QFileInfo info(filePath);
    return info.lastModified();
}

// ============================================================================
// Backup Operations
// ============================================================================

void NppIO::setBackupEnabled(bool enabled) {
    _backupEnabled = enabled;
}

void NppIO::setBackupDirectory(const QString& dir) {
    _backupDir = dir;
}

void NppIO::setBackupFeature(BackupFeature feature) {
    _backupFeature = feature;
}

bool NppIO::createBackup(const QString& filePath) {
    if (_backupFeature == BackupFeature::None) {
        return true; // No backup needed
    }

    if (_backupFeature == BackupFeature::Simple) {
        return performSimpleBackup(filePath);
    } else {
        return performVerboseBackup(filePath);
    }
}

bool NppIO::createBackup(Buffer* buffer) {
    if (!buffer) return false;
    QString filePath = IOUtils::wstringToQstring(buffer->getFullPathName());
    return createBackup(filePath);
}

bool NppIO::performSimpleBackup(const QString& filePath) {
    QString backupPath = getBackupFilePath(filePath);
    if (backupPath.isEmpty()) return false;

    // Ensure backup directory exists
    if (!ensureBackupDirectoryExists()) {
        return false;
    }

    // Copy file
    return QFile::copy(filePath, backupPath);
}

bool NppIO::performVerboseBackup(const QString& filePath) {
    QString backupPath = getBackupFilePath(filePath);
    if (backupPath.isEmpty()) return false;

    // Ensure backup directory exists
    if (!ensureBackupDirectoryExists()) {
        return false;
    }

    // Copy file
    return QFile::copy(filePath, backupPath);
}

QString NppIO::getBackupFilePath(const QString& originalPath) const {
    QFileInfo info(originalPath);
    QString backupDir;

    if (!_backupDir.isEmpty()) {
        backupDir = _backupDir;
    } else {
        backupDir = info.path() + "/nppBackup";
    }

    QString fileName = info.fileName();

    if (_backupFeature == BackupFeature::Simple) {
        return backupDir + "/" + fileName + ".bak";
    } else if (_backupFeature == BackupFeature::Verbose) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmmss");
        return backupDir + "/" + fileName + "." + timestamp + ".bak";
    }

    return QString();
}

bool NppIO::ensureBackupDirectoryExists() {
    QString backupDir;

    if (!_backupDir.isEmpty()) {
        backupDir = _backupDir;
    }

    if (!backupDir.isEmpty()) {
        QDir dir;
        return dir.mkpath(backupDir);
    }

    return true;
}

// ============================================================================
// File Change Detection
// ============================================================================

void NppIO::startFileChangeDetection() {
    // File watcher is already set up in constructor
}

void NppIO::stopFileChangeDetection() {
    if (_fileWatcher) {
        _fileWatcher->removePaths(_fileWatcher->files());
        _fileWatcher->removePaths(_fileWatcher->directories());
    }
    _watchedFiles.clear();
    _fileLastModified.clear();
    _fileLastSize.clear();
}

void NppIO::watchFile(const QString& filePath) {
    if (filePath.isEmpty()) return;

    if (!_watchedFiles.contains(filePath)) {
        _fileWatcher->addPath(filePath);
        _watchedFiles.append(filePath);

        QFileInfo info(filePath);
        _fileLastModified[filePath] = info.lastModified();
        _fileLastSize[filePath] = info.size();
    }
}

void NppIO::unwatchFile(const QString& filePath) {
    if (_watchedFiles.contains(filePath)) {
        _fileWatcher->removePath(filePath);
        _watchedFiles.removeAll(filePath);
        _fileLastModified.remove(filePath);
        _fileLastSize.remove(filePath);
    }
}

void NppIO::onFileChanged(const QString& filePath) {
    QFileInfo info(filePath);

    if (!info.exists()) {
        // File was deleted
        emit fileDeletedExternally(filePath);
        unwatchFile(filePath);
        return;
    }

    QDateTime lastModified = info.lastModified();
    qint64 size = info.size();

    // Check if actually changed
    if (_fileLastModified.contains(filePath)) {
        if (_fileLastModified[filePath] == lastModified && _fileLastSize[filePath] == size) {
            return; // No actual change
        }
    }

    // Update stored values
    _fileLastModified[filePath] = lastModified;
    _fileLastSize[filePath] = size;

    emit fileModifiedExternally(filePath);
}

void NppIO::onDirectoryChanged(const QString& path) {
    // Check all watched files in this directory
    for (const QString& filePath : _watchedFiles) {
        if (filePath.startsWith(path)) {
            onFileChanged(filePath);
        }
    }
}

// ============================================================================
// Batch Operations
// ============================================================================

bool NppIO::saveAllFiles(bool promptIfUnsaved) {
    size_t bufferCount = MainFileManager.getNbBuffers();
    int savedCount = 0;

    for (size_t i = 0; i < bufferCount; ++i) {
        Buffer* buffer = MainFileManager.getBufferByIndex(i);
        if (buffer && buffer->isDirty() && !buffer->isReadOnly()) {
            SaveFileResult result = fileSave(buffer);
            if (result.status == FileStatus::Success) {
                ++savedCount;
            }

            emit progressUpdated(
                static_cast<int>((i * 100) / bufferCount),
                tr("Saving %1...").arg(IOUtils::wstringToQstring(buffer->getFileName()))
            );
        }
    }

    emit progressUpdated(100, tr("Saved %1 files").arg(savedCount));
    return true;
}

bool NppIO::closeAllFiles(bool promptIfUnsaved) {
    return fileCloseAll(promptIfUnsaved);
}

// ============================================================================
// Auto-save
// ============================================================================

void NppIO::setAutoSaveEnabled(bool enabled) {
    _autoSaveEnabled = enabled;
    if (_autoSaveEnabled) {
        _autoSaveTimer->start(_autoSaveInterval * 60 * 1000);
    } else {
        _autoSaveTimer->stop();
    }
}

void NppIO::setAutoSaveInterval(int minutes) {
    _autoSaveInterval = minutes;
    if (_autoSaveEnabled) {
        _autoSaveTimer->start(_autoSaveInterval * 60 * 1000);
    }
}

void NppIO::doAutoSave() {
    size_t bufferCount = MainFileManager.getNbBuffers();

    for (size_t i = 0; i < bufferCount; ++i) {
        Buffer* buffer = MainFileManager.getBufferByIndex(i);
        if (buffer && buffer->isDirty() && !buffer->isUntitled() && !buffer->isReadOnly()) {
            fileSave(buffer);
        }
    }
}

void NppIO::onAutoSaveTimer() {
    if (_autoSaveEnabled) {
        doAutoSave();
    }
}

// ============================================================================
// File Rename and Delete
// ============================================================================

bool NppIO::fileRename(Buffer* buffer, const QString& newName) {
    if (!buffer) return false;

    QString oldPath = IOUtils::wstringToQstring(buffer->getFullPathName());
    QString newPath = IOUtils::getDirectory(oldPath) + "/" + newName;

    if (QFile::rename(oldPath, newPath)) {
        buffer->setFileName(IOUtils::qstringToWstring(newPath).c_str());
        return true;
    }

    return false;
}

bool NppIO::fileDelete(Buffer* buffer) {
    if (!buffer) return false;

    QString filePath = IOUtils::wstringToQstring(buffer->getFullPathName());

    int ret = QMessageBox::question(nullptr, tr("Delete File"),
        tr("Are you sure you want to delete \"%1\"?").arg(filePath),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) {
        return false;
    }

    if (QFile::remove(filePath)) {
        fileClose(buffer, false);
        return true;
    }

    return false;
}

// ============================================================================
// Session Operations
// ============================================================================

bool NppIO::isFileSession(const QString& filePath) const {
    // Check extension
    if (filePath.endsWith(QLatin1String(SESSION_FILE_EXT), Qt::CaseInsensitive)) {
        return true;
    }

    // Check user-defined session extension from settings
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();
    QString customExt = QString::fromStdWString(
        settings.readString(L"Session", L"FileExt", L"")
    );

    if (!customExt.isEmpty()) {
        if (!customExt.startsWith(".")) {
            customExt = "." + customExt;
        }
        if (filePath.endsWith(customExt, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

bool NppIO::isFileWorkspace(const QString& filePath) const {
    // Check extension
    if (filePath.endsWith(QLatin1String(WORKSPACE_FILE_EXT), Qt::CaseInsensitive)) {
        return true;
    }

    // Check user-defined workspace extension from settings
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();
    QString customExt = QString::fromStdWString(
        settings.readString(L"Workspace", L"FileExt", L"")
    );

    if (!customExt.isEmpty()) {
        if (!customExt.startsWith(".")) {
            customExt = "." + customExt;
        }
        if (filePath.endsWith(customExt, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

// ============================================================================
// Utility
// ============================================================================

bool NppIO::isLargeFile(qint64 fileSize) const {
    return fileSize > LARGE_FILE_THRESHOLD;
}

bool NppIO::promptForSave(const QString& fileName, bool multipleFiles) {
    QString message;
    if (multipleFiles) {
        message = tr("There are multiple files with unsaved changes. Save all changes?");
    } else {
        message = tr("The file \"%1\" has unsaved changes. Save changes?").arg(fileName);
    }

    int ret = QMessageBox::question(nullptr, tr("Save Changes"), message,
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    return ret == QMessageBox::Save;
}

Buffer* NppIO::findBufferByFilePath(const QString& filePath) {
    std::wstring wpath = IOUtils::qstringToWstring(filePath);
    BufferID id = MainFileManager.getBufferFromName(wpath.c_str());
    if (id != BUFFER_INVALID) {
        return MainFileManager.getBufferByID(id);
    }
    return nullptr;
}

QString NppIO::showSaveDialog(const QString& defaultName, const QString& defaultDir) {
    QString dir = defaultDir;
    if (dir.isEmpty()) {
        dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    QString filePath = QFileDialog::getSaveFileName(nullptr, tr("Save File"),
        dir + "/" + defaultName);

    return filePath;
}

QStringList NppIO::showOpenDialog(bool multiple) {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    if (multiple) {
        return QFileDialog::getOpenFileNames(nullptr, tr("Open Files"), dir);
    } else {
        QString file = QFileDialog::getOpenFileName(nullptr, tr("Open File"), dir);
        if (file.isEmpty()) {
            return QStringList();
        }
        return QStringList(file);
    }
}

// ============================================================================
// IOUtils Implementation
// ============================================================================

namespace IOUtils {

std::wstring qstringToWstring(const QString& str) {
    return str.toStdWString();
}

QString wstringToQstring(const std::wstring& str) {
    return QString::fromStdWString(str);
}

QString byteArrayToQString(const QByteArray& data, const QString& encoding) {
    auto enc = QStringDecoder::encodingForName(encoding.toUtf8().constData())
                   .value_or(QStringDecoder::Utf8);
    QStringDecoder decoder(enc);
    return decoder.decode(data);
}

QByteArray qstringToByteArray(const QString& str, const QString& encoding) {
    auto enc = QStringEncoder::encodingForName(encoding.toUtf8().constData())
                   .value_or(QStringEncoder::Utf8);
    QStringEncoder encoder(enc);
    return encoder.encode(str);
}

QString getFileName(const QString& filePath) {
    return QFileInfo(filePath).fileName();
}

QString getDirectory(const QString& filePath) {
    return QFileInfo(filePath).path();
}

QString getFileExtension(const QString& filePath) {
    return QFileInfo(filePath).suffix();
}

bool isAbsolutePath(const QString& path) {
    return QFileInfo(path).isAbsolute();
}

QString normalizePath(const QString& path) {
    return QFileInfo(path).canonicalFilePath();
}

QString getTempFilePath(const QString& prefix) {
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString fileName = prefix + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".tmp";
    return tempDir + "/" + fileName;
}

bool copyFileWithProgress(const QString& source, const QString& dest,
                          std::function<void(int percent)> progress) {
    QFile srcFile(source);
    QFile dstFile(dest);

    if (!srcFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    if (!dstFile.open(QIODevice::WriteOnly)) {
        return false;
    }

    qint64 totalSize = srcFile.size();
    qint64 copied = 0;
    const qint64 bufferSize = 64 * 1024; // 64KB chunks

    while (!srcFile.atEnd()) {
        QByteArray buffer = srcFile.read(bufferSize);
        dstFile.write(buffer);
        copied += buffer.size();

        if (progress && totalSize > 0) {
            progress(static_cast<int>((copied * 100) / totalSize));
        }
    }

    return true;
}

bool moveFile(const QString& source, const QString& dest) {
    // Try rename first (fast)
    if (QFile::rename(source, dest)) {
        return true;
    }

    // Fall back to copy + delete
    if (copyFileWithProgress(source, dest)) {
        return QFile::remove(source);
    }

    return false;
}

bool isBinaryFile(const QByteArray& data, int checkLength) {
    int len = qMin(data.size(), checkLength);
    int nullCount = 0;

    for (int i = 0; i < len; ++i) {
        char c = data[i];
        // Check for null bytes (common in binary files)
        if (c == '\0') {
            nullCount++;
            // If more than 1% null bytes, consider it binary
            if (nullCount > len / 100) {
                return true;
            }
        }
    }

    return false;
}

ReadFileResult readFileWithEncoding(const QString& filePath, const QString& suggestedEncoding) {
    ReadFileResult result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.success = false;
        result.error = QObject::tr("Cannot open file: %1").arg(filePath);
        return result;
    }

    result.content = file.readAll();
    file.close();

    // Detect encoding
    if (!suggestedEncoding.isEmpty()) {
        result.encoding = suggestedEncoding;
    } else {
        // Use NppIO's detection
        NppIO* io = getNppIO();
        if (io) {
            EncodingDetectionResult encResult = io->detectEncoding(result.content);
            result.encoding = encResult.encoding;
            result.hasBOM = encResult.hasBOM;
        } else {
            result.encoding = "UTF-8";
        }
    }

    result.success = true;
    return result;
}

} // namespace IOUtils

} // namespace QtIO

#else // NPP_LINUX

// ============================================================================
// Linux Stub Implementations
// ============================================================================
// These stubs are required because the MOC (Meta-Object Compiler) generates
// code that references these methods, even though the full implementation
// is excluded on Linux. QtCore::BufferManager provides the actual I/O
// implementation on Linux.
// ============================================================================

#include "NppIO.h"
#include <QTimer>
#include <QFileSystemWatcher>

static QtIO::NppIO* g_nppIO = nullptr;

QtIO::NppIO* getNppIO() {
    return g_nppIO;
}

void setNppIO(QtIO::NppIO* nppIO) {
    g_nppIO = nppIO;
}

namespace QtIO {

NppIO::NppIO(QObject* parent)
    : QObject(parent)
    , _maxRecentFiles(10)
{
    // Initialize file watcher
    _fileWatcher = new QFileSystemWatcher(this);
    connect(_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &NppIO::onFileChanged);
    connect(_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, &NppIO::onDirectoryChanged);

    // Initialize auto-save timer
    _autoSaveTimer = new QTimer(this);
    connect(_autoSaveTimer, &QTimer::timeout, this, &NppIO::onAutoSaveTimer);

    ::setNppIO(this);
}

NppIO::~NppIO() {
    stopFileChangeDetection();
    ::setNppIO(nullptr);
}

void NppIO::onFileChanged(const QString& filePath) {
    // Stub implementation - file change detection handled by BufferManager on Linux
    Q_UNUSED(filePath)
}

void NppIO::onDirectoryChanged(const QString& path) {
    // Stub implementation - directory change detection handled by BufferManager on Linux
    Q_UNUSED(path)
}

void NppIO::onAutoSaveTimer() {
    // Stub implementation - auto-save handled by BufferManager on Linux
}

void NppIO::stopFileChangeDetection() {
    // Stub implementation - file watching handled by BufferManager on Linux
}

} // namespace QtIO

#endif // NPP_LINUX
