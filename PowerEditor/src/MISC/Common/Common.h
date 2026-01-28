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


#pragma once

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#else
// Linux platform types
#include <cstdint>
#include <cwchar>
#include <cstdarg>
#include <cstring>
#include <cerrno>
#include <strings.h>  // For strcasecmp

using HWND = void*;
using UINT = unsigned int;
using WPARAM = uintptr_t;
using LPARAM = intptr_t;
using LRESULT = intptr_t;
using UINT_PTR = uintptr_t;
using INT_PTR = intptr_t;
using BYTE = uint8_t;
using UCHAR = unsigned char;
using WORD = uint16_t;
using DWORD = uint32_t;
using BOOL = int;
using INT = int;
using LONG = long;
using WCHAR = wchar_t;
using TCHAR = wchar_t;
using LPWSTR = wchar_t*;
using LPCWSTR = const wchar_t*;
using PWSTR = wchar_t*;
using LPTSTR = wchar_t*;  // TCHAR is wchar_t on Linux
using _locale_t = void*;
using HFONT = void*;
using HBRUSH = void*;
using HBITMAP = void*;
using HICON = void*;
using HCURSOR = void*;
using HPEN = void*;
using HDC = void*;
using HANDLE = void*;
using COLORREF = uint32_t;
using HINSTANCE = void*;
using HRESULT = long;
using DWORD_PTR = uintptr_t;
using ULONG_PTR = uintptr_t;

// Basic structures must come before DRAWITEMSTRUCT and pointer types
#ifndef RECT_DEFINED
#define RECT_DEFINED
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};
#endif

#ifndef POINT_DEFINED
#define POINT_DEFINED
struct POINT {
    long x;
    long y;
};
#endif

#ifndef SIZE_DEFINED
#define SIZE_DEFINED
struct SIZE {
    long cx;
    long cy;
};
#endif

// Pointer types (must come after structure definitions)
using LPRECT = RECT*;
using LPCRECT = const RECT*;
using LPPOINT = POINT*;
using LPSIZE = SIZE*;

// DRAWITEMSTRUCT
struct DRAWITEMSTRUCT {
    UINT CtlType;
    UINT CtlID;
    UINT itemID;
    UINT itemAction;
    UINT itemState;
    HWND hwndItem;
    HDC hDC;
    RECT rcItem;
    ULONG_PTR itemData;
};

// Stub functions
inline void DestroyIcon(void*) {}
inline void DestroyWindow(void*) {}

// String functions
inline int lstrcmp(const wchar_t* s1, const wchar_t* s2) {
    return wcscmp(s1, s2);
}

inline int wsprintfW(wchar_t* buffer, const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vswprintf(buffer, 1024, format, args);  // Assume max 1024 chars
    va_end(args);
    return result;
}

using errno_t = int;

inline errno_t wcscpy_s(wchar_t* dest, size_t destSize, const wchar_t* src) {
    if (!dest || !src) return EINVAL;
    if (wcslen(src) >= destSize) return ERANGE;
    wcscpy(dest, src);
    return 0;
}

// Color conversion stub
struct HLSCOLOR {
    WORD hue;
    WORD lightness;
    WORD saturation;
};

inline void ColorRGBToHLS(COLORREF rgb, WORD* h, WORD* l, WORD* s) {
    // Stub implementation - set default values
    *h = 0; *l = 120; *s = 120; // Medium gray
}

inline COLORREF ColorHLSToRGB(WORD hue, WORD lightness, WORD saturation) {
    // Stub implementation - return medium gray
    (void)hue; (void)lightness; (void)saturation;
    return 0x808080; // Medium gray RGB
}

// CALLBACK macro
#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Code page constants - only define if not already defined
#ifndef CP_UTF8_DEFINED
#define CP_UTF8_DEFINED
const UINT CP_ACP = 0;
const UINT CP_UTF8 = 65001;
#endif

// SetWindowPos flags
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_NOREDRAW 0x0008
#define SWP_NOACTIVATE 0x0010
#define SWP_SHOWWINDOW 0x0040

// Button states
#define BST_CHECKED 0x0001
#define BST_UNCHECKED 0x0000

// Messages
#define BM_GETCHECK 0x00F0
#define BM_SETCHECK 0x00F1

struct SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

struct FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

struct WIN32_FILE_ATTRIBUTE_DATA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
};

#define ULARGE_INTEGER uint64_t

// ToolTip structure
struct TOOLINFO {
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    UINT_PTR uId;
    RECT rect;
    HINSTANCE hinst;
    LPWSTR lpszText;
    LPARAM lParam;
};

// COM-related definitions
#define COINIT_APARTMENTTHREADED 0x2
#define COINIT_MULTITHREADED 0x0
#define RPC_E_CHANGED_MODE 0x80010106L
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define S_OK 0

inline HRESULT CoInitializeEx(void*, DWORD) { return S_OK; }
inline void CoUninitialize() {}

// Menu types
using HMENU = void*;
using HACCEL = void*;

// ACCEL structure for accelerator table
struct ACCEL {
    BYTE fVirt;
    WORD key;
    WORD cmd;
};

// Accelerator table stub
inline void DestroyAcceleratorTable(void*) {}

// Virtual Key codes
#define VK_DOWN     0x28
#define VK_UP       0x26
#define VK_LEFT     0x25
#define VK_RIGHT    0x27
#define VK_HOME     0x24
#define VK_END      0x23
#define VK_PRIOR    0x21  // Page Up
#define VK_NEXT     0x22  // Page Down
#define VK_DELETE   0x2E
#define VK_INSERT   0x2D
#define VK_ESCAPE   0x1B
#define VK_BACK     0x08
#define VK_TAB      0x09
#define VK_RETURN   0x0D
#define VK_ADD      0x6B
#define VK_SUBTRACT 0x6D
#define VK_DIVIDE   0x6F
#define VK_OEM_2    0xBF  // '/?' key
#define VK_OEM_3    0xC0  // '`~' key
#define VK_OEM_4    0xDB  // '[{' key
#define VK_OEM_5    0xDC  // '\\|' key
#define VK_OEM_6    0xDD  // ']}' key
#define VK_SPACE    0x20
#define VK_CAPITAL  0x14  // Caps Lock

// Function keys
#define VK_F1     0x70
#define VK_F2     0x71
#define VK_F3     0x72
#define VK_F4     0x73
#define VK_F5     0x74
#define VK_F6     0x75
#define VK_F7     0x76
#define VK_F8     0x77
#define VK_F9     0x78
#define VK_F10    0x79
#define VK_F11    0x7A
#define VK_F12    0x7B

// Numpad keys
#define VK_NUMPAD0  0x60
#define VK_NUMPAD1  0x61
#define VK_NUMPAD2  0x62
#define VK_NUMPAD3  0x63
#define VK_NUMPAD4  0x64
#define VK_NUMPAD5  0x65
#define VK_NUMPAD6  0x66
#define VK_NUMPAD7  0x67
#define VK_NUMPAD8  0x68
#define VK_NUMPAD9  0x69

// Locale constants
#define LOCALE_NAME_SYSTEM_DEFAULT L""
#define LOCALE_IDEFAULTANSICODEPAGE 0x00001004
#define LOCALE_RETURN_NUMBER 0x20000000

// Architecture constants
#define IMAGE_FILE_MACHINE_I386  0x014c
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_FILE_MACHINE_ARM64 0xaa64

// Stub for GetSystemTimeAsFileTime
inline void GetSystemTimeAsFileTime(FILETIME* ft) {
    if (ft) {
        ft->dwLowDateTime = 0;
        ft->dwHighDateTime = 0;
    }
}

// Stub for GetLocaleInfoEx
inline int GetLocaleInfoEx(const wchar_t*, uint32_t, wchar_t*, int) {
    return 0;
}

// Accelerator virtual key flags
#define FVIRTKEY  0x01
#define FNOINVERT 0x02
#define FSHIFT    0x04
#define FCONTROL  0x08
#define FALT      0x10

// Toolbar status enum (needed in global scope for Parameters.h)
enum toolBarStatusType { TB_SMALL, TB_LARGE, TB_SMALL2, TB_LARGE2, TB_STANDARD };

// File access constants
#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define NO_ERROR 0

// __inout macro for SAL annotations
#ifndef __inout
#define __inout
#endif

// Event/Handle functions
inline void* CreateEvent(void*, BOOL, BOOL, const wchar_t*) { return nullptr; }
inline BOOL SetEvent(void*) { return TRUE; }
inline BOOL CloseHandle(void*) { return TRUE; }

// MessageBox constants and stub
// Button types
#define MB_OK                   0x00000000
#define MB_OKCANCEL             0x00000001
#define MB_ABORTRETRYIGNORE     0x00000002
#define MB_YESNOCANCEL          0x00000003
#define MB_YESNO                0x00000004
#define MB_RETRYCANCEL          0x00000005
#define MB_CANCELTRYCONTINUE    0x00000006

// Icon types
#define MB_ICONERROR            0x00000010
#define MB_ICONHAND             MB_ICONERROR
#define MB_ICONQUESTION         0x00000020
#define MB_ICONWARNING          0x00000030
#define MB_ICONINFORMATION      0x00000040

// Return values
#define IDOK                    1
#define IDCANCEL                2
#define IDABORT                 3
#define IDRETRY                 4
#define IDIGNORE                5
#define IDYES                   6
#define IDNO                    7
#define IDTRYAGAIN              10
#define IDCONTINUE              11

inline int MessageBox(void*, const wchar_t*, const wchar_t*, UINT) { return 0; }

// Boolean values
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Menu flags
#define TPM_LEFTALIGN 0x0000
#define MF_ENABLED 0x0000
#define MF_BYCOMMAND 0x0000
#define MF_DISABLED 0x0002
#define MF_GRAYED 0x0001
#define MF_CHECKED 0x0008
#define MF_UNCHECKED 0x0000

// Menu flags continued
#define MF_BYPOSITION 0x0400
#define MF_POPUP 0x0010
#define MF_STRING 0x0000

// Menu functions (stubs for Linux)
inline void TrackPopupMenu(void*, int, int, int, int, void*, void*) {}
inline void EnableMenuItem(void*, int, int) {}
inline void CheckMenuItem(void*, int, int) {}
inline void ModifyMenu(void*, int, int, void*, const wchar_t*) {}
inline void* GetSubMenu(void*, int) { return nullptr; }
inline int GetMenuString(void*, int, wchar_t*, int, int) { return 0; }
inline int GetMenuItemID(void*, int) { return 0; }

// Window functions (stubs for Linux)
inline void SetWindowText(void*, const wchar_t*) {}
inline void* GetDlgItem(void*, int) { return nullptr; }

// String comparison functions
inline int _stricmp(const char* s1, const char* s2) { return strcasecmp(s1, s2); }
inline int _wcsicmp(const wchar_t* s1, const wchar_t* s2) { return wcscasecmp(s1, s2); }

// tchar function
#define _istspace iswspace

#endif // _WIN32

#include <algorithm>
#include <cstdint>
#include <locale>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "NppConstants.h"

#if defined(_MSC_VER)
#pragma deprecated(PathFileExists)  // Use doesFileExist, doesDirectoryExist or doesPathExist (for file or directory) instead.
#pragma deprecated(PathIsDirectory) // Use doesDirectoryExist instead.
#endif



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

std::wstring string2wstring(const std::string& rString, UINT codepage = CP_UTF8);
std::string wstring2string(const std::wstring& rwString, UINT codepage = CP_UTF8);
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

	const wchar_t* char2wchar(const char* mbcs2Convert, size_t codepage, int lenMbcs = -1, int* pLenWc = nullptr, int* pBytesNotProcessed = NULL);
	const wchar_t* char2wchar(const char* mbcs2Convert, size_t codepage, intptr_t* mstart, intptr_t* mend, int mbcsLen = 0);
	size_t getSizeW() const { return _wideCharStr.size(); }
	const char* wchar2char(const wchar_t* wcharStr2Convert, size_t codepage, int lenWc = -1, int* pLenMbcs = nullptr);
	const char* wchar2char(const wchar_t* wcharStr2Convert, size_t codepage, intptr_t* mstart, intptr_t* mend, int wcharLenIn = 0, int* lenOut = nullptr);
	size_t getSizeA() const { return _multiByteStr.size(); }

	const char* encode(UINT fromCodepage, UINT toCodepage, const char* txt2Encode, int lenIn = -1, int* pLenOut = NULL, int* pBytesNotProcessed = NULL) {
		int lenWc = 0;
		const wchar_t* strW = char2wchar(txt2Encode, fromCodepage, lenIn, &lenWc, pBytesNotProcessed);
		return wchar2char(strW, toCodepage, lenWc, pLenOut);
	}

protected:
	WcharMbcsConvertor() = default;
	~WcharMbcsConvertor() = default;

	// Since there's no public ctor, we need to void the default assignment operator and copy ctor.
	// Since these are marked as deleted does not matter under which access specifier are kept
	WcharMbcsConvertor(const WcharMbcsConvertor&) = delete;
	WcharMbcsConvertor& operator= (const WcharMbcsConvertor&) = delete;

	// No move ctor and assignment
	WcharMbcsConvertor(WcharMbcsConvertor&&) = delete;
	WcharMbcsConvertor& operator= (WcharMbcsConvertor&&) = delete;

	template <class T> class StringBuffer final
	{
	public:
		~StringBuffer() { if (_allocLen) delete[] _str; }

		void sizeTo(size_t size) {
			if (_allocLen < size + 1)
			{
				if (_allocLen)
					delete[] _str;
				_allocLen = std::max<size_t>(size + 1, initSize);
				_str = new T[_allocLen]{};
			}
			_dataLen = size;
		}

		void empty() {
			static T nullStr = 0; // routines may return an empty string, with null terminator, without allocating memory; a pointer to this null character will be returned in that case
			if (_allocLen == 0)
				_str = &nullStr;
			else
				_str[0] = 0;
			_dataLen = 0;
		}

		size_t size() const { return _dataLen; }
		operator T* () { return _str; }
		operator const T* () const { return _str; }

	protected:
		static constexpr int initSize = 1024;
		size_t _allocLen = 0;
		size_t _dataLen = 0;
		T* _str = nullptr;
	};

	StringBuffer<char> _multiByteStr;
	StringBuffer<wchar_t> _wideCharStr;
};

std::wstring pathRemoveFileSpec(std::wstring & path);
std::wstring pathAppend(std::wstring &strDest, const std::wstring & str2append);
COLORREF getCtrlBgColor(HWND hWnd);
std::wstring stringToUpper(std::wstring strToConvert);
std::wstring stringToLower(std::wstring strToConvert);
std::wstring stringReplace(std::wstring subject, const std::wstring& search, const std::wstring& replace);
void stringSplit(const std::wstring& input, const std::wstring& delimiter, std::vector<std::wstring>& output);
bool str2numberVector(std::wstring str2convert, std::vector<size_t>& numVect);
void stringJoin(const std::vector<std::wstring>& strings, const std::wstring& separator, std::wstring& joinedString);
std::wstring stringTakeWhileAdmissable(const std::wstring& input, const std::wstring& admissable);
double stodLocale(const std::wstring& str, _locale_t loc, size_t* idx = NULL);

bool str2Clipboard(const std::wstring &str2cpy, HWND hwnd);
std::wstring strFromClipboard();
class Buffer;
bool buf2Clipboard(const std::vector<Buffer*>& buffers, bool isFullPath, HWND hwnd);

std::wstring GetLastErrorAsString(DWORD errorCode = 0);

std::wstring intToString(int val);
std::wstring uintToString(unsigned int val);

HWND CreateToolTip(int toolID, HWND hDlg, HINSTANCE hInst, const PWSTR pszText, bool isRTL);
HWND CreateToolTipRect(int toolID, HWND hWnd, HINSTANCE hInst, const PWSTR pszText, const RECT rc);

bool isCertificateValidated(const std::wstring & fullFilePath, const std::wstring & subjectName2check);
bool isAssoCommandExisting(LPCWSTR FullPathName);

bool deleteFileOrFolder(const std::wstring& f2delete);

void getFilesInFolder(std::vector<std::wstring>& files, const std::wstring& extTypeFilter, const std::wstring& inFolder);

template<typename T> size_t vecRemoveDuplicates(std::vector<T>& vec, bool isSorted = false, bool canSort = false)
{
	if (!isSorted && canSort)
	{
		std::sort(vec.begin(), vec.end());
		isSorted = true;
	}

	if (isSorted)
	{
		typename std::vector<T>::iterator it;
		it = std::unique(vec.begin(), vec.end());
		vec.resize(distance(vec.begin(), it));  // unique() does not shrink the vector
	}
	else
	{
		std::unordered_set<T> seen;
		auto newEnd = std::remove_if(vec.begin(), vec.end(), [&seen](const T& value)
			{
				return !seen.insert(value).second;
			});
		vec.erase(newEnd, vec.end());
	}
	return vec.size();
}

void trim(std::wstring& str);

int nbDigitsFromNbLines(size_t nbLines);

std::wstring getDateTimeStrFrom(const std::wstring& dateTimeFormat, const SYSTEMTIME& st);

HFONT createFont(const wchar_t* fontName, int fontSize, bool isBold, HWND hDestParent);
bool removeReadOnlyFlagFromFileAttributes(const wchar_t* fileFullPath);
bool toggleReadOnlyFlagFromFileAttributes(const wchar_t* fileFullPath, bool& isChangedToReadOnly);

bool isWin32NamespacePrefixedFileName(const std::wstring& fileName);
bool isWin32NamespacePrefixedFileName(const wchar_t* szFileName);
bool isUnsupportedFileName(const std::wstring& fileName);
bool isUnsupportedFileName(const wchar_t* szFileName);

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
			find_if(s.begin(), s.end(), [](auto c) { return !std::isdigit(c, loc); }) == s.end();
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


BOOL getDiskFreeSpaceWithTimeout(const wchar_t* dirPath, ULARGE_INTEGER* freeBytesForUser,
	DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);
BOOL getFileAttributesExWithTimeout(const wchar_t* filePath, WIN32_FILE_ATTRIBUTE_DATA* fileAttr,
	DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr, DWORD* pdwWin32ApiError = nullptr);

bool doesFileExist(const wchar_t* filePath, DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);
bool doesDirectoryExist(const wchar_t* dirPath, DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);
bool doesPathExist(const wchar_t* path, DWORD milliSec2wait = 0, bool* isTimeoutReached = nullptr);


// check if the window rectangle intersects with any currently active monitor's working area
bool isWindowVisibleOnAnyMonitor(const RECT& rectWndIn);

bool isCoreWindows();


class ControlInfoTip final
{
public:
	ControlInfoTip() = default;
	~ControlInfoTip() {
		if (_hWndInfoTip) {
			hide();
		}
	}

	bool init(HINSTANCE hInst, HWND ctrl2attached, HWND ctrl2attachedParent, const std::wstring& tipStr, bool isRTL, unsigned int remainTimeMillisecond = 0, int maxWidth = 200); // remainTimeMillisecond = 0: no timeout

	bool isValid() const {
		return _hWndInfoTip != nullptr;
	}

	HWND getTipHandle() const {
		return _hWndInfoTip;
	}

	enum showPosition {beginning, middle, end};
	void show(showPosition pos = middle) const;
	
	void hide();

private:
	HWND _hWndInfoTip = nullptr;
	TOOLINFO _toolInfo = {};

	ControlInfoTip(const ControlInfoTip&) = delete;
	ControlInfoTip& operator=(const ControlInfoTip&) = delete;
};

DWORD invokeNppUacOp(const std::wstring& strCmdLineParams);
bool fileTimeToYMD(const FILETIME& ft, int& yyyymmdd);
void expandEnv(std::wstring& path2Expand);

class ScopedCOMInit final // never use this in DllMain
{
public:
	ScopedCOMInit() {
		HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // attempt STA init 1st (older CoInitialize(NULL))
		if (hr == RPC_E_CHANGED_MODE) {
			hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED); // STA init failed, switch to MTA
		}
		if (SUCCEEDED(hr)) {
			// S_OK or S_FALSE, both needs subsequent CoUninitialize()
			_bInitialized = true;
		}
	}

	~ScopedCOMInit() {
		if (_bInitialized) {
			_bInitialized = false;
			::CoUninitialize();
		}
	}

	bool isInitialized() const {
		return _bInitialized;
	}

private:
	bool _bInitialized = false;

	ScopedCOMInit(const ScopedCOMInit&) = delete;
	ScopedCOMInit& operator=(const ScopedCOMInit&) = delete;
};
