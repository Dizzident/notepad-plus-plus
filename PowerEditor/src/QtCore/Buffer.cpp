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

// Include ScintillaEditView for the real class definition
#include "ScintillaEditView.h"
#include "Scintilla.h"  // For SCI_* message constants

namespace QtCore {

// ============================================================================
// Static helper functions
// ============================================================================

static QString getBackupDirectory()
{
    QString backupDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (backupDir.isEmpty()) {
        // Fallback to home directory/.local/share
        const char* home = getenv("HOME");
        if (home) {
            backupDir = QString::fromUtf8(home) + "/.local/share/notepad-plus-plus";
        } else {
            backupDir = "/tmp/notepad-plus-plus";
        }
    }
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
    if (autoSaveDir.isEmpty()) {
        // Fallback to home directory/.local/share
        const char* home = getenv("HOME");
        if (home) {
            autoSaveDir = QString::fromUtf8(home) + "/.local/share/notepad-plus-plus";
        } else {
            autoSaveDir = "/tmp/notepad-plus-plus";
        }
    }
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
        { "txt", L_TEXT },
        { "php", L_PHP },
        { "php3", L_PHP },
        { "php4", L_PHP },
        { "php5", L_PHP },
        { "phtml", L_PHP },
        { "c", L_C },
        { "h", L_C },
        { "cpp", L_CPP },
        { "cxx", L_CPP },
        { "cc", L_CPP },
        { "hpp", L_CPP },
        { "hxx", L_CPP },
        { "cs", L_CS },
        { "m", L_OBJC },
        { "mm", L_OBJC },
        { "java", L_JAVA },
        { "rc", L_RC },
        { "html", L_HTML },
        { "htm", L_HTML },
        { "shtml", L_HTML },
        { "xml", L_XML },
        { "xaml", L_XML },
        { "xsl", L_XML },
        { "xslt", L_XML },
        { "mak", L_MAKEFILE },
        { "makefile", L_MAKEFILE },
        { "pas", L_PASCAL },
        { "pp", L_PASCAL },
        { "inc", L_PASCAL },
        { "bat", L_BATCH },
        { "cmd", L_BATCH },
        { "nt", L_BATCH },
        { "ini", L_INI },
        { "inf", L_INI },
        { "reg", L_REGISTRY },
        { "cfg", L_INI },
        { "conf", L_INI },
        { "sql", L_SQL },
        { "vb", L_VB },
        { "vbs", L_VB },
        { "bas", L_VB },
        { "frm", L_VB },
        { "cls", L_VB },
        { "js", L_JAVASCRIPT },
        { "json", L_JSON },
        { "css", L_CSS },
        { "pl", L_PERL },
        { "pm", L_PERL },
        { "py", L_PYTHON },
        { "pyw", L_PYTHON },
        { "lua", L_LUA },
        { "tex", L_LATEX },
        { "latex", L_LATEX },
        { "f", L_FORTRAN },
        { "for", L_FORTRAN },
        { "f90", L_FORTRAN },
        { "f95", L_FORTRAN },
        { "f2k", L_FORTRAN },
        { "sh", L_BASH },
        { "bash", L_BASH },
        { "zsh", L_BASH },
        { "rb", L_RUBY },
        { "rbw", L_RUBY },
        { "rake", L_RUBY },
        { "gemspec", L_RUBY },
        { "tcl", L_TCL },
        { "tk", L_TCL },
        { "lisp", L_LISP },
        { "lsp", L_LISP },
        { "scm", L_SCHEME },
        { "ss", L_SCHEME },
        { "asm", L_ASM },
        { "s", L_ASM },
        { "nasm", L_ASM },
        { "diff", L_DIFF },
        { "patch", L_DIFF },
        { "props", L_PROPS },
        { "properties", L_PROPS },
        { "ps", L_PS },
        { "yaml", L_YAML },
        { "yml", L_YAML },
        { "cmake", L_CMAKE },
        { "md", L_TEXT },
        { "markdown", L_TEXT },
        { "rs", L_RUST },
        { "go", L_TEXT },  // Go not in original enum, map to text
        { "ts", L_TYPESCRIPT },
        { "tsx", L_TYPESCRIPT },
        { "coffee", L_COFFEESCRIPT },
        { "ps1", L_POWERSHELL },
        { "psm1", L_POWERSHELL },
        { "psd1", L_POWERSHELL },
        { "r", L_R },
        { "swift", L_SWIFT },
        { "kt", L_TEXT },  // Kotlin not in original enum
        { "scala", L_TEXT },  // Scala not in original enum
        { "gd", L_GDSCRIPT },
    };

    QString lowerExt = ext.toLower();
    return extMap.value(lowerExt, L_TEXT);
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
                return L_PYTHON;
            } else if (firstLine.contains("perl")) {
                return L_PERL;
            } else if (firstLine.contains("ruby")) {
                return L_RUBY;
            } else if (firstLine.contains("bash") || firstLine.contains("sh")) {
                return L_BASH;
            } else if (firstLine.contains("php")) {
                return L_PHP;
            } else if (firstLine.contains("node")) {
                return L_JAVASCRIPT;
            }
        }
    }

    // Check for XML declaration
    if (content.trimmed().startsWith("<?xml")) {
        return L_XML;
    }

    // Check for HTML doctype
    QByteArray trimmed = content.trimmed().toLower();
    if (trimmed.startsWith("<!doctype html")) {
        return L_HTML;
    }
    if (trimmed.startsWith("<html")) {
        return L_HTML;
    }

    return L_TEXT;
}

static QString getLanguageName(DocLangType type)
{
    static const QHash<DocLangType, QString> langNames = {
        { L_TEXT, QObject::tr("Normal text") },
        { L_PHP, QObject::tr("PHP") },
        { L_C, QObject::tr("C") },
        { L_CPP, QObject::tr("C++") },
        { L_CS, QObject::tr("C#") },
        { L_OBJC, QObject::tr("Objective-C") },
        { L_JAVA, QObject::tr("Java") },
        { L_RC, QObject::tr("Resource file") },
        { L_HTML, QObject::tr("HTML") },
        { L_XML, QObject::tr("XML") },
        { L_MAKEFILE, QObject::tr("Makefile") },
        { L_PASCAL, QObject::tr("Pascal") },
        { L_BATCH, QObject::tr("Batch") },
        { L_INI, QObject::tr("INI file") },
        { L_ASCII, QObject::tr("ASCII") },
        { L_USER, QObject::tr("User defined") },
        { L_SQL, QObject::tr("SQL") },
        { L_VB, QObject::tr("Visual Basic") },
        { L_JAVASCRIPT, QObject::tr("JavaScript") },
        { L_CSS, QObject::tr("CSS") },
        { L_PERL, QObject::tr("Perl") },
        { L_PYTHON, QObject::tr("Python") },
        { L_LUA, QObject::tr("Lua") },
        { L_TEX, QObject::tr("TeX") },
        { L_FORTRAN, QObject::tr("Fortran") },
        { L_BASH, QObject::tr("Shell") },
        { L_RUBY, QObject::tr("Ruby") },
        { L_YAML, QObject::tr("YAML") },
        { L_JSON, QObject::tr("JSON") },
        { L_TEXT, QObject::tr("Markdown") },
        { L_RUST, QObject::tr("Rust") },
        { L_TYPESCRIPT, QObject::tr("TypeScript") },
        { L_POWERSHELL, QObject::tr("PowerShell") },
        { L_CMAKE, QObject::tr("CMake") },
        { L_GDSCRIPT, QObject::tr("GDScript") },
    };

    return langNames.value(type, QObject::tr("Unknown"));
}

// ============================================================================
// Buffer implementation
// ============================================================================

Buffer::Buffer(QObject* parent)
    : QObject(parent)
{
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

void Buffer::setScintillaView(::ScintillaEditView* view)
{
    QMutexLocker locker(&_mutex);
    _pView = view;
}

void Buffer::setID(BufferID id)
{
    _id = id;
}

BufferID Buffer::getID() const
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

    // Detect encoding from BOM
    QString detectedEncoding = detectEncodingFromBOM(content);
    if (!detectedEncoding.isEmpty()) {
        _encoding = detectedEncoding;
        _useBOM = true;
    } else {
        // Default to UTF-8 if no BOM detected
        _encoding = "UTF-8";
        _useBOM = false;
    }

    // Detect line endings
    _lineEnding = detectLineEnding(content);

    // Detect language from file name
    _langType = detectLanguageFromFileName(fileInfo.suffix());
    if (_langType == L_TEXT) {
        _langType = detectLanguageFromShebang(content);
    }

    // Set file path (this will also update _fileName and _isUntitled)
    setFilePath(filePath);

    // Set content in Scintilla view
    if (_pView) {
        _pView->execute(SCI_CLEARALL);
        _pView->execute(SCI_APPENDTEXT, static_cast<WPARAM>(content.size()), reinterpret_cast<LPARAM>(content.constData()));
        _pView->execute(SCI_SETSAVEPOINT);
        _pView->execute(SCI_EMPTYUNDOBUFFER);
    }

    // Update status
    _isDirty = false;
    _status = DOC_REGULAR;
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
        int length = static_cast<int>(_pView->execute(SCI_GETLENGTH));
        content.resize(length);
        if (length > 0) {
            _pView->getText(content.data(), 0, length);
        }
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
    _status = DOC_REGULAR;

    if (_pView) {
        _pView->execute(SCI_SETSAVEPOINT);
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

const wchar_t* Buffer::getFileName() const
{
    QMutexLocker locker(&_mutex);
    // Return the filename as wchar_t* for Windows API compatibility
    return reinterpret_cast<const wchar_t*>(_fileName.utf16());
}

QString Buffer::getFileNameQString() const
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
        if (detectedLang != L_TEXT && detectedLang != _langType) {
            _langType = detectedLang;
            emit langTypeChanged(_langType);
        }
    }

    emit filePathChanged(path);
}

void Buffer::setFileName(const wchar_t* fileName)
{
    if (fileName) {
        setFilePath(QString::fromWCharArray(fileName));
    }
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
        int length = static_cast<int>(_pView->execute(SCI_GETLENGTH));
        QByteArray content(length, Qt::Uninitialized);
        if (length > 0) {
            _pView->getText(content.data(), 0, length);
        }
        return content;
    }

    return QByteArray();
}

void Buffer::setContent(const QByteArray& content)
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        _pView->execute(SCI_CLEARALL);
        _pView->execute(SCI_APPENDTEXT, static_cast<WPARAM>(content.size()), reinterpret_cast<LPARAM>(content.constData()));
    }

    _isDirty = true;
    _status = DOC_MODIFIED;

    emit contentChanged();
    emit statusChanged(_status);
    emit dirtyChanged(true);
}

QString Buffer::getText() const
{
    QByteArray content = getContent();
    auto encoding = QStringDecoder::encodingForName(_encoding.toUtf8().constData())
                        .value_or(QStringDecoder::Utf8);
    QStringDecoder decoder(encoding);
    return decoder.decode(content);
}

void Buffer::setText(const QString& text)
{
    auto encoding = QStringEncoder::encodingForName(_encoding.toUtf8().constData())
                        .value_or(QStringEncoder::Utf8);
    QStringEncoder encoder(encoding);
    QByteArray content = encoder.encode(text);
    setContent(content);
}

int Buffer::getLineCount() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        return static_cast<int>(_pView->execute(SCI_GETLINECOUNT));
    }

    return 1;
}

int Buffer::getCharCount() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        return static_cast<int>(_pView->execute(SCI_GETLENGTH));
    }

    return 0;
}

size_t Buffer::docLength() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        return static_cast<size_t>(_pView->execute(SCI_GETLENGTH));
    }

    return 0;
}

int Buffer::getWordCount() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        // Get text and count words
        int length = static_cast<int>(_pView->execute(SCI_GETLENGTH));
        if (length > 0) {
            QByteArray text(length, Qt::Uninitialized);
            _pView->getText(text.data(), 0, length);
            QString str = QString::fromUtf8(text);
            QStringList words = str.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            return words.count();
        }
        return 0;
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
        return static_cast<int>(_pView->execute(SCI_GETCURRENTPOS));
    }

    return 0;
}

int Buffer::getCurrentLine() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        int pos = static_cast<int>(_pView->execute(SCI_GETCURRENTPOS));
        return static_cast<int>(_pView->execute(SCI_LINEFROMPOSITION, pos));
    }

    return 0;
}

int Buffer::getCurrentColumn() const
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        int pos = static_cast<int>(_pView->execute(SCI_GETCURRENTPOS));
        return static_cast<int>(_pView->execute(SCI_GETCOLUMN, pos));
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
        _status = dirty ? DOC_MODIFIED : DOC_REGULAR;

        emit dirtyChanged(dirty);
        emit statusChanged(_status);
    }
}

DocFileStatus Buffer::getStatus() const
{
    QMutexLocker locker(&_mutex);
    return _status;
}

void Buffer::setStatus(DocFileStatus status)
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
            _pView->execute(SCI_SETREADONLY, isReadOnly());
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
            _pView->execute(SCI_SETREADONLY, isReadOnly());
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
        // Validate encoding using QStringDecoder::encodingForName
        auto encodingOpt = QStringDecoder::encodingForName(encoding.toUtf8().constData());
        if (encodingOpt.has_value()) {
            _encoding = encoding;
            emit encodingChanged(encoding);
        }
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

QString Buffer::detectEncodingFromBOM(const QByteArray& content) {
    if (content.size() >= 3) {
        if (static_cast<unsigned char>(content[0]) == 0xEF &&
            static_cast<unsigned char>(content[1]) == 0xBB &&
            static_cast<unsigned char>(content[2]) == 0xBF) {
            return "UTF-8";
        }
    }
    if (content.size() >= 2) {
        if (static_cast<unsigned char>(content[0]) == 0xFE &&
            static_cast<unsigned char>(content[1]) == 0xFF) {
            return "UTF-16 BE";
        }
        if (static_cast<unsigned char>(content[0]) == 0xFF &&
            static_cast<unsigned char>(content[1]) == 0xFE) {
            return "UTF-16 LE";
        }
    }
    return QString();  // No BOM detected
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
            _pView->execute(SCI_CONVERTEOLS, sciEolMode);
            _pView->execute(SCI_SETEOLMODE, sciEolMode);
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
            // Note: Lexer setup is handled by ScintillaEditView::defineDocType
            // which calls SCI_SETILEXER with the appropriate lexer
            // This is a simplified placeholder - the actual lexer setup
            // should be done through the ScintillaEditView interface
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

    if (detectedLang != L_TEXT) {
        setLangType(detectedLang);
    }
}

void Buffer::setLangTypeFromContent()
{
    QByteArray content = getContent();
    DocLangType detectedLang = detectLanguageFromShebang(content);

    if (detectedLang != L_TEXT) {
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
        _pView->execute(SCI_SETUSETABS, useTab);
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
        _pView->execute(SCI_SETTABWIDTH, width);
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
        _pView->execute(SCI_SETINDENT, width);
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
        _pView->execute(SCI_CLEARALL);
        _pView->execute(SCI_APPENDTEXT, static_cast<WPARAM>(content.size()), reinterpret_cast<LPARAM>(content.constData()));
    }

    _isDirty = true;
    _status = DOC_MODIFIED;

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
        int length = static_cast<int>(_pView->execute(SCI_GETLENGTH));
        content.resize(length);
        if (length > 0) {
            _pView->getText(content.data(), 0, length);
        }
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
        _status = DOC_MODIFIED;
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
        _savedPosition._startPos = static_cast<int>(_pView->execute(SCI_GETCURRENTPOS));
        _savedPosition._endPos = static_cast<int>(_pView->execute(SCI_GETANCHOR));
        int pos = static_cast<int>(_pView->execute(SCI_GETCURRENTPOS));
        _savedPosition._firstVisibleLine = static_cast<int>(_pView->execute(SCI_LINEFROMPOSITION, pos));
        _savedPosition._xOffset = static_cast<int>(_pView->execute(SCI_GETXOFFSET));
    }
}

void Buffer::restorePosition()
{
    QMutexLocker locker(&_mutex);

    if (_pView) {
        _pView->execute(SCI_SETSEL, _savedPosition._startPos, _savedPosition._endPos);
        _pView->execute(SCI_SETFIRSTVISIBLELINE, _savedPosition._firstVisibleLine);
        _pView->execute(SCI_SETXOFFSET, _savedPosition._xOffset);
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
        _status = DOC_MODIFIED;
        emit fileModifiedExternally();
        return true;
    }

    if (fileInfo.lastModified() > _lastSavedTime) {
        // File was modified externally
        _status = DOC_MODIFIED;
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
        case L_CPP:
        case L_C:
        case L_JAVA:
        case L_CS:
        case L_OBJC:
        case L_JS_EMBEDDED:
        case L_JAVASCRIPT:
        case L_TYPESCRIPT:
        case L_RUST:
        case L_SWIFT:
        case L_GOLANG:
            return "//";
        case L_PYTHON:
        case L_RUBY:
        case L_PERL:
        case L_BASH:
        case L_MAKEFILE:
        case L_YAML:
            return "#";
        case L_SQL:
        case L_LUA:
            return "--";
        case L_LISP:
        case L_SCHEME:
            return ";";
        case L_HTML:
        case L_XML:
        case L_TEXT:
            return "<!--";  // Block comment
        case L_BATCH:
            return "REM";
        case L_VB:
            return "'";
        case L_PASCAL:
        case L_ADA:
        case L_INNO:
            return "//";
        case L_FORTRAN:
        case L_FORTRAN_77:
            return "!";
        case L_MATLAB:
            return "%";
        case L_LATEX:
            return "%";
        case L_ASM:
            return ";";
        default:
            return QString();
    }
}

QString Buffer::getCommentStart() const
{
    // Return block comment start based on language type
    switch (_langType) {
        case L_CPP:
        case L_C:
        case L_JAVA:
        case L_CS:
        case L_OBJC:
        case L_JS_EMBEDDED:
        case L_JAVASCRIPT:
        case L_TYPESCRIPT:
        case L_RUST:
        case L_SWIFT:
        case L_GOLANG:
            return "/*";
        case L_HTML:
        case L_XML:
        case L_TEXT:
            return "<!--";
        case L_PASCAL:
        case L_ADA:
            return "(*";
        case L_HASKELL:
            return "{-";
        default:
            return QString();
    }
}

QString Buffer::getCommentEnd() const
{
    // Return block comment end based on language type
    switch (_langType) {
        case L_CPP:
        case L_C:
        case L_JAVA:
        case L_CS:
        case L_OBJC:
        case L_JS_EMBEDDED:
        case L_JAVASCRIPT:
        case L_TYPESCRIPT:
        case L_RUST:
        case L_SWIFT:
        case L_GOLANG:
            return "*/";
        case L_HTML:
        case L_XML:
        case L_TEXT:
            return "-->";
        case L_PASCAL:
        case L_ADA:
            return "*)";
        case L_HASKELL:
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
    buffer->setID(buffer);  // BufferID is Buffer*, use pointer as ID

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

Buffer* BufferManager::getBufferByID(BufferID id)
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
            QString fileName = buffer->getFileNameQString();
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

// ============================================================================
// Compatibility methods for ScintillaEditView integration
// ============================================================================

void Buffer::setHideLineChanged(bool isHide, size_t location)
{
    QMutexLocker locker(&_mutex);
    // Stub implementation - hide line tracking would be implemented here
    Q_UNUSED(isHide)
    Q_UNUSED(location)
}

void Buffer::setHeaderLineState(const std::vector<size_t>& folds, void* identifier)
{
    QMutexLocker locker(&_mutex);
    // Find or create entry for this view identifier
    for (size_t i = 0; i < _viewIdentifiers.size(); ++i) {
        if (_viewIdentifiers[i] == identifier) {
            if (i < _foldStates.size()) {
                _foldStates[i] = folds;
            }
            return;
        }
    }
    // New view identifier
    _viewIdentifiers.push_back(identifier);
    _foldStates.push_back(folds);
}

std::vector<size_t> Buffer::getHeaderLineState(void* identifier) const
{
    QMutexLocker locker(&_mutex);
    for (size_t i = 0; i < _viewIdentifiers.size(); ++i) {
        if (_viewIdentifiers[i] == identifier && i < _foldStates.size()) {
            return _foldStates[i];
        }
    }
    return std::vector<size_t>();
}

void* Buffer::getDocument() const
{
    QMutexLocker locker(&_mutex);
    // Return the document pointer (would be Scintilla document in full implementation)
    return _document;
}

::UniMode Buffer::getUnicodeMode() const
{
    QMutexLocker locker(&_mutex);
    return _unicodeMode;
}

void Buffer::setUnicodeMode(::UniMode mode)
{
    QMutexLocker locker(&_mutex);
    _unicodeMode = mode;
}

Buffer::EolType Buffer::getEolFormat() const
{
    QMutexLocker locker(&_mutex);
    // Convert LineEnding to EolType
    switch (_lineEnding) {
        case LineEnding::Windows:
            return eolWindows;
        case LineEnding::Unix:
            return eolUnix;
        case LineEnding::MacOS:
            return eolMac;
        default:
            return eolUnix;
    }
}

void Buffer::setEolFormat(EolType format)
{
    QMutexLocker locker(&_mutex);
    _eolFormat = format;
    // Convert EolType to LineEnding
    switch (format) {
        case eolWindows:
            _lineEnding = LineEnding::Windows;
            break;
        case eolUnix:
            _lineEnding = LineEnding::Unix;
            break;
        case eolMac:
            _lineEnding = LineEnding::MacOS;
            break;
        default:
            _lineEnding = LineEnding::Unix;
            break;
    }
}

bool Buffer::getNeedsLexing() const
{
    QMutexLocker locker(&_mutex);
    return _needsLexing;
}

void Buffer::setNeedsLexing(bool needs)
{
    QMutexLocker locker(&_mutex);
    _needsLexing = needs;
}

// ============================================================================
// File Monitoring
// ============================================================================

bool Buffer::isMonitoringOn() const
{
    QMutexLocker locker(&_mutex);
    return _isMonitoringOn;
}

void Buffer::startMonitoring()
{
    QMutexLocker locker(&_mutex);
    _isMonitoringOn = true;
    if (_fileWatcher && !_filePath.isEmpty()) {
        _fileWatcher->addPath(_filePath);
    }
}

void Buffer::stopMonitoring()
{
    QMutexLocker locker(&_mutex);
    _isMonitoringOn = false;
    if (_fileWatcher && !_filePath.isEmpty()) {
        _fileWatcher->removePath(_filePath);
    }
}

// ============================================================================
// FileManager implementation - Compatibility layer for Notepad++ core
// ============================================================================

FileManager* FileManager::getInstance()
{
    static FileManager instance;
    return &instance;
}

Buffer* FileManager::getBufferByID(Buffer* id)
{
    // id is the Buffer pointer itself
    return id;
}

Buffer* FileManager::newEmptyDocument()
{
    // Create a new buffer through the BufferManager
    BufferManager* mgr = BufferManager::getInstance();
    Buffer* buf = mgr->createBuffer();
    return buf;
}

// ============================================================================
// Additional Buffer compatibility methods
// ============================================================================

void Buffer::setPosition(const Position& pos, void* identifier)
{
    QMutexLocker locker(&_mutex);
    // Store position for the given view identifier
    // This is a simplified implementation
    Q_UNUSED(pos)
    Q_UNUSED(identifier)
}

Position Buffer::getPosition(void* identifier) const
{
    QMutexLocker locker(&_mutex);
    // Return position for the given view identifier
    // This is a simplified implementation
    Q_UNUSED(identifier)
    return Position();
}

int Buffer::getEncodingNumber() const
{
    QMutexLocker locker(&_mutex);
    // Convert QString encoding to int
    // This is a simplified mapping - full implementation would map all encodings
    if (_encoding == "UTF-8") return 65001;
    if (_encoding == "UTF-16 LE") return 1200;
    if (_encoding == "UTF-16 BE") return 1201;
    // Return -1 for unknown (use default)
    return -1;
}

void Buffer::setEncodingNumber(int encoding)
{
    QMutexLocker locker(&_mutex);
    // Convert int encoding to QString
    switch (encoding) {
        case 65001:
            _encoding = "UTF-8";
            break;
        case 1200:
            _encoding = "UTF-16 LE";
            break;
        case 1201:
            _encoding = "UTF-16 BE";
            break;
        default:
            _encoding = "UTF-8";
            break;
    }
}

// ============================================================================
// Additional Buffer methods for FileManager compatibility
// ============================================================================

const wchar_t* Buffer::getFullPathName() const
{
    QMutexLocker locker(&_mutex);
    // Return the file path as wchar_t*
    // Note: This relies on Qt's implicit sharing - the data remains valid
    // as long as the QString is not modified
    return reinterpret_cast<const wchar_t*>(_filePath.utf16());
}

bool Buffer::isUnsync() const
{
    QMutexLocker locker(&_mutex);
    return _isUnsync;
}

void Buffer::setUnsync(bool unsync)
{
    QMutexLocker locker(&_mutex);
    _isUnsync = unsync;
}

bool Buffer::isSavePointDirty() const
{
    QMutexLocker locker(&_mutex);
    return _isSavePointDirty;
}

void Buffer::setSavePointDirty(bool dirty)
{
    QMutexLocker locker(&_mutex);
    _isSavePointDirty = dirty;
}

void Buffer::setBackupFilePath(const QString& path)
{
    QMutexLocker locker(&_mutex);
    _backupFilePath = path;
}

// ============================================================================
// FileManager method implementations
// ============================================================================

FileManager::FileManager()
    : _nextBufferID(nullptr), _nbBufs(0)
{
}

FileManager::~FileManager()
{
    // Clean up remaining buffers
    for (Buffer* buf : _buffers) {
        delete buf;
    }
    _buffers.clear();
}

BufferID FileManager::loadFile(const wchar_t* filename, Document doc, int encoding, const wchar_t* backupFileName, FILETIME fileNameTimestamp)
{
    Q_UNUSED(doc)
    Q_UNUSED(fileNameTimestamp)

    if (!filename) {
        return nullptr;
    }

    // Convert wchar_t filename to QString
    QString filePath = QString::fromWCharArray(filename);

    // Check if file exists (or backup file)
    QString loadPath = filePath;
    if (!QFile::exists(loadPath) && backupFileName) {
        loadPath = QString::fromWCharArray(backupFileName);
    }

    if (!QFile::exists(loadPath)) {
        return nullptr;
    }

    // Create a new buffer
    Buffer* newBuf = new Buffer();
    newBuf->setFilePath(filePath);

    // Load file content
    if (!newBuf->loadFromFile(loadPath)) {
        delete newBuf;
        return nullptr;
    }

    // Handle backup file mode
    if (backupFileName != nullptr) {
        newBuf->setBackupFilePath(QString::fromWCharArray(backupFileName));
    }

    // Set encoding if specified
    if (encoding != -1) {
        newBuf->setEncodingNumber(encoding);
    }

    // Add to buffer list
    BufferID id = newBuf;
    _buffers.append(newBuf);
    ++_nbBufs;

    return id;
}

bool FileManager::reloadBuffer(BufferID id)
{
    if (!id) {
        return false;
    }

    Buffer* buf = id;
    QString filePath = buf->getFilePath();

    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        return false;
    }

    // Reload the file
    if (!buf->loadFromFile(filePath)) {
        return false;
    }

    // Update sync status
    buf->setUnsync(false);
    buf->setSavePointDirty(false);

    return true;
}

BufferID FileManager::getBufferFromName(const wchar_t* name)
{
    if (!name) {
        return nullptr;
    }

    QString targetPath = QString::fromWCharArray(name);
    QString canonicalTarget = QFileInfo(targetPath).canonicalFilePath();

    for (Buffer* buf : _buffers) {
        QString bufPath = buf->getFilePath();
        if (QFileInfo(bufPath).canonicalFilePath() == canonicalTarget) {
            return buf;
        }
    }

    return nullptr;
}

bool FileManager::deleteBufferBackup(BufferID id)
{
    if (!id) {
        return true;
    }

    Buffer* buffer = id;
    QString backupPath = buffer->getBackupFilePath();

    if (!backupPath.isEmpty() && QFile::exists(backupPath)) {
        QFile::remove(backupPath);
        buffer->removeBackup();
    }

    return true;
}

size_t FileManager::getNbBuffers() const
{
    return _nbBufs;
}

size_t FileManager::getNbDirtyBuffers() const
{
    size_t count = 0;
    for (Buffer* buf : _buffers) {
        if (buf->isDirty()) {
            ++count;
        }
    }
    return count;
}

Buffer* FileManager::getBufferByIndex(size_t index)
{
    if (index >= static_cast<size_t>(_buffers.size())) {
        return nullptr;
    }
    return _buffers.at(static_cast<int>(index));
}

int FileManager::getBufferIndexByID(BufferID id)
{
    if (!id) {
        return -1;
    }

    return _buffers.indexOf(id);
}

size_t FileManager::nextUntitledNewNumber() const
{
    size_t maxNumber = 0;

    for (Buffer* buf : _buffers) {
        if (buf->isUntitled()) {
            QString fileName = buf->getFileNameQString();
            // Parse "new X" format
            QRegularExpression re(R"(new\s+(\d+))");
            QRegularExpressionMatch match = re.match(fileName);
            if (match.hasMatch()) {
                int num = match.captured(1).toInt();
                if (static_cast<size_t>(num) > maxNumber) {
                    maxNumber = num;
                }
            }
        }
    }

    return maxNumber + 1;
}

BufferID FileManager::bufferFromDocument(Document doc, bool isMainEditZone)
{
    Q_UNUSED(doc)
    Q_UNUSED(isMainEditZone)

    // Create a new buffer for the existing document
    BufferManager* mgr = BufferManager::getInstance();
    Buffer* newBuf = mgr->createBuffer();

    if (!newBuf) {
        return nullptr;
    }

    // Generate untitled name
    QString untitledName = mgr->getNextUntitledName();
    newBuf->setFilePath(untitledName);

    // The document already exists in Scintilla, so we just wrap it
    // The buffer is now associated with the document
    _buffers.append(newBuf);
    ++_nbBufs;

    return newBuf;
}

void FileManager::addBufferReference(BufferID id, ScintillaEditView* identifier)
{
    if (!id || !identifier) {
        return;
    }

    Buffer* buf = id;
    buf->setScintillaView(identifier);

    // Add to our tracking if not already present
    if (!_buffers.contains(buf)) {
        _buffers.append(buf);
        ++_nbBufs;
    }
}

SavingStatus FileManager::saveBuffer(BufferID id, const wchar_t* filename, bool isCopy)
{
    if (!id || !filename) {
        return SavingStatus::SaveOpenFailed;
    }

    Buffer* buffer = id;
    QString filePath = QString::fromWCharArray(filename);

    // Perform the save operation
    if (!buffer->saveToFile(filePath)) {
        return SavingStatus::SaveWritingFailed;
    }

    // If not a copy, update buffer state
    if (!isCopy) {
        buffer->setDirty(false);
        buffer->setUnsync(false);
        buffer->setSavePointDirty(false);
        buffer->setStatus(DOC_REGULAR);
    }

    return SavingStatus::SaveOK;
}

} // namespace QtCore

// ============================================================================
// FileManager implementation - Compatibility layer for Notepad++ core
// This implements the FileManager interface from ScintillaComponent/Buffer.h
// ============================================================================

// This code is only for Windows compatibility mode
// On Linux, QtCore::BufferManager provides the buffer management
#ifndef NPP_LINUX

// Include required headers for FileManager implementation
#include "Notepad_plus.h"
#include "ScintillaEditView.h"
#include "Parameters.h"

// Static instance accessor defined in Buffer.h as macro:
// #define MainFileManager FileManager::getInstance()

// Buffer static member
long Buffer::_recentTagCtr = 0;

// Buffer constructor for Linux/Qt port
Buffer::Buffer(FileManager* pManager, BufferID id, Document doc, DocFileStatus type, const wchar_t* fileName, bool isLargeFile)
    : _pManager(pManager), _id(id), _doc(doc), _lang(L_TEXT), _isLargeFile(isLargeFile)
{
    // Set default EOL format based on OS
#ifdef Q_OS_WIN
    _eolFormat = EolType::windows;
#elif defined(Q_OS_MAC)
    _eolFormat = EolType::macos;
#else
    _eolFormat = EolType::unix;
#endif

    _currentStatus = type;

    // Set filename and related properties
    setFileName(fileName);

    // Initialize timestamp
    updateTimeStamp();

    // Check file state
    checkFileState();

    // Enable notifications after initialization
    _canNotify = true;
}

// FileManager constructor and destructor
FileManager::~FileManager()
{
    // Clean up all buffers
    for (Buffer* buf : _buffers) {
        delete buf;
    }
    _buffers.clear();
}

void FileManager::init(Notepad_plus* pNotepadPlus, ScintillaEditView* pscratchTilla)
{
    _pNotepadPlus = pNotepadPlus;
    _pscratchTilla = pscratchTilla;
    // Note: Scintilla initialization is handled differently in Qt port
    _scratchDocDefault = 0;
}

void FileManager::checkFilesystemChanges(bool bCheckOnlyCurrentBuffer)
{
    if (bCheckOnlyCurrentBuffer) {
        Buffer* buffer = _pNotepadPlus->getCurrentBuffer();
        if (buffer) {
            buffer->checkFileState();
        }
    } else {
        for (size_t i = 0; i < _nbBufs; ++i) {
            _buffers[i]->checkFileState();
        }
    }
}

size_t FileManager::getNbDirtyBuffers() const
{
    size_t nb_dirtyBufs = 0;
    for (size_t i = 0; i < _nbBufs; ++i) {
        if (_buffers[i]->isDirty()) {
            ++nb_dirtyBufs;
        }
    }
    return nb_dirtyBufs;
}

int FileManager::getBufferIndexByID(BufferID id)
{
    for (size_t i = 0; i < _nbBufs; ++i) {
        if (_buffers[i]->_id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

Buffer* FileManager::getBufferByIndex(size_t index)
{
    if (index >= _buffers.size()) {
        return nullptr;
    }
    return _buffers.at(index);
}

void FileManager::beNotifiedOfBufferChange(Buffer* theBuf, int mask)
{
    if (_pNotepadPlus) {
        _pNotepadPlus->notifyBufferChanged(theBuf, mask);
    }
}

void FileManager::addBufferReference(BufferID id, ScintillaEditView* identifier)
{
    Buffer* buf = getBufferByID(id);
    if (buf) {
        buf->addReference(identifier);
    }
}

void FileManager::closeBuffer(BufferID id, const ScintillaEditView* identifier)
{
    int index = getBufferIndexByID(id);
    if (index == -1) {
        return;
    }

    Buffer* buf = getBufferByIndex(index);
    if (!buf) {
        return;
    }

    int refs = buf->removeReference(identifier);

    if (!refs) { // buffer can be deallocated
        _buffers.erase(_buffers.begin() + index);
        delete buf;
        _nbBufs--;
    }
}

BufferID FileManager::loadFile(const wchar_t* filename, Document doc, int encoding, const wchar_t* backupFileName, FILETIME fileNameTimestamp)
{
    if (!filename) {
        return BUFFER_INVALID;
    }

    // Convert wchar_t filename to QString
    QString filePath = QString::fromWCharArray(filename);

    // Check if file exists (or backup file)
    QString loadPath = filePath;
    if (!QFile::exists(loadPath) && backupFileName) {
        loadPath = QString::fromWCharArray(backupFileName);
    }

    if (!QFile::exists(loadPath)) {
        return BUFFER_INVALID;
    }

    // Create a new buffer
    Buffer* newBuf = new Buffer(this, _nextBufferID, doc, DOC_REGULAR, filename, false);
    BufferID id = newBuf;
    newBuf->_id = id;

    // Load file content
    QFile file(loadPath);
    if (!file.open(QIODevice::ReadOnly)) {
        delete newBuf;
        return BUFFER_INVALID;
    }

    QByteArray content = file.readAll();
    file.close();

    // Set up buffer properties
    QFileInfo fileInfo(loadPath);
    // Convert QDateTime to FILETIME
    QDateTime lastModified = fileInfo.lastModified();
    FILETIME ft;
    // Convert to Windows FILETIME (100-nanosecond intervals since January 1, 1601)
    // This is a simplified conversion - full implementation would use proper conversion
    unsigned long long timeValue = static_cast<unsigned long long>(lastModified.toMSecsSinceEpoch());
    // Add difference between Unix epoch (1970) and Windows epoch (1601) in 100-nanosecond intervals
    timeValue = (timeValue + 11644473600000ULL) * 10000ULL;
    ft.dwLowDateTime = static_cast<DWORD>(timeValue);
    ft.dwHighDateTime = static_cast<DWORD>(timeValue >> 32);
    newBuf->_timeStamp = ft;
    newBuf->_isFileReadOnly = !fileInfo.isWritable();

    // Handle backup file mode
    if (backupFileName != nullptr) {
        newBuf->_backupFileName = backupFileName;
        if (!QFile::exists(filePath)) {
            newBuf->_currentStatus = DOC_UNNAMED;
        }
    }

    // Set timestamp if provided
    // FILETIME handling for Qt - convert to QDateTime if needed
    // For now, we use the file's last modified time

    // Detect encoding
    // Simplified encoding detection - in full implementation would use uchardet
    if (content.size() >= 3) {
        if (static_cast<unsigned char>(content[0]) == 0xEF &&
            static_cast<unsigned char>(content[1]) == 0xBB &&
            static_cast<unsigned char>(content[2]) == 0xBF) {
            newBuf->_unicodeMode = uniUTF8;
            newBuf->_encoding = -1;
        } else if (static_cast<unsigned char>(content[0]) == 0xFE &&
                   static_cast<unsigned char>(content[1]) == 0xFF) {
            newBuf->_unicodeMode = uni16BE;
            newBuf->_encoding = -1;
        } else if (static_cast<unsigned char>(content[0]) == 0xFF &&
                   static_cast<unsigned char>(content[1]) == 0xFE) {
            newBuf->_unicodeMode = uni16LE;
            newBuf->_encoding = -1;
        } else {
            newBuf->_unicodeMode = uni8Bit;
            newBuf->_encoding = encoding;
        }
    } else {
        newBuf->_unicodeMode = uni8Bit;
        newBuf->_encoding = encoding;
    }

    // Detect EOL format
    if (content.contains("\r\n")) {
        newBuf->_eolFormat = EolType::windows;
    } else if (content.contains("\n")) {
        newBuf->_eolFormat = EolType::unix;
    } else if (content.contains("\r")) {
        newBuf->_eolFormat = EolType::macos;
    } else {
        newBuf->_eolFormat = EolType::osdefault;
    }

    // Detect language from extension
    QString ext = fileInfo.suffix();
    // Simplified language detection - full implementation would use NppParameters
    if (ext == "cpp" || ext == "cxx" || ext == "cc" || ext == "hpp") {
        newBuf->_lang = L_CPP;
    } else if (ext == "c" || ext == "h") {
        newBuf->_lang = L_C;
    } else if (ext == "java") {
        newBuf->_lang = L_JAVA;
    } else if (ext == "py" || ext == "pyw") {
        newBuf->_lang = L_PYTHON;
    } else if (ext == "js") {
        newBuf->_lang = L_JAVASCRIPT;
    } else if (ext == "html" || ext == "htm") {
        newBuf->_lang = L_HTML;
    } else if (ext == "xml") {
        newBuf->_lang = L_XML;
    } else if (ext == "json") {
        newBuf->_lang = L_JSON;
    } else if (ext == "css") {
        newBuf->_lang = L_CSS;
    } else {
        newBuf->_lang = L_TEXT;
    }

    // Store content - in real implementation this would go into Scintilla document
    // For now, we just track that the buffer has content
    newBuf->_isDirty = false;

    _buffers.push_back(newBuf);
    ++_nbBufs;
    ++_nextBufferID;

    return id;
}

BufferID FileManager::newEmptyDocument()
{
    // Create untitled document name
    std::wstring newTitle = L"new ";
    wchar_t nb[10];
    swprintf(nb, 10, L"%zu", nextUntitledNewNumber());
    newTitle += nb;

    // Create new document (doc = 0 means create new)
    Document doc = 0;  // In real implementation, would call Scintilla to create document

    Buffer* newBuf = new Buffer(this, _nextBufferID, doc, DOC_UNNAMED, newTitle.c_str(), false);
    BufferID id = newBuf;
    newBuf->_id = id;

    // Set default properties
    newBuf->_lang = L_TEXT;
    newBuf->_eolFormat = EolType::osdefault;
    newBuf->_unicodeMode = uni8Bit;
    newBuf->_encoding = -1;
    newBuf->_isDirty = false;

    _buffers.push_back(newBuf);
    ++_nbBufs;
    ++_nextBufferID;

    return id;
}

BufferID FileManager::newPlaceholderDocument(const wchar_t* missingFilename, int whichOne, const wchar_t* userCreatedSessionName)
{
    // Create placeholder for missing file when loading session
    Buffer* newBuf = new Buffer(this, _nextBufferID, 0, DOC_INACCESSIBLE, missingFilename, false);
    BufferID id = newBuf;
    newBuf->_id = id;

    newBuf->_isFileReadOnly = true;
    newBuf->_isInaccessible = true;

    _buffers.push_back(newBuf);
    ++_nbBufs;
    ++_nextBufferID;

    return id;
}

BufferID FileManager::bufferFromDocument(Document doc, bool isMainEditZone)
{
    std::wstring newTitle = L"new ";
    wchar_t nb[10];
    swprintf(nb, 10, L"%zu", nextUntitledNewNumber());
    newTitle += nb;

    Buffer* newBuf = new Buffer(this, _nextBufferID, doc, DOC_UNNAMED, newTitle.c_str(), false);
    BufferID id = newBuf;
    newBuf->_id = id;

    newBuf->_lang = L_TEXT;

    _buffers.push_back(newBuf);
    ++_nbBufs;
    ++_nextBufferID;

    return id;
}

BufferID FileManager::getBufferFromName(const wchar_t* name)
{
    QString targetPath = QString::fromWCharArray(name);
    QString canonicalTarget = QFileInfo(targetPath).canonicalFilePath();

    for (Buffer* buf : _buffers) {
        QString bufPath = QString::fromWCharArray(buf->getFullPathName());
        if (QFileInfo(bufPath).canonicalFilePath() == canonicalTarget) {
            if (!buf->_referees.empty() && buf->_referees[0]->isVisible()) {
                return buf->getID();
            }
        }
    }
    return BUFFER_INVALID;
}

BufferID FileManager::getBufferFromDocument(Document doc)
{
    for (size_t i = 0; i < _nbBufs; ++i) {
        if (_buffers[i]->_doc == doc) {
            return _buffers[i]->_id;
        }
    }
    return BUFFER_INVALID;
}

bool FileManager::reloadBuffer(BufferID id)
{
    Buffer* buf = getBufferByID(id);
    if (!buf) {
        return false;
    }

    const wchar_t* fileName = buf->getFullPathName();
    QString filePath = QString::fromWCharArray(fileName);

    if (!QFile::exists(filePath)) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray content = file.readAll();
    file.close();

    // Update buffer
    QFileInfo fileInfo(filePath);
    // Convert QDateTime to FILETIME
    QDateTime lastModified = fileInfo.lastModified();
    FILETIME ft;
    unsigned long long timeValue = static_cast<unsigned long long>(lastModified.toMSecsSinceEpoch());
    timeValue = (timeValue + 11644473600000ULL) * 10000ULL;
    ft.dwLowDateTime = static_cast<DWORD>(timeValue);
    ft.dwHighDateTime = static_cast<DWORD>(timeValue >> 32);
    buf->_timeStamp = ft;
    buf->setDirty(false);
    buf->setUnsync(false);
    buf->setSavePointDirty(false);

    return true;
}

bool FileManager::reloadBufferDeferred(BufferID id)
{
    Buffer* buf = getBufferByID(id);
    if (buf) {
        buf->setDeferredReload();
    }
    return true;
}

SavingStatus FileManager::saveBuffer(BufferID id, const wchar_t* filename, bool isCopy)
{
    Buffer* buffer = getBufferByID(id);
    if (!buffer) {
        return SavingStatus::SaveOpenFailed;
    }

    QString filePath = QString::fromWCharArray(filename);

    // In real implementation, would get content from Scintilla
    // For now, we just create/truncate the file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return SavingStatus::SaveOpenFailed;
    }

    // Write content would go here in full implementation

    file.close();

    if (!isCopy) {
        buffer->setFileName(filename);
        buffer->setDirty(false);
        buffer->setUnsync(false);
        buffer->setSavePointDirty(false);
        buffer->setStatus(DOC_REGULAR);
    }

    return SavingStatus::SaveOK;
}

bool FileManager::backupCurrentBuffer()
{
    // Simplified backup implementation
    Buffer* buffer = _pNotepadPlus->getCurrentBuffer();
    if (!buffer || buffer->isLargeFile()) {
        return false;
    }

    if (buffer->isDirty()) {
        // In full implementation, would write backup file
        return true;
    }

    return true;
}

bool FileManager::deleteBufferBackup(BufferID id)
{
    Buffer* buffer = getBufferByID(id);
    if (!buffer) {
        return true;
    }

    std::wstring backupPath = buffer->getBackupFileName();
    if (!backupPath.empty()) {
        QString path = QString::fromWCharArray(backupPath.c_str());
        QFile::remove(path);
        buffer->setBackupFileName(std::wstring());
    }

    return true;
}

bool FileManager::deleteFile(BufferID id)
{
    if (id == BUFFER_INVALID) {
        return false;
    }

    Buffer* buf = getBufferByID(id);
    if (!buf) {
        return false;
    }

    QString filePath = QString::fromWCharArray(buf->getFullPathName());
    if (!QFile::exists(filePath)) {
        return false;
    }

    // Move to trash instead of permanent delete
    // In Qt, we can use QFile::moveToTrash if available (Qt 5.15+)
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    return QFile::moveToTrash(filePath);
#else
    // Fallback: just delete the file
    return QFile::remove(filePath);
#endif
}

bool FileManager::moveFile(BufferID id, const wchar_t* newFileName)
{
    if (id == BUFFER_INVALID) {
        return false;
    }

    Buffer* buf = getBufferByID(id);
    if (!buf) {
        return false;
    }

    QString oldPath = QString::fromWCharArray(buf->getFullPathName());
    QString newPath = QString::fromWCharArray(newFileName);

    if (!QFile::exists(oldPath)) {
        return false;
    }

    // Remove destination if it exists
    if (QFile::exists(newPath)) {
        QFile::remove(newPath);
    }

    if (!QFile::rename(oldPath, newPath)) {
        return false;
    }

    buf->setFileName(newFileName);
    return true;
}

bool FileManager::createEmptyFile(const wchar_t* path)
{
    QString filePath = QString::fromWCharArray(path);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.close();
    return true;
}

void FileManager::setLoadedBufferEncodingAndEol(Buffer* buf, const Utf8_16_Read& UnicodeConvertor, int encoding, EolType bkformat)
{
    // Set encoding and EOL format after loading
    if (encoding != -1) {
        buf->setEncoding(encoding);
    }

    if (bkformat != EolType::unknown) {
        buf->setEolFormat(bkformat);
    }
}

size_t FileManager::nextUntitledNewNumber() const
{
    std::vector<size_t> usedNumbers;
    for (Buffer* buf : _buffers) {
        if (buf->isUntitled()) {
            // Parse number from "new X" format
            const wchar_t* fileName = buf->getFileNameQString();
            if (fileName) {
                // Simple parsing - look for digits after "new "
                const wchar_t* numStart = wcsstr(fileName, L"new ");
                if (numStart) {
                    int num = _wtoi(numStart + 4);
                    if (num > 0) {
                        usedNumbers.push_back(num);
                    }
                }
            }
        }
    }

    size_t newNumber = 1;
    while (true) {
        bool found = false;
        for (size_t used : usedNumbers) {
            if (used == newNumber) {
                found = true;
                break;
            }
        }
        if (!found) {
            break;
        }
        ++newNumber;
    }

    return newNumber;
}

int FileManager::getFileNameFromBuffer(BufferID id, wchar_t* fn2copy)
{
    if (getBufferIndexByID(id) == -1) {
        return -1;
    }

    Buffer* buf = getBufferByID(id);
    if (!buf) {
        return -1;
    }

    const wchar_t* fileName = buf->getFullPathName();
    if (fn2copy) {
        wcscpy(fn2copy, fileName);
    }

    return wcslen(fileName);
}

size_t FileManager::docLength(Buffer* buffer) const
{
    // In full implementation, would get document length from Scintilla
    // For now, return 0 as placeholder
    return 0;
}

void FileManager::removeHotSpot(Buffer* buffer) const
{
    // In full implementation, would remove URL hotspots from Scintilla
    // For now, this is a no-op
}

// ============================================================================
// Buffer class method implementations for Linux/Qt port
// ============================================================================

void Buffer::setFileName(const wchar_t* fn)
{
    if (!fn) {
        return;
    }

    // Convert wchar_t to QString and store as full path
    QString fullPath = QString::fromWCharArray(fn);
    _fullPathName = fullPath.toStdWString();

    // Extract filename from full path
    QFileInfo fileInfo(fullPath);
    QString fileName = fileInfo.fileName();

    // Update _fileName pointer to point into _fullPathName
    // Find the last separator in the full path
    size_t lastSep = _fullPathName.find_last_of(L"/\\");
    if (lastSep != std::wstring::npos) {
        _fileName = const_cast<wchar_t*>(_fullPathName.c_str() + lastSep + 1);
    } else {
        _fileName = const_cast<wchar_t*>(_fullPathName.c_str());
    }

    // Detect language from extension
    QString ext = fileInfo.suffix();
    // Simplified language detection - full implementation would use NppParameters
    if (ext == "cpp" || ext == "cxx" || ext == "cc" || ext == "hpp") {
        _lang = L_CPP;
    } else if (ext == "c" || ext == "h") {
        _lang = L_C;
    } else if (ext == "java") {
        _lang = L_JAVA;
    } else if (ext == "py" || ext == "pyw") {
        _lang = L_PYTHON;
    } else if (ext == "js") {
        _lang = L_JAVASCRIPT;
    } else if (ext == "html" || ext == "htm") {
        _lang = L_HTML;
    } else if (ext == "xml") {
        _lang = L_XML;
    } else if (ext == "json") {
        _lang = L_JSON;
    } else if (ext == "css") {
        _lang = L_CSS;
    } else {
        _lang = L_TEXT;
    }

    // Get last modified time
    updateTimeStamp();

    // Refresh compact filename
    refreshCompactFileName();

    // Notify filename change
    doNotify(BufferChangeFilename);
}

void Buffer::updateTimeStamp()
{
    if (_currentStatus == DOC_UNNAMED || _fullPathName.empty()) {
        _timeStamp = FILETIME{};
        return;
    }

    QString filePath = QString::fromStdWString(_fullPathName);
    QFileInfo fileInfo(filePath);

    if (fileInfo.exists()) {
        QDateTime lastModified = fileInfo.lastModified();
        // Convert QDateTime to FILETIME
        unsigned long long timeValue = static_cast<unsigned long long>(lastModified.toMSecsSinceEpoch());
        // Add difference between Unix epoch (1970) and Windows epoch (1601) in 100-nanosecond intervals
        timeValue = (timeValue + 11644473600000ULL) * 10000ULL;
        _timeStamp.dwLowDateTime = static_cast<DWORD>(timeValue);
        _timeStamp.dwHighDateTime = static_cast<DWORD>(timeValue >> 32);
    } else {
        _timeStamp = FILETIME{};
    }
}

bool Buffer::checkFileState()
{
    if (_currentStatus == DOC_UNNAMED || _fullPathName.empty()) {
        return false;
    }

    QString filePath = QString::fromStdWString(_fullPathName);
    QFileInfo fileInfo(filePath);

    bool changed = false;

    if (!fileInfo.exists()) {
        // File was deleted externally
        if (_currentStatus != DOC_DELETED) {
            setStatus(DOC_DELETED);
            changed = true;
        }
    } else {
        // Check if file was modified externally
        QDateTime lastModified = fileInfo.lastModified();
        FILETIME currentFt;
        unsigned long long timeValue = static_cast<unsigned long long>(lastModified.toMSecsSinceEpoch());
        timeValue = (timeValue + 11644473600000ULL) * 10000ULL;
        currentFt.dwLowDateTime = static_cast<DWORD>(timeValue);
        currentFt.dwHighDateTime = static_cast<DWORD>(timeValue >> 32);

        // Compare FILETIME values
        if (currentFt.dwLowDateTime != _timeStamp.dwLowDateTime ||
            currentFt.dwHighDateTime != _timeStamp.dwHighDateTime) {
            if (_currentStatus != DOC_MODIFIED) {
                setStatus(DOC_MODIFIED);
                changed = true;
            }
        }

        // Check read-only status
        bool newReadOnly = !fileInfo.isWritable();
        if (_isFileReadOnly != newReadOnly) {
            _isFileReadOnly = newReadOnly;
            doNotify(BufferChangeReadonly);
            changed = true;
        }
    }

    return changed;
}

int Buffer::addReference(ScintillaEditView* identifier)
{
    // Check if already exists
    for (size_t i = 0; i < _referees.size(); ++i) {
        if (_referees[i] == identifier) {
            return static_cast<int>(_references);
        }
    }

    _referees.push_back(identifier);
    _positions.emplace_back();
    _foldStates.emplace_back();
    ++_references;

    return static_cast<int>(_references);
}

int Buffer::removeReference(const ScintillaEditView* identifier)
{
    for (auto it = _referees.begin(); it != _referees.end(); ++it) {
        if (*it == identifier) {
            size_t index = std::distance(_referees.begin(), it);
            _referees.erase(it);
            _positions.erase(_positions.begin() + index);
            _foldStates.erase(_foldStates.begin() + index);
            --_references;
            break;
        }
    }

    return static_cast<int>(_references);
}

void Buffer::setDirty(bool dirty)
{
    if (_isDirty != dirty) {
        _isDirty = dirty;
        doNotify(BufferChangeDirty);
    }
}

void Buffer::setDeferredReload()
{
    _needReloading = true;
    // In the original implementation, this would trigger a deferred reload
    // For the Qt port, we can emit a signal or set a flag for later processing
}

void Buffer::setEncoding(int encoding)
{
    if (_encoding != encoding) {
        _encoding = encoding;
        doNotify(BufferChangeUnicode);
    }
}

void Buffer::doNotify(int mask)
{
    if (_canNotify && _pManager) {
        _pManager->beNotifiedOfBufferChange(this, mask);
    }
}

std::wstring Buffer::getTimeString(FILETIME rawtime) const
{
    if (rawtime.dwLowDateTime == 0 && rawtime.dwHighDateTime == 0) {
        return std::wstring();
    }

    // Convert FILETIME to QDateTime
    unsigned long long timeValue = (static_cast<unsigned long long>(rawtime.dwHighDateTime) << 32) |
                                    static_cast<unsigned long long>(rawtime.dwLowDateTime);
    // Convert from 100-nanosecond intervals to milliseconds
    timeValue = (timeValue / 10000ULL) - 11644473600000ULL;
    QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(timeValue));

    // Format the date/time string
    QString formatted = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    return formatted.toStdWString();
}

#ifdef __linux__
void Buffer::setTabCreatedTimeStringWithCurrentTime()
{
    if (_currentStatus == DOC_UNNAMED) {
        // Get current time as FILETIME using Qt
        QDateTime now = QDateTime::currentDateTime();
        unsigned long long timeValue = static_cast<unsigned long long>(now.toMSecsSinceEpoch());
        timeValue = (timeValue + 11644473600000ULL) * 10000ULL;
        FILETIME ft;
        ft.dwLowDateTime = static_cast<DWORD>(timeValue);
        ft.dwHighDateTime = static_cast<DWORD>(timeValue >> 32);
        _tabCreatedTimeString = getTimeString(ft);
    }
}
#endif

// Additional Buffer methods needed for the Qt port

void Buffer::setUserReadOnly(bool ro)
{
    if (_isUserReadOnly != ro) {
        _isUserReadOnly = ro;
        doNotify(BufferChangeReadonly);
    }
}

void Buffer::setLangType(LangType lang, const wchar_t* userLangName)
{
    if (_lang != lang) {
        _lang = lang;
        if (userLangName) {
            _userLangExt = userLangName;
        }
        doNotify(BufferChangeLanguage);
    }
}

void Buffer::setUnicodeMode(UniMode mode)
{
    if (_unicodeMode != mode) {
        _unicodeMode = mode;
        doNotify(BufferChangeUnicode);
    }
}

void Buffer::setPosition(const Position& pos, const ScintillaEditView* identifier)
{
    int index = indexOfReference(identifier);
    if (index != -1) {
        _positions[index] = pos;
    }
}

Position& Buffer::getPosition(const ScintillaEditView* identifier)
{
    int index = indexOfReference(identifier);
    if (index != -1) {
        return _positions[index];
    }
    // Return a static empty position as fallback
    static Position emptyPos;
    return emptyPos;
}

void Buffer::setHeaderLineState(const std::vector<size_t>& folds, ScintillaEditView* identifier)
{
    int index = indexOfReference(identifier);
    if (index != -1) {
        _foldStates[index] = folds;
    }
}

const std::vector<size_t>& Buffer::getHeaderLineState(const ScintillaEditView* identifier) const
{
    int index = indexOfReference(identifier);
    if (index != -1) {
        return _foldStates[index];
    }
    // Return a static empty vector as fallback
    static std::vector<size_t> emptyVec;
    return emptyVec;
}

void Buffer::setHideLineChanged(bool isHide, size_t location)
{
    // Implementation for hide line changed
    // This would typically update some internal state
    Q_UNUSED(isHide)
    Q_UNUSED(location)
}

void Buffer::reload()
{
    if (_pManager) {
        _pManager->reloadBuffer(_id);
    }
}

int64_t Buffer::getFileLength() const
{
    if (_fullPathName.empty()) {
        return -1;
    }

    QString filePath = QString::fromStdWString(_fullPathName);
    QFileInfo fileInfo(filePath);

    if (fileInfo.exists()) {
        return fileInfo.size();
    }

    return -1;
}

std::wstring Buffer::getFileTime(fileTimeType ftt) const
{
    if (_fullPathName.empty()) {
        return std::wstring();
    }

    QString filePath = QString::fromStdWString(_fullPathName);
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        return std::wstring();
    }

    QDateTime dateTime;
    switch (ftt) {
        case ft_created:
            dateTime = fileInfo.birthTime();
            if (!dateTime.isValid()) {
                dateTime = fileInfo.lastModified(); // Fallback
            }
            break;
        case ft_modified:
            dateTime = fileInfo.lastModified();
            break;
        case ft_accessed:
            dateTime = fileInfo.lastRead();
            break;
        default:
            return std::wstring();
    }

    QString formatted = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    return formatted.toStdWString();
}

Lang* Buffer::getCurrentLang() const
{
    // In the original implementation, this would return a pointer to the Lang structure
    // For the Qt port, we return nullptr as the language handling is different
    return nullptr;
}

bool Buffer::allowBraceMach() const
{
    // Allow brace matching for most languages
    return _lang != L_TEXT;
}

bool Buffer::allowAutoCompletion() const
{
    // Allow auto-completion for programming languages
    return _lang != L_TEXT && !_isLargeFile;
}

bool Buffer::allowSmartHilite() const
{
    // Allow smart highlighting for all documents
    return true;
}

bool Buffer::allowClickableLink() const
{
    // Allow clickable links for most documents
    return _lang != L_TEXT || !_fullPathName.empty();
}

int Buffer::indexOfReference(const ScintillaEditView* identifier) const
{
    for (size_t i = 0; i < _referees.size(); ++i) {
        if (_referees[i] == identifier) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void Buffer::refreshCompactFileName()
{
    // Create a compact version of the filename with ellipsis if needed
    const wchar_t* fname = getFileName();
    if (fname) {
        _compactFileName = fname;
        // If filename is too long, truncate with ellipsis
        if (_compactFileName.length() > 50) {
            _compactFileName = _compactFileName.substr(0, 20) + L"..." + _compactFileName.substr(_compactFileName.length() - 20);
        }
    }
}

void Buffer::normalizeTabName(std::wstring& tabName)
{
    // Normalize tab name by replacing special characters
    // This is a simplified implementation
    for (auto& c : tabName) {
        if (c == L'\t' || c == L'\n' || c == L'\r') {
            c = L' ';
        }
    }
}

// ============================================================================
// End of FileManager implementation (global namespace)
// ============================================================================

#endif // NPP_LINUX
