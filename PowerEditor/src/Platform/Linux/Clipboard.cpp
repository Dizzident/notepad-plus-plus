// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../Clipboard.h"
#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <QUrl>
#include <QFile>
#include <QTimer>
#include <QDebug>
#include <QDateTime>

namespace PlatformLayer {

// Helper functions
namespace {
    QString stdWStringToQString(const std::wstring& wstr) {
        return QString::fromWCharArray(wstr.c_str());
    }

    std::wstring qStringToStdWString(const QString& qstr) {
        return qstr.toStdWString();
    }
}

// ClipboardData implementation
ClipboardData::ClipboardData(const std::string& text) {
    format = ClipboardFormat::Text;
    data.assign(text.begin(), text.end());
}

ClipboardData::ClipboardData(const std::wstring& text) {
    format = ClipboardFormat::UnicodeText;
    QByteArray utf8Data = QString::fromWCharArray(text.c_str()).toUtf8();
    data.assign(utf8Data.begin(), utf8Data.end());
}

ClipboardData::ClipboardData(const std::vector<uint8_t>& binaryData) {
    format = ClipboardFormat::Binary;
    data = binaryData;
    isBinary = true;
}

std::string ClipboardData::toString() const {
    return std::string(data.begin(), data.end());
}

std::wstring ClipboardData::toWString() const {
    QString qstr = QString::fromUtf8(reinterpret_cast<const char*>(data.data()), data.size());
    return qstr.toStdWString();
}

// Linux implementation using Qt
class ClipboardLinux : public QObject, public IClipboard {
    Q_OBJECT
public:
    ClipboardLinux() : _monitoring(false), _lastSequenceNumber(0) {
        _clipboard = QApplication::clipboard();
        _nextFormatId = 1;
        _isOpen = false;
    }

    ~ClipboardLinux() override {
        stopMonitoring();
    }

    bool hasData() override {
        return _clipboard->ownsClipboard() || !_clipboard->text().isEmpty();
    }

    bool hasFormat(ClipboardFormat format) override {
        const QMimeData* mimeData = _clipboard->mimeData();
        if (!mimeData) return false;

        switch (format) {
            case ClipboardFormat::Text:
            case ClipboardFormat::UnicodeText:
                return mimeData->hasText() || mimeData->hasHtml();
            case ClipboardFormat::Html:
                return mimeData->hasHtml();
            case ClipboardFormat::Rtf:
                return mimeData->hasFormat("text/rtf");
            default:
                return false;
        }
    }

    bool hasCustomFormat(const std::string& formatName) override {
        const QMimeData* mimeData = _clipboard->mimeData();
        return mimeData && mimeData->hasFormat(QString::fromStdString(formatName));
    }

    std::wstring getText() override {
        return qStringToStdWString(_clipboard->text());
    }

    bool setText(const std::wstring& text) override {
        _clipboard->setText(stdWStringToQString(text));
        return true;
    }

    bool setTextWithLength(const std::wstring& text, size_t originalLength) override {
        QMimeData* mimeData = new QMimeData();
        mimeData->setText(stdWStringToQString(text));
        mimeData->setData("application/x-notepadpp-length", QByteArray::number(originalLength));
        _clipboard->setMimeData(mimeData);
        return true;
    }

    ClipboardData getData() override {
        const QMimeData* mimeData = _clipboard->mimeData();
        if (!mimeData) return ClipboardData();

        if (mimeData->hasHtml()) {
            return getData(ClipboardFormat::Html);
        }

        ClipboardData data;
        data.format = ClipboardFormat::UnicodeText;
        QString text = mimeData->text();
        QByteArray utf8Data = text.toUtf8();
        data.data.assign(utf8Data.begin(), utf8Data.end());
        return data;
    }

    ClipboardData getData(ClipboardFormat format) override {
        const QMimeData* mimeData = _clipboard->mimeData();
        if (!mimeData) return ClipboardData();

        ClipboardData result;
        result.format = format;

        switch (format) {
            case ClipboardFormat::Text:
            case ClipboardFormat::UnicodeText: {
                QString text = mimeData->text();
                QByteArray utf8Data = text.toUtf8();
                result.data.assign(utf8Data.begin(), utf8Data.end());
                break;
            }
            case ClipboardFormat::Html: {
                QString html = mimeData->html();
                QByteArray htmlData = html.toUtf8();
                result.data.assign(htmlData.begin(), htmlData.end());
                break;
            }
            case ClipboardFormat::Rtf: {
                QByteArray rtfData = mimeData->data("text/rtf");
                result.data.assign(rtfData.begin(), rtfData.end());
                break;
            }
            default:
                break;
        }

        return result;
    }

    bool setData(const ClipboardData& data) override {
        QMimeData* mimeData = new QMimeData();

        switch (data.format) {
            case ClipboardFormat::Text:
            case ClipboardFormat::UnicodeText: {
                QString text = QString::fromUtf8(reinterpret_cast<const char*>(data.data.data()), data.data.size());
                mimeData->setText(text);
                break;
            }
            case ClipboardFormat::Html: {
                QString html = QString::fromUtf8(reinterpret_cast<const char*>(data.data.data()), data.data.size());
                mimeData->setHtml(html);
                break;
            }
            case ClipboardFormat::Rtf: {
                mimeData->setData("text/rtf", QByteArray(reinterpret_cast<const char*>(data.data.data()), data.data.size()));
                break;
            }
            default: {
                mimeData->setData("application/octet-stream", QByteArray(reinterpret_cast<const char*>(data.data.data()), data.data.size()));
                break;
            }
        }

        _clipboard->setMimeData(mimeData);
        return true;
    }

    bool setData(const std::vector<ClipboardData>& dataItems) override {
        QMimeData* mimeData = new QMimeData();

        for (const auto& item : dataItems) {
            switch (item.format) {
                case ClipboardFormat::Text:
                case ClipboardFormat::UnicodeText: {
                    QString text = QString::fromUtf8(reinterpret_cast<const char*>(item.data.data()), item.data.size());
                    mimeData->setText(text);
                    break;
                }
                case ClipboardFormat::Html: {
                    QString html = QString::fromUtf8(reinterpret_cast<const char*>(item.data.data()), item.data.size());
                    mimeData->setHtml(html);
                    break;
                }
                case ClipboardFormat::Rtf: {
                    mimeData->setData("text/rtf", QByteArray(reinterpret_cast<const char*>(item.data.data()), item.data.size()));
                    break;
                }
                default: {
                    mimeData->setData("application/octet-stream", QByteArray(reinterpret_cast<const char*>(item.data.data()), item.data.size()));
                    break;
                }
            }
        }

        _clipboard->setMimeData(mimeData);
        return true;
    }

    std::string getHtml() override {
        return _clipboard->mimeData()->html().toStdString();
    }

    bool setHtml(const std::string& html, const std::string& text) override {
        QMimeData* mimeData = new QMimeData();
        mimeData->setHtml(QString::fromStdString(html));
        mimeData->setText(QString::fromStdString(text));
        _clipboard->setMimeData(mimeData);
        return true;
    }

    std::string getRtf() override {
        QByteArray rtfData = _clipboard->mimeData()->data("text/rtf");
        return std::string(rtfData.begin(), rtfData.end());
    }

    bool setRtf(const std::string& rtf, const std::string& text) override {
        QMimeData* mimeData = new QMimeData();
        mimeData->setData("text/rtf", QByteArray(rtf.c_str(), rtf.length()));
        mimeData->setText(QString::fromStdString(text));
        _clipboard->setMimeData(mimeData);
        return true;
    }

    uint32_t registerFormat(const std::string& formatName) override {
        // On Linux/X11, MIME types are used directly
        // Check if format already registered - return existing ID if found
        auto it = _customFormatIds.find(formatName);
        if (it != _customFormatIds.end()) {
            // Format already exists, return its existing ID
            return it->second;
        }

        // New format - assign new ID and store
        uint32_t newId = _nextFormatId++;
        _customFormatIds[formatName] = newId;
        _customFormats[newId] = QString::fromStdString(formatName);
        return newId;
    }

    std::vector<uint8_t> getCustomData(const std::string& formatName) override {
        const QMimeData* mimeData = _clipboard->mimeData();
        if (!mimeData) return std::vector<uint8_t>();

        QByteArray data = mimeData->data(QString::fromStdString(formatName));
        return std::vector<uint8_t>(data.begin(), data.end());
    }

    bool setCustomData(const std::string& formatName, const std::vector<uint8_t>& data) override {
        QMimeData* mimeData = new QMimeData();
        mimeData->setData(QString::fromStdString(formatName), QByteArray(reinterpret_cast<const char*>(data.data()), data.size()));
        _clipboard->setMimeData(mimeData);
        return true;
    }

    void startMonitoring() override {
        _monitoring = true;
        connect(_clipboard, &QClipboard::dataChanged, this, &ClipboardLinux::onClipboardChanged);
    }

    void stopMonitoring() override {
        _monitoring = false;
        disconnect(_clipboard, &QClipboard::dataChanged, this, &ClipboardLinux::onClipboardChanged);
    }

    bool isMonitoring() override {
        return _monitoring;
    }

    void setChangeCallback(ClipboardChangeCallback callback) override {
        _changeCallback = callback;
    }

    uint32_t getSequenceNumber() override {
        // Qt doesn't provide a sequence number, simulate with a counter
        static uint32_t sequence = 0;
        return ++sequence;
    }

    bool clear() override {
        _clipboard->clear();
        return true;
    }

    bool open() override {
        // Qt clipboard is always accessible, but track open state for API compatibility
        _isOpen = true;
        return true;
    }

    void close() override {
        _isOpen = false;
    }

    bool isOpen() override {
        return _isOpen;
    }

    bool flush() override {
        // No-op on Linux
        return true;
    }

private slots:
    void onClipboardChanged() {
        if (_changeCallback) {
            _changeCallback();
        }
    }

private:
    QClipboard* _clipboard;
    bool _monitoring;
    uint32_t _lastSequenceNumber;
    std::map<std::string, uint32_t> _customFormatIds;  // format name -> ID
    std::map<uint32_t, QString> _customFormats;        // ID -> format string
    uint32_t _nextFormatId = 1;
    bool _isOpen = false;
    ClipboardChangeCallback _changeCallback;
};

// Singleton accessor
IClipboard& IClipboard::getInstance() {
    static ClipboardLinux instance;
    return instance;
}

// ClipboardHistoryEntry implementation
ClipboardHistoryEntry::ClipboardHistoryEntry(const ClipboardData& d)
    : data(d), timestamp(QDateTime::currentDateTime().toMSecsSinceEpoch()), isBinary(d.isBinary) {
    if (isBinary) {
        displayText = L"[Binary data]";
    } else {
        std::wstring text = d.toWString();
        if (text.length() > 64) {
            displayText = text.substr(0, 61) + L"...";
        } else {
            displayText = text;
        }
    }
}

// Linux implementation of clipboard history
class ClipboardHistoryLinux : public IClipboardHistory {
public:
    ClipboardHistoryLinux() : _maxSize(20) {}

    void init() override {
        loadHistory();
    }

    void addEntry(const ClipboardData& data) override {
        // Check for duplicates
        for (size_t i = 0; i < _entries.size(); ++i) {
            if (_entries[i].data.data == data.data) {
                // Move to front
                ClipboardHistoryEntry entry = _entries[i];
                _entries.erase(_entries.begin() + i);
                _entries.insert(_entries.begin(), entry);
                return;
            }
        }

        ClipboardHistoryEntry entry(data);
        _entries.insert(_entries.begin(), entry);

        // Limit size
        while (_entries.size() > _maxSize) {
            _entries.pop_back();
        }
    }

    ClipboardHistoryEntry getEntry(size_t index) override {
        if (index < _entries.size()) {
            return _entries[index];
        }
        return ClipboardHistoryEntry();
    }

    std::vector<ClipboardHistoryEntry> getAllEntries() override {
        return _entries;
    }

    size_t getEntryCount() override {
        return _entries.size();
    }

    void removeEntry(size_t index) override {
        if (index < _entries.size()) {
            _entries.erase(_entries.begin() + index);
        }
    }

    void clear() override {
        _entries.clear();
    }

    void setMaxSize(size_t maxSize) override {
        _maxSize = maxSize;
        while (_entries.size() > _maxSize) {
            _entries.pop_back();
        }
    }

    size_t getMaxSize() override {
        return _maxSize;
    }

    int findEntryIndex(const ClipboardData& data) override {
        for (size_t i = 0; i < _entries.size(); ++i) {
            if (_entries[i].data.data == data.data) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    void saveHistory() override {
        // TODO: Save to QSettings
    }

    void loadHistory() override {
        // TODO: Load from QSettings
    }

private:
    std::vector<ClipboardHistoryEntry> _entries;
    size_t _maxSize;
};

// Singleton accessor
IClipboardHistory& IClipboardHistory::getInstance() {
    static ClipboardHistoryLinux instance;
    return instance;
}

// Utility functions
namespace ClipboardUtils {

const char* formatToString(ClipboardFormat format) {
    switch (format) {
        case ClipboardFormat::Text: return "Text";
        case ClipboardFormat::UnicodeText: return "UnicodeText";
        case ClipboardFormat::Html: return "HTML";
        case ClipboardFormat::Rtf: return "RTF";
        case ClipboardFormat::Binary: return "Binary";
        case ClipboardFormat::Custom: return "Custom";
        default: return "Unknown";
    }
}

bool containsBinary(const std::vector<uint8_t>& data) {
    for (uint8_t byte : data) {
        if (byte < 32 && byte != '\t' && byte != '\n' && byte != '\r') {
            return true;
        }
    }
    return false;
}

std::wstring truncateForDisplay(const std::wstring& text, size_t maxLength) {
    if (text.length() <= maxLength) {
        return text;
    }
    return text.substr(0, maxLength - 3) + L"...";
}

std::wstring binaryToDisplay(const std::vector<uint8_t>& data, size_t maxLength) {
    std::wstring result = L"[";
    size_t count = std::min(data.size(), maxLength);
    for (size_t i = 0; i < count; ++i) {
        wchar_t buf[4];
        swprintf(buf, 4, L"%02X ", data[i]);
        result += buf;
    }
    if (data.size() > maxLength) {
        result += L"...";
    }
    result += L"]";
    return result;
}

std::string convertToHtmlFormat(const std::string& html, const std::string& fragment) {
    // Simple HTML wrapper for compatibility
    std::string result = "<html><body>";
    result += fragment;
    result += "</body></html>";
    return result;
}

std::string extractTextFromHtml(const std::string& html) {
    // Simple HTML tag removal
    std::string result;
    bool inTag = false;
    for (char c : html) {
        if (c == '<') {
            inTag = true;
        } else if (c == '>') {
            inTag = false;
        } else if (!inTag) {
            result += c;
        }
    }
    return result;
}

} // namespace ClipboardUtils

} // namespace PlatformLayer

#include "Clipboard.moc"