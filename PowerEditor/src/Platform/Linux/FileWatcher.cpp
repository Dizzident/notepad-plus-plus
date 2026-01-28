// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../FileWatcher.h"
#include "../FileSystem.h"
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <map>
#include <mutex>
#include <set>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>

namespace Platform {

// Helper functions
namespace {
    QString wstringToQString(const std::wstring& wstr) {
        return QString::fromWCharArray(wstr.c_str());
    }

    std::wstring qstringToWstring(const QString& qstr) {
        return qstr.toStdWString();
    }
}

// ============================================================================
// Linux Implementation of IFileWatcher using QFileSystemWatcher
// ============================================================================
class FileWatcherLinux : public IFileWatcher, public QObject {
    Q_OBJECT

public:
    FileWatcherLinux() {
        _nextHandle = 1;
        _watcher = new QFileSystemWatcher(this);

        // Connect Qt signals to our handlers
        connect(_watcher, &QFileSystemWatcher::fileChanged,
                this, &FileWatcherLinux::onFileChanged);
        connect(_watcher, &QFileSystemWatcher::directoryChanged,
                this, &FileWatcherLinux::onDirectoryChanged);

        // Set up polling timer for files that don't trigger inotify well
        _pollTimer = new QTimer(this);
        connect(_pollTimer, &QTimer::timeout, this, &FileWatcherLinux::pollWatches);
        _pollTimer->start(1000); // Poll every second
    }

    ~FileWatcherLinux() override {
        unwatchAll();
    }

    FileWatchHandle watchDirectory(const std::wstring& path,
                                   const FileWatchOptions& options,
                                   FileChangeCallback callback) override {
        std::lock_guard<std::mutex> lock(_mutex);

        QString qPath = wstringToQString(path);

        // Normalize path
        QFileInfo fileInfo(qPath);
        if (!fileInfo.exists() || !fileInfo.isDir()) {
            return INVALID_WATCH_HANDLE;
        }

        QString canonicalPath = fileInfo.canonicalFilePath();

        // Create watch entry
        auto watch = std::make_unique<DirectoryWatch>();
        watch->path = qstringToWstring(canonicalPath);
        watch->qPath = canonicalPath;
        watch->callback = callback;
        watch->options = options;
        watch->enabled = true;

        // Add to QFileSystemWatcher
        if (!_watcher->addPath(canonicalPath)) {
            return INVALID_WATCH_HANDLE;
        }

        // If watching subtree, add all subdirectories
        if (options.watchSubtree) {
            addSubdirectories(canonicalPath, watch.get());
        }

        // Store directory contents for change detection
        watch->lastContents = getDirectoryContents(canonicalPath);

        FileWatchHandle handle = _nextHandle++;
        _directoryWatches[handle] = std::move(watch);

        return handle;
    }

    bool unwatchDirectory(FileWatchHandle handle) override {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _directoryWatches.find(handle);
        if (it == _directoryWatches.end()) {
            return false;
        }

        // Remove from QFileSystemWatcher
        DirectoryWatch* watch = it->second.get();
        _watcher->removePath(watch->qPath);

        for (const QString& subPath : watch->subdirectories) {
            _watcher->removePath(subPath);
        }

        _directoryWatches.erase(it);
        return true;
    }

    void unwatchAll() override {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_watcher) {
            _watcher->removePaths(_watcher->files());
            _watcher->removePaths(_watcher->directories());
        }

        _directoryWatches.clear();
        _fileWatches.clear();
    }

    FileWatchHandle watchFile(const std::wstring& filePath,
                              FileChangeCallback callback) override {
        std::lock_guard<std::mutex> lock(_mutex);

        QString qPath = wstringToQString(filePath);

        QFileInfo fileInfo(qPath);
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            return INVALID_WATCH_HANDLE;
        }

        QString canonicalPath = fileInfo.canonicalFilePath();

        // Create watch entry
        auto watch = std::make_unique<FileWatch>();
        watch->path = qstringToWstring(canonicalPath);
        watch->qPath = canonicalPath;
        watch->callback = callback;
        watch->lastModified = fileInfo.lastModified();
        watch->lastSize = fileInfo.size();
        watch->enabled = true;

        // Add to QFileSystemWatcher
        if (!_watcher->addPath(canonicalPath)) {
            // Some files don't work well with inotify (e.g., network files)
            // We'll rely on polling for these
            watch->pollingOnly = true;
        }

        FileWatchHandle handle = _nextHandle++;
        _fileWatches[handle] = std::move(watch);

        return handle;
    }

    bool unwatchFile(FileWatchHandle handle) override {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _fileWatches.find(handle);
        if (it == _fileWatches.end()) {
            return false;
        }

        FileWatch* watch = it->second.get();
        if (!watch->pollingOnly) {
            _watcher->removePath(watch->qPath);
        }

        _fileWatches.erase(it);
        return true;
    }

    size_t processEvents(uint32_t timeoutMs) override {
        // QFileSystemWatcher uses signals, so we just process any pending events
        // by checking if our event queue has items
        std::lock_guard<std::mutex> lock(_mutex);

        size_t count = 0;

        // Process pending events for file watches
        for (auto& pair : _fileWatches) {
            FileWatch* watch = pair.second.get();
            if (!watch->enabled || watch->pollingOnly) continue;

            QFileInfo info(watch->qPath);
            if (!info.exists()) {
                // File was deleted
                if (watch->callback) {
                    FileChangeEvent event;
                    event.type = FileChangeType::Deleted;
                    event.path = watch->path;
                    event.isDirectory = false;
                    watch->callback(event);
                    count++;
                }
            } else {
                QDateTime modified = info.lastModified();
                qint64 size = info.size();

                if (modified != watch->lastModified || size != watch->lastSize) {
                    watch->lastModified = modified;
                    watch->lastSize = size;

                    if (watch->callback) {
                        FileChangeEvent event;
                        event.type = FileChangeType::Modified;
                        event.path = watch->path;
                        event.isDirectory = false;
                        watch->callback(event);
                        count++;
                    }
                }
            }
        }

        // Note: Directory changes are handled via onDirectoryChanged signal

        return count;
    }

    bool hasPendingEvents() override {
        // QFileSystemWatcher doesn't provide a direct way to check pending events
        // We rely on the signals being emitted
        return false;
    }

    void* getWaitHandle() override {
        // Qt uses signals/slots, not wait handles
        // Return nullptr to indicate not supported
        return nullptr;
    }

    bool setWatchEnabled(FileWatchHandle handle, bool enabled) override {
        std::lock_guard<std::mutex> lock(_mutex);

        auto dirIt = _directoryWatches.find(handle);
        if (dirIt != _directoryWatches.end()) {
            dirIt->second->enabled = enabled;
            return true;
        }

        auto fileIt = _fileWatches.find(handle);
        if (fileIt != _fileWatches.end()) {
            fileIt->second->enabled = enabled;
            return true;
        }

        return false;
    }

    bool updateWatchOptions(FileWatchHandle handle, const FileWatchOptions& options) override {
        // Requires unwatching and re-watching with new options
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _directoryWatches.find(handle);
        if (it == _directoryWatches.end()) {
            return false;
        }

        DirectoryWatch* watch = it->second.get();
        std::wstring path = watch->path;
        FileChangeCallback callback = watch->callback;

        lock.unlock();

        unwatchDirectory(handle);
        watchDirectory(path, options, callback);
        return true;
    }

    bool isWatchValid(FileWatchHandle handle) override {
        std::lock_guard<std::mutex> lock(_mutex);
        return _directoryWatches.find(handle) != _directoryWatches.end() ||
               _fileWatches.find(handle) != _fileWatches.end();
    }

    std::wstring getWatchPath(FileWatchHandle handle) override {
        std::lock_guard<std::mutex> lock(_mutex);

        auto dirIt = _directoryWatches.find(handle);
        if (dirIt != _directoryWatches.end()) {
            return dirIt->second->path;
        }

        auto fileIt = _fileWatches.find(handle);
        if (fileIt != _fileWatches.end()) {
            return fileIt->second->path;
        }

        return std::wstring();
    }

private slots:
    void onFileChanged(const QString& path) {
        std::lock_guard<std::mutex> lock(_mutex);

        for (auto& pair : _fileWatches) {
            FileWatch* watch = pair.second.get();
            if (watch->qPath == path && watch->enabled) {
                QFileInfo info(path);

                if (!info.exists()) {
                    // File was deleted
                    if (watch->callback) {
                        FileChangeEvent event;
                        event.type = FileChangeType::Deleted;
                        event.path = watch->path;
                        event.isDirectory = false;
                        watch->callback(event);
                    }
                } else {
                    QDateTime modified = info.lastModified();
                    qint64 size = info.size();

                    if (modified != watch->lastModified || size != watch->lastSize) {
                        watch->lastModified = modified;
                        watch->lastSize = size;

                        if (watch->callback) {
                            FileChangeEvent event;
                            event.type = FileChangeType::Modified;
                            event.path = watch->path;
                            event.isDirectory = false;
                            watch->callback(event);
                        }
                    }
                }
                break;
            }
        }
    }

    void onDirectoryChanged(const QString& path) {
        std::lock_guard<std::mutex> lock(_mutex);

        for (auto& pair : _directoryWatches) {
            DirectoryWatch* watch = pair.second.get();
            if (!watch->enabled) continue;

            // Check if this is the watched directory or a subdirectory
            if (watch->qPath == path || watch->subdirectories.contains(path)) {
                // Get current contents
                QDir dir(path);
                QStringList currentContents = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

                // Compare with last known contents
                std::set<QString> currentSet(currentContents.begin(), currentContents.end());

                if (path == watch->qPath) {
                    // Root directory
                    detectChanges(watch, path, currentSet, watch->lastContents);
                    watch->lastContents = currentSet;
                } else {
                    // Subdirectory
                    auto it = watch->subdirContents.find(path);
                    if (it != watch->subdirContents.end()) {
                        detectChanges(watch, path, currentSet, it->second);
                        it->second = currentSet;
                    }
                }
            }
        }
    }

    void pollWatches() {
        // Poll files that don't work well with inotify
        std::lock_guard<std::mutex> lock(_mutex);

        for (auto& pair : _fileWatches) {
            FileWatch* watch = pair.second.get();
            if (!watch->enabled || !watch->pollingOnly) continue;

            QFileInfo info(watch->qPath);
            if (!info.exists()) continue;

            QDateTime modified = info.lastModified();
            qint64 size = info.size();

            if (modified != watch->lastModified || size != watch->lastSize) {
                watch->lastModified = modified;
                watch->lastSize = size;

                if (watch->callback) {
                    FileChangeEvent event;
                    event.type = FileChangeType::Modified;
                    event.path = watch->path;
                    event.isDirectory = false;
                    watch->callback(event);
                }
            }
        }
    }

private:
    struct DirectoryWatch {
        std::wstring path;
        QString qPath;
        FileChangeCallback callback;
        FileWatchOptions options;
        bool enabled = true;
        std::set<QString> subdirectories;
        std::set<QString> lastContents;
        std::map<QString, std::set<QString>> subdirContents;
    };

    struct FileWatch {
        std::wstring path;
        QString qPath;
        FileChangeCallback callback;
        QDateTime lastModified;
        qint64 lastSize = 0;
        bool enabled = true;
        bool pollingOnly = false;
    };

    QFileSystemWatcher* _watcher;
    QTimer* _pollTimer;
    std::map<FileWatchHandle, std::unique_ptr<DirectoryWatch>> _directoryWatches;
    std::map<FileWatchHandle, std::unique_ptr<FileWatch>> _fileWatches;
    FileWatchHandle _nextHandle;
    std::mutex _mutex;

    std::set<QString> getDirectoryContents(const QString& path) {
        QDir dir(path);
        QStringList entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        return std::set<QString>(entries.begin(), entries.end());
    }

    void addSubdirectories(const QString& path, DirectoryWatch* watch) {
        QDir dir(path);
        QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QFileInfo& info : entries) {
            QString subPath = info.canonicalFilePath();
            if (_watcher->addPath(subPath)) {
                watch->subdirectories.insert(subPath);
                watch->subdirContents[subPath] = getDirectoryContents(subPath);

                // Recursively add if needed
                if (watch->options.watchSubtree) {
                    addSubdirectories(subPath, watch);
                }
            }
        }
    }

    void detectChanges(DirectoryWatch* watch, const QString& dirPath,
                      const std::set<QString>& current,
                      const std::set<QString>& previous) {
        if (!watch->callback) return;

        // Detect added files
        for (const QString& name : current) {
            if (previous.find(name) == previous.end()) {
                FileChangeEvent event;
                event.type = FileChangeType::Created;
                event.path = qstringToWstring(dirPath + QDir::separator() + name);
                event.isDirectory = QFileInfo(dirPath + QDir::separator() + name).isDir();
                watch->callback(event);
            }
        }

        // Detect removed files
        for (const QString& name : previous) {
            if (current.find(name) == current.end()) {
                FileChangeEvent event;
                event.type = FileChangeType::Deleted;
                event.path = qstringToWstring(dirPath + QDir::separator() + name);
                watch->callback(event);
            }
        }
    }
};

// ============================================================================
// Singleton Accessor
// ============================================================================
IFileWatcher& IFileWatcher::getInstance() {
    static FileWatcherLinux instance;
    return instance;
}

// ============================================================================
// DirectoryWatcher Implementation (Qt-based for Linux)
// ============================================================================
class DirectoryWatcher::Impl : public QObject {
    Q_OBJECT

public:
    QFileSystemWatcher watcher;
    std::queue<std::pair<FileChangeType, std::wstring>> eventQueue;
    std::mutex queueMutex;
    QString watchPath;

    Impl() {
        connect(&watcher, &QFileSystemWatcher::directoryChanged,
                this, &Impl::onDirectoryChanged);
    }

private slots:
    void onDirectoryChanged(const QString& path) {
        std::lock_guard<std::mutex> lock(queueMutex);

        // Get current contents
        QDir dir(path);
        QStringList current = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        // Simple detection - just report modified
        // Full implementation would track contents
        for (const QString& name : current) {
            eventQueue.push({FileChangeType::Modified, qstringToWstring(path + QDir::separator() + name)});
        }
    }
};

DirectoryWatcher::DirectoryWatcher() : _impl(std::make_unique<Impl>()) {}

DirectoryWatcher::~DirectoryWatcher() {
    terminate();
}

void DirectoryWatcher::init() {
    // Nothing special needed for Qt version
}

void DirectoryWatcher::terminate() {
    _impl->watcher.removePaths(_impl->watcher.directories());
}

void DirectoryWatcher::addDirectory(const std::wstring& path,
                                    bool watchSubtree,
                                    uint32_t notifyFilter,
                                    uint32_t bufferSize) {
    (void)watchSubtree;  // Not supported in basic Qt implementation
    (void)notifyFilter;
    (void)bufferSize;

    QString qPath = wstringToQString(path);
    _impl->watchPath = qPath;
    _impl->watcher.addPath(qPath);
}

void* DirectoryWatcher::getWaitHandle() {
    // Qt doesn't use wait handles
    return nullptr;
}

bool DirectoryWatcher::pop(FileChangeType& type, std::wstring& filename) {
    std::lock_guard<std::mutex> lock(_impl->queueMutex);

    if (_impl->eventQueue.empty()) {
        return false;
    }

    auto& front = _impl->eventQueue.front();
    type = front.first;
    filename = front.second;
    _impl->eventQueue.pop();
    return true;
}

bool DirectoryWatcher::hasEvents() {
    std::lock_guard<std::mutex> lock(_impl->queueMutex);
    return !_impl->eventQueue.empty();
}

// ============================================================================
// FileWatcher Implementation (Qt-based for Linux)
// ============================================================================
class FileWatcher::Impl : public QObject {
    Q_OBJECT

public:
    QFileInfo fileInfo;
    QDateTime lastModified;
    QTimer pollTimer;

    Impl() {
        connect(&pollTimer, &QTimer::timeout, this, &Impl::checkForChanges);
    }

    void checkForChanges() {
        if (!fileInfo.exists()) return;

        QFileInfo current(fileInfo.filePath());
        if (current.lastModified() != lastModified) {
            lastModified = current.lastModified();
            // In a full implementation, this would signal the change
        }
    }
};

FileWatcher::FileWatcher() : _impl(std::make_unique<Impl>()) {}

FileWatcher::~FileWatcher() = default;

void FileWatcher::addFile(const std::wstring& filePath) {
    _impl->fileInfo.setFile(wstringToQString(filePath));
    _impl->lastModified = _impl->fileInfo.lastModified();
    _impl->pollTimer.start(1000); // Poll every second
}

bool FileWatcher::detectChanges() {
    if (!_impl->fileInfo.exists()) return false;

    QFileInfo current(_impl->fileInfo.filePath());
    if (current.lastModified() != _impl->lastModified) {
        _impl->lastModified = current.lastModified();
        return true;
    }
    return false;
}

void FileWatcher::terminate() {
    _impl->pollTimer.stop();
}

// ============================================================================
// Utility Functions
// ============================================================================
namespace FileWatcherUtils {

FileChangeType convertAction(uint32_t platformAction) {
    // Linux inotify events
    switch (platformAction) {
        case IN_CREATE:      return FileChangeType::Created;
        case IN_DELETE:      return FileChangeType::Deleted;
        case IN_MODIFY:      return FileChangeType::Modified;
        case IN_MOVED_FROM:  return FileChangeType::RenamedOld;
        case IN_MOVED_TO:    return FileChangeType::RenamedNew;
        case IN_ATTRIB:      return FileChangeType::Attributes;
        default:             return FileChangeType::Unknown;
    }
}

const wchar_t* changeTypeToString(FileChangeType type) {
    switch (type) {
        case FileChangeType::Created:     return L"Created";
        case FileChangeType::Deleted:     return L"Deleted";
        case FileChangeType::Modified:    return L"Modified";
        case FileChangeType::RenamedOld:  return L"RenamedOld";
        case FileChangeType::RenamedNew:  return L"RenamedNew";
        case FileChangeType::Attributes:  return L"Attributes";
        case FileChangeType::Security:    return L"Security";
        default:                          return L"Unknown";
    }
}

uint32_t buildNotifyFilter(const FileWatchOptions& options) {
    uint32_t filter = 0;
    if (options.watchFileName)    filter |= IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO;
    if (options.watchAttributes)  filter |= IN_ATTRIB;
    if (options.watchLastWrite)   filter |= IN_MODIFY;
    if (options.watchSecurity)    filter |= IN_ATTRIB;  // inotify doesn't have separate security
    return filter;
}

bool isContentModified(FileChangeType type) {
    return type == FileChangeType::Modified ||
           type == FileChangeType::Created ||
           type == FileChangeType::Deleted ||
           type == FileChangeType::RenamedNew;
}

} // namespace FileWatcherUtils

} // namespace Platform

#include "FileWatcher.moc"
