// This file is part of Notepad++ project
// Copyright (C)2025 Notepad++ contributors

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
// PluginData.cpp - Linux implementation of plugin data structures
// ============================================================================
// This file provides Linux-compatible implementations of PluginUpdateInfo
// and related data structures that are defined in the Windows pluginsAdmin.cpp.
//
// These implementations are used by the Qt-based PluginsAdmin dialog on Linux.
// ============================================================================

#ifndef _WIN32

#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/stat.h>

#include "PluginViewList.h"
#include "../../MISC/Common/Common.h"

// ============================================================================
// Helper Functions
// ============================================================================

static std::string wstringToUtf8(const std::wstring& wstr)
{
	if (wstr.empty()) return "";
	size_t len = wcstombs(nullptr, wstr.c_str(), 0);
	if (len == static_cast<size_t>(-1)) return "";
	std::string result(len, 0);
	wcstombs(&result[0], wstr.c_str(), len);
	return result;
}

static std::wstring utf8ToWstring(const std::string& str)
{
	if (str.empty()) return L"";
	size_t len = mbstowcs(nullptr, str.c_str(), 0);
	if (len == static_cast<size_t>(-1)) return L"";
	std::wstring result(len, 0);
	mbstowcs(&result[0], str.c_str(), len);
	return result;
}

// Helper to read file content
static std::string getFileContent(const char* filePath)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file)
		return "";

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

// ============================================================================
// PluginUpdateInfo Implementation
// ============================================================================

std::wstring PluginUpdateInfo::describe()
{
	std::wstring desc;
	const wchar_t *EOL = L"\r\n";
	if (!_description.empty())
	{
		desc = _description;
		desc += EOL;
	}

	if (!_author.empty())
	{
		desc += L"Author: ";
		desc += _author;
		desc += EOL;
	}

	if (!_homepage.empty())
	{
		desc += L"Homepage: ";
		desc += _homepage;
		desc += EOL;
	}

	return desc;
}

PluginUpdateInfo::PluginUpdateInfo(const std::wstring& fullFilePath, const std::wstring& filename)
{
	std::string utf8Path = wstringToUtf8(fullFilePath);

	struct stat st;
	if (stat(utf8Path.c_str(), &st) != 0)
		return;

	if (!S_ISREG(st.st_mode))
		return;

	_fullFilePath = fullFilePath;
	_displayName = filename;

	// Try to extract version from filename
	// Format: pluginName-X.Y.Z.so or pluginName.so
	size_t lastDash = filename.find_last_of(L'-');
	size_t lastDot = filename.find_last_of(L'.');

	if (lastDash != std::wstring::npos && lastDot != std::wstring::npos && lastDot > lastDash)
	{
		std::wstring versionStr = filename.substr(lastDash + 1, lastDot - lastDash - 1);
		_version = Version(versionStr);
	}
}

// ============================================================================
// NppCurrentStatus Implementation
// ============================================================================
// Note: Most of these are no-ops or simplified for Linux since we don't have
// the same admin mode / Program Files concepts as Windows

// The implementation is inline in the header for now

#endif // !_WIN32
