// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../FileSystem.h"
#include "../../MISC/Common/Common.h"
#include "../../MISC/Common/FileInterface.h"
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <cstdio>
#include <chrono>

namespace Platform {

// ============================================================================
// Windows Implementation of IFileSystem
// ============================================================================
class FileSystemWin32 : public IFileSystem {
public:
    // File Existence Checks
    bool fileExists(const std::wstring& path) override {
        return ::doesFileExist(path.c_str(), 0, nullptr);
    }

    bool fileExists(const std::wstring& path, DWORD milliSec2wait, bool* isTimeoutReached) override {
        return ::doesFileExist(path.c_str(), milliSec2wait, isTimeoutReached);
    }

    bool directoryExists(const std::wstring& path) override {
        return ::doesDirectoryExist(path.c_str());
    }

    bool pathExists(const std::wstring& path) override {
        return ::doesPathExist(path.c_str());
    }

    // File Attributes
    bool getFileAttributes(const std::wstring& path, FileAttributes& attrs) override {
        WIN32_FILE_ATTRIBUTE_DATA wfad{};
        if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &wfad)) {
            attrs.exists = false;
            return false;
        }

        attrs.exists = true;
        attrs.size = (static_cast<uint64_t>(wfad.nFileSizeHigh) << 32) | wfad.nFileSizeLow;
        attrs.attributes = static_cast<FileAttr>(wfad.dwFileAttributes);

        // Convert FILETIME to FileTime
        ULARGE_INTEGER ull;
        ull.LowPart = wfad.ftCreationTime.dwLowDateTime;
        ull.HighPart = wfad.ftCreationTime.dwHighDateTime;
        attrs.creationTime.seconds = ull.QuadPart / 10000000ULL - 11644473600ULL;
        attrs.creationTime.nanoseconds = (ull.QuadPart % 10000000ULL) * 100;

        ull.LowPart = wfad.ftLastAccessTime.dwLowDateTime;
        ull.HighPart = wfad.ftLastAccessTime.dwHighDateTime;
        attrs.lastAccessTime.seconds = ull.QuadPart / 10000000ULL - 11644473600ULL;
        attrs.lastAccessTime.nanoseconds = (ull.QuadPart % 10000000ULL) * 100;

        ull.LowPart = wfad.ftLastWriteTime.dwLowDateTime;
        ull.HighPart = wfad.ftLastWriteTime.dwHighDateTime;
        attrs.lastWriteTime.seconds = ull.QuadPart / 10000000ULL - 11644473600ULL;
        attrs.lastWriteTime.nanoseconds = (ull.QuadPart % 10000000ULL) * 100;

        return true;
    }

    bool setFileAttributes(const std::wstring& path, FileAttr attributes) override {
        return SetFileAttributesW(path.c_str(), static_cast<DWORD>(attributes)) != FALSE;
    }

    bool removeReadOnlyFlag(const std::wstring& path) override {
        DWORD attrs = GetFileAttributesW(path.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) return false;
        if (attrs & FILE_ATTRIBUTE_READONLY) {
            attrs &= ~FILE_ATTRIBUTE_READONLY;
            return SetFileAttributesW(path.c_str(), attrs) != FALSE;
        }
        return true;
    }

    // File Operations
    bool copyFile(const std::wstring& src, const std::wstring& dest, bool overwrite) override {
        return CopyFileW(src.c_str(), dest.c_str(), !overwrite) != FALSE;
    }

    bool moveFile(const std::wstring& src, const std::wstring& dest, bool overwrite) override {
        DWORD flags = MOVEFILE_COPY_ALLOWED;
        if (overwrite) flags |= MOVEFILE_REPLACE_EXISTING;
        return MoveFileExW(src.c_str(), dest.c_str(), flags) != FALSE;
    }

    bool deleteFile(const std::wstring& path) override {
        return DeleteFileW(path.c_str()) != FALSE;
    }

    bool replaceFile(const std::wstring& replaced, const std::wstring& replacement,
                     const std::wstring& backup) override {
        return ReplaceFileW(replaced.c_str(), replacement.c_str(),
                           backup.empty() ? nullptr : backup.c_str(),
                           REPLACEFILE_WRITE_THROUGH, nullptr, nullptr) != FALSE;
    }

    bool moveToTrash(const std::wstring& path) override {
        SHFILEOPSTRUCTW shfos{};
        shfos.wFunc = FO_DELETE;

        // Need double-null terminated string
        std::wstring doubleNullPath = path + L'\0';
        shfos.pFrom = doubleNullPath.c_str();
        shfos.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

        return SHFileOperationW(&shfos) == 0;
    }

    // Directory Operations
    bool createDirectory(const std::wstring& path) override {
        return CreateDirectoryW(path.c_str(), nullptr) != FALSE || GetLastError() == ERROR_ALREADY_EXISTS;
    }

    bool createDirectoryRecursive(const std::wstring& path) override {
        // Use SHCreateDirectoryEx which creates intermediate directories
        int result = SHCreateDirectoryExW(nullptr, path.c_str(), nullptr);
        return result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS;
    }

    bool removeDirectory(const std::wstring& path) override {
        return RemoveDirectoryW(path.c_str()) != FALSE;
    }

    bool removeDirectoryRecursive(const std::wstring& path) override {
        // Build full path with wildcard
        std::wstring searchPath = path;
        if (searchPath.back() != L'\\' && searchPath.back() != L'/') {
            searchPath += L'\\';
        }
        searchPath += L'*';

        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            return RemoveDirectoryW(path.c_str()) != FALSE;
        }

        do {
            std::wstring name = findData.cFileName;
            if (name == L"." || name == L"..") continue;

            std::wstring fullPath = path;
            if (fullPath.back() != L'\\' && fullPath.back() != L'/') {
                fullPath += L'\\';
            }
            fullPath += name;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                removeDirectoryRecursive(fullPath);
            } else {
                DeleteFileW(fullPath.c_str());
            }
        } while (FindNextFileW(hFind, &findData));

        FindClose(hFind);
        return RemoveDirectoryW(path.c_str()) != FALSE;
    }

    // Directory Enumeration
    bool enumerateFiles(const std::wstring& directory, const std::wstring& pattern,
                        std::vector<FileInfo>& files) override {
        std::wstring searchPath = directory;
        if (searchPath.back() != L'\\' && searchPath.back() != L'/') {
            searchPath += L'\\';
        }
        searchPath += pattern;

        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            return false;
        }

        files.clear();
        do {
            std::wstring name = findData.cFileName;
            if (name == L"." || name == L"..") continue;

            FileInfo info;
            info.name = name;
            info.size = (static_cast<uint64_t>(findData.nFileSizeHigh) << 32) | findData.nFileSizeLow;
            info.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            info.isHidden = (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;

            ULARGE_INTEGER ull;
            ull.LowPart = findData.ftCreationTime.dwLowDateTime;
            ull.HighPart = findData.ftCreationTime.dwHighDateTime;
            info.creationTime.seconds = ull.QuadPart / 10000000ULL - 11644473600ULL;

            ull.LowPart = findData.ftLastWriteTime.dwLowDateTime;
            ull.HighPart = findData.ftLastWriteTime.dwHighDateTime;
            info.lastWriteTime.seconds = ull.QuadPart / 10000000ULL - 11644473600ULL;

            files.push_back(info);
        } while (FindNextFileW(hFind, &findData));

        FindClose(hFind);
        return true;
    }

    bool enumerateFilesRecursive(const std::wstring& directory, const std::wstring& pattern,
                                 std::vector<FileInfo>& files) override {
        // First, enumerate files in current directory
        std::vector<FileInfo> currentFiles;
        if (enumerateFiles(directory, pattern, currentFiles)) {
            files.insert(files.end(), currentFiles.begin(), currentFiles.end());
        }

        // Then, enumerate subdirectories
        std::vector<FileInfo> subdirs;
        if (enumerateFiles(directory, L"*", subdirs)) {
            for (const auto& subdir : subdirs) {
                if (subdir.isDirectory) {
                    std::wstring subPath = directory;
                    if (subPath.back() != L'\\' && subPath.back() != L'/') {
                        subPath += L'\\';
                    }
                    subPath += subdir.name;
                    enumerateFilesRecursive(subPath, pattern, files);
                }
            }
        }

        return true;
    }

    // Path Operations
    std::wstring getFullPathName(const std::wstring& path) override {
        wchar_t buffer[MAX_PATH];
        DWORD len = GetFullPathNameW(path.c_str(), MAX_PATH, buffer, nullptr);
        if (len == 0 || len >= MAX_PATH) {
            return path;
        }
        return std::wstring(buffer, len);
    }

    std::wstring getLongPathName(const std::wstring& path) override {
        wchar_t buffer[MAX_PATH];
        DWORD len = GetLongPathNameW(path.c_str(), buffer, MAX_PATH);
        if (len == 0 || len >= MAX_PATH) {
            return path;
        }
        return std::wstring(buffer, len);
    }

    std::wstring getTempPath() override {
        wchar_t buffer[MAX_PATH];
        DWORD len = GetTempPathW(MAX_PATH, buffer);
        if (len == 0 || len >= MAX_PATH) {
            return L"C:\\Temp\\";
        }
        return std::wstring(buffer, len);
    }

    std::wstring getCurrentDirectory() override {
        wchar_t buffer[MAX_PATH];
        DWORD len = GetCurrentDirectoryW(MAX_PATH, buffer);
        if (len == 0 || len >= MAX_PATH) {
            return L"";
        }
        return std::wstring(buffer, len);
    }

    bool setCurrentDirectory(const std::wstring& path) override {
        return SetCurrentDirectoryW(path.c_str()) != FALSE;
    }

    // Special Folders
    std::wstring getUserConfigDir() override {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
            std::wstring result = path;
            if (result.back() != L'\\') result += L'\\';
            result += L"Notepad++";
            return result;
        }
        return L"";
    }

    std::wstring getUserDataDir() override {
        // Same as config dir on Windows
        return getUserConfigDir();
    }

    std::wstring getUserCacheDir() override {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
            std::wstring result = path;
            if (result.back() != L'\\') result += L'\\';
            result += L"Notepad++\\cache";
            return result;
        }
        return L"";
    }

    std::wstring getProgramFilesDir() override {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, 0, path))) {
            std::wstring result = path;
            if (result.back() != L'\\') result += L'\\';
            result += L"Notepad++";
            return result;
        }
        return L"";
    }

    std::wstring getDocumentsDir() override {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, 0, path))) {
            return std::wstring(path);
        }
        return L"";
    }

    // Disk Operations
    bool getDiskFreeSpace(const std::wstring& path, uint64_t& freeBytes) override {
        ULARGE_INTEGER freeBytesAvailable;
        if (GetDiskFreeSpaceExW(path.c_str(), &freeBytesAvailable, nullptr, nullptr)) {
            freeBytes = freeBytesAvailable.QuadPart;
            return true;
        }
        return false;
    }

    // File Time Operations
    bool getFileTime(const std::wstring& path, FileTime& creation,
                     FileTime& lastAccess, FileTime& lastWrite) override {
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                   nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        FILETIME ftCreate, ftAccess, ftWrite;
        bool success = GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite) != FALSE;
        CloseHandle(hFile);

        if (success) {
            ULARGE_INTEGER ull;
            ull.LowPart = ftCreate.dwLowDateTime;
            ull.HighPart = ftCreate.dwHighDateTime;
            creation.seconds = ull.QuadPart / 10000000ULL - 11644473600ULL;
            creation.nanoseconds = (ull.QuadPart % 10000000ULL) * 100;

            ull.LowPart = ftAccess.dwLowDateTime;
            ull.HighPart = ftAccess.dwHighDateTime;
            lastAccess.seconds = ull.QuadPart / 10000000ULL - 11644473600ULL;
            lastAccess.nanoseconds = (ull.QuadPart % 10000000ULL) * 100;

            ull.LowPart = ftWrite.dwLowDateTime;
            ull.HighPart = ftWrite.dwHighDateTime;
            lastWrite.seconds = ull.QuadPart / 10000000ULL - 11644473600ULL;
            lastWrite.nanoseconds = (ull.QuadPart % 10000000ULL) * 100;
        }

        return success;
    }

    bool setFileTime(const std::wstring& path, const FileTime* creation,
                     const FileTime* lastAccess, const FileTime* lastWrite) override {
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                                   nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        FILETIME ftCreate, ftAccess, ftWrite;
        FILETIME* pftCreate = nullptr;
        FILETIME* pftAccess = nullptr;
        FILETIME* pftWrite = nullptr;

        if (creation) {
            ULARGE_INTEGER ull;
            ull.QuadPart = (creation->seconds + 11644473600ULL) * 10000000ULL + creation->nanoseconds / 100;
            ftCreate.dwLowDateTime = ull.LowPart;
            ftCreate.dwHighDateTime = ull.HighPart;
            pftCreate = &ftCreate;
        }

        if (lastAccess) {
            ULARGE_INTEGER ull;
            ull.QuadPart = (lastAccess->seconds + 11644473600ULL) * 10000000ULL + lastAccess->nanoseconds / 100;
            ftAccess.dwLowDateTime = ull.LowPart;
            ftAccess.dwHighDateTime = ull.HighPart;
            pftAccess = &ftAccess;
        }

        if (lastWrite) {
            ULARGE_INTEGER ull;
            ull.QuadPart = (lastWrite->seconds + 11644473600ULL) * 10000000ULL + lastWrite->nanoseconds / 100;
            ftWrite.dwLowDateTime = ull.LowPart;
            ftWrite.dwHighDateTime = ull.HighPart;
            pftWrite = &ftWrite;
        }

        bool success = SetFileTime(hFile, pftCreate, pftAccess, pftWrite) != FALSE;
        CloseHandle(hFile);
        return success;
    }
};

// ============================================================================
// Singleton Accessor
// ============================================================================
IFileSystem& IFileSystem::getInstance() {
    static FileSystemWin32 instance;
    return instance;
}

// ============================================================================
// Static Helper Implementations
// ============================================================================
std::wstring IFileSystem::pathAppend(const std::wstring& base, const std::wstring& append) {
    std::wstring result = base;
    return ::pathAppend(result, append);
}

std::wstring IFileSystem::pathRemoveFileSpec(const std::wstring& path) {
    std::wstring result = path;
    return ::pathRemoveFileSpec(result);
}

std::wstring IFileSystem::getFileName(const std::wstring& path) {
    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

std::wstring IFileSystem::getDirectoryName(const std::wstring& path) {
    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        return path.substr(0, pos);
    }
    return L"";
}

std::wstring IFileSystem::getExtension(const std::wstring& path) {
    size_t pos = path.find_last_of(L'.');
    size_t slashPos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos && (slashPos == std::wstring::npos || pos > slashPos)) {
        return path.substr(pos);
    }
    return L"";
}

std::wstring IFileSystem::changeExtension(const std::wstring& path, const std::wstring& ext) {
    size_t pos = path.find_last_of(L'.');
    size_t slashPos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos && (slashPos == std::wstring::npos || pos > slashPos)) {
        return path.substr(0, pos) + ext;
    }
    return path + ext;
}

bool IFileSystem::isRelativePath(const std::wstring& path) {
    return PathIsRelativeW(path.c_str()) != FALSE;
}

bool IFileSystem::isAbsolutePath(const std::wstring& path) {
    return !isRelativePath(path);
}

int IFileSystem::compareFileTime(const FileTime& t1, const FileTime& t2) {
    if (t1.seconds < t2.seconds) return -1;
    if (t1.seconds > t2.seconds) return 1;
    if (t1.nanoseconds < t2.nanoseconds) return -1;
    if (t1.nanoseconds > t2.nanoseconds) return 1;
    return 0;
}

FileTime IFileSystem::getCurrentFileTime() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    FileTime result;
    result.seconds = ull.QuadPart / 10000000ULL - 11644473600ULL;
    result.nanoseconds = (ull.QuadPart % 10000000ULL) * 100;
    return result;
}

// ============================================================================
// File Class Implementation
// ============================================================================
class File::Impl {
public:
    HANDLE hFile = INVALID_HANDLE_VALUE;
    uint32_t lastError = 0;

    ~Impl() {
        close();
    }

    void close() {
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
        }
    }

    DWORD getAccessMode(FileMode mode) {
        switch (mode) {
            case FileMode::Read: return GENERIC_READ;
            case FileMode::Write: return GENERIC_WRITE;
            case FileMode::Append: return GENERIC_WRITE;
            case FileMode::ReadWrite: return GENERIC_READ | GENERIC_WRITE;
        }
        return GENERIC_READ | GENERIC_WRITE;
    }

    DWORD getCreationDisposition(FileMode mode) {
        switch (mode) {
            case FileMode::Read: return OPEN_EXISTING;
            case FileMode::Write: return CREATE_ALWAYS;
            case FileMode::Append: return OPEN_ALWAYS;
            case FileMode::ReadWrite: return OPEN_ALWAYS;
        }
        return OPEN_EXISTING;
    }
};

File::File() : _impl(std::make_unique<Impl>()) {}

File::File(const std::wstring& path, FileMode mode) : File() {
    open(path, mode);
}

File::~File() = default;

File::File(File&& other) noexcept : _impl(std::move(other._impl)) {
    other._impl = std::make_unique<Impl>();
}

File& File::operator=(File&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
        other._impl = std::make_unique<Impl>();
    }
    return *this;
}

bool File::open(const std::wstring& path, FileMode mode) {
    _impl->close();

    DWORD access = _impl->getAccessMode(mode);
    DWORD creation = _impl->getCreationDisposition(mode);

    _impl->hFile = CreateFileW(path.c_str(), access,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                               creation, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (_impl->hFile == INVALID_HANDLE_VALUE) {
        _impl->lastError = GetLastError();
        return false;
    }

    // For append mode, seek to end
    if (mode == FileMode::Append) {
        SetFilePointer(_impl->hFile, 0, nullptr, FILE_END);
    }

    return true;
}

void File::close() {
    _impl->close();
}

bool File::isOpen() const {
    return _impl->hFile != INVALID_HANDLE_VALUE;
}

size_t File::read(void* buffer, size_t bytesToRead) {
    if (!isOpen()) return 0;

    DWORD bytesRead = 0;
    DWORD toRead = (bytesToRead > MAXDWORD) ? MAXDWORD : static_cast<DWORD>(bytesToRead);

    if (!ReadFile(_impl->hFile, buffer, toRead, &bytesRead, nullptr)) {
        _impl->lastError = GetLastError();
        return 0;
    }

    return bytesRead;
}

std::string File::readAll() {
    if (!isOpen()) return "";

    int64_t size = getSize();
    if (size <= 0) return "";

    std::string result;
    result.resize(static_cast<size_t>(size));

    size_t totalRead = 0;
    while (totalRead < result.size()) {
        size_t bytesRead = read(&result[totalRead], result.size() - totalRead);
        if (bytesRead == 0) break;
        totalRead += bytesRead;
    }

    result.resize(totalRead);
    return result;
}

size_t File::write(const void* buffer, size_t bytesToWrite) {
    if (!isOpen()) return 0;

    DWORD bytesWritten = 0;
    DWORD toWrite = (bytesToWrite > MAXDWORD) ? MAXDWORD : static_cast<DWORD>(bytesToWrite);

    if (!WriteFile(_impl->hFile, buffer, toWrite, &bytesWritten, nullptr)) {
        _impl->lastError = GetLastError();
        return 0;
    }

    return bytesWritten;
}

bool File::writeString(const std::string& str) {
    return write(str.c_str(), str.length()) == str.length();
}

bool File::writeString(const std::wstring& str) {
    // Convert to UTF-8
    int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len == 0) return false;

    std::string utf8(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &utf8[0], len, nullptr, nullptr);

    return writeString(utf8);
}

int64_t File::seek(int64_t offset, int origin) {
    if (!isOpen()) return -1;

    DWORD method;
    switch (origin) {
        case 0: method = FILE_BEGIN; break;
        case 1: method = FILE_CURRENT; break;
        case 2: method = FILE_END; break;
        default: return -1;
    }

    LONG highOffset = static_cast<LONG>((offset >> 32) & 0xFFFFFFFF);
    DWORD lowOffset = SetFilePointer(_impl->hFile, static_cast<LONG>(offset & 0xFFFFFFFF), &highOffset, method);

    if (lowOffset == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        _impl->lastError = GetLastError();
        return -1;
    }

    return (static_cast<int64_t>(highOffset) << 32) | lowOffset;
}

int64_t File::tell() const {
    if (!isOpen()) return -1;

    LONG highOffset = 0;
    DWORD lowOffset = SetFilePointer(_impl->hFile, 0, &highOffset, FILE_CURRENT);

    if (lowOffset == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        return -1;
    }

    return (static_cast<int64_t>(highOffset) << 32) | lowOffset;
}

int64_t File::getSize() const {
    if (!isOpen()) return -1;

    LARGE_INTEGER size;
    if (!GetFileSizeEx(_impl->hFile, &size)) {
        return -1;
    }

    return size.QuadPart;
}

bool File::flush() {
    if (!isOpen()) return false;
    return FlushFileBuffers(_impl->hFile) != FALSE;
}

uint32_t File::getLastError() const {
    return _impl->lastError;
}

// ============================================================================
// Utility Functions
// ============================================================================
namespace FileSystemUtils {

std::string readFileContent(const std::wstring& path, bool* pFailed) {
    File file(path, FileMode::Read);
    if (!file.isOpen()) {
        if (pFailed) *pFailed = true;
        return "";
    }
    if (pFailed) *pFailed = false;
    return file.readAll();
}

bool writeFileContent(const std::wstring& path, const std::string& content) {
    File file(path, FileMode::Write);
    if (!file.isOpen()) return false;
    return file.writeString(content);
}

bool writeFileContent(const std::wstring& path, const std::wstring& content, UINT codepage) {
    if (codepage == CP_UTF8) {
        // Convert to UTF-8
        int len = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (len == 0) return false;

        std::string utf8(len - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, &utf8[0], len, nullptr, nullptr);

        return writeFileContent(path, utf8);
    }
    // For other codepages, use the WcharMbcsConvertor
    // This is a simplified version - full implementation would use existing convertor
    return false;
}

bool ensureDirectoryExists(const std::wstring& path) {
    return IFileSystem::getInstance().createDirectoryRecursive(path);
}

std::wstring getTempFilePath(const std::wstring& prefix) {
    std::wstring tempDir = IFileSystem::getInstance().getTempPath();
    std::wstring tempFile;

    wchar_t buffer[MAX_PATH];
    if (GetTempFileNameW(tempDir.c_str(), prefix.c_str(), 0, buffer)) {
        tempFile = buffer;
    } else {
        // Fallback
        tempFile = IFileSystem::pathAppend(tempDir, prefix + L"_temp.tmp");
    }

    return tempFile;
}

std::wstring resolvePath(const std::wstring& baseDir, const std::wstring& relativePath) {
    if (IFileSystem::isAbsolutePath(relativePath)) {
        return relativePath;
    }

    std::wstring combined = IFileSystem::pathAppend(baseDir, relativePath);
    return IFileSystem::getInstance().getFullPathName(combined);
}

std::wstring charToWchar(const std::string& str) {
    if (str.empty()) return L"";

    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (len == 0) return L"";

    std::wstring result(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], len);
    return result;
}

std::string wcharToChar(const std::wstring& str) {
    if (str.empty()) return "";

    int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len == 0) return "";

    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &result[0], len, nullptr, nullptr);
    return result;
}

} // namespace FileSystemUtils

} // namespace Platform
