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

// Qt headers first (for moc compatibility)
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>

// Local headers
#include "NppCommands.h"
#include "Notepad_plus.h"
#include "ScintillaEditView.h"
#include "Buffer.h"
#include "Parameters.h"
#include "FindReplaceDlg.h"
#include "GoToLineDlg.h"
#include "menuCmdID.h"

namespace QtCommands {

// ============================================================================
// CommandHandler Implementation
// ============================================================================

void CommandHandler::registerCommand(int id, CommandHandlerFunc handler) {
    _handlers[id] = std::move(handler);
}

void CommandHandler::executeCommand(int id) {
    auto it = _handlers.find(id);
    if (it != _handlers.end() && it->second) {
        it->second();
    }
}

bool CommandHandler::canExecute(int id) const {
    auto it = _handlers.find(id);
    return it != _handlers.end() && it->second != nullptr;
}

void CommandHandler::unregisterCommand(int id) {
    _handlers.erase(id);
}

void CommandHandler::clearCommands() {
    _handlers.clear();
}

// ============================================================================
// NppCommands Implementation
// ============================================================================

NppCommands::NppCommands(Notepad_plus* notepad_plus)
    : _pNotepad_plus(notepad_plus)
    , _pEditView(nullptr) {
}

NppCommands::~NppCommands() {
    _handler.clearCommands();
}

void NppCommands::initializeCommands() {
    registerFileCommands();
    registerEditCommands();
    registerSearchCommands();
    registerViewCommands();
    registerMacroCommands();
    registerFormatCommands();
    registerLanguageCommands();
    registerRunCommands();
    registerSettingsCommands();
}

void NppCommands::execute(int commandID) {
    _handler.executeCommand(commandID);
}

bool NppCommands::canExecute(int commandID) const {
    return _handler.canExecute(commandID);
}

void NppCommands::updateCommandState() {
    // Update menu item states based on current document/context
    // This will be called to enable/disable menu items
}

ScintillaEditView* NppCommands::getCurrentEditView() {
    if (_pNotepad_plus) {
        return _pNotepad_plus->getCurrentEditView();
    }
    return nullptr;
}

bool NppCommands::isDocumentDirty() const {
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        return buf && buf->isDirty();
    }
    return false;
}

bool NppCommands::hasSelection() const {
    if (_pNotepad_plus) {
        ScintillaEditView* view = const_cast<NppCommands*>(this)->getCurrentEditView();
        if (view) {
            return view->hasSelection();
        }
    }
    return false;
}

bool NppCommands::canPaste() const {
    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    return mimeData && mimeData->hasText();
}

// ============================================================================
// File Commands Registration
// ============================================================================

void NppCommands::registerFileCommands() {
    _handler.registerCommand(CMD_FILE_NEW, [this]() { fileNew(); });
    _handler.registerCommand(CMD_FILE_OPEN, [this]() { fileOpen(); });
    _handler.registerCommand(CMD_FILE_SAVE, [this]() { fileSave(); });
    _handler.registerCommand(CMD_FILE_SAVEAS, [this]() { fileSaveAs(); });
    _handler.registerCommand(CMD_FILE_SAVECOPYAS, [this]() { fileSaveCopyAs(); });
    _handler.registerCommand(CMD_FILE_SAVEALL, [this]() { fileSaveAll(); });
    _handler.registerCommand(CMD_FILE_CLOSE, [this]() { fileClose(); });
    _handler.registerCommand(CMD_FILE_CLOSEALL, [this]() { fileCloseAll(); });
    _handler.registerCommand(CMD_FILE_CLOSEALL_BUT_CURRENT, [this]() { fileCloseAllButCurrent(); });
    _handler.registerCommand(CMD_FILE_CLOSEALL_BUT_PINNED, [this]() { fileCloseAllButPinned(); });
    _handler.registerCommand(CMD_FILE_CLOSEALL_TOLEFT, [this]() { fileCloseAllToLeft(); });
    _handler.registerCommand(CMD_FILE_CLOSEALL_TORIGHT, [this]() { fileCloseAllToRight(); });
    _handler.registerCommand(CMD_FILE_CLOSEALL_UNCHANGED, [this]() { fileCloseAllUnchanged(); });
    _handler.registerCommand(CMD_FILE_PRINT, [this]() { filePrint(true); });
    _handler.registerCommand(CMD_FILE_PRINTNOW, [this]() { filePrintNow(); });
    _handler.registerCommand(CMD_FILE_EXIT, [this]() { fileExit(); });
    _handler.registerCommand(CMD_FILE_RELOAD, [this]() { fileReload(); });
    _handler.registerCommand(CMD_FILE_DELETE, [this]() { fileDelete(); });
    _handler.registerCommand(CMD_FILE_RENAME, [this]() { fileRename(); });
    _handler.registerCommand(CMD_FILE_LOADSESSION, [this]() { fileLoadSession(); });
    _handler.registerCommand(CMD_FILE_SAVESESSION, [this]() { fileSaveSession(); });
    _handler.registerCommand(CMD_FILE_OPENFOLDERASWORKSPACE, [this]() { fileOpenFolderAsWorkspace(); });
    _handler.registerCommand(CMD_FILE_OPEN_FOLDER, [this]() { fileOpenContainingFolder(); });
    _handler.registerCommand(CMD_FILE_OPEN_CMD, [this]() { fileOpenCmd(); });
}

void NppCommands::fileNew() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileNew();
    }
}

void NppCommands::fileOpen() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileOpen();
    }
}

void NppCommands::fileSave() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSave();
    }
}

void NppCommands::fileSaveAs() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSaveAs();
    }
}

void NppCommands::fileSaveCopyAs() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSaveAs(BUFFER_INVALID, true);
    }
}

void NppCommands::fileSaveAll() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSaveAll();
    }
}

void NppCommands::fileClose() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileClose();
    }
}

void NppCommands::fileCloseAll() {
    if (_pNotepad_plus) {
        bool isSnapshotMode = NppParameters::getInstance().getNppGUI().isSnapshotMode();
        _pNotepad_plus->fileCloseAll(isSnapshotMode, false);
    }
}

void NppCommands::fileCloseAllButCurrent() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileCloseAllButCurrent();
    }
}

void NppCommands::fileCloseAllButPinned() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileCloseAllButPinned();
    }
}

void NppCommands::fileCloseAllToLeft() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileCloseAllToLeft();
    }
}

void NppCommands::fileCloseAllToRight() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileCloseAllToRight();
    }
}

void NppCommands::fileCloseAllUnchanged() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileCloseAllUnchanged();
    }
}

void NppCommands::filePrint(bool showDialog) {
    if (_pNotepad_plus) {
        _pNotepad_plus->filePrint(showDialog);
    }
}

void NppCommands::filePrintNow() {
    filePrint(false);
}

void NppCommands::fileExit() {
    // Emit close signal to main window
    if (_pNotepad_plus) {
        // Trigger application close
        QApplication::quit();
    }
}

void NppCommands::fileReload() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileReload();
    }
}

void NppCommands::fileDelete() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileDelete();
    }
}

void NppCommands::fileRename() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileRename();
    }
}

void NppCommands::fileLoadSession() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileLoadSession();
    }
}

void NppCommands::fileSaveSession() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fileSaveSession();
    }
}

void NppCommands::fileOpenFolderAsWorkspace() {
    if (_pNotepad_plus) {
        // Open folder dialog and add to file browser
        QString dir = QFileDialog::getExistingDirectory(nullptr,
            QObject::tr("Select a folder to add in Folder as Workspace panel"),
            QString(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (!dir.isEmpty()) {
            // Add to file browser
            std::vector<std::wstring> folders;
            folders.push_back(dir.toStdWString());
            _pNotepad_plus->launchFileBrowser(folders, L"");
        }
    }
}

void NppCommands::fileOpenContainingFolder() {
    // Open the folder containing the current file
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            std::wstring path = buf->getFullPathName();
            // Extract directory and open it
            size_t lastSlash = path.find_last_of(L"/\\");
            if (lastSlash != std::wstring::npos) {
                std::wstring dir = path.substr(0, lastSlash);
                // Open folder using platform abstraction
                // For now, use QDesktopServices
                // QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdWString(dir)));
            }
        }
    }
}

void NppCommands::fileOpenCmd() {
    // Open command prompt in current directory
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            std::wstring path = buf->getFullPathName();
            size_t lastSlash = path.find_last_of(L"/\\");
            if (lastSlash != std::wstring::npos) {
                std::wstring dir = path.substr(0, lastSlash);
                // Launch terminal in directory
                // Platform-specific implementation needed
            }
        }
    }
}

// ============================================================================
// Edit Commands Registration
// ============================================================================

void NppCommands::registerEditCommands() {
    _handler.registerCommand(CMD_EDIT_UNDO, [this]() { editUndo(); });
    _handler.registerCommand(CMD_EDIT_REDO, [this]() { editRedo(); });
    _handler.registerCommand(CMD_EDIT_CUT, [this]() { editCut(); });
    _handler.registerCommand(CMD_EDIT_COPY, [this]() { editCopy(); });
    _handler.registerCommand(CMD_EDIT_PASTE, [this]() { editPaste(); });
    _handler.registerCommand(CMD_EDIT_DELETE, [this]() { editDelete(); });
    _handler.registerCommand(CMD_EDIT_SELECTALL, [this]() { editSelectAll(); });
    _handler.registerCommand(CMD_EDIT_BEGINENDSELECT, [this]() { editBeginEndSelect(false); });
    _handler.registerCommand(CMD_EDIT_BEGINENDSELECT_COLUMNMODE, [this]() { editBeginEndSelect(true); });
    _handler.registerCommand(CMD_EDIT_INS_TAB, [this]() { editInsertTab(); });
    _handler.registerCommand(CMD_EDIT_RMV_TAB, [this]() { editRemoveTab(); });
    _handler.registerCommand(CMD_EDIT_DUP_LINE, [this]() { editDuplicateLine(); });
    _handler.registerCommand(CMD_EDIT_REMOVE_CONSECUTIVE_DUP_LINES, [this]() { editRemoveDuplicateLines(); });
    _handler.registerCommand(CMD_EDIT_REMOVE_ANY_DUP_LINES, [this]() { editRemoveAnyDuplicateLines(); });
    _handler.registerCommand(CMD_EDIT_TRANSPOSE_LINE, [this]() { editTransposeLine(); });
    _handler.registerCommand(CMD_EDIT_SPLIT_LINES, [this]() { editSplitLines(); });
    _handler.registerCommand(CMD_EDIT_JOIN_LINES, [this]() { editJoinLines(); });
    _handler.registerCommand(CMD_EDIT_LINE_UP, [this]() { editLineUp(); });
    _handler.registerCommand(CMD_EDIT_LINE_DOWN, [this]() { editLineDown(); });
    _handler.registerCommand(CMD_EDIT_UPPERCASE, [this]() { editUpperCase(); });
    _handler.registerCommand(CMD_EDIT_LOWERCASE, [this]() { editLowerCase(); });
    _handler.registerCommand(CMD_EDIT_BLOCK_COMMENT, [this]() { editBlockComment(); });
    _handler.registerCommand(CMD_EDIT_BLOCK_COMMENT_SET, [this]() { editBlockCommentSet(); });
    _handler.registerCommand(CMD_EDIT_BLOCK_UNCOMMENT, [this]() { editBlockUncomment(); });
    _handler.registerCommand(CMD_EDIT_STREAM_COMMENT, [this]() { editStreamComment(); });
    _handler.registerCommand(CMD_EDIT_STREAM_UNCOMMENT, [this]() { editStreamUncomment(); });
    _handler.registerCommand(CMD_EDIT_TRIMTRAILING, [this]() { editTrimTrailing(); });
    _handler.registerCommand(CMD_EDIT_TRIMLINEHEAD, [this]() { editTrimLineHead(); });
    _handler.registerCommand(CMD_EDIT_TRIM_BOTH, [this]() { editTrimBoth(); });
    _handler.registerCommand(CMD_EDIT_TAB2SPACE, [this]() { editTabToSpace(); });
    _handler.registerCommand(CMD_EDIT_SPACE2TAB_LEADING, [this]() { editSpaceToTabLeading(); });
    _handler.registerCommand(CMD_EDIT_SPACE2TAB_ALL, [this]() { editSpaceToTabAll(); });
}

void NppCommands::editUndo() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_UNDO);
    }
}

void NppCommands::editRedo() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_REDO);
    }
}

void NppCommands::editCut() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        if (view->hasSelection()) {
            view->execute(SCI_CUT);
        } else if (NppParameters::getInstance().getSVP()._lineCopyCutWithoutSelection) {
            // Cut the entire line with EOL
            view->execute(SCI_COPYALLOWLINE);
            view->execute(SCI_LINEDELETE);
        }
    }
}

void NppCommands::editCopy() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        if (view->hasSelection()) {
            view->execute(SCI_COPY);
        } else if (NppParameters::getInstance().getSVP()._lineCopyCutWithoutSelection) {
            // Copy the entire line with EOL
            view->execute(SCI_COPYALLOWLINE);
        }
    }
}

void NppCommands::editPaste() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_PASTE);
    }
}

void NppCommands::editDelete() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_CLEAR);
    }
}

void NppCommands::editSelectAll() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_SELECTALL);
    }
}

void NppCommands::editBeginEndSelect(bool columnMode) {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->beginOrEndSelect(columnMode);
    }
}

void NppCommands::editInsertTab() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        size_t selStartPos = view->execute(SCI_GETSELECTIONSTART);
        size_t lineNumber = view->execute(SCI_LINEFROMPOSITION, selStartPos);
        size_t nbSelections = view->execute(SCI_GETSELECTIONS);
        size_t selEndPos = view->execute(SCI_GETSELECTIONEND);
        size_t selEndLineNumber = view->execute(SCI_LINEFROMPOSITION, selEndPos);

        if ((nbSelections > 1) || (lineNumber != selEndLineNumber)) {
            // Multiple-selection or multi-line selection
            view->execute(SCI_TAB);
        } else {
            // Single line - adjust indentation
            size_t currentIndent = view->execute(SCI_GETLINEINDENTATION, lineNumber);
            intptr_t indentDelta = view->execute(SCI_GETTABWIDTH);
            view->setLineIndent(lineNumber, static_cast<intptr_t>(currentIndent) + indentDelta);
        }
    }
}

void NppCommands::editRemoveTab() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        size_t selStartPos = view->execute(SCI_GETSELECTIONSTART);
        size_t lineNumber = view->execute(SCI_LINEFROMPOSITION, selStartPos);
        size_t nbSelections = view->execute(SCI_GETSELECTIONS);
        size_t selEndPos = view->execute(SCI_GETSELECTIONEND);
        size_t selEndLineNumber = view->execute(SCI_LINEFROMPOSITION, selEndPos);

        if ((nbSelections > 1) || (lineNumber != selEndLineNumber)) {
            // Multiple-selection or multi-line selection
            view->execute(SCI_BACKTAB);
        } else {
            // Single line - adjust indentation
            size_t currentIndent = view->execute(SCI_GETLINEINDENTATION, lineNumber);
            intptr_t indentDelta = view->execute(SCI_GETTABWIDTH);
            view->setLineIndent(lineNumber, static_cast<intptr_t>(currentIndent) - indentDelta);
        }
    }
}

void NppCommands::editDuplicateLine() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_LINEDUPLICATE);
    }
}

void NppCommands::editRemoveDuplicateLines() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_BEGINUNDOACTION);
        // Call remove duplicate lines function
        // This would need to be implemented or call existing function
        view->execute(SCI_ENDUNDOACTION);
    }
}

void NppCommands::editRemoveAnyDuplicateLines() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_BEGINUNDOACTION);
        view->removeAnyDuplicateLines();
        view->execute(SCI_ENDUNDOACTION);
    }
}

void NppCommands::editTransposeLine() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_LINETRANSPOSE);
    }
}

void NppCommands::editSplitLines() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        if (view->execute(SCI_GETSELECTIONS) == 1) {
            // Get selection line range
            size_t selStart = view->execute(SCI_GETSELECTIONSTART);
            size_t selEnd = view->execute(SCI_GETSELECTIONEND);
            size_t startLine = view->execute(SCI_LINEFROMPOSITION, selStart);
            size_t endLine = view->execute(SCI_LINEFROMPOSITION, selEnd);

            auto anchorPos = view->execute(SCI_POSITIONFROMLINE, startLine);
            auto caretPos = view->execute(SCI_GETLINEENDPOSITION, endLine);
            view->execute(SCI_SETSELECTION, caretPos, anchorPos);
            view->execute(SCI_TARGETFROMSELECTION);

            size_t edgeMode = view->execute(SCI_GETEDGEMODE);
            if (edgeMode == EDGE_NONE) {
                view->execute(SCI_LINESSPLIT, 0);
            } else {
                auto textWidth = view->execute(SCI_TEXTWIDTH, STYLE_DEFAULT, reinterpret_cast<sptr_t>("P"));
                auto edgeCol = view->execute(SCI_GETEDGECOLUMN);
                if (edgeMode == EDGE_MULTILINE) {
                    NppParameters& nppParam = NppParameters::getInstance();
                    ScintillaViewParams& svp = const_cast<ScintillaViewParams&>(nppParam.getSVP());
                    edgeCol = svp._edgeMultiColumnPos.back();
                }
                ++edgeCol;
                view->execute(SCI_LINESSPLIT, textWidth * edgeCol);
            }
        }
    }
}

void NppCommands::editJoinLines() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        size_t selStart = view->execute(SCI_GETSELECTIONSTART);
        size_t selEnd = view->execute(SCI_GETSELECTIONEND);
        size_t startLine = view->execute(SCI_LINEFROMPOSITION, selStart);
        size_t endLine = view->execute(SCI_LINEFROMPOSITION, selEnd);

        if (startLine != endLine) {
            auto anchorPos = view->execute(SCI_POSITIONFROMLINE, startLine);
            auto caretPos = view->execute(SCI_GETLINEENDPOSITION, endLine);
            view->execute(SCI_SETSELECTION, caretPos, anchorPos);
            view->execute(SCI_TARGETFROMSELECTION);
            view->execute(SCI_LINESJOIN);
        }
    }
}

void NppCommands::editLineUp() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->currentLinesUp();
    }
}

void NppCommands::editLineDown() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->currentLinesDown();
    }
}

void NppCommands::editUpperCase() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->convertSelectedTextToUpperCase();
    }
}

void NppCommands::editLowerCase() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->convertSelectedTextToLowerCase();
    }
}

void NppCommands::editToggleComment() {
    if (_pNotepad_plus) {
        _pNotepad_plus->doBlockComment(Notepad_plus::cm_toggle);
    }
}

void NppCommands::editBlockComment() {
    if (_pNotepad_plus) {
        _pNotepad_plus->doBlockComment(Notepad_plus::cm_toggle);
    }
}

void NppCommands::editBlockCommentSet() {
    if (_pNotepad_plus) {
        _pNotepad_plus->doBlockComment(Notepad_plus::cm_comment);
    }
}

void NppCommands::editBlockUncomment() {
    if (_pNotepad_plus) {
        _pNotepad_plus->doBlockComment(Notepad_plus::cm_uncomment);
    }
}

void NppCommands::editStreamComment() {
    if (_pNotepad_plus) {
        _pNotepad_plus->doStreamComment();
    }
}

void NppCommands::editStreamUncomment() {
    if (_pNotepad_plus) {
        _pNotepad_plus->undoStreamComment();
    }
}

void NppCommands::editTrimTrailing() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_BEGINUNDOACTION);
        // Call trim trailing function
        view->execute(SCI_ENDUNDOACTION);
    }
}

void NppCommands::editTrimLineHead() {
    // Trim leading whitespace
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_BEGINUNDOACTION);
        // Call trim leading function
        view->execute(SCI_ENDUNDOACTION);
    }
}

void NppCommands::editTrimBoth() {
    // Trim both leading and trailing whitespace
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_BEGINUNDOACTION);
        // Call trim both function
        view->execute(SCI_ENDUNDOACTION);
    }
}

void NppCommands::editTabToSpace() {
    if (_pNotepad_plus) {
        // Convert tabs to spaces
    }
}

void NppCommands::editSpaceToTabLeading() {
    if (_pNotepad_plus) {
        // Convert leading spaces to tabs
    }
}

void NppCommands::editSpaceToTabAll() {
    if (_pNotepad_plus) {
        // Convert all spaces to tabs
    }
}

void NppCommands::editToggleReadOnly() {
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            buf->setUserReadOnly(!buf->getUserReadOnly());
        }
    }
}

void NppCommands::editToggleSystemReadOnly() {
    if (_pNotepad_plus) {
        // Toggle system read-only attribute
    }
}

void NppCommands::editSetReadOnlyForAllDocs() {
    if (_pNotepad_plus) {
        _pNotepad_plus->changeReadOnlyUserModeForAllOpenedTabs(true);
    }
}

void NppCommands::editClearReadOnlyForAllDocs() {
    if (_pNotepad_plus) {
        _pNotepad_plus->changeReadOnlyUserModeForAllOpenedTabs(false);
    }
}

void NppCommands::editFullPathToClipboard() {
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(QString::fromStdWString(buf->getFullPathName()));
        }
    }
}

void NppCommands::editFileNameToClipboard() {
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(QString::fromStdWString(buf->getFileName()));
        }
    }
}

void NppCommands::editCurrentDirToClipboard() {
    if (_pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf) {
            std::wstring path = buf->getFullPathName();
            size_t lastSlash = path.find_last_of(L"/\\");
            if (lastSlash != std::wstring::npos) {
                std::wstring dir = path.substr(0, lastSlash);
                QClipboard* clipboard = QApplication::clipboard();
                clipboard->setText(QString::fromStdWString(dir));
            }
        }
    }
}

void NppCommands::editCopyAllNames() {
    // Copy all file names to clipboard
}

void NppCommands::editCopyAllPaths() {
    // Copy all file paths to clipboard
}

void NppCommands::editColumnMode() {
    if (_pNotepad_plus) {
        // Show column editor dialog
    }
}

void NppCommands::editColumnModeTip() {
    // Show column mode tip dialog
    QMessageBox::information(nullptr, QObject::tr("Column Mode Tip"),
        QObject::tr("There are 3 ways to switch to column-select mode:\n\n"
                   "1. (Keyboard and Mouse) Hold Alt while left-click dragging\n\n"
                   "2. (Keyboard only) Hold Alt+Shift while using arrow keys\n\n"
                   "3. (Keyboard or Mouse)\n"
                   "      Put caret at desired start of column block position, then\n"
                   "       execute \"Begin/End Select in Column Mode\" command;\n"
                   "      Move caret to desired end of column block position, then\n"
                   "       execute \"Begin/End Select in Column Mode\" command again\n"));
}

// ============================================================================
// Search Commands Registration
// ============================================================================

void NppCommands::registerSearchCommands() {
    _handler.registerCommand(CMD_SEARCH_FIND, [this]() { searchFind(); });
    _handler.registerCommand(CMD_SEARCH_REPLACE, [this]() { searchReplace(); });
    _handler.registerCommand(CMD_SEARCH_MARK, [this]() { searchMark(); });
    _handler.registerCommand(CMD_SEARCH_FINDNEXT, [this]() { searchFindNext(); });
    _handler.registerCommand(CMD_SEARCH_FINDPREV, [this]() { searchFindPrev(); });
    _handler.registerCommand(CMD_SEARCH_VOLATILE_FINDNEXT, [this]() { searchFindNextVolatile(); });
    _handler.registerCommand(CMD_SEARCH_VOLATILE_FINDPREV, [this]() { searchFindPrevVolatile(); });
    _handler.registerCommand(CMD_SEARCH_FINDINFILES, [this]() { searchFindInFiles(); });
    _handler.registerCommand(CMD_SEARCH_FINDINCREMENT, [this]() { searchFindIncrement(); });
    _handler.registerCommand(CMD_SEARCH_SETANDFINDNEXT, [this]() { searchSetAndFindNext(); });
    _handler.registerCommand(CMD_SEARCH_SETANDFINDPREV, [this]() { searchSetAndFindPrev(); });
    _handler.registerCommand(CMD_SEARCH_GOTONEXTFOUND, [this]() { searchGoToNextFound(); });
    _handler.registerCommand(CMD_SEARCH_GOTOPREVFOUND, [this]() { searchGoToPrevFound(); });
    _handler.registerCommand(CMD_SEARCH_GOTOLINE, [this]() { searchGoToLine(); });
    _handler.registerCommand(CMD_SEARCH_GOTOMATCHINGBRACE, [this]() { searchGoToMatchingBrace(); });
    _handler.registerCommand(CMD_SEARCH_SELECTMATCHINGBRACES, [this]() { searchSelectMatchingBraces(); });
    _handler.registerCommand(CMD_SEARCH_TOGGLE_BOOKMARK, [this]() { searchToggleBookmark(); });
    _handler.registerCommand(CMD_SEARCH_NEXT_BOOKMARK, [this]() { searchNextBookmark(); });
    _handler.registerCommand(CMD_SEARCH_PREV_BOOKMARK, [this]() { searchPrevBookmark(); });
    _handler.registerCommand(CMD_SEARCH_CLEAR_BOOKMARKS, [this]() { searchClearBookmarks(); });
    _handler.registerCommand(CMD_SEARCH_CUTMARKEDLINES, [this]() { searchCutMarkedLines(); });
    _handler.registerCommand(CMD_SEARCH_COPYMARKEDLINES, [this]() { searchCopyMarkedLines(); });
    _handler.registerCommand(CMD_SEARCH_PASTEMARKEDLINES, [this]() { searchPasteMarkedLines(); });
    _handler.registerCommand(CMD_SEARCH_DELETEMARKEDLINES, [this]() { searchDeleteMarkedLines(); });
    _handler.registerCommand(CMD_SEARCH_DELETEUNMARKEDLINES, [this]() { searchDeleteUnmarkedLines(); });
    _handler.registerCommand(CMD_SEARCH_INVERSEMARKS, [this]() { searchInverseMarks(); });
    _handler.registerCommand(CMD_SEARCH_CLEARALLMARKS, [this]() { searchClearAllMarks(); });
    _handler.registerCommand(CMD_SEARCH_FINDCHARINRANGE, [this]() { searchFindCharInRange(); });

    // Mark style commands
    _handler.registerCommand(CMD_SEARCH_MARKALLEXT1, [this]() { searchMarkAllExt(SCE_UNIVERSAL_FOUND_STYLE_EXT1); });
    _handler.registerCommand(CMD_SEARCH_MARKALLEXT2, [this]() { searchMarkAllExt(SCE_UNIVERSAL_FOUND_STYLE_EXT2); });
    _handler.registerCommand(CMD_SEARCH_MARKALLEXT3, [this]() { searchMarkAllExt(SCE_UNIVERSAL_FOUND_STYLE_EXT3); });
    _handler.registerCommand(CMD_SEARCH_MARKALLEXT4, [this]() { searchMarkAllExt(SCE_UNIVERSAL_FOUND_STYLE_EXT4); });
    _handler.registerCommand(CMD_SEARCH_MARKALLEXT5, [this]() { searchMarkAllExt(SCE_UNIVERSAL_FOUND_STYLE_EXT5); });
    _handler.registerCommand(CMD_SEARCH_UNMARKALLEXT1, [this]() { searchUnmarkAllExt(SCE_UNIVERSAL_FOUND_STYLE_EXT1); });
    _handler.registerCommand(CMD_SEARCH_UNMARKALLEXT2, [this]() { searchUnmarkAllExt(SCE_UNIVERSAL_FOUND_STYLE_EXT2); });
    _handler.registerCommand(CMD_SEARCH_UNMARKALLEXT3, [this]() { searchUnmarkAllExt(SCE_UNIVERSAL_FOUND_STYLE_EXT3); });
    _handler.registerCommand(CMD_SEARCH_UNMARKALLEXT4, [this]() { searchUnmarkAllExt(SCE_UNIVERSAL_FOUND_STYLE_EXT4); });
    _handler.registerCommand(CMD_SEARCH_UNMARKALLEXT5, [this]() { searchUnmarkAllExt(SCE_UNIVERSAL_FOUND_STYLE_EXT5); });

    // Jump marker commands
    _handler.registerCommand(CMD_SEARCH_GONEXTMARKER1, [this]() { searchGoNextMarker(SCE_UNIVERSAL_FOUND_STYLE_EXT1); });
    _handler.registerCommand(CMD_SEARCH_GONEXTMARKER2, [this]() { searchGoNextMarker(SCE_UNIVERSAL_FOUND_STYLE_EXT2); });
    _handler.registerCommand(CMD_SEARCH_GONEXTMARKER3, [this]() { searchGoNextMarker(SCE_UNIVERSAL_FOUND_STYLE_EXT3); });
    _handler.registerCommand(CMD_SEARCH_GONEXTMARKER4, [this]() { searchGoNextMarker(SCE_UNIVERSAL_FOUND_STYLE_EXT4); });
    _handler.registerCommand(CMD_SEARCH_GONEXTMARKER5, [this]() { searchGoNextMarker(SCE_UNIVERSAL_FOUND_STYLE_EXT5); });
    _handler.registerCommand(CMD_SEARCH_GONEXTMARKER_DEF, [this]() { searchGoNextMarker(SCE_UNIVERSAL_FOUND_STYLE); });
    _handler.registerCommand(CMD_SEARCH_GOPREVMARKER1, [this]() { searchGoPrevMarker(SCE_UNIVERSAL_FOUND_STYLE_EXT1); });
    _handler.registerCommand(CMD_SEARCH_GOPREVMARKER2, [this]() { searchGoPrevMarker(SCE_UNIVERSAL_FOUND_STYLE_EXT2); });
    _handler.registerCommand(CMD_SEARCH_GOPREVMARKER3, [this]() { searchGoPrevMarker(SCE_UNIVERSAL_FOUND_STYLE_EXT3); });
    _handler.registerCommand(CMD_SEARCH_GOPREVMARKER4, [this]() { searchGoPrevMarker(SCE_UNIVERSAL_FOUND_STYLE_EXT4); });
    _handler.registerCommand(CMD_SEARCH_GOPREVMARKER5, [this]() { searchGoPrevMarker(SCE_UNIVERSAL_FOUND_STYLE_EXT5); });
    _handler.registerCommand(CMD_SEARCH_GOPREVMARKER_DEF, [this]() { searchGoPrevMarker(SCE_UNIVERSAL_FOUND_STYLE); });
}

void NppCommands::searchFind() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showFindReplaceDlg(FIND_DLG);
    }
}

void NppCommands::searchReplace() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showFindReplaceDlg(REPLACE_DLG);
    }
}

void NppCommands::searchMark() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showFindReplaceDlg(MARK_DLG);
    }
}

void NppCommands::searchFindNext() {
    if (_pNotepad_plus) {
        _pNotepad_plus->findNext(DIR_DOWN);
    }
}

void NppCommands::searchFindPrev() {
    if (_pNotepad_plus) {
        _pNotepad_plus->findNext(DIR_UP);
    }
}

void NppCommands::searchFindNextVolatile() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        auto str = view->getSelectedTextToWChar();
        if (str.empty()) return;

        FindOption op;
        op._isMatchCase = false;
        op._isWholeWord = false;
        op._isWrapAround = true;
        op._searchType = FindNormal;
        op._whichDirection = DIR_DOWN;

        _pNotepad_plus->processFindNext(str.c_str(), &op);
    }
}

void NppCommands::searchFindPrevVolatile() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        auto str = view->getSelectedTextToWChar();
        if (str.empty()) return;

        FindOption op;
        op._isMatchCase = false;
        op._isWholeWord = false;
        op._isWrapAround = true;
        op._searchType = FindNormal;
        op._whichDirection = DIR_UP;

        _pNotepad_plus->processFindNext(str.c_str(), &op);
    }
}

void NppCommands::searchFindInFiles() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showFindReplaceDlg(FINDINFILES_DLG);
    }
}

void NppCommands::searchFindInProjects() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showFindReplaceDlg(FINDINPROJECTS_DLG);
    }
}

void NppCommands::searchFindIncrement() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showIncrementalFindDlg();
    }
}

void NppCommands::searchSetAndFindNext() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        auto str = view->getSelectedTextToWChar();
        if (str.empty()) return;

        _pNotepad_plus->setSearchText(str.c_str());

        FindOption op;
        op._searchType = FindNormal;
        op._whichDirection = DIR_DOWN;

        _pNotepad_plus->processFindNext(str.c_str(), &op);
    }
}

void NppCommands::searchSetAndFindPrev() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        auto str = view->getSelectedTextToWChar();
        if (str.empty()) return;

        _pNotepad_plus->setSearchText(str.c_str());

        FindOption op;
        op._searchType = FindNormal;
        op._whichDirection = DIR_UP;

        _pNotepad_plus->processFindNext(str.c_str(), &op);
    }
}

void NppCommands::searchGoToNextFound() {
    if (_pNotepad_plus) {
        _pNotepad_plus->gotoNextFoundResult();
    }
}

void NppCommands::searchGoToPrevFound() {
    if (_pNotepad_plus) {
        _pNotepad_plus->gotoNextFoundResult(-1);
    }
}

void NppCommands::searchGoToLine() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showGoToLineDlg();
    }
}

void NppCommands::searchGoToMatchingBrace() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        intptr_t braceAtCaret = -1;
        intptr_t braceOpposite = -1;
        _pNotepad_plus->findMatchingBracePos(braceAtCaret, braceOpposite);

        if (braceOpposite != -1) {
            view->execute(SCI_GOTOPOS, braceOpposite);
            view->execute(SCI_CHOOSECARETX);
        }
    }
}

void NppCommands::searchSelectMatchingBraces() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        intptr_t braceAtCaret = -1;
        intptr_t braceOpposite = -1;
        _pNotepad_plus->findMatchingBracePos(braceAtCaret, braceOpposite);

        if (braceOpposite != -1) {
            view->execute(SCI_SETSEL,
                std::min<intptr_t>(braceAtCaret, braceOpposite),
                std::max<intptr_t>(braceAtCaret, braceOpposite) + 1);
            view->execute(SCI_CHOOSECARETX);
        }
    }
}

void NppCommands::searchToggleBookmark() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        intptr_t lineno = view->getCurrentLineNumber();
        int state = static_cast<int>(view->execute(SCI_MARKERGET, lineno));
        if (state & (1 << MARK_BOOKMARK)) {
            view->execute(SCI_MARKERDELETE, lineno, MARK_BOOKMARK);
        } else {
            view->execute(SCI_MARKERADD, lineno, MARK_BOOKMARK);
        }
    }
}

void NppCommands::searchNextBookmark() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        intptr_t lineno = view->getCurrentLineNumber();
        intptr_t nextLine = view->execute(SCI_MARKERNEXT, lineno + 1, (1 << MARK_BOOKMARK));
        if (nextLine < 0) {
            nextLine = view->execute(SCI_MARKERNEXT, 0, (1 << MARK_BOOKMARK));
        }
        if (nextLine >= 0) {
            view->execute(SCI_GOTOLINE, nextLine);
        }
    }
}

void NppCommands::searchPrevBookmark() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        intptr_t lineno = view->getCurrentLineNumber();
        intptr_t prevLine = view->execute(SCI_MARKERPREVIOUS, lineno - 1, (1 << MARK_BOOKMARK));
        if (prevLine < 0) {
            intptr_t lastLine = view->execute(SCI_GETLINECOUNT) - 1;
            prevLine = view->execute(SCI_MARKERPREVIOUS, lastLine, (1 << MARK_BOOKMARK));
        }
        if (prevLine >= 0) {
            view->execute(SCI_GOTOLINE, prevLine);
        }
    }
}

void NppCommands::searchClearBookmarks() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_MARKERDELETEALL, MARK_BOOKMARK);
    }
}

void NppCommands::searchCutMarkedLines() {
    if (_pNotepad_plus) {
        _pNotepad_plus->cutMarkedLines();
    }
}

void NppCommands::searchCopyMarkedLines() {
    if (_pNotepad_plus) {
        _pNotepad_plus->copyMarkedLines();
    }
}

void NppCommands::searchPasteMarkedLines() {
    if (_pNotepad_plus) {
        _pNotepad_plus->pasteToMarkedLines();
    }
}

void NppCommands::searchDeleteMarkedLines() {
    if (_pNotepad_plus) {
        _pNotepad_plus->deleteMarkedLines(true);
    }
}

void NppCommands::searchDeleteUnmarkedLines() {
    if (_pNotepad_plus) {
        _pNotepad_plus->deleteMarkedLines(false);
    }
}

void NppCommands::searchInverseMarks() {
    if (_pNotepad_plus) {
        _pNotepad_plus->inverseMarks();
    }
}

void NppCommands::searchMarkAllExt(int styleID) {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        auto selectedText = view->getSelectedTextToWChar(true);
        if (!selectedText.empty()) {
            _pNotepad_plus->markAll(selectedText.c_str(), styleID);
        }
    }
}

void NppCommands::searchUnmarkAllExt(int styleID) {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->clearIndicator(styleID);
    }
}

void NppCommands::searchMarkOneExt(int styleID) {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        Sci_CharacterRangeFull range;
        range.cpMin = static_cast<Sci_PositionCR>(view->execute(SCI_GETSELECTIONSTART));
        range.cpMax = static_cast<Sci_PositionCR>(view->execute(SCI_GETSELECTIONEND));

        if (range.cpMin == range.cpMax) {
            auto caretPos = view->execute(SCI_GETCURRENTPOS);
            range.cpMin = static_cast<Sci_PositionCR>(view->execute(SCI_WORDSTARTPOSITION, caretPos, true));
            range.cpMax = static_cast<Sci_PositionCR>(view->execute(SCI_WORDENDPOSITION, caretPos, true));
        }

        if (range.cpMax > range.cpMin) {
            view->execute(SCI_SETINDICATORCURRENT, styleID);
            view->execute(SCI_INDICATORFILLRANGE, range.cpMin, range.cpMax - range.cpMin);
        }
    }
}

void NppCommands::searchClearAllMarks() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE_EXT1);
        view->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE_EXT2);
        view->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE_EXT3);
        view->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE_EXT4);
        view->clearIndicator(SCE_UNIVERSAL_FOUND_STYLE_EXT5);
    }
}

void NppCommands::searchGoNextMarker(int styleID) {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        _pNotepad_plus->goToNextIndicator(styleID);
    }
}

void NppCommands::searchGoPrevMarker(int styleID) {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        _pNotepad_plus->goToPreviousIndicator(styleID);
    }
}

void NppCommands::searchFindCharInRange() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showFindCharsInRangeDlg();
    }
}

void NppCommands::searchChangedNext() {
    // Navigate to next changed line
}

void NppCommands::searchChangedPrev() {
    // Navigate to previous changed line
}

void NppCommands::searchClearChangeHistory() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_EMPTYUNDOBUFFER);
    }
}

// ============================================================================
// View Commands Registration
// ============================================================================

void NppCommands::registerViewCommands() {
    _handler.registerCommand(CMD_VIEW_FULLSCREENTOGGLE, [this]() { viewFullScreen(); });
    _handler.registerCommand(CMD_VIEW_POSTIT, [this]() { viewPostIt(); });
    _handler.registerCommand(CMD_VIEW_DISTRACTIONFREE, [this]() { viewDistractionFree(); });
    _handler.registerCommand(CMD_VIEW_ALWAYSONTOP, [this]() { viewAlwaysOnTop(); });
    _handler.registerCommand(CMD_VIEW_WRAP, [this]() { viewWordWrap(); });
    _handler.registerCommand(CMD_VIEW_WRAP_SYMBOL, [this]() { viewWrapSymbol(); });
    _handler.registerCommand(CMD_VIEW_HIDELINES, [this]() { viewHideLines(); });
    _handler.registerCommand(CMD_VIEW_ZOOMIN, [this]() { viewZoomIn(); });
    _handler.registerCommand(CMD_VIEW_ZOOMOUT, [this]() { viewZoomOut(); });
    _handler.registerCommand(CMD_VIEW_ZOOMRESTORE, [this]() { viewZoomRestore(); });
    _handler.registerCommand(CMD_VIEW_INDENT_GUIDE, [this]() { viewIndentGuide(); });
    _handler.registerCommand(CMD_VIEW_TAB_SPACE, [this]() { viewShowWhiteSpace(); });
    _handler.registerCommand(CMD_VIEW_EOL, [this]() { viewShowEOL(); });
    _handler.registerCommand(CMD_VIEW_ALL_CHARACTERS, [this]() { viewShowAllCharacters(); });
    _handler.registerCommand(CMD_VIEW_SUMMARY, [this]() { viewSummary(); });
    _handler.registerCommand(CMD_VIEW_MONITORING, [this]() { viewMonitoring(); });
    _handler.registerCommand(CMD_VIEW_SYNSCROLLV, [this]() { viewSyncScrollV(); });
    _handler.registerCommand(CMD_VIEW_SYNSCROLLH, [this]() { viewSyncScrollH(); });

    // Fold commands
    _handler.registerCommand(CMD_VIEW_FOLDALL, [this]() { viewFoldAll(); });
    _handler.registerCommand(CMD_VIEW_UNFOLDALL, [this]() { viewUnfoldAll(); });
    _handler.registerCommand(CMD_VIEW_FOLD_CURRENT, [this]() { viewFoldCurrent(); });
    _handler.registerCommand(CMD_VIEW_UNFOLD_CURRENT, [this]() { viewUnfoldCurrent(); });
    _handler.registerCommand(CMD_VIEW_FOLD_1, [this]() { viewFoldLevel(0); });
    _handler.registerCommand(CMD_VIEW_FOLD_2, [this]() { viewFoldLevel(1); });
    _handler.registerCommand(CMD_VIEW_FOLD_3, [this]() { viewFoldLevel(2); });
    _handler.registerCommand(CMD_VIEW_FOLD_4, [this]() { viewFoldLevel(3); });
    _handler.registerCommand(CMD_VIEW_FOLD_5, [this]() { viewFoldLevel(4); });
    _handler.registerCommand(CMD_VIEW_FOLD_6, [this]() { viewFoldLevel(5); });
    _handler.registerCommand(CMD_VIEW_FOLD_7, [this]() { viewFoldLevel(6); });
    _handler.registerCommand(CMD_VIEW_FOLD_8, [this]() { viewFoldLevel(7); });
    _handler.registerCommand(CMD_VIEW_UNFOLD_1, [this]() { viewUnfoldLevel(0); });
    _handler.registerCommand(CMD_VIEW_UNFOLD_2, [this]() { viewUnfoldLevel(1); });
    _handler.registerCommand(CMD_VIEW_UNFOLD_3, [this]() { viewUnfoldLevel(2); });
    _handler.registerCommand(CMD_VIEW_UNFOLD_4, [this]() { viewUnfoldLevel(3); });
    _handler.registerCommand(CMD_VIEW_UNFOLD_5, [this]() { viewUnfoldLevel(4); });
    _handler.registerCommand(CMD_VIEW_UNFOLD_6, [this]() { viewUnfoldLevel(5); });
    _handler.registerCommand(CMD_VIEW_UNFOLD_7, [this]() { viewUnfoldLevel(6); });
    _handler.registerCommand(CMD_VIEW_UNFOLD_8, [this]() { viewUnfoldLevel(7); });

    // Panel commands
    _handler.registerCommand(CMD_VIEW_DOCLIST, [this]() { viewDocumentList(); });
    _handler.registerCommand(CMD_VIEW_DOC_MAP, [this]() { viewDocumentMap(); });
    _handler.registerCommand(CMD_VIEW_FUNC_LIST, [this]() { viewFunctionList(); });
    _handler.registerCommand(CMD_VIEW_FILEBROWSER, [this]() { viewFileBrowser(); });
    _handler.registerCommand(CMD_VIEW_PROJECT_PANEL_1, [this]() { viewProjectPanel(0); });
    _handler.registerCommand(CMD_VIEW_PROJECT_PANEL_2, [this]() { viewProjectPanel(1); });
    _handler.registerCommand(CMD_VIEW_PROJECT_PANEL_3, [this]() { viewProjectPanel(2); });
    _handler.registerCommand(CMD_VIEW_SWITCHTO_PROJECT_PANEL_1, [this]() { viewSwitchToProjectPanel(0); });
    _handler.registerCommand(CMD_VIEW_SWITCHTO_PROJECT_PANEL_2, [this]() { viewSwitchToProjectPanel(1); });
    _handler.registerCommand(CMD_VIEW_SWITCHTO_PROJECT_PANEL_3, [this]() { viewSwitchToProjectPanel(2); });
    _handler.registerCommand(CMD_VIEW_SWITCHTO_FILEBROWSER, [this]() { viewSwitchToFileBrowser(); });
    _handler.registerCommand(CMD_VIEW_SWITCHTO_FUNC_LIST, [this]() { viewSwitchToFuncList(); });
    _handler.registerCommand(CMD_VIEW_SWITCHTO_DOCLIST, [this]() { viewSwitchToDocList(); });
    _handler.registerCommand(CMD_VIEW_SWITCHTO_OTHER_VIEW, [this]() { viewSwitchToOtherView(); });

    // Tab commands
    _handler.registerCommand(CMD_VIEW_TAB1, [this]() { viewTab(0); });
    _handler.registerCommand(CMD_VIEW_TAB2, [this]() { viewTab(1); });
    _handler.registerCommand(CMD_VIEW_TAB3, [this]() { viewTab(2); });
    _handler.registerCommand(CMD_VIEW_TAB4, [this]() { viewTab(3); });
    _handler.registerCommand(CMD_VIEW_TAB5, [this]() { viewTab(4); });
    _handler.registerCommand(CMD_VIEW_TAB6, [this]() { viewTab(5); });
    _handler.registerCommand(CMD_VIEW_TAB7, [this]() { viewTab(6); });
    _handler.registerCommand(CMD_VIEW_TAB8, [this]() { viewTab(7); });
    _handler.registerCommand(CMD_VIEW_TAB9, [this]() { viewTab(8); });
    _handler.registerCommand(CMD_VIEW_TAB_NEXT, [this]() { viewTabNext(); });
    _handler.registerCommand(CMD_VIEW_TAB_PREV, [this]() { viewTabPrev(); });
    _handler.registerCommand(CMD_VIEW_TAB_START, [this]() { viewTabStart(); });
    _handler.registerCommand(CMD_VIEW_TAB_END, [this]() { viewTabEnd(); });
    _handler.registerCommand(CMD_VIEW_TAB_MOVEFORWARD, [this]() { viewTabMoveForward(); });
    _handler.registerCommand(CMD_VIEW_TAB_MOVEBACKWARD, [this]() { viewTabMoveBackward(); });
}

void NppCommands::viewFullScreen() {
    if (_pNotepad_plus) {
        _pNotepad_plus->fullScreenToggle();
    }
}

void NppCommands::viewPostIt() {
    if (_pNotepad_plus) {
        _pNotepad_plus->postItToggle();
    }
}

void NppCommands::viewDistractionFree() {
    if (_pNotepad_plus) {
        _pNotepad_plus->distractionFreeToggle();
    }
}

void NppCommands::viewAlwaysOnTop() {
    // Toggle always on top window flag
}

void NppCommands::viewWordWrap() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        bool isWrapped = !view->isWrap();
        _pNotepad_plus->wrapAllEditors(isWrapped);
    }
}

void NppCommands::viewWrapSymbol() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        bool visible = !view->isWrapSymbolVisible();
        view->showWrapSymbol(visible);
    }
}

void NppCommands::viewHideLines() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->hideLines();
    }
}

void NppCommands::viewZoomIn() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_ZOOMIN);
    }
}

void NppCommands::viewZoomOut() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_ZOOMOUT);
    }
}

void NppCommands::viewZoomRestore() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->execute(SCI_SETZOOM, 0);
    }
}

void NppCommands::viewIndentGuide() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        bool show = !view->isShownIndentGuide();
        _pNotepad_plus->showIndentGuide(show);
    }
}

void NppCommands::viewShowWhiteSpace() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showWhiteSpace(!_pNotepad_plus->isWhiteSpaceShown());
    }
}

void NppCommands::viewShowEOL() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showEOL(!_pNotepad_plus->isEOLShown());
    }
}

void NppCommands::viewShowAllCharacters() {
    if (_pNotepad_plus) {
        bool show = !_pNotepad_plus->isAllCharactersShown();
        _pNotepad_plus->showInvisibleChars(show);
    }
}

void NppCommands::viewShowNpc() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showNpc(!_pNotepad_plus->isNpcShown());
    }
}

void NppCommands::viewShowNpcCcUniEol() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showCcUniEol(!_pNotepad_plus->isCcUniEolShown());
    }
}

void NppCommands::viewSyncScrollV() {
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleSyncScrollV();
    }
}

void NppCommands::viewSyncScrollH() {
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleSyncScrollH();
    }
}

void NppCommands::viewSummary() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showSummary();
    }
}

void NppCommands::viewMonitoring() {
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleMonitoring();
    }
}

void NppCommands::viewFoldAll() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->foldAll(fold_collapse);
    }
}

void NppCommands::viewUnfoldAll() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->foldAll(fold_expand);
    }
}

void NppCommands::viewFoldCurrent() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->foldCurrentPos(fold_collapse);
    }
}

void NppCommands::viewUnfoldCurrent() {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->foldCurrentPos(fold_expand);
    }
}

void NppCommands::viewFoldLevel(int level) {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->foldLevel(level, fold_collapse);
    }
}

void NppCommands::viewUnfoldLevel(int level) {
    ScintillaEditView* view = getCurrentEditView();
    if (view) {
        view->foldLevel(level, fold_expand);
    }
}

void NppCommands::viewDocumentList() {
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleDocumentList();
    }
}

void NppCommands::viewDocumentMap() {
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleDocumentMap();
    }
}

void NppCommands::viewFunctionList() {
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleFunctionList();
    }
}

void NppCommands::viewFileBrowser() {
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleFileBrowser();
    }
}

void NppCommands::viewProjectPanel(int index) {
    if (_pNotepad_plus) {
        _pNotepad_plus->toggleProjectPanel(index);
    }
}

void NppCommands::viewSwitchToProjectPanel(int index) {
    if (_pNotepad_plus) {
        _pNotepad_plus->switchToProjectPanel(index);
    }
}

void NppCommands::viewSwitchToFileBrowser() {
    if (_pNotepad_plus) {
        _pNotepad_plus->switchToFileBrowser();
    }
}

void NppCommands::viewSwitchToFuncList() {
    if (_pNotepad_plus) {
        _pNotepad_plus->switchToFunctionList();
    }
}

void NppCommands::viewSwitchToDocList() {
    if (_pNotepad_plus) {
        _pNotepad_plus->switchToDocumentList();
    }
}

void NppCommands::viewSwitchToOtherView() {
    if (_pNotepad_plus) {
        _pNotepad_plus->switchEditViewTo(_pNotepad_plus->otherView());
    }
}

void NppCommands::viewTab(int index) {
    if (_pNotepad_plus) {
        _pNotepad_plus->activateDoc(index);
    }
}

void NppCommands::viewTabNext() {
    if (_pNotepad_plus) {
        _pNotepad_plus->activateNextDoc(true);
    }
}

void NppCommands::viewTabPrev() {
    if (_pNotepad_plus) {
        _pNotepad_plus->activateNextDoc(false);
    }
}

void NppCommands::viewTabStart() {
    if (_pNotepad_plus) {
        _pNotepad_plus->activateDoc(0);
    }
}

void NppCommands::viewTabEnd() {
    if (_pNotepad_plus) {
        int count = _pNotepad_plus->getCurrentDocTab()->nbItem();
        _pNotepad_plus->activateDoc(count - 1);
    }
}

void NppCommands::viewTabMoveForward() {
    if (_pNotepad_plus) {
        _pNotepad_plus->moveTabForward();
    }
}

void NppCommands::viewTabMoveBackward() {
    if (_pNotepad_plus) {
        _pNotepad_plus->moveTabBackward();
    }
}

// ============================================================================
// Macro Commands Registration
// ============================================================================

void NppCommands::registerMacroCommands() {
    _handler.registerCommand(CMD_MACRO_STARTRECORDINGMACRO, [this]() { macroStartRecording(); });
    _handler.registerCommand(CMD_MACRO_STOPRECORDINGMACRO, [this]() { macroStopRecording(); });
    _handler.registerCommand(CMD_MACRO_PLAYBACKRECORDEDMACRO, [this]() { macroPlayback(); });
    _handler.registerCommand(CMD_MACRO_SAVECURRENTMACRO, [this]() { macroSaveCurrent(); });
    _handler.registerCommand(CMD_MACRO_RUNMULTIMACRODLG, [this]() { macroRunMultiMacroDlg(); });
}

void NppCommands::macroStartRecording() {
    if (_pNotepad_plus) {
        _pNotepad_plus->startMacroRecording();
    }
}

void NppCommands::macroStopRecording() {
    if (_pNotepad_plus) {
        _pNotepad_plus->stopMacroRecording();
    }
}

void NppCommands::macroPlayback() {
    if (_pNotepad_plus) {
        _pNotepad_plus->macroPlayback();
    }
}

void NppCommands::macroSaveCurrent() {
    if (_pNotepad_plus) {
        _pNotepad_plus->saveCurrentMacro();
    }
}

void NppCommands::macroRunMultiMacroDlg() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showRunMacroDlg();
    }
}

// ============================================================================
// Format Commands Registration
// ============================================================================

void NppCommands::registerFormatCommands() {
    _handler.registerCommand(CMD_FORMAT_TODOS, [this]() { formatConvertToWindows(); });
    _handler.registerCommand(CMD_FORMAT_TOUNIX, [this]() { formatConvertToUnix(); });
    _handler.registerCommand(CMD_FORMAT_TOMAC, [this]() { formatConvertToMac(); });
    _handler.registerCommand(CMD_FORMAT_ANSI, [this]() { formatSetEncoding(0); });
    _handler.registerCommand(CMD_FORMAT_UTF_8, [this]() { formatSetEncoding(1); });
    _handler.registerCommand(CMD_FORMAT_UTF_16BE, [this]() { formatSetEncoding(2); });
    _handler.registerCommand(CMD_FORMAT_UTF_16LE, [this]() { formatSetEncoding(3); });
    _handler.registerCommand(CMD_FORMAT_AS_UTF_8, [this]() { formatSetEncoding(4); });
}

void NppCommands::formatConvertToWindows() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf && !buf->isReadOnly()) {
            buf->setEolFormat(EolType::windows);
            view->execute(SCI_CONVERTEOLS, static_cast<WPARAM>(EolType::windows));
        }
    }
}

void NppCommands::formatConvertToUnix() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf && !buf->isReadOnly()) {
            buf->setEolFormat(EolType::unix);
            view->execute(SCI_CONVERTEOLS, static_cast<WPARAM>(EolType::unix));
        }
    }
}

void NppCommands::formatConvertToMac() {
    ScintillaEditView* view = getCurrentEditView();
    if (view && _pNotepad_plus) {
        Buffer* buf = _pNotepad_plus->getCurrentBuffer();
        if (buf && !buf->isReadOnly()) {
            buf->setEolFormat(EolType::macos);
            view->execute(SCI_CONVERTEOLS, static_cast<WPARAM>(EolType::macos));
        }
    }
}

void NppCommands::formatSetEncoding(int encoding) {
    if (_pNotepad_plus) {
        _pNotepad_plus->setEncoding(encoding);
    }
}

// ============================================================================
// Language Commands Registration
// ============================================================================

void NppCommands::registerLanguageCommands() {
    _handler.registerCommand(CMD_LANG_USER_DLG, [this]() { langUserDlg(); });
}

void NppCommands::langUserDlg() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showUserDefineDlg();
    }
}

// ============================================================================
// Run Commands Registration
// ============================================================================

void NppCommands::registerRunCommands() {
    _handler.registerCommand(CMD_EXECUTE, [this]() { executeRun(); });
}

void NppCommands::executeRun() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showRunDlg();
    }
}

// ============================================================================
// Settings Commands Registration
// ============================================================================

void NppCommands::registerSettingsCommands() {
    _handler.registerCommand(CMD_SETTING_PREFERENCE, [this]() { settingPreference(); });
}

void NppCommands::settingPreference() {
    if (_pNotepad_plus) {
        _pNotepad_plus->showPreferenceDlg();
    }
}

} // namespace QtCommands
