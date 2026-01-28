// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace Platform {

// File attribute flags (cross-platform abstraction)
enum class FileAttr : uint32_t {
    Normal      = 0x0000,
    ReadOnly    = 0x0001,
    Hidden      = 0x0002,
    System      = 0x0004,
    Directory   = 0x0010,
    Archive     = 0x0020,
    Device      = 0x0040,
    Temporary   = 0x0100,
    SparseFile  = 0x0200,
    ReparsePoint= 0x0400,
    Compressed  = 0x0800,
    Offline     = 0x1000,
    NotIndexed  = 0x2000,
    Encrypted   = 0x4000
};

inline FileAttr operator|(FileAttr a, FileAttr b) {
    return static_cast<FileAttr>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline FileAttr operator&(FileAttr a, FileAttr b) {
    return static_cast<FileAttr>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool hasFlag(FileAttr value, FileAttr flag) {
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

// Platform-neutral file time structure
struct FileTime {
    uint64_t seconds;      // Seconds since Unix epoch
    uint32_t nanoseconds;  // Nanoseconds portion (0-999999999)

    FileTime() : seconds(0), nanoseconds(0) {}
    FileTime(uint64_t s, uint32_t ns = 0) : seconds(s), nanoseconds(ns) {}
};

// Platform-neutral file attributes
struct FileAttributes {
    uint64_t size;
    FileTime creationTime;
    FileTime lastAccessTime;
    FileTime lastWriteTime;
    FileAttr attributes;
    bool exists;

    FileAttributes() : size(0), attributes(FileAttr::Normal), exists(false) {}
};

// File information for directory enumeration
struct FileInfo {
    std::wstring name;
    uint64_t size;
    FileTime creationTime;
    FileTime lastWriteTime;
    bool isDirectory;
    bool isHidden;

    FileInfo() : size(0), isDirectory(false), isHidden(false) {}
};

// File open modes
enum class FileMode {
    Read,
    Write,
    Append,
    ReadWrite
};

// ============================================================================
// IFileSystem Interface
// ============================================================================
class IFileSystem {
public:
    virtual ~IFileSystem() = default;

    // Singleton accessor - returns platform-specific implementation
    static IFileSystem& getInstance();

    // ------------------------------------------------------------------------
    // File Existence Checks
    // ------------------------------------------------------------------------
    virtual bool fileExists(const std::wstring& path) = 0;
    virtual bool fileExists(const std::wstring& path, DWORD milliSec2wait, bool* isTimeoutReached) = 0;
    virtual bool directoryExists(const std::wstring& path) = 0;
    virtual bool pathExists(const std::wstring& path) = 0;

    // ------------------------------------------------------------------------
    // File Attributes
    // ------------------------------------------------------------------------
    virtual bool getFileAttributes(const std::wstring& path, FileAttributes& attrs) = 0;
    virtual bool setFileAttributes(const std::wstring& path, FileAttr attributes) = 0;
    virtual bool removeReadOnlyFlag(const std::wstring& path) = 0;

    // ------------------------------------------------------------------------
    // File Operations
    // ------------------------------------------------------------------------
    virtual bool copyFile(const std::wstring& src, const std::wstring& dest, bool overwrite = true) = 0;
    virtual bool moveFile(const std::wstring& src, const std::wstring& dest, bool overwrite = true) = 0;
    virtual bool deleteFile(const std::wstring& path) = 0;
    virtual bool replaceFile(const std::wstring& replaced, const std::wstring& replacement,
                             const std::wstring& backup = L"") = 0;

    // Move to trash/recycle bin
    virtual bool moveToTrash(const std::wstring& path) = 0;

    // ------------------------------------------------------------------------
    // Directory Operations
    // ------------------------------------------------------------------------
    virtual bool createDirectory(const std::wstring& path) = 0;
    virtual bool createDirectoryRecursive(const std::wstring& path) = 0;
    virtual bool removeDirectory(const std::wstring& path) = 0;
    virtual bool removeDirectoryRecursive(const std::wstring& path) = 0;

    // ------------------------------------------------------------------------
    // Directory Enumeration
    // ------------------------------------------------------------------------
    virtual bool enumerateFiles(const std::wstring& directory,
                                const std::wstring& pattern,
                                std::vector<FileInfo>& files) = 0;

    virtual bool enumerateFilesRecursive(const std::wstring& directory,
                                         const std::wstring& pattern,
                                         std::vector<FileInfo>& files) = 0;

    // ------------------------------------------------------------------------
    // Path Operations
    // ------------------------------------------------------------------------
    virtual std::wstring getFullPathName(const std::wstring& path) = 0;
    virtual std::wstring getLongPathName(const std::wstring& path) = 0;
    virtual std::wstring getTempPath() = 0;
    virtual std::wstring getCurrentDirectory() = 0;
    virtual bool setCurrentDirectory(const std::wstring& path) = 0;

    // Path manipulation (static helpers - implemented in header for inline use)
    static std::wstring pathAppend(const std::wstring& base, const std::wstring& append);
    static std::wstring pathRemoveFileSpec(const std::wstring& path);
    static std::wstring getFileName(const std::wstring& path);
    static std::wstring getDirectoryName(const std::wstring& path);
    static std::wstring getExtension(const std::wstring& path);
    static std::wstring changeExtension(const std::wstring& path, const std::wstring& ext);
    static bool isRelativePath(const std::wstring& path);
    static bool isAbsolutePath(const std::wstring& path);

    // ------------------------------------------------------------------------
    // Special Folders (XDG on Linux, CSIDL on Windows)
    // ------------------------------------------------------------------------
    virtual std::wstring getUserConfigDir() = 0;      // ~/.config/notepad++ (Linux), %APPDATA%/Notepad++ (Windows)
    virtual std::wstring getUserDataDir() = 0;        // ~/.local/share/notepad++ (Linux), %APPDATA%/Notepad++ (Windows)
    virtual std::wstring getUserCacheDir() = 0;       // ~/.cache/notepad++ (Linux), %LOCALAPPDATA%/Notepad++ (Windows)
    virtual std::wstring getProgramFilesDir() = 0;    // /usr/share/notepad++ (Linux), %PROGRAMFILES% (Windows)
    virtual std::wstring getDocumentsDir() = 0;       // ~/Documents (Linux), %USERPROFILE%/Documents (Windows)

    // ------------------------------------------------------------------------
    // Disk Operations
    // ------------------------------------------------------------------------
    virtual bool getDiskFreeSpace(const std::wstring& path, uint64_t& freeBytes) = 0;

    // ------------------------------------------------------------------------
    // File Time Operations
    // ------------------------------------------------------------------------
    virtual bool getFileTime(const std::wstring& path, FileTime& creation,
                             FileTime& lastAccess, FileTime& lastWrite) = 0;
    virtual bool setFileTime(const std::wstring& path, const FileTime* creation,
                             const FileTime* lastAccess, const FileTime* lastWrite) = 0;
    static int compareFileTime(const FileTime& t1, const FileTime& t2);
    static FileTime getCurrentFileTime();
};

// ============================================================================
// File I/O Class (replaces Win32_IO_File)
// ============================================================================
class File {
public:
    File();
    explicit File(const std::wstring& path, FileMode mode = FileMode::ReadWrite);
    ~File();

    // Non-copyable
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    // Movable
    File(File&& other) noexcept;
    File& operator=(File&& other) noexcept;

    bool open(const std::wstring& path, FileMode mode);
    void close();
    bool isOpen() const;

    // Read operations
    size_t read(void* buffer, size_t bytesToRead);
    std::string readAll();

    // Write operations
    size_t write(const void* buffer, size_t bytesToWrite);
    bool writeString(const std::string& str);
    bool writeString(const std::wstring& str);

    // Position operations
    int64_t seek(int64_t offset, int origin);  // 0=begin, 1=current, 2=end
    int64_t tell() const;
    int64_t getSize() const;

    // Flush to disk
    bool flush();

    // Get last error
    uint32_t getLastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

// ============================================================================
// Utility Functions
// ============================================================================
namespace FileSystemUtils {

// Read entire file to string
std::string readFileContent(const std::wstring& path, bool* pFailed = nullptr);

// Write string to file
bool writeFileContent(const std::wstring& path, const std::string& content);
bool writeFileContent(const std::wstring& path, const std::wstring& content, UINT codepage = CP_UTF8);

// Ensure directory exists (create if not)
bool ensureDirectoryExists(const std::wstring& path);

// Get unique temporary file path
std::wstring getTempFilePath(const std::wstring& prefix = L"npp");

// Resolve relative path against base directory
std::wstring resolvePath(const std::wstring& baseDir, const std::wstring& relativePath);

// Convert between wchar_t paths and platform-native paths
std::wstring charToWchar(const std::string& str);
std::string wcharToChar(const std::wstring& str);

} // namespace FileSystemUtils

} // namespace Platform
