// This file is part of Notepad++ project
// Copyright (C)2025 Don HO <don.h@free.fr>

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

// Standard headers
#include <functional>
#include <map>
#include <vector>

// Qt headers
#include <QString>

// Forward declarations
class Notepad_plus;
class ScintillaEditView;

namespace QtCommands {

// Command ID enumeration for Qt version
// Maps to menuCmdID.h values for compatibility
enum CommandID {
    // File commands (IDM_FILE = 41000)
    CMD_FILE_NEW = 41001,
    CMD_FILE_OPEN = 41002,
    CMD_FILE_CLOSE = 41003,
    CMD_FILE_CLOSEALL = 41004,
    CMD_FILE_CLOSEALL_BUT_CURRENT = 41005,
    CMD_FILE_SAVE = 41006,
    CMD_FILE_SAVEALL = 41007,
    CMD_FILE_SAVEAS = 41008,
    CMD_FILE_CLOSEALL_TOLEFT = 41009,
    CMD_FILE_PRINT = 41010,
    CMD_FILE_PRINTNOW = 1001,
    CMD_FILE_EXIT = 41011,
    CMD_FILE_LOADSESSION = 41012,
    CMD_FILE_SAVESESSION = 41013,
    CMD_FILE_RELOAD = 41014,
    CMD_FILE_SAVECOPYAS = 41015,
    CMD_FILE_DELETE = 41016,
    CMD_FILE_RENAME = 41017,
    CMD_FILE_CLOSEALL_TORIGHT = 41018,
    CMD_FILE_OPEN_FOLDER = 41019,
    CMD_FILE_OPEN_CMD = 41020,
    CMD_FILE_RESTORELASTCLOSEDFILE = 41021,
    CMD_FILE_OPENFOLDERASWORKSPACE = 41022,
    CMD_FILE_OPEN_DEFAULT_VIEWER = 41023,
    CMD_FILE_CLOSEALL_UNCHANGED = 41024,
    CMD_FILE_CONTAININGFOLDERASWORKSPACE = 41025,
    CMD_FILE_CLOSEALL_BUT_PINNED = 41026,

    // Edit commands (IDM_EDIT = 42000)
    CMD_EDIT_CUT = 42001,
    CMD_EDIT_COPY = 42002,
    CMD_EDIT_UNDO = 42003,
    CMD_EDIT_REDO = 42004,
    CMD_EDIT_PASTE = 42005,
    CMD_EDIT_DELETE = 42006,
    CMD_EDIT_SELECTALL = 42007,
    CMD_EDIT_INS_TAB = 42008,
    CMD_EDIT_RMV_TAB = 42009,
    CMD_EDIT_DUP_LINE = 42010,
    CMD_EDIT_TRANSPOSE_LINE = 42011,
    CMD_EDIT_SPLIT_LINES = 42012,
    CMD_EDIT_JOIN_LINES = 42013,
    CMD_EDIT_LINE_UP = 42014,
    CMD_EDIT_LINE_DOWN = 42015,
    CMD_EDIT_UPPERCASE = 42016,
    CMD_EDIT_LOWERCASE = 42017,
    CMD_EDIT_BEGINENDSELECT = 42020,
    CMD_EDIT_BLOCK_COMMENT = 42022,
    CMD_EDIT_BLOCK_COMMENT_SET = 42035,
    CMD_EDIT_BLOCK_UNCOMMENT = 42036,
    CMD_EDIT_STREAM_COMMENT = 42023,
    CMD_EDIT_STREAM_UNCOMMENT = 42047,
    CMD_EDIT_TRIMTRAILING = 42024,
    CMD_EDIT_TRIMLINEHEAD = 42042,
    CMD_EDIT_TRIM_BOTH = 42043,
    CMD_EDIT_TAB2SPACE = 42046,
    CMD_EDIT_SPACE2TAB_LEADING = 42053,
    CMD_EDIT_SPACE2TAB_ALL = 42054,
    CMD_EDIT_REMOVE_CONSECUTIVE_DUP_LINES = 42077,
    CMD_EDIT_REMOVE_ANY_DUP_LINES = 42079,
    CMD_EDIT_BEGINENDSELECT_COLUMNMODE = 42089,

    // Search commands (IDM_SEARCH = 43000)
    CMD_SEARCH_FIND = 43001,
    CMD_SEARCH_FINDNEXT = 43002,
    CMD_SEARCH_REPLACE = 43003,
    CMD_SEARCH_GOTOLINE = 43004,
    CMD_SEARCH_TOGGLE_BOOKMARK = 43005,
    CMD_SEARCH_NEXT_BOOKMARK = 43006,
    CMD_SEARCH_PREV_BOOKMARK = 43007,
    CMD_SEARCH_CLEAR_BOOKMARKS = 43008,
    CMD_SEARCH_GOTOMATCHINGBRACE = 43009,
    CMD_SEARCH_FINDPREV = 43010,
    CMD_SEARCH_FINDINCREMENT = 43011,
    CMD_SEARCH_FINDINFILES = 43013,
    CMD_SEARCH_VOLATILE_FINDNEXT = 43014,
    CMD_SEARCH_VOLATILE_FINDPREV = 43015,
    CMD_SEARCH_CUTMARKEDLINES = 43018,
    CMD_SEARCH_COPYMARKEDLINES = 43019,
    CMD_SEARCH_PASTEMARKEDLINES = 43020,
    CMD_SEARCH_DELETEMARKEDLINES = 43021,
    CMD_SEARCH_MARKALLEXT1 = 43022,
    CMD_SEARCH_UNMARKALLEXT1 = 43023,
    CMD_SEARCH_MARKALLEXT2 = 43024,
    CMD_SEARCH_UNMARKALLEXT2 = 43025,
    CMD_SEARCH_MARKALLEXT3 = 43026,
    CMD_SEARCH_UNMARKALLEXT3 = 43027,
    CMD_SEARCH_MARKALLEXT4 = 43028,
    CMD_SEARCH_UNMARKALLEXT4 = 43029,
    CMD_SEARCH_MARKALLEXT5 = 43030,
    CMD_SEARCH_UNMARKALLEXT5 = 43031,
    CMD_SEARCH_CLEARALLMARKS = 43032,
    CMD_SEARCH_GOPREVMARKER1 = 43033,
    CMD_SEARCH_GOPREVMARKER2 = 43034,
    CMD_SEARCH_GOPREVMARKER3 = 43035,
    CMD_SEARCH_GOPREVMARKER4 = 43036,
    CMD_SEARCH_GOPREVMARKER5 = 43037,
    CMD_SEARCH_GOPREVMARKER_DEF = 43038,
    CMD_SEARCH_GONEXTMARKER1 = 43039,
    CMD_SEARCH_GONEXTMARKER2 = 43040,
    CMD_SEARCH_GONEXTMARKER3 = 43041,
    CMD_SEARCH_GONEXTMARKER4 = 43042,
    CMD_SEARCH_GONEXTMARKER5 = 43043,
    CMD_SEARCH_GONEXTMARKER_DEF = 43044,
    CMD_SEARCH_GOTONEXTFOUND = 43046,
    CMD_SEARCH_GOTOPREVFOUND = 43047,
    CMD_SEARCH_SETANDFINDNEXT = 43048,
    CMD_SEARCH_SETANDFINDPREV = 43049,
    CMD_SEARCH_INVERSEMARKS = 43050,
    CMD_SEARCH_DELETEUNMARKEDLINES = 43051,
    CMD_SEARCH_FINDCHARINRANGE = 43052,
    CMD_SEARCH_SELECTMATCHINGBRACES = 43053,
    CMD_SEARCH_MARK = 43054,

    // View commands (IDM_VIEW = 44000)
    CMD_VIEW_POSTIT = 44009,
    CMD_VIEW_FOLDALL = 44010,
    CMD_VIEW_DISTRACTIONFREE = 44011,
    CMD_VIEW_ALL_CHARACTERS = 44019,
    CMD_VIEW_INDENT_GUIDE = 44020,
    CMD_VIEW_WRAP = 44022,
    CMD_VIEW_ZOOMIN = 44023,
    CMD_VIEW_ZOOMOUT = 44024,
    CMD_VIEW_TAB_SPACE = 44025,
    CMD_VIEW_EOL = 44026,
    CMD_VIEW_UNFOLDALL = 44029,
    CMD_VIEW_FOLD_CURRENT = 44030,
    CMD_VIEW_UNFOLD_CURRENT = 44031,
    CMD_VIEW_FULLSCREENTOGGLE = 44032,
    CMD_VIEW_ZOOMRESTORE = 44033,
    CMD_VIEW_ALWAYSONTOP = 44034,
    CMD_VIEW_SYNSCROLLV = 44035,
    CMD_VIEW_SYNSCROLLH = 44036,
    CMD_VIEW_WRAP_SYMBOL = 44041,
    CMD_VIEW_HIDELINES = 44042,
    CMD_VIEW_SUMMARY = 44049,
    CMD_VIEW_FOLD_1 = 44051,
    CMD_VIEW_FOLD_2 = 44052,
    CMD_VIEW_FOLD_3 = 44053,
    CMD_VIEW_FOLD_4 = 44054,
    CMD_VIEW_FOLD_5 = 44055,
    CMD_VIEW_FOLD_6 = 44056,
    CMD_VIEW_FOLD_7 = 44057,
    CMD_VIEW_FOLD_8 = 44058,
    CMD_VIEW_UNFOLD_1 = 44061,
    CMD_VIEW_UNFOLD_2 = 44062,
    CMD_VIEW_UNFOLD_3 = 44063,
    CMD_VIEW_UNFOLD_4 = 44064,
    CMD_VIEW_UNFOLD_5 = 44065,
    CMD_VIEW_UNFOLD_6 = 44066,
    CMD_VIEW_UNFOLD_7 = 44067,
    CMD_VIEW_UNFOLD_8 = 44068,
    CMD_VIEW_DOCLIST = 44070,
    CMD_VIEW_SWITCHTO_OTHER_VIEW = 44072,
    CMD_VIEW_DOC_MAP = 44080,
    CMD_VIEW_PROJECT_PANEL_1 = 44081,
    CMD_VIEW_PROJECT_PANEL_2 = 44082,
    CMD_VIEW_PROJECT_PANEL_3 = 44083,
    CMD_VIEW_FUNC_LIST = 44084,
    CMD_VIEW_FILEBROWSER = 44085,
    CMD_VIEW_TAB1 = 44086,
    CMD_VIEW_TAB2 = 44087,
    CMD_VIEW_TAB3 = 44088,
    CMD_VIEW_TAB4 = 44089,
    CMD_VIEW_TAB5 = 44090,
    CMD_VIEW_TAB6 = 44091,
    CMD_VIEW_TAB7 = 44092,
    CMD_VIEW_TAB8 = 44093,
    CMD_VIEW_TAB9 = 44094,
    CMD_VIEW_TAB_NEXT = 44095,
    CMD_VIEW_TAB_PREV = 44096,
    CMD_VIEW_MONITORING = 44097,
    CMD_VIEW_TAB_MOVEFORWARD = 44098,
    CMD_VIEW_TAB_MOVEBACKWARD = 44099,
    CMD_VIEW_SWITCHTO_PROJECT_PANEL_1 = 44104,
    CMD_VIEW_SWITCHTO_PROJECT_PANEL_2 = 44105,
    CMD_VIEW_SWITCHTO_PROJECT_PANEL_3 = 44106,
    CMD_VIEW_SWITCHTO_FILEBROWSER = 44107,
    CMD_VIEW_SWITCHTO_FUNC_LIST = 44108,
    CMD_VIEW_SWITCHTO_DOCLIST = 44109,
    CMD_VIEW_TAB_START = 44116,
    CMD_VIEW_TAB_END = 44117,

    // Macro commands (IDM_EDIT + 18-21, 25, 32)
    CMD_MACRO_STARTRECORDINGMACRO = 42018,
    CMD_MACRO_STOPRECORDINGMACRO = 42019,
    CMD_MACRO_PLAYBACKRECORDEDMACRO = 42021,
    CMD_MACRO_SAVECURRENTMACRO = 42025,
    CMD_MACRO_RUNMULTIMACRODLG = 42032,

    // Format commands (IDM_FORMAT = 45000)
    CMD_FORMAT_TODOS = 45001,
    CMD_FORMAT_TOUNIX = 45002,
    CMD_FORMAT_TOMAC = 45003,
    CMD_FORMAT_ANSI = 45004,
    CMD_FORMAT_UTF_8 = 45005,
    CMD_FORMAT_UTF_16BE = 45006,
    CMD_FORMAT_UTF_16LE = 45007,
    CMD_FORMAT_AS_UTF_8 = 45008,

    // Language commands (IDM_LANG = 46000)
    CMD_LANG_USER_DLG = 46001,

    // Execute command
    CMD_EXECUTE = 46020,

    // Settings/Preference
    CMD_SETTING_PREFERENCE = 47000
};

// Command handler function type
using CommandHandlerFunc = std::function<void()>;

// Command handler registration and execution
class CommandHandler {
public:
    void registerCommand(int id, CommandHandlerFunc handler);
    void executeCommand(int id);
    bool canExecute(int id) const;
    void unregisterCommand(int id);
    void clearCommands();

private:
    std::map<int, CommandHandlerFunc> _handlers;
};

// Main command dispatcher class
class NppCommands {
public:
    explicit NppCommands(Notepad_plus* notepad_plus);
    ~NppCommands();

    // Initialize all command handlers
    void initializeCommands();

    // Execute a command by ID
    void execute(int commandID);

    // Check if command can be executed
    bool canExecute(int commandID) const;

    // Update command states (enable/disable menu items)
    void updateCommandState();

    // File commands
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void fileSaveCopyAs();
    void fileSaveAll();
    void fileClose();
    void fileCloseAll();
    void fileCloseAllButCurrent();
    void fileCloseAllButPinned();
    void fileCloseAllToLeft();
    void fileCloseAllToRight();
    void fileCloseAllUnchanged();
    void filePrint(bool showDialog);
    void filePrintNow();
    void fileExit();
    void fileReload();
    void fileDelete();
    void fileRename();
    void fileLoadSession();
    void fileSaveSession();
    void fileOpenFolderAsWorkspace();
    void fileOpenContainingFolder();
    void fileOpenCmd();

    // Edit commands
    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editDelete();
    void editSelectAll();
    void editBeginEndSelect(bool columnMode = false);
    void editInsertTab();
    void editRemoveTab();
    void editDuplicateLine();
    void editRemoveDuplicateLines();
    void editRemoveAnyDuplicateLines();
    void editTransposeLine();
    void editSplitLines();
    void editJoinLines();
    void editLineUp();
    void editLineDown();
    void editUpperCase();
    void editLowerCase();
    void editToggleComment();
    void editBlockComment();
    void editBlockCommentSet();
    void editBlockUncomment();
    void editStreamComment();
    void editStreamUncomment();
    void editTrimTrailing();
    void editTrimLineHead();
    void editTrimBoth();
    void editTabToSpace();
    void editSpaceToTabLeading();
    void editSpaceToTabAll();
    void editToggleReadOnly();
    void editToggleSystemReadOnly();
    void editSetReadOnlyForAllDocs();
    void editClearReadOnlyForAllDocs();
    void editFullPathToClipboard();
    void editFileNameToClipboard();
    void editCurrentDirToClipboard();
    void editCopyAllNames();
    void editCopyAllPaths();
    void editColumnMode();
    void editColumnModeTip();

    // Search commands
    void searchFind();
    void searchReplace();
    void searchMark();
    void searchFindNext();
    void searchFindPrev();
    void searchFindNextVolatile();
    void searchFindPrevVolatile();
    void searchFindInFiles();
    void searchFindInProjects();
    void searchFindIncrement();
    void searchSetAndFindNext();
    void searchSetAndFindPrev();
    void searchGoToNextFound();
    void searchGoToPrevFound();
    void searchGoToLine();
    void searchGoToMatchingBrace();
    void searchSelectMatchingBraces();
    void searchToggleBookmark();
    void searchNextBookmark();
    void searchPrevBookmark();
    void searchClearBookmarks();
    void searchCutMarkedLines();
    void searchCopyMarkedLines();
    void searchPasteMarkedLines();
    void searchDeleteMarkedLines();
    void searchDeleteUnmarkedLines();
    void searchInverseMarks();
    void searchMarkAllExt(int styleID);
    void searchUnmarkAllExt(int styleID);
    void searchMarkOneExt(int styleID);
    void searchClearAllMarks();
    void searchGoNextMarker(int styleID);
    void searchGoPrevMarker(int styleID);
    void searchFindCharInRange();
    void searchChangedNext();
    void searchChangedPrev();
    void searchClearChangeHistory();

    // View commands
    void viewFullScreen();
    void viewPostIt();
    void viewDistractionFree();
    void viewAlwaysOnTop();
    void viewWordWrap();
    void viewWrapSymbol();
    void viewHideLines();
    void viewZoomIn();
    void viewZoomOut();
    void viewZoomRestore();
    void viewIndentGuide();
    void viewShowWhiteSpace();
    void viewShowEOL();
    void viewShowAllCharacters();
    void viewShowNpc();
    void viewShowNpcCcUniEol();
    void viewSyncScrollV();
    void viewSyncScrollH();
    void viewSummary();
    void viewMonitoring();

    // Fold commands
    void viewFoldAll();
    void viewUnfoldAll();
    void viewFoldCurrent();
    void viewUnfoldCurrent();
    void viewFoldLevel(int level);
    void viewUnfoldLevel(int level);

    // Panel commands
    void viewDocumentList();
    void viewDocumentMap();
    void viewFunctionList();
    void viewFileBrowser();
    void viewProjectPanel(int index);
    void viewSwitchToProjectPanel(int index);
    void viewSwitchToFileBrowser();
    void viewSwitchToFuncList();
    void viewSwitchToDocList();
    void viewSwitchToOtherView();

    // Tab commands
    void viewTab(int index);
    void viewTabNext();
    void viewTabPrev();
    void viewTabStart();
    void viewTabEnd();
    void viewTabMoveForward();
    void viewTabMoveBackward();

    // Macro commands
    void macroStartRecording();
    void macroStopRecording();
    void macroPlayback();
    void macroSaveCurrent();
    void macroRunMultiMacroDlg();

    // Format commands
    void formatConvertToWindows();
    void formatConvertToUnix();
    void formatConvertToMac();
    void formatSetEncoding(int encoding);

    // Language commands
    void langUserDlg();

    // Run commands
    void executeRun();

    // Settings commands
    void settingPreference();

private:
    Notepad_plus* _pNotepad_plus;
    CommandHandler _handler;
    ScintillaEditView* _pEditView;

    // Helper methods
    void registerFileCommands();
    void registerEditCommands();
    void registerSearchCommands();
    void registerViewCommands();
    void registerMacroCommands();
    void registerFormatCommands();
    void registerLanguageCommands();
    void registerRunCommands();
    void registerSettingsCommands();

    // Get current edit view
    ScintillaEditView* getCurrentEditView();

    // Check if document is dirty (modified)
    bool isDocumentDirty() const;

    // Check if there is a selection
    bool hasSelection() const;

    // Check clipboard availability
    bool canPaste() const;
};

} // namespace QtCommands
