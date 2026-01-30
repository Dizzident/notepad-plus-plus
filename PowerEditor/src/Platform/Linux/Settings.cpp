// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../Settings.h"
#include "../FileSystem.h"
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <fstream>
#include <cstdlib>
#include <pwd.h>
#include <unistd.h>

namespace PlatformLayer {

// Helper functions
namespace {
    QString wstringToQString(const std::wstring& wstr) {
        return QString::fromWCharArray(wstr.c_str());
    }

    std::wstring qstringToWstring(const QString& qstr) {
        return qstr.toStdWString();
    }

    std::string wstringToUtf8(const std::wstring& wstr) {
        QString qstr = QString::fromWCharArray(wstr.c_str());
        return qstr.toUtf8().constData();
    }

    std::wstring utf8ToWstring(const std::string& str) {
        QString qstr = QString::fromUtf8(str.c_str());
        return qstr.toStdWString();
    }
}

// ============================================================================
// Linux Implementation of ISettings
// ============================================================================
class SettingsLinux : public ISettings {
public:
    SettingsLinux() = default;
    ~SettingsLinux() override = default;

    bool init() override {
        // Ensure config directories exist
        std::wstring configDir = getSettingsDir();
        if (!configDir.empty()) {
            IFileSystem::getInstance().createDirectoryRecursive(configDir);

            // Initialize QSettings
            QSettings::setDefaultFormat(QSettings::IniFormat);
            QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                              wstringToQString(configDir));

            // Load or create config.xml
            loadConfig();
        }

        return true;
    }

    std::wstring getConfigPath() const override {
        // Return the installation prefix config path
        return L"/usr/share/notepad-plus-plus";
    }

    std::wstring getSettingsDir() const override {
        // XDG config directory
        QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
        if (configPath.isEmpty()) {
            // Fallback to home directory/.config
            const char* home = getenv("HOME");
            if (home) {
                configPath = QString::fromUtf8(home) + "/.config";
            } else {
                // Last resort fallback
                configPath = "/tmp";
            }
        }
        configPath += "/notepad-plus-plus";
        return qstringToWstring(configPath);
    }

    std::wstring getUserPluginsDir() const override {
        // XDG data directory for plugins
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (dataPath.isEmpty()) {
            // Fallback to home directory/.local/share
            const char* home = getenv("HOME");
            if (home) {
                dataPath = QString::fromUtf8(home) + "/.local/share/notepad-plus-plus";
            } else {
                // Last resort fallback
                dataPath = "/tmp/notepad-plus-plus";
            }
        }
        dataPath += "/plugins";
        return qstringToWstring(dataPath);
    }

    // Basic Settings (using QSettings with INI format)
    bool writeInt(const std::wstring& section, const std::wstring& key, int value) override {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "notepad-plus-plus", wstringToQString(section));
        settings.setValue(wstringToQString(key), value);
        settings.sync();
        return settings.status() == QSettings::NoError;
    }

    bool writeString(const std::wstring& section, const std::wstring& key, const std::wstring& value) override {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "notepad-plus-plus", wstringToQString(section));
        settings.setValue(wstringToQString(key), wstringToQString(value));
        settings.sync();
        return settings.status() == QSettings::NoError;
    }

    bool writeBool(const std::wstring& section, const std::wstring& key, bool value) override {
        return writeInt(section, key, value ? 1 : 0);
    }

    bool writeBinary(const std::wstring& section, const std::wstring& key, const uint8_t* data, size_t size) override {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "notepad-plus-plus", wstringToQString(section));
        QByteArray byteArray(reinterpret_cast<const char*>(data), static_cast<int>(size));
        settings.setValue(wstringToQString(key), byteArray.toBase64());
        settings.sync();
        return settings.status() == QSettings::NoError;
    }

    int readInt(const std::wstring& section, const std::wstring& key, int defaultValue) override {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "notepad-plus-plus", wstringToQString(section));
        return settings.value(wstringToQString(key), defaultValue).toInt();
    }

    std::wstring readString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue) override {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "notepad-plus-plus", wstringToQString(section));
        QString value = settings.value(wstringToQString(key), wstringToQString(defaultValue)).toString();
        return qstringToWstring(value);
    }

    bool readBool(const std::wstring& section, const std::wstring& key, bool defaultValue) override {
        return readInt(section, key, defaultValue ? 1 : 0) != 0;
    }

    std::vector<uint8_t> readBinary(const std::wstring& section, const std::wstring& key) override {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "notepad-plus-plus", wstringToQString(section));
        QByteArray byteArray = QByteArray::fromBase64(
            settings.value(wstringToQString(key)).toByteArray());
        return std::vector<uint8_t>(byteArray.begin(), byteArray.end());
    }

    // XML Configuration
    bool saveConfig() override {
        std::wstring configPath = getSettingsDir() + L"/config.xml";
        QString qtPath = wstringToQString(configPath);

        QFile file(qtPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("NotepadPlus");

        // Write all config values
        for (const auto& [path, value] : _configValues) {
            QStringList parts = wstringToQString(path).split("/");
            writeXmlPath(writer, parts, wstringToQString(value));
        }

        writer.writeEndElement();
        writer.writeEndDocument();

        return true;
    }

    bool loadConfig() override {
        std::wstring configPath = getSettingsDir() + L"/config.xml";

        if (!IFileSystem::getInstance().fileExists(configPath)) {
            // Create default config
            SettingsUtils::createDefaultConfig(configPath);
            return true;
        }

        std::string utf8Path = wstringToUtf8(configPath);
        std::ifstream file(utf8Path, std::ios::binary);
        if (!file.is_open()) return false;

        std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
        QByteArray data(buffer.data(), static_cast<int>(buffer.size()));

        QXmlStreamReader reader(data);
        parseXmlConfig(reader);

        return true;
    }

    // XML value access
    bool setXmlValue(const std::wstring& path, const std::wstring& value) override {
        _configValues[path] = value;
        return true;
    }

    bool setXmlValueInt(const std::wstring& path, int value) override {
        _configValues[path] = std::to_wstring(value);
        return true;
    }

    bool setXmlValueBool(const std::wstring& path, bool value) override {
        _configValues[path] = value ? L"yes" : L"no";
        return true;
    }

    std::wstring getXmlValue(const std::wstring& path, const std::wstring& defaultValue) override {
        auto it = _configValues.find(path);
        if (it != _configValues.end()) {
            return it->second;
        }
        return defaultValue;
    }

    int getXmlValueInt(const std::wstring& path, int defaultValue) override {
        auto it = _configValues.find(path);
        if (it != _configValues.end()) {
            try {
                return std::stoi(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    bool getXmlValueBool(const std::wstring& path, bool defaultValue) override {
        auto it = _configValues.find(path);
        if (it != _configValues.end()) {
            const std::wstring& val = it->second;
            return (val == L"yes" || val == L"true" || val == L"1");
        }
        return defaultValue;
    }

    // Session Management
    bool saveSession(const SessionInfo& session) override {
        std::wstring sessionPath = getSettingsDir() + L"/session.xml";
        QString qtPath = wstringToQString(sessionPath);

        QFile file(qtPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("NotepadPlus");

        // Write session info
        writer.writeAttribute("activeIndex", QString::number(session.activeIndex));
        writer.writeAttribute("activeLeaf", QString::number(session.activeLeaf));

        // Write files
        writer.writeStartElement("Files");
        for (const auto& fileInfo : session.files) {
            writer.writeStartElement("File");
            writer.writeAttribute("fileName", wstringToQString(fileInfo.fileName));
            writer.writeAttribute("encoding", QString::number(fileInfo.encoding));
            writer.writeAttribute("lang", wstringToQString(fileInfo.langName));
            writer.writeAttribute("readOnly", fileInfo.isUserReadOnly ? "yes" : "no");
            writer.writeEndElement();
        }
        writer.writeEndElement();

        writer.writeEndElement();
        writer.writeEndDocument();

        return true;
    }

    bool loadSession(SessionInfo& session) override {
        std::wstring sessionPath = getSettingsDir() + L"/session.xml";

        if (!IFileSystem::getInstance().fileExists(sessionPath)) {
            return false;
        }

        std::string utf8Path = wstringToUtf8(sessionPath);
        std::ifstream file(utf8Path, std::ios::binary);
        if (!file.is_open()) return false;

        std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
        QByteArray data(buffer.data(), static_cast<int>(buffer.size()));

        QXmlStreamReader reader(data);

        session.files.clear();
        session.folderWorkspaces.clear();

        while (!reader.atEnd()) {
            reader.readNext();

            if (reader.isStartElement()) {
                QStringView name = reader.name();

                if (name == "NotepadPlus") {
                    session.activeIndex = reader.attributes().value("activeIndex").toInt();
                    session.activeLeaf = reader.attributes().value("activeLeaf").toInt();
                }
                else if (name == "File") {
                    SessionFileInfo info;
                    info.fileName = qstringToWstring(reader.attributes().value("fileName").toString());
                    info.encoding = reader.attributes().value("encoding").toInt();
                    info.langName = qstringToWstring(reader.attributes().value("lang").toString());
                    info.isUserReadOnly = (reader.attributes().value("readOnly") == "yes");
                    session.files.push_back(info);
                }
            }
        }

        return !reader.hasError();
    }

    // Recent Files History
    void addToRecentFiles(const std::wstring& filePath) override {
        // Read current list
        std::vector<std::wstring> files = getRecentFiles();

        // Remove if already exists
        auto it = std::find(files.begin(), files.end(), filePath);
        if (it != files.end()) {
            files.erase(it);
        }

        // Add to front
        files.insert(files.begin(), filePath);

        // Limit to 10 items
        if (files.size() > 10) {
            files.resize(10);
        }

        // Save
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "notepad-plus-plus", "History");
        QStringList list;
        for (const auto& f : files) {
            list.append(wstringToQString(f));
        }
        settings.setValue("recentFiles", list);
    }

    std::vector<std::wstring> getRecentFiles() override {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "notepad-plus-plus", "History");
        QStringList list = settings.value("recentFiles").toStringList();

        std::vector<std::wstring> files;
        for (const QString& str : list) {
            files.push_back(qstringToWstring(str));
        }
        return files;
    }

    void clearRecentFiles() override {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                          "notepad-plus-plus", "History");
        settings.setValue("recentFiles", QStringList());
    }

    // File Associations (Linux - using MIME types)
    bool registerFileAssociation(const std::wstring& extension, const std::wstring& description) override {
        (void)description;
        // On Linux, file associations are handled via MIME types and .desktop files
        // This would require creating a mimeapps.list entry
        std::wstring ext = extension;
        if (ext[0] == L'.') {
            ext = ext.substr(1);
        }

        // Create MIME type association (simplified)
        std::wstring mimeType = L"text/" + ext;

        // In a full implementation, this would:
        // 1. Create/update ~/.config/mimeapps.list
        // 2. Create a .desktop file if needed
        // 3. Run update-mime-database

        return true;
    }

    bool unregisterFileAssociation(const std::wstring& extension) override {
        std::wstring ext = extension;
        if (ext[0] == L'.') {
            ext = ext.substr(1);
        }

        // Remove from mimeapps.list
        return true;
    }

    bool isFileAssociated(const std::wstring& extension) override {
        (void)extension;
        // Check mimeapps.list for association
        return false;
    }

    // Plugin Settings
    bool writePluginSetting(const std::wstring& pluginName, const std::wstring& key, const std::wstring& value) override {
        std::wstring section = L"Plugin_" + pluginName;
        return writeString(section, key, value);
    }

    std::wstring readPluginSetting(const std::wstring& pluginName, const std::wstring& key, const std::wstring& defaultValue) override {
        std::wstring section = L"Plugin_" + pluginName;
        return readString(section, key, defaultValue);
    }

private:
    std::map<std::wstring, std::wstring> _configValues;

    void writeXmlPath(QXmlStreamWriter& writer, const QStringList& parts, const QString& value) {
        // Simplified XML writer - full implementation would handle nested elements
        if (!parts.isEmpty()) {
            writer.writeTextElement(parts.last(), value);
        }
    }

    void parseXmlConfig(QXmlStreamReader& reader) {
        _configValues.clear();

        QString currentPath;

        while (!reader.atEnd()) {
            reader.readNext();

            if (reader.isStartElement()) {
                QString name = reader.name().toString();
                currentPath += "/" + name;

                // Read attributes and text content
                QString text = reader.readElementText();
                if (!text.isEmpty()) {
                    _configValues[qstringToWstring(currentPath)] = qstringToWstring(text);
                }
            }
            else if (reader.isEndElement()) {
                int lastSlash = currentPath.lastIndexOf('/');
                if (lastSlash > 0) {
                    currentPath = currentPath.left(lastSlash);
                }
            }
        }
    }
};

// ============================================================================
// Singleton Accessor
// ============================================================================
ISettings& ISettings::getInstance() {
    static SettingsLinux instance;
    return instance;
}

// ============================================================================
// Utility Functions
// ============================================================================
namespace SettingsUtils {

std::wstring getConfigFilePath(const std::wstring& filename) {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (configPath.isEmpty()) {
        // Fallback to home directory/.config
        const char* home = getenv("HOME");
        if (home) {
            configPath = QString::fromUtf8(home) + "/.config";
        } else {
            // Last resort fallback
            configPath = "/tmp";
        }
    }
    configPath += "/notepad-plus-plus/";
    configPath += QString::fromWCharArray(filename.c_str());
    return qstringToWstring(configPath);
}

std::wstring getSessionFilePath(const std::wstring& filename) {
    return getConfigFilePath(filename);
}

void createDefaultConfig(const std::wstring& path) {
    std::string utf8Path = wstringToUtf8(path);

    std::ofstream file(utf8Path);
    if (!file.is_open()) return;

    // Write default config.xml
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    file << "<NotepadPlus>\n";
    file << "    <GUIConfigs>\n";
    file << "        <GUIConfig name=\"NewDoc\" format=\"0\" encoding=\"0\" />\n";
    file << "        <GUIConfig name=\"TabBar\" dragAndDrop=\"yes\" drawTopBar=\"yes\" drawInactiveTab=\"yes\" reduce=\"yes\" closeButton=\"yes\" doubleClick2Close=\"no\" vertical=\"no\" multiLine=\"no\" hide=\"no\" quitOnClose=\"no\" />\n";
    file << "        <GUIConfig name=\"ScintillaBackups\" backupDir=\"\" usePluginDir=\"yes\" />\n";
    file << "    </GUIConfigs>\n";
    file << "</NotepadPlus>\n";
}

void createDefaultSession(const std::wstring& path) {
    std::string utf8Path = wstringToUtf8(path);

    std::ofstream file(utf8Path);
    if (!file.is_open()) return;

    file << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    file << "<NotepadPlus>\n";
    file << "    <Files />\n";
    file << "</NotepadPlus>\n";
}

} // namespace SettingsUtils

} // namespace PlatformLayer
