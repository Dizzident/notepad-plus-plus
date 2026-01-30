// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// ============================================================================
// Qt/Linux Implementation of Notepad_plus file operations
// ============================================================================
// This file provides Linux/Qt implementations of the Notepad_plus class methods
// that are implemented in NppIO.cpp on Windows.

// This file contains Qt/Linux-specific implementations
// These implementations use QtCore::Buffer and QtCore::BufferManager
#ifdef NPP_LINUX

#include "Notepad_plus.h"
#include "NppIO.h"
#include "Buffer.h"
#include "ScintillaEditView.h"
// Note: DocTabView.h is already included via Notepad_plus.h
#include "Parameters.h"
#include "lastRecentFileList.h"
#include "MISC/Common/LinuxTypes.h"
#include "Platform/Clipboard.h"
#include "menuCmdID.h"

// Panel headers needed for destructor
#include "ClipboardHistory/ClipboardHistoryPanel.h"
#include "ProjectPanel/ProjectPanel.h"
#include "DocumentMap/DocumentMap.h"
#include "FunctionList/FunctionListPanel.h"
#include "FileBrowser/FileBrowser.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QAbstractButton>
#include <QClipboard>
#include <QApplication>
#include <memory>
#include <mutex>
#include <chrono>

// ============================================================================
// Global Variables
// ============================================================================

std::chrono::steady_clock::time_point g_nppStartTimePoint{};
std::chrono::steady_clock::duration g_pluginsLoadingTime{};

// ============================================================================
// Constructor and Destructor
// ============================================================================

Notepad_plus::Notepad_plus()
	: _autoCompleteMain(&_mainEditView)
	, _autoCompleteSub(&_subEditView)
	, _smartHighlighter(&_findReplaceDlg)
{
	// Initialize member variables that need explicit initialization
	memset(&_prevSelectedRange, 0, sizeof(_prevSelectedRange));

	NppParameters& nppParam = NppParameters::getInstance();
	NppXml::Document nativeLangDocRoot = nppParam.getNativeLang();
	_nativeLangSpeaker.init(nativeLangDocRoot);

	LocalizationSwitcher & localizationSwitcher = nppParam.getLocalizationSwitcher();
	const char *fn = _nativeLangSpeaker.getFileName();
	if (fn)
	{
		localizationSwitcher.setFileName(fn);
	}

	nppParam.setNativeLangSpeaker(&_nativeLangSpeaker);

	// Note: On Linux, admin mode detection is handled differently
	// For now, assume non-admin mode (can be enhanced later)
	nppParam.setAdminMode(false);
	_isAdministrator = false;
}

Notepad_plus::~Notepad_plus()
{
	// ATTENTION : the order of the destruction is very important
	// because if the parent's window handle is destroyed before
	// the destruction of its children windows' handles,
	// its children windows' handles will be destroyed automatically!

	(NppParameters::getInstance()).destroyInstance();

#ifndef NPP_LINUX
	delete _pTrayIco;
#endif
	delete _pAnsiCharPanel;
	delete _pClipboardHistoryPanel;
	delete _pDocumentListPanel;
	delete _pProjectPanel_1;
	delete _pProjectPanel_2;
	delete _pProjectPanel_3;
	delete _pDocMap;
	delete _pFuncList;
	delete _pFileBrowser;
}

// ============================================================================
// File Operations
// ============================================================================

void Notepad_plus::fileNew()
{
    BufferID newBufID = MainFileManager.newEmptyDocument();
    if (newBufID != BUFFER_INVALID)
    {
        loadBufferIntoView(newBufID, currentView(), true);
        switchToFile(newBufID);
    }
}

void Notepad_plus::fileOpen()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QStringList fileNames = QFileDialog::getOpenFileNames(
        nullptr,
        QObject::tr("Open"),
        dir,
        QObject::tr("All Files (*);;Text Files (*.txt)"));

    BufferID lastOpened = BUFFER_INVALID;
    for (const QString& fileName : fileNames)
    {
        if (!fileName.isEmpty())
        {
            BufferID test = doOpen(fileName.toStdWString());
            if (test != BUFFER_INVALID)
                lastOpened = test;
        }
    }

    if (lastOpened != BUFFER_INVALID)
    {
        switchToFile(lastOpened);
    }
}

bool Notepad_plus::fileReload()
{
    if (!_pEditView)
        return false;

    BufferID buf = _pEditView->getCurrentBufferID();
    Buffer* buffer = MainFileManager.getBufferByID(buf);
    if (!buffer)
        return false;

    return doReload(buf, buffer->isDirty());
}

bool Notepad_plus::fileClose(BufferID id, int curView)
{
    BufferID bufferID = id;
    if (id == BUFFER_INVALID)
        bufferID = _pEditView->getCurrentBufferID();

    Buffer* buf = MainFileManager.getBufferByID(bufferID);
    if (!buf)
        return false;

    int viewToClose = currentView();
    if (curView != -1)
        viewToClose = curView;

    // Determine if it's a cloned buffer
    DocTabView* nonCurrentTab = (viewToClose == MAIN_VIEW) ? &_subDocTab : &_mainDocTab;
    bool isCloned = nonCurrentTab->getIndexByBuffer(bufferID) != -1;

    if ((buf->isUntitled() && buf->docLength() == 0) || isCloned)
    {
        // Do nothing
    }
    else if (buf->isDirty())
    {
        const wchar_t* fileNamePath = buf->getFullPathName();
        int res = doSaveOrNot(fileNamePath);

        if (res == IDYES)
        {
            if (!fileSave(id))
                return false;
        }
        else if (res == IDCANCEL)
        {
            return false;
        }
    }

    bool isSnapshotMode = NppParameters::getInstance().getNppGUI().isSnapshotMode();
    bool doDeleteBackup = isSnapshotMode;
    if (isSnapshotMode && isCloned)
        doDeleteBackup = false;

    doClose(bufferID, viewToClose, doDeleteBackup);
    return true;
}

bool Notepad_plus::fileCloseAll(bool doDeleteBackup, bool isSnapshotMode)
{
    bool noSaveToAll = false;
    bool saveToAll = false;

    // Check in both views
    std::vector<BufferID> uniqueBuffers;

    for (size_t i = 0; i < _mainDocTab.nbItem() && !noSaveToAll; ++i)
    {
        BufferID id = _mainDocTab.getBufferByIndex(i);
        Buffer* buf = MainFileManager.getBufferByID(id);

        uniqueBuffers.push_back(id);

        if (buf->isUntitled() && buf->docLength() == 0)
        {
            // Do nothing
        }
        else if (buf->isDirty())
        {
            activateBuffer(id, MAIN_VIEW);

            int res = -1;
            if (saveToAll)
            {
                res = IDYES;
            }
            else
            {
                size_t nbDirtyFiles = MainFileManager.getNbDirtyBuffers();
                res = doSaveOrNot(buf->getFullPathName(), nbDirtyFiles > 1);
            }

            if (res == IDYES)
            {
                if (!fileSave(id))
                    return false;
            }
            else if (res == IDCANCEL)
            {
                return false;
            }
            else if (res == IDIGNORE)
            {
                noSaveToAll = true;
            }
            else if (res == IDRETRY)
            {
                if (!fileSave(id))
                    return false;
                saveToAll = true;
            }
        }
    }

    for (size_t i = 0; i < _subDocTab.nbItem() && !noSaveToAll; ++i)
    {
        BufferID id = _subDocTab.getBufferByIndex(i);
        Buffer* buf = MainFileManager.getBufferByID(id);

        // Check if already processed
        bool alreadyProcessed = false;
        for (BufferID processedId : uniqueBuffers)
        {
            if (processedId == id)
            {
                alreadyProcessed = true;
                break;
            }
        }
        if (alreadyProcessed)
            continue;

        if (buf->isUntitled() && buf->docLength() == 0)
        {
            // Do nothing
        }
        else if (buf->isDirty())
        {
            activateBuffer(id, SUB_VIEW);
            switchEditViewTo(SUB_VIEW);

            int res = -1;
            if (saveToAll)
            {
                res = IDYES;
            }
            else
            {
                size_t nbDirtyFiles = MainFileManager.getNbDirtyBuffers();
                res = doSaveOrNot(buf->getFullPathName(), nbDirtyFiles > 1);
            }

            if (res == IDYES)
            {
                if (!fileSave(id))
                    return false;
            }
            else if (res == IDCANCEL)
            {
                return false;
            }
            else if (res == IDIGNORE)
            {
                noSaveToAll = true;
            }
            else if (res == IDRETRY)
            {
                if (!fileSave(id))
                    return false;
                saveToAll = true;
            }
        }
    }

    // Now close all
    for (size_t i = _mainDocTab.nbItem(); i > 0; --i)
    {
        doClose(_mainDocTab.getBufferByIndex(i - 1), MAIN_VIEW, doDeleteBackup);
    }

    for (size_t i = _subDocTab.nbItem(); i > 0; --i)
    {
        doClose(_subDocTab.getBufferByIndex(i - 1), SUB_VIEW, doDeleteBackup);
    }

    return true;
}

bool Notepad_plus::fileCloseAllButCurrent()
{
    BufferID current = _pEditView->getCurrentBufferID();
    int active = _pDocTab->getCurrentTabIndex();
    bool noSaveToAll = false;
    bool saveToAll = false;

    bool isSnapshotMode = NppParameters::getInstance().getNppGUI().isSnapshotMode();

    // First check main view
    for (size_t i = 0; i < _mainDocTab.nbItem() && !noSaveToAll; ++i)
    {
        BufferID id = _mainDocTab.getBufferByIndex(i);
        if (id == current)
            continue;

        Buffer* buf = MainFileManager.getBufferByID(id);

        if (buf->isUntitled() && buf->docLength() == 0)
        {
            // Do nothing
        }
        else if (buf->isDirty())
        {
            activateBuffer(id, MAIN_VIEW);

            int res = -1;
            if (saveToAll)
            {
                res = IDYES;
            }
            else
            {
                size_t nbDirtyFiles = MainFileManager.getNbDirtyBuffers();
                res = doSaveOrNot(buf->getFullPathName(), nbDirtyFiles > 1);
            }

            if (res == IDYES)
            {
                if (!fileSave(id))
                    return false;
            }
            else if (res == IDCANCEL)
            {
                return false;
            }
            else if (res == IDIGNORE)
            {
                noSaveToAll = true;
            }
            else if (res == IDRETRY)
            {
                if (!fileSave(id))
                    return false;
                saveToAll = true;
            }
        }
    }

    // Then check sub view
    for (size_t i = 0; i < _subDocTab.nbItem() && !noSaveToAll; ++i)
    {
        BufferID id = _subDocTab.getBufferByIndex(i);
        if (id == current)
            continue;

        Buffer* buf = MainFileManager.getBufferByID(id);

        if (buf->isUntitled() && buf->docLength() == 0)
        {
            // Do nothing
        }
        else if (buf->isDirty())
        {
            activateBuffer(id, SUB_VIEW);
            switchEditViewTo(SUB_VIEW);

            int res = -1;
            if (saveToAll)
            {
                res = IDYES;
            }
            else
            {
                size_t nbDirtyFiles = MainFileManager.getNbDirtyBuffers();
                res = doSaveOrNot(buf->getFullPathName(), nbDirtyFiles > 1);
            }

            if (res == IDYES)
            {
                if (!fileSave(id))
                    return false;
            }
            else if (res == IDCANCEL)
            {
                return false;
            }
            else if (res == IDIGNORE)
            {
                noSaveToAll = true;
            }
            else if (res == IDRETRY)
            {
                if (!fileSave(id))
                    return false;
                saveToAll = true;
            }
        }
    }

    // Close all but current in current view
    int currentViewID = currentView();
    DocTabView* pDocTab = (currentViewID == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;

    for (int32_t i = static_cast<int32_t>(pDocTab->nbItem()) - 1; i >= 0; --i)
    {
        if (i == active)
            continue;

        doClose(pDocTab->getBufferByIndex(i), currentViewID, isSnapshotMode);
    }

    // Close all in other view
    int otherViewID = otherFromView(currentViewID);
    DocTabView* pOtherDocTab = (otherViewID == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;

    for (int32_t i = static_cast<int32_t>(pOtherDocTab->nbItem()) - 1; i >= 0; --i)
    {
        doClose(pOtherDocTab->getBufferByIndex(i), otherViewID, isSnapshotMode);
    }

    return true;
}

void Notepad_plus::fileCloseAllButPinned()
{
    std::vector<BufferViewInfo> bufsToClose;

    int iPinned = -1;
    for (int j = 0; j < int(_mainDocTab.nbItem()); ++j)
    {
        BufferID id = _mainDocTab.getBufferByIndex(j);
        Buffer* buf = MainFileManager.getBufferByID(id);
        if (buf->isPinned())
            iPinned++;
        else
            break;
    }

    for (int i = int(_mainDocTab.nbItem()) - 1; i > iPinned; i--)
    {
        bufsToClose.push_back(BufferViewInfo(_mainDocTab.getBufferByIndex(i), MAIN_VIEW));
    }

    iPinned = -1;
    for (int j = 0; j < int(_subDocTab.nbItem()); ++j)
    {
        BufferID id = _subDocTab.getBufferByIndex(j);
        Buffer* buf = MainFileManager.getBufferByID(id);
        if (buf->isPinned())
            iPinned++;
        else
            break;
    }

    for (int i = int(_subDocTab.nbItem()) - 1; i > iPinned; i--)
    {
        bufsToClose.push_back(BufferViewInfo(_subDocTab.getBufferByIndex(i), SUB_VIEW));
    }

    fileCloseAllGiven(bufsToClose);
}

bool Notepad_plus::fileCloseAllToLeft()
{
    std::vector<BufferViewInfo> bufsToClose;
    for (int i = _pDocTab->getCurrentTabIndex() - 1; i >= 0; i--)
    {
        bufsToClose.push_back(BufferViewInfo(_pDocTab->getBufferByIndex(i), currentView()));
    }
    return fileCloseAllGiven(bufsToClose);
}

bool Notepad_plus::fileCloseAllToRight()
{
    const int iActive = _pDocTab->getCurrentTabIndex();
    std::vector<BufferViewInfo> bufsToClose;
    for (int i = int(_pDocTab->nbItem()) - 1; i > iActive; i--)
    {
        bufsToClose.push_back(BufferViewInfo(_pDocTab->getBufferByIndex(i), currentView()));
    }
    return fileCloseAllGiven(bufsToClose);
}

bool Notepad_plus::fileCloseAllUnchanged()
{
    std::vector<BufferViewInfo> bufsToClose;

    for (int i = int(_pDocTab->nbItem()) - 1; i >= 0; i--)
    {
        BufferID id = _pDocTab->getBufferByIndex(i);
        Buffer* buf = MainFileManager.getBufferByID(id);
        if ((buf->isUntitled() && buf->docLength() == 0) || !buf->isDirty())
        {
            bufsToClose.push_back(BufferViewInfo(_pDocTab->getBufferByIndex(i), currentView()));
        }
    }

    return fileCloseAllGiven(bufsToClose);
}

bool Notepad_plus::fileSave(BufferID id)
{
    BufferID bufferID = id;
    if (id == BUFFER_INVALID)
        bufferID = _pEditView->getCurrentBufferID();

    Buffer* buf = MainFileManager.getBufferByID(bufferID);
    if (!buf)
        return false;

    if (!buf->getFileReadOnly() && buf->isDirty())
    {
        if (buf->isUntitled())
        {
            return fileSaveAs(bufferID);
        }

        // TODO: Implement backup functionality
        // For now, just save directly

        return doSave(bufferID, buf->getFullPathName(), false);
    }
    return false;
}

bool Notepad_plus::fileSaveAll()
{
    size_t nbDirty = getNbDirtyBuffer(MAIN_VIEW) + getNbDirtyBuffer(SUB_VIEW);

    if (!nbDirty)
        return false;

    if (fileSaveAllConfirm())
    {
        // Save all in main view
        for (size_t i = 0; i < _mainDocTab.nbItem(); ++i)
        {
            BufferID id = _mainDocTab.getBufferByIndex(i);
            Buffer* buf = MainFileManager.getBufferByID(id);
            if (buf->isDirty())
            {
                if (!fileSave(id))
                    return false;
            }
        }

        // Save all in sub view
        for (size_t i = 0; i < _subDocTab.nbItem(); ++i)
        {
            BufferID id = _subDocTab.getBufferByIndex(i);
            Buffer* buf = MainFileManager.getBufferByID(id);
            if (buf->isDirty())
            {
                if (!fileSave(id))
                    return false;
            }
        }
    }

    return true;
}

bool Notepad_plus::fileSaveAs(BufferID id, bool isSaveCopy)
{
    BufferID bufferID = id;
    if (id == BUFFER_INVALID)
        bufferID = _pEditView->getCurrentBufferID();

    Buffer* buf = MainFileManager.getBufferByID(bufferID);
    if (!buf)
        return false;

    QString defaultName = QString::fromStdWString(buf->getFileName());
    QString defaultDir = QString::fromStdWString(
        buf->isUntitled() ? L"" : buf->getFullPathName());

    QString fileName = QFileDialog::getSaveFileName(
        nullptr,
        QObject::tr("Save As"),
        defaultDir.isEmpty() ? defaultName : defaultDir,
        QObject::tr("All Files (*)"));

    if (fileName.isEmpty())
        return false;

    bool success = doSave(bufferID, fileName.toStdWString().c_str(), isSaveCopy);

    if (success && !isSaveCopy)
    {
        // Update buffer filename
        buf->setFileName(fileName.toStdWString().c_str());
    }

    return success;
}

bool Notepad_plus::fileDelete(BufferID id)
{
    BufferID bufferID = id;
    if (id == BUFFER_INVALID)
        bufferID = _pEditView->getCurrentBufferID();

    Buffer* buf = MainFileManager.getBufferByID(bufferID);
    if (!buf)
        return false;

    QString filePath = QString::fromStdWString(buf->getFullPathName());

    int ret = QMessageBox::question(
        nullptr,
        QObject::tr("Delete File"),
        QObject::tr("Are you sure you want to delete \"%1\"?").arg(filePath),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
        return false;

    // Close the buffer first
    fileClose(bufferID);

    // Delete the file
    return QFile::remove(filePath);
}

bool Notepad_plus::fileRename(BufferID id)
{
    BufferID bufferID = id;
    if (id == BUFFER_INVALID)
        bufferID = _pEditView->getCurrentBufferID();

    Buffer* buf = MainFileManager.getBufferByID(bufferID);
    if (!buf)
        return false;

    QString oldPath = QString::fromStdWString(buf->getFullPathName());
    QString oldName = QString::fromStdWString(buf->getFileName());

    bool ok;
    QString newName = QInputDialog::getText(
        nullptr,
        QObject::tr("Rename"),
        QObject::tr("New name:"),
        QLineEdit::Normal,
        oldName,
        &ok);

    if (!ok || newName.isEmpty() || newName == oldName)
        return false;

    QString newPath = QFileInfo(oldPath).path() + "/" + newName;

    if (QFile::rename(oldPath, newPath))
    {
        buf->setFileName(newPath.toStdWString().c_str());
        return true;
    }

    return false;
}

void Notepad_plus::filePrint(bool showDialog)
{
    // TODO: Implement printing for Linux
    // This would use QPrinter and QPrintDialog
    Q_UNUSED(showDialog);
    QMessageBox::information(nullptr, QObject::tr("Print"), QObject::tr("Printing not yet implemented for Linux."));
}

// ============================================================================
// Session Operations
// ============================================================================

bool Notepad_plus::fileLoadSession(const wchar_t* fn)
{
    QString sessionFile;
    if (fn == nullptr)
    {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        sessionFile = QFileDialog::getOpenFileName(
            nullptr,
            QObject::tr("Load Session"),
            dir,
            QObject::tr("Session Files (*.session);;All Files (*)"));

        if (sessionFile.isEmpty())
            return false;
    }
    else
    {
        sessionFile = QString::fromStdWString(fn);
    }

    // TODO: Implement session loading
    // This would parse the session file and open all files
    Q_UNUSED(sessionFile);
    return false;
}

const wchar_t* Notepad_plus::fileSaveSession(size_t nbFile, wchar_t** fileNames, const wchar_t* sessionFile2save, bool includeFileBrowser)
{
    Q_UNUSED(nbFile);
    Q_UNUSED(fileNames);
    Q_UNUSED(sessionFile2save);
    Q_UNUSED(includeFileBrowser);

    QString sessionFile;
    if (sessionFile2save == nullptr)
    {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        sessionFile = QFileDialog::getSaveFileName(
            nullptr,
            QObject::tr("Save Session"),
            dir + "/session.session",
            QObject::tr("Session Files (*.session);;All Files (*)"));

        if (sessionFile.isEmpty())
            return nullptr;
    }
    else
    {
        sessionFile = QString::fromStdWString(sessionFile2save);
    }

    // TODO: Implement session saving
    // This would save the current session to a file

    // Return the session file path (store it in a member variable to keep it valid)
    static std::wstring sessionPath;
    sessionPath = sessionFile.toStdWString();
    return sessionPath.c_str();
}

const wchar_t* Notepad_plus::fileSaveSession(size_t nbFile, wchar_t** fileNames)
{
    return fileSaveSession(nbFile, fileNames, nullptr, false);
}

// ============================================================================
// Comment Operations
// ============================================================================

bool Notepad_plus::doBlockComment(comment_mode currCommentMode)
{
    Buffer* buf = _pEditView->getCurrentBuffer();
    if (!buf)
        return false;

    // Avoid side-effects when file is read-only
    if (buf->isReadOnly())
        return false;

    // Get comment symbols for current language
    QString commentLineSymbol = buf->getCommentLineSymbol();
    if (commentLineSymbol.isEmpty())
    {
        // Try stream comment as fallback
        QString commentStart = buf->getCommentStart();
        QString commentEnd = buf->getCommentEnd();
        if (!commentStart.isEmpty() && !commentEnd.isEmpty())
        {
            // Use stream comment for block comment
            if (currCommentMode == cm_comment)
            {
                return doStreamComment();
            }
        }
        return false;
    }

    // Get selection
    size_t selectionStart = _pEditView->execute(SCI_GETSELECTIONSTART);
    size_t selectionEnd = _pEditView->execute(SCI_GETSELECTIONEND);
    intptr_t selStartLine = _pEditView->execute(SCI_LINEFROMPOSITION, selectionStart);
    intptr_t selEndLine = _pEditView->execute(SCI_LINEFROMPOSITION, selectionEnd);

    // Adjust for caret at beginning of line
    if (selectionEnd == static_cast<size_t>(_pEditView->execute(SCI_POSITIONFROMLINE, selEndLine)))
        selEndLine--;

    std::string comment = commentLineSymbol.toStdString();
    comment += " ";
    size_t comment_length = comment.length();

    _pEditView->execute(SCI_BEGINUNDOACTION);

    for (intptr_t i = selStartLine; i <= selEndLine; ++i)
    {
        size_t lineStart = _pEditView->execute(SCI_POSITIONFROMLINE, i);
        size_t lineIndent = _pEditView->execute(SCI_GETLINEINDENTPOSITION, i);
        size_t lineEnd = _pEditView->execute(SCI_GETLINEENDPOSITION, i);

        // Skip empty lines
        if (lineIndent == lineEnd)
            continue;

        if (currCommentMode == cm_comment)
        {
            // Add comment at beginning of line (after indentation)
            _pEditView->execute(SCI_INSERTTEXT, lineIndent, reinterpret_cast<sptr_t>(comment.c_str()));
        }
        else if (currCommentMode == cm_uncomment)
        {
            // Remove comment if present
            size_t bufferSize = lineEnd - lineIndent + 1;
            auto linebuf = std::make_unique<char[]>(bufferSize);
            _pEditView->getGenericText(linebuf.get(), bufferSize, lineIndent, lineEnd);

            std::string linebufStr = linebuf.get();
            std::string commentSymbolStr = commentLineSymbol.toStdString();
            size_t pos = linebufStr.find(commentSymbolStr);
            if (pos != std::string::npos)
            {
                size_t len = comment_length;
                if (pos + len < linebufStr.size() && linebufStr[pos + len - 1] == ' ')
                {
                    // Remove the space too
                }
                else
                {
                    len = commentSymbolStr.length();
                }

                _pEditView->execute(SCI_SETSEL, lineIndent + pos, lineIndent + pos + len);
                _pEditView->replaceSelWith("");
            }
        }
        else if (currCommentMode == cm_toggle)
        {
            // Toggle comment
            size_t bufferSize = lineEnd - lineIndent + 1;
            auto linebuf = std::make_unique<char[]>(bufferSize);
            _pEditView->getGenericText(linebuf.get(), bufferSize, lineIndent, lineEnd);

            std::string linebufStr = linebuf.get();
            std::string commentSymbolStr = commentLineSymbol.toStdString();
            size_t pos = linebufStr.find(commentSymbolStr);
            if (pos != std::string::npos)
            {
                // Uncomment
                size_t len = comment_length;
                if (pos + len > linebufStr.size() || linebufStr[pos + len - 1] != ' ')
                    len = commentSymbolStr.length();

                _pEditView->execute(SCI_SETSEL, lineIndent + pos, lineIndent + pos + len);
                _pEditView->replaceSelWith("");
            }
            else
            {
                // Comment
                _pEditView->execute(SCI_INSERTTEXT, lineIndent, reinterpret_cast<sptr_t>(comment.c_str()));
            }
        }
    }

    _pEditView->execute(SCI_ENDUNDOACTION);
    return true;
}

bool Notepad_plus::doStreamComment()
{
    Buffer* buf = _pEditView->getCurrentBuffer();
    if (!buf)
        return false;

    if (buf->isReadOnly())
        return false;

    QString commentStart = buf->getCommentStart();
    QString commentEnd = buf->getCommentEnd();

    if (commentStart.isEmpty() || commentEnd.isEmpty())
        return false;

    size_t selectionStart = _pEditView->execute(SCI_GETSELECTIONSTART);
    size_t selectionEnd = _pEditView->execute(SCI_GETSELECTIONEND);

    // If no selection, select current line
    if (selectionStart == selectionEnd)
    {
        intptr_t currentLine = _pEditView->getCurrentLineNumber();
        selectionStart = _pEditView->execute(SCI_POSITIONFROMLINE, currentLine);
        selectionEnd = _pEditView->execute(SCI_GETLINEENDPOSITION, currentLine);
    }

    _pEditView->execute(SCI_BEGINUNDOACTION);

    // Insert end comment first (so positions don't shift)
    std::string commentEndStr = commentEnd.toStdString();
    std::string commentStartStr = commentStart.toStdString();
    _pEditView->execute(SCI_INSERTTEXT, selectionEnd, reinterpret_cast<sptr_t>(commentEndStr.c_str()));

    // Insert start comment
    _pEditView->execute(SCI_INSERTTEXT, selectionStart, reinterpret_cast<sptr_t>(commentStartStr.c_str()));

    _pEditView->execute(SCI_ENDUNDOACTION);
    return true;
}

// ============================================================================
// Buffer Change Notification
// ============================================================================

void Notepad_plus::notifyBufferChanged(Buffer* buffer, int mask)
{
    if (!buffer)
        return;

    // Update views
    _mainEditView.bufferUpdated(buffer, mask);
    _subEditView.bufferUpdated(buffer, mask);
    _mainDocTab.bufferUpdated(buffer, mask);
    _subDocTab.bufferUpdated(buffer, mask);

    bool mainActive = (_mainEditView.getCurrentBuffer() == buffer);
    bool subActive = (_subEditView.getCurrentBuffer() == buffer);

    if (mask & BufferChangeStatus)
    {
        switch (buffer->getStatus())
        {
            case QtCore::DOC_UNNAMED:
            case QtCore::DOC_REGULAR:
            case QtCore::DOC_INACCESSIBLE:
                break;

            case QtCore::DOC_MODIFIED:
            {
                // File modified externally
                if (!buffer->isMonitoringOn())
                {
                    // Ask user to reload
                    QString fileName = QString::fromStdWString(buffer->getFullPathName());
                    int ret = QMessageBox::question(
                        nullptr,
                        QObject::tr("Reload"),
                        QObject::tr("\"%1\"\n\nThis file has been modified by another program.\nDo you want to reload it?").arg(fileName),
                        QMessageBox::Yes | QMessageBox::No);

                    if (ret == QMessageBox::Yes)
                    {
                        buffer->setDirty(false);
                        buffer->setUnsync(false);
                        doReload(buffer->getID(), false);
                        if (mainActive || subActive)
                        {
                            performPostReload(mainActive ? MAIN_VIEW : SUB_VIEW);
                        }
                    }
                    else
                    {
                        buffer->setDirty(true);
                        buffer->setUnsync(true);
                    }
                }
                break;
            }

            case QtCore::DOC_NEEDRELOAD:
            {
                doReload(buffer->getID(), false);
                if (buffer == _mainEditView.getCurrentBuffer())
                {
                    _mainEditView.setPositionRestoreNeeded(false);
                    _mainEditView.execute(SCI_DOCUMENTEND);
                }
                if (buffer == _subEditView.getCurrentBuffer())
                {
                    _subEditView.setPositionRestoreNeeded(false);
                    _subEditView.execute(SCI_DOCUMENTEND);
                }
                break;
            }

            case QtCore::DOC_DELETED:
            {
                // File deleted externally
                QString fileName = QString::fromStdWString(buffer->getFullPathName());
                int ret = QMessageBox::question(
                    nullptr,
                    QObject::tr("Keep File"),
                    QObject::tr("\"%1\"\n\nThis file has been deleted by another program.\nDo you want to keep it in the editor?").arg(fileName),
                    QMessageBox::Yes | QMessageBox::No);

                if (ret == QMessageBox::No)
                {
                    doClose(buffer->getID(), otherView());
                    doClose(buffer->getID(), currentView());
                }
                break;
            }
        }
    }
}

// ============================================================================
// File Browser
// ============================================================================

void Notepad_plus::launchFileBrowser(const std::vector<std::wstring>& folders, const std::wstring& selectedItemPath, bool fromScratch)
{
    Q_UNUSED(folders);
    Q_UNUSED(selectedItemPath);
    Q_UNUSED(fromScratch);

    // TODO: Implement file browser panel launch
    // This would show the file browser panel with the specified folders
}

// ============================================================================
// Helper Methods
// ============================================================================

bool Notepad_plus::fileSaveAllConfirm()
{
    bool confirmed = false;

    if (NppParameters::getInstance().getNppGUI()._saveAllConfirm)
    {
        int answer = doSaveAll();

        if (answer == IDYES)
        {
            confirmed = true;
        }

        if (answer == IDRETRY)
        {
            NppParameters::getInstance().getNppGUI()._saveAllConfirm = false;
            confirmed = true;
        }
    }
    else
    {
        confirmed = true;
    }

    return confirmed;
}

bool Notepad_plus::fileSaveSpecific(const std::wstring& fileNameToSave)
{
    BufferID idToSave = _mainDocTab.findBufferByName(fileNameToSave.c_str());
    if (idToSave == BUFFER_INVALID)
    {
        idToSave = _subDocTab.findBufferByName(fileNameToSave.c_str());
    }

    if (idToSave != BUFFER_INVALID)
    {
        fileSave(idToSave);
        checkDocState();
        return true;
    }

    return false;
}

bool Notepad_plus::fileCloseAllGiven(const std::vector<BufferViewInfo>& krvecBuffer)
{
    bool isSnapshotMode = NppParameters::getInstance().getNppGUI().isSnapshotMode();

    for (const auto& i : krvecBuffer)
    {
        doClose(i._bufID, i._iView, isSnapshotMode);
    }

    return true;
}

void Notepad_plus::prepareBufferChangedDialog(Buffer* buffer)
{
    // Switch to the file that changed
    int index = _pDocTab->getIndexByBuffer(buffer->getID());
    int iView = currentView();
    if (index == -1)
        iView = otherView();

    activateBuffer(buffer->getID(), iView);
}

bool Notepad_plus::isFileSession(const wchar_t* filename)
{
    const wchar_t* definedSessionExt = NppParameters::getInstance().getNppGUI()._definedSessionExt.c_str();
    if (*definedSessionExt != L'\0')
    {
        std::wstring fncp = filename;
        std::wstring usrSessionExt = L"";
        if (*definedSessionExt != L'.')
        {
            usrSessionExt += L".";
        }
        usrSessionExt += definedSessionExt;

        size_t pos = fncp.rfind(L'.');
        if (pos != std::wstring::npos)
        {
            std::wstring ext = fncp.substr(pos);
            if (ext == usrSessionExt)
                return true;
        }
    }

    // Check default extension
    std::wstring fn = filename;
    if (fn.size() > 8 && fn.substr(fn.size() - 8) == L".session")
        return true;

    return false;
}

bool Notepad_plus::isFileWorkspace(const wchar_t* filename)
{
    const wchar_t* definedWorkspaceExt = NppParameters::getInstance().getNppGUI()._definedWorkspaceExt.c_str();
    if (*definedWorkspaceExt != L'\0')
    {
        std::wstring fncp = filename;
        std::wstring usrWorkspaceExt = L"";
        if (*definedWorkspaceExt != L'.')
        {
            usrWorkspaceExt += L".";
        }
        usrWorkspaceExt += definedWorkspaceExt;

        size_t pos = fncp.rfind(L'.');
        if (pos != std::wstring::npos)
        {
            std::wstring ext = fncp.substr(pos);
            if (ext == usrWorkspaceExt)
                return true;
        }
    }

    // Check default extension
    std::wstring fn = filename;
    if (fn.size() > 10 && fn.substr(fn.size() - 10) == L".workspace")
        return true;

    return false;
}

// ============================================================================
// Core File Operations (doSave, doReload, doClose)
// ============================================================================

bool Notepad_plus::doSave(BufferID id, const wchar_t* filename, bool isCopy)
{
    const int index = MainFileManager.getBufferIndexByID(id);
    if (index == -1)
    {
        QMessageBox::warning(nullptr, QObject::tr("Save failed"),
            QObject::tr("Cannot save: Buffer is invalid."));
        return false;
    }

    // Notify plugins that current file is about to be saved
    if (!isCopy)
    {
        // TODO: Plugin notification
        // scnN.nmhdr.code = NPPN_FILEBEFORESAVE;
    }

    SavingStatus res = MainFileManager.saveBuffer(id, filename, isCopy);

    if (!isCopy)
    {
        // TODO: Plugin notification
        // scnN.nmhdr.code = NPPN_FILESAVED;
    }

    if (res == SavingStatus::FullReadOnlySavingForbidden)
    {
        QMessageBox::warning(nullptr, QObject::tr("Save failed"),
            QObject::tr("Cannot save file.\nThe Notepad++ full read-only saving forbidden mode prevented the file from being saved."));
    }
    else if (res == SavingStatus::NotEnoughRoom)
    {
        QMessageBox::warning(nullptr, QObject::tr("Save failed"),
            QObject::tr("Failed to save file.\nIt seems there's not enough space on disk to save file. Your file is not saved."));
    }
    else if (res == SavingStatus::SaveWritingFailed)
    {
        QMessageBox::warning(nullptr, QObject::tr("Save failed"),
            QObject::tr("Failed to write file."));
    }
    else if (res == SavingStatus::SaveOpenFailed)
    {
        QMessageBox::warning(nullptr, QObject::tr("Save failed"),
            QObject::tr("Please check whether if this file is opened in another program or is write-protected."));
    }

    return res == SavingStatus::SaveOK;
}

bool Notepad_plus::doReload(BufferID id, bool alert)
{
    if (id == BUFFER_INVALID)
        return false;

    Buffer* buf = MainFileManager.getBufferByID(id);
    if (!buf)
        return false;

    // Check if file is dirty and alert user
    if (alert && buf->isDirty())
    {
        QString fileName = QString::fromStdWString(buf->getFullPathName());
        int ret = QMessageBox::question(nullptr, QObject::tr("Reload"),
            QObject::tr("\"%1\"\n\nThis file has been modified by another program.\nDo you want to reload it and lose the changes made in Notepad++?").arg(fileName),
            QMessageBox::Yes | QMessageBox::No);

        if (ret != QMessageBox::Yes)
            return false;
    }

    // Perform reload
    bool result = MainFileManager.reloadBuffer(id);

    if (result)
    {
        // Update UI
        buf->setDirty(false);
        buf->setUnsync(false);
    }

    return result;
}

void Notepad_plus::doClose(BufferID id, int whichOne, bool doDeleteBackup)
{
    DocTabView* tabToClose = (whichOne == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;
    int i = tabToClose->getIndexByBuffer(id);
    if (i == -1)
        return;

    if (doDeleteBackup)
        MainFileManager.deleteBufferBackup(id);

    Buffer* buf = MainFileManager.getBufferByID(id);
    if (!buf)
        return;

    // TODO: Notify plugins that current file is about to be closed
    // scnN.nmhdr.code = NPPN_FILEBEFORECLOSE;

    // Get file path for recent files
    std::wstring fileFullPath;
    if (!buf->isUntitled())
    {
        const wchar_t* fn = buf->getFullPathName();
        if (QFile::exists(QString::fromStdWString(fn)))
            fileFullPath = fn;
    }

    size_t nbDocs = (whichOne == MAIN_VIEW) ? _mainDocTab.nbItem() : _subDocTab.nbItem();

    // Turn off monitoring if active
    if (buf->isMonitoringOn())
    {
        monitoringStartOrStopAndUpdateUI(buf, false);
    }

    // Do all the work
    bool isBufRemoved = removeBufferFromView(id, whichOne);
    BufferID hiddenBufferID = BUFFER_INVALID;

    if (nbDocs == 1 && canHideView(whichOne))
    {
        // Close the view if both visible
        hideView(whichOne);
        hiddenBufferID = (whichOne == MAIN_VIEW) ?
            _mainDocTab.getBufferByIndex(0) : _subDocTab.getBufferByIndex(0);
    }

    checkSyncState();

    // Notify plugins that current file is closed
    if (isBufRemoved)
    {
        // TODO: Plugin notification
        // scnN.nmhdr.code = NPPN_FILECLOSED;

        // Add to recent files if file was removed and exists
        if (!fileFullPath.empty())
        {
            _lastRecentFileList.add(fileFullPath.c_str());
        }
    }
}

// ============================================================================
// Additional Helper Methods
// ============================================================================

BufferID Notepad_plus::doOpen(const std::wstring& fileName, bool isRecursive, bool isReadOnly, int encoding, const wchar_t* backupFileName, FILETIME fileNameTimestamp)
{
    Q_UNUSED(isRecursive);
    Q_UNUSED(fileNameTimestamp);

    if (fileName.empty())
        return BUFFER_INVALID;

    QString qFileName = QString::fromStdWString(fileName);

    // Check if file already open
    BufferID existingID = MainFileManager.getBufferFromName(fileName.c_str());
    if (existingID != BUFFER_INVALID)
    {
        // File already open, just switch to it
        activateBuffer(existingID, currentView());
        return existingID;
    }

    // Check if file exists
    if (!QFile::exists(qFileName))
    {
        return BUFFER_INVALID;
    }

    // Load the file
    BufferID bufferID = MainFileManager.loadFile(fileName.c_str(), static_cast<Document>(NULL), encoding);

    if (bufferID != BUFFER_INVALID)
    {
        Buffer* buf = MainFileManager.getBufferByID(bufferID);
        if (buf)
        {
            if (isReadOnly)
                buf->setFileReadOnly(true);

            loadBufferIntoView(bufferID, currentView());
            switchToFile(bufferID);
        }
    }

    return bufferID;
}

void Notepad_plus::monitoringStartOrStopAndUpdateUI(Buffer* pBuf, bool isStarting)
{
    if (!pBuf)
        return;

    if (isStarting)
    {
        pBuf->startMonitoring();
        // TODO: Start file monitoring thread
    }
    else
    {
        pBuf->stopMonitoring();
        // TODO: Stop file monitoring thread
    }
}

// ============================================================================
// Dialog Helper Methods
// ============================================================================

int Notepad_plus::doSaveOrNot(const wchar_t* fn, bool isMulti)
{
    if (NppParameters::getInstance().isEndSessionCritical())
        return IDCANCEL;

    QString fileName = QString::fromStdWString(fn);

    if (!isMulti)
    {
        int ret = QMessageBox::question(
            nullptr,
            QObject::tr("Save"),
            QObject::tr("Save file \"%1\"?").arg(fileName),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (ret == QMessageBox::Yes)
            return IDYES;
        else if (ret == QMessageBox::No)
            return IDNO;
        else
            return IDCANCEL;
    }
    else
    {
        // For multiple files, use a custom dialog with Yes to All/No to All options
        QMessageBox msgBox;
        msgBox.setWindowTitle(QObject::tr("Save All"));
        msgBox.setText(QObject::tr("Save file \"%1\"?").arg(fileName));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.addButton(QObject::tr("Yes to All"), QMessageBox::YesRole);
        msgBox.addButton(QObject::tr("No to All"), QMessageBox::NoRole);

        int ret = msgBox.exec();

        // Map button roles to return values
        QAbstractButton* clickedButton = msgBox.clickedButton();
        QString buttonText = clickedButton->text();

        if (buttonText == QObject::tr("Yes to All"))
            return IDRETRY;  // Using IDRETRY for "Yes to All"
        else if (buttonText == QObject::tr("No to All"))
            return IDIGNORE; // Using IDIGNORE for "No to All"
        else if (ret == QMessageBox::Yes)
            return IDYES;
        else if (ret == QMessageBox::No)
            return IDNO;
        else
            return IDCANCEL;
    }
}

int Notepad_plus::doSaveAll()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(QObject::tr("Save All"));
    msgBox.setText(QObject::tr("Save all modified files?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.addButton(QObject::tr("Always Yes"), QMessageBox::YesRole);

    int ret = msgBox.exec();

    QAbstractButton* clickedButton = msgBox.clickedButton();
    if (clickedButton->text() == QObject::tr("Always Yes"))
        return IDRETRY;  // Using IDRETRY for "Always Yes"
    else if (ret == QMessageBox::Yes)
        return IDYES;
    else
        return IDNO;
}

// ============================================================================
// Stub Implementations for Missing Methods
// These are simplified implementations that need to be fully implemented later
// ============================================================================

size_t Notepad_plus::getNbDirtyBuffer(int view)
{
    DocTabView* pDocTab = (view == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;
    size_t count = 0;
    for (size_t i = 0; i < pDocTab->nbItem(); ++i)
    {
        BufferID id = pDocTab->getBufferByIndex(i);
        Buffer* buf = MainFileManager.getBufferByID(id);
        if (buf && buf->isDirty())
            count++;
    }
    return count;
}

int Notepad_plus::otherView()
{
    return (currentView() == MAIN_VIEW) ? SUB_VIEW : MAIN_VIEW;
}

void Notepad_plus::checkSyncState()
{
    // TODO: Implement sync state checking
}

void Notepad_plus::checkDocState()
{
    // TODO: Implement document state checking
}

bool Notepad_plus::activateBuffer(BufferID id, int view, bool forceApplyHilite)
{
    Q_UNUSED(forceApplyHilite);
    if (view == MAIN_VIEW)
    {
        bool activated = _mainDocTab.activateBuffer(id);
        if (activated)
            _mainEditView.activateBuffer(id, forceApplyHilite);
        return activated;
    }
    else
    {
        bool activated = _subDocTab.activateBuffer(id);
        if (activated)
            _subEditView.activateBuffer(id, forceApplyHilite);
        return activated;
    }
}

void Notepad_plus::loadBufferIntoView(BufferID id, int view, bool dontClose)
{
    Q_UNUSED(dontClose);
    DocTabView* tabToOpen = (view == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;

    // Check if buffer already exists
    int index = tabToOpen->getIndexByBuffer(id);
    if (index != -1)  // Already open, done
        return;

    // Add buffer to tab
    tabToOpen->addBuffer(id);
}

bool Notepad_plus::switchToFile(BufferID id)
{
    // Activate the buffer in the current view
    bool activated = _pDocTab->activateBuffer(id);
    if (activated)
        _pEditView->activateBuffer(id, false);
    return activated;
}

bool Notepad_plus::removeBufferFromView(BufferID id, int view)
{
    DocTabView* tabToClose = (view == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;
    int index = tabToClose->getIndexByBuffer(id);
    if (index == -1)
        return false;

    // Remove from tab view - use deletItemAt (note the typo in original code)
    tabToClose->deletItemAt(static_cast<size_t>(index));

    // Check if buffer is still in use in other view
    int otherViewID = otherFromView(view);
    DocTabView* pOtherDocTab = (otherViewID == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;
    if (pOtherDocTab->getIndexByBuffer(id) == -1)
    {
        // Buffer no longer in use, can be removed from file manager
        // MainFileManager.closeBuffer(id, view);
        return true;
    }
    return false;
}

bool Notepad_plus::canHideView(int view)
{
    if (!viewVisible(view))
        return false;   //cannot hide hidden view
    if (!bothActive())
        return false;   //cannot hide only window

    DocTabView* tabToCheck = (view == MAIN_VIEW) ? &_mainDocTab : &_subDocTab;
    Buffer* buf = MainFileManager.getBufferByID(tabToCheck->getBufferByIndex(0));
    bool canHide = ((tabToCheck->nbItem() == 1) && !buf->isDirty() && buf->isUntitled());
    return canHide;
}

void Notepad_plus::hideView(int whichOne)
{
    if (!bothActive())  //cannot close if not both views visible
        return;

    if (whichOne == MAIN_VIEW)
    {
        _mainWindowStatus &= ~WindowMainActive;
        _mainEditView.display(false);
        _mainDocTab.display(false);
    }
    else if (whichOne == SUB_VIEW)
    {
        _mainWindowStatus &= ~WindowSubActive;
        _subEditView.display(false);
        _subDocTab.display(false);
    }

    // Update the splitter layout - Qt handles this automatically
    // when visibility changes via the layout system
}

void Notepad_plus::performPostReload(int whichOne)
{
    Q_UNUSED(whichOne);
    // TODO: Implement post-reload actions
}

// ============================================================================
// Marked/Bookmarked Lines Operations
// ============================================================================

std::mutex mark_mutex;

void Notepad_plus::copyMarkedLines()
{
    intptr_t lastLine = _pEditView->lastZeroBasedLineNumber();
    std::wstring globalStr = L"";
    for (intptr_t i = lastLine; i >= 0; i--)
    {
        if (bookmarkPresent(i))
        {
            std::wstring currentStr = getMarkedLine(i) + globalStr;
            globalStr = currentStr;
        }
    }

    // Use platform clipboard
    PlatformLayer::IClipboard::getInstance().setText(globalStr);
}

void Notepad_plus::cutMarkedLines()
{
    std::lock_guard<std::mutex> lock(mark_mutex);

    intptr_t lastLine = _pEditView->lastZeroBasedLineNumber();
    std::wstring globalStr = L"";

    _pEditView->execute(SCI_BEGINUNDOACTION);
    for (intptr_t i = lastLine; i >= 0; i--)
    {
        if (bookmarkPresent(i))
        {
            std::wstring currentStr = getMarkedLine(i) + globalStr;
            globalStr = currentStr;

            deleteMarkedline(i);
        }
    }
    _pEditView->execute(SCI_ENDUNDOACTION);

    PlatformLayer::IClipboard::getInstance().setText(globalStr);
}

void Notepad_plus::deleteMarkedLines(bool isMarked)
{
    std::lock_guard<std::mutex> lock(mark_mutex);

    intptr_t lastLine = _pEditView->lastZeroBasedLineNumber();

    _pEditView->execute(SCI_BEGINUNDOACTION);
    for (intptr_t i = lastLine; i >= 0; i--)
    {
        if (bookmarkPresent(i) == isMarked)
            deleteMarkedline(i);
    }
    _pEditView->execute(SCI_ENDUNDOACTION);
}

void Notepad_plus::pasteToMarkedLines()
{
    std::lock_guard<std::mutex> lock(mark_mutex);

    // Get text from clipboard
    std::wstring clipboardStr = PlatformLayer::IClipboard::getInstance().getText();
    if (clipboardStr.empty())
        return;

    intptr_t lastLine = _pEditView->lastZeroBasedLineNumber();

    _pEditView->execute(SCI_BEGINUNDOACTION);
    for (intptr_t i = lastLine; i >= 0; i--)
    {
        if (bookmarkPresent(i))
        {
            replaceMarkedline(i, clipboardStr.c_str());
        }
    }
    _pEditView->execute(SCI_ENDUNDOACTION);
}

void Notepad_plus::inverseMarks()
{
    intptr_t lastLine = _pEditView->lastZeroBasedLineNumber();
    for (intptr_t i = 0; i <= lastLine; ++i)
    {
        if (bookmarkPresent(i))
        {
            bookmarkDelete(i);
        }
        else
        {
            bookmarkAdd(i);
        }
    }
}

void Notepad_plus::deleteMarkedline(size_t ln)
{
    intptr_t lineLen = _pEditView->execute(SCI_LINELENGTH, ln);
    intptr_t lineBegin = _pEditView->execute(SCI_POSITIONFROMLINE, ln);

    bookmarkDelete(ln);
    wchar_t emptyString[2] = L"";
    _pEditView->replaceTarget(emptyString, lineBegin, lineBegin + lineLen);
}

void Notepad_plus::replaceMarkedline(size_t ln, const wchar_t* str)
{
    intptr_t lineBegin = _pEditView->execute(SCI_POSITIONFROMLINE, ln);
    intptr_t lineEnd = _pEditView->execute(SCI_GETLINEENDPOSITION, ln);

    _pEditView->replaceTarget(str, lineBegin, lineEnd);
}

std::wstring Notepad_plus::getMarkedLine(size_t ln)
{
    auto lineLen = _pEditView->execute(SCI_LINELENGTH, ln);
    auto lineBegin = _pEditView->execute(SCI_POSITIONFROMLINE, ln);

    wchar_t* buf = new wchar_t[lineLen + 1];
    _pEditView->getGenericText(buf, lineLen + 1, lineBegin, lineBegin + lineLen);
    std::wstring line = buf;
    delete[] buf;

    return line;
}

// ============================================================================
// Comment Operations
// ============================================================================

bool Notepad_plus::undoStreamComment(bool tryBlockComment)
{
    QString commentStart;
    QString commentEnd;
    QString commentLineSymbol;

    std::string symbolStart;
    std::string symbolEnd;
    std::string symbol;

    Buffer* buf = _pEditView->getCurrentBuffer();
    // Avoid side-effects when file is read-only
    if (buf->isReadOnly())
        return false;

    if (buf->getLangType() == L_USER)
    {
        // User defined language - get from user lang container
        // For now, return false as this requires more complex handling
        return false;
    }
    else
    {
        commentLineSymbol = buf->getCommentLineSymbol();
        commentStart = buf->getCommentStart();
        commentEnd = buf->getCommentEnd();
    }

    // If there is no stream-comment symbol and we came not from doBlockComment, try the block comment
    if (commentStart.isEmpty() || commentEnd.isEmpty())
    {
        if (!commentLineSymbol.isEmpty() && tryBlockComment)
            return doBlockComment(cm_uncomment);
        else
            return false;
    }

    std::string start_comment = commentStart.toStdString();
    std::string end_comment = commentEnd.toStdString();
    size_t start_comment_length = start_comment.length();
    size_t end_comment_length = end_comment.length();

    size_t selectionStart = _pEditView->execute(SCI_GETSELECTIONSTART);
    size_t selectionEnd = _pEditView->execute(SCI_GETSELECTIONEND);

    _pEditView->execute(SCI_BEGINUNDOACTION);

    // Simple implementation: find and remove stream comments within selection
    // This is a simplified version - the full version handles nested comments etc.
    bool found = false;
    size_t pos = selectionStart;
    while (pos < selectionEnd)
    {
        // Use string_view overload for searchInTarget
        std::string_view startView(start_comment);
        intptr_t foundPos = _pEditView->searchInTarget(startView, pos, selectionEnd);
        if (foundPos == -1)
            break;

        std::string_view endView(end_comment);
        intptr_t endPos = _pEditView->searchInTarget(endView, foundPos + start_comment_length, selectionEnd + end_comment_length);
        if (endPos == -1)
            break;

        // Remove end comment first (so positions don't shift)
        _pEditView->replaceTarget("", static_cast<size_t>(endPos), static_cast<size_t>(endPos) + end_comment_length);
        // Remove start comment
        _pEditView->replaceTarget("", static_cast<size_t>(foundPos), static_cast<size_t>(foundPos) + start_comment_length);

        found = true;
        pos = static_cast<size_t>(foundPos);
        selectionEnd -= (start_comment_length + end_comment_length);
    }

    _pEditView->execute(SCI_ENDUNDOACTION);

    return found;
}

// ============================================================================
// Read-Only Mode Operations
// ============================================================================

void Notepad_plus::changeReadOnlyUserModeForAllOpenedTabs(const bool ro)
{
    if (ro != true && NppParameters::getInstance().getNppGUI()._isFullReadOnlySavingForbidden)
        return; // safety for FullReadOnlySavingForbidden mode, refuse to cease the R/O state

    // make R/O changes in both views
    std::vector<DocTabView*> tabViews = { &_mainDocTab, &_subDocTab };
    for (auto& pTabView : tabViews)
    {
        for (size_t i = 0; i < pTabView->nbItem(); ++i)
        {
            BufferID id = pTabView->getBufferByIndex(i);
            if (id != BUFFER_INVALID)
            {
                Buffer* buf = MainFileManager.getBufferByID(id);
                if (buf != nullptr)
                    buf->setUserReadOnly(ro);
            }
        }
    }
}

// ============================================================================
// View Mode Operations (Full Screen, Post-It, Distraction Free)
// ============================================================================

void Notepad_plus::fullScreenToggle()
{
    // TODO: Implement full screen toggle for Qt
    // This requires access to the main window which is managed by Notepad_plus_Window
    // For now, this is a stub that will be implemented when the window integration is complete
}

void Notepad_plus::postItToggle()
{
    // TODO: Implement post-it toggle for Qt
    // This requires access to the main window which is managed by Notepad_plus_Window
    // For now, this is a stub that will be implemented when the window integration is complete
}

void Notepad_plus::distractionFreeToggle()
{
    // TODO: Implement distraction free toggle for Qt
    // This requires access to the main window which is managed by Notepad_plus_Window
    // For now, this is a stub that will be implemented when the window integration is complete
}

// ============================================================================
// View Operations (Wrap, Indent Guide, Whitespace, EOL, etc.)
// ============================================================================

void Notepad_plus::wrapAllEditors(bool isWrapped)
{
    _mainEditView.wrap(isWrapped);
    _subEditView.wrap(isWrapped);
}

void Notepad_plus::showIndentGuide(bool show)
{
    _mainEditView.showIndentGuideLine(show);
    _subEditView.showIndentGuideLine(show);
}

bool Notepad_plus::isWhiteSpaceShown()
{
    NppParameters& nppParam = NppParameters::getInstance();
    const ScintillaViewParams& svp = nppParam.getSVP();
    return svp._whiteSpaceShow;
}

void Notepad_plus::showWhiteSpace(bool show)
{
    NppParameters& nppParam = NppParameters::getInstance();
    ScintillaViewParams& svp = const_cast<ScintillaViewParams&>(nppParam.getSVP());
    svp._whiteSpaceShow = show;
    _mainEditView.showWSAndTab(show);
    _subEditView.showWSAndTab(show);
}

bool Notepad_plus::isEOLShown()
{
    NppParameters& nppParam = NppParameters::getInstance();
    const ScintillaViewParams& svp = nppParam.getSVP();
    return svp._eolShow;
}

void Notepad_plus::showEOL(bool show)
{
    NppParameters& nppParam = NppParameters::getInstance();
    ScintillaViewParams& svp = const_cast<ScintillaViewParams&>(nppParam.getSVP());
    svp._eolShow = show;
    _mainEditView.showEOL(show);
    _subEditView.showEOL(show);
}

bool Notepad_plus::isAllCharactersShown()
{
    return isNpcShown() && isEOLShown();
}

void Notepad_plus::showInvisibleChars(bool show)
{
    showNpc(show);
    showEOL(show);
}

bool Notepad_plus::isNpcShown()
{
    NppParameters& nppParam = NppParameters::getInstance();
    const ScintillaViewParams& svp = nppParam.getSVP();
    return svp._npcShow;
}

void Notepad_plus::showNpc(bool show)
{
    NppParameters& nppParam = NppParameters::getInstance();
    ScintillaViewParams& svp = const_cast<ScintillaViewParams&>(nppParam.getSVP());
    svp._npcShow = show;
    _mainEditView.showNpc(show);
    _subEditView.showNpc(show);
}

bool Notepad_plus::isCcUniEolShown()
{
    NppParameters& nppParam = NppParameters::getInstance();
    const ScintillaViewParams& svp = nppParam.getSVP();
    return svp._ccUniEolShow;
}

void Notepad_plus::showCcUniEol(bool show)
{
    NppParameters& nppParam = NppParameters::getInstance();
    ScintillaViewParams& svp = const_cast<ScintillaViewParams&>(nppParam.getSVP());
    svp._ccUniEolShow = show;
    _mainEditView.showCcUniEol(show);
    _subEditView.showCcUniEol(show);
}

void Notepad_plus::toggleSyncScrollV()
{
    _syncInfo._isSynScrollV = !_syncInfo._isSynScrollV;
}

void Notepad_plus::toggleSyncScrollH()
{
    _syncInfo._isSynScrollH = !_syncInfo._isSynScrollH;
}

// ============================================================================
// Panel Toggle Operations
// ============================================================================

void Notepad_plus::showSummary()
{
    // TODO: Implement summary dialog for Qt
    // This would show document statistics like line count, word count, etc.
}

void Notepad_plus::toggleMonitoring()
{
    Buffer* buf = getCurrentBuffer();
    if (buf)
    {
        bool isMonitoring = buf->isMonitoringOn();
        monitoringStartOrStopAndUpdateUI(buf, !isMonitoring);
    }
}

void Notepad_plus::toggleDocumentList()
{
    launchDocumentListPanel(true);
}

void Notepad_plus::toggleDocumentMap()
{
    launchDocMap();
}

void Notepad_plus::toggleFunctionList()
{
    launchFunctionList();
}

void Notepad_plus::toggleFileBrowser()
{
    // TODO: Implement for Qt
    // Note: Qt FileBrowser is in QtControls namespace and has different interface
    // For now, just launch the file browser via launchFileBrowser
    if (_pFileBrowser == nullptr)
    {
        std::vector<std::wstring> folders;
        launchFileBrowser(folders, L"", true);
    }
    // Note: Cannot call display() on _pFileBrowser as it's a forward-declared type
    // The docking manager handles visibility for Qt version
}

void Notepad_plus::toggleProjectPanel(int index)
{
    int cmdID = IDM_VIEW_PROJECT_PANEL_1 + index;
    ProjectPanel** ppProjPanel = nullptr;
    if (index == 0)
        ppProjPanel = &_pProjectPanel_1;
    else if (index == 1)
        ppProjPanel = &_pProjectPanel_2;
    else
        ppProjPanel = &_pProjectPanel_3;

    launchProjectPanel(cmdID, ppProjPanel, index);
}

void Notepad_plus::switchToProjectPanel(int index)
{
    // Implementation would activate the project panel
    // For now, just toggle it
    toggleProjectPanel(index);
}

void Notepad_plus::switchToFileBrowser()
{
    // TODO: Implement for Qt
    // Note: Cannot call display() on _pFileBrowser as it's a forward-declared type
    // The docking manager handles visibility for Qt version
}

// ============================================================================
// Search and Find Operations
// ============================================================================

void Notepad_plus::showFindReplaceDlg(int dialogType)
{
    // Map dialog type to DIALOG_TYPE enum
    DIALOG_TYPE dlgType = static_cast<DIALOG_TYPE>(dialogType);

    // Show the Find/Replace dialog with the specified type
    _findReplaceDlg.doDialog(dlgType);
}

void Notepad_plus::findNext(int direction)
{
    // Use the find/replace dialog to perform find next
    FindOption opt;
    opt._whichDirection = (direction == DIR_DOWN);
    opt._isWrapAround = true;

    // Get current search text from options
    if (!_findReplaceDlg._env->_str2Search.empty())
    {
        processFindNext(_findReplaceDlg._env->_str2Search.c_str(), &opt);
    }
}

void Notepad_plus::processFindNext(const wchar_t* text, const FindOption* opt)
{
    if (!text || !opt || !_pEditView)
        return;

    std::wstring searchText(text);
    if (searchText.empty())
        return;

    // Use the find/replace dialog's processFindNext method
    FindStatus findStatus = FSNoMessage;
    _findReplaceDlg.processFindNext(text, opt, &findStatus);
}

void Notepad_plus::showIncrementalFindDlg()
{
    // Show the incremental find dialog
    _incrementFindDlg.display(true);
}

void Notepad_plus::setSearchText(const wchar_t* text)
{
    if (text)
    {
        _findReplaceDlg.setSearchText(text);
    }
}

void Notepad_plus::gotoNextFoundResult(int direction)
{
    // Navigate to next/previous found result in the finder panel
    _findReplaceDlg.gotoNextFoundResult(direction);
}

void Notepad_plus::showGoToLineDlg()
{
    // Show the Go To Line dialog
    // Initialize with Windows-compatible interface (HINSTANCE, HWND, ScintillaEditView**)
    _goToLineDlg.init(nullptr, nullptr, &_pEditView);
    _goToLineDlg.doDialog(false);
}

void Notepad_plus::findMatchingBracePos(intptr_t& braceAtCaret, intptr_t& braceOpposite)
{
    if (!_pEditView)
    {
        braceAtCaret = -1;
        braceOpposite = -1;
        return;
    }

    intptr_t currentPos = _pEditView->execute(SCI_GETCURRENTPOS);
    char charBefore = static_cast<char>(_pEditView->execute(SCI_GETCHARAT, currentPos - 1));
    char charAfter = static_cast<char>(_pEditView->execute(SCI_GETCHARAT, currentPos));

    // Define matching braces
    const char* openBraces = "([{<";
    const char* closeBraces = ")]}>";

    braceAtCaret = -1;
    braceOpposite = -1;

    // Check character before caret
    const char* match = strchr(openBraces, charBefore);
    if (match)
    {
        braceAtCaret = currentPos - 1;
        braceOpposite = _pEditView->execute(SCI_BRACEMATCH, braceAtCaret);
        return;
    }

    match = strchr(closeBraces, charBefore);
    if (match)
    {
        braceAtCaret = currentPos - 1;
        braceOpposite = _pEditView->execute(SCI_BRACEMATCH, braceAtCaret);
        return;
    }

    // Check character at caret
    match = strchr(openBraces, charAfter);
    if (match)
    {
        braceAtCaret = currentPos;
        braceOpposite = _pEditView->execute(SCI_BRACEMATCH, braceAtCaret);
        return;
    }

    match = strchr(closeBraces, charAfter);
    if (match)
    {
        braceAtCaret = currentPos;
        braceOpposite = _pEditView->execute(SCI_BRACEMATCH, braceAtCaret);
        return;
    }
}

void Notepad_plus::markAll(const wchar_t* text, int styleID)
{
    if (!text || !_pEditView)
        return;

    std::wstring searchText(text);
    if (searchText.empty())
        return;

    // Use the find/replace dialog's markAll function
    _findReplaceDlg.markAll(searchText.c_str(), styleID);
}

bool Notepad_plus::goToNextIndicator(int indicID2Search, bool isWrap) const
{
    if (!_pEditView)
        return false;

    intptr_t currentPos = _pEditView->execute(SCI_GETCURRENTPOS);
    intptr_t endPos = _pEditView->execute(SCI_GETTEXTLENGTH);

    _pEditView->execute(SCI_SETINDICATORCURRENT, indicID2Search);

    // Search forward for the indicator
    intptr_t pos = currentPos;
    while (pos < endPos)
    {
        intptr_t value = _pEditView->execute(SCI_INDICATORVALUEAT, indicID2Search, pos);
        if (value > 0)
        {
            // Found indicator - jump to it
            _pEditView->execute(SCI_GOTOPOS, pos);
            return true;
        }
        pos++;
    }

    // Wrap around if enabled
    if (isWrap)
    {
        pos = 0;
        while (pos < currentPos)
        {
            intptr_t value = _pEditView->execute(SCI_INDICATORVALUEAT, indicID2Search, pos);
            if (value > 0)
            {
                _pEditView->execute(SCI_GOTOPOS, pos);
                return true;
            }
            pos++;
        }
    }

    return false;
}

bool Notepad_plus::goToPreviousIndicator(int indicID2Search, bool isWrap) const
{
    if (!_pEditView)
        return false;

    intptr_t currentPos = _pEditView->execute(SCI_GETCURRENTPOS);

    _pEditView->execute(SCI_SETINDICATORCURRENT, indicID2Search);

    // Search backward for the indicator
    intptr_t pos = currentPos;
    while (pos >= 0)
    {
        intptr_t value = _pEditView->execute(SCI_INDICATORVALUEAT, indicID2Search, pos);
        if (value > 0)
        {
            // Found indicator - jump to it
            _pEditView->execute(SCI_GOTOPOS, pos);
            return true;
        }
        pos--;
    }

    // Wrap around if enabled
    if (isWrap)
    {
        intptr_t endPos = _pEditView->execute(SCI_GETTEXTLENGTH);
        pos = endPos - 1;
        while (pos > currentPos)
        {
            intptr_t value = _pEditView->execute(SCI_INDICATORVALUEAT, indicID2Search, pos);
            if (value > 0)
            {
                _pEditView->execute(SCI_GOTOPOS, pos);
                return true;
            }
            pos--;
        }
    }

    return false;
}

void Notepad_plus::showFindCharsInRangeDlg()
{
    // Show the Find Characters in Range dialog
    _findCharsInRangeDlg.doDialog(false);
}

// ============================================================================
// Panel Switching Operations
// ============================================================================

void Notepad_plus::switchToFunctionList()
{
    // Switch focus to the function list panel
    if (_pFuncList)
    {
        // Activate the function list panel
        // For Qt, this would involve focusing the panel widget
        // The actual implementation depends on the docking manager
    }
}

void Notepad_plus::switchToDocumentList()
{
    // Switch focus to the document list panel
    if (_pDocumentListPanel)
    {
        // Activate the document list panel
        // For Qt, this would involve focusing the panel widget
    }
}

// ============================================================================
// View and Document Activation Operations
// ============================================================================

int Notepad_plus::switchEditViewTo(int gid)
{
    // Switch to the specified edit view (MAIN_VIEW or SUB_VIEW)
    if (gid == MAIN_VIEW)
    {
        if (_mainWindowStatus & WindowMainActive)
        {
            _activeView = MAIN_VIEW;
            _pEditView = &_mainEditView;
            _pDocTab = &_mainDocTab;
            _pNonEditView = &_subEditView;
            _pNonDocTab = &_subDocTab;
            // Update UI focus using Scintilla's focus command
            _pEditView->execute(SCI_GRABFOCUS);
        }
    }
    else if (gid == SUB_VIEW)
    {
        if (_mainWindowStatus & WindowSubActive)
        {
            _activeView = SUB_VIEW;
            _pEditView = &_subEditView;
            _pDocTab = &_subDocTab;
            _pNonEditView = &_mainEditView;
            _pNonDocTab = &_mainDocTab;
            // Update UI focus using Scintilla's focus command
            _pEditView->execute(SCI_GRABFOCUS);
        }
    }
    return _activeView;
}

void Notepad_plus::activateDoc(size_t pos)
{
    // Activate document at specified position in current view
    if (_pDocTab && pos < _pDocTab->nbItem())
    {
        BufferID id = _pDocTab->getBufferByIndex(pos);
        if (id != BUFFER_INVALID)
        {
            switchToFile(id);
        }
    }
}

void Notepad_plus::activateNextDoc(bool direction)
{
    // Activate next/previous document in current view
    if (!_pDocTab)
        return;

    size_t currentIndex = _pDocTab->getCurrentTabIndex();
    size_t nbItems = _pDocTab->nbItem();

    if (nbItems <= 1)
        return;

    size_t newIndex;
    if (direction)
    {
        // Next tab
        newIndex = (currentIndex + 1) % nbItems;
    }
    else
    {
        // Previous tab
        newIndex = (currentIndex == 0) ? nbItems - 1 : currentIndex - 1;
    }

    activateDoc(newIndex);
}

// ============================================================================
// Tab Movement Operations
// ============================================================================

void Notepad_plus::moveTabForward()
{
    // Move current tab forward in the tab bar
    if (!_pDocTab)
        return;

    int currentIndex = _pDocTab->getCurrentTabIndex();
    if (currentIndex < 0)
        return;

    size_t nbItems = _pDocTab->nbItem();
    if (nbItems <= 1 || static_cast<size_t>(currentIndex) >= nbItems - 1)
        return;

    // Use TabBarPlus's exchangeTabItemData to swap with next tab
    // This is a simplified implementation - just swap the buffer IDs
    BufferID currentID = _pDocTab->getBufferByIndex(currentIndex);
    BufferID nextID = _pDocTab->getBufferByIndex(currentIndex + 1);

    // Set the buffers in swapped positions
    _pDocTab->setBuffer(currentIndex, nextID);
    _pDocTab->setBuffer(currentIndex + 1, currentID);
    _pDocTab->activateBuffer(currentID);
}

void Notepad_plus::moveTabBackward()
{
    // Move current tab backward in the tab bar
    if (!_pDocTab)
        return;

    int currentIndex = _pDocTab->getCurrentTabIndex();
    if (currentIndex <= 0)
        return;

    size_t nbItems = _pDocTab->nbItem();
    if (nbItems <= 1)
        return;

    // Swap with previous tab
    BufferID currentID = _pDocTab->getBufferByIndex(currentIndex);
    BufferID prevID = _pDocTab->getBufferByIndex(currentIndex - 1);

    // Set the buffers in swapped positions
    _pDocTab->setBuffer(currentIndex, prevID);
    _pDocTab->setBuffer(currentIndex - 1, currentID);
    _pDocTab->activateBuffer(currentID);
}

// ============================================================================
// Macro Operations
// ============================================================================

void Notepad_plus::startMacroRecording()
{
    // Start recording a macro
    if (_recordingMacro)
        return;

    _recordingMacro = true;
    _macro.clear();
    _recordingSaved = false;

    // Notify Scintilla to start recording
    _pEditView->execute(SCI_STARTRECORD);
}

void Notepad_plus::stopMacroRecording()
{
    // Stop recording a macro
    if (!_recordingMacro)
        return;

    _recordingMacro = false;
    _pEditView->execute(SCI_STOPRECORD);

    // If macro is not empty, mark it as saved
    if (!_macro.empty())
    {
        _recordingSaved = true;
    }
}

void Notepad_plus::macroPlayback()
{
    // Playback the recorded macro
    if (_recordingMacro || _macro.empty())
        return;

    _playingBackMacro = true;

    // Execute each macro step
    for (size_t i = 0; i < _macro.size(); ++i)
    {
        const recordedMacroStep& step = _macro[i];
        if (step.isScintillaMacro())
        {
            _pEditView->execute(step._message, step._wParameter, step._lParameter);
        }
        // TODO: Handle menu commands and other macro types
    }

    _playingBackMacro = false;
}

void Notepad_plus::saveCurrentMacro()
{
    // Save the current macro
    if (_macro.empty())
        return;

    // Show dialog to save macro with a name and shortcut
    // This would typically open a dialog for the user to name the macro
    // and assign a keyboard shortcut
    // For now, just add it to the macro list
    addCurrentMacro();
}

void Notepad_plus::showRunMacroDlg()
{
    // Show the Run Macro dialog
    _runMacroDlg.doDialog(false);
}

// ============================================================================
// Encoding and Dialog Operations
// ============================================================================

void Notepad_plus::setEncoding(int encoding)
{
    // Set the encoding for the current buffer
    Buffer* buf = getCurrentBuffer();
    if (!buf)
        return;

    UniMode mode = uni8Bit;
    switch (encoding)
    {
        case 0: // ANSI
            mode = uni8Bit;
            break;
        case 1: // UTF-8
            mode = uniUTF8;
            break;
        case 2: // UTF-16 BE
            mode = uni16BE;
            break;
        case 3: // UTF-16 LE
            mode = uni16LE;
            break;
        case 4: // UTF-8 without BOM
            mode = uniUTF8_NoBOM;
            break;
        default:
            return;
    }

    buf->setUnicodeMode(mode);
    // Update UI to reflect encoding change
}

void Notepad_plus::showUserDefineDlg()
{
    // Show the User Defined Language dialog
    // This dialog allows users to define custom language syntax highlighting
    // For Qt, this would show the UserDefineDialog
    // TODO: Implement when UserDefineDialog is fully ported
}

void Notepad_plus::showRunDlg()
{
    // Show the Run dialog
    _runDlg.doDialog(false);
}

void Notepad_plus::showPreferenceDlg()
{
    // Show the Preferences dialog
    _preference.doDialog(false);
}

// ============================================================================
// Panel Launching Operations
// ============================================================================

void Notepad_plus::launchDocumentListPanel(bool changeFromBtnCmd)
{
    Q_UNUSED(changeFromBtnCmd);

    // TODO: Implement Document List panel launch for Qt
    // This would create and show the VerticalFileSwitcher panel
    // For now, this is a stub that will be implemented when the panel is fully ported

    if (!_pDocumentListPanel)
    {
        // Create the document list panel
        // _pDocumentListPanel = new QtControls::DocumentListPanel(...);
        // Initialize and dock the panel
    }

    if (_pDocumentListPanel)
    {
        // Show/activate the panel
        // _pDocumentListPanel->display();
    }
}

void Notepad_plus::launchDocMap()
{
    // TODO: Implement Document Map panel launch for Qt
    // This would create and show the DocumentMap panel

    if (!_pDocMap)
    {
        // Create the document map panel
        // _pDocMap = new QtControls::DocumentMap(...);
        // _pDocMap->init(&_pEditView);
        // Initialize and dock the panel
    }

    if (_pDocMap)
    {
        // Show/activate the panel
        // _pDocMap->display();
        // _pDocMap->initWrapMap();
        // _pDocMap->wrapMap();
    }
}

void Notepad_plus::launchFunctionList()
{
    // TODO: Implement Function List panel launch for Qt
    // This would create and show the FunctionListPanel

    if (!_pFuncList)
    {
        // Create the function list panel
        // _pFuncList = new QtControls::FunctionListPanel(...);
        // _pFuncList->init(&_pEditView);
        // Initialize and dock the panel
    }

    if (_pFuncList)
    {
        // Show/activate the panel
        // _pFuncList->display();
    }
}

void Notepad_plus::launchProjectPanel(int cmdID, ProjectPanel** ppProjPanel, int panelID)
{
    Q_UNUSED(cmdID);
    Q_UNUSED(panelID);

    // TODO: Implement Project Panel launch for Qt
    // This would create and show the ProjectPanel

    if (!(*ppProjPanel))
    {
        // Create the project panel
        // (*ppProjPanel) = new QtControls::ProjectPanel(...);
        // (*ppProjPanel)->init(&_pEditView);
        // (*ppProjPanel)->setWorkSpaceFilePath(...);
        // Initialize and dock the panel
    }
    else
    {
        // Panel already exists, open workspace if needed
        // (*ppProjPanel)->openWorkSpace(...);
    }

    if (*ppProjPanel)
    {
        // Show/activate the panel
        // (*ppProjPanel)->display();
        // Update menu state
        // checkMenuItem(cmdID, true);
        // checkProjectMenuItem();
        // (*ppProjPanel)->setClosed(false);
    }
}

// ============================================================================
// Macro Operations
// ============================================================================

bool Notepad_plus::addCurrentMacro()
{
    // TODO: Implement add current macro for Qt
    // This would save the currently recorded macro with a name and shortcut

    if (_macro.empty())
        return false;

    // Show dialog to save macro with name and shortcut
    // For now, just mark as saved
    _recordingSaved = true;

    return true;
}

// ============================================================================
// Session Operations
// ============================================================================

void Notepad_plus::loadLastSession()
{
    NppParameters& nppParams = NppParameters::getInstance();
    const NppGUI& nppGui = nppParams.getNppGUI();
    Session lastSession = nppParams.getSession();
    bool isSnapshotMode = nppGui.isSnapshotMode();
    _isFolding = true;
    loadSession(lastSession, isSnapshotMode);
    _isFolding = false;
}

// ============================================================================
// View Visibility Operations
// ============================================================================

void Notepad_plus::showView(int whichOne)
{
    if (viewVisible(whichOne))  //no use making visible view visible
        return;

    // For Qt, we need to show the appropriate view
    // The main window status tracks which views are active
    if (whichOne == MAIN_VIEW)
    {
        _mainWindowStatus |= WindowMainActive;
        _mainEditView.display(true);
        _mainDocTab.display(true);
    }
    else if (whichOne == SUB_VIEW)
    {
        _mainWindowStatus |= WindowSubActive;
        _subEditView.display(true);
        _subDocTab.display(true);
    }

    // Update the splitter layout - Qt handles this automatically
    // when visibility changes via the layout system
}

bool Notepad_plus::viewVisible(int whichOne)
{
    int viewToCheck = (whichOne == SUB_VIEW ? WindowSubActive : WindowMainActive);
    return (_mainWindowStatus & viewToCheck) != 0;
}

// ============================================================================
// Session Loading
// ============================================================================

bool Notepad_plus::loadSession(Session& session, bool isSnapshotMode, const wchar_t* userCreatedSessionName)
{
    Q_UNUSED(userCreatedSessionName);

    NppParameters& nppParam = NppParameters::getInstance();
    const NppGUI& nppGUI = nppParam.getNppGUI();

    nppParam.setTheWarningHasBeenGiven(false);

    bool allSessionFilesLoaded = true;
    BufferID lastOpened = BUFFER_INVALID;

    showView(MAIN_VIEW);
    switchEditViewTo(MAIN_VIEW);

    // If no session files, just set up RTL if needed
    if (!session.nbMainFiles() && !session.nbSubFiles())
    {
        Buffer* buf = getCurrentBuffer();
        if (nppParam.getNativeLangSpeaker()->isRTL() && nppParam.getNativeLangSpeaker()->isEditZoneRTL())
            buf->setRTL(true);
        _mainEditView.changeTextDirection(buf->isRTL());
        return true;
    }

    // Load main view files
    for (size_t i = 0; i < session.nbMainFiles(); )
    {
        const wchar_t* pFn = session._mainViewFiles[i]._fileName.c_str();

        if (isFileSession(pFn) || isFileWorkspace(pFn))
        {
            session._mainViewFiles.erase(session._mainViewFiles.begin() + i);
            continue;
        }

        // Check if file exists
        if (QFile::exists(QString::fromStdWString(pFn)))
        {
            if (isSnapshotMode && !session._mainViewFiles[i]._backupFilePath.empty())
                lastOpened = doOpen(pFn, false, false, session._mainViewFiles[i]._encoding,
                                   session._mainViewFiles[i]._backupFilePath.c_str(),
                                   session._mainViewFiles[i]._originalFileLastModifTimestamp);
            else
                lastOpened = doOpen(pFn, false, false, session._mainViewFiles[i]._encoding);
        }
        else if (isSnapshotMode && !session._mainViewFiles[i]._backupFilePath.empty() &&
                 QFile::exists(QString::fromStdWString(session._mainViewFiles[i]._backupFilePath)))
        {
            lastOpened = doOpen(pFn, false, false, session._mainViewFiles[i]._encoding,
                               session._mainViewFiles[i]._backupFilePath.c_str(),
                               session._mainViewFiles[i]._originalFileLastModifTimestamp);
        }
        else
        {
            // File doesn't exist - try to find if already open or create placeholder
            BufferID foundBufID = MainFileManager.getBufferFromName(pFn);
            if (foundBufID == BUFFER_INVALID)
            {
                // For now, skip absent files (placeholder documents not implemented)
                lastOpened = BUFFER_INVALID;
            }
        }

        if (lastOpened != BUFFER_INVALID)
        {
            showView(MAIN_VIEW);
            Buffer* buf = MainFileManager.getBufferByID(lastOpened);

            // Set language type
            const wchar_t* pLn = session._mainViewFiles[i]._langName.c_str();
            LangType langTypeToSet = L_TEXT;

            // Try to determine language from menu name
            // Simplified: just use text for now
            if (pLn && *pLn)
            {
                // TODO: Map language name to LangType
                langTypeToSet = L_TEXT;
            }

            // Set position and other properties
            buf->setPosition(session._mainViewFiles[i], &_mainEditView);
            buf->setLangType(langTypeToSet);
            // TODO: Handle user language name (pLn) if needed
            if (session._mainViewFiles[i]._encoding != -1)
                buf->setEncodingNumber(session._mainViewFiles[i]._encoding);

            buf->setUserReadOnly(session._mainViewFiles[i]._isUserReadOnly ||
                                nppGUI._isFullReadOnly ||
                                nppGUI._isFullReadOnlySavingForbidden);
            buf->setPinned(session._mainViewFiles[i]._isPinned);
            // Note: setUntitledTabRenamedStatus not implemented in Qt version

            if (isSnapshotMode && !session._mainViewFiles[i]._backupFilePath.empty())
                buf->setDirty(true);

            buf->setRTL(session._mainViewFiles[i]._isRTL);
            if (i == 0 && session._activeMainIndex == 0)
                _mainEditView.changeTextDirection(buf->isRTL());

            ++i;
        }
        else
        {
            session._mainViewFiles.erase(session._mainViewFiles.begin() + i);
            allSessionFilesLoaded = false;
        }
    }

    // Load sub view files
    showView(SUB_VIEW);
    switchEditViewTo(SUB_VIEW);

    for (size_t k = 0; k < session.nbSubFiles(); )
    {
        const wchar_t* pFn = session._subViewFiles[k]._fileName.c_str();

        if (isFileSession(pFn) || isFileWorkspace(pFn))
        {
            session._subViewFiles.erase(session._subViewFiles.begin() + k);
            continue;
        }

        if (QFile::exists(QString::fromStdWString(pFn)) ||
            (isSnapshotMode && !session._subViewFiles[k]._backupFilePath.empty() &&
             QFile::exists(QString::fromStdWString(session._subViewFiles[k]._backupFilePath))))
        {
            // Check if already open in main view - if so, clone it
            BufferID clonedBuf = _mainDocTab.findBufferByName(pFn);
            if (clonedBuf != BUFFER_INVALID)
            {
                loadBufferIntoView(clonedBuf, SUB_VIEW);
                lastOpened = clonedBuf;
            }
            else
            {
                if (isSnapshotMode && !session._subViewFiles[k]._backupFilePath.empty())
                    lastOpened = doOpen(pFn, false, false, session._subViewFiles[k]._encoding,
                                       session._subViewFiles[k]._backupFilePath.c_str(),
                                       session._subViewFiles[k]._originalFileLastModifTimestamp);
                else
                    lastOpened = doOpen(pFn, false, false, session._subViewFiles[k]._encoding);
            }
        }
        else
        {
            BufferID foundBufID = MainFileManager.getBufferFromName(pFn);
            if (foundBufID == BUFFER_INVALID)
            {
                lastOpened = BUFFER_INVALID;
            }
        }

        if (lastOpened != BUFFER_INVALID)
        {
            showView(SUB_VIEW);
            Buffer* buf = MainFileManager.getBufferByID(lastOpened);

            const wchar_t* pLn = session._subViewFiles[k]._langName.c_str();
            LangType typeToSet = L_TEXT;

            buf->setPosition(session._subViewFiles[k], &_subEditView);
            buf->setLangType(typeToSet);
            // TODO: Handle user language name (pLn) if needed
            buf->setEncodingNumber(session._subViewFiles[k]._encoding);
            buf->setUserReadOnly(session._subViewFiles[k]._isUserReadOnly ||
                                nppGUI._isFullReadOnly ||
                                nppGUI._isFullReadOnlySavingForbidden);
            buf->setPinned(session._subViewFiles[k]._isPinned);
            // Note: setUntitledTabRenamedStatus not implemented in Qt version

            if (isSnapshotMode && !session._subViewFiles[k]._backupFilePath.empty())
                buf->setDirty(true);

            buf->setRTL(session._subViewFiles[k]._isRTL);

            ++k;
        }
        else
        {
            session._subViewFiles.erase(session._subViewFiles.begin() + k);
            allSessionFilesLoaded = false;
        }
    }

    // Activate the appropriate files
    if (session._activeMainIndex < session._mainViewFiles.size())
    {
        const wchar_t* fileName = session._mainViewFiles[session._activeMainIndex]._fileName.c_str();
        BufferID buf = _mainDocTab.findBufferByName(fileName);
        if (buf != BUFFER_INVALID)
            activateBuffer(buf, MAIN_VIEW);
    }

    if (session._activeSubIndex < session._subViewFiles.size())
    {
        const wchar_t* fileName = session._subViewFiles[session._activeSubIndex]._fileName.c_str();
        BufferID buf = _subDocTab.findBufferByName(fileName);
        if (buf != BUFFER_INVALID)
            activateBuffer(buf, SUB_VIEW);
    }

    // Switch to the active view
    if ((session.nbSubFiles() > 0) &&
        (session._activeView == MAIN_VIEW || session._activeView == SUB_VIEW))
        switchEditViewTo(static_cast<int32_t>(session._activeView));
    else
        switchEditViewTo(MAIN_VIEW);

    // Hide empty views
    if (canHideView(otherView()))
        hideView(otherView());
    else if (canHideView(currentView()))
        hideView(currentView());

    checkSyncState();

    return allSessionFilesLoaded;
}

#endif // NPP_LINUX
