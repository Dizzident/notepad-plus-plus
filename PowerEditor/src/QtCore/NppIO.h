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

#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QByteArray>
#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QMap>
#include <vector>
#include <functional>
#include <memory>

// Include Buffer header from ScintillaComponent which provides the Buffer type alias
// On Linux: using Buffer = QtCore::Buffer;
// On Windows: class Buffer;
#include "../ScintillaComponent/Buffer.h"

// Forward declarations
class ScintillaEditView;
class QFile;
class QTextCodec;

namespace QtIO {

// ============================================================================
// Enums
// ============================================================================

enum class LineEnding {
    Windows,    // CRLF
    Unix,       // LF
    ClassicMac, // CR
    Mixed
};

enum class FileStatus {
    Success,
    Cancelled,
    ReadError,
    WriteError,
    EncodingError,
    AccessDenied,
    FileNotFound,
    DiskFull
};

enum class BackupFeature {
    None,       // No backup
    Simple,     // .bak extension
    Verbose     // Timestamped backup
};

// ============================================================================
// Structs
// ============================================================================

struct FileInfo {
    QString filePath;
    QString fileName;
    qint64 fileSize = 0;
    QDateTime modifiedTime;
    QString encoding;
    LineEnding lineEnding = LineEnding::Unix;
    bool isReadOnly = false;
    bool isHidden = false;
    bool exists = false;
};

struct OpenFileResult {
    FileStatus status = FileStatus::Success;
    Buffer* buffer = nullptr;
    QString errorMessage;
};

struct SaveFileResult {
    FileStatus status = FileStatus::Success;
    QString errorMessage;
    QString newFilePath;
};

struct EncodingDetectionResult {
    QString encoding;
    bool hasBOM = false;
    int confidence = 0;  // 0-100
};

// ============================================================================
// NppIO Class
// ============================================================================

class NppIO : public QObject {
    Q_OBJECT

public:
    explicit NppIO(QObject* parent = nullptr);
    ~NppIO() override;

    // Non-copyable
    NppIO(const NppIO&) = delete;
    NppIO& operator=(const NppIO&) = delete;

    // ------------------------------------------------------------------------
    // Initialization
    // ------------------------------------------------------------------------
    void setEditView(ScintillaEditView* editView);
    void setScratchEditView(ScintillaEditView* scratchView);

    // ------------------------------------------------------------------------
    // File Operations
    // ------------------------------------------------------------------------
    Buffer* fileNew();
    OpenFileResult fileOpen(const QString& filePath, bool addToRecent = true, int encoding = -1);
    OpenFileResult fileOpenMultiple(const QStringList& filePaths);
    SaveFileResult fileSave(Buffer* buffer);
    SaveFileResult fileSaveAs(Buffer* buffer, const QString& newPath = QString());
    SaveFileResult fileSaveCopyAs(Buffer* buffer, const QString& newPath);
    bool fileClose(Buffer* buffer, bool promptIfUnsaved = true);
    bool fileCloseAll(bool promptIfUnsaved = true);
    bool fileCloseAllButCurrent(Buffer* currentBuffer);
    bool fileCloseAllButPinned();
    bool fileCloseAllToLeft(Buffer* buffer);
    bool fileCloseAllToRight(Buffer* buffer);
    bool fileCloseAllUnchanged();

    // ------------------------------------------------------------------------
    // File Reloading
    // ------------------------------------------------------------------------
    bool fileReload(Buffer* buffer, bool alert = true);
    bool reloadAllFiles();

    // ------------------------------------------------------------------------
    // Recent Files Management
    // ------------------------------------------------------------------------
    void addToRecentFiles(const QString& filePath);
    void removeFromRecentFiles(const QString& filePath);
    void clearRecentFiles();
    QStringList getRecentFiles() const;
    void setMaxRecentFiles(int max);
    int getMaxRecentFiles() const { return _maxRecentFiles; }
    void updateRecentFilesMenu();

    // ------------------------------------------------------------------------
    // Encoding Operations
    // ------------------------------------------------------------------------
    EncodingDetectionResult detectEncoding(const QString& filePath);
    EncodingDetectionResult detectEncoding(const QByteArray& data);
    QByteArray convertEncoding(const QByteArray& data, const QString& fromEncoding, const QString& toEncoding);
    QString getEncodingName(int encoding) const;
    int getEncodingFromName(const QString& name) const;

    // ------------------------------------------------------------------------
    // Line Ending Operations
    // ------------------------------------------------------------------------
    LineEnding detectLineEnding(const QByteArray& data);
    QByteArray convertLineEnding(const QByteArray& data, LineEnding targetEnding);
    QString lineEndingToString(LineEnding ending) const;
    LineEnding stringToLineEnding(const QString& str) const;

    // ------------------------------------------------------------------------
    // File Information
    // ------------------------------------------------------------------------
    FileInfo getFileInfo(const QString& filePath);
    bool fileExists(const QString& filePath) const;
    bool isFileReadOnly(const QString& filePath) const;
    bool isFileHidden(const QString& filePath) const;
    qint64 getFileSize(const QString& filePath) const;
    QDateTime getFileModifiedTime(const QString& filePath) const;

    // ------------------------------------------------------------------------
    // Backup Operations
    // ------------------------------------------------------------------------
    void setBackupEnabled(bool enabled);
    void setBackupDirectory(const QString& dir);
    void setBackupFeature(BackupFeature feature);
    bool createBackup(const QString& filePath);
    bool createBackup(Buffer* buffer);
    QString getBackupFilePath(const QString& originalPath) const;

    // ------------------------------------------------------------------------
    // File Change Detection
    // ------------------------------------------------------------------------
    void startFileChangeDetection();
    void stopFileChangeDetection();
    void watchFile(const QString& filePath);
    void unwatchFile(const QString& filePath);

    // ------------------------------------------------------------------------
    // Batch Operations with Progress
    // ------------------------------------------------------------------------
    bool saveAllFiles(bool promptIfUnsaved = true);
    bool closeAllFiles(bool promptIfUnsaved = true);

    // ------------------------------------------------------------------------
    // Auto-save
    // ------------------------------------------------------------------------
    void setAutoSaveEnabled(bool enabled);
    void setAutoSaveInterval(int minutes);
    bool isAutoSaveEnabled() const { return _autoSaveEnabled; }
    int getAutoSaveInterval() const { return _autoSaveInterval; }
    void doAutoSave();

    // ------------------------------------------------------------------------
    // File Rename and Delete
    // ------------------------------------------------------------------------
    bool fileRename(Buffer* buffer, const QString& newName);
    bool fileDelete(Buffer* buffer);

    // ------------------------------------------------------------------------
    // Session Operations
    // ------------------------------------------------------------------------
    bool isFileSession(const QString& filePath) const;
    bool isFileWorkspace(const QString& filePath) const;

    // ------------------------------------------------------------------------
    // Utility
    // ------------------------------------------------------------------------
    bool isLargeFile(qint64 fileSize) const;
    bool promptForSave(const QString& fileName, bool multipleFiles = false);

signals:
    void fileOpened(const QString& filePath);
    void fileSaved(const QString& filePath);
    void fileClosed(const QString& filePath);
    void fileModifiedExternally(const QString& filePath);
    void fileDeletedExternally(const QString& filePath);
    void recentFilesChanged();
    void progressUpdated(int percent, const QString& message);
    void encodingDetected(const QString& encoding, bool hasBOM);
    void lineEndingDetected(QtIO::LineEnding ending);

private slots:
    void onFileChanged(const QString& filePath);
    void onDirectoryChanged(const QString& path);
    void onAutoSaveTimer();

private:
    // ------------------------------------------------------------------------
    // Internal Helpers
    // ------------------------------------------------------------------------
    bool loadFileIntoBuffer(const QString& filePath, Buffer* buffer, int encoding = -1);
    bool saveBufferToFile(Buffer* buffer, const QString& filePath, bool isCopy = false);
    QByteArray readFileContent(const QString& filePath, bool& success, QString& error);
    bool writeFileContent(const QString& filePath, const QByteArray& content, QString& error);

    // Encoding helpers
    QByteArray decodeContent(const QByteArray& data, const EncodingDetectionResult& encoding);
    QByteArray encodeContent(const QByteArray& data, int encoding, bool addBOM);

    // Line ending helpers
    LineEnding detectLineEndingFromContent(const char* data, size_t length);
    QByteArray convertToLineEnding(const QByteArray& data, LineEnding ending);

    // Backup helpers
    bool performSimpleBackup(const QString& filePath);
    bool performVerboseBackup(const QString& filePath);
    bool ensureBackupDirectoryExists();

    // File watching helpers
    void setupFileWatcher();
    void checkFileState(Buffer* buffer);

    // Dialog helpers
    QString showSaveDialog(const QString& defaultName, const QString& defaultDir);
    QStringList showOpenDialog(bool multiple = true);

    // Buffer management
    Buffer* findBufferByFilePath(const QString& filePath);
    bool isBufferDirty(Buffer* buffer);
    bool isBufferUntitled(Buffer* buffer);

    // Recent files persistence
    void loadRecentFiles();
    void saveRecentFiles();

    // ------------------------------------------------------------------------
    // Member Variables
    // ------------------------------------------------------------------------
    ScintillaEditView* _pEditView = nullptr;
    ScintillaEditView* _pScratchEditView = nullptr;

    // Recent files
    QStringList _recentFiles;
    int _maxRecentFiles = 10;

    // Backup settings
    bool _backupEnabled = true;
    BackupFeature _backupFeature = BackupFeature::Simple;
    QString _backupDir;

    // Auto-save settings
    bool _autoSaveEnabled = false;
    int _autoSaveInterval = 7; // minutes
    QTimer* _autoSaveTimer = nullptr;

    // File watching
    QFileSystemWatcher* _fileWatcher = nullptr;
    QMap<QString, QDateTime> _fileLastModified;
    QMap<QString, qint64> _fileLastSize;
    QStringList _watchedFiles;

    // Encoding detection
    bool _autoDetectEncoding = true;

    // Progress tracking
    int _totalOperations = 0;
    int _completedOperations = 0;
};

// ============================================================================
// Utility Functions
// ============================================================================

namespace IOUtils {

/// Convert QString to std::wstring
std::wstring qstringToWstring(const QString& str);

/// Convert std::wstring to QString
QString wstringToQstring(const std::wstring& str);

/// Convert QByteArray to QString with encoding
QString byteArrayToQString(const QByteArray& data, const QString& encoding);

/// Convert QString to QByteArray with encoding
QByteArray qstringToByteArray(const QString& str, const QString& encoding);

/// Get the filename from a path
QString getFileName(const QString& filePath);

/// Get the directory from a path
QString getDirectory(const QString& filePath);

/// Get the file extension
QString getFileExtension(const QString& filePath);

/// Check if path is absolute
bool isAbsolutePath(const QString& path);

/// Normalize path (resolve .. and .)
QString normalizePath(const QString& path);

/// Get unique temporary file path
QString getTempFilePath(const QString& prefix = QString());

/// Copy file with progress
bool copyFileWithProgress(const QString& source, const QString& dest,
                          std::function<void(int percent)> progress = nullptr);

/// Move file (with fallback to copy+delete)
bool moveFile(const QString& source, const QString& dest);

/// Check if file is binary
bool isBinaryFile(const QByteArray& data, int checkLength = 8000);

/// Read file with encoding detection
struct ReadFileResult {
    QByteArray content;
    QString encoding;
    bool hasBOM = false;
    bool success = false;
    QString error;
};

ReadFileResult readFileWithEncoding(const QString& filePath,
                                    const QString& suggestedEncoding = QString());

} // namespace IOUtils

} // namespace QtIO

// ============================================================================
// Global Access
// ============================================================================

QtIO::NppIO* getNppIO();
void setNppIO(QtIO::NppIO* nppIO);
