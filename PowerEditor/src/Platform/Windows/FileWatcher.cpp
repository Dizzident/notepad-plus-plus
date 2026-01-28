// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../FileWatcher.h"
#include "../../WinControls/ReadDirectoryChanges/ReadDirectoryChanges.h"
#include "../../WinControls/ReadDirectoryChanges/ReadFileChanges.h"
#include <windows.h>
#include <map>
#include <mutex>
#include <queue>

namespace Platform {

// ============================================================================
// Windows Implementation of IFileWatcher
// ============================================================================
class FileWatcherWin32 : public IFileWatcher {
public:
    FileWatcherWin32() {
        _nextHandle = 1;
    }

    ~FileWatcherWin32() override {
        unwatchAll();
    }

    FileWatchHandle watchDirectory(const std::wstring& path,
                                   const FileWatchOptions& options,
                                   FileChangeCallback callback) override {
        std::lock_guard<std::mutex> lock(_mutex);

        // Convert options to Win32 filter flags
        DWORD filter = 0;
        if (options.watchFileName)    filter |= FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;
        if (options.watchAttributes)  filter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
        if (options.watchLastWrite)   filter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
        if (options.watchSecurity)    filter |= FILE_NOTIFY_CHANGE_SECURITY;
        if (options.watchCreation)    filter |= FILE_NOTIFY_CHANGE_CREATION;
        if (options.watchFileSize)    filter |= FILE_NOTIFY_CHANGE_SIZE;

        // Create watch entry
        auto watch = std::make_unique<DirectoryWatch>();
        watch->path = path;
        watch->callback = callback;
        watch->options = options;

        // Initialize and add directory
        watch->watcher.Init();
        watch->watcher.AddDirectory(path.c_str(), options.watchSubtree ? TRUE : FALSE, filter, options.bufferSize);

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

        it->second->watcher.Terminate();
        _directoryWatches.erase(it);
        return true;
    }

    void unwatchAll() override {
        std::lock_guard<std::mutex> lock(_mutex);

        for (auto& pair : _directoryWatches) {
            pair.second->watcher.Terminate();
        }
        _directoryWatches.clear();

        for (auto& pair : _fileWatches) {
            // FileWatcher doesn't need explicit termination
        }
        _fileWatches.clear();
    }

    FileWatchHandle watchFile(const std::wstring& filePath,
                              FileChangeCallback callback) override {
        std::lock_guard<std::mutex> lock(_mutex);

        auto watch = std::make_unique<FileWatch>();
        watch->path = filePath;
        watch->callback = callback;

        // Extract directory and filename
        size_t lastSlash = filePath.find_last_of(L"\\/");
        if (lastSlash == std::wstring::npos) {
            return INVALID_WATCH_HANDLE;
        }

        std::wstring directory = filePath.substr(0, lastSlash);
        std::wstring filename = filePath.substr(lastSlash + 1);

        // Set up directory watcher for this specific file
        watch->watcher.Init();

        DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE |
                       FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_ATTRIBUTES;
        watch->watcher.AddDirectory(directory.c_str(), FALSE, filter);

        watch->targetFilename = filename;

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

        it->second->watcher.Terminate();
        _fileWatches.erase(it);
        return true;
    }

    size_t processEvents(uint32_t timeoutMs) override {
        size_t totalProcessed = 0;
        DWORD dwAction;
        std::wstring filename;

        // Process directory watches
        {
            std::lock_guard<std::mutex> lock(_mutex);

            for (auto& pair : _directoryWatches) {
                DirectoryWatch* watch = pair.second.get();

                while (watch->watcher.Pop(dwAction, filename)) {
                    FileChangeEvent event;
                    event.type = convertWin32Action(dwAction);
                    event.path = watch->path + L"\\" + filename;
                    event.isDirectory = false;  // Could be determined from attributes

                    if (watch->callback) {
                        watch->callback(event);
                    }
                    totalProcessed++;
                }
            }

            // Process file watches
            for (auto& pair : _fileWatches) {
                FileWatch* watch = pair.second.get();

                while (watch->watcher.Pop(dwAction, filename)) {
                    // Only report events for our target file
                    if (_wcsicmp(filename.c_str(), watch->targetFilename.c_str()) != 0) {
                        continue;
                    }

                    FileChangeEvent event;
                    event.type = convertWin32Action(dwAction);
                    event.path = watch->path;
                    event.isDirectory = false;

                    if (watch->callback) {
                        watch->callback(event);
                    }
                    totalProcessed++;
                }
            }
        }

        return totalProcessed;
    }

    bool hasPendingEvents() override {
        std::lock_guard<std::mutex> lock(_mutex);

        // Check directory watches
        for (auto& pair : _directoryWatches) {
            HANDLE hEvent = pair->second->watcher.GetWaitHandle();
            if (hEvent && WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0) {
                return true;
            }
        }

        return false;
    }

    void* getWaitHandle() override {
        // Return handle from first directory watch, or nullptr
        std::lock_guard<std::mutex> lock(_mutex);

        for (auto& pair : _directoryWatches) {
            return pair->second->watcher.GetWaitHandle();
        }

        return nullptr;
    }

    bool setWatchEnabled(FileWatchHandle handle, bool enabled) override {
        // Windows implementation doesn't support temporary disable
        // Would require unwatching and re-watching
        (void)handle;
        (void)enabled;
        return false;
    }

    bool updateWatchOptions(FileWatchHandle handle, const FileWatchOptions& options) override {
        // Requires unwatching and re-watching with new options
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _directoryWatches.find(handle);
        if (it == _directoryWatches.end()) {
            return false;
        }

        std::wstring path = it->second->path;
        FileChangeCallback callback = it->second->callback;

        // Terminate and recreate
        it->second->watcher.Terminate();
        _directoryWatches.erase(it);

        lock.unlock();
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

private:
    struct DirectoryWatch {
        CReadDirectoryChanges watcher;
        std::wstring path;
        FileChangeCallback callback;
        FileWatchOptions options;
    };

    struct FileWatch {
        CReadDirectoryChanges watcher;
        std::wstring path;
        std::wstring targetFilename;
        FileChangeCallback callback;
    };

    std::map<FileWatchHandle, std::unique_ptr<DirectoryWatch>> _directoryWatches;
    std::map<FileWatchHandle, std::unique_ptr<FileWatch>> _fileWatches;
    FileWatchHandle _nextHandle;
    std::mutex _mutex;

    FileChangeType convertWin32Action(DWORD action) {
        switch (action) {
            case FILE_ACTION_ADDED:            return FileChangeType::Created;
            case FILE_ACTION_REMOVED:          return FileChangeType::Deleted;
            case FILE_ACTION_MODIFIED:         return FileChangeType::Modified;
            case FILE_ACTION_RENAMED_OLD_NAME: return FileChangeType::RenamedOld;
            case FILE_ACTION_RENAMED_NEW_NAME: return FileChangeType::RenamedNew;
            default:                           return FileChangeType::Unknown;
        }
    }
};

// ============================================================================
// Singleton Accessor
// ============================================================================
IFileWatcher& IFileWatcher::getInstance() {
    static FileWatcherWin32 instance;
    return instance;
}

// ============================================================================
// DirectoryWatcher Implementation
// ============================================================================
class DirectoryWatcher::Impl {
public:
    CReadDirectoryChanges watcher;
    std::queue<std::pair<FileChangeType, std::wstring>> eventQueue;
    std::mutex queueMutex;
};

DirectoryWatcher::DirectoryWatcher() : _impl(std::make_unique<Impl>()) {}

DirectoryWatcher::~DirectoryWatcher() {
    terminate();
}

void DirectoryWatcher::init() {
    _impl->watcher.Init();
}

void DirectoryWatcher::terminate() {
    _impl->watcher.Terminate();
}

void DirectoryWatcher::addDirectory(const std::wstring& path,
                                    bool watchSubtree,
                                    uint32_t notifyFilter,
                                    uint32_t bufferSize) {
    _impl->watcher.AddDirectory(path.c_str(), watchSubtree ? TRUE : FALSE,
                                notifyFilter, bufferSize);
}

void* DirectoryWatcher::getWaitHandle() {
    return _impl->watcher.GetWaitHandle();
}

bool DirectoryWatcher::pop(FileChangeType& type, std::wstring& filename) {
    DWORD dwAction;

    if (_impl->watcher.Pop(dwAction, filename)) {
        switch (dwAction) {
            case FILE_ACTION_ADDED:            type = FileChangeType::Created; break;
            case FILE_ACTION_REMOVED:          type = FileChangeType::Deleted; break;
            case FILE_ACTION_MODIFIED:         type = FileChangeType::Modified; break;
            case FILE_ACTION_RENAMED_OLD_NAME: type = FileChangeType::RenamedOld; break;
            case FILE_ACTION_RENAMED_NEW_NAME: type = FileChangeType::RenamedNew; break;
            default:                           type = FileChangeType::Unknown; break;
        }
        return true;
    }

    return false;
}

bool DirectoryWatcher::hasEvents() {
    // Check if the event handle is signaled
    HANDLE hEvent = _impl->watcher.GetWaitHandle();
    return hEvent && WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0;
}

// ============================================================================
// FileWatcher Implementation
// ============================================================================
class FileWatcher::Impl {
public:
    CReadFileChanges watcher;
};

FileWatcher::FileWatcher() : _impl(std::make_unique<Impl>()) {}

FileWatcher::~FileWatcher() = default;

void FileWatcher::addFile(const std::wstring& filePath) {
    DWORD filter = FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE |
                   FILE_NOTIFY_CHANGE_ATTRIBUTES;
    _impl->watcher.AddFile(filePath.c_str(), filter);
}

bool FileWatcher::detectChanges() {
    return _impl->watcher.DetectChanges() == TRUE;
}

void FileWatcher::terminate() {
    _impl->watcher.Terminate();
}

// ============================================================================
// Utility Functions
// ============================================================================
namespace FileWatcherUtils {

FileChangeType convertAction(uint32_t platformAction) {
    switch (platformAction) {
        case FILE_ACTION_ADDED:            return FileChangeType::Created;
        case FILE_ACTION_REMOVED:          return FileChangeType::Deleted;
        case FILE_ACTION_MODIFIED:         return FileChangeType::Modified;
        case FILE_ACTION_RENAMED_OLD_NAME: return FileChangeType::RenamedOld;
        case FILE_ACTION_RENAMED_NEW_NAME: return FileChangeType::RenamedNew;
        default:                           return FileChangeType::Unknown;
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
    DWORD filter = 0;
    if (options.watchFileName)    filter |= FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;
    if (options.watchAttributes)  filter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
    if (options.watchLastWrite)   filter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
    if (options.watchSecurity)    filter |= FILE_NOTIFY_CHANGE_SECURITY;
    if (options.watchCreation)    filter |= FILE_NOTIFY_CHANGE_CREATION;
    if (options.watchFileSize)    filter |= FILE_NOTIFY_CHANGE_SIZE;
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
