// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../FileSystem.h"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <dirent.h>
#include <cstring>
#include <pwd.h>

namespace PlatformLayer {

// Helper functions for string conversion
namespace {
    std::string wstringToUtf8(const std::wstring& wstr) {
        if (wstr.empty()) return "";
        size_t len = wcstombs(nullptr, wstr.c_str(), 0);
        if (len == static_cast<size_t>(-1)) return "";
        std::string result(len, 0);
        wcstombs(&result[0], wstr.c_str(), len);
        return result;
    }

    std::wstring utf8ToWstring(const std::string& str) {
        if (str.empty()) return L"";
        size_t len = mbstowcs(nullptr, str.c_str(), 0);
        if (len == static_cast<size_t>(-1)) return L"";
        std::wstring result(len, 0);
        mbstowcs(&result[0], str.c_str(), len);
        return result;
    }

    std::filesystem::path toPath(const std::wstring& wstr) {
        return std::filesystem::path(wstringToUtf8(wstr));
    }

    timespec toTimespec(const FileTime& ft) {
        timespec ts;
        ts.tv_sec = ft.seconds;
        ts.tv_nsec = ft.nanoseconds;
        return ts;
    }
}

// ============================================================================
// Linux Implementation of IFileSystem
// ============================================================================
class FileSystemLinux : public IFileSystem {
public:
    // File Existence Checks
    bool fileExists(const std::wstring& path) override {
        try {
            auto p = toPath(path);
            return std::filesystem::exists(p) && std::filesystem::is_regular_file(p);
        } catch (...) {
            return false;
        }
    }

    bool fileExists(const std::wstring& path, DWORD milliSec2wait, bool* isTimeoutReached) override {
        if (isTimeoutReached) *isTimeoutReached = false;

        auto start = std::chrono::steady_clock::now();
        while (true) {
            if (fileExists(path)) return true;

            auto elapsed = std::chrono::steady_clock::now() - start;
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

            if (elapsedMs >= static_cast<int64_t>(milliSec2wait)) {
                if (isTimeoutReached) *isTimeoutReached = true;
                return false;
            }

            usleep(10000); // 10ms sleep
        }
    }

    bool directoryExists(const std::wstring& path) override {
        try {
            auto p = toPath(path);
            return std::filesystem::exists(p) && std::filesystem::is_directory(p);
        } catch (...) {
            return false;
        }
    }

    bool pathExists(const std::wstring& path) override {
        try {
            return std::filesystem::exists(toPath(path));
        } catch (...) {
            return false;
        }
    }

    // File Attributes
    bool getFileAttributes(const std::wstring& path, FileAttributes& attrs) override {
        try {
            auto p = toPath(path);

            if (!std::filesystem::exists(p)) {
                attrs.exists = false;
                return false;
            }

            attrs.exists = true;
            attrs.size = std::filesystem::is_regular_file(p) ? std::filesystem::file_size(p) : 0;

            struct stat st;
            if (stat(wstringToUtf8(path).c_str(), &st) == 0) {
                attrs.creationTime.seconds = st.st_ctim.tv_sec;
                attrs.creationTime.nanoseconds = st.st_ctim.tv_nsec;
                attrs.lastAccessTime.seconds = st.st_atim.tv_sec;
                attrs.lastAccessTime.nanoseconds = st.st_atim.tv_nsec;
                attrs.lastWriteTime.seconds = st.st_mtim.tv_sec;
                attrs.lastWriteTime.nanoseconds = st.st_mtim.tv_nsec;

                FileAttr attr = FileAttr::Normal;
                if ((st.st_mode & S_IWUSR) == 0) attr = attr | FileAttr::ReadOnly;
                if (S_ISDIR(st.st_mode)) attr = attr | FileAttr::Directory;
                attrs.attributes = attr;
            }

            return true;
        } catch (...) {
            attrs.exists = false;
            return false;
        }
    }

    bool setFileAttributes(const std::wstring& path, FileAttr attributes) override {
        try {
            std::string utf8Path = wstringToUtf8(path);
            struct stat st;
            if (stat(utf8Path.c_str(), &st) != 0) return false;

            mode_t mode = st.st_mode;

            if (hasFlag(attributes, FileAttr::ReadOnly)) {
                mode &= ~S_IWUSR;
            } else {
                mode |= S_IWUSR;
            }

            return chmod(utf8Path.c_str(), mode) == 0;
        } catch (...) {
            return false;
        }
    }

    bool removeReadOnlyFlag(const std::wstring& path) override {
        try {
            std::string utf8Path = wstringToUtf8(path);
            struct stat st;
            if (stat(utf8Path.c_str(), &st) != 0) return false;

            if ((st.st_mode & S_IWUSR) == 0) {
                return chmod(utf8Path.c_str(), st.st_mode | S_IWUSR) == 0;
            }
            return true;
        } catch (...) {
            return false;
        }
    }

    // File Operations
    bool copyFile(const std::wstring& src, const std::wstring& dest, bool overwrite) override {
        try {
            auto from = toPath(src);
            auto to = toPath(dest);

            if (!overwrite && std::filesystem::exists(to)) {
                return false;
            }

            std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool moveFile(const std::wstring& src, const std::wstring& dest, bool overwrite) override {
        try {
            auto from = toPath(src);
            auto to = toPath(dest);

            if (!overwrite && std::filesystem::exists(to)) {
                return false;
            }

            std::filesystem::rename(from, to);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool deleteFile(const std::wstring& path) override {
        try {
            return std::filesystem::remove(toPath(path));
        } catch (...) {
            return false;
        }
    }

    bool replaceFile(const std::wstring& replaced, const std::wstring& replacement,
                     const std::wstring& backup) override {
        try {
            auto target = toPath(replaced);
            auto source = toPath(replacement);

            // If backup specified, copy target to backup first
            if (!backup.empty()) {
                auto backupPath = toPath(backup);
                if (std::filesystem::exists(target)) {
                    std::filesystem::copy_file(target, backupPath,
                                               std::filesystem::copy_options::overwrite_existing);
                }
            }

            // Remove target if exists
            if (std::filesystem::exists(target)) {
                std::filesystem::remove(target);
            }

            // Rename source to target
            std::filesystem::rename(source, target);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool moveToTrash(const std::wstring& path) override {
        // XDG Trash specification implementation
        try {
            std::filesystem::path source = toPath(path);
            if (!std::filesystem::exists(source)) return false;

            std::filesystem::path trashBase = getXdgTrashDir();
            std::filesystem::path trashFiles = trashBase / "files";
            std::filesystem::path trashInfo = trashBase / "info";

            // Ensure trash directories exist
            std::filesystem::create_directories(trashFiles);
            std::filesystem::create_directories(trashInfo);

            // Generate unique filename in trash
            std::wstring filename = IFileSystem::getFileName(path);
            std::filesystem::path destPath = trashFiles / wstringToUtf8(filename);
            int counter = 1;
            while (std::filesystem::exists(destPath)) {
                std::wstring newName = filename + L"." + std::to_wstring(counter);
                destPath = trashFiles / wstringToUtf8(newName);
                counter++;
            }

            // Create .trashinfo file
            std::wstring destName = utf8ToWstring(destPath.filename().string());
            std::filesystem::path infoPath = trashInfo / (wstringToUtf8(destName) + ".trashinfo");

            std::ofstream infoFile(infoPath);
            if (infoFile) {
                infoFile << "[Trash Info]\n";
                infoFile << "Path=" << wstringToUtf8(path) << "\n";

                auto now = std::chrono::system_clock::now();
                auto time = std::chrono::system_clock::to_time_t(now);
                std::tm* utc = std::gmtime(&time);
                char buf[32];
                strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", utc);
                infoFile << "DeletionDate=" << buf << "\n";
            }

            // Move file to trash
            std::filesystem::rename(source, destPath);
            return true;
        } catch (...) {
            return false;
        }
    }

    // Directory Operations
    bool createDirectory(const std::wstring& path) override {
        try {
            return std::filesystem::create_directory(toPath(path));
        } catch (...) {
            return false;
        }
    }

    bool createDirectoryRecursive(const std::wstring& path) override {
        if (path.empty()) return false;
        try {
            std::filesystem::create_directories(toPath(path));
            return true;
        } catch (...) {
            return false;
        }
    }

    bool removeDirectory(const std::wstring& path) override {
        try {
            return std::filesystem::remove(toPath(path));
        } catch (...) {
            return false;
        }
    }

    bool removeDirectoryRecursive(const std::wstring& path) override {
        try {
            auto removed = std::filesystem::remove_all(toPath(path));
            return removed > 0;
        } catch (...) {
            return false;
        }
    }

    // Directory Enumeration
    bool enumerateFiles(const std::wstring& directory, const std::wstring& pattern,
                        std::vector<FileInfo>& files) override {
        try {
            files.clear();
            std::filesystem::path dir = toPath(directory);

            if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
                return false;
            }

            // Convert pattern to simple filter (not regex for compatibility)
            std::wstring patLower = pattern;
            for (auto& c : patLower) c = towlower(c);
            bool allFiles = (pattern == L"*" || pattern == L"*.*");

            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                std::wstring name = utf8ToWstring(entry.path().filename().string());

                // Simple pattern matching
                if (!allFiles) {
                    std::wstring nameLower = name;
                    for (auto& c : nameLower) c = towlower(c);
                    // Very basic wildcard matching - could be improved
                    if (!matchWildcard(nameLower, patLower)) continue;
                }

                FileInfo info;
                info.name = name;
                info.isDirectory = entry.is_directory();
                info.isHidden = entry.path().filename().string()[0] == '.';

                if (entry.is_regular_file()) {
                    info.size = entry.file_size();
                }

                struct stat st;
                std::string pathStr = entry.path().string();
                if (stat(pathStr.c_str(), &st) == 0) {
                    info.creationTime.seconds = st.st_ctim.tv_sec;
                    info.lastWriteTime.seconds = st.st_mtim.tv_sec;
                }

                files.push_back(info);
            }

            return true;
        } catch (...) {
            return false;
        }
    }

    bool enumerateFilesRecursive(const std::wstring& directory, const std::wstring& pattern,
                                 std::vector<FileInfo>& files) override {
        try {
            std::filesystem::path dir = toPath(directory);

            if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
                return false;
            }

            std::wstring patLower = pattern;
            for (auto& c : patLower) c = towlower(c);
            bool allFiles = (pattern == L"*" || pattern == L"*.*");

            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
                std::wstring name = utf8ToWstring(entry.path().filename().string());

                if (!allFiles) {
                    std::wstring nameLower = name;
                    for (auto& c : nameLower) c = towlower(c);
                    if (!matchWildcard(nameLower, patLower)) continue;
                }

                FileInfo info;
                info.name = utf8ToWstring(std::filesystem::relative(entry.path(), dir).string());
                info.isDirectory = entry.is_directory();
                info.isHidden = entry.path().filename().string()[0] == '.';

                if (entry.is_regular_file()) {
                    info.size = entry.file_size();
                }

                struct stat st;
                std::string pathStr = entry.path().string();
                if (stat(pathStr.c_str(), &st) == 0) {
                    info.creationTime.seconds = st.st_ctim.tv_sec;
                    info.lastWriteTime.seconds = st.st_mtim.tv_sec;
                }

                files.push_back(info);
            }

            return true;
        } catch (...) {
            return false;
        }
    }

    // Path Operations
    std::wstring getFullPathName(const std::wstring& path) override {
        try {
            return utf8ToWstring(std::filesystem::absolute(toPath(path)).string());
        } catch (...) {
            return path;
        }
    }

    std::wstring getLongPathName(const std::wstring& path) override {
        // Linux doesn't have short path names like Windows
        return getFullPathName(path);
    }

    std::wstring getTempPath() override {
        const char* tmp = getenv("TMPDIR");
        if (!tmp) tmp = "/tmp";
        std::wstring result = utf8ToWstring(tmp);
        if (result.back() != L'/') result += L'/';
        return result;
    }

    std::wstring getCurrentDirectory() override {
        char buffer[PATH_MAX];
        if (getcwd(buffer, sizeof(buffer))) {
            return utf8ToWstring(buffer);
        }
        return L"";
    }

    bool setCurrentDirectory(const std::wstring& path) override {
        return chdir(wstringToUtf8(path).c_str()) == 0;
    }

    // Special Folders (XDG Base Directory)
    std::wstring getUserConfigDir() override {
        return getXdgDir("XDG_CONFIG_HOME", ".config") + L"/notepad++";
    }

    std::wstring getUserDataDir() override {
        return getXdgDir("XDG_DATA_HOME", ".local/share") + L"/notepad++";
    }

    std::wstring getUserCacheDir() override {
        return getXdgDir("XDG_CACHE_HOME", ".cache") + L"/notepad++";
    }

    std::wstring getProgramFilesDir() override {
        // Typically /usr/share or /usr/local/share
        return L"/usr/share/notepad++";
    }

    std::wstring getDocumentsDir() override {
        return getXdgDir("XDG_DOCUMENTS_DIR", "Documents");
    }

    // Disk Operations
    bool getDiskFreeSpace(const std::wstring& path, uint64_t& freeBytes) override {
        struct statvfs svfs;
        if (statvfs(wstringToUtf8(path).c_str(), &svfs) == 0) {
            freeBytes = static_cast<uint64_t>(svfs.f_bavail) * svfs.f_frsize;
            return true;
        }
        return false;
    }

    // File Time Operations
    bool getFileTime(const std::wstring& path, FileTime& creation,
                     FileTime& lastAccess, FileTime& lastWrite) override {
        struct stat st;
        if (stat(wstringToUtf8(path).c_str(), &st) != 0) return false;

        creation.seconds = st.st_ctim.tv_sec;
        creation.nanoseconds = st.st_ctim.tv_nsec;
        lastAccess.seconds = st.st_atim.tv_sec;
        lastAccess.nanoseconds = st.st_atim.tv_nsec;
        lastWrite.seconds = st.st_mtim.tv_sec;
        lastWrite.nanoseconds = st.st_mtim.tv_nsec;
        return true;
    }

    bool setFileTime(const std::wstring& path, const FileTime* creation,
                     const FileTime* lastAccess, const FileTime* lastWrite) override {
        std::string utf8Path = wstringToUtf8(path);

        struct timespec times[2];
        bool hasAccess = (lastAccess != nullptr);
        bool hasWrite = (lastWrite != nullptr);

        if (hasAccess) {
            times[0] = toTimespec(*lastAccess);
        } else {
            times[0].tv_nsec = UTIME_OMIT;
        }

        if (hasWrite) {
            times[1] = toTimespec(*lastWrite);
        } else {
            times[1].tv_nsec = UTIME_OMIT;
        }

        return utimensat(AT_FDCWD, utf8Path.c_str(), times, 0) == 0;
    }

private:
    std::wstring getXdgDir(const char* envVar, const char* defaultSubdir) {
        const char* value = getenv(envVar);
        if (value) {
            std::wstring result = utf8ToWstring(value);
            if (result.back() == L'/') result.pop_back();
            return result;
        }

        // Fallback to home directory + default subdir
        const char* home = getenv("HOME");
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            if (pw) home = pw->pw_dir;
        }

        if (home) {
            return utf8ToWstring(home) + L"/" + utf8ToWstring(defaultSubdir);
        }

        return L"/tmp";
    }

    std::filesystem::path getXdgTrashDir() {
        const char* dataHome = getenv("XDG_DATA_HOME");
        if (dataHome) {
            return std::filesystem::path(dataHome) / "Trash";
        }

        const char* home = getenv("HOME");
        if (home) {
            return std::filesystem::path(home) / ".local/share/Trash";
        }

        return std::filesystem::path("/tmp/Trash");
    }

    bool matchWildcard(const std::wstring& text, const std::wstring& pattern) {
        // Simple wildcard matching: * matches any sequence, ? matches single char
        size_t t = 0, p = 0;
        size_t star = std::wstring::npos, tstar = 0;

        while (t < text.size()) {
            if (p < pattern.size() && (pattern[p] == text[t] || pattern[p] == L'?')) {
                t++; p++;
            } else if (p < pattern.size() && pattern[p] == L'*') {
                star = p++;
                tstar = t;
            } else if (star != std::wstring::npos) {
                p = star + 1;
                t = ++tstar;
            } else {
                return false;
            }
        }

        while (p < pattern.size() && pattern[p] == L'*') p++;
        return p == pattern.size();
    }
};

// ============================================================================
// Singleton Accessor
// ============================================================================
IFileSystem& IFileSystem::getInstance() {
    static FileSystemLinux instance;
    return instance;
}

// ============================================================================
// Static Helper Implementations
// ============================================================================
std::wstring IFileSystem::pathAppend(const std::wstring& base, const std::wstring& append) {
    if (base.empty()) return append;
    if (append.empty()) return base;

    std::wstring result = base;
    // Normalize path separator
    for (auto& c : result) if (c == L'\\') c = L'/';

    if (result.back() != L'/') result += L'/';

    std::wstring app = append;
    for (auto& c : app) if (c == L'\\') c = L'/';

    // Remove leading slash from append if present
    if (!app.empty() && app.front() == L'/') app = app.substr(1);

    return result + app;
}

std::wstring IFileSystem::pathRemoveFileSpec(const std::wstring& path) {
    size_t pos = path.find_last_of(L"/\\");
    if (pos != std::wstring::npos) {
        return path.substr(0, pos);
    }
    return L"";
}

std::wstring IFileSystem::getFileName(const std::wstring& path) {
    size_t pos = path.find_last_of(L"/\\");
    if (pos != std::wstring::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

std::wstring IFileSystem::getDirectoryName(const std::wstring& path) {
    return pathRemoveFileSpec(path);
}

std::wstring IFileSystem::getExtension(const std::wstring& path) {
    size_t pos = path.find_last_of(L'.');
    size_t slashPos = path.find_last_of(L"/\\");
    if (pos != std::wstring::npos && (slashPos == std::wstring::npos || pos > slashPos)) {
        return path.substr(pos);
    }
    return L"";
}

std::wstring IFileSystem::changeExtension(const std::wstring& path, const std::wstring& ext) {
    size_t pos = path.find_last_of(L'.');
    size_t slashPos = path.find_last_of(L"/\\");
    if (pos != std::wstring::npos && (slashPos == std::wstring::npos || pos > slashPos)) {
        return path.substr(0, pos) + ext;
    }
    return path + ext;
}

bool IFileSystem::isRelativePath(const std::wstring& path) {
    if (path.empty()) return true;
    // Absolute paths start with / or ~
    if (path[0] == L'/' || path[0] == L'\\') return false;
    if (path[0] == L'~') return false;
    return true;
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
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);

    return FileTime(static_cast<uint64_t>(seconds.count()), static_cast<uint32_t>(nanos.count()));
}

// ============================================================================
// File Class Implementation
// ============================================================================
class File::Impl {
public:
    std::fstream file;
    uint32_t lastError = 0;

    void close() {
        if (file.is_open()) {
            file.close();
        }
    }

    std::ios::openmode getOpenMode(FileMode mode) {
        switch (mode) {
            case FileMode::Read: return std::ios::in | std::ios::binary;
            case FileMode::Write: return std::ios::out | std::ios::binary | std::ios::trunc;
            case FileMode::Append: return std::ios::out | std::ios::binary | std::ios::app;
            case FileMode::ReadWrite: return std::ios::in | std::ios::out | std::ios::binary;
        }
        return std::ios::in | std::ios::binary;
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

    std::string utf8Path = wstringToUtf8(path);
    _impl->file.open(utf8Path, _impl->getOpenMode(mode));

    if (!_impl->file.is_open()) {
        _impl->lastError = errno;
        return false;
    }

    return true;
}

void File::close() {
    _impl->close();
}

bool File::isOpen() const {
    return _impl->file.is_open();
}

size_t File::read(void* buffer, size_t bytesToRead) {
    if (!isOpen()) return 0;

    _impl->file.read(static_cast<char*>(buffer), bytesToRead);
    return _impl->file.gcount();
}

std::string File::readAll() {
    if (!isOpen()) return "";

    int64_t size = getSize();
    if (size <= 0) {
        // Read until EOF
        std::string result;
        char buffer[4096];
        while (true) {
            size_t bytesRead = read(buffer, sizeof(buffer));
            if (bytesRead == 0) break;
            result.append(buffer, bytesRead);
        }
        return result;
    }

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

    auto pos = _impl->file.tellp();
    _impl->file.write(static_cast<const char*>(buffer), bytesToWrite);

    if (_impl->file.fail()) {
        _impl->lastError = errno;
        return 0;
    }

    return bytesToWrite;
}

bool File::writeString(const std::string& str) {
    return write(str.c_str(), str.length()) == str.length();
}

bool File::writeString(const std::wstring& str) {
    // Convert to UTF-8
    std::string utf8 = wstringToUtf8(str);
    return writeString(utf8);
}

int64_t File::seek(int64_t offset, int origin) {
    if (!isOpen()) return -1;

    std::ios::seekdir dir;
    switch (origin) {
        case 0: dir = std::ios::beg; break;
        case 1: dir = std::ios::cur; break;
        case 2: dir = std::ios::end; break;
        default: return -1;
    }

    _impl->file.seekp(offset, dir);
    _impl->file.seekg(offset, dir);

    if (_impl->file.fail()) {
        return -1;
    }

    return tell();
}

int64_t File::tell() const {
    if (!isOpen()) return -1;
    return _impl->file.tellp();
}

int64_t File::getSize() const {
    if (!isOpen()) return -1;

    auto current = _impl->file.tellg();
    _impl->file.seekg(0, std::ios::end);
    auto size = _impl->file.tellg();
    _impl->file.seekg(current, std::ios::beg);

    return size;
}

bool File::flush() {
    if (!isOpen()) return false;
    _impl->file.flush();
    return !_impl->file.fail();
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
    (void)codepage; // Linux uses UTF-8 by default
    return writeFileContent(path, wstringToUtf8(content));
}

bool ensureDirectoryExists(const std::wstring& path) {
    return IFileSystem::getInstance().createDirectoryRecursive(path);
}

std::wstring getTempFilePath(const std::wstring& prefix) {
    std::wstring tempDir = IFileSystem::getInstance().getTempPath();

    // Generate unique filename
    static int counter = 0;
    std::wstring filename = prefix + L"_" + std::to_wstring(getpid()) + L"_" +
                            std::to_wstring(counter++) + L".tmp";

    return IFileSystem::pathAppend(tempDir, filename);
}

std::wstring resolvePath(const std::wstring& baseDir, const std::wstring& relativePath) {
    if (IFileSystem::isAbsolutePath(relativePath)) {
        return relativePath;
    }

    std::wstring combined = IFileSystem::pathAppend(baseDir, relativePath);
    return IFileSystem::getInstance().getFullPathName(combined);
}

std::wstring charToWchar(const std::string& str) {
    return utf8ToWstring(str);
}

std::string wcharToChar(const std::wstring& str) {
    return wstringToUtf8(str);
}

} // namespace FileSystemUtils

} // namespace PlatformLayer
