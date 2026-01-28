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

#include "Buffer.h"

#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QCryptographicHash>

// For Scintilla integration - these would normally come from Scintilla headers
// Using forward declarations and mock implementations for the Qt port
namespace Scintilla {

// Mock ScintillaEditView for Qt implementation
// In the full implementation, this would be the actual Scintilla Qt wrapper
class ScintillaEditView {
public:
    virtual ~ScintillaEditView() = default;

    // Document operations
    virtual int getLength() const { return 0; }
    virtual QByteArray getTextRange(int start, int end) const { return QByteArray(); }
    virtual void setText(const QByteArray& text) {}
    virtual void insertText(int pos, const QByteArray& text) {}
    virtual void clearAll() {}

    // Position and selection
    virtual int getCurrentPos() const { return 0; }
    virtual int getAnchor() const { return 0; }
    virtual void setSelection(int anchor, int caret) {}
    virtual int getCurrentLine() const { return 0; }
    virtual int getColumn(int pos) const { return 0; }
    virtual int positionFromLine(int line) const { return 0; }

    // Line operations
    virtual int getLineCount() const { return 1; }
    virtual int getLineEndPosition(int line) const { return 0; }
    virtual QByteArray getLine(int line) const { return QByteArray(); }

    // Document state
    virtual bool isModified() const { return false; }
    virtual void setSavePoint() {}
    virtual void emptyUndoBuffer() {}

    // Read-only
    virtual bool getReadOnly() const { return false; }
    virtual void setReadOnly(bool readOnly) {}

    // Encoding
    virtual void setCodePage(int codePage) {}
    virtual int getCodePage() const { return 65001; } // UTF-8

    // Line endings
    virtual void setEOLMode(int mode) {}
    virtual int getEOLMode() const { return 0; }
    virtual void convertEOLs(int mode) {}

    // Lexer
    virtual void setLexer(int lexer) {}
    virtual void setLexerLanguage(const char* language) {}

    // Tab settings
    virtual void setUseTabs(bool useTabs) {}
    virtual void setTabWidth(int width) {}
    virtual void setIndentWidth(int width) {}

    // Word count helper
    virtual int getWordCount() const { return 0; }

    // First visible line for position save/restore
    virtual int getFirstVisibleLine() const { return 0; }
    virtual void setFirstVisibleLine(int line) {}
    virtual int getXOffset() const { return 0; }
    virtual void setXOffset(int xOffset) {}

    // Document pointer (for buffer management)
    virtual void* getDocPointer() const { return nullptr; }
    virtual void setDocPointer(void* doc) {}
};

} // namespace Scintilla

namespace QtCore {

// ============================================================================
// Static helper functions
// ============================================================================

static QString getBackupDirectory()
{
    QString backupDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    backupDir += "/backup";
    QDir dir(backupDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return backupDir;
}

static QString getAutoSaveDirectory()
{
    QString autoSaveDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    autoSaveDir += "/autosave";
    QDir dir(autoSaveDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return autoSaveDir;
}

static LineEnding detectLineEnding(const QByteArray& content)
{
    if (content.contains("\r\n")) {
        return LineEnding::Windows;
    } else if (content.contains("\n")) {
        return LineEnding::Unix;
    } else if (content.contains("\r")) {
        return LineEnding::MacOS;
    }

#if defined(Q_OS_WIN)
    return LineEnding::Windows;
#elif defined(Q_OS_MAC)
    return LineEnding::MacOS;
#else
    return LineEnding::Unix;
#endif
}

static QByteArray convertLineEndings(const QByteArray& content, LineEnding ending)
{
    QByteArray result = content;
    QByteArray targetEol;

    switch (ending) {
        case LineEnding::Windows:
            targetEol = "\r\n";
            break;
        case LineEnding::Unix:
            targetEol = "\n";
            break;
        case LineEnding::MacOS:
            targetEol = "\r";
            break;
        default:
            targetEol = "\n";
            break;
    }

    // Normalize to LF first
    result.replace("\r\n", "\n");
    result.replace("\r", "\n");

    // Convert to target
    if (targetEol != "\n") {
        result.replace("\n", targetEol);
    }

    return result;
}

static DocLangType detectLanguageFromExtension(const QString& ext)
{
    static const QHash<QString, DocLangType> extMap = {
        { "txt", DocLangType::L_TEXT },
        { "php", DocLangType::L_PHP },
        { "php3", DocLangType::L_PHP },
        { "php4", DocLangType::L_PHP },
        { "php5", DocLangType::L_PHP },
        { "phtml", DocLangType::L_PHP },
        { "c", DocLangType::L_C },
        { "h", DocLangType::L_C },
        { "cpp", DocLangType::L_CPP },
        { "cxx", DocLangType::L_CPP },
        { "cc", DocLangType::L_CPP },
        { "hpp", DocLangType::L_CPP },
        { "hxx", DocLangType::L_CPP },
        { "cs", DocLangType::L_CS },
        { "m", DocLangType::L_OBJC },
        { "mm", DocLangType::L_OBJC },
        { "java", DocLangType::L_JAVA },
        { "rc", DocLangType::L_RC },
        { "html", DocLangType::L_HTML },
        { "htm", DocLangType::L_HTML },
        { "shtml", DocLangType::L_HTML },
        { "xml", DocLangType::L_XML },
        { "xaml", DocLangType::L_XML },
        { "xsl", DocLangType::L_XML },
        { "xslt", DocLangType::L_XML },
        { "mak", DocLangType::L_MAKEFILE },
        { "makefile", DocLangType::L_MAKEFILE },
        { "pas", DocLangType::L_PASCAL },
        { "pp", DocLangType::L_PASCAL },
        { "inc", DocLangType::L_PASCAL },
        { "bat", DocLangType::L_BATCH },
        { "cmd", DocLangType::L_BATCH },
        { "nt", DocLangType::L_BATCH },
        { "ini", DocLangType::L_INI },
        { "inf", DocLangType::L_INI },
        { "reg", DocLangType::L_REGISTRY },
        { "cfg", DocLangType::L_INI },
        { "conf", DocLangType::L_INI },
        { "sql", DocLangType::L_SQL },
        { "vb", DocLangType::L_VB },
        { "vbs", DocLangType::L_VBSCRIPT },
        { "bas", DocLangType::L_VB },
        { "frm", DocLangType::L_VB },
        { "cls", DocLangType::L_VB },
        { "js", DocLangType::L_JAVASCRIPT },
        { "json", DocLangType::L_JSON },
        { "css", DocLangType::L_CSS },
        { "pl", DocLangType::L_PERL },
        { "pm", DocLangType::L_PERL },
        { "py", DocLangType::L_PYTHON },
        { "pyw", DocLangType::L_PYTHON },
        { "lua", DocLangType::L_LUA },
        { "tex", DocLangType::L_LATEX },
        { "latex", DocLangType::L_LATEX },
        { "f", DocLangType::L_FORTRAN },
        { "for", DocLangType::L_FORTRAN },
        { "f90", DocLangType::L_FORTRAN },
        { "f95", DocLangType::L_FORTRAN },
        { "f2k", DocLangType::L_FORTRAN },
        { "sh", DocLangType::L_BASH },
        { "bash", DocLangType::L_BASH },
        { "zsh", DocLangType::L_BASH },
        { "rb", DocLangType::L_RUBY },
        { "rbw", DocLangType::L_RUBY },
        { "rake", DocLangType::L_RUBY },
        { "gemspec", DocLangType::L_RUBY },
        { "tcl", DocLangType::L_TCL },
        { "tk", DocLangType::L_TCL },
        { "lisp", DocLangType::L_LISP },
        { "lsp", DocLangType::L_LISP },
        { "scm", DocLangType::L_SCHEME },
        { "ss", DocLangType::L_SCHEME },
        { "asm", DocLangType::L_ASM },
        { "s", DocLangType::L_ASM },
        { "nasm", DocLangType::L_ASM },
        { "diff", DocLangType::L_DIFF },
        { "patch", DocLangType::L_DIFF },
        { "props", DocLangType::L_PROPS },
        { "properties", DocLangType::L_PROPS },
        { "ps", DocLangType::L_PS },
        { "yaml", DocLangType::L_YAML },
        { "yml", DocLangType::L_YAML },
        { "cmake", DocLangType::L_CMAKE },
        { "md", DocLangType::L_MARKDOWN },
        { "markdown", DocLangType::L_MARKDOWN },
        { "rs", DocLangType::L_RUST },
        { "go", DocLangType::L_TEXT },  // Go not in original enum, map to text
        { "ts", DocLangType::L_TYPESCRIPT },
        { "tsx", DocLangType::L_TYPESCRIPT },
        { "coffee", DocLangType::L_COFFEESCRIPT },
        { "ps1", DocLangType::L_POWERSHELL },
        { "psm1", DocLangType::L_POWERSHELL },
        { "psd1", DocLangType::L_POWERSHELL },
        { "r", DocLangType::L_R },
        { "swift", DocLangType::L_SWIFT },
        { "kt", DocLangType::L_TEXT },  // Kotlin not in original enum
        { "scala", DocLangType::L_TEXT },  // Scala not in original enum
        { "gd", DocLangType::L_GDSCRIPT },
    };

    QString lowerExt = ext.toLower();
    return extMap.value(lowerExt, DocLangType::L_TEXT);
}

static DocLangType detectLanguageFromShebang(const QByteArray& content)
{
    // Check for shebang line
    int newlinePos = content.indexOf('\n');
    if (newlinePos == -1) {
        newlinePos = content.indexOf('\r');
    }

    if (newlinePos > 0) {
        QByteArray firstLine = content.left(newlinePos).trimmed();
        if (firstLine.startsWith("#!")) {
            if (firstLine.contains("python")) {
                return DocLangType::L_PYTHON;
            } else if (firstLine.contains("perl")) {
                return DocLangType::L_PERL;
            } else if (firstLine.contains("ruby")) {
                return DocLangType::L_RUBY;
            } else if (firstLine.contains("bash") || firstLine.contains("sh")) {
                return DocLangType::L_BASH;
            } else if (firstLine.contains("php")) {
                return DocLangType::L_PHP;
            } else if (firstLine.contains("node")) {
                return DocLangType::L_JAVASCRIPT;
            }
        }
    }

    // Check for XML declaration
    if (content.trimmed().startsWith("<?xml")) {
        return DocLangType::L_XML;
    }

    // Check for HTML doctype
    QByteArray trimmed = content.trimmed().toLower();
    if (trimmed.startsWith("<!doctype html")) {
        return DocLangType::L_HTML;
    }
    if (trimmed.startsWith("<html")) {
        return DocLangType::L_HTML;
    }

    return DocLangType::L_TEXT;
}

static QString getLanguageName(DocLangType type)
{
    static const QHash<DocLangType, QString> langNames = {
        { DocLangType::L_TEXT, QObject::tr("Normal text") },
        { DocLangType::L_PHP, QObject::tr("PHP") },
        { DocLangType::L_C, QObject::tr("C") },
        { DocLangType::L_CPP, QObject::tr("C++") },
        { DocLangType::L_CS, QObject::tr("C#") },
        { DocLangType::L_OBJC, QObject::tr("Objective-C") },
        { DocLangType::L_JAVA, QObject::tr("Java") },
        { DocLangType::L_RC, QObject::tr("Resource file") },
        { DocLangType::L_HTML, QObject::tr("HTML") },
        { DocLangType::L_XML, QObject::tr("XML") },
        { DocLangType::L_MAKEFILE, QObject::tr("Makefile") },
        { DocLangType::L_PASCAL, QObject::tr("Pascal") },
        { DocLangType::L_BATCH, QObject::tr("Batch") },
        { DocLangType::L_INI, QObject::tr("INI file") },
        { DocLangType::L_ASCII, QObject::tr("ASCII") },
        { DocLangType::L_USER, QObject::tr("User defined") },
        { DocLangType::L_SQL, QObject::tr("SQL") },
        { DocLangType::L_VB, QObject::tr("Visual Basic") },
        { DocLangType::L_JAVASCRIPT, QObject::tr("JavaScript") },
        { DocLangType::L_CSS, QObject::tr("CSS") },
        { DocLangType::L_PERL, QObject::tr("Perl") },
        { DocLangType::L_PYTHON, QObject::tr("Python") },
        { DocLangType::L_LUA, QObject::tr("Lua") },
        { DocLangType::L_TEX, QObject::tr("TeX") },
        { DocLangType::L_FORTRAN, QObject::tr("Fortran") },
        { DocLangType::L_BASH, QObject::tr("Shell") },
        { DocLangType::L_RUBY, QObject::tr("Ruby") },
        { DocLangType::L_YAML, QObject::tr("YAML") },
        { DocLangType::L_JSON, QObject::tr("JSON") },
        { DocLangType::L_MARKDOWN, QObject::tr("Markdown") },
        { DocLangType::L_RUST, QObject::tr("Rust") },
        { DocLangType::L_TYPESCRIPT, QObject::tr("TypeScript") },
        { DocLangType::L_POWERSHELL, QObject::tr("PowerShell") },
        { DocLangType::L_CMAKE, QObject::tr("CMake") },
        { DocLangType::L_GDSCRIPT, QObject::tr("GDScript") },
    };

    return langNames.value(type, QObject::tr("Unknown"));
}

// ============================================================================
// Buffer implementation
// ============================================================================

Buffer::Buffer(QObject* parent)
    : QObject(parent)
    , _textCodec(QTextCodec::codecForName("UTF-8"))
{
    // Setup default text codec
    if (!_textCodec) {
        _textCodec = QTextCodec::codecForLocale();
    }

    // Determine default line ending from OS
#if defined(Q_OS_WIN)
    _lineEnding = LineEnding::Windows;
#elif defined(Q_OS_MAC)
    _lineEnding = LineEnding::MacOS;
#else
    _lineEnding = LineEnding::Unix;
#endif
}

Buffer::~Buffer()
{
    removeFileWatcher();

    // Remove backup file if exists
    if (!_backupFilePath.isEmpty() && QFile::exists(_backupFilePath)) {
        QFile::remove(_backupFilePath);
    }

    // Remove auto-save file if exists
    QString autoSavePath = getAutoSaveFilePath();
    if (QFile::exists(autoSavePath)) {
        QFile::remove(autoSavePath);
    }
}

void Buffer::setScintillaView(Scintilla::ScintillaEditView* view)
{
    QMutexLocker locker(&_mutex);
    _pView = view;
}

void Buffer::setID(int id)
{
    _id = id;
}

int Buffer::getID() const
{
    return _id;
}

bool Buffer::loadFromFile(const QString& filePath)
{
    QMutexLocker locker(&_mutex);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << filePath << "-" << file.errorString();
        return false;
    }

    // Read file content
    QByteArray content = file.readAll();
    file.close();

    // Get file info
    QFileInfo fileInfo(filePath);
    _lastModifiedTime = fileInfo.lastModified();
    _isFileReadOnly = !fileInfo.isWritable();

    // Detect encoding
    QTextCodec* detectedCodec = QTextCodec::codecForUtfText(content, nullptr);
    if (!detectedCodec) {
        // Try to detect from content
        detectedCodec = QTextCodec::codecForName("UTF-8");
    }

    if (detectedCodec) {
        _textCodec = detectedCodec;
        _encoding = QString::fromLatin1(detectedCodec->name());
    }

    // Detect line endings
    _lineEnding = detectLineEnding(content);

    // Detect language from file name
    _langType = detectLanguageFromFileName(fileInfo.suffix());
    if (_langType == DocLangType::L_TEXT) {
        _langType = detectLanguageFromShebang(content);
    }

    // Set file path (this will also update _fileName and _isUntitled)
    setFilePath(filePath);

    // Set content in Scintilla view
    if (_pView) {
        _pView->clearAll();
        _pView->setText(content);
        _pView->setSavePoint();
        _pView->emptyUndoBuffer();
    }

    // Update status
    _isDirty = false;
    _status = BufferStatus::Clean;
    _lastSavedTime = QDateTime::currentDateTime();

    // Setup file watcher
    setupFileWatcher();

    emit loaded();
    emit statusChanged(_status);
    emit dirtyChanged(false);

    return true;
}

bool Buffer::saveToFile(const QString& filePath)
{
    QMutexLocker locker(&_mutex);

    QByteArray content;

    if (_pView) {
        content = _pView->getTextRange(0, _pView->getLength());
    }

    // Convert line endings if needed
    content = convertLineEndings(content, _lineEnding);

    // Write to file
    if (!writeToFile(filePath, content)) {
        return false;
    }

    // Update file info
    QFileInfo fileInfo(filePath);
    _lastModifiedTime = fileInfo.lastModified();
    _lastSavedTime = QDateTime::currentDateTime();

    // Update file path if changed
    if (_filePath != filePath) {
        setFilePath(filePath);
    }

    // Update status
    _isDirty = false;
    _status = BufferStatus::Clean;

    if (_pView) {
        _pView->setSavePoint();
    }

    // Remove backup file after successful save
    if (!_backupFilePath.isEmpty() && QFile::exists(_backupFilePath)) {
        QFile::remove(_backupFilePath);
        _backupFilePath.clear();
    }

    // Remove auto-save file
    QString autoSavePath = getAutoSaveFilePath();
    if (QFile::exists(autoSavePath)) {
        QFile::remove(autoSavePath);
    }

    emit saved();
    emit statusChanged(_status);
    emit dirtyChanged(false);

    return true;
}

bool Buffer::reloadFromFile()
{
    if (_isUntitled || _filePath.isEmpty()) {
        return false;
    }

    return loadFromFile(_filePath);
}

QString Buffer::getFilePath() const
{
    QMutexLocker locker(&_mutex);
    return _filePath;
}

QString Buffer::getFileName() const
{
    QMutexLocker locker(&_mutex);
    return _fileName;
}

void Buffer::setFilePath(const QString& path)
{
    QMutexLocker locker(&_mutex);

    QString oldPath = _filePath;
    _filePath = path;

    QFileInfo fileInfo(path);
    _fileName = fileInfo.fileName();
    _isUntitled = fileInfo.fileName().startsWith("new ") || path.isEmpty();

    // Update file watcher
    if (oldPath != path) {
        removeFileWatcher();
        setupFileWatcher();
    }

    // Detect language from new file name
    if (!_isUntitled) {
        DocLangType detectedLang = detectLanguageFromExtension(fileInfo.suffix());
        if (detectedLang != DocLangType::L_TEXT && detectedLang != _langType) {
            _langType = detectedLang;
            emit langTypeChanged(_langType);
        }
    }

    emit filePathChanged(path);
}

bool Buffer::isUntitled() const
{
    QMutexLocker locker(&_mutex);
    return _isUntitled;
}

bool Buffer::isNew() const
{
    QMutexLocker locker(&_mutex);
    return _isUntitled && !_isDirty;
}

QByteArray Buffer::getContent() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        return _pView->getTextRange(0, _pView->getLength());
    }

    return QByteArray();
}

void Buffer::setContent(const QByteArray& content)
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        _pView->clearAll();
        _pView->setText(content);
    }

    _isDirty = true;
    _status = BufferStatus::Dirty;

    emit contentChanged();
    emit statusChanged(_status);
    emit dirtyChanged(true);
}

QString Buffer::getText() const
{
    QByteArray content = getContent();
    if (_textCodec) {
        return _textCodec->toUnicode(content);
    }
    return QString::fromUtf8(content);
}

void Buffer::setText(const QString& text)
{
    QByteArray content;
    if (_textCodec) {
        content = _textCodec->fromUnicode(text);
    } else {
        content = text.toUtf8();
    }
    setContent(content);
}

int Buffer::getLineCount() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        return _pView->getLineCount();
    }

    return 1;
}

int Buffer::getCharCount() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        return _pView->getLength();
    }

    return 0;
}

int Buffer::getWordCount() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        return _pView->getWordCount();
    }

    // Fallback: count words in content
    QString text = getText();
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    return words.count();
}

int Buffer::getCurrentPos() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        return _pView->getCurrentPos();
    }

    return 0;
}

int Buffer::getCurrentLine() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        return _pView->getCurrentLine();
    }

    return 0;
}

int Buffer::getCurrentColumn() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        int pos = _pView->getCurrentPos();
        return _pView->getColumn(pos);
    }

    return 0;
}

bool Buffer::isDirty() const
{
    QMutexLocker locker(&_mutex);
    return _isDirty;
}

void Buffer::setDirty(bool dirty)
{
    QMutexLocker locker(&_mutex);

    if (_isDirty != dirty) {
        _isDirty = dirty;
        _status = dirty ? BufferStatus::Dirty : BufferStatus::Clean;

        emit dirtyChanged(dirty);
        emit statusChanged(_status);
    }
}

BufferStatus Buffer::getStatus() const
{
    QMutexLocker locker(&_mutex);
    return _status;
}

void Buffer::setStatus(BufferStatus status)
{
    QMutexLocker locker(&_mutex);

    if (_status != status) {
        _status = status;
        emit statusChanged(status);
    }
}

bool Buffer::isReadOnly() const
{
    QMutexLocker locker(&_mutex);
    return _isUserReadOnly || _isFileReadOnly;
}

void Buffer::setReadOnly(bool readOnly)
{
    setUserReadOnly(readOnly);
}

bool Buffer::isUserReadOnly() const
{
    QMutexLocker locker(&_mutex);
    return _isUserReadOnly;
}

void Buffer::setUserReadOnly(bool readOnly)
{
    QMutexLocker locker(&_mutex);

    if (_isUserReadOnly != readOnly) {
        _isUserReadOnly = readOnly;

        if (_pView) {
            _pView->setReadOnly(isReadOnly());
        }

        emit readOnlyChanged(isReadOnly());
    }
}

bool Buffer::isFileReadOnly() const
{
    QMutexLocker locker(&_mutex);
    return _isFileReadOnly;
}

void Buffer::setFileReadOnly(bool readOnly)
{
    QMutexLocker locker(&_mutex);

    if (_isFileReadOnly != readOnly) {
        _isFileReadOnly = readOnly;

        if (_pView) {
            _pView->setReadOnly(isReadOnly());
        }

        emit readOnlyChanged(isReadOnly());
    }
}

QString Buffer::getEncoding() const
{
    QMutexLocker locker(&_mutex);
    return _encoding;
}

void Buffer::setEncoding(const QString& encoding)
{
    QMutexLocker locker(&_mutex);

    if (_encoding != encoding) {
        _encoding = encoding;
        _textCodec = QTextCodec::codecForName(encoding.toLatin1());

        if (!_textCodec) {
            _textCodec = QTextCodec::codecForLocale();
        }

        emit encodingChanged(encoding);
    }
}

bool Buffer::getUseBOM() const
{
    QMutexLocker locker(&_mutex);
    return _useBOM;
}

void Buffer::setUseBOM(bool use)
{
    QMutexLocker locker(&_mutex);
    _useBOM = use;
}

QTextCodec* Buffer::getTextCodec() const
{
    QMutexLocker locker(&_mutex);
    return _textCodec;
}

LineEnding Buffer::getLineEnding() const
{
    QMutexLocker locker(&_mutex);
    return _lineEnding;
}

void Buffer::setLineEnding(LineEnding ending)
{
    QMutexLocker locker(&_mutex);

    if (_lineEnding != ending) {
        _lineEnding = ending;

        // Convert document line endings
        if (_pView) {
            int sciEolMode = 0; // SC_EOL_CRLF
            switch (ending) {
                case LineEnding::Windows:
                    sciEolMode = 0; // SC_EOL_CRLF
                    break;
                case LineEnding::Unix:
                    sciEolMode = 2; // SC_EOL_LF
                    break;
                case LineEnding::MacOS:
                    sciEolMode = 1; // SC_EOL_CR
                    break;
                default:
                    sciEolMode = 2; // Default to LF
                    break;
            }
            _pView->convertEOLs(sciEolMode);
            _pView->setEOLMode(sciEolMode);
        }
    }
}

QString Buffer::getLineEndingString() const
{
    QMutexLocker locker(&_mutex);

    switch (_lineEnding) {
        case LineEnding::Windows:
            return "\r\n";
        case LineEnding::Unix:
            return "\n";
        case LineEnding::MacOS:
            return "\r";
        default:
            return "\n";
    }
}

DocLangType Buffer::getLangType() const
{
    QMutexLocker locker(&_mutex);
    return _langType;
}

void Buffer::setLangType(DocLangType type)
{
    QMutexLocker locker(&_mutex);

    if (_langType != type) {
        _langType = type;

        // Update Scintilla lexer
        if (_pView) {
            // Map DocLangType to Scintilla lexer
            // This is a simplified mapping - full implementation would map all types
            _pView->setLexer(static_cast<int>(type));
        }

        emit langTypeChanged(type);
    }
}

QString Buffer::getLangTypeName() const
{
    QMutexLocker locker(&_mutex);
    return getLanguageName(_langType);
}

void Buffer::setLangTypeFromFileName(const QString& fileName)
{
    QFileInfo fileInfo(fileName);
    DocLangType detectedLang = detectLanguageFromExtension(fileInfo.suffix());

    if (detectedLang != DocLangType::L_TEXT) {
        setLangType(detectedLang);
    }
}

void Buffer::setLangTypeFromContent()
{
    QByteArray content = getContent();
    DocLangType detectedLang = detectLanguageFromShebang(content);

    if (detectedLang != DocLangType::L_TEXT) {
        setLangType(detectedLang);
    }
}

bool Buffer::isIndentTab() const
{
    QMutexLocker locker(&_mutex);
    return _useTabs;
}

void Buffer::setIndentTab(bool useTab)
{
    QMutexLocker locker(&_mutex);

    _useTabs = useTab;

    if (_pView) {
        _pView->setUseTabs(useTab);
    }
}

int Buffer::getTabWidth() const
{
    QMutexLocker locker(&_mutex);
    return _tabWidth;
}

void Buffer::setTabWidth(int width)
{
    QMutexLocker locker(&_mutex);

    _tabWidth = width;

    if (_pView) {
        _pView->setTabWidth(width);
    }
}

int Buffer::getIndentWidth() const
{
    QMutexLocker locker(&_mutex);
    return _indentWidth;
}

void Buffer::setIndentWidth(int width)
{
    QMutexLocker locker(&_mutex);

    _indentWidth = width;

    if (_pView) {
        _pView->setIndentWidth(width);
    }
}

bool Buffer::needsAutoSave() const
{
    QMutexLocker locker(&_mutex);

    if (!_isDirty || _isUntitled) {
        return false;
    }

    // Check if enough time has passed since last auto-save
    QDateTime now = QDateTime::currentDateTime();
    int secondsSinceLastAutoSave = _lastAutoSaveTime.isValid()
        ? _lastAutoSaveTime.secsTo(now)
        : 999999;

    // Auto-save every 7 seconds (configurable)
    return secondsSinceLastAutoSave >= 7;
}

void Buffer::setAutoSavePoint()
{
    QMutexLocker locker(&_mutex);
    _lastAutoSaveTime = QDateTime::currentDateTime();
}

QString Buffer::getAutoSaveFilePath() const
{
    QMutexLocker locker(&_mutex);

    if (_filePath.isEmpty()) {
        return QString();
    }

    QString autoSaveDir = getAutoSaveDirectory();
    QString fileName = QFileInfo(_filePath).fileName();

    // Create hash of full path to avoid collisions
    QByteArray pathHash = QCryptographicHash::hash(
        _filePath.toUtf8(),
        QCryptographicHash::Md5
    ).toHex().left(8);

    return QString("%1/%2.%3.autosave").arg(autoSaveDir, fileName, QString(pathHash));
}

bool Buffer::recoverFromAutoSave()
{
    QString autoSavePath = getAutoSaveFilePath();

    if (!QFile::exists(autoSavePath)) {
        return false;
    }

    QFile file(autoSavePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray content = file.readAll();
    file.close();

    if (_pView) {
        _pView->clearAll();
        _pView->setText(content);
    }

    _isDirty = true;
    _status = BufferStatus::Dirty;

    emit contentChanged();
    emit statusChanged(_status);
    emit dirtyChanged(true);

    return true;
}

bool Buffer::hasBackup() const
{
    QMutexLocker locker(&_mutex);
    return !_backupFilePath.isEmpty() && QFile::exists(_backupFilePath);
}

QString Buffer::getBackupFilePath() const
{
    QMutexLocker locker(&_mutex);
    return _backupFilePath;
}

bool Buffer::createBackup()
{
    QMutexLocker locker(&_mutex);

    if (_filePath.isEmpty()) {
        return false;
    }

    QByteArray content;
    if (_pView) {
        content = _pView->getTextRange(0, _pView->getLength());
    }

    if (_backupFilePath.isEmpty()) {
        _backupFilePath = generateBackupFilePath();
    }

    return writeToFile(_backupFilePath, content);
}

bool Buffer::restoreFromBackup()
{
    if (_backupFilePath.isEmpty() || !QFile::exists(_backupFilePath)) {
        return false;
    }

    return loadFromFile(_backupFilePath);
}

void Buffer::removeBackup()
{
    QMutexLocker locker(&_mutex);

    if (!_backupFilePath.isEmpty() && QFile::exists(_backupFilePath)) {
        QFile::remove(_backupFilePath);
        _backupFilePath.clear();
    }
}

void Buffer::setFileMonitoringEnabled(bool enabled)
{
    QMutexLocker locker(&_mutex);

    _fileMonitoringEnabled = enabled;

    if (enabled) {
        setupFileWatcher();
    } else {
        removeFileWatcher();
    }
}

bool Buffer::isFileMonitoringEnabled() const
{
    QMutexLocker locker(&_mutex);
    return _fileMonitoringEnabled;
}

void Buffer::onFileChanged()
{
    QMutexLocker locker(&_mutex);

    if (_isUntitled || !_fileMonitoringEnabled) {
        return;
    }

    // Check if file has been modified externally
    QFileInfo fileInfo(_filePath);
    if (fileInfo.exists() && fileInfo.lastModified() > _lastSavedTime) {
        _status = BufferStatus::ModifiedOutside;
        emit fileModifiedExternally();
        emit statusChanged(_status);
    }
}

QDateTime Buffer::getLastModifiedTime() const
{
    QMutexLocker locker(&_mutex);
    return _lastModifiedTime;
}

void Buffer::setLastModifiedTime(const QDateTime& time)
{
    QMutexLocker locker(&_mutex);
    _lastModifiedTime = time;
}

QDateTime Buffer::getLastSavedTime() const
{
    QMutexLocker locker(&_mutex);
    return _lastSavedTime;
}

void Buffer::updateLastSavedTime()
{
    QMutexLocker locker(&_mutex);
    _lastSavedTime = QDateTime::currentDateTime();
}

void Buffer::savePosition()
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        _savedPosition.pos = _pView->getCurrentPos();
        _savedPosition.anchor = _pView->getAnchor();
        _savedPosition.line = _pView->getCurrentLine();
        _savedPosition.column = getCurrentColumn();
        _savedPosition.firstVisibleLine = _pView->getFirstVisibleLine();
        _savedPosition.xOffset = _pView->getXOffset();
    }
}

void Buffer::restorePosition()
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        _pView->setSelection(_savedPosition.anchor, _savedPosition.pos);
        _pView->setFirstVisibleLine(_savedPosition.firstVisibleLine);
        _pView->setXOffset(_savedPosition.xOffset);
    }
}

void Buffer::saveSelection()
{
    savePosition();
}

void Buffer::restoreSelection()
{
    restorePosition();
}

MapPosition Buffer::getMapPosition() const
{
    QMutexLocker locker(&_mutex);
    return _mapPosition;
}

void Buffer::setMapPosition(const MapPosition& pos)
{
    QMutexLocker locker(&_mutex);
    _mapPosition = pos;
}

bool Buffer::isLargeFile() const
{
    QMutexLocker locker(&_mutex);
    return _isLargeFile;
}

void Buffer::setLargeFile(bool isLarge)
{
    QMutexLocker locker(&_mutex);
    _isLargeFile = isLarge;
}

bool Buffer::isPinned() const
{
    QMutexLocker locker(&_mutex);
    return _isPinned;
}

void Buffer::setPinned(bool pinned)
{
    QMutexLocker locker(&_mutex);
    _isPinned = pinned;
}

bool Buffer::isRTL() const
{
    QMutexLocker locker(&_mutex);
    return _isRTL;
}

void Buffer::setRTL(bool rtl)
{
    QMutexLocker locker(&_mutex);
    _isRTL = rtl;
}

int Buffer::getDocColorId() const
{
    QMutexLocker locker(&_mutex);
    return _docColorId;
}

void Buffer::setDocColorId(int colorId)
{
    QMutexLocker locker(&_mutex);
    _docColorId = colorId;
}

bool Buffer::checkFileState()
{
    QMutexLocker locker(&_mutex);

    if (_isUntitled || _filePath.isEmpty()) {
        return false;
    }

    QFileInfo fileInfo(_filePath);

    if (!fileInfo.exists()) {
        // File was deleted externally
        _status = BufferStatus::Dirty;
        emit fileModifiedExternally();
        return true;
    }

    if (fileInfo.lastModified() > _lastSavedTime) {
        // File was modified externally
        _status = BufferStatus::ModifiedOutside;
        _lastModifiedTime = fileInfo.lastModified();
        emit fileModifiedExternally();
        return true;
    }

    // Check read-only status
    bool newReadOnly = !fileInfo.isWritable();
    if (_isFileReadOnly != newReadOnly) {
        _isFileReadOnly = newReadOnly;
        emit readOnlyChanged(isReadOnly());
        return true;
    }

    return false;
}

QString Buffer::getCommentLineSymbol() const
{
    // Return comment symbols based on language type
    switch (_langType) {
        case DocLangType::L_CPP:
        case DocLangType::L_C:
        case DocLangType::L_JAVA:
        case DocLangType::L_CS:
        case DocLangType::L_OBJC:
        case DocLangType::L_JS_EMBEDDED:
        case DocLangType::L_JAVASCRIPT:
        case DocLangType::L_TYPESCRIPT:
        case DocLangType::L_RUST:
        case DocLangType::L_SWIFT:
        case DocLangType::L_GO:
            return "//";
        case DocLangType::L_PYTHON:
        case DocLangType::L_RUBY:
        case DocLangType::L_PERL:
        case DocLangType::L_BASH:
        case DocLangType::L_MAKEFILE:
        case DocLangType::L_YAML:
            return "#";
        case DocLangType::L_SQL:
        case DocLangType::L_LUA:
            return "--";
        case DocLangType::L_LISP:
        case DocLangType::L_SCHEME:
            return ";";
        case DocLangType::L_HTML:
        case DocLangType::L_XML:
        case DocLangType::L_MARKDOWN:
            return "<!--";  // Block comment
        case DocLangType::L_BATCH:
            return "REM";
        case DocLangType::L_VB:
        case DocLangType::L_VBSCRIPT:
            return "'";
        case DocLangType::L_PASCAL:
        case DocLangType::L_ADA:
        case DocLangType::L_INNO:
            return "//";
        case DocLangType::L_FORTRAN:
        case DocLangType::L_FORTRAN_77:
            return "!";
        case DocLangType::L_MATLAB:
            return "%";
        case DocLangType::L_LATEX:
            return "%";
        case DocLangType::L_ASM:
            return ";";
        default:
            return QString();
    }
}

QString Buffer::getCommentStart() const
{
    // Return block comment start based on language type
    switch (_langType) {
        case DocLangType::L_CPP:
        case DocLangType::L_C:
        case DocLangType::L_JAVA:
        case DocLangType::L_CS:
        case DocLangType::L_OBJC:
        case DocLangType::L_JS_EMBEDDED:
        case DocLangType::L_JAVASCRIPT:
        case DocLangType::L_TYPESCRIPT:
        case DocLangType::L_RUST:
        case DocLangType::L_SWIFT:
        case DocLangType::L_GO:
            return "/*";
        case DocLangType::L_HTML:
        case DocLangType::L_XML:
        case DocLangType::L_MARKDOWN:
            return "<!--";
        case DocLangType::L_PASCAL:
        case DocLangType::L_ADA:
            return "(*";
        case DocLangType::L_HASKELL:
            return "{-";
        default:
            return QString();
    }
}

QString Buffer::getCommentEnd() const
{
    // Return block comment end based on language type
    switch (_langType) {
        case DocLangType::L_CPP:
        case DocLangType::L_C:
        case DocLangType::L_JAVA:
        case DocLangType::L_CS:
        case DocLangType::L_OBJC:
        case DocLangType::L_JS_EMBEDDED:
        case DocLangType::L_JAVASCRIPT:
        case DocLangType::L_TYPESCRIPT:
        case DocLangType::L_RUST:
        case DocLangType::L_SWIFT:
        case DocLangType::L_GO:
            return "*/";
        case DocLangType::L_HTML:
        case DocLangType::L_XML:
        case DocLangType::L_MARKDOWN:
            return "-->";
        case DocLangType::L_PASCAL:
        case DocLangType::L_ADA:
            return "*)";
        case DocLangType::L_HASKELL:
            return "-}";
        default:
            return QString();
    }
}

void Buffer::onFileSystemChanged(const QString& path)
{
    if (path == _filePath) {
        onFileChanged();
    }
}

void Buffer::setupFileWatcher()
{
    if (!_fileMonitoringEnabled || _filePath.isEmpty() || _isUntitled) {
        return;
    }

    if (!_fileWatcher) {
        _fileWatcher = new QFileSystemWatcher(this);
        connect(_fileWatcher, &QFileSystemWatcher::fileChanged,
                this, &Buffer::onFileSystemChanged);
    }

    if (!_fileWatcher->files().contains(_filePath)) {
        _fileWatcher->addPath(_filePath);
    }
}

void Buffer::removeFileWatcher()
{
    if (_fileWatcher && !_filePath.isEmpty()) {
        if (_fileWatcher->files().contains(_filePath)) {
            _fileWatcher->removePath(_filePath);
        }
    }
}

bool Buffer::writeToFile(const QString& filePath, const QByteArray& content)
{
    QFile file(filePath);

    // Ensure directory exists
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Failed to open file for writing:" << filePath << "-" << file.errorString();
        return false;
    }

    qint64 written = file.write(content);
    file.close();

    if (written != content.size()) {
        qWarning() << "Failed to write complete content to:" << filePath;
        return false;
    }

    return true;
}

QByteArray Buffer::readFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    QByteArray content = file.readAll();
    file.close();

    return content;
}

QString Buffer::generateBackupFilePath() const
{
    QString backupDir = getBackupDirectory();
    QString fileName = QFileInfo(_filePath).fileName();

    // Create timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmmss");

    // Create hash of full path to avoid collisions
    QByteArray pathHash = QCryptographicHash::hash(
        _filePath.toUtf8(),
        QCryptographicHash::Md5
    ).toHex().left(8);

    return QString("%1/%2@%3.%4.bak").arg(backupDir, fileName, timestamp, QString(pathHash));
}

DocLangType Buffer::detectLanguageFromFileName(const QString& fileName) const
{
    QFileInfo fileInfo(fileName);
    return detectLanguageFromExtension(fileInfo.suffix());
}

DocLangType Buffer::detectLanguageFromContent(const QByteArray& content) const
{
    return detectLanguageFromShebang(content);
}

// ============================================================================
// BufferManager implementation
// ============================================================================

BufferManager* BufferManager::getInstance()
{
    static BufferManager instance;
    return &instance;
}

BufferManager::BufferManager()
    : QObject(nullptr)
{
}

BufferManager::~BufferManager()
{
    QMutexLocker locker(&_mutex);

    // Clean up all buffers
    for (Buffer* buffer : _buffers) {
        delete buffer;
    }
    _buffers.clear();
}

Buffer* BufferManager::createBuffer()
{
    QMutexLocker locker(&_mutex);

    Buffer* buffer = new Buffer(this);
    buffer->setID(_nextBufferID++);

    _buffers.append(buffer);

    emit bufferCreated(buffer);

    // If this is the first buffer, set it as current
    if (_buffers.size() == 1) {
        _currentIndex = 0;
        emit currentBufferChanged(buffer);
    }

    return buffer;
}

void BufferManager::deleteBuffer(Buffer* buffer)
{
    QMutexLocker locker(&_mutex);

    if (!buffer) {
        return;
    }

    int index = _buffers.indexOf(buffer);
    if (index == -1) {
        return;
    }

    _buffers.removeAt(index);

    // Update current index if needed
    if (index < _currentIndex) {
        _currentIndex--;
    } else if (index == _currentIndex) {
        // Current buffer was deleted
        if (_buffers.isEmpty()) {
            _currentIndex = -1;
        } else if (_currentIndex >= _buffers.size()) {
            _currentIndex = _buffers.size() - 1;
        }

        if (_currentIndex >= 0) {
            emit currentBufferChanged(_buffers[_currentIndex]);
        } else {
            emit currentBufferChanged(nullptr);
        }
    }

    emit bufferDeleted(buffer);

    // Check if we still have dirty buffers
    bool hasDirty = hasDirtyBuffers();
    emit buffersModified(hasDirty);

    delete buffer;
}

Buffer* BufferManager::getBufferByID(int id)
{
    QMutexLocker locker(&_mutex);

    for (Buffer* buffer : _buffers) {
        if (buffer->getID() == id) {
            return buffer;
        }
    }

    return nullptr;
}

Buffer* BufferManager::getBufferByFilePath(const QString& filePath)
{
    QMutexLocker locker(&_mutex);

    QString canonicalPath = QFileInfo(filePath).canonicalFilePath();

    for (Buffer* buffer : _buffers) {
        if (QFileInfo(buffer->getFilePath()).canonicalFilePath() == canonicalPath) {
            return buffer;
        }
    }

    return nullptr;
}

QList<Buffer*> BufferManager::getAllBuffers() const
{
    QMutexLocker locker(&_mutex);
    return _buffers;
}

int BufferManager::getBufferCount() const
{
    QMutexLocker locker(&_mutex);
    return _buffers.size();
}

int BufferManager::getCurrentBufferIndex() const
{
    QMutexLocker locker(&_mutex);
    return _currentIndex;
}

void BufferManager::setCurrentBufferIndex(int index)
{
    QMutexLocker locker(&_mutex);

    if (index < 0 || index >= _buffers.size()) {
        return;
    }

    if (_currentIndex != index) {
        _currentIndex = index;
        emit currentBufferChanged(_buffers[index]);
    }
}

Buffer* BufferManager::getCurrentBuffer() const
{
    QMutexLocker locker(&_mutex);

    if (_currentIndex >= 0 && _currentIndex < _buffers.size()) {
        return _buffers[_currentIndex];
    }

    return nullptr;
}

bool BufferManager::hasDirtyBuffers() const
{
    QMutexLocker locker(&_mutex);

    for (Buffer* buffer : _buffers) {
        if (buffer->isDirty()) {
            return true;
        }
    }

    return false;
}

QList<Buffer*> BufferManager::getDirtyBuffers() const
{
    QMutexLocker locker(&_mutex);

    QList<Buffer*> dirtyBuffers;

    for (Buffer* buffer : _buffers) {
        if (buffer->isDirty()) {
            dirtyBuffers.append(buffer);
        }
    }

    return dirtyBuffers;
}

void BufferManager::saveAllBuffers()
{
    QMutexLocker locker(&_mutex);

    for (Buffer* buffer : _buffers) {
        if (buffer->isDirty() && !buffer->isUntitled()) {
            buffer->saveToFile(buffer->getFilePath());
        }
    }
}

void BufferManager::closeAllBuffers()
{
    QMutexLocker locker(&_mutex);

    // Create a copy of the list since deleteBuffer modifies _buffers
    QList<Buffer*> buffersToDelete = _buffers;

    for (Buffer* buffer : buffersToDelete) {
        deleteBuffer(buffer);
    }
}

QString BufferManager::getNextUntitledName() const
{
    QMutexLocker locker(&_mutex);

    int maxNumber = 0;

    for (Buffer* buffer : _buffers) {
        if (buffer->isUntitled()) {
            QString fileName = buffer->getFileName();
            // Parse "new X" format
            QRegularExpression re("new\\s+(\\d+)");
            QRegularExpressionMatch match = re.match(fileName);
            if (match.hasMatch()) {
                int num = match.captured(1).toInt();
                if (num > maxNumber) {
                    maxNumber = num;
                }
            }
        }
    }

    return QString("new %1").arg(maxNumber + 1);
}

} // namespace QtCore
