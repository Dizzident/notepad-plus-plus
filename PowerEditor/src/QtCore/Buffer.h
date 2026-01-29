// This file is part of Notepad++ project
// Copyright (C)2021 Don HO <don.h@free.fr>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <QString>
#include <QDateTime>
#include <QObject>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QStringEncoder>
#include <QStringDecoder>
#include <QMutex>
#include <QList>

// Include LangType and Position definitions
#include "MISC/PluginsManager/Notepad_plus_msgs.h"
#include "Parameters.h"

// Forward declaration for Scintilla
namespace Scintilla {
class ScintillaEditView;
}

namespace QtCore {

// Buffer status enumeration
enum class BufferStatus {
    Clean,          // Saved, no changes
    Dirty,          // Unsaved changes
    ModifiedOutside // File modified externally
};

// Document language type enumeration (mirrors LangType from Notepad_plus_msgs.h)
// Using regular enum (not enum class) for implicit conversion compatibility with LangType
enum DocLangType {
    L_TEXT, L_PHP, L_C, L_CPP, L_CS, L_OBJC, L_JAVA, L_GO, L_RC,
    L_HTML, L_XML, L_MAKEFILE, L_PASCAL, L_BATCH, L_INI,
    L_ASCII, L_USER, L_ASP, L_SQL, L_VB, L_JS_EMBEDDED, L_CSS,
    L_PERL, L_PYTHON, L_LUA, L_TEX, L_FORTRAN, L_BASH, L_FLASH,
    L_NSIS, L_TCL, L_LISP, L_SCHEME, L_ASM, L_DIFF, L_PROPS,
    L_PS, L_RUBY, L_SMALLTALK, L_VHDL, L_KIX, L_AU3, L_CAML,
    L_ADA, L_VERILOG, L_MATLAB, L_HASKELL, L_INNO, L_SEARCHRESULT,
    L_CMAKE, L_YAML, L_COBOL, L_GUI4CLI, L_D, L_POWERSHELL, L_R,
    L_JSP, L_COFFEESCRIPT, L_JSON, L_JAVASCRIPT, L_FORTRAN_77,
    L_BAANC, L_SREC, L_IHEX, L_TEHEX, L_SWIFT, L_ASN1, L_AVS,
    L_BLITZBASIC, L_PUREBASIC, L_FREEBASIC, L_CSOUND, L_ERLANG,
    L_ESCRIPT, L_FORTH, L_LATEX, L_MMIXAL, L_NIM, L_NNCRONTAB,
    L_OSCRIPT, L_REBOL, L_REGISTRY, L_RUST, L_SPICE, L_TXT2TAGS,
    L_VISUALPROLOG, L_TYPESCRIPT, L_HOLLYWOOD, L_RAKU, L_TOML,
    L_SAS, L_SML, L_MARKDOWN, L_GDSCRIPT, L_VBSCRIPT,
    L_EXTERNAL
};

// Line ending types
enum class LineEnding {
    Windows,    // CRLF \r\n
    Unix,       // LF \n
    MacOS,      // CR \r
    OSDefault
};

// Buffer change notification flags (mirrors BufferStatusInfo from Windows Buffer.h)
enum BufferStatusInfo {
    BufferChangeNone        = 0x000,  // Nothing to change
    BufferChangeLanguage    = 0x001,  // Language was altered
    BufferChangeDirty       = 0x002,  // Buffer has changed dirty state
    BufferChangeFormat      = 0x004,  // EOL type was changed
    BufferChangeUnicode     = 0x008,  // Unicode type was changed
    BufferChangeReadonly    = 0x010,  // Readonly state was changed, can be both file and user
    BufferChangeStatus      = 0x020,  // Filesystem Status has changed
    BufferChangeTimestamp   = 0x040,  // Timestamp was changed
    BufferChangeFilename    = 0x080,  // Filename was changed
    BufferChangeRecentTag   = 0x100,  // Recent tag has changed
    BufferChangeLexing      = 0x200,  // Document needs lexing
    BufferChangeMask        = 0x3FF   // Mask: covers all changes
};

// Position structure is defined in Parameters.h
// Use a typedef for QtCore::Position to refer to the global ::Position
typedef ::Position Position;

// Map position for document map
struct MapPosition {
    int firstVisibleDisplayLine = -1;
    int firstVisibleDocLine = -1;
    int lastVisibleDocLine = -1;
    int nbLine = -1;
    int higherPos = -1;
    int width = -1;
    int height = -1;
    bool isWrap = false;

    bool isValid() const { return firstVisibleDisplayLine != -1; }
};

/**
 * @brief The Buffer class manages a single document's lifecycle, file association, and metadata.
 *
 * This class wraps a Scintilla document and provides Qt-friendly interfaces for:
 * - Document content management (via Scintilla)
 * - File path management
 * - Encoding management
 * - Modification state tracking
 * - Language/lexer association
 * - Document statistics (lines, words, characters)
 * - Auto-save state
 * - Backup management
 * - Read-only handling
 * - File monitoring integration
 */
class Buffer : public QObject {
    Q_OBJECT

public:
    explicit Buffer(QObject* parent = nullptr);
    ~Buffer() override;

    // Initialization
    void setScintillaView(Scintilla::ScintillaEditView* view);
    void setID(int id);
    int getID() const;

    // File operations
    bool loadFromFile(const QString& filePath);
    bool saveToFile(const QString& filePath);
    bool reloadFromFile();

    // File info
    QString getFilePath() const;
    QString getFileName() const;
    void setFilePath(const QString& path);
    bool isUntitled() const;
    bool isNew() const;

    // Content
    QByteArray getContent() const;
    void setContent(const QByteArray& content);
    QString getText() const;
    void setText(const QString& text);

    // Document stats
    int getLineCount() const;
    int getCharCount() const;
    int getWordCount() const;
    int getCurrentPos() const;
    int getCurrentLine() const;
    int getCurrentColumn() const;

    // Modification state
    bool isDirty() const;
    void setDirty(bool dirty);
    BufferStatus getStatus() const;
    void setStatus(BufferStatus status);

    // Read-only
    bool isReadOnly() const;
    void setReadOnly(bool readOnly);
    bool isUserReadOnly() const;
    void setUserReadOnly(bool readOnly);
    bool isFileReadOnly() const;
    void setFileReadOnly(bool readOnly);

    // Encoding
    QString getEncoding() const;
    void setEncoding(const QString& encoding);
    bool getUseBOM() const;
    void setUseBOM(bool use);

    // Line endings
    LineEnding getLineEnding() const;
    void setLineEnding(LineEnding ending);
    QString getLineEndingString() const;

    // Language/Lexer
    DocLangType getLangType() const;
    void setLangType(DocLangType type);
    QString getLangTypeName() const;
    void setLangTypeFromFileName(const QString& fileName);
    void setLangTypeFromContent();

    // Compatibility: get language as LangType
    LangType getLanguage() const { return static_cast<LangType>(getLangType()); }

    // Format
    bool isIndentTab() const;
    void setIndentTab(bool useTab);
    int getTabWidth() const;
    void setTabWidth(int width);
    int getIndentWidth() const;
    void setIndentWidth(int width);

    // Auto-save
    bool needsAutoSave() const;
    void setAutoSavePoint();
    QString getAutoSaveFilePath() const;
    bool recoverFromAutoSave();

    // Backup
    bool hasBackup() const;
    QString getBackupFilePath() const;
    bool createBackup();
    bool restoreFromBackup();
    void removeBackup();

    // File monitoring
    void setFileMonitoringEnabled(bool enabled);
    bool isFileMonitoringEnabled() const;
    void onFileChanged();

    // Time stamps
    QDateTime getLastModifiedTime() const;
    void setLastModifiedTime(const QDateTime& time);
    QDateTime getLastSavedTime() const;
    void updateLastSavedTime();

    // Position/selection save/restore
    void savePosition();
    void restorePosition();
    void saveSelection();
    void restoreSelection();

    // Map position (for document map)
    MapPosition getMapPosition() const;
    void setMapPosition(const MapPosition& pos);

    // Large file handling
    bool isLargeFile() const;
    void setLargeFile(bool isLarge);

    // Pin status
    bool isPinned() const;
    void setPinned(bool pinned);

    // RTL support
    bool isRTL() const;
    void setRTL(bool rtl);

    // Tab color
    int getDocColorId() const;
    void setDocColorId(int colorId);

    // Check file state (for external modifications)
    bool checkFileState();

    // Get language comment symbols
    QString getCommentLineSymbol() const;
    QString getCommentStart() const;
    QString getCommentEnd() const;

    // Compatibility methods for ScintillaEditView integration
    void setHideLineChanged(bool isHide, size_t location);
    void setHeaderLineState(const std::vector<size_t>& folds, void* identifier);
    std::vector<size_t> getHeaderLineState(void* identifier) const;
    void* getDocument() const;

    // Position compatibility (for save/restore) - uses Position from QtCore namespace
    void setPosition(const Position& pos, void* identifier);
    Position getPosition(void* identifier) const;

    // Unicode mode compatibility
    enum UniMode { uni8Bit, uniUTF8, uni16BE, uni16LE, uniCookie, uni7Bit, uni16BE_NoBOM, uni16LE_NoBOM, uniUTF8_NoBOM };
    UniMode getUnicodeMode() const;
    void setUnicodeMode(UniMode mode);

    // EOL format compatibility
    enum EolType { eolWindows, eolUnix, eolMac, eolUnknown };
    EolType getEolFormat() const;
    void setEolFormat(EolType format);

    // Lexing state
    bool getNeedsLexing() const;
    void setNeedsLexing(bool needs);

    // Encoding as int (for compatibility with Windows API)
    int getEncodingNumber() const;
    void setEncodingNumber(int encoding);

signals:
    void contentChanged();
    void statusChanged(BufferStatus status);
    void filePathChanged(const QString& newPath);
    void encodingChanged(const QString& encoding);
    void langTypeChanged(DocLangType type);
    void readOnlyChanged(bool readOnly);
    void fileModifiedExternally();
    void saved();
    void loaded();
    void dirtyChanged(bool dirty);

private slots:
    void onFileSystemChanged(const QString& path);

private:
    // Buffer identification
    int _id = -1;

    // File information
    QString _filePath;
    QString _fileName;
    bool _isUntitled = true;

    // Encoding settings
    QString _encoding = "UTF-8";
    bool _useBOM = false;

    // Line ending
    LineEnding _lineEnding = LineEnding::OSDefault;

    // Language/Lexer
    DocLangType _langType = L_TEXT;
    QString _userLangName;

    // Status
    BufferStatus _status = BufferStatus::Clean;
    bool _isDirty = false;

    // Read-only flags
    bool _isUserReadOnly = false;
    bool _isFileReadOnly = false;

    // Format settings
    bool _useTabs = true;
    int _tabWidth = 4;
    int _indentWidth = 4;

    // File monitoring
    bool _fileMonitoringEnabled = true;
    QFileSystemWatcher* _fileWatcher = nullptr;

    // Timestamps
    QDateTime _lastModifiedTime;
    QDateTime _lastSavedTime;
    QDateTime _lastAutoSaveTime;

    // Saved positions
    Position _savedPosition;
    MapPosition _mapPosition;

    // Large file flag
    bool _isLargeFile = false;

    // Pin status
    bool _isPinned = false;

    // RTL support
    bool _isRTL = false;

    // Tab color ID
    int _docColorId = -1;

    // Backup file path
    QString _backupFilePath;

    // Scintilla view reference
    Scintilla::ScintillaEditView* _pView = nullptr;

    // Mutex for thread safety
    mutable QMutex _mutex;

    // Compatibility members for ScintillaEditView integration
    UniMode _unicodeMode = uniUTF8;
    EolType _eolFormat = eolUnix;
    bool _needsLexing = false;
    void* _document = nullptr;
    std::vector<std::vector<size_t>> _foldStates;  // Per-view fold states
    std::vector<void*> _viewIdentifiers;  // View identifiers for fold states

    // Helper methods
    void updateStatus();
    QString generateAutoSaveFilePath() const;
    QString generateBackupFilePath() const;
    DocLangType detectLanguageFromFileName(const QString& fileName) const;
    DocLangType detectLanguageFromContent(const QByteArray& content) const;
    void setupFileWatcher();
    void removeFileWatcher();
    bool writeToFile(const QString& filePath, const QByteArray& content);
    QByteArray readFromFile(const QString& filePath);
    QString detectEncodingFromBOM(const QByteArray& content);
};

/**
 * @brief The BufferManager class manages all open buffers/documents.
 *
 * This is a singleton class that provides centralized management of all
 * document buffers in the application.
 */
class BufferManager : public QObject {
    Q_OBJECT

public:
    static BufferManager* getInstance();

    Buffer* createBuffer();
    void deleteBuffer(Buffer* buffer);
    Buffer* getBufferByID(int id);
    Buffer* getBufferByFilePath(const QString& filePath);
    QList<Buffer*> getAllBuffers() const;
    int getBufferCount() const;

    int getCurrentBufferIndex() const;
    void setCurrentBufferIndex(int index);
    Buffer* getCurrentBuffer() const;

    bool hasDirtyBuffers() const;
    QList<Buffer*> getDirtyBuffers() const;

    void saveAllBuffers();
    void closeAllBuffers();

    // Generate next untitled document name
    QString getNextUntitledName() const;

signals:
    void bufferCreated(Buffer* buffer);
    void bufferDeleted(Buffer* buffer);
    void currentBufferChanged(Buffer* buffer);
    void buffersModified(bool hasDirty);

private:
    BufferManager();
    ~BufferManager();

    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;

    QList<Buffer*> _buffers;
    int _currentIndex = -1;
    int _nextBufferID = 1;
    mutable QMutex _mutex;
};

/**
 * @brief The FileManager class provides compatibility with Notepad++ Windows API.
 *
 * This is a simplified compatibility layer that wraps BufferManager functionality
 * to match the Windows FileManager interface.
 */
class FileManager {
public:
    static FileManager* getInstance();

    Buffer* getBufferByID(Buffer* id);
    Buffer* newEmptyDocument();

private:
    FileManager() = default;
    ~FileManager() = default;
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;
};

} // namespace QtCore

// Compatibility macro for Notepad++ Windows API
#define MainFileManager (*QtCore::FileManager::getInstance())
