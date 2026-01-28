// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>

namespace Platform {

// File change types
enum class FileChangeType {
    Unknown     = 0,
    Created     = 1,
    Deleted     = 2,
    Modified    = 3,
    RenamedOld  = 4,
    RenamedNew  = 5,
    Attributes  = 6,
    Security    = 7
};

// File change event structure
struct FileChangeEvent {
    FileChangeType type = FileChangeType::Unknown;
    std::wstring path;
    std::wstring oldPath;  // For rename operations
    bool isDirectory = false;
};

// Watch options
struct FileWatchOptions {
    bool watchSubtree = false;      // Watch subdirectories
    bool watchFileSize = true;      // Watch for file size changes
    bool watchLastWrite = true;     // Watch for modification time changes
    bool watchCreation = false;     // Watch for creation time changes
    bool watchAttributes = false;   // Watch for attribute changes
    bool watchSecurity = false;     // Watch for security descriptor changes
    bool watchFileName = true;      // Watch for file name changes (rename/create/delete)
    uint32_t bufferSize = 16384;    // Buffer size for events

    FileWatchOptions() = default;
};

// Watch handle
using FileWatchHandle = uint64_t;
const FileWatchHandle INVALID_WATCH_HANDLE = 0;

// Callback for file changes
using FileChangeCallback = std::function<void(const FileChangeEvent& event)>;

// ============================================================================
// IFileWatcher Interface
// ============================================================================
class IFileWatcher {
public:
    virtual ~IFileWatcher() = default;

    // Singleton accessor
    static IFileWatcher& getInstance();

    // ------------------------------------------------------------------------
    // Directory Watching
    // ------------------------------------------------------------------------

    /// Watch a directory for changes
    /// @param path Directory path to watch
    /// @param options Watch options
    /// @param callback Callback function for change events
    /// @return Handle to the watch (INVALID_WATCH_HANDLE on failure)
    virtual FileWatchHandle watchDirectory(const std::wstring& path,
                                           const FileWatchOptions& options,
                                           FileChangeCallback callback) = 0;

    /// Unwatch a directory
    /// @param handle Handle returned by watchDirectory
    /// @return true on success
    virtual bool unwatchDirectory(FileWatchHandle handle) = 0;

    /// Unwatch all directories
    virtual void unwatchAll() = 0;

    // ------------------------------------------------------------------------
    // File Watching
    // ------------------------------------------------------------------------

    /// Watch a specific file for changes
    /// @param filePath File path to watch
    /// @param callback Callback function for change events
    /// @return Handle to the watch (INVALID_WATCH_HANDLE on failure)
    virtual FileWatchHandle watchFile(const std::wstring& filePath,
                                      FileChangeCallback callback) = 0;

    /// Unwatch a file
    /// @param handle Handle returned by watchFile
    /// @return true on success
    virtual bool unwatchFile(FileWatchHandle handle) = 0;

    // ------------------------------------------------------------------------
    // Event Processing
    // ------------------------------------------------------------------------

    /// Process pending events (call periodically or when signaled)
    /// @param timeoutMs Timeout in milliseconds (0 = non-blocking, INFINITE = wait forever)
    /// @return Number of events processed
    virtual size_t processEvents(uint32_t timeoutMs = 0) = 0;

    /// Check if any events are pending without processing them
    /// @return true if events are available
    virtual bool hasPendingEvents() = 0;

    /// Get the wait handle for integration with event loops (platform-specific)
    /// @return Platform-specific handle or nullptr if not supported
    virtual void* getWaitHandle() = 0;

    // ------------------------------------------------------------------------
    // Utility Functions
    // ------------------------------------------------------------------------

    /// Enable or disable a watch temporarily
    virtual bool setWatchEnabled(FileWatchHandle handle, bool enabled) = 0;

    /// Update watch options
    virtual bool updateWatchOptions(FileWatchHandle handle,
                                    const FileWatchOptions& options) = 0;

    /// Check if a watch handle is valid
    virtual bool isWatchValid(FileWatchHandle handle) = 0;

    /// Get the path being watched by a handle
    virtual std::wstring getWatchPath(FileWatchHandle handle) = 0;
};

// ============================================================================
// Convenience Class (similar to original CReadDirectoryChanges)
// ============================================================================
class DirectoryWatcher {
public:
    DirectoryWatcher();
    ~DirectoryWatcher();

    /// Initialize the watcher
    void init();

    /// Terminate the watcher
    void terminate();

    /// Add a directory to watch
    void addDirectory(const std::wstring& path,
                     bool watchSubtree,
                     uint32_t notifyFilter,
                     uint32_t bufferSize = 16384);

    /// Get the wait handle for Win32-style waiting
    void* getWaitHandle();

    /// Pop a change event from the queue
    /// @param type Output: change type
    /// @param filename Output: filename
    /// @return true if an event was retrieved
    bool pop(FileChangeType& type, std::wstring& filename);

    /// Check if any events are available
    bool hasEvents();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

// ============================================================================
// File Watcher Utility Class
// ============================================================================
class FileWatcher {
public:
    FileWatcher();
    ~FileWatcher();

    /// Start watching a file
    void addFile(const std::wstring& filePath);

    /// Check if the file has changed
    /// @return true if the file has been modified since last check
    bool detectChanges();

    /// Stop watching
    void terminate();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

// ============================================================================
// Utility Functions
// ============================================================================
namespace FileWatcherUtils {

/// Convert platform-specific action to FileChangeType
FileChangeType convertAction(uint32_t platformAction);

/// Convert FileChangeType to string
const wchar_t* changeTypeToString(FileChangeType type);

/// Build notify filter from options
uint32_t buildNotifyFilter(const FileWatchOptions& options);

/// Check if a file change type indicates content modification
bool isContentModified(FileChangeType type);

} // namespace FileWatcherUtils

} // namespace Platform
