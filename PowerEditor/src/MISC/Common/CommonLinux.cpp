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

// ============================================================================
// CommonLinux.cpp - Linux implementations of Common utility functions
// ============================================================================
// This file provides Linux-compatible implementations of utility functions
// that are in Common.cpp for Windows. Common.cpp uses Windows-specific APIs
// (MultiByteToWideChar, WideCharToMultiByte) that are not available on Linux.
//
// These implementations use standard C++ and iconv for character conversion.
// ============================================================================

#include "Common.h"
#include "WinControls/shortcut/shortcut.h"

#ifndef _WIN32

// System headers must be included first
#include <sys/stat.h>
#include <unistd.h>

#include <iconv.h>
#include <cstring>
#include <cstdlib>
#include <locale>
#include <codecvt>
#include <algorithm>

// NppDarkMode header for Linux stub implementation
#include "NppDarkMode.h"

using namespace std;

// ============================================================================
// WcharMbcsConvertor Implementation for Linux
// ============================================================================

// Helper function to get iconv encoding name from Windows codepage
static const char* getIconvEncoding(size_t codepage)
{
    switch (codepage)
    {
        case 65001: // CP_UTF8
            return "UTF-8";
        case 1200:  // UTF-16 LE
            return "UTF-16LE";
        case 1201:  // UTF-16 BE
            return "UTF-16BE";
        case 1252:  // Windows-1252 (Western European)
            return "WINDOWS-1252";
        case 1251:  // Windows-1251 (Cyrillic)
            return "WINDOWS-1251";
        case 1250:  // Windows-1250 (Central European)
            return "WINDOWS-1250";
        case 936:   // GB2312 (Simplified Chinese)
            return "GB2312";
        case 950:   // BIG5 (Traditional Chinese)
            return "BIG5";
        case 932:   // SHIFT_JIS (Japanese)
            return "SHIFT_JIS";
        case 949:   // EUC-KR (Korean)
            return "EUC-KR";
        case 1253:  // Windows-1253 (Greek)
            return "WINDOWS-1253";
        case 1254:  // Windows-1254 (Turkish)
            return "WINDOWS-1254";
        case 1255:  // Windows-1255 (Hebrew)
            return "WINDOWS-1255";
        case 1256:  // Windows-1256 (Arabic)
            return "WINDOWS-1256";
        case 1257:  // Windows-1257 (Baltic)
            return "WINDOWS-1257";
        case 1258:  // Windows-1258 (Vietnamese)
            return "WINDOWS-1258";
        case 850:   // CP850 (DOS Western European)
            return "CP850";
        case 866:   // CP866 (DOS Russian)
            return "CP866";
        case 437:   // CP437 (DOS US)
            return "CP437";
        case 0:     // CP_ACP (default ANSI codepage)
        default:
            return "UTF-8";  // Default to UTF-8
    }
}

// Helper template to resize StringBuffer
template <class T>
void WcharMbcsConvertor::StringBuffer<T>::sizeTo(size_t size)
{
    if (size + 1 > _allocLen)
    {
        if (_allocLen > 0)
            delete[] _str;
        _allocLen = size + 1;
        _str = new T[_allocLen];
    }
    _dataLen = size;
}

// Explicit instantiation for char and wchar_t
template void WcharMbcsConvertor::StringBuffer<char>::sizeTo(size_t);
template void WcharMbcsConvertor::StringBuffer<wchar_t>::sizeTo(size_t);

const wchar_t* WcharMbcsConvertor::char2wchar(const char* mbcs2Convert, size_t codepage, int lenMbcs, int* pLenWc, int* pBytesNotProcessed)
{
    // Do not process NULL pointer
    if (!mbcs2Convert)
        return nullptr;

    // Do not process empty strings
    if (lenMbcs == 0 || (lenMbcs == -1 && mbcs2Convert[0] == 0))
    {
        _wideCharStr.clear();
        return _wideCharStr.data();
    }

    // Determine input length
    size_t inputLen = (lenMbcs == -1) ? strlen(mbcs2Convert) : static_cast<size_t>(lenMbcs);

    // For UTF-8, use standard C++ conversion
    if (codepage == 65001 || codepage == 0)  // UTF-8 or CP_ACP
    {
        try
        {
            // Use std::wstring_convert for UTF-8 to wchar_t conversion
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring result = converter.from_bytes(mbcs2Convert, mbcs2Convert + inputLen);

            _wideCharStr.resize(result.size() + 1);
            memcpy(_wideCharStr.data(), result.c_str(), result.size() * sizeof(wchar_t));
            _wideCharStr[result.size()] = L'\0';

            if (pLenWc)
                *pLenWc = static_cast<int>(result.size());
            if (pBytesNotProcessed)
                *pBytesNotProcessed = 0;

            return _wideCharStr.data();
        }
        catch (...)
        {
            // Fallback: treat as Latin-1
            _wideCharStr.resize(inputLen + 1);
            for (size_t i = 0; i < inputLen; ++i)
            {
                _wideCharStr[i] = static_cast<wchar_t>(static_cast<unsigned char>(mbcs2Convert[i]));
            }
            _wideCharStr[inputLen] = L'\0';

            if (pLenWc)
                *pLenWc = static_cast<int>(inputLen);
            if (pBytesNotProcessed)
                *pBytesNotProcessed = 0;

            return _wideCharStr.data();
        }
    }

    // For other codepages, use iconv
    iconv_t cd = iconv_open("WCHAR_T", getIconvEncoding(codepage));
    if (cd == (iconv_t)-1)
    {
        // Fallback: treat as Latin-1
        _wideCharStr.resize(inputLen + 1);
        for (size_t i = 0; i < inputLen; ++i)
        {
            _wideCharStr[i] = static_cast<wchar_t>(static_cast<unsigned char>(mbcs2Convert[i]));
        }
        _wideCharStr[inputLen] = L'\0';

        if (pLenWc)
            *pLenWc = static_cast<int>(inputLen);
        if (pBytesNotProcessed)
            *pBytesNotProcessed = 0;

        return _wideCharStr.data();
    }

    // Allocate buffer for conversion (wchar_t can be up to 4 bytes)
    size_t outputBufSize = inputLen * 4 + 4;
    _wideCharStr.resize(outputBufSize / sizeof(wchar_t) + 1);

    char* inBuf = const_cast<char*>(mbcs2Convert);
    size_t inBytesLeft = inputLen;
    char* outBuf = reinterpret_cast<char*>(_wideCharStr.data());
    size_t outBytesLeft = outputBufSize;

    size_t result = iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft);
    iconv_close(cd);

    if (result == (size_t)-1 && inBytesLeft > 0)
    {
        // Conversion error - fallback to Latin-1
        _wideCharStr.resize(inputLen + 1);
        for (size_t i = 0; i < inputLen; ++i)
        {
            _wideCharStr[i] = static_cast<wchar_t>(static_cast<unsigned char>(mbcs2Convert[i]));
        }
        _wideCharStr[inputLen] = L'\0';

        if (pBytesNotProcessed)
            *pBytesNotProcessed = 0;
    }
    else
    {
        size_t convertedChars = (outputBufSize - outBytesLeft) / sizeof(wchar_t);
        _wideCharStr[convertedChars] = L'\0';

        if (pBytesNotProcessed)
            *pBytesNotProcessed = static_cast<int>(inBytesLeft);
    }

    if (pLenWc)
        *pLenWc = static_cast<int>(wcslen(_wideCharStr.data()));

    return _wideCharStr.data();
}

const wchar_t* WcharMbcsConvertor::char2wchar(const char* mbcs2Convert, size_t codepage, intptr_t* mstart, intptr_t* mend, int mbcsLen)
{
    // Do not process NULL pointer
    if (!mbcs2Convert)
    {
        _wideCharStr.clear();
        if (mstart) *mstart = 0;
        if (mend) *mend = 0;
        return _wideCharStr.data();
    }

    if (mbcsLen == 0 || (mbcsLen == -1 && mbcs2Convert[0] == 0))
    {
        _wideCharStr.clear();
        if (mstart) *mstart = 0;
        if (mend) *mend = 0;
        return _wideCharStr.data();
    }

    // First convert the entire string
    const wchar_t* result = char2wchar(mbcs2Convert, codepage, mbcsLen, nullptr, nullptr);

    // Convert the byte positions to character positions
    if (mstart && mend)
    {
        const intptr_t mbcsLen2 = mbcsLen ? mbcsLen : static_cast<intptr_t>(strlen(mbcs2Convert));
        if (*mstart < mbcsLen2 && *mend <= mbcsLen2)
        {
            // For UTF-8, we need to count characters up to the byte position
            if (codepage == 65001 || codepage == 0)
            {
                *mstart = static_cast<intptr_t>(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(mbcs2Convert, mbcs2Convert + *mstart).size());
                *mend = static_cast<intptr_t>(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(mbcs2Convert, mbcs2Convert + *mend).size());
            }
            else
            {
                // For single-byte encodings, byte position = character position
                // This is a simplification that works for most legacy encodings
            }
        }
        else
        {
            *mstart = 0;
            *mend = 0;
        }
    }

    return result;
}

const char* WcharMbcsConvertor::wchar2char(const wchar_t* wcharStr2Convert, size_t codepage, int lenWc, int* pLenMbcs)
{
    if (!wcharStr2Convert)
        return nullptr;

    if (lenWc == 0 || (lenWc == -1 && wcharStr2Convert[0] == 0))
    {
        _multiByteStr.clear();
        return _multiByteStr.data();
    }

    // Determine input length
    size_t inputLen = (lenWc == -1) ? wcslen(wcharStr2Convert) : static_cast<size_t>(lenWc);

    // For UTF-8, use standard C++ conversion
    if (codepage == 65001 || codepage == 0)  // UTF-8 or CP_ACP
    {
        try
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::string result = converter.to_bytes(wcharStr2Convert, wcharStr2Convert + inputLen);

            _multiByteStr.resize(result.size() + 1);
            memcpy(_multiByteStr.data(), result.c_str(), result.size());
            _multiByteStr[result.size()] = '\0';

            if (pLenMbcs)
                *pLenMbcs = static_cast<int>(result.size());

            return _multiByteStr.data();
        }
        catch (...)
        {
            // Fallback: truncate to single bytes
            _multiByteStr.resize(inputLen + 1);
            for (size_t i = 0; i < inputLen; ++i)
            {
                _multiByteStr[i] = (wcharStr2Convert[i] > 255) ? '?' : static_cast<char>(wcharStr2Convert[i]);
            }
            _multiByteStr[inputLen] = '\0';

            if (pLenMbcs)
                *pLenMbcs = static_cast<int>(inputLen);

            return _multiByteStr.data();
        }
    }

    // For other codepages, use iconv
    iconv_t cd = iconv_open(getIconvEncoding(codepage), "WCHAR_T");
    if (cd == (iconv_t)-1)
    {
        // Fallback: truncate to single bytes
        _multiByteStr.resize(inputLen + 1);
        for (size_t i = 0; i < inputLen; ++i)
        {
            _multiByteStr[i] = (wcharStr2Convert[i] > 255) ? '?' : static_cast<char>(wcharStr2Convert[i]);
        }
        _multiByteStr[inputLen] = '\0';

        if (pLenMbcs)
            *pLenMbcs = static_cast<int>(inputLen);

        return _multiByteStr.data();
    }

    // Allocate buffer for conversion
    size_t outputBufSize = inputLen * 6 + 4;  // Worst case: 6 bytes per character
    _multiByteStr.resize(outputBufSize);

    char* inBuf = reinterpret_cast<char*>(const_cast<wchar_t*>(wcharStr2Convert));
    size_t inBytesLeft = inputLen * sizeof(wchar_t);
    char* outBuf = _multiByteStr.data();
    size_t outBytesLeft = outputBufSize;

    size_t result = iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft);
    iconv_close(cd);

    size_t convertedBytes = outputBufSize - outBytesLeft;
    _multiByteStr[convertedBytes] = '\0';

    if (pLenMbcs)
        *pLenMbcs = static_cast<int>(convertedBytes);

    return _multiByteStr.data();
}

const char* WcharMbcsConvertor::wchar2char(const wchar_t* wcharStr2Convert, size_t codepage, intptr_t* mstart, intptr_t* mend, int wcharLenIn, int* lenOut)
{
    if (!wcharStr2Convert)
    {
        _multiByteStr.clear();
        if (mstart) *mstart = 0;
        if (mend) *mend = 0;
        return _multiByteStr.data();
    }

    if (wcharLenIn == 0 || (wcharLenIn == -1 && wcharStr2Convert[0] == 0))
    {
        _multiByteStr.clear();
        if (mstart) *mstart = 0;
        if (mend) *mend = 0;
        return _multiByteStr.data();
    }

    // First convert the entire string
    const char* result = wchar2char(wcharStr2Convert, codepage, wcharLenIn, lenOut);

    // Convert character positions to byte positions
    if (mstart && mend)
    {
        const intptr_t wcharLenIn2 = wcharLenIn ? wcharLenIn : static_cast<intptr_t>(wcslen(wcharStr2Convert));
        if (*mstart < wcharLenIn2 && *mend < wcharLenIn2)
        {
            // Calculate byte positions by converting substrings
            // This is approximate and works best for UTF-8
            if (codepage == 65001 || codepage == 0)
            {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::string prefix = converter.to_bytes(wcharStr2Convert, wcharStr2Convert + *mstart);
                std::string upToEnd = converter.to_bytes(wcharStr2Convert, wcharStr2Convert + *mend);
                *mstart = static_cast<intptr_t>(prefix.size());
                *mend = static_cast<intptr_t>(upToEnd.size());
            }
            else
            {
                // For single-byte encodings
                // This is a simplification
            }
        }
        else
        {
            *mstart = 0;
            *mend = 0;
        }
    }

    return result;
}

const char* WcharMbcsConvertor::encode(UINT fromCodepage, UINT toCodepage, const char* txt2Encode, int lenIn, int* pLenOut, int* pBytesNotProcessed)
{
    if (!txt2Encode)
        return nullptr;

    if (lenIn == 0 || (lenIn == -1 && txt2Encode[0] == 0))
    {
        _multiByteStr.clear();
        return _multiByteStr.data();
    }

    // First convert to wide char, then to target encoding
    const wchar_t* wideStr = char2wchar(txt2Encode, fromCodepage, lenIn, nullptr, pBytesNotProcessed);
    if (!wideStr)
        return nullptr;

    return wchar2char(wideStr, toCodepage, -1, pLenOut);
}

// ============================================================================
// String Utility Functions
// ============================================================================

void stringSplit(const wstring& input, const wstring& delimiter, std::vector<wstring>& output)
{
    output.clear();
    size_t start = 0;
    size_t end = input.find(delimiter);
    const size_t delimiterLength = delimiter.length();

    while (end != wstring::npos)
    {
        output.push_back(input.substr(start, end - start));
        start = end + delimiterLength;
        end = input.find(delimiter, start);
    }
    output.push_back(input.substr(start));
}

void stringJoin(const std::vector<wstring>& strings, const wstring& separator, wstring& joinedString)
{
    joinedString.clear();
    size_t length = strings.size();
    for (size_t i = 0; i < length; ++i)
    {
        joinedString += strings.at(i);
        if (i != length - 1)
        {
            joinedString += separator;
        }
    }
}

std::wstring stringToUpper(const std::wstring& strToConvert)
{
    static const auto loc = std::locale("");
    std::wstring result = strToConvert;
    std::transform(result.begin(), result.end(), result.begin(),
        [](wchar_t ch) { return std::toupper(ch, loc); });
    return result;
}

std::wstring stringToLower(const std::wstring& strToConvert)
{
    static const auto loc = std::locale("");
    std::wstring result = strToConvert;
    std::transform(result.begin(), result.end(), result.begin(),
        [](wchar_t ch) { return std::tolower(ch, loc); });
    return result;
}

std::wstring stringReplace(const std::wstring& str, const std::wstring& from, const std::wstring& to)
{
    std::wstring result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::wstring::npos)
    {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

// ============================================================================
// String Conversion Functions
// ============================================================================

std::wstring string2wstring(const std::string& rString, UINT codepage)
{
    if (rString.empty())
        return L"";

    // Use WcharMbcsConvertor for consistent conversion behavior
    const wchar_t* result = WcharMbcsConvertor::getInstance().char2wchar(rString.c_str(), codepage, static_cast<int>(rString.length()), nullptr, nullptr);
    if (result)
        return std::wstring(result);
    return L"";
}

std::string wstring2string(const std::wstring& rwString, UINT codepage)
{
    if (rwString.empty())
        return "";

    // Use WcharMbcsConvertor for consistent conversion behavior
    const char* result = WcharMbcsConvertor::getInstance().wchar2char(rwString.c_str(), codepage, static_cast<int>(rwString.length()), nullptr);
    if (result)
        return std::string(result);
    return "";
}

// ============================================================================
// Path and File Utility Functions
// ============================================================================

std::wstring pathAppend(std::wstring& strDest, const std::wstring& str2append)
{
    if (strDest.empty() && str2append.empty()) // "" + ""
    {
        strDest = L"/";
        return strDest;
    }

    if (strDest.empty() && !str2append.empty()) // "" + titi
    {
        strDest = str2append;
        return strDest;
    }

    // Handle both forward slashes and backslashes for cross-platform compatibility
    if (strDest[strDest.length() - 1] == L'/' || strDest[strDest.length() - 1] == L'\\')
    {
        if (!str2append.empty() && (str2append[0] == L'/' || str2append[0] == L'\\'))
        {
            // toto/ + /titi -> remove trailing slash from dest
            strDest.erase(strDest.length() - 1, 1);
        }
    }
    else
    {
        if (str2append.empty())
        {
            // toto + "" -> just return dest
            return strDest;
        }
        else if (str2append[0] != L'/' && str2append[0] != L'\\')
        {
            // toto + titi -> add separator
            strDest += L'/';
        }
    }

    strDest += str2append;
    return strDest;
}

bool doesFileExist(const wchar_t* filePath, DWORD milliSec2wait, bool* isTimeoutReached)
{
    (void)milliSec2wait;  // Timeout not implemented for Linux
    (void)isTimeoutReached;

    if (!filePath || !filePath[0])
        return false;

    // Convert wchar_t path to char path for stat
    std::string narrowPath = WcharMbcsConvertor::getInstance().wchar2char(filePath, CP_UTF8);

    struct stat fileStat;
    if (stat(narrowPath.c_str(), &fileStat) != 0)
        return false;

    return S_ISREG(fileStat.st_mode);
}

bool doesDirectoryExist(const wchar_t* dirPath, DWORD milliSec2wait, bool* isTimeoutReached)
{
    (void)milliSec2wait;  // Timeout not implemented for Linux
    (void)isTimeoutReached;

    if (!dirPath || !dirPath[0])
        return false;

    // Convert wchar_t path to char path for stat
    std::string narrowPath = WcharMbcsConvertor::getInstance().wchar2char(dirPath, CP_UTF8);

    struct stat dirStat;
    if (stat(narrowPath.c_str(), &dirStat) != 0)
        return false;

    return S_ISDIR(dirStat.st_mode);
}

int nbDigitsFromNbLines(size_t nbLines)
{
    int nbDigits = 0;
    do
    {
        ++nbDigits;
        nbLines /= 10;
    } while (nbLines != 0);
    return nbDigits;
}

// ============================================================================
// NppDarkMode Stub Implementation for Linux
// ============================================================================
// These functions provide Linux-compatible implementations of NppDarkMode
// functionality that is Windows-specific in NppDarkMode.cpp.
// The getDarkModeDefaultColors function is needed by Parameters.h for the
// DarkModeConf structure initialization.

namespace NppDarkMode
{
    // Helper to convert RGB hex to COLORREF (little-endian 0xBBGGRR)
    static constexpr COLORREF HEXRGB(DWORD rrggbb)
    {
        return ((rrggbb & 0xFF0000) >> 16) |
               ((rrggbb & 0x00FF00)) |
               ((rrggbb & 0x0000FF) << 16);
    }

    // Black (default) dark mode colors
    static constexpr Colors darkColors{
        HEXRGB(0x202020),   // background
        HEXRGB(0x383838),   // softerBackground
        HEXRGB(0x454545),   // hotBackground
        HEXRGB(0x202020),   // pureBackground
        HEXRGB(0xB00000),   // errorBackground
        HEXRGB(0xE0E0E0),   // textColor
        HEXRGB(0xC0C0C0),   // darkerTextColor
        HEXRGB(0x808080),   // disabledTextColor
        HEXRGB(0xFFFF00),   // linkTextColor
        HEXRGB(0x646464),   // edgeColor
        HEXRGB(0x9B9B9B),   // hotEdgeColor
        HEXRGB(0x484848)    // disabledEdgeColor
    };

    static constexpr int offsetEdge = HEXRGB(0x1C1C1C);

    // Red tone
    static constexpr int offsetRed = HEXRGB(0x100000);
    static constexpr Colors darkRedColors{
        darkColors.background + offsetRed,
        darkColors.softerBackground + offsetRed,
        darkColors.hotBackground + offsetRed,
        darkColors.pureBackground + offsetRed,
        darkColors.errorBackground,
        darkColors.text,
        darkColors.darkerText,
        darkColors.disabledText,
        darkColors.linkText,
        darkColors.edge + offsetEdge + offsetRed,
        darkColors.hotEdge + offsetRed,
        darkColors.disabledEdge + offsetRed
    };

    // Green tone
    static constexpr int offsetGreen = HEXRGB(0x001000);
    static constexpr Colors darkGreenColors{
        darkColors.background + offsetGreen,
        darkColors.softerBackground + offsetGreen,
        darkColors.hotBackground + offsetGreen,
        darkColors.pureBackground + offsetGreen,
        darkColors.errorBackground,
        darkColors.text,
        darkColors.darkerText,
        darkColors.disabledText,
        darkColors.linkText,
        darkColors.edge + offsetEdge + offsetGreen,
        darkColors.hotEdge + offsetGreen,
        darkColors.disabledEdge + offsetGreen
    };

    // Blue tone
    static constexpr int offsetBlue = HEXRGB(0x000020);
    static constexpr Colors darkBlueColors{
        darkColors.background + offsetBlue,
        darkColors.softerBackground + offsetBlue,
        darkColors.hotBackground + offsetBlue,
        darkColors.pureBackground + offsetBlue,
        darkColors.errorBackground,
        darkColors.text,
        darkColors.darkerText,
        darkColors.disabledText,
        darkColors.linkText,
        darkColors.edge + offsetEdge + offsetBlue,
        darkColors.hotEdge + offsetBlue,
        darkColors.disabledEdge + offsetBlue
    };

    // Purple tone
    static constexpr int offsetPurple = HEXRGB(0x100020);
    static constexpr Colors darkPurpleColors{
        darkColors.background + offsetPurple,
        darkColors.softerBackground + offsetPurple,
        darkColors.hotBackground + offsetPurple,
        darkColors.pureBackground + offsetPurple,
        darkColors.errorBackground,
        darkColors.text,
        darkColors.darkerText,
        darkColors.disabledText,
        darkColors.linkText,
        darkColors.edge + offsetEdge + offsetPurple,
        darkColors.hotEdge + offsetPurple,
        darkColors.disabledEdge + offsetPurple
    };

    // Cyan tone
    static constexpr int offsetCyan = HEXRGB(0x001020);
    static constexpr Colors darkCyanColors{
        darkColors.background + offsetCyan,
        darkColors.softerBackground + offsetCyan,
        darkColors.hotBackground + offsetCyan,
        darkColors.pureBackground + offsetCyan,
        darkColors.errorBackground,
        darkColors.text,
        darkColors.darkerText,
        darkColors.disabledText,
        darkColors.linkText,
        darkColors.edge + offsetEdge + offsetCyan,
        darkColors.hotEdge + offsetCyan,
        darkColors.disabledEdge + offsetCyan
    };

    // Olive tone
    static constexpr int offsetOlive = HEXRGB(0x101000);
    static constexpr Colors darkOliveColors{
        darkColors.background + offsetOlive,
        darkColors.softerBackground + offsetOlive,
        darkColors.hotBackground + offsetOlive,
        darkColors.pureBackground + offsetOlive,
        darkColors.errorBackground,
        darkColors.text,
        darkColors.darkerText,
        darkColors.disabledText,
        darkColors.linkText,
        darkColors.edge + offsetEdge + offsetOlive,
        darkColors.hotEdge + offsetOlive,
        darkColors.disabledEdge + offsetOlive
    };

    Colors getDarkModeDefaultColors(ColorTone colorTone)
    {
        switch (colorTone)
        {
            case NppDarkMode::ColorTone::redTone:
                return darkRedColors;

            case NppDarkMode::ColorTone::greenTone:
                return darkGreenColors;

            case NppDarkMode::ColorTone::blueTone:
                return darkBlueColors;

            case NppDarkMode::ColorTone::purpleTone:
                return darkPurpleColors;

            case NppDarkMode::ColorTone::cyanTone:
                return darkCyanColors;

            case NppDarkMode::ColorTone::oliveTone:
                return darkOliveColors;

            case NppDarkMode::ColorTone::customizedTone:
            case NppDarkMode::ColorTone::blackTone:
            default:
                return darkColors;
        }
    }
}

// ============================================================================
// Accelerator Stub Implementation for Linux
// ============================================================================
// These functions provide Linux-compatible implementations of Accelerator
// functionality that is Windows-specific in shortcut.cpp.
// The updateFullMenu function is needed by LastRecentFileList.cpp.

void Accelerator::updateFullMenu()
{
    // Qt/Linux: Menu updates are handled by the Qt menu system
    // This is a stub for compatibility
}

#endif // !_WIN32
