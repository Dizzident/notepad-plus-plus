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
// PluginsManagerLinux.cpp - Linux implementation of plugin loading
// ============================================================================
// This file provides Linux-compatible plugin loading using dlopen/dlsym
// instead of Windows LoadLibrary/GetProcAddress.
//
// Plugin loading on Linux:
// - Plugins are .so files stored in ~/.local/share/notepad++/plugins/
// - Each plugin has its own subdirectory: <pluginName>/<pluginName>.so
// - The plugin must export the required functions:
//   - isUnicode(): Always returns TRUE on Linux (Unicode only)
//   - setInfo(NppData): Called to pass NppData to plugin
//   - getName(): Returns plugin name
//   - getFuncsArray(int*): Returns array of FuncItem commands
//   - beNotified(SCNotification*): Called for notifications
//   - messageProc(UINT, WPARAM, LPARAM): Called for messages
// ============================================================================

#include "PluginsManager.h"

#ifndef _WIN32

#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <filesystem>
#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <ILexer.h>
#include <Lexilla.h>
#include <Scintilla.h>

#include "Common.h"
#include "Notepad_plus_msgs.h"
#include "NppConstants.h"
#include "NppXml.h"
#include "Parameters.h"
#include "PluginInterface.h"
#include "menuCmdID.h"
// Note: pluginsAdmin.h is Windows-specific, using Qt version instead
// #include "pluginsAdmin.h"
#include "resource.h"
#include "../../QtControls/Shortcut/Shortcut.h"

// Forward declarations for plugin admin types (Qt version)
class PluginUpdateInfo;
class PluginViewList;

// Qt includes for message boxes
#include <QMessageBox>
#include <QString>

using namespace std;

const wchar_t * USERMSG = L" is not compatible with the current version of Notepad++.\n\n\
Do you want to remove this plugin from the plugins directory to prevent this message from the next launch?";

// Helper function to convert wstring to string for filesystem operations
static std::string wstringToUtf8(const std::wstring& wstr)
{
	if (wstr.empty()) return "";
	size_t len = wcstombs(nullptr, wstr.c_str(), 0);
	if (len == static_cast<size_t>(-1)) return "";
	std::string result(len, 0);
	wcstombs(&result[0], wstr.c_str(), len);
	return result;
}

// Helper function to convert string to wstring
static std::wstring utf8ToWstring(const std::string& str)
{
	if (str.empty()) return L"";
	size_t len = mbstowcs(nullptr, str.c_str(), 0);
	if (len == static_cast<size_t>(-1)) return L"";
	std::wstring result(len, 0);
	mbstowcs(&result[0], str.c_str(), len);
	return result;
}

// Linux-specific: Get file name from path (handles both / and \)
static std::wstring pathFindFileName(const std::wstring& path)
{
	size_t pos = path.find_last_of(L"/\\");
	if (pos != std::wstring::npos)
		return path.substr(pos + 1);
	return path;
}

// Linux-specific: Check if file exists
static bool fileExistsLinux(const std::string& path)
{
	struct stat st;
	return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

// Linux-specific: Check architecture compatibility
// On Linux, we check ELF header for architecture
static bool isArchitectureCompatible(const std::string& filePath)
{
	FILE* f = fopen(filePath.c_str(), "rb");
	if (!f)
		return false;

	unsigned char elfHeader[20];
	size_t read = fread(elfHeader, 1, sizeof(elfHeader), f);
	fclose(f);

	if (read < 20)
		return false;

	// Check ELF magic
	if (elfHeader[0] != 0x7F || elfHeader[1] != 'E' ||
	    elfHeader[2] != 'L' || elfHeader[3] != 'F')
		return false;

	// elfHeader[4] = 1 for 32-bit, 2 for 64-bit
	// elfHeader[18] and [19] = machine type

	// Check if architecture matches current process
#if defined(__x86_64__)
	// Require 64-bit ELF
	if (elfHeader[4] != 2)
		return false;
	// Machine type should be x86_64 (0x3E)
	if (elfHeader[18] != 0x3E && elfHeader[19] != 0x00)
		return false;
#elif defined(__i386__)
	// Require 32-bit ELF
	if (elfHeader[4] != 1)
		return false;
	// Machine type should be x86 (0x03)
	if (elfHeader[18] != 0x03 && elfHeader[19] != 0x00)
		return false;
#elif defined(__aarch64__)
	// Require 64-bit ELF
	if (elfHeader[4] != 2)
		return false;
	// Machine type should be ARM64 (0xB7)
	if (elfHeader[18] != 0xB7 && elfHeader[19] != 0x00)
		return false;
#endif

	return true;
}

// ============================================================================
// Linux Implementation of Plugin Loading
// ============================================================================

int PluginsManager::loadPluginFromPath(const wchar_t *pluginFilePath)
{
	const wchar_t *pluginFileName = pathFindFileName(pluginFilePath).c_str();
	if (isInLoadedDlls(pluginFileName))
		return 0;

	NppParameters& nppParams = NppParameters::getInstance();

	auto pi = std::make_unique<PluginInfo>();
	try
	{
		pi->_moduleName = pluginFileName;

		std::string utf8Path = wstringToUtf8(pluginFilePath);

		// Check architecture compatibility
		if (!isArchitectureCompatible(utf8Path))
		{
			const wchar_t* archErrMsg = L"Cannot load plugin - architecture mismatch.";
#if defined(__x86_64__)
			archErrMsg = L"Cannot load 32-bit or non-x86_64 plugin on x86_64 system.";
#elif defined(__aarch64__)
			archErrMsg = L"Cannot load non-ARM64 plugin on ARM64 system.";
#endif
			throw std::wstring(archErrMsg);
		}

		// Load the shared library
		pi->_hLib = dlopen(utf8Path.c_str(), RTLD_NOW | RTLD_LOCAL);
		if (!pi->_hLib)
		{
			const char* dlError = dlerror();
			std::wstring errMsg = L"Failed to load plugin library: ";
			if (dlError)
				errMsg += utf8ToWstring(dlError);
			else
				errMsg += L"Unknown error";
			throw errMsg;
		}

		// Get isUnicode function (optional on Linux, we always use Unicode)
		pi->_pFuncIsUnicode = reinterpret_cast<PFUNCISUNICODE>(dlsym(pi->_hLib, "isUnicode"));

		// Get setInfo function (required)
		pi->_pFuncSetInfo = reinterpret_cast<PFUNCSETINFO>(dlsym(pi->_hLib, "setInfo"));
		if (!pi->_pFuncSetInfo)
		{
			throw std::wstring(L"Missing \"setInfo\" function");
		}

		// Get getName function (required)
		pi->_pFuncGetName = reinterpret_cast<PFUNCGETNAME>(dlsym(pi->_hLib, "getName"));
		if (!pi->_pFuncGetName)
		{
			throw std::wstring(L"Missing \"getName\" function");
		}
		pi->_funcName = pi->_pFuncGetName();

		// Get beNotified function (required)
		pi->_pBeNotified = reinterpret_cast<PBENOTIFIED>(dlsym(pi->_hLib, "beNotified"));
		if (!pi->_pBeNotified)
		{
			throw std::wstring(L"Missing \"beNotified\" function");
		}

		// Get messageProc function (required)
		pi->_pMessageProc = reinterpret_cast<PMESSAGEPROC>(dlsym(pi->_hLib, "messageProc"));
		if (!pi->_pMessageProc)
		{
			throw std::wstring(L"Missing \"messageProc\" function");
		}

		// Call setInfo to pass NppData to plugin
		pi->_pFuncSetInfo(_nppData);

		// Get getFuncsArray function (required)
		pi->_pFuncGetFuncsArray = reinterpret_cast<PFUNCGETFUNCSARRAY>(dlsym(pi->_hLib, "getFuncsArray"));
		if (!pi->_pFuncGetFuncsArray)
		{
			throw std::wstring(L"Missing \"getFuncsArray\" function");
		}

		pi->_funcItems = pi->_pFuncGetFuncsArray(&pi->_nbFuncItem);

		if ((!pi->_funcItems) || (pi->_nbFuncItem <= 0))
		{
			throw std::wstring(L"Missing \"FuncItems\" array, or the nb of Function Item is not set correctly");
		}

		// Create plugin menu (placeholder for Linux - actual menu creation happens elsewhere)
		pi->_pluginMenu = nullptr; // Will be created in initMenu

		// Check for lexer plugin
		Lexilla::GetLexerCountFn GetLexerCount = reinterpret_cast<Lexilla::GetLexerCountFn>(dlsym(pi->_hLib, LEXILLA_GETLEXERCOUNT));
		if (GetLexerCount)
		{
			Lexilla::GetLexerNameFn GetLexerName = reinterpret_cast<Lexilla::GetLexerNameFn>(dlsym(pi->_hLib, LEXILLA_GETLEXERNAME));
			if (!GetLexerName)
			{
				throw std::wstring(L"Loading GetLexerName function failed.");
			}

			Lexilla::CreateLexerFn CreateLexer = reinterpret_cast<Lexilla::CreateLexerFn>(dlsym(pi->_hLib, LEXILLA_CREATELEXER));
			if (!CreateLexer)
			{
				throw std::wstring(L"Loading CreateLexer function failed.");
			}

			// Assign a buffer for the lexer name
			char lexName[MAX_EXTERNAL_LEXER_NAME_LEN]{};

			int numLexers = GetLexerCount();

			std::unique_ptr<ExternalLangContainer> containers[30]{};

			for (int x = 0; x < numLexers; ++x)
			{
				GetLexerName(x, lexName, MAX_EXTERNAL_LEXER_NAME_LEN);
				if (!nppParams.isExistingExternalLangName(lexName) && nppParams.ExternalLangHasRoom())
				{
					containers[x] = std::make_unique<ExternalLangContainer>();
					containers[x]->_name = lexName;
					containers[x]->fnCL = CreateLexer;
				}
				else
				{
					containers[x] = nullptr;
				}
			}

			namespace fs = ::std::filesystem;

			fs::path pluginPath = L"plugins";
			pluginPath /= L"Config";
			pluginPath /= pi->_moduleName;
			pluginPath.replace_extension(".xml");

			fs::path xmlPath = nppParams.getNppPath() / pluginPath;
			std::wstring xmlPathW = xmlPath.wstring();

			if (!doesFileExist(xmlPathW.c_str()))
			{
				xmlPath = nppParams.getAppDataNppDir() / pluginPath;
				xmlPathW = xmlPath.wstring();

				if (!doesFileExist(xmlPathW.c_str()))
				{
					throw std::wstring(xmlPathW + L" is missing.");
				}
			}

			TiXmlDocument *pXmlDoc = new TiXmlDocument();

			if (!pXmlDoc->LoadFile(xmlPathW))
			{
				delete pXmlDoc;
				pXmlDoc = nullptr;
				throw std::wstring(xmlPathW + L" failed to load.");
			}

			for (int x = 0; x < numLexers; ++x)
			{
				if (containers[x] != nullptr)
					nppParams.addExternalLangToEnd(std::move(containers[x]));
			}

			nppParams.getExternalLexerFromXmlTree(pXmlDoc);
			nppParams.getExternalLexerDoc()->push_back(pXmlDoc);
		}

		addInLoadedDlls(pluginFilePath, pluginFileName);
		_pluginInfos.push_back(std::move(pi));
		return static_cast<int>(_pluginInfos.size() - 1);
	}
	catch (std::exception& e)
	{
		pluginExceptionAlert(pluginFileName, e);
		return -1;
	}
	catch (wstring& s)
	{
		if (pi && pi->_hLib)
		{
			dlclose(pi->_hLib);
		}

		s += L"\n\n";
		s += pluginFileName;
		s += USERMSG;

		// Use Qt message box for Linux
		QString msg = QString::fromStdWString(s);
		QString title = QString::fromStdWString(pluginFilePath);
		QMessageBox::StandardButton reply = QMessageBox::question(
			nullptr, title, msg,
			QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::Yes)
		{
			std::string utf8Path = wstringToUtf8(pluginFilePath);
			unlink(utf8Path.c_str());
		}
		return -1;
	}
	catch (...)
	{
		if (pi && pi->_hLib)
		{
			dlclose(pi->_hLib);
		}

		wstring msg = L"Failed to load";
		msg += L"\n\n";
		msg += pluginFileName;
		msg += USERMSG;

		QString qmsg = QString::fromStdWString(msg);
		QString title = QString::fromStdWString(pluginFilePath);
		QMessageBox::StandardButton reply = QMessageBox::question(
			nullptr, title, qmsg,
			QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::Yes)
		{
			std::string utf8Path = wstringToUtf8(pluginFilePath);
			unlink(utf8Path.c_str());
		}
		return -1;
	}
}

bool PluginsManager::loadPlugins(const wchar_t* dir, const PluginViewList* pluginUpdateInfoList, PluginViewList* pluginIncompatibleList)
{
	if (_isDisabled)
		return false;

	vector<wstring> dllNames;

	NppParameters& nppParams = NppParameters::getInstance();
	wstring nppPath = nppParams.getNppPath();

	wstring pluginsFolder;
	if (dir && dir[0])
	{
		pluginsFolder = dir;
	}
	else
	{
		pluginsFolder = nppPath;
		pluginsFolder += L"/plugins";
	}

	// Get Notepad++ current version
	// On Linux, we use a different method to get version
	Version nppVer;
	nppVer.setVersionFrom(nppParams.getNppPath().c_str());

	// Open plugins directory
	std::string pluginsFolderUtf8 = wstringToUtf8(pluginsFolder);
	DIR* dirp = opendir(pluginsFolderUtf8.c_str());
	if (!dirp)
		return false;

	struct dirent* entry;
	while ((entry = readdir(dirp)) != nullptr)
	{
		// Skip . and .. and Config
		if (strcmp(entry->d_name, ".") == 0 ||
		    strcmp(entry->d_name, "..") == 0 ||
		    strcasecmp(entry->d_name, "Config") == 0)
		{
			continue;
		}

		// Build path to plugin folder
		std::string pluginDirPath = pluginsFolderUtf8 + "/" + entry->d_name;

		struct stat st;
		if (stat(pluginDirPath.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
			continue;

		// Look for .so file with same name as directory
		std::string soName = std::string(entry->d_name) + ".so";
		std::string soPath = pluginDirPath + "/" + soName;

		if (fileExistsLinux(soPath))
		{
			// Plugin admin compatibility check disabled for Linux
			// TODO: Implement using Qt version of PluginUpdateInfo
			(void)pluginUpdateInfoList;
			(void)pluginIncompatibleList;
			(void)nppVer;
			dllNames.push_back(utf8ToWstring(soPath));
		}
	}

	closedir(dirp);

	// Load each plugin
	for (size_t i = 0, len = dllNames.size(); i < len; ++i)
	{
		loadPluginFromPath(dllNames[i].c_str());
	}

	return true;
}

// return true if cmdID found and its shortcut is enable
// false otherwise
bool PluginsManager::getShortcutByCmdID(int cmdID, ShortcutKey* sk)
{
	if (cmdID == 0 || !sk)
		return false;

	const vector<PluginCmdShortcut> & pluginCmdSCList = (NppParameters::getInstance()).getPluginCommandList();

	for (size_t i = 0, len = pluginCmdSCList.size(); i < len ; ++i)
	{
		if (pluginCmdSCList[i].getID() == static_cast<unsigned long>(cmdID))
		{
			const KeyCombo & kc = pluginCmdSCList[i].getKeyCombo();
			if (kc._key == 0x00)
				return false;

			sk->_isAlt = kc._isAlt;
			sk->_isCtrl = kc._isCtrl;
			sk->_isShift = kc._isShift;
			sk->_key = kc._key;
			return true;
		}
	}
	return false;
}

// returns false if cmdID not provided, true otherwise
bool PluginsManager::removeShortcutByCmdID(int cmdID)
{
	if (cmdID == 0)
		return false;

	NppParameters& nppParam = NppParameters::getInstance();
	vector<PluginCmdShortcut> & pluginCmdSCList = nppParam.getPluginCommandList();

	for (size_t i = 0, len = pluginCmdSCList.size(); i < len; ++i)
	{
		if (pluginCmdSCList[i].getID() == static_cast<unsigned long>(cmdID))
		{
			//remove shortcut
			pluginCmdSCList[i].clear();

			// inform accelerator instance
			nppParam.getAccelerator()->updateShortcuts();

			// set dirty flag to force writing shortcuts.xml on shutdown
			nppParam.setShortcutDirty();
			break;
		}
	}
	return true;
}

void PluginsManager::addInMenuFromPMIndex(int i)
{
	// On Linux, menu items are added through Qt menu system
	// This is handled by the Qt integration layer
	// For now, we just register the commands

	vector<PluginCmdShortcut> & pluginCmdSCList = (NppParameters::getInstance()).getPluginCommandList();

	for (int j = 0; j < _pluginInfos[i]->_nbFuncItem; ++j)
	{
		if (_pluginInfos[i]->_funcItems[j]._pFunc == nullptr)
		{
			// Separator - handled by menu system
			continue;
		}

		_pluginsCommands.push_back(PluginCommand(
			_pluginInfos[i]->_moduleName.c_str(),
			j,
			_pluginInfos[i]->_funcItems[j]._pFunc));

		const int cmdID = ID_PLUGINS_CMD + static_cast<int>(_pluginsCommands.size() - 1);
		_pluginInfos[i]->_funcItems[j]._cmdID = cmdID;

		string itemName = wstring2string(_pluginInfos[i]->_funcItems[j]._itemName, CP_UTF8);

		if (_pluginInfos[i]->_funcItems[j]._pShKey)
		{
			ShortcutKey & sKey = *(_pluginInfos[i]->_funcItems[j]._pShKey);
			PluginCmdShortcut pcs(
				Shortcut(itemName.c_str(), sKey._isCtrl, sKey._isAlt, sKey._isShift, sKey._key),
				cmdID,
				wstring2string(_pluginInfos[i]->_moduleName, CP_UTF8).c_str(),
				j);
			pluginCmdSCList.push_back(pcs);
		}
		else
		{
			// No shortcut provided, add disabled shortcut
			Shortcut sc(itemName.c_str(), false, false, false, 0x00);
			PluginCmdShortcut pcs(sc, cmdID, wstring2string(_pluginInfos[i]->_moduleName, CP_UTF8).c_str(), j);
			pluginCmdSCList.push_back(pcs);
		}
	}
}

HMENU PluginsManager::initMenu(HMENU hMenu, bool enablePluginAdmin)
{
	size_t nbPlugin = _pluginInfos.size();

	// On Linux, we don't use Windows HMENU
	// The menu handle is stored for compatibility but actual menu
	// management is done through Qt
	_hPluginsMenu = hMenu;

	// Add plugins to menu
	for (size_t i = 0; i < nbPlugin; ++i)
	{
		addInMenuFromPMIndex(static_cast<int>(i));
	}

	return _hPluginsMenu;
}

void PluginsManager::runPluginCommand(size_t i)
{
	if (i < _pluginsCommands.size())
	{
		if (_pluginsCommands[i]._pFunc != nullptr)
		{
			try
			{
				_pluginsCommands[i]._pFunc();
			}
			catch (std::exception& e)
			{
				QString msg = QString("Exception: %1").arg(e.what());
				QMessageBox::critical(nullptr, "Plugin Exception", msg);
			}
			catch (...)
			{
				static constexpr size_t bufSize = 128;
				wchar_t funcInfo[bufSize] = { '\0' };
				swprintf(funcInfo, bufSize, L"runPluginCommand(size_t i : %zu)", i);
				pluginCrashAlert(_pluginsCommands[i]._pluginName.c_str(), funcInfo);
			}
		}
	}
}

void PluginsManager::runPluginCommand(const wchar_t *pluginName, int commandID)
{
	for (size_t i = 0, len = _pluginsCommands.size(); i < len; ++i)
	{
		if (::_wcsicmp(_pluginsCommands[i]._pluginName.c_str(), pluginName) == 0)
		{
			if (_pluginsCommands[i]._funcID == commandID)
			{
				try
				{
					_pluginsCommands[i]._pFunc();
				}
				catch (std::exception& e)
				{
					pluginExceptionAlert(_pluginsCommands[i]._pluginName.c_str(), e);
				}
				catch (...)
				{
					static constexpr size_t bufSize = 128;
					wchar_t funcInfo[bufSize] = { '\0' };
					swprintf(funcInfo, bufSize, L"runPluginCommand(const wchar_t *pluginName : %s, int commandID : %d)", pluginName, commandID);
					pluginCrashAlert(_pluginsCommands[i]._pluginName.c_str(), funcInfo);
				}
			}
		}
	}
}

// send the notification to a specific plugin
void PluginsManager::notify(size_t indexPluginInfo, const SCNotification *notification)
{
	if (indexPluginInfo >= _pluginInfos.size())
		return;

	if (_pluginInfos[indexPluginInfo]->_hLib)
	{
		// To avoid the plugin change the data in SCNotification
		// Each notification to pass to a plugin is a copy of SCNotification instance
		SCNotification scNotif = *notification;
		try
		{
			_pluginInfos[indexPluginInfo]->_pBeNotified(&scNotif);
		}
		catch (std::exception& e)
		{
			pluginExceptionAlert(_pluginInfos[indexPluginInfo]->_moduleName.c_str(), e);
		}
		catch (...)
		{
			static constexpr size_t bufSize = 256;
			wchar_t funcInfo[bufSize] = { '\0' };
			swprintf(funcInfo, bufSize, L"notify(SCNotification *notification) : \r notification->nmhdr.code == %d\r notification->nmhdr.hwndFrom == %p\r notification->nmhdr.idFrom == %" PRIuPTR,
				scNotif.nmhdr.code, scNotif.nmhdr.hwndFrom, scNotif.nmhdr.idFrom);
			pluginCrashAlert(_pluginInfos[indexPluginInfo]->_moduleName.c_str(), funcInfo);
		}
	}
}

// broadcast the notification to all plugins
void PluginsManager::notify(const SCNotification *notification)
{
	if (_noMoreNotification) // this boolean should be enabled after NPPN_SHUTDOWN has been sent
		return;
	_noMoreNotification = notification->nmhdr.code == NPPN_SHUTDOWN;

	for (size_t i = 0, len = _pluginInfos.size(); i < len; ++i)
	{
		notify(i, notification);
	}
}

void PluginsManager::relayNppMessages(UINT Message, WPARAM wParam, LPARAM lParam)
{
	for (size_t i = 0, len = _pluginInfos.size(); i < len; ++i)
	{
		if (_pluginInfos[i]->_hLib)
		{
			try
			{
				_pluginInfos[i]->_pMessageProc(Message, wParam, lParam);
			}
			catch (std::exception& e)
			{
				pluginExceptionAlert(_pluginInfos[i]->_moduleName.c_str(), e);
			}
			catch (...)
			{
				static constexpr size_t bufSize = 128;
				wchar_t funcInfo[bufSize] = { '\0' };
				swprintf(funcInfo, bufSize, L"relayNppMessages(UINT Message : %u, WPARAM wParam : %" PRIuPTR ", LPARAM lParam : %" PRIiPTR ")", Message, wParam, lParam);
				pluginCrashAlert(_pluginInfos[i]->_moduleName.c_str(), funcInfo);
			}
		}
	}
}

bool PluginsManager::relayPluginMessages(UINT Message, WPARAM wParam, LPARAM lParam)
{
	const auto* moduleName = reinterpret_cast<wchar_t*>(wParam);
	if (!moduleName || !moduleName[0] || !lParam)
		return false;

	for (size_t i = 0, len = _pluginInfos.size(); i < len; ++i)
	{
		if (_pluginInfos[i]->_moduleName == moduleName)
		{
			if (_pluginInfos[i]->_hLib)
			{
				try
				{
					_pluginInfos[i]->_pMessageProc(Message, wParam, lParam);
				}
				catch (std::exception& e)
				{
					pluginExceptionAlert(_pluginInfos[i]->_moduleName.c_str(), e);
				}
				catch (...)
				{
					static constexpr size_t bufSize = 128;
					wchar_t funcInfo[bufSize] = { '\0' };
					swprintf(funcInfo, bufSize, L"relayPluginMessages(UINT Message : %u, WPARAM wParam : %" PRIuPTR ", LPARAM lParam : %" PRIiPTR ")", Message, wParam, lParam);
					pluginCrashAlert(_pluginInfos[i]->_moduleName.c_str(), funcInfo);
				}
				return true;
			}
		}
	}
	return false;
}

bool PluginsManager::allocateCmdID(int numberRequired, int *start)
{
	bool retVal = true;

	*start = _dynamicIDAlloc.allocate(numberRequired);

	if (*start == -1)
	{
		*start = 0;
		retVal = false;
	}
	return retVal;
}

bool PluginsManager::allocateMarker(int numberRequired, int* start)
{
	bool retVal = true;
	*start = _markerAlloc.allocate(numberRequired);
	if (*start == -1)
	{
		*start = 0;
		retVal = false;
	}
	return retVal;
}

bool PluginsManager::allocateIndicator(int numberRequired, int* start)
{
	bool retVal = false;
	int possibleStart = _indicatorAlloc.allocate(numberRequired);
	if (possibleStart != -1)
	{
		*start = possibleStart;
		retVal = true;
	}
	return retVal;
}

wstring PluginsManager::getLoadedPluginNames() const
{
	wstring pluginPaths;
	for (const auto &dll : _loadedDlls)
	{
		pluginPaths += L"\r\n    ";
		pluginPaths += dll._displayName;
	}
	return pluginPaths;
}

#endif // !_WIN32
