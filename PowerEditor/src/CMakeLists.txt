# @file:<dictaolib-root>/CMakeLists.txt

# TODO
# - Move defs XX_INCLUDE_DIR, XX_LIBRARIES, XX_LIBRARY_DIRS to the subdirectories
#   scripts. I could not find a way to export variables defined in a subscript
#   so as a workaround, I define the variables myself in the top script. This is
#   not optimal.
#

cmake_minimum_required(VERSION 3.10)

PROJECT(Notepad++)

SET(projIncludDir	../../scintilla/include/
					../../lexilla/include/
					./
					./WinControls/AboutDlg/
					./WinControls/
					./WinControls/ImageListSet/
					./WinControls/OpenSaveFileDialog/
					./WinControls/SplitterContainer/
					./WinControls/StaticDialog/
					./WinControls/TabBar/
					./WinControls/ToolBar/
					./MISC/Process/
					./ScintillaComponent/
					./MISC/
					./MISC/SysMsg/
					./WinControls/StatusBar/
					./WinControls/StaticDialog/RunDlg/
					./pugixml/
					./TinyXml/
					./WinControls/ColourPicker/
					./Win32Explr/
					./MISC/RegExt/
					./WinControls/TrayIcon/
					./WinControls/shortcut/
					./WinControls/Grid/
					./WinControls/ContextMenu/
					./MISC/PluginsManager/
					./WinControls/Preference/
					./WinControls/WindowsDlg/
					./WinControls/TaskList/
					./WinControls/DockingWnd/
					./WinControls/TreeView/
					./WinControls/ToolTip/
					./MISC/Exception/
					./MISC/Common/
					./WinControls/AnsiCharPanel/
					./WinControls/ClipboardHistory/
					./WinControls/FindCharsInRange/
					./WinControls/VerticalFileSwitcher/
					./WinControls/ProjectPanel/
					./WinControls/DocumentMap/
					./WinControls/FunctionList/
					./uchardet/
					./WinControls/FileBrowser/
					./WinControls/ReadDirectoryChanges/
					./MISC/md5/
					./MISC/sha1/
					./MISC/sha2/
					./MISC/sha512/
					./WinControls/PluginsAdmin/
					./json/)



SET(src_files
			./DarkMode/DarkMode.cpp
			./MISC/Common/verifySignedfile.cpp
			./MISC/md5/md5Dlgs.cpp
			./MISC/sha1/calc_sha1.cpp
			./MISC/sha1/sha1.cpp
			./MISC/sha2/sha-256.cpp
			./MISC/sha512/sha512.cpp
			./NppDarkMode.cpp
			./dpiManagerV2.cpp
			./MISC/Process/Processus.cpp
			./WinControls/AboutDlg/AboutDlg.cpp
			./WinControls/AnsiCharPanel/ansiCharPanel.cpp
			./ScintillaComponent/AutoCompletion.cpp
			./WinControls/AnsiCharPanel/asciiListView.cpp
			./WinControls/FileBrowser/fileBrowser.cpp
			./WinControls/Grid/BabyGrid.cpp
			./WinControls/Grid/BabyGridWrapper.cpp
			./ScintillaComponent/Buffer.cpp
			./uchardet/CharDistribution.cpp
			./WinControls/ClipboardHistory/clipboardHistoryPanel.cpp
			./WinControls/ColourPicker/ColourPicker.cpp
			./WinControls/ColourPicker/ColourPopup.cpp
			./ScintillaComponent/columnEditor.cpp
			./MISC/Common/Common.cpp
			./MISC/Common/FileInterface.cpp
			./MISC/Common/SortLocale.cpp
			./WinControls/ContextMenu/ContextMenu.cpp
			./WinControls/OpenSaveFileDialog/CustomFileDialog.cpp
			./WinControls/PluginsAdmin/pluginsAdmin.cpp
			./WinControls/ReadDirectoryChanges/ReadDirectoryChanges.cpp
			./WinControls/ReadDirectoryChanges/ReadDirectoryChangesPrivate.cpp
			./WinControls/ReadDirectoryChanges/ReadFileChanges.cpp
			./WinControls/TabBar/ControlsTab.cpp
			./WinControls/DockingWnd/DockingCont.cpp
			./WinControls/DockingWnd/DockingManager.cpp
			./WinControls/DockingWnd/DockingSplitter.cpp
			./ScintillaComponent/DocTabView.cpp
			./WinControls/DocumentMap/documentMap.cpp
			./WinControls/DocumentMap/documentSnapshot.cpp
			./WinControls/DoubleBuffer/DoubleBuffer.cpp
			./EncodingMapper.cpp
			./WinControls/FindCharsInRange/FindCharsInRange.cpp
			./ScintillaComponent/FindReplaceDlg.cpp
			./ScintillaComponent/FunctionCallTip.cpp
			./WinControls/FunctionList/functionListPanel.cpp
			./WinControls/FunctionList/functionParser.cpp
			./ScintillaComponent/GoToLineDlg.cpp
			./WinControls/DockingWnd/Gripper.cpp
			./WinControls/ImageListSet/ImageListSet.cpp
			./uchardet/JpCntx.cpp
			./uchardet/LangBulgarianModel.cpp
			./uchardet/LangCyrillicModel.cpp
			./uchardet/LangGreekModel.cpp
			./uchardet/LangHebrewModel.cpp
			./uchardet/LangHungarianModel.cpp
			./uchardet/LangThaiModel.cpp
			./lastRecentFileList.cpp
			./lesDlgs.cpp
			./WinControls/AnsiCharPanel/ListView.cpp
			./localization.cpp
			./MISC/Exception/MiniDumper.cpp
			./Notepad_plus.cpp
			./Notepad_plus_Window.cpp
			./NppBigSwitch.cpp
			./NppCommands.cpp
			./NppIO.cpp
			./NppNotification.cpp
			./uchardet/nsBig5Prober.cpp
			./uchardet/nsCharSetProber.cpp
			./uchardet/nsEscCharsetProber.cpp
			./uchardet/nsEscSM.cpp
			./uchardet/nsEUCJPProber.cpp
			./uchardet/nsEUCKRProber.cpp
			./uchardet/nsEUCTWProber.cpp
			./uchardet/nsGB2312Prober.cpp
			./uchardet/nsHebrewProber.cpp
			./uchardet/nsLatin1Prober.cpp
			./uchardet/nsMBCSGroupProber.cpp
			./uchardet/nsMBCSSM.cpp
			./uchardet/nsSBCharSetProber.cpp
			./uchardet/nsSBCSGroupProber.cpp
			./uchardet/nsSJISProber.cpp
			./uchardet/nsUniversalDetector.cpp
			./uchardet/nsUTF8Prober.cpp
			./Parameters.cpp
			./MISC/PluginsManager/PluginsManager.cpp
			./WinControls/Preference/preferenceDlg.cpp
			./ScintillaComponent/Printer.cpp
			./WinControls/ProjectPanel/ProjectPanel.cpp
			./MISC/RegExt/regExtDlg.cpp
			./WinControls/StaticDialog/RunDlg/RunDlg.cpp
			./WinControls/shortcut/RunMacroDlg.cpp
			./ScintillaComponent/ScintillaCtrls.cpp
			./ScintillaComponent/ScintillaEditView.cpp
			./WinControls/shortcut/shortcut.cpp
			./WinControls/Grid/ShortcutMapper.cpp
			./WinControls/TreeView/TreeView.cpp
			./WinControls/WindowsDlg/SizeableDlg.cpp
			./ScintillaComponent/SmartHighlighter.cpp
			./WinControls/SplitterContainer/Splitter.cpp
			./WinControls/SplitterContainer/SplitterContainer.cpp
			./WinControls/StaticDialog/StaticDialog.cpp
			./WinControls/StatusBar/StatusBar.cpp
			./WinControls/TabBar/TabBar.cpp
			./WinControls/TaskList/TaskList.cpp
			./WinControls/TaskList/TaskListDlg.cpp
			./pugixml/pugixml.cpp
			./TinyXml/tinystr.cpp
			./TinyXml/tinyxml.cpp
			./TinyXml/tinyxmlerror.cpp
			./TinyXml/tinyxmlparser.cpp
			./WinControls/ToolBar/ToolBar.cpp
			./WinControls/ToolTip/ToolTip.cpp
			./WinControls/TrayIcon/trayIconControler.cpp
			./uchardet/uchardet.cpp
			./WinControls/AboutDlg/URLCtrl.cpp
			./ScintillaComponent/UserDefineDialog.cpp
			./Utf8_16.cpp
			./WinControls/VerticalFileSwitcher/VerticalFileSwitcher.cpp
			./WinControls/VerticalFileSwitcher/VerticalFileSwitcherListView.cpp
			./MISC/Exception/Win32Exception.cpp
			./WinControls/WindowsDlg/WindowsDlg.cpp
			./winmain.cpp
			./WinControls/WindowsDlg/WinMgr.cpp
			./WinControls/WindowsDlg/WinRect.cpp
			./WinControls/ColourPicker/WordStyleDlg.cpp
			./ScintillaComponent/xmlMatchedTagsHighlighter.cpp
			./dpiAware.manifest
			./notepad++.exe.manifest
)

SET(include_files
			./MISC/Common/NppConstants.h
			./clipboardFormats.h
			./DarkMode/DarkMode.h
			./DarkMode/IatHook.h
			./DarkMode/UAHMenuBar.h
			./dpiManagerV2.h
			./keys.h
			./localizationString.h
			./MISC/Common/Sorters.h
			./MISC/Common/verifySignedfile.h
			./MISC/md5/md5.h
			./MISC/md5/md5Dlgs.h
			./MISC/md5/md5Dlgs_rc.h
			./MISC/sha1/calc_sha1.h
			./MISC/sha1/sha1.h
			./MISC/sha2/sha-256.h
			./MISC/sha512/sha512.h
			./NppDarkMode.h
			./NppXml.h
			./ScintillaComponent/columnEditor_rc.h
			./ScintillaComponent/FindReplaceDlg_rc.h
			./Utf8.h
			./WinControls/AboutDlg/AboutDlg.h
			./WinControls/AnsiCharPanel/ansiCharPanel.h
			./ScintillaComponent/AutoCompletion.h
			./WinControls/AnsiCharPanel/ansiCharPanel_rc.h
			./WinControls/AnsiCharPanel/asciiListView.h
			./WinControls/ColourPicker/ColourPopupResource.h
			./WinControls/ColourPicker/WordStyleDlgRes.h
			./WinControls/DocumentMap/documentMap_rc.h
			./WinControls/DocumentMap/documentSnapshot.h
			./WinControls/DocumentMap/documentSnapshot_rc.h
			./WinControls/FindCharsInRange/findCharsInRange_rc.h
			./WinControls/Grid/BabyGrid.h
			./WinControls/Grid/BabyGridWrapper.h
			./ScintillaComponent/Buffer.h
			./uchardet/CharDistribution.h
			./WinControls/ClipboardHistory/clipboardHistoryPanel.h
			./WinControls/ClipboardHistory/clipboardHistoryPanel_rc.h
			./ScintillaComponent/colors.h
			./WinControls/ColourPicker/ColourPicker.h
			./WinControls/ColourPicker/ColourPopup.h
			./ScintillaComponent/columnEditor.h
			./MISC/Common/Common.h
			./MISC/Common/FileInterface.h
			./WinControls/ContextMenu/ContextMenu.h
			./WinControls/OpenSaveFileDialog/CustomFileDialog.h
			./WinControls/PluginsAdmin/pluginsAdmin.h
			./WinControls/PluginsAdmin/pluginsAdminRes.h
			./WinControls/Preference/preference_rc.h
			./WinControls/ReadDirectoryChanges/targetver.h
			./WinControls/shortcut/shortcutRc.h
			./WinControls/StaticDialog/RunDlg/RunDlg_rc.h
			./WinControls/TabBar/ControlsTab.h
			./WinControls/DockingWnd/Docking.h
			./WinControls/DockingWnd/DockingCont.h
			./WinControls/DockingWnd/DockingDlgInterface.h
			./WinControls/DockingWnd/DockingManager.h
			./WinControls/DockingWnd/dockingResource.h
			./WinControls/DockingWnd/DockingSplitter.h
			./ScintillaComponent/DocTabView.h
			./WinControls/DocumentMap/documentMap.h
			./WinControls/DocumentMap/documentSnapshot.h
			./WinControls/DoubleBuffer/DoubleBuffer.h
			./EncodingMapper.h
			./MISC/FileNameStringSplitter.h
			./WinControls/FindCharsInRange/FindCharsInRange.h
			./ScintillaComponent/FindReplaceDlg.h
			./ScintillaComponent/FunctionCallTip.h
			./WinControls/FunctionList/functionListPanel.h
			./WinControls/FunctionList/functionListPanel_rc.h
			./WinControls/FunctionList/functionParser.h
			./ScintillaComponent/GoToLineDlg.h
			./WinControls/DockingWnd/Gripper.h
			./MISC/PluginsManager/IDAllocator.h
			./WinControls/ImageListSet/ImageListSet.h
			./uchardet/JpCntx.h
			./lastRecentFileList.h
			./lesDlgs.h
			./WinControls/AnsiCharPanel/ListView.h
			./localization.h
			./menuCmdID.h
			./MISC/Exception/MiniDumper.h
			./Notepad_plus.h
			./MISC/PluginsManager/Notepad_plus_msgs.h
			./Notepad_plus_Window.h
			./uchardet/nsBig5Prober.h
			./uchardet/nsCharSetProber.h
			./uchardet/nsCodingStateMachine.h
			./uchardet/nscore.h
			./uchardet/nsEscCharsetProber.h
			./uchardet/nsEUCJPProber.h
			./uchardet/nsEUCKRProber.h
			./uchardet/nsEUCTWProber.h
			./uchardet/nsGB2312Prober.h
			./uchardet/nsHebrewProber.h
			./uchardet/nsLatin1Prober.h
			./uchardet/nsMBCSGroupProber.h
			./uchardet/nsPkgInt.h
			./uchardet/nsSBCharSetProber.h
			./uchardet/nsSBCSGroupProber.h
			./uchardet/nsSJISProber.h
			./uchardet/nsUniversalDetector.h
			./uchardet/nsUTF8Prober.h
			./Parameters.h
			./MISC/PluginsManager/PluginInterface.h
			./MISC/PluginsManager/PluginsManager.h
			./WinControls/Preference/preferenceDlg.h
			./ScintillaComponent/Printer.h
			./uchardet/prmem.h
			./WinControls/ProjectPanel/ProjectPanel.h
			./WinControls/ProjectPanel/ProjectPanel_rc.h
			./MISC/RegExt/regExtDlg.h
			./MISC/RegExt/regExtDlgRc.h
			./resource.h
			./WinControls/StaticDialog/RunDlg/RunDlg.h
			./WinControls/shortcut/RunMacroDlg.h
			./WinControls/shortcut/RunMacroDlg_rc.h
			./ScintillaComponent/ScintillaCtrls.h
			./ScintillaComponent/ScintillaEditView.h
			./WinControls/shortcut/shortcut.h
			./WinControls/Grid/ShortcutMapper.h
			./WinControls/Grid/ShortcutMapper_rc.h
			./WinControls/TreeView/TreeView.h
			./WinControls/WindowsDlg/SizeableDlg.h
			./ScintillaComponent/SmartHighlighter.h
			./WinControls/SplitterContainer/Splitter.h
			./WinControls/SplitterContainer/SplitterContainer.h
			./WinControls/StaticDialog/StaticDialog.h
			./WinControls/StatusBar/StatusBar.h
			./WinControls/TabBar/TabBar.h
			./WinControls/TaskList/TaskList.h
			./WinControls/TaskList/TaskListDlg.h
			./WinControls/TaskList/TaskListDlg_rc.h
			./pugixml/pugiconfig.hpp
			./pugixml/pugixml.hpp
			./TinyXml/tinystr.h
			./TinyXml/tinyxml.h
			./WinControls/ToolBar/ToolBar.h
			./WinControls/ToolTip/ToolTip.h
			./WinControls/TrayIcon/trayIconControler.h
			./uchardet/uchardet.h
			./WinControls/AboutDlg/URLCtrl.h
			./ScintillaComponent/UserDefineDialog.h
			./ScintillaComponent/UserDefineResource.h
			./Utf8_16.h
			./WinControls/VerticalFileSwitcher/VerticalFileSwitcher.h
			./WinControls/VerticalFileSwitcher/VerticalFileSwitcher_rc.h
			./WinControls/VerticalFileSwitcher/VerticalFileSwitcherListView.h
			./MISC/Exception/Win32Exception.h
			./WinControls/Window.h
			./WinControls/WindowsDlg/WindowsDlg.h
			./WinControls/WindowsDlg/WindowsDlgRc.h
			./WinControls/WindowsDlg/WinMgr.h
			./WinControls/ColourPicker/WordStyleDlg.h
			./ScintillaComponent/xmlMatchedTagsHighlighter.h
			./rgba_icons.h
)


SET(rcFiles
			./MISC/md5/md5Dlgs.rc
			./WinControls/AnsiCharPanel/ansiCharPanel.rc
			./WinControls/ClipboardHistory/clipboardHistoryPanel.rc
			./WinControls/ColourPicker/ColourPopup.rc
			./ScintillaComponent/columnEditor.rc
			./WinControls/DockingWnd/DockingGUIWidget.rc
			./WinControls/DocumentMap/documentMap.rc
			./WinControls/DocumentMap/documentSnapshot.rc
			./WinControls/FileBrowser/fileBrowser.rc
			./WinControls/FindCharsInRange/findCharsInRange.rc
			./ScintillaComponent/FindReplaceDlg.rc
			./WinControls/FunctionList/functionListPanel.rc
			./Notepad_plus.rc
			./WinControls/PluginsAdmin/pluginsAdmin.rc
			./WinControls/Preference/preference.rc
			./WinControls/ProjectPanel/ProjectPanel.rc
			./MISC/RegExt/regExtDlg.rc
			./WinControls/StaticDialog/RunDlg/RunDlg.rc
			./WinControls/shortcut/RunMacroDlg.rc
			./WinControls/shortcut/shortcut.rc
			./WinControls/Grid/ShortcutMapper.rc
			./WinControls/TaskList/TaskListDlg.rc
			./ScintillaComponent/UserDefineDialog.rc
			./WinControls/VerticalFileSwitcher/VerticalFileSwitcher.rc
			./WinControls/WindowsDlg/WindowsDlg.rc
			./WinControls/ColourPicker/WordStyleDlg.rc
)

# ============================================================================
# Platform Detection and Configuration
# ============================================================================
IF (WIN32)
	SET(option WIN32)
	SET(win32_LIBRARIES comctl32 shlwapi dbghelp version crypt32 wintrust sensapi wininet imm32 msimg32 uxtheme dwmapi)
	set(CMAKE_CXX_STANDARD 20)
	set(NPP_PLATFORM_WINDOWS TRUE)

	# Add Platform abstraction for Windows
	list(APPEND src_files
		./Platform/Windows/FileSystem.cpp
		./Platform/Windows/Settings.cpp
		./Platform/Windows/Process.cpp
		./Platform/Windows/FileWatcher.cpp
		./Platform/Windows/Clipboard.cpp
		./Platform/Windows/Threading.cpp
		./Platform/Windows/Dialogs.cpp
	)

	list(APPEND projIncludDir ./Platform)

if ( MSVC )
	#do not use for mingw builds
	SET(CMAKE_CXX_FLAGS "/EHa /MP /W4")
	SET(defs -DUNICODE -D_UNICODE -D_WIN32_WINNT=_WIN32_WINNT_WIN7 -DNTDDI_VERSION=NTDDI_WIN7 -D_USE_64BIT_TIME_T -DTIXML_USE_STL -DNOMINMAX -DOEMRESOURCE -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING )
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else ( MSVC )
	# For possible MinGW compilation
	SET(CMAKE_CXX_FLAGS "-include../gcc/gcc-fixes.h -std=c++20 -fpermissive -municode")
	SET(defs -DUNICODE -D_UNICODE -D_WIN32_WINNT=_WIN32_WINNT_WIN7 -DNTDDI_VERSION=NTDDI_WIN7 -D_USE_64BIT_TIME_T -DTIXML_USE_STL -DNOMINMAX -DOEMRESOURCE)
endif ( MSVC )

	# Windows pre-build event
	execute_process(
		COMMAND cmd /C "cd ${CMAKE_SOURCE_DIR} && NppLibsVersionH-generator.bat"
		RESULT_VARIABLE pre-build-result
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
	if(NOT pre-build-result EQUAL 0)
		message(WARNING "Pre-build event failed with exit code: ${pre-build-result}")
	endif()

	find_library(SCINTILLA_STATIC_LIBRARY
		NAMES libscintilla
		HINTS "../../scintilla/bin"
		REQUIRED
	)

	find_library(LEXILLA_STATIC_LIBRARY
		NAMES liblexilla
		HINTS "../../lexilla/bin"
		REQUIRED
	)

ELSEIF(UNIX AND NOT APPLE)
	# ============================================================================
	# Linux Build Configuration
	# ============================================================================
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(NPP_PLATFORM_LINUX TRUE)

	message(STATUS "Configuring Notepad++ for Linux")

	# Find Qt6
	find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network PrintSupport)
	if(Qt6_FOUND)
		message(STATUS "Qt6 found: ${Qt6_VERSION}")
	endif()

	# Enable Qt MOC/UIC/RCC
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)

	# Linux-specific definitions
	SET(defs -DNPP_LINUX -D_USE_64BIT_TIME_T -DTIXML_USE_STL)

	# Remove Windows-specific files that have Qt/Linux replacements
	list(REMOVE_ITEM src_files
		./Parameters.cpp
	)

	# Add Platform abstraction for Linux
	list(APPEND src_files
		./Platform/Linux/FileSystem.cpp
		./Platform/Linux/Settings.cpp
		./Platform/Linux/Process.cpp
		./Platform/Linux/FileWatcher.cpp
		./Platform/Linux/Clipboard.cpp
		./Platform/Linux/Threading.cpp
		./Platform/Linux/Dialogs.cpp
	)

	# Add QtControls for Linux
	list(APPEND src_files
		./QtControls/StaticDialog/StaticDialog.cpp
		./QtControls/ToolBar/ToolBar.cpp
		./QtControls/StatusBar/StatusBar.cpp
		./QtControls/TabBar/TabBar.cpp
		./QtControls/Splitter/Splitter.cpp
		./QtControls/GoToLine/GoToLineDlg.cpp
		./QtControls/RunDlg/RunDlg.cpp
		./QtControls/AboutDlg/AboutDlg.cpp
		./QtControls/TreeView/TreeView.cpp
		./QtControls/FindReplace/FindReplaceDlg.cpp
		./QtControls/ShortcutMapper/ShortcutMapper.cpp
		./QtControls/KDEStyle/KDEStyleManager.cpp
		./QtControls/KDEStyle/KDEUtils.cpp
	)

	# Add QtCore for Linux
	list(APPEND src_files
		./QtCore/Buffer.cpp
		./QtCore/NppIO.cpp
		./QtCore/NppCommands.cpp
	)

	# Add Linux main entry point
	list(APPEND src_files
		./main_linux.cpp
	)

	# For Linux, use a set of files that have been ported or are portable
	# First clear the existing src_files and rebuild for Linux
	set(src_files "")

	# ============================================================================
	# Core Notepad++ Source Files
	# ============================================================================

	# Qt-specific core files (Linux adaptations)
	list(APPEND src_files
		./QtCore/Buffer.cpp
		./QtCore/NppIO.cpp
		./QtCore/NppCommands.cpp
		./QtCore/ScintillaEditViewQt.cpp
		./QtCore/Parameters.cpp
		./QtCore/Win32_IO_File.cpp
		./QtCore/NativeLangSpeaker.cpp
		./QtCore/NppDarkMode.cpp
	)

	# Main entry point for Linux
	list(APPEND src_files
		./main_linux.cpp
	)

	# Core application files (portable or Qt-specific)
	list(APPEND src_files
		./EncodingMapper.cpp
		./Utf8_16.cpp
	)

	# ============================================================================
	# Platform Abstraction Layer
	# ============================================================================
	list(APPEND src_files
		./Platform/Linux/FileSystem.cpp
		./Platform/Linux/Settings.cpp
		./Platform/Linux/Process.cpp
		./Platform/Linux/FileWatcher.cpp
		./Platform/Linux/Clipboard.cpp
		./Platform/Linux/Threading.cpp
		./Platform/Linux/Dialogs.cpp
	)

	# ============================================================================
	# MISC - Common Utilities
	# ============================================================================
	# Note: SortLocale.cpp is excluded from Linux build as it uses Windows-specific
	# locale functions (LCMAP_SORTKEY, NORM_LINGUISTIC_CASING, LINGUISTIC_IGNORECASE,
	# SORT_DIGITSASNUMBERS, LINGUISTIC_IGNOREDIACRITIC, NORM_IGNORESYMBOLS,
	# LOCALE_NAME_USER_DEFAULT, MultiByteToWideChar, LCMapStringEx)
	# Note: Common.cpp is excluded from Linux build as it uses Windows-specific
	# APIs (MultiByteToWideChar, WideCharToMultiByte, Windows-specific file APIs).
	# CommonLinux.cpp provides the Linux implementation using iconv.
	list(APPEND src_files
		./MISC/Common/verifySignedfile.cpp
		./MISC/Common/CommonLinux.cpp
	)

	# MISC - Cryptography
	# Note: sha1.cpp is included for Linux build but with SHA1_NO_UTILITY_FUNCTIONS
	# defined to exclude the Windows-specific _wfopen function in HashFile.
	# The CSHA1 class (constructor, Update, Final, GetHash) is fully portable.
	list(APPEND src_files
		./MISC/sha1/calc_sha1.cpp
		./MISC/sha1/sha1.cpp
		./MISC/sha2/sha-256.cpp
		./MISC/sha512/sha512.cpp
	)
	# Disable SHA1 utility functions (HashFile uses Windows-specific _wfopen)
	# Also disable STL functions as they depend on utility functions
	list(APPEND defs -DSHA1_NO_UTILITY_FUNCTIONS -DSHA1_NO_STL_FUNCTIONS)

	# MISC - Plugins and Process
	# Note: PluginsManager.cpp is excluded from Linux build as it uses Windows-specific
	# DLL loading (windows.h, LoadLibrary, GetProcAddress).
	# PluginsManagerLinux.cpp provides the Linux implementation using dlopen/dlsym.
	# Note: Processus.cpp is excluded from Linux build as it uses Windows-specific
	# process APIs (ShellExecute, ShellExecuteEx, SHELLEXECUTEINFO, GetExitCodeProcess,
	# GetLastError). Platform/Linux/Process.cpp provides the Linux implementation.
	# Note: regExtDlg.cpp is excluded from Linux build as it uses Windows-specific
	# registry APIs (RegCreateKeyEx, RegOpenKeyEx, RegSetValueEx, RegQueryValueEx,
	# RegDeleteValue, RegDeleteKey, RegCloseKey, HKEY_CLASSES_ROOT, KEY_ALL_ACCESS,
	# REG_SZ). This file is for Windows file associations only.
	list(APPEND src_files
		./MISC/PluginsManager/PluginsManagerLinux.cpp
	)

	# MISC - Exception handling
	# Note: MiniDumper.cpp and Win32Exception.cpp are excluded from Linux build
	# as they use Windows-specific APIs (shlwapi.h, Windows crash dump generation,
	# dbghelp.h, SetUnhandledExceptionFilter, etc.).
	# Linux exception handling is provided by the standard C++ runtime and Qt.

	# ============================================================================
	# Scintilla Component
	# ============================================================================
	# Note: Buffer.cpp is excluded from Linux build as it uses Windows-specific
	# APIs (_wfopen, __except/EXCEPTION_EXECUTE_HANDLER, PathRemoveFileSpec,
	# lstrcpy, lstrlen, ERROR_ACCESS_DENIED, invokeNppUacOp).
	# The Qt version in QtCore/Buffer.cpp is used instead.
	#
	# Note: The following Windows-specific ScintillaComponent files are excluded
	# from Linux build as they use Windows APIs. Qt alternatives are in QtControls/:
	# - AutoCompletion.cpp - Windows UI APIs
	# - columnEditor.cpp - Windows dialog APIs (GetDlgCtrlID, WM_PRINTCLIENT, etc.)
	# - DocTabView.cpp - Windows tab control
	# - FindReplaceDlg.cpp - Windows dialog (Qt alternative: QtControls/FindReplace/)
	# - FunctionCallTip.cpp - Windows tooltips
	# - GoToLineDlg.cpp - Windows dialog (Qt alternative: QtControls/GoToLine/)
	# - Printer.cpp - Windows printing APIs
	# - ScintillaEditView.cpp - Windows Scintilla integration
	# - SmartHighlighter.cpp - Windows-specific
	# - UserDefineDialog.cpp - Windows dialog (Qt alternative: QtControls/UserDefineDialog/)
	# - xmlMatchedTagsHighlighter.cpp - Windows-specific
	list(APPEND src_files
		./ScintillaComponent/ScintillaCtrls.cpp
	)

	# ============================================================================
	# Character Encoding Detection (uchardet)
	# ============================================================================
	list(APPEND src_files
		./uchardet/CharDistribution.cpp
		./uchardet/JpCntx.cpp
		./uchardet/LangBulgarianModel.cpp
		./uchardet/LangCyrillicModel.cpp
		./uchardet/LangGreekModel.cpp
		./uchardet/LangHebrewModel.cpp
		./uchardet/LangHungarianModel.cpp
		./uchardet/LangThaiModel.cpp
		./uchardet/nsBig5Prober.cpp
		./uchardet/nsCharSetProber.cpp
		./uchardet/nsEscCharsetProber.cpp
		./uchardet/nsEscSM.cpp
		./uchardet/nsEUCJPProber.cpp
		./uchardet/nsEUCKRProber.cpp
		./uchardet/nsEUCTWProber.cpp
		./uchardet/nsGB2312Prober.cpp
		./uchardet/nsHebrewProber.cpp
		./uchardet/nsLatin1Prober.cpp
		./uchardet/nsMBCSGroupProber.cpp
		./uchardet/nsMBCSSM.cpp
		./uchardet/nsSBCharSetProber.cpp
		./uchardet/nsSBCSGroupProber.cpp
		./uchardet/nsSJISProber.cpp
		./uchardet/nsUniversalDetector.cpp
		./uchardet/nsUTF8Prober.cpp
		./uchardet/uchardet.cpp
	)

	# ============================================================================
	# XML Processing
	# ============================================================================
	list(APPEND src_files
		./pugixml/pugixml.cpp
		./TinyXml/tinystr.cpp
		./TinyXml/tinyxml.cpp
		./TinyXml/tinyxmlerror.cpp
		./TinyXml/tinyxmlparser.cpp
	)

	# ============================================================================
	# Dark Mode Support
	# ============================================================================
	list(APPEND src_files
		./DarkMode/DarkMode.cpp
	)

	# ============================================================================
	# Qt Controls (Linux-specific UI implementations)
	# ============================================================================
	list(APPEND src_files
		./QtControls/Window.cpp
		./QtControls/StaticDialog/StaticDialog.cpp
		./QtControls/ToolBar/ToolBar.cpp
		./QtControls/StatusBar/StatusBar.cpp
		./QtControls/TabBar/TabBar.cpp
		./QtControls/DocTabView/DocTabView.cpp
		./QtControls/Splitter/Splitter.cpp
		./QtControls/SplitterContainer/SplitterContainer.cpp
		./QtControls/GoToLine/GoToLineDlg.cpp
		./QtControls/RunDlg/RunDlg.cpp
		./QtControls/RunMacroDlg/RunMacroDlg.cpp
		./QtControls/AboutDlg/AboutDlg.cpp
		./QtControls/AboutDlg/DebugInfoDlg.cpp
		./QtControls/AboutDlg/CmdLineArgsDlg.cpp
		./QtControls/TreeView/TreeView.cpp
		./QtControls/FindReplace/FindReplaceDlg.cpp
		./QtControls/ShortcutMapper/ShortcutMapper.cpp
		./QtControls/ShortcutManager/ShortcutManager.cpp
		./QtControls/DockingManager/DockingManager.cpp
		./QtControls/ListView/ListView.cpp
		./QtControls/WordStyleDlg/WordStyleDlg.cpp
		./QtControls/UserDefineDialog/UserDefineDialog.cpp
		./QtControls/ClipboardHistory/ClipboardHistoryPanel.cpp
		./QtControls/DocumentMap/DocumentMap.cpp
		./QtControls/ProjectPanel/ProjectPanel.cpp
		./QtControls/FileBrowser/FileBrowser.cpp
		./QtControls/FunctionList/FunctionListPanel.cpp
		./QtControls/MainWindow/Notepad_plus_Window.cpp
		./QtControls/Preference/preferenceDlg.cpp
		./QtControls/FindCharsInRange/FindCharsInRangeDlg.cpp
		./QtControls/ColumnEditor/ColumnEditorDlg.cpp
		./QtControls/SmartHighlighter/SmartHighlighter.cpp
		./QtControls/HashDlgs/HashFromFilesDlg.cpp
		./QtControls/HashDlgs/HashFromTextDlg.cpp
		./QtControls/VerticalFileSwitcher/VerticalFileSwitcher.cpp
		./QtControls/PluginsAdmin/PluginsAdminDlg.cpp
		./QtControls/PluginsAdmin/PluginViewList.cpp
		./QtControls/PluginsAdmin/PluginData.cpp
		./QtControls/Notepad_plus.cpp
	)

	# ============================================================================
	# Localization
	# ============================================================================
	list(APPEND src_files
		./localization.cpp
	)

	# ============================================================================
	# Recent Files List
	# ============================================================================
	list(APPEND src_files
		./QtCore/LastRecentFileList.cpp
	)

	# ============================================================================
	# Optional KDE Frameworks Integration
	# ============================================================================
	option(NPP_ENABLE_KDE "Enable KDE Frameworks integration for native Plasma dialogs" OFF)

	if(NPP_ENABLE_KDE)
		# Find KDE Frameworks components
		find_package(ECM QUIET)
		if(ECM_FOUND)
			set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
			find_package(KF6 COMPONENTS CoreAddons WidgetsAddons KIO Notifications Wallet Config)

			if(KF6CoreAddons_FOUND AND KF6WidgetsAddons_FOUND AND KF6KIO_FOUND)
				message(STATUS "KDE Frameworks 6 found - enabling native Plasma integration")
				set(NPP_KDE_AVAILABLE TRUE)
				list(APPEND defs -DNPP_KDE_AVAILABLE)

				# Add KDE Dialogs implementation
				list(APPEND src_files
					./Platform/KDE/KDEDialogs.cpp
				)

				# Add KDE Style integration (requires Qt6 D-Bus and KDE Frameworks)
				list(APPEND src_files
					./QtControls/KDEStyle/KDEStyleManager.cpp
					./QtControls/KDEStyle/KDEUtils.cpp
				)

				# Find Qt6 D-Bus for KDE integration
				find_package(Qt6 COMPONENTS DBus QUIET)
				if(Qt6DBus_FOUND)
					list(APPEND linux_LIBRARIES Qt6::DBus)
					message(STATUS "Qt6 D-Bus found - enabling KDE D-Bus integration")
				else()
					message(WARNING "Qt6 D-Bus not found - KDE style integration may be limited")
				endif()
				list(APPEND projIncludDir
					./Platform/KDE
					${KF6CoreAddons_INCLUDE_DIRS}
					${KF6WidgetsAddons_INCLUDE_DIRS}
					${KF6KIO_INCLUDE_DIRS}
				)
				list(APPEND linux_LIBRARIES
					KF6::CoreAddons
					KF6::WidgetsAddons
					KF6::KIOWidgets
				)

				if(KF6Notifications_FOUND)
					list(APPEND linux_LIBRARIES KF6::Notifications)
				endif()

				if(KF6Wallet_FOUND)
					list(APPEND linux_LIBRARIES KF6::Wallet)
				endif()

				if(KF6Config_FOUND)
					list(APPEND linux_LIBRARIES KF6::ConfigCore KF6::ConfigGui)
				endif()
			else()
				message(WARNING "KDE Frameworks components not found - falling back to Qt dialogs")
			endif()
		else()
			message(STATUS "ECM not found - KDE integration disabled")
		endif()
	else()
		message(STATUS "KDE integration disabled (use -DNPP_ENABLE_KDE=ON to enable)")
	endif()

	# Linux-specific includes
	# Note: Qt6 include directories are handled via target_include_directories
	# after the target is created, using the Qt6:: targets which automatically
	# provide their include directories
	# IMPORTANT: QtControls must come BEFORE WinControls so Qt versions of headers
	# are found first (ToolBar, StatusBar, StaticDialog, etc.)
	# Remove WinControls paths that have Qt replacements
	# Remove WinControls subdirectories that have Qt replacements
	# This ensures Qt versions of headers are found first
	list(REMOVE_ITEM projIncludDir ./WinControls/ToolBar/)
	list(REMOVE_ITEM projIncludDir ./WinControls/StaticDialog/)
	list(REMOVE_ITEM projIncludDir ./WinControls/StaticDialog/RunDlg/)
	list(REMOVE_ITEM projIncludDir ./WinControls/AboutDlg/)
	list(REMOVE_ITEM projIncludDir ./WinControls/ContextMenu/)
	list(REMOVE_ITEM projIncludDir ./WinControls/shortcut/)
	list(REMOVE_ITEM projIncludDir ./WinControls/ClipboardHistory/)
	list(REMOVE_ITEM projIncludDir ./WinControls/DocumentMap/)
	list(REMOVE_ITEM projIncludDir ./WinControls/FileBrowser/)
	list(REMOVE_ITEM projIncludDir ./WinControls/FunctionList/)
	list(REMOVE_ITEM projIncludDir ./WinControls/Preference/)
	list(REMOVE_ITEM projIncludDir ./WinControls/ProjectPanel/)
	list(REMOVE_ITEM projIncludDir ./WinControls/SplitterContainer/)
	list(REMOVE_ITEM projIncludDir ./WinControls/StatusBar/)
	list(REMOVE_ITEM projIncludDir ./WinControls/TabBar/)
	list(REMOVE_ITEM projIncludDir ./WinControls/TreeView/)
	list(REMOVE_ITEM projIncludDir ./WinControls/ColourPicker/)
	list(REMOVE_ITEM projIncludDir ./WinControls/AnsiCharPanel/)
	list(REMOVE_ITEM projIncludDir ./WinControls/FindCharsInRange/)
	list(REMOVE_ITEM projIncludDir ./WinControls/PluginsAdmin/)
	list(REMOVE_ITEM projIncludDir ./WinControls/WindowsDlg/)
	list(REMOVE_ITEM projIncludDir ./WinControls/DockingWnd/)
	list(REMOVE_ITEM projIncludDir ./WinControls/Grid/)
	list(INSERT projIncludDir 0
		.
		./Platform
		./QtCore
		./QtControls
		./QtControls/HashDlgs
		./QtControls/ToolBar
		./QtControls/StatusBar
		./QtControls/ShortcutManager
		./QtControls/PluginsAdmin
		./MISC
		./MISC/Common
		./MISC/PluginsManager
		./WinControls
		./WinControls/PluginsAdmin
		./ScintillaComponent
		${CMAKE_SOURCE_DIR}/../../scintilla/include
	)

	# Find or build Scintilla for Linux
	# Try to find QScintilla first
	find_package(QScintilla2 QUIET)
	if(QScintilla2_FOUND)
		message(STATUS "QScintilla2 found")
	else()
		message(STATUS "QScintilla2 not found - will use built-in Scintilla")
		# Build Scintilla from source
		set(SCINTILLA_QT_DIR "${CMAKE_SOURCE_DIR}/../../scintilla/qt")
		if(EXISTS "${SCINTILLA_QT_DIR}/ScintillaEditBase")
			file(GLOB SCINTILLA_QT_SOURCES
				"${SCINTILLA_QT_DIR}/ScintillaEditBase/*.cpp"
			)
			file(GLOB SCINTILLA_CORE_SOURCES
				"${CMAKE_SOURCE_DIR}/../../scintilla/src/*.cxx"
			)
			add_library(scintilla_qt STATIC
				${SCINTILLA_QT_SOURCES}
				${SCINTILLA_CORE_SOURCES}
			)
			target_include_directories(scintilla_qt PUBLIC
				"${CMAKE_SOURCE_DIR}/../../scintilla/include"
				"${CMAKE_SOURCE_DIR}/../../scintilla/src"
				"${SCINTILLA_QT_DIR}/ScintillaEditBase"
			)
			target_link_libraries(scintilla_qt PUBLIC Qt6::Widgets)
			set(SCINTILLA_STATIC_LIBRARY scintilla_qt)
		endif()
	endif()

	# Build Lexilla for Linux
	file(GLOB LEXILLA_SOURCES
		"${CMAKE_SOURCE_DIR}/../../lexilla/src/*.cxx"
		"${CMAKE_SOURCE_DIR}/../../lexilla/lexlib/*.cxx"
		"${CMAKE_SOURCE_DIR}/../../lexilla/lexers/*.cxx"
	)
	# Remove Windows-specific lexers from Linux build
	list(REMOVE_ITEM LEXILLA_SOURCES
		"${CMAKE_SOURCE_DIR}/../../lexilla/lexers/LexUser.cxx"
	)
	add_library(lexilla STATIC ${LEXILLA_SOURCES})
	target_include_directories(lexilla PUBLIC
		"${CMAKE_SOURCE_DIR}/../../lexilla/include"
		"${CMAKE_SOURCE_DIR}/../../lexilla/lexlib"
		"${CMAKE_SOURCE_DIR}/../../scintilla/include"
	)
	set(LEXILLA_STATIC_LIBRARY lexilla)

	# Find OpenSSL for Linux (needed for SHA512)
	find_package(OpenSSL REQUIRED)
	if(OpenSSL_FOUND)
		message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
	endif()

	# Linux libraries
	SET(linux_LIBRARIES
		Qt6::Core
		Qt6::Widgets
		Qt6::Gui
		Qt6::Network
		Qt6::PrintSupport
		OpenSSL::Crypto
		pthread
		dl
	)

	if(QScintilla2_FOUND)
		list(APPEND linux_LIBRARIES QScintilla2::QScintilla2)
	endif()

	# No RC files on Linux
	SET(rcFiles "")

	message(STATUS "Linux configuration complete")

	# Get Qt6 include directories for the global include path
	get_target_property(Qt6Core_INCLUDE_DIRS Qt6::Core INTERFACE_INCLUDE_DIRECTORIES)
	get_target_property(Qt6Widgets_INCLUDE_DIRS Qt6::Widgets INTERFACE_INCLUDE_DIRECTORIES)
	get_target_property(Qt6Gui_INCLUDE_DIRS Qt6::Gui INTERFACE_INCLUDE_DIRECTORIES)
	list(APPEND projIncludDir ${Qt6Core_INCLUDE_DIRS} ${Qt6Widgets_INCLUDE_DIRS} ${Qt6Gui_INCLUDE_DIRS})
ENDIF()

ADD_DEFINITIONS(${defs})

INCLUDE_DIRECTORIES(${projIncludDir})

# ============================================================================
# Executable Target
# ============================================================================
ADD_EXECUTABLE(notepad++ ${option} ${src_files} ${include_files} ${rcFiles})

# ============================================================================
# Include Directories (target-specific for Linux)
# ============================================================================
IF(UNIX AND NOT APPLE)
	# Use target_include_directories with Qt6 targets for proper include path propagation
	# Note: QtControls paths must come BEFORE ScintillaComponent to ensure Qt versions
	# of dialogs (GoToLineDlg, FindReplaceDlg, etc.) are found first
	TARGET_INCLUDE_DIRECTORIES(notepad++ PRIVATE
		./Platform
		./QtCore
		./MISC
		./MISC/Common
		./MISC/Process
		./MISC/PluginsManager
		./MISC/RegExt
		./MISC/md5
		./MISC/sha1
		./MISC/sha2
		./MISC/sha512
		./MISC/Exception
		./QtControls
		./QtControls/StaticDialog
		./QtControls/ToolBar
		./QtControls/StatusBar
		./QtControls/TabBar
		./QtControls/DocTabView
		./QtControls/Splitter
		./QtControls/SplitterContainer
		./QtControls/GoToLine
		./QtControls/RunDlg
		./QtControls/AboutDlg
		./QtControls/TreeView
		./QtControls/FindReplace
		./QtControls/ShortcutMapper
		./QtControls/Shortcut
		./QtControls/KDEStyle
		./QtControls/DockingManager
		./QtControls/ListView
		./QtControls/WordStyleDlg
		./QtControls/UserDefineDialog
		./QtControls/ClipboardHistory
		./QtControls/DocumentMap
		./QtControls/ProjectPanel
		./QtControls/FileBrowser
		./QtControls/FunctionList
		./QtControls/MainWindow
		./QtControls/Preference
		./QtControls/FindCharsInRange
		./QtControls/ColumnEditor
		./QtControls/VerticalFileSwitcher
		./WinControls
		./WinControls/AboutDlg
		./WinControls/AnsiCharPanel
		./WinControls/ClipboardHistory
		./WinControls/ColourPicker
		./WinControls/ContextMenu
		./WinControls/DockingWnd
		./WinControls/DocumentMap
		./WinControls/FileBrowser
		./WinControls/FindCharsInRange
		./WinControls/FunctionList
		./WinControls/Grid
		./WinControls/ImageListSet
		./WinControls/OpenSaveFileDialog
		./WinControls/PluginsAdmin
		./WinControls/Preference
		./WinControls/ProjectPanel
		./WinControls/ReadDirectoryChanges
		./WinControls/Shortcut
		./WinControls/SplitterContainer
		./WinControls/StaticDialog
		./WinControls/StaticDialog/RunDlg
		./WinControls/StatusBar
		./WinControls/TabBar
		./WinControls/TaskList
		./WinControls/ToolTip
		./WinControls/TrayIcon
		./WinControls/TreeView
		./WinControls/VerticalFileSwitcher
		./WinControls/WindowsDlg
		./uchardet
		./TinyXml
		./pugixml
		./DarkMode
	)
ENDIF()

# ============================================================================
# Link Libraries
# ============================================================================
IF(WIN32)
	TARGET_LINK_LIBRARIES(notepad++ ${SCINTILLA_STATIC_LIBRARY} ${LEXILLA_STATIC_LIBRARY} ${win32_LIBRARIES})
ELSEIF(UNIX AND NOT APPLE)
	TARGET_LINK_LIBRARIES(notepad++ ${LEXILLA_STATIC_LIBRARY} ${linux_LIBRARIES})

	# Set output name for Linux
	set_target_properties(notepad++ PROPERTIES
		OUTPUT_NAME "notepad-plus-plus"
	)
ENDIF()

# ============================================================================
# Installation
# ============================================================================
IF(UNIX AND NOT APPLE)
	# Install binary
	install(TARGETS notepad++
		RUNTIME DESTINATION bin
	)

	# Install documentation
	install(FILES
		${CMAKE_SOURCE_DIR}/../../LICENSE
		${CMAKE_SOURCE_DIR}/../../README.md
		${CMAKE_SOURCE_DIR}/../../BUILD.md
		${CMAKE_SOURCE_DIR}/../../CONTRIBUTING.md
		DESTINATION share/doc/notepad-plus-plus
	)

	# Install desktop file (if it exists in source)
	if(EXISTS "${CMAKE_SOURCE_DIR}/../../packaging/common/notepad-plus-plus.desktop")
		install(FILES
			${CMAKE_SOURCE_DIR}/../../packaging/common/notepad-plus-plus.desktop
			DESTINATION share/applications
		)
	endif()

	# Install metainfo/appdata (if it exists)
	if(EXISTS "${CMAKE_SOURCE_DIR}/../../packaging/common/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml")
		install(FILES
			${CMAKE_SOURCE_DIR}/../../packaging/common/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml
			DESTINATION share/metainfo
		)
	endif()

	# Install MIME type definition (if it exists)
	if(EXISTS "${CMAKE_SOURCE_DIR}/../../packaging/common/notepad-plus-plus-mime.xml")
		install(FILES
			${CMAKE_SOURCE_DIR}/../../packaging/common/notepad-plus-plus-mime.xml
			DESTINATION share/mime/packages
		)
	endif()

	# Install icons (if they exist)
	if(EXISTS "${CMAKE_SOURCE_DIR}/../../packaging/common/notepad-plus-plus.svg")
		install(FILES
			${CMAKE_SOURCE_DIR}/../../packaging/common/notepad-plus-plus.svg
			DESTINATION share/icons/hicolor/scalable/apps
		)
	endif()

	# Create symlink for notepad++
	install(CODE "
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E create_symlink
				notepad-plus-plus
				notepad++
			WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX}/bin
		)
	")

	# Install localization files (if they exist)
	if(EXISTS "${CMAKE_SOURCE_DIR}/../../PowerEditor/installer/nativeLang")
		install(DIRECTORY
			${CMAKE_SOURCE_DIR}/../../PowerEditor/installer/nativeLang/
			DESTINATION share/notepad-plus-plus/localization
			FILES_MATCHING PATTERN "*.xml"
		)
	endif()

	# Install themes (if they exist)
	if(EXISTS "${CMAKE_SOURCE_DIR}/../../PowerEditor/installer/themes")
		install(DIRECTORY
			${CMAKE_SOURCE_DIR}/../../PowerEditor/installer/themes/
			DESTINATION share/notepad-plus-plus/themes
			FILES_MATCHING PATTERN "*.xml"
		)
	endif()

	# Install plugin directory structure
	install(DIRECTORY
		DESTINATION lib/notepad-plus-plus/plugins
	)

	# ============================================================================
	# Tests (Linux only)
	# ============================================================================
	option(BUILD_TESTS "Build test suite" ON)
	if(BUILD_TESTS)
		message(STATUS "Tests enabled - configure Tests/ directory")
		add_subdirectory(Tests)
	endif()
ENDIF()

