// This file is part of Notepad++ project
// Copyright (C)2021 Don HO <don.h@free.fr>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.

#pragma once

// Include platform-specific types
#ifdef _WIN32
#include "WindowsTypes.h"
#else
#include "LinuxTypes.h"
#endif

// Standard C++ headers
#include <algorithm>
#include <cstdint>
#include <locale>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "NppConstants.h"

#if defined(_MSC_VER)
#pragma deprecated(PathFileExists)
#pragma deprecated(PathIsDirectory)
#endif

// Utility function declarations
std::wstring folderBrowser(HWND parent, const std::wstring & title = L"", int outputCtrlID = 0, const wchar_t *defaultStr = NULL);
std::wstring getFolderName(HWND parent, const wchar_t *defaultDir = NULL);

void printInt(int int2print);
void printStr(const wchar_t *str2print);
std::wstring commafyInt(size_t n);

void writeLog(const wchar_t* logFileName, const char* log2write);
void writeLog(const wchar_t* logFileName, const wchar_t* log2write);
int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep);
std::wstring purgeMenuItemString(const wchar_t* menuItemStr, bool keepAmpersand = false);
std::vector<std::wstring> tokenizeString(const std::wstring & tokenString, const char delim);

void ClientRectToScreenRect(HWND hWnd, RECT* rect);
void ScreenRectToClientRect(HWND hWnd, RECT* rect);

std::wstring string2wstring(const std::string& rString, UINT codepage);
std::string wstring2string(const std::wstring& rwString, UINT codepage);
bool isInList(const wchar_t* token, const wchar_t* list);
std::wstring BuildMenuFileName(int filenameLen, unsigned int pos, const std::wstring &filename, bool ordinalNumber = true);

std::string getFileContent(const wchar_t* file2read, bool* pbFailed = nullptr);
std::wstring relativeFilePathToFullFilePath(const wchar_t *relativeFilePath);
void writeFileContent(const wchar_t *file2write, const char *content2write);
bool matchInList(const wchar_t *fileName, const std::vector<std::wstring> & patterns);
bool matchInExcludeDirList(const wchar_t* dirName, const std::vector<std::wstring>& patterns, size_t level);
bool allPatternsAreExclusion(const std::vector<std::wstring>& patterns);

class WcharMbcsConvertor final
{
public:
    static WcharMbcsConvertor& getInstance() {
        static WcharMbcsConvertor instance;
        return instance;
    }
    const wchar_t* char2wchar(const char* mbcs2Convert, size_t codepage, int lenMbcs = -1, int* pLenWc = nullptr, int* pBytesNotProcessed = nullptr);
    const wchar_t* char2wchar(const char* mbcs2Convert, size_t codepage, intptr_t* mstart, intptr_t* mend, int mbcsLen = 0);
    size_t getSizeW() const { return _wideCharStr.size(); }
    const char* wchar2char(const wchar_t* wcharStr2Convert, size_t codepage, int lenWc = -1, int* pLenMbcs = nullptr);
    const char* wchar2char(const wchar_t* wcharStr2Convert, size_t codepage, intptr_t* mstart, intptr_t* mend, int wcharLenIn = 0, int* lenOut = nullptr);
    size_t getSizeA() const { return _multiByteStr.size(); }
    const char* encode(UINT fromCodepage, UINT toCodepage, const char* txt2Encode, int lenIn = -1, int* pLenOut = nullptr, int* pBytesNotProcessed = nullptr);
private:
    WcharMbcsConvertor() = default;
    ~WcharMbcsConvertor() = default;
    WcharMbcsConvertor(const WcharMbcsConvertor&) = delete;
    WcharMbcsConvertor& operator= (const WcharMbcsConvertor&) = delete;
    WcharMbcsConvertor(WcharMbcsConvertor&&) = delete;
    WcharMbcsConvertor& operator= (WcharMbcsConvertor&&) = delete;
    template <class T> class StringBuffer final {
    public:
        ~StringBuffer() { if (_allocLen) delete[] _str; }
        void sizeTo(size_t size);
        T* begin() { return _str; }
        T* end() { return _str + _dataLen; }
        size_t size() const { return _dataLen; }
    private:
        size_t _allocLen = 0;
        size_t _dataLen = 0;
        T* _str = nullptr;
    };
    std::vector<char> _multiByteStr;
    std::vector<wchar_t> _wideCharStr;
};


// More utility functions
void trim(std::wstring& str);
int nbDigitsFromNbLines(size_t nbLines);
std::wstring getDateTimeStrFrom(const std::wstring& dateTimeFormat, const SYSTEMTIME& st);

class Version final
{
public:
    Version() = default;
    explicit Version(const std::wstring& versionStr);
    void setVersionFrom(const std::wstring& filePath);
    std::wstring toString() const;
    static bool isNumber(const std::wstring& s) {
        static const auto& loc = std::locale::classic();
        return !s.empty() &&
            std::find_if(s.begin(), s.end(), [](auto c) { return !std::isdigit(c, loc); }) == s.end();
    }
    int compareTo(const Version& v2c) const;
    bool operator < (const Version& v2c) const {
        return compareTo(v2c) == -1;
    }
    bool operator <= (const Version& v2c) const {
        int r = compareTo(v2c);
        return r == -1 || r == 0;
    }
    bool operator > (const Version& v2c) const {
        return compareTo(v2c) == 1;
    }
    bool operator >= (const Version& v2c) const {
        int r = compareTo(v2c);
        return r == 1 || r == 0;
    }
    bool operator == (const Version& v2c) const {
        return compareTo(v2c) == 0;
    }
    bool operator != (const Version& v2c) const {
        return compareTo(v2c) != 0;
    }
    bool empty() const {
        return _major == 0 && _minor == 0 && _patch == 0 && _build == 0;
    }
    bool isCompatibleTo(const Version& from, const Version& to) const;
private:
    unsigned long _major = 0;
    unsigned long _minor = 0;
    unsigned long _patch = 0;
    unsigned long _build = 0;
};

// File utilities
BOOL getDiskFreeSpaceWithTimeout(const wchar_t* dirPath, ULARGE_INTEGER* freeBytesForUser,
    DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);
BOOL getFileAttributesExWithTimeout(const wchar_t* filePath, WIN32_FILE_ATTRIBUTE_DATA* fileAttr,
    DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr, DWORD* pdwWin32ApiError = nullptr);

bool doesFileExist(const wchar_t* filePath, DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);
bool doesDirectoryExist(const wchar_t* dirPath, DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);
bool doesPathExist(const wchar_t* path, DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);

bool isWindowVisibleOnAnyMonitor(const RECT& rectWndIn);
bool isCoreWindows();

// String utilities
std::wstring stringToLower(const std::wstring& str);
std::wstring stringToUpper(const std::wstring& str);
std::wstring stringReplace(const std::wstring& str, const std::wstring& from, const std::wstring& to);
void stringSplit(const std::wstring& input, const std::wstring& delimiter, std::vector<std::wstring>& output);
void stringJoin(const std::vector<std::wstring>& strings, const std::wstring& separator, std::wstring& joinedString);

// Vector utilities - remove duplicate elements while preserving order
template<typename T>
size_t vecRemoveDuplicates(std::vector<T>& vec) {
    std::unordered_set<T> seen;
    size_t writeIndex = 0;
    for (size_t readIndex = 0; readIndex < vec.size(); ++readIndex) {
        if (seen.find(vec[readIndex]) == seen.end()) {
            seen.insert(vec[readIndex]);
            vec[writeIndex++] = vec[readIndex];
        }
    }
    vec.resize(writeIndex);
    return writeIndex;
}

// Windows-only classes
#ifdef _WIN32
class ControlInfoTip final
{
public:
    ControlInfoTip() = default;
    ~ControlInfoTip() {
        if (_hWndInfoTip) {
            hide();
        }
    }
    bool init(HINSTANCE hInst, HWND ctrl2attached, HWND ctrl2attachedParent, const std::wstring& tipStr, bool isRTL, unsigned int remainTimeMillisecond = 0, int maxWidth = 200);
    bool isValid() const { return _hWndInfoTip != nullptr; }
    enum showPosition { beginning, middle, end };
    void show(showPosition pos = middle) const;
    void hide();
    HWND getTipHandle() const { return _hWndInfoTip; }
private:
    HWND _hWndInfoTip = nullptr;
    TOOLINFO _toolInfo = {};
    ControlInfoTip(const ControlInfoTip&) = delete;
    ControlInfoTip& operator=(const ControlInfoTip&) = delete;
};

DWORD invokeNppUacOp(const std::wstring& strCmdLineParams);
bool fileTimeToYMD(const FILETIME& ft, int& yyyymmdd);
void expandEnv(const wchar_t *strSrc, wchar_t *strDest, size_t nbChar);

class ScopedCOMInit final
{
public:
    explicit ScopedCOMInit(DWORD initParam = COINIT_APARTMENTTHREADED) {
        HRESULT hr = CoInitializeEx(nullptr, initParam);
        if (SUCCEEDED(hr)) {
            _bInitialized = true;
        }
    }
    ~ScopedCOMInit() {
        if (_bInitialized) {
            _bInitialized = false;
            CoUninitialize();
        }
    }
    bool isInitialized() const { return _bInitialized; }
private:
    bool _bInitialized = false;
    ScopedCOMInit(const ScopedCOMInit&) = delete;
    ScopedCOMInit& operator=(const ScopedCOMInit&) = delete;
};
#endif // _WIN32

