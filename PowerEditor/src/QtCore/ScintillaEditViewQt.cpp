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
// ScintillaEditViewQt.cpp - Linux/Qt implementations of ScintillaEditView methods
// ============================================================================
// This file provides Linux-compatible implementations of ScintillaEditView
// methods that are excluded from the Windows build. The Windows implementation
// is in ScintillaComponent/ScintillaEditView.cpp and uses Windows-specific APIs.
//
// These implementations use the Scintilla Qt port (scintilla/qt/ScintillaEditBase)
// and are compatible with the Qt-based Linux build.
// ============================================================================

// This file contains Linux/Qt-specific implementations that don't compile on Windows
// These implementations use the Scintilla Qt port for the Linux build
#ifdef NPP_LINUX

#include "ScintillaEditView.h"
#include "Parameters.h"
#include "Common.h"

#include <algorithm>
#include <vector>
#include <string>
#include <unordered_set>
#include <cctype>
#include <cwctype>

using namespace std;

// ============================================================================
// Selection and Column Mode Operations
// ============================================================================

void ScintillaEditView::beginOrEndSelect(bool isColumnMode)
{
    auto currPos = execute(SCI_GETCURRENTPOS);

    if (_beginSelectPosition == -1)
    {
        _beginSelectPosition = currPos;
    }
    else
    {
        execute(SCI_CHANGESELECTIONMODE, static_cast<WPARAM>(isColumnMode ? SC_SEL_RECTANGLE : SC_SEL_STREAM));
        execute(isColumnMode ? SCI_SETANCHOR : SCI_SETSEL, static_cast<WPARAM>(_beginSelectPosition), static_cast<LPARAM>(currPos));
        _beginSelectPosition = -1;
    }
}

// ============================================================================
// Line Indentation Operations
// ============================================================================

void ScintillaEditView::setLineIndent(size_t line, size_t indent) const
{
    size_t nbSelections = execute(SCI_GETSELECTIONS);

    if (nbSelections == 1)
    {
        Sci_CharacterRangeFull crange = getSelection();
        int64_t posBefore = execute(SCI_GETLINEINDENTPOSITION, line);
        execute(SCI_SETLINEINDENTATION, line, indent);
        int64_t posAfter = execute(SCI_GETLINEINDENTPOSITION, line);
        long long posDifference = posAfter - posBefore;
        if (posAfter > posBefore)
        {
            // Move selection on
            if (crange.cpMin >= posBefore)
            {
                crange.cpMin += static_cast<Sci_Position>(posDifference);
            }
            if (crange.cpMax >= posBefore)
            {
                crange.cpMax += static_cast<Sci_Position>(posDifference);
            }
        }
        else if (posAfter < posBefore)
        {
            // Move selection back
            if (crange.cpMin >= posAfter)
            {
                if (crange.cpMin >= posBefore)
                    crange.cpMin += static_cast<Sci_Position>(posDifference);
                else
                    crange.cpMin = static_cast<Sci_Position>(posAfter);
            }

            if (crange.cpMax >= posAfter)
            {
                if (crange.cpMax >= posBefore)
                    crange.cpMax += static_cast<Sci_Position>(posDifference);
                else
                    crange.cpMax = static_cast<Sci_Position>(posAfter);
            }
        }
        execute(SCI_SETSEL, crange.cpMin, crange.cpMax);
    }
    else
    {
        execute(SCI_BEGINUNDOACTION);
        for (size_t i = 0; i < nbSelections; ++i)
        {
            LRESULT posStart = execute(SCI_GETSELECTIONNSTART, i);
            LRESULT posEnd = execute(SCI_GETSELECTIONNEND, i);

            size_t l = execute(SCI_LINEFROMPOSITION, posStart);

            int64_t posBefore = execute(SCI_GETLINEINDENTPOSITION, l);
            execute(SCI_SETLINEINDENTATION, l, indent);
            int64_t posAfter = execute(SCI_GETLINEINDENTPOSITION, l);

            long long posDifference = posAfter - posBefore;
            if (posAfter > posBefore)
            {
                // Move selection on
                if (posStart >= posBefore)
                {
                    posStart += static_cast<Sci_Position>(posDifference);
                }
                if (posEnd >= posBefore)
                {
                    posEnd += static_cast<Sci_Position>(posDifference);
                }
            }
            else if (posAfter < posBefore)
            {
                // Move selection back
                if (posStart >= posAfter)
                {
                    if (posStart >= posBefore)
                        posStart += static_cast<Sci_Position>(posDifference);
                    else
                        posStart = static_cast<Sci_Position>(posAfter);
                }

                if (posEnd >= posAfter)
                {
                    if (posEnd >= posBefore)
                        posEnd += static_cast<Sci_Position>(posDifference);
                    else
                        posEnd = static_cast<Sci_Position>(posAfter);
                }
            }

            execute(SCI_SETSELECTIONNSTART, i, posStart);
            execute(SCI_SETSELECTIONNEND, i, posEnd);
        }
        execute(SCI_ENDUNDOACTION);
    }
}

// ============================================================================
// Line Movement Operations
// ============================================================================

void ScintillaEditView::currentLinesUp() const
{
    execute(SCI_MOVESELECTEDLINESUP);
}

void ScintillaEditView::currentLinesDown() const
{
    execute(SCI_MOVESELECTEDLINESDOWN);

    // Ensure the selection is within view
    execute(SCI_SCROLLRANGE, execute(SCI_GETSELECTIONEND), execute(SCI_GETSELECTIONSTART));
}

// ============================================================================
// Word Selection Operations
// ============================================================================

pair<size_t, size_t> ScintillaEditView::getWordRange()
{
    size_t caretPos = execute(SCI_GETCURRENTPOS, 0, 0);
    size_t startPos = execute(SCI_WORDSTARTPOSITION, caretPos, true);
    size_t endPos = execute(SCI_WORDENDPOSITION, caretPos, true);
    return pair<size_t, size_t>(startPos, endPos);
}

bool ScintillaEditView::expandWordSelection()
{
    pair<size_t, size_t> wordRange = getWordRange();
    if (wordRange.first != wordRange.second)
    {
        execute(SCI_SETSELECTIONSTART, wordRange.first);
        execute(SCI_SETSELECTIONEND, wordRange.second);
        return true;
    }
    return false;
}

// ============================================================================
// Text Selection Operations
// ============================================================================

wstring ScintillaEditView::getSelectedTextToWChar(bool expand, Sci_Position* selCharNumber)
{
    WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
    size_t cp = execute(SCI_GETCODEPAGE);
    char *txtA = nullptr;

    Sci_CharacterRangeFull range = getSelection();
    if (range.cpMax == range.cpMin && expand)
    {
        expandWordSelection();
        range = getSelection();
    }

    auto selNum = execute(SCI_COUNTCHARACTERS, range.cpMin, range.cpMax);

    // return the selected string's character number
    if (selCharNumber)
        *selCharNumber = selNum;

    if (selNum == 0)
        return L"";

    // then get the selected string's total bytes (without counting the last NULL char)
    auto neededByte = execute(SCI_GETSELTEXT, 0, 0);

    txtA = new char[neededByte + 1];
    execute(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(txtA));

    const wchar_t * txtW = wmc.char2wchar(txtA, cp);
    delete [] txtA;

    return txtW;
}

// ============================================================================
// Duplicate Line Removal
// ============================================================================

void ScintillaEditView::removeAnyDuplicateLines()
{
    size_t fromLine = 0, toLine = 0;
    bool hasLineSelection = false;

    auto selStart = execute(SCI_GETSELECTIONSTART);
    auto selEnd = execute(SCI_GETSELECTIONEND);
    hasLineSelection = selStart != selEnd;

    if (hasLineSelection)
    {
        const pair<size_t, size_t> lineRange = getSelectionLinesRange();
        // One single line selection is not allowed.
        if (lineRange.first == lineRange.second)
        {
            return;
        }
        fromLine = lineRange.first;
        toLine = lineRange.second;
    }
    else
    {
        // No selection.
        fromLine = 0;
        toLine = execute(SCI_GETLINECOUNT) - 1;
    }

    if (fromLine >= toLine)
    {
        return;
    }

    const auto startPos = execute(SCI_POSITIONFROMLINE, fromLine);
    const auto endPos = execute(SCI_POSITIONFROMLINE, toLine) + execute(SCI_LINELENGTH, toLine);
    const wstring text = getGenericTextAsString(startPos, endPos);
    std::vector<wstring> linesVect;
    stringSplit(text, getEOLString(), linesVect);
    const size_t lineCount = execute(SCI_GETLINECOUNT);

    const bool doingEntireDocument = toLine == lineCount - 1;
    if (!doingEntireDocument)
    {
        if (linesVect.rbegin()->empty())
        {
            linesVect.pop_back();
        }
    }

    size_t origSize = linesVect.size();
    size_t newSize = vecRemoveDuplicates(linesVect);
    if (origSize != newSize)
    {
        wstring joined;
        stringJoin(linesVect, getEOLString(), joined);
        if (!doingEntireDocument)
        {
            joined += getEOLString();
        }
        if (text != joined)
        {
            replaceTarget(joined.c_str(), startPos, endPos);
        }
    }
}

// ============================================================================
// Search and Replace Operations
// ============================================================================

intptr_t ScintillaEditView::searchInTarget(const std::string_view& text2Find, size_t fromPos, size_t toPos) const
{
    execute(SCI_SETTARGETRANGE, fromPos, toPos);
    return execute(SCI_SEARCHINTARGET, text2Find.length(), reinterpret_cast<LPARAM>(text2Find.data()));
}

intptr_t ScintillaEditView::searchInTarget(const wchar_t* text2Find, size_t lenOfText2Find, size_t fromPos, size_t toPos) const
{
    execute(SCI_SETTARGETRANGE, fromPos, toPos);

    WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
    size_t cp = execute(SCI_GETCODEPAGE);
    const char* text2FindA = wmc.wchar2char(text2Find, cp);
    size_t text2FindALen = strlen(text2FindA);
    size_t len = (lenOfText2Find > text2FindALen) ? lenOfText2Find : text2FindALen;
    return execute(SCI_SEARCHINTARGET, len, reinterpret_cast<LPARAM>(text2FindA));
}

intptr_t ScintillaEditView::replaceTarget(const std::string& str2replace, intptr_t fromTargetPos, intptr_t toTargetPos) const
{
    if (fromTargetPos != -1 || toTargetPos != -1)
    {
        execute(SCI_SETTARGETRANGE, fromTargetPos, toTargetPos);
    }

    return execute(SCI_REPLACETARGET, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(str2replace.c_str()));
}

intptr_t ScintillaEditView::replaceTarget(const wchar_t* str2replace, intptr_t fromTargetPos, intptr_t toTargetPos) const
{
    if (fromTargetPos != -1 || toTargetPos != -1)
    {
        execute(SCI_SETTARGETRANGE, fromTargetPos, toTargetPos);
    }
    WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
    size_t cp = execute(SCI_GETCODEPAGE);
    const char* str2replaceA = wmc.wchar2char(str2replace, cp);
    return execute(SCI_REPLACETARGET, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(str2replaceA));
}

intptr_t ScintillaEditView::replaceTargetRegExMode(const wchar_t* re, intptr_t fromTargetPos, intptr_t toTargetPos) const
{
    if (fromTargetPos != -1 || toTargetPos != -1)
    {
        execute(SCI_SETTARGETRANGE, fromTargetPos, toTargetPos);
    }
    WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
    size_t cp = execute(SCI_GETCODEPAGE);
    const char* reA = wmc.wchar2char(re, cp);
    return execute(SCI_REPLACETARGETRE, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(reA));
}

// ============================================================================
// Code Folding Operations
// ============================================================================

bool ScintillaEditView::isFoldIndentationBased() const
{
    const auto lexer = execute(SCI_GETLEXER);
    // search IndentAmount in scintilla\lexers folder
    return lexer == SCLEX_PYTHON
        || lexer == SCLEX_COFFEESCRIPT
        || lexer == SCLEX_HASKELL
        || lexer == SCLEX_NIMROD
        || lexer == SCLEX_VB
        || lexer == SCLEX_YAML
    ;
}

namespace {

struct FoldLevelStack
{
    int levelCount = 0; // 1-based level number
    intptr_t levelStack[8]{}; // MAX_FOLD_COLLAPSE_LEVEL = 8

    void push(intptr_t level)
    {
        while (levelCount != 0 && level <= levelStack[levelCount - 1])
        {
            --levelCount;
        }
        levelStack[levelCount++] = level;
    }
};

}

void ScintillaEditView::foldIndentationBasedLevel(int level2Collapse, bool mode)
{
    FoldLevelStack levelStack;
    ++level2Collapse; // 1-based level number

    const intptr_t maxLine = execute(SCI_GETLINECOUNT);
    intptr_t line = 0;

    while (line < maxLine)
    {
        intptr_t level = execute(SCI_GETFOLDLEVEL, line);
        if (level & SC_FOLDLEVELHEADERFLAG)
        {
            level &= SC_FOLDLEVELNUMBERMASK;
            // don't need the actually level number, only the relationship.
            levelStack.push(level);
            if (level2Collapse == levelStack.levelCount)
            {
                if (isFolded(line) != mode)
                {
                    fold(line, mode);
                }
                // skip all children lines, required to avoid buffer overrun.
                line = execute(SCI_GETLASTCHILD, line, -1);
            }
        }
        ++line;
    }
}

void ScintillaEditView::foldAll(bool mode)
{
    execute(SCI_FOLDALL, (mode == fold_expand ? SC_FOLDACTION_EXPAND : SC_FOLDACTION_CONTRACT) | SC_FOLDACTION_CONTRACT_EVERY_LEVEL, 0);

    if (mode == fold_expand)
    {
        hideMarkedLines(0, true);
        execute(SCI_SCROLLCARET);
    }
}

void ScintillaEditView::foldCurrentPos(bool mode)
{
    auto currentLine = getCurrentLineNumber();
    fold(currentLine, mode);
}

void ScintillaEditView::foldLevel(int level2Collapse, bool mode)
{
    if (isFoldIndentationBased())
    {
        foldIndentationBasedLevel(level2Collapse, mode);
        return;
    }

    intptr_t maxLine = execute(SCI_GETLINECOUNT);

    for (int line = 0; line < maxLine; ++line)
    {
        intptr_t level = execute(SCI_GETFOLDLEVEL, line);
        if (level & SC_FOLDLEVELHEADERFLAG)
        {
            level -= SC_FOLDLEVELBASE;
            if (level2Collapse == (level & SC_FOLDLEVELNUMBERMASK))
                if (isFolded(line) != mode)
                {
                    fold(line, mode);
                }
        }
    }

    if (mode == fold_expand)
        hideMarkedLines(0, true);
}

void ScintillaEditView::fold(size_t line, bool mode, bool shouldBeNotified)
{
    auto endStyled = execute(SCI_GETENDSTYLED);
    auto len = execute(SCI_GETTEXTLENGTH);

    if (endStyled < len)
        execute(SCI_COLOURISE, 0, -1);

    intptr_t headerLine;
    auto level = execute(SCI_GETFOLDLEVEL, line);

    if (level & SC_FOLDLEVELHEADERFLAG)
        headerLine = line;
    else
    {
        headerLine = execute(SCI_GETFOLDPARENT, line);
        if (headerLine == -1)
            return;
    }

    if (isFolded(headerLine) != mode)
    {
        execute(SCI_TOGGLEFOLD, headerLine);

        if (shouldBeNotified)
        {
            // Notification handled by Scintilla
        }
    }
}

bool ScintillaEditView::isCurrentLineFolded() const
{
    auto currentLine = getCurrentLineNumber();

    intptr_t headerLine;
    auto level = execute(SCI_GETFOLDLEVEL, currentLine);

    if (level & SC_FOLDLEVELHEADERFLAG)
        headerLine = currentLine;
    else
    {
        headerLine = execute(SCI_GETFOLDPARENT, currentLine);
        if (headerLine == -1)
            return false;
    }

    bool isExpanded = execute(SCI_GETFOLDEXPANDED, headerLine);
    return !isExpanded;
}

void ScintillaEditView::expand(size_t& line, bool doExpand, bool force, intptr_t visLevels, intptr_t level)
{
    size_t lineMaxSubord = execute(SCI_GETLASTCHILD, line, level & SC_FOLDLEVELNUMBERMASK);
    ++line;
    while (line <= lineMaxSubord)
    {
        if (force)
        {
            execute(((visLevels > 0) ? SCI_SHOWLINES : SCI_HIDELINES), line, line);
        }
        else
        {
            if (doExpand)
                execute(SCI_SHOWLINES, line, line);
        }

        intptr_t levelLine = level;
        if (levelLine == -1)
            levelLine = execute(SCI_GETFOLDLEVEL, line, 0);

        if (levelLine & SC_FOLDLEVELHEADERFLAG)
        {
            if (force)
            {
                if (visLevels > 1)
                    execute(SCI_SETFOLDEXPANDED, line, 1);
                else
                    execute(SCI_SETFOLDEXPANDED, line, 0);
                expand(line, doExpand, force, visLevels - 1);
            }
            else
            {
                if (doExpand)
                {
                    if (!isFolded(line))
                        execute(SCI_SETFOLDEXPANDED, line, 1);

                    expand(line, true, force, visLevels - 1);
                }
                else
                    expand(line, false, force, visLevels - 1);
            }
        }
        else
        {
            ++line;
        }
    }
}

// ============================================================================
// Hide Lines Operations
// ============================================================================

void ScintillaEditView::hideLines()
{
    // Unfolding can screw up hide lines badly if it unfolds a hidden section.
    // Using hideMarkedLines() after unfolding can help

    size_t startLine = execute(SCI_LINEFROMPOSITION, execute(SCI_GETSELECTIONSTART));
    size_t endLine = execute(SCI_LINEFROMPOSITION, execute(SCI_GETSELECTIONEND));

    // perform range check: cannot hide very first and very last lines
    // Offset them one off the edges, and then check if they are within the reasonable
    size_t nbLines = execute(SCI_GETLINECOUNT);
    if (nbLines < 3)
        return; // cannot possibly hide anything

    if (!startLine)
        ++startLine;

    if (endLine == (nbLines - 1))
        --endLine;

    if (startLine > endLine)
        return; // tried to hide line at edge

    int scope = 0;
    bool recentMarkerWasOpen = false;

    auto removeMarker = [this, &scope, &recentMarkerWasOpen](size_t line, int markerMask)
    {
        auto state = execute(SCI_MARKERGET, line) & markerMask;
        bool closePresent = (state & (1 << MARK_HIDELINESEND)) != 0;
        bool openPresent = (state & (1 << MARK_HIDELINESBEGIN)) != 0;

        if (closePresent)
        {
            execute(SCI_MARKERDELETE, line, MARK_HIDELINESEND);
            recentMarkerWasOpen = false;
            --scope;
        }

        if (openPresent)
        {
            execute(SCI_MARKERDELETE, line, MARK_HIDELINESBEGIN);
            recentMarkerWasOpen = true;
            ++scope;
        }
    };

    size_t startMarker = startLine - 1;
    size_t endMarker = endLine + 1;

    // Previous markers must be removed in the selected region:

    removeMarker(startMarker, 1 << MARK_HIDELINESBEGIN);

    for (size_t i = startLine; i <= endLine; ++i)
        removeMarker(i, (1 << MARK_HIDELINESBEGIN) | (1 << MARK_HIDELINESEND));

    removeMarker(endMarker, 1 << MARK_HIDELINESEND);

    // When hiding lines just below/above other hidden lines,
    // merge them into one hidden section:

    if (scope == 0 && recentMarkerWasOpen)
    {
        // Special case: user wants to hide every line in between other hidden sections.
        // Both "while" loops are executed (merge with above AND below hidden section):

        while (scope == 0 && static_cast<intptr_t>(startMarker) >= 0)
            removeMarker(--startMarker, 1 << MARK_HIDELINESBEGIN);

        while (scope != 0 && endMarker < nbLines)
            removeMarker(++endMarker, 1 << MARK_HIDELINESEND);
    }
    else
    {
        // User wants to hide some lines below/above other hidden section.
        // If true, only one "while" loop is executed (merge with adjacent hidden section):

        while (scope < 0 && static_cast<intptr_t>(startMarker) >= 0)
            removeMarker(--startMarker, 1 << MARK_HIDELINESBEGIN);

        while (scope > 0 && endMarker < nbLines)
            removeMarker(++endMarker, 1 << MARK_HIDELINESEND);
    }

    execute(SCI_MARKERADD, startMarker, MARK_HIDELINESBEGIN);
    execute(SCI_MARKERADD, endMarker, MARK_HIDELINESEND);

    _currentBuffer->setHideLineChanged(true, startMarker);
}

bool ScintillaEditView::hidelineMarkerClicked(intptr_t lineNumber)
{
    auto state = execute(SCI_MARKERGET, lineNumber);
    bool openPresent = (state & (1 << MARK_HIDELINESBEGIN)) != 0;
    bool closePresent = (state & (1 << MARK_HIDELINESEND)) != 0;

    if (!openPresent && !closePresent)
        return false;

    // First call show with location of opening marker. Then remove the marker manually
    if (openPresent)
    {
        showHiddenLines(lineNumber, false, true);
    }
    else if (closePresent)
    {
        // Find the opening marker by searching backwards
        intptr_t openingLine = lineNumber;
        while (openingLine >= 0)
        {
            auto markerState = execute(SCI_MARKERGET, openingLine);
            if (markerState & (1 << MARK_HIDELINESBEGIN))
                break;
            --openingLine;
        }
        if (openingLine >= 0)
            showHiddenLines(openingLine, false, true);
    }

    return true;
}

void ScintillaEditView::notifyHidelineMarkers(Buffer* buf, bool isHide, size_t location, bool del)
{
    // Notify buffer about hide line changes
    if (buf)
    {
        buf->setHideLineChanged(isHide, location);
    }
}

void ScintillaEditView::hideMarkedLines(size_t searchStart, bool endOfDoc)
{
    size_t maxLines = execute(SCI_GETLINECOUNT);

    auto startHiding = searchStart;
    bool isInSection = false;

    for (auto i = searchStart; i < maxLines; ++i)
    {
        auto state = execute(SCI_MARKERGET, i);
        if (((state & (1 << MARK_HIDELINESEND)) != 0))
        {
            if (isInSection)
            {
                execute(SCI_HIDELINES, startHiding, i - 1);
                if (!endOfDoc)
                {
                    return; // done, only single section requested
                } // otherwise keep going
            }
            isInSection = false;
        }

        if ((state & (1 << MARK_HIDELINESBEGIN)) != 0)
        {
            startHiding = i + 1;
            isInSection = true;
        }
    }

    // If we reached the end and are still in a section, hide till the end
    if (isInSection && endOfDoc)
    {
        execute(SCI_HIDELINES, startHiding, maxLines - 1);
    }
}

void ScintillaEditView::showHiddenLines(size_t searchStart, bool endOfDoc, bool doDelete)
{
    size_t maxLines = execute(SCI_GETLINECOUNT);

    for (auto i = searchStart; i < maxLines; ++i)
    {
        auto state = execute(SCI_MARKERGET, i);
        if ((state & (1 << MARK_HIDELINESBEGIN)) != 0)
        {
            // Found start marker, now find the matching end marker
            size_t startLine = i + 1;
            size_t endLine = startLine;

            for (auto j = startLine; j < maxLines; ++j)
            {
                auto innerState = execute(SCI_MARKERGET, j);
                if ((innerState & (1 << MARK_HIDELINESEND)) != 0)
                {
                    endLine = j - 1;
                    break;
                }
            }

            // Show the hidden lines
            execute(SCI_SHOWLINES, startLine, endLine);

            if (doDelete)
            {
                execute(SCI_MARKERDELETE, i, MARK_HIDELINESBEGIN);
                execute(SCI_MARKERDELETE, endLine + 1, MARK_HIDELINESEND);
            }

            if (!endOfDoc)
                return;
        }
    }
}

void ScintillaEditView::restoreHiddenLines()
{
    // Restore all hidden lines by showing them and removing markers
    size_t maxLines = execute(SCI_GETLINECOUNT);

    for (auto i = 0; i < maxLines; ++i)
    {
        auto state = execute(SCI_MARKERGET, i);

        if ((state & (1 << MARK_HIDELINESBEGIN)) != 0)
        {
            execute(SCI_MARKERDELETE, i, MARK_HIDELINESBEGIN);
        }

        if ((state & (1 << MARK_HIDELINESEND)) != 0)
        {
            execute(SCI_MARKERDELETE, i, MARK_HIDELINESEND);
        }
    }

    // Show all lines
    execute(SCI_SHOWLINES, 0, maxLines - 1);
}

// ============================================================================
// Generic Text Retrieval
// ============================================================================

void ScintillaEditView::getGenericText(wchar_t* dest, size_t destlen, size_t start, size_t end) const
{
    WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
    char* destA = new char[end - start + 1];
    getText(destA, start, end);
    size_t cp = execute(SCI_GETCODEPAGE);
    const wchar_t* destW = wmc.char2wchar(destA, cp);

    // Safe string copy
    size_t lenW = wcslen(destW);
    if (lenW >= destlen)
        lenW = destlen - 1;
    wmemcpy(dest, destW, lenW);
    dest[lenW] = L'\0';

    delete[] destA;
}

wstring ScintillaEditView::getGenericTextAsString(size_t start, size_t end) const
{
    if (end <= start)
        return L"";
    const size_t bufSize = end - start + 1;
    wchar_t* buf = new wchar_t[bufSize];
    getGenericText(buf, bufSize, start, end);
    wstring text = buf;
    delete[] buf;
    return text;
}

// ============================================================================
// Text Case Conversion
// ============================================================================

void ScintillaEditView::convertSelectedTextTo(const TextCase& caseToConvert)
{
    // Get the selection range
    Sci_CharacterRangeFull range = getSelection();
    if (range.cpMin == range.cpMax)
        return; // No selection

    // Get the selected text
    wstring selectedText = getGenericTextAsString(range.cpMin, range.cpMax);
    if (selectedText.empty())
        return;

    // Convert the text case
    changeCase(const_cast<wchar_t*>(selectedText.c_str()), static_cast<int>(selectedText.length()), caseToConvert);

    // Replace the selected text with the converted text
    replaceTarget(selectedText.c_str(), range.cpMin, range.cpMax);

    // Restore the selection
    execute(SCI_SETSEL, range.cpMin, range.cpMin + selectedText.length());
}

void ScintillaEditView::changeCase(wchar_t* strWToConvert, const int& nbChars, const TextCase& caseToConvert) const
{
    if (!strWToConvert || nbChars <= 0)
        return;

    switch (caseToConvert)
    {
        case UPPERCASE:
        {
            for (int i = 0; i < nbChars; ++i)
            {
                strWToConvert[i] = static_cast<wchar_t>(toupper(strWToConvert[i]));
            }
            break;
        }
        case LOWERCASE:
        {
            for (int i = 0; i < nbChars; ++i)
            {
                strWToConvert[i] = static_cast<wchar_t>(tolower(strWToConvert[i]));
            }
            break;
        }
        case PROPERCASE_FORCE:
        case PROPERCASE_BLEND:
        {
            // Proper case: capitalize first letter of each word
            bool newWord = true;
            for (int i = 0; i < nbChars; ++i)
            {
                if (iswspace(strWToConvert[i]) || ispunct(strWToConvert[i]))
                {
                    newWord = true;
                }
                else if (newWord)
                {
                    strWToConvert[i] = static_cast<wchar_t>(toupper(strWToConvert[i]));
                    newWord = false;
                }
                else if (caseToConvert == PROPERCASE_FORCE)
                {
                    strWToConvert[i] = static_cast<wchar_t>(tolower(strWToConvert[i]));
                }
            }
            break;
        }
        default:
            break;
    }
}

// ============================================================================
// Selection Lines Range
// ============================================================================

std::pair<size_t, size_t> ScintillaEditView::getSelectionLinesRange(intptr_t selectionNumber) const
{
    size_t startPos = 0;
    size_t endPos = 0;

    if (selectionNumber < 0)
    {
        // Use the main selection
        startPos = execute(SCI_GETSELECTIONSTART);
        endPos = execute(SCI_GETSELECTIONEND);
    }
    else
    {
        // Use a specific selection
        startPos = execute(SCI_GETSELECTIONNSTART, selectionNumber);
        endPos = execute(SCI_GETSELECTIONNEND, selectionNumber);
    }

    size_t startLine = execute(SCI_LINEFROMPOSITION, startPos);
    size_t endLine = execute(SCI_LINEFROMPOSITION, endPos);

    // If the selection ends at the start of a line, don't include that line
    if (endPos == execute(SCI_POSITIONFROMLINE, endLine) && endLine > startLine)
    {
        --endLine;
    }

    return std::make_pair(startLine, endLine);
}

// ============================================================================
// EOL String
// ============================================================================

std::wstring ScintillaEditView::getEOLString() const
{
    int eolMode = static_cast<int>(execute(SCI_GETEOLMODE));
    switch (eolMode)
    {
        case SC_EOL_CRLF:
            return L"\r\n";
        case SC_EOL_CR:
            return L"\r";
        case SC_EOL_LF:
        default:
            return L"\n";
    }
}

// ============================================================================
// Text Retrieval
// ============================================================================

void ScintillaEditView::getText(char* dest, size_t start, size_t end) const
{
    if (!dest || end <= start)
        return;

    Sci_TextRangeFull tr{};
    tr.chrg.cpMin = static_cast<Sci_Position>(start);
    tr.chrg.cpMax = static_cast<Sci_Position>(end);
    tr.lpstrText = dest;
    execute(SCI_GETTEXTRANGEFULL, 0, reinterpret_cast<LPARAM>(&tr));
}

// ============================================================================
// Missing Method Implementations for Linux/Qt Build
// ============================================================================

// Static member definitions
bool ScintillaEditView::_SciInit = false;
int ScintillaEditView::_refCount = 0;
std::string ScintillaEditView::_defaultCharList = "";

const int ScintillaEditView::_SC_MARGE_LINENUMBER = 0;
const int ScintillaEditView::_SC_MARGE_SYMBOL = 1;
const int ScintillaEditView::_SC_MARGE_CHANGEHISTORY = 2;
const int ScintillaEditView::_SC_MARGE_FOLDER = 3;

const int ScintillaEditView::_markersArray[][NB_FOLDER_STATE] = {
  {SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB, SC_MARKNUM_FOLDERTAIL, SC_MARKNUM_FOLDEREND,        SC_MARKNUM_FOLDEROPENMID,     SC_MARKNUM_FOLDERMIDTAIL},
  {SC_MARK_MINUS,         SC_MARK_PLUS,      SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
  {SC_MARK_ARROWDOWN,     SC_MARK_ARROW,     SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
  {SC_MARK_CIRCLEMINUS,   SC_MARK_CIRCLEPLUS,SC_MARK_VLINE,        SC_MARK_LCORNERCURVE,  SC_MARK_CIRCLEPLUSCONNECTED, SC_MARK_CIRCLEMINUSCONNECTED, SC_MARK_TCORNERCURVE},
  {SC_MARK_BOXMINUS,      SC_MARK_BOXPLUS,   SC_MARK_VLINE,        SC_MARK_LCORNER,       SC_MARK_BOXPLUSCONNECTED,    SC_MARK_BOXMINUSCONNECTED,    SC_MARK_TCORNER}
};

// Language name info array
LanguageNameInfo ScintillaEditView::_langNameInfoArray[L_EXTERNAL + 1] = {
    {L"normal",           L"Normal text",            L"Normal text file",                                  L_TEXT,            "null"},
    {L"php",              L"PHP",                    L"PHP Hypertext Preprocessor file",                   L_PHP,             "phpscript"},
    {L"c",                L"C",                      L"C source file",                                     L_C,               "cpp"},
    {L"cpp",              L"C++",                    L"C++ source file",                                   L_CPP,             "cpp"},
    {L"cs",               L"C#",                     L"C# source file",                                    L_CS,              "cpp"},
    {L"objc",             L"Objective-C",            L"Objective-C source file",                           L_OBJC,            "objc"},
    {L"java",             L"Java",                   L"Java source file",                                  L_JAVA,            "cpp"},
    {L"rc",               L"RC",                     L"Windows Resource file",                             L_RC,              "cpp"},
    {L"html",             L"HTML",                   L"Hyper Text Markup Language file",                   L_HTML,            "hypertext"},
    {L"xml",              L"XML",                    L"eXtensible Markup Language file",                   L_XML,             "xml"},
    {L"makefile",         L"Makefile",               L"Makefile",                                          L_MAKEFILE,        "makefile"},
    {L"pascal",           L"Pascal",                 L"Pascal source file",                                L_PASCAL,          "pascal"},
    {L"batch",            L"Batch",                  L"Batch file",                                        L_BATCH,           "batch"},
    {L"ini",              L"ini",                    L"MS ini file",                                       L_INI,             "props"},
    {L"nfo",              L"NFO",                    L"MSDOS Style/ASCII Art",                             L_ASCII,           "null"},
    {L"udf",              L"udf",                    L"User Defined language file",                        L_USER,            "user"},
    {L"asp",              L"ASP",                    L"Active Server Pages script file",                   L_ASP,             "hypertext"},
    {L"sql",              L"SQL",                    L"Structured Query Language file",                    L_SQL,             "sql"},
    {L"vb",               L"Visual Basic",           L"Visual Basic file",                                 L_VB,              "vb"},
    {L"javascript",       L"Embedded JS",            L"Embedded JavaScript",                               L_JS_EMBEDDED,     "cpp"},
    {L"css",              L"CSS",                    L"Cascade Style Sheets File",                         L_CSS,             "css"},
    {L"perl",             L"Perl",                   L"Perl source file",                                  L_PERL,            "perl"},
    {L"python",           L"Python",                 L"Python file",                                       L_PYTHON,          "python"},
    {L"lua",              L"Lua",                    L"Lua source File",                                   L_LUA,             "lua"},
    {L"tex",              L"TeX",                    L"TeX file",                                          L_TEX,             "tex"},
    {L"fortran",          L"Fortran free form",      L"Fortran free form source file",                     L_FORTRAN,         "fortran"},
    {L"bash",             L"Shell",                  L"Unix script file",                                  L_BASH,            "bash"},
    {L"actionscript",     L"ActionScript",           L"Flash ActionScript file",                           L_FLASH,           "cpp"},
    {L"nsis",             L"NSIS",                   L"Nullsoft Scriptable Install System script file",    L_NSIS,            "nsis"},
    {L"tcl",              L"TCL",                    L"Tool Command Language file",                        L_TCL,             "tcl"},
    {L"lisp",             L"Lisp",                   L"List Processing language file",                     L_LISP,            "lisp"},
    {L"scheme",           L"Scheme",                 L"Scheme file",                                       L_SCHEME,          "lisp"},
    {L"asm",              L"Assembly",               L"Assembly language source file",                     L_ASM,             "asm"},
    {L"diff",             L"Diff",                   L"Diff file",                                         L_DIFF,            "diff"},
    {L"props",            L"Properties file",        L"Properties file",                                   L_PROPS,           "props"},
    {L"postscript",       L"PostScript",             L"PostScript file",                                   L_PS,              "ps"},
    {L"ruby",             L"Ruby",                   L"Ruby file",                                         L_RUBY,            "ruby"},
    {L"smalltalk",        L"Smalltalk",              L"Smalltalk file",                                    L_SMALLTALK,       "smalltalk"},
    {L"vhdl",             L"VHDL",                   L"VHSIC Hardware Description Language file",          L_VHDL,            "vhdl"},
    {L"kix",              L"KiXtart",                L"KiXtart file",                                      L_KIX,             "kix"},
    {L"autoit",           L"AutoIt",                 L"AutoIt",                                            L_AU3,             "au3"},
    {L"caml",             L"CAML",                   L"Categorical Abstract Machine Language",             L_CAML,            "caml"},
    {L"ada",              L"Ada",                    L"Ada file",                                          L_ADA,             "ada"},
    {L"verilog",          L"Verilog",                L"Verilog file",                                      L_VERILOG,         "verilog"},
    {L"matlab",           L"MATLAB",                 L"MATrix LABoratory",                                 L_MATLAB,          "matlab"},
    {L"haskell",          L"Haskell",                L"Haskell",                                           L_HASKELL,         "haskell"},
    {L"inno",             L"Inno Setup",             L"Inno Setup script",                                 L_INNO,            "inno"},
    {L"searchResult",     L"Internal Search",        L"Internal Search",                                   L_SEARCHRESULT,    "searchResult"},
    {L"cmake",            L"CMake",                  L"CMake file",                                        L_CMAKE,           "cmake"},
    {L"yaml",             L"YAML",                   L"YAML Ain't Markup Language",                        L_YAML,            "yaml"},
    {L"cobol",            L"COBOL",                  L"COmmon Business Oriented Language",                 L_COBOL,           "COBOL"},
    {L"gui4cli",          L"Gui4Cli",                L"Gui4Cli file",                                      L_GUI4CLI,         "gui4cli"},
    {L"d",                L"D",                      L"D programming language",                            L_D,               "d"},
    {L"powershell",       L"PowerShell",             L"Windows PowerShell",                                L_POWERSHELL,      "powershell"},
    {L"r",                L"R",                      L"R programming language",                            L_R,               "r"},
    {L"jsp",              L"JSP",                    L"JavaServer Pages script file",                      L_JSP,             "hypertext"},
    {L"coffeescript",     L"CoffeeScript",           L"CoffeeScript file",                                 L_COFFEESCRIPT,    "coffeescript"},
    {L"json",             L"json",                   L"JSON file",                                         L_JSON,            "json"},
    {L"javascript.js",    L"JavaScript",             L"JavaScript file",                                   L_JAVASCRIPT,      "cpp"},
    {L"fortran77",        L"Fortran fixed form",     L"Fortran fixed form source file",                    L_FORTRAN_77,      "f77"},
    {L"baanc",            L"BaanC",                  L"BaanC File",                                        L_BAANC,           "baan"},
    {L"srec",             L"S-Record",               L"Motorola S-Record binary data",                     L_SREC,            "srec"},
    {L"ihex",             L"Intel HEX",              L"Intel HEX binary data",                             L_IHEX,            "ihex"},
    {L"tehex",            L"Tektronix extended HEX", L"Tektronix extended HEX binary data",                L_TEHEX,           "tehex"},
    {L"swift",            L"Swift",                  L"Swift file",                                        L_SWIFT,           "cpp"},
    {L"asn1",             L"ASN.1",                  L"Abstract Syntax Notation One file",                 L_ASN1,            "asn1"},
    {L"avs",              L"AviSynth",               L"AviSynth scripts files",                            L_AVS,             "avs"},
    {L"blitzbasic",       L"BlitzBasic",             L"BlitzBasic file",                                   L_BLITZBASIC,      "blitzbasic"},
    {L"purebasic",        L"PureBasic",              L"PureBasic file",                                    L_PUREBASIC,       "purebasic"},
    {L"freebasic",        L"FreeBasic",              L"FreeBasic file",                                    L_FREEBASIC,       "freebasic"},
    {L"csound",           L"Csound",                 L"Csound file",                                       L_CSOUND,          "csound"},
    {L"erlang",           L"Erlang",                 L"Erlang file",                                       L_ERLANG,          "erlang"},
    {L"escript",          L"ESCRIPT",                L"ESCRIPT file",                                      L_ESCRIPT,         "escript"},
    {L"forth",            L"Forth",                  L"Forth file",                                        L_FORTH,           "forth"},
    {L"latex",            L"LaTeX",                  L"LaTeX file",                                        L_LATEX,           "latex"},
    {L"mmixal",           L"MMIXAL",                 L"MMIXAL file",                                       L_MMIXAL,          "mmixal"},
    {L"nim",              L"Nim",                    L"Nim file",                                          L_NIM,             "nimrod"},
    {L"nncrontab",        L"Nncrontab",              L"extended crontab file",                             L_NNCRONTAB,       "nncrontab"},
    {L"oscript",          L"OScript",                L"OScript source file",                               L_OSCRIPT,         "oscript"},
    {L"rebol",            L"REBOL",                  L"REBOL file",                                        L_REBOL,           "rebol"},
    {L"registry",         L"registry",               L"registry file",                                     L_REGISTRY,        "registry"},
    {L"rust",             L"Rust",                   L"Rust file",                                         L_RUST,            "rust"},
    {L"spice",            L"Spice",                  L"spice file",                                        L_SPICE,           "spice"},
    {L"txt2tags",         L"txt2tags",               L"txt2tags file",                                     L_TXT2TAGS,        "txt2tags"},
    {L"visualprolog",     L"Visual Prolog",          L"Visual Prolog file",                                L_VISUALPROLOG,    "visualprolog"},
    {L"typescript",       L"TypeScript",             L"TypeScript file",                                   L_TYPESCRIPT,      "cpp"},
    {L"json5",            L"json5",                  L"JSON5 file",                                        L_JSON5,           "json"},
    {L"mssql",            L"mssql",                  L"Microsoft Transact-SQL (SQL Server) file",          L_MSSQL,           "mssql"},
    {L"gdscript",         L"GDScript",               L"GDScript file",                                     L_GDSCRIPT,        "gdscript"},
    {L"hollywood",        L"Hollywood",              L"Hollywood script",                                  L_HOLLYWOOD,       "hollywood"},
    {L"go",               L"Go",                     L"Go source file",                                    L_GOLANG,          "cpp"},
    {L"raku",             L"Raku",                   L"Raku source file",                                  L_RAKU,            "raku"},
    {L"toml",             L"TOML",                   L"Tom's Obvious Minimal Language file",               L_TOML,            "toml"},
    {L"sas",              L"SAS",                    L"SAS file",                                          L_SAS,             "sas"},
    {L"errorlist",        L"ErrorList",              L"ErrorList",                                         L_ERRORLIST,       "errorlist"},
    {L"ext",              L"External",               L"External",                                          L_EXTERNAL,        "null"}
};

void ScintillaEditView::replaceSelWith(const char* replaceText)
{
    execute(SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(replaceText));
}

void ScintillaEditView::getCurrentFoldStates(std::vector<size_t>& lineStateVector)
{
    size_t contractedFoldHeaderLine = 0;

    do {
        contractedFoldHeaderLine = execute(SCI_CONTRACTEDFOLDNEXT, contractedFoldHeaderLine);
        if (static_cast<intptr_t>(contractedFoldHeaderLine) != -1)
        {
            lineStateVector.push_back(contractedFoldHeaderLine);
            ++contractedFoldHeaderLine;
        }
    } while (static_cast<intptr_t>(contractedFoldHeaderLine) != -1);
}

void ScintillaEditView::syncFoldStateWith(const std::vector<size_t>& lineStateVectorNew)
{
    size_t nbLineState = lineStateVectorNew.size();

    if (nbLineState > 0)
    {
        for (size_t i = 0; i < nbLineState; ++i)
        {
            auto line = lineStateVectorNew.at(i);
            fold(line, fold_collapse, false);
        }
    }
}

void ScintillaEditView::defineDocType(LangType typeDoc)
{
    StyleArray& stylers = NppParameters::getInstance().getMiscStylerArray();
    Style* pStyleDefault = stylers.findByID(STYLE_DEFAULT);
    if (pStyleDefault)
    {
        pStyleDefault->_colorStyle = COLORSTYLE_ALL;
        setStyle(*pStyleDefault);
    }

    execute(SCI_STYLECLEARALL);

    Style defaultIndicatorStyle;
    const Style* pStyle;

    defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE;
    defaultIndicatorStyle._bgColor = red;
    pStyle = stylers.findByID(defaultIndicatorStyle._styleID);
    setSpecialIndicator(pStyle ? *pStyle : defaultIndicatorStyle);

    defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_SMART;
    defaultIndicatorStyle._bgColor = liteGreen;
    pStyle = stylers.findByID(defaultIndicatorStyle._styleID);
    setSpecialIndicator(pStyle ? *pStyle : defaultIndicatorStyle);

    defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_INC;
    defaultIndicatorStyle._bgColor = blue;
    pStyle = stylers.findByID(defaultIndicatorStyle._styleID);
    setSpecialIndicator(pStyle ? *pStyle : defaultIndicatorStyle);

    defaultIndicatorStyle._styleID = SCE_UNIVERSAL_TAGMATCH;
    defaultIndicatorStyle._bgColor = RGB(0x80, 0x00, 0xFF);
    pStyle = stylers.findByID(defaultIndicatorStyle._styleID);
    setSpecialIndicator(pStyle ? *pStyle : defaultIndicatorStyle);

    defaultIndicatorStyle._styleID = SCE_UNIVERSAL_TAGATTR;
    defaultIndicatorStyle._bgColor = yellow;
    pStyle = stylers.findByID(defaultIndicatorStyle._styleID);
    setSpecialIndicator(pStyle ? *pStyle : defaultIndicatorStyle);

    defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_EXT1;
    defaultIndicatorStyle._bgColor = cyan;
    pStyle = stylers.findByID(defaultIndicatorStyle._styleID);
    setSpecialIndicator(pStyle ? *pStyle : defaultIndicatorStyle);

    defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_EXT2;
    defaultIndicatorStyle._bgColor = orange;
    pStyle = stylers.findByID(defaultIndicatorStyle._styleID);
    setSpecialIndicator(pStyle ? *pStyle : defaultIndicatorStyle);

    defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_EXT3;
    defaultIndicatorStyle._bgColor = yellow;
    pStyle = stylers.findByID(defaultIndicatorStyle._styleID);
    setSpecialIndicator(pStyle ? *pStyle : defaultIndicatorStyle);

    defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_EXT4;
    defaultIndicatorStyle._bgColor = purple;
    pStyle = stylers.findByID(defaultIndicatorStyle._styleID);
    setSpecialIndicator(pStyle ? *pStyle : defaultIndicatorStyle);

    defaultIndicatorStyle._styleID = SCE_UNIVERSAL_FOUND_STYLE_EXT5;
    defaultIndicatorStyle._bgColor = darkGreen;
    pStyle = stylers.findByID(defaultIndicatorStyle._styleID);
    setSpecialIndicator(pStyle ? *pStyle : defaultIndicatorStyle);

    if (isCJK())
    {
        if (getCurrentBuffer()->getUnicodeMode() == uni8Bit)
        {
            if (typeDoc == L_CSS || typeDoc == L_CAML || typeDoc == L_ASM || typeDoc == L_MATLAB)
                execute(SCI_SETCODEPAGE, CP_ACP);
            else
                execute(SCI_SETCODEPAGE, _codepage);
        }
    }

    const ScintillaViewParams& svp = NppParameters::getInstance().getSVP();
    if (svp._folderStyle != FOLDER_STYLE_NONE)
        showMargin(_SC_MARGE_FOLDER, isNeededFolderMargin(typeDoc));

    switch (typeDoc)
    {
        case L_C:
        case L_CPP:
        case L_JAVA:
        case L_RC:
        case L_CS:
        case L_FLASH:
        case L_SWIFT:
        case L_GOLANG:
            setCppLexer(typeDoc); break;

        case L_JS_EMBEDDED:
        case L_JAVASCRIPT:
            setJsLexer(); break;

        case L_TCL:
            setTclLexer(); break;

        case L_XML:
        case L_HTML:
        case L_PHP:
        case L_ASP:
        case L_JSP:
            setXmlLexer(typeDoc); break;

        case L_CSS:
            setCssLexer(); break;

        case L_LUA:
            setLuaLexer(); break;

        case L_MAKEFILE:
            setMakefileLexer(); break;

        case L_INI:
        case L_PROPS:
            setPropsLexer(typeDoc == L_PROPS); break;

        case L_SQL:
            setSqlLexer(); break;

        case L_VB:
            setVBLexer(); break;

        case L_PERL:
            setPerlLexer(); break;

        case L_PYTHON:
            setPythonLexer(); break;

        case L_BATCH:
            setBatchLexer(); break;

        case L_TEX:
            setTeXLexer(); break;

        case L_FORTRAN:
            setFortranLexer(); break;

        case L_BASH:
            setBashLexer(); break;

        case L_NSIS:
            setNsisLexer(); break;

        case L_LISP:
            setLispLexer(); break;

        case L_SCHEME:
            setSchemeLexer(); break;

        case L_ASM:
            setAsmLexer(); break;

        case L_DIFF:
            setDiffLexer(); break;

        case L_PS:
            setPostscriptLexer(); break;

        case L_RUBY:
            setRubyLexer(); break;

        case L_SMALLTALK:
            setSmalltalkLexer(); break;

        case L_VHDL:
            setVhdlLexer(); break;

        case L_KIX:
            setKixLexer(); break;

        case L_AU3:
            setAutoItLexer(); break;

        case L_CAML:
            setCamlLexer(); break;

        case L_ADA:
            setAdaLexer(); break;

        case L_VERILOG:
            setVerilogLexer(); break;

        case L_MATLAB:
            setMatlabLexer(); break;

        case L_HASKELL:
            setHaskellLexer(); break;

        case L_INNO:
            setInnoLexer(); break;

        case L_CMAKE:
            setCmakeLexer(); break;

        case L_YAML:
            setYamlLexer(); break;

        case L_COBOL:
            setCobolLexer(); break;

        case L_GUI4CLI:
            setGui4CliLexer(); break;

        case L_D:
            setDLexer(); break;

        case L_POWERSHELL:
            setPowerShellLexer(); break;

        case L_R:
            setRLexer(); break;

        case L_COFFEESCRIPT:
            setCoffeeScriptLexer(); break;

        case L_BAANC:
            setBaanCLexer(); break;

        case L_SREC:
            setSrecLexer(); break;

        case L_IHEX:
            setIHexLexer(); break;

        case L_TEHEX:
            setTEHexLexer(); break;

        case L_ASN1:
            setAsn1Lexer(); break;

        case L_AVS:
            setAVSLexer(); break;

        case L_BLITZBASIC:
            setBlitzBasicLexer(); break;

        case L_PUREBASIC:
            setPureBasicLexer(); break;

        case L_FREEBASIC:
            setFreeBasicLexer(); break;

        case L_CSOUND:
            setCsoundLexer(); break;

        case L_ERLANG:
            setErlangLexer(); break;

        case L_ESCRIPT:
            setESCRIPTLexer(); break;

        case L_FORTH:
            setForthLexer(); break;

        case L_LATEX:
            setLatexLexer(); break;

        case L_MMIXAL:
            setMMIXALLexer(); break;

        case L_NIM:
            setNimrodLexer(); break;

        case L_NNCRONTAB:
            setNncrontabLexer(); break;

        case L_OSCRIPT:
            setOScriptLexer(); break;

        case L_REBOL:
            setREBOLLexer(); break;

        case L_REGISTRY:
            setRegistryLexer(); break;

        case L_RUST:
            setRustLexer(); break;

        case L_SPICE:
            setSpiceLexer(); break;

        case L_TXT2TAGS:
            setTxt2tagsLexer(); break;

        case L_VISUALPROLOG:
            setVisualPrologLexer(); break;

        case L_TYPESCRIPT:
            setTypeScriptLexer(); break;

        case L_HOLLYWOOD:
            setHollywoodLexer(); break;

        case L_RAKU:
            setRakuLexer(); break;

        case L_TOML:
            setTomlLexer(); break;

        case L_SAS:
            setSasLexer(); break;

        case L_GDSCRIPT:
            setGDScriptLexer(); break;

        case L_OBJC:
            setObjCLexer(typeDoc); break;

        case L_USER:
            setUserLexer(); break;

        case L_SEARCHRESULT:
            setSearchResultLexer(); break;

        case L_ERRORLIST:
            setErrorListLexer(); break;

        default:
            // For external/user-defined languages, use setExternalLexer or default to text
            if (typeDoc >= L_EXTERNAL && typeDoc < L_EXTERNAL + 100)  // Allow up to 100 user languages
                setExternalLexer(typeDoc);
            else
                setLexerFromLangID(L_TEXT);
            break;
    }
}

void ScintillaEditView::showMargin(int whichMarge, bool willBeShown)
{
    if (whichMarge == _SC_MARGE_LINENUMBER)
    {
        bool forcedToHide = !willBeShown;
        updateLineNumbersMargin(forcedToHide);
    }
    else
    {
        int width = 0;
        if (whichMarge == _SC_MARGE_SYMBOL)
            width = 16;
        else if (whichMarge == _SC_MARGE_FOLDER)
            width = 14;
        else
            width = 3;

        execute(SCI_SETMARGINWIDTHN, whichMarge, willBeShown ? width : 0);
    }
}

int ScintillaEditView::getTextZoneWidth() const
{
    // Get the client rect - for Qt, we use Scintilla to get the text area width
    intptr_t marginWidths = 0;
    for (int m = 0; m < 4; ++m)
    {
        marginWidths += execute(SCI_GETMARGINWIDTHN, m);
    }

    // Get the width from Scintilla
    intptr_t totalWidth = execute(SCI_GETSCROLLWIDTH);
    if (totalWidth < marginWidths)
        totalWidth = execute(SCI_GETCOLUMN, execute(SCI_GETLENGTH)) * 8; // Approximate char width

    return static_cast<int>(totalWidth - marginWidths);
}

bool ScintillaEditView::isTextDirectionRTL() const
{
    // Qt/Linux: Check if RTL layout is enabled via Scintilla
    // Scintilla doesn't have a direct SCI_GETLAYOUTRTL, so we track it via a member or check bidirectional settings
    // For now, return false as default (LTR)
    // TODO: Implement proper RTL tracking if needed
    return false;
}

void ScintillaEditView::changeTextDirection(bool isRTL)
{
    if (isTextDirectionRTL() == isRTL)
        return;

    // Qt/Linux: Scintilla's bidirectional support is different from Win32
    // We use SCI_SETBIDIRECTIONAL to enable RTL support
    if (isRTL)
    {
        execute(SCI_SETBIDIRECTIONAL, SC_BIDIRECTIONAL_R2L);
    }
    else
    {
        execute(SCI_SETBIDIRECTIONAL, SC_BIDIRECTIONAL_L2R);
    }
}

void ScintillaEditView::getGenericText(char* dest, size_t destlen, size_t start, size_t end) const
{
    auto buffer = std::make_unique<char[]>(end - start + 1);
    getText(buffer.get(), start, end);
    strncpy(dest, buffer.get(), destlen - 1);
    dest[destlen - 1] = '\0';
}

void ScintillaEditView::bufferUpdated(Buffer* buffer, int mask)
{
    if (buffer == _currentBuffer)
    {
        if (mask & BufferChangeLanguage)
        {
            defineDocType(static_cast<LangType>(buffer->getLangType()));
            foldAll(fold_expand);
        }

        if (mask & BufferChangeLexing)
        {
            if (buffer->getNeedsLexing())
            {
                restyleBuffer();
            }
        }

        if (mask & BufferChangeFormat)
        {
            execute(SCI_SETEOLMODE, static_cast<int>(_currentBuffer->getEolFormat()));
        }

        if (mask & BufferChangeReadonly)
        {
            execute(SCI_SETREADONLY, _currentBuffer->isReadOnly());
        }

        if (mask & BufferChangeUnicode)
        {
            int enc = CP_ACP;
            if (static_cast<UniMode>(buffer->getUnicodeMode()) == uni8Bit)
            {
                if (isCJK())
                {
                    LangType typeDoc = static_cast<LangType>(buffer->getLangType());
                    if (typeDoc == L_CSS || typeDoc == L_CAML || typeDoc == L_ASM || typeDoc == L_MATLAB)
                        enc = CP_ACP;
                    else
                        enc = _codepage;
                }
                else
                    enc = CP_ACP;
            }
            else
                enc = SC_CP_UTF8;
            execute(SCI_SETCODEPAGE, enc);
        }
    }
}

void ScintillaEditView::activateBuffer(BufferID buffer, bool force)
{
    if (buffer == BUFFER_INVALID) return;
    if (!force && buffer == _currentBuffer) return;

    Buffer* newBuf = MainFileManager.getBufferByID(buffer);

    saveCurrentPos();

    std::vector<size_t> lineStateVector;
    getCurrentFoldStates(lineStateVector);

    _currentBuffer->setHeaderLineState(lineStateVector, this);

    _currentBufferID = buffer;
    _currentBuffer = newBuf;

    unsigned long MODEVENTMASK_ON = NppParameters::getInstance().getScintillaModEventMask();

    execute(SCI_SETMODEVENTMASK, MODEVENTMASK_OFF);
    execute(SCI_SETDOCPOINTER, 0, reinterpret_cast<LPARAM>(_currentBuffer->getDocument()));
    execute(SCI_SETMODEVENTMASK, MODEVENTMASK_ON);

    defineDocType(static_cast<LangType>(_currentBuffer->getLangType()));

    setWordChars();
    maintainStateForNpc();

    bufferUpdated(_currentBuffer, (BufferChangeMask & ~BufferChangeLanguage));

    const std::vector<size_t>& lineStateVectorNew = newBuf->getHeaderLineState(this);
    syncFoldStateWith(lineStateVectorNew);

    restoreCurrentPosPreStep();
    restoreHiddenLines();
    setCRLF();

    NppParameters& nppParam = NppParameters::getInstance();
    const ScintillaViewParams& svp = nppParam.getSVP();

    int enabledCHFlag = SC_CHANGE_HISTORY_DISABLED;
    if (svp._isChangeHistoryMarginEnabled || svp._isChangeHistoryIndicatorEnabled)
    {
        enabledCHFlag = SC_CHANGE_HISTORY_ENABLED;

        if (svp._isChangeHistoryMarginEnabled)
            enabledCHFlag |= SC_CHANGE_HISTORY_MARKERS;

        if (svp._isChangeHistoryIndicatorEnabled)
            enabledCHFlag |= SC_CHANGE_HISTORY_INDICATORS;
    }
    execute(SCI_SETCHANGEHISTORY, enabledCHFlag);

    if (isTextDirectionRTL() != buffer->isRTL())
        changeTextDirection(buffer->isRTL());

    return;
}

void ScintillaEditView::showIndentGuideLine(bool willBeShown)
{
    auto typeDoc = static_cast<LangType>(_currentBuffer->getLangType());
    const int docIndentMode = isPythonStyleIndentation(typeDoc) ? SC_IV_LOOKFORWARD : SC_IV_LOOKBOTH;
    execute(SCI_SETINDENTATIONGUIDES, willBeShown ? docIndentMode : SC_IV_NONE);
}

void ScintillaEditView::showNpc(bool willBeShown, bool isSearchResult)
{
    const auto& svp = NppParameters::getInstance().getSVP();

    if (willBeShown)
    {
        // Set representations for non-printing characters
        // For Linux/Qt, we use a simplified approach without the full g_nonPrintingChars table
        execute(SCI_SETREPRESENTATION, reinterpret_cast<WPARAM>("\xC2\xA0"), reinterpret_cast<LPARAM>("NBSP"));
        execute(SCI_SETREPRESENTATION, reinterpret_cast<WPARAM>("\xE2\x80\x8B"), reinterpret_cast<LPARAM>("ZWSP"));

        showEOL(isShownEol());
    }
    else
    {
        execute(SCI_CLEARALLREPRESENTATIONS);

        if (!isSearchResult && svp._eolMode != svp.roundedRectangleText)
        {
            setCRLF();
        }

        showCcUniEol(svp._ccUniEolShow);
    }
}

void ScintillaEditView::showCcUniEol(bool willBeShown, bool isSearchResult)
{
    const auto& svp = NppParameters::getInstance().getSVP();

    if (willBeShown)
    {
        // Set representations for Unicode EOL characters
        execute(SCI_SETREPRESENTATION, reinterpret_cast<WPARAM>("\xC2\x85"), reinterpret_cast<LPARAM>("NEL"));
        execute(SCI_SETREPRESENTATION, reinterpret_cast<WPARAM>("\xE2\x80\xA8"), reinterpret_cast<LPARAM>("LS"));
        execute(SCI_SETREPRESENTATION, reinterpret_cast<WPARAM>("\xE2\x80\xA9"), reinterpret_cast<LPARAM>("PS"));
    }
    else
    {
        execute(SCI_CLEARALLREPRESENTATIONS);

        for (const auto& ch : { "\xC2\x85", "\xE2\x80\xA8", "\xE2\x80\xA9" })
        {
            execute(SCI_SETREPRESENTATION, reinterpret_cast<WPARAM>(ch), reinterpret_cast<LPARAM>("\xE2\x80\x8B"));
            execute(SCI_SETREPRESENTATIONAPPEARANCE, reinterpret_cast<WPARAM>(ch), SC_REPRESENTATION_PLAIN);
        }

        if (!isSearchResult && svp._eolMode != svp.roundedRectangleText)
        {
            setCRLF();
        }

        if (svp._npcShow)
        {
            showNpc();
            return;
        }
    }

    showEOL(isShownEol());
}

void ScintillaEditView::scrollPosToCenter(size_t pos)
{
    _positionRestoreNeeded = false;

    execute(SCI_GOTOPOS, pos);
    size_t line = execute(SCI_LINEFROMPOSITION, pos);

    size_t firstVisibleDisplayLine = execute(SCI_GETFIRSTVISIBLELINE);
    size_t firstVisibleDocLine = execute(SCI_DOCLINEFROMVISIBLE, firstVisibleDisplayLine);
    size_t nbLine = execute(SCI_LINESONSCREEN, firstVisibleDisplayLine);
    size_t lastVisibleDocLine = execute(SCI_DOCLINEFROMVISIBLE, firstVisibleDisplayLine + nbLine);

    size_t middleLine;
    if (line - firstVisibleDocLine < lastVisibleDocLine - line)
        middleLine = firstVisibleDocLine + nbLine/2;
    else
        middleLine = lastVisibleDocLine - nbLine/2;
    size_t nbLines2scroll = line - middleLine;
    scroll(0, static_cast<intptr_t>(nbLines2scroll));
    execute(SCI_ENSUREVISIBLEENFORCEPOLICY, line);
}

// ============================================================================
// Style and Lexer Methods
// ============================================================================

void ScintillaEditView::setSpecialStyle(const Style& styleToSet)
{
    int styleID = styleToSet._styleID;
    if (styleToSet._colorStyle & COLORSTYLE_FOREGROUND)
        execute(SCI_STYLESETFORE, styleID, styleToSet._fgColor);

    if (styleToSet._colorStyle & COLORSTYLE_BACKGROUND)
        execute(SCI_STYLESETBACK, styleID, styleToSet._bgColor);

    if (!styleToSet._fontName.empty())
    {
        if (!NppParameters::getInstance().isInFontList(styleToSet._fontName))
        {
            execute(SCI_STYLESETFONT, styleID, reinterpret_cast<LPARAM>("Courier New"));
        }
        else
        {
            std::string fontNameA = wstring2string(styleToSet._fontName, CP_UTF8);
            execute(SCI_STYLESETFONT, styleID, reinterpret_cast<LPARAM>(fontNameA.c_str()));
        }
    }
    int fontStyle = styleToSet._fontStyle;
    if (fontStyle != STYLE_NOT_USED)
    {
        execute(SCI_STYLESETBOLD, styleID, fontStyle & FONTSTYLE_BOLD);
        execute(SCI_STYLESETITALIC, styleID, fontStyle & FONTSTYLE_ITALIC);
        execute(SCI_STYLESETUNDERLINE, styleID, fontStyle & FONTSTYLE_UNDERLINE);
    }

    if (styleToSet._fontSize > 0)
        execute(SCI_STYLESETSIZE, styleID, styleToSet._fontSize);
}

void ScintillaEditView::setStyle(Style styleToSet)
{
    GlobalOverride& go = NppParameters::getInstance().getGlobalOverrideStyle();

    if (go.isEnable())
    {
        const Style* pStyle = NppParameters::getInstance().getMiscStylerArray().findByName(L"Global override");
        if (pStyle)
        {
            if (go.enableFg)
            {
                if (pStyle->_colorStyle & COLORSTYLE_FOREGROUND)
                {
                    styleToSet._colorStyle |= COLORSTYLE_FOREGROUND;
                    styleToSet._fgColor = pStyle->_fgColor;
                }
                else
                {
                    if (styleToSet._styleID == STYLE_DEFAULT)
                        styleToSet._colorStyle |= COLORSTYLE_FOREGROUND;
                    else
                        styleToSet._colorStyle &= ~COLORSTYLE_FOREGROUND;
                }
            }

            if (go.enableBg)
            {
                if (pStyle->_colorStyle & COLORSTYLE_BACKGROUND)
                {
                    styleToSet._colorStyle |= COLORSTYLE_BACKGROUND;
                    styleToSet._bgColor = pStyle->_bgColor;
                }
                else
                {
                    if (styleToSet._styleID == STYLE_DEFAULT)
                        styleToSet._colorStyle |= COLORSTYLE_BACKGROUND;
                    else
                        styleToSet._colorStyle &= ~COLORSTYLE_BACKGROUND;
                }
            }
            if (go.enableFont && !pStyle->_fontName.empty())
                styleToSet._fontName = pStyle->_fontName;
            if (go.enableFontSize && (pStyle->_fontSize > 0))
                styleToSet._fontSize = pStyle->_fontSize;

            if (pStyle->_fontStyle != STYLE_NOT_USED)
            {
                if (go.enableBold)
                {
                    if (pStyle->_fontStyle & FONTSTYLE_BOLD)
                        styleToSet._fontStyle |= FONTSTYLE_BOLD;
                    else
                        styleToSet._fontStyle &= ~FONTSTYLE_BOLD;
                }
                if (go.enableItalic)
                {
                    if (pStyle->_fontStyle & FONTSTYLE_ITALIC)
                        styleToSet._fontStyle |= FONTSTYLE_ITALIC;
                    else
                        styleToSet._fontStyle &= ~FONTSTYLE_ITALIC;
                }
                if (go.enableUnderLine)
                {
                    if (pStyle->_fontStyle & FONTSTYLE_UNDERLINE)
                        styleToSet._fontStyle |= FONTSTYLE_UNDERLINE;
                    else
                        styleToSet._fontStyle &= ~FONTSTYLE_UNDERLINE;
                }
            }
        }
    }
    setSpecialStyle(styleToSet);
}

void ScintillaEditView::makeStyle(LangType language, const wchar_t** keywordArray)
{
    const wchar_t* lexerName = ScintillaEditView::_langNameInfoArray[language]._langName;
    LexerStyler* pStyler = (NppParameters::getInstance().getLStylerArray()).getLexerStylerByName(lexerName);
    if (pStyler)
    {
        for (const Style& style : *pStyler)
        {
            setStyle(style);
            if (keywordArray)
            {
                if ((style._keywordClass != STYLE_NOT_USED) && (!style._keywords.empty()))
                    keywordArray[style._keywordClass] = style._keywords.c_str();
            }
        }
    }
}

const char* ScintillaEditView::concatToBuildKeywordList(std::basic_string<char>& kwl, LangType langType, int keywordIndex)
{
    kwl += " ";
    const wchar_t* defKwl_generic = NppParameters::getInstance().getWordList(langType, keywordIndex);
    if (defKwl_generic)
    {
        WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
        const char* defKwl = wmc.wchar2char(defKwl_generic, CP_ACP);
        kwl += defKwl ? defKwl : "";
    }

    return kwl.c_str();
}

void ScintillaEditView::setKeywords(LangType langType, const char* keywords, int index)
{
    std::basic_string<char> wordList;
    wordList = (keywords) ? keywords : "";
    execute(SCI_SETKEYWORDS, index, reinterpret_cast<LPARAM>(concatToBuildKeywordList(wordList, langType, index)));
}

void ScintillaEditView::populateSubStyleKeywords(LangType langType, int baseStyleID, int numSubStyles, int firstLangIndex, const wchar_t** pKwArray)
{
    WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
    int firstID = execute(SCI_ALLOCATESUBSTYLES, baseStyleID, numSubStyles) & 0xFF;

    if (pKwArray && (firstID >= 0))
    {
        for (int i = 0; i < numSubStyles; i++)
        {
            int ss = firstLangIndex + i;
            int styleID = firstID + i;
            basic_string<char> userWords = pKwArray[ss] ? wmc.wchar2char(pKwArray[ss], CP_ACP) : "";
            execute(SCI_SETIDENTIFIERS, styleID, reinterpret_cast<LPARAM>(concatToBuildKeywordList(userWords, langType, ss)));
        }
    }
}

void ScintillaEditView::setLexer(LangType langType, int whichList, int baseStyleID, int numSubStyles)
{
    setLexerFromLangID(langType);

    const wchar_t* pKwArray[NB_LIST] = {NULL};

    makeStyle(langType, pKwArray);

    WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();

    if (whichList & LIST_0)
    {
        const char* keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_INSTR], CP_ACP);
        setKeywords(langType, keyWords_char, LANG_INDEX_INSTR);
    }

    if (whichList & LIST_1)
    {
        const char* keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_INSTR2], CP_ACP);
        setKeywords(langType, keyWords_char, LANG_INDEX_INSTR2);
    }

    if (whichList & LIST_2)
    {
        const char* keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE], CP_ACP);
        setKeywords(langType, keyWords_char, LANG_INDEX_TYPE);
    }

    if (whichList & LIST_3)
    {
        const char* keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE2], CP_ACP);
        setKeywords(langType, keyWords_char, LANG_INDEX_TYPE2);
    }

    if (whichList & LIST_4)
    {
        const char* keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE3], CP_ACP);
        setKeywords(langType, keyWords_char, LANG_INDEX_TYPE3);
    }

    if (whichList & LIST_5)
    {
        const char* keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE4], CP_ACP);
        setKeywords(langType, keyWords_char, LANG_INDEX_TYPE4);
    }

    if (whichList & LIST_6)
    {
        const char* keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE5], CP_ACP);
        setKeywords(langType, keyWords_char, LANG_INDEX_TYPE5);
    }

    if (whichList & LIST_7)
    {
        const char* keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE6], CP_ACP);
        setKeywords(langType, keyWords_char, LANG_INDEX_TYPE6);
    }

    if (whichList & LIST_8)
    {
        const char* keyWords_char = wmc.wchar2char(pKwArray[LANG_INDEX_TYPE7], CP_ACP);
        setKeywords(langType, keyWords_char, LANG_INDEX_TYPE7);
    }

    if (baseStyleID != STYLE_NOT_USED)
    {
        populateSubStyleKeywords(langType, baseStyleID, numSubStyles, LANG_INDEX_SUBSTYLE1, pKwArray);
    }

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
}

bool ScintillaEditView::setLexerFromLangID(int langID)
{
    if (langID >= L_EXTERNAL)
        return false;

    const char* lexerNameID = _langNameInfoArray[langID]._lexerID;
    execute(SCI_SETILEXER, 0, reinterpret_cast<LPARAM>(CreateLexer(lexerNameID)));
    return true;
}

void ScintillaEditView::restyleBuffer()
{
    execute(SCI_CLEARDOCUMENTSTYLE);
    execute(SCI_COLOURISE, 0, -1);
    _currentBuffer->setNeedsLexing(false);
}

void ScintillaEditView::saveCurrentPos()
{
    size_t displayedLine = execute(SCI_GETFIRSTVISIBLELINE);
    size_t docLine = execute(SCI_DOCLINEFROMVISIBLE, displayedLine);
    size_t offset = displayedLine - execute(SCI_VISIBLEFROMDOCLINE, docLine);
    size_t wrapCount = execute(SCI_WRAPCOUNT, docLine);

    Buffer* buf = MainFileManager.getBufferByID(_currentBufferID);

    Position pos;
    pos._firstVisibleLine = docLine;
    pos._startPos = execute(SCI_GETANCHOR);
    pos._endPos = execute(SCI_GETCURRENTPOS);
    pos._xOffset = execute(SCI_GETXOFFSET);
    pos._selMode = execute(SCI_GETSELECTIONMODE);
    pos._scrollWidth = execute(SCI_GETSCROLLWIDTH);
    pos._offset = offset;
    pos._wrapCount = wrapCount;

    buf->setPosition(pos, this);
}

void ScintillaEditView::restoreCurrentPosPreStep()
{
    Buffer* buf = MainFileManager.getBufferByID(_currentBufferID);
    const Position& pos = buf->getPosition(this);

    execute(SCI_SETSELECTIONMODE, pos._selMode);
    execute(SCI_SETANCHOR, pos._startPos);
    execute(SCI_SETCURRENTPOS, pos._endPos);
    execute(SCI_CANCEL);
    if (!isWrap())
    {
        execute(SCI_SETSCROLLWIDTH, pos._scrollWidth);
        execute(SCI_SETXOFFSET, pos._xOffset);
    }
    execute(SCI_CHOOSECARETX);
    intptr_t lineToShow = execute(SCI_VISIBLEFROMDOCLINE, pos._firstVisibleLine);
    execute(SCI_SETFIRSTVISIBLELINE, lineToShow);
    if (isWrap())
    {
        _positionRestoreNeeded = true;
    }
    _restorePositionRetryCount = 0;
}

void ScintillaEditView::setWordChars()
{
    NppParameters& nppParam = NppParameters::getInstance();
    const NppGUI& nppGUI = nppParam.getNppGUI();
    if (nppGUI._isWordCharDefault)
        restoreDefaultWordChars();
    else
        addCustomWordChars();
}

void ScintillaEditView::restoreDefaultWordChars()
{
    execute(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>(_defaultCharList.c_str()));
}

void ScintillaEditView::addCustomWordChars()
{
    NppParameters& nppParam = NppParameters::getInstance();
    const NppGUI& nppGUI = nppParam.getNppGUI();

    if (nppGUI._customWordChars.empty())
        return;

    string chars2addStr;
    for (size_t i = 0; i < nppGUI._customWordChars.length(); ++i)
    {
        bool found = false;
        char char2check = nppGUI._customWordChars[i];
        for (size_t j = 0; j < _defaultCharList.length(); ++j)
        {
            char wordChar = _defaultCharList[j];
            if (char2check == wordChar)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            chars2addStr.push_back(char2check);
        }
    }

    if (!chars2addStr.empty())
    {
        string newCharList = _defaultCharList;
        newCharList += chars2addStr;
        execute(SCI_SETWORDCHARS, 0, reinterpret_cast<LPARAM>(newCharList.c_str()));
    }
}

void ScintillaEditView::setCRLF(long color)
{
    NppParameters& nppParams = NppParameters::getInstance();
    const ScintillaViewParams& svp = nppParams.getSVP();

    COLORREF eolCustomColor = liteGrey;

    if (color == -1)
    {
        StyleArray& stylers = nppParams.getMiscStylerArray();
        Style* pStyle = stylers.findByName(L"EOL custom color");
        if (pStyle)
        {
            eolCustomColor = pStyle->_fgColor;
        }
    }
    else
    {
        eolCustomColor = color;
    }

    ScintillaViewParams::crlfMode eolMode = svp._eolMode;
    long appearance = SC_REPRESENTATION_BLOB;

    if (eolMode == ScintillaViewParams::crlfMode::plainText)
        appearance = SC_REPRESENTATION_PLAIN;
    else if (eolMode == ScintillaViewParams::crlfMode::plainTextCustomColor)
        appearance = SC_REPRESENTATION_PLAIN | SC_REPRESENTATION_COLOUR;
    else if (eolMode == ScintillaViewParams::crlfMode::roundedRectangleText)
        appearance = SC_REPRESENTATION_BLOB;

    execute(SCI_SETREPRESENTATIONAPPEARANCE, reinterpret_cast<WPARAM>("\r\n"), appearance);
    execute(SCI_SETREPRESENTATIONAPPEARANCE, reinterpret_cast<WPARAM>("\n"), appearance);
    execute(SCI_SETREPRESENTATIONAPPEARANCE, reinterpret_cast<WPARAM>("\r"), appearance);

    if (appearance & SC_REPRESENTATION_COLOUR)
    {
        execute(SCI_SETREPRESENTATIONCOLOUR, reinterpret_cast<WPARAM>("\r\n"), eolCustomColor);
        execute(SCI_SETREPRESENTATIONCOLOUR, reinterpret_cast<WPARAM>("\n"), eolCustomColor);
        execute(SCI_SETREPRESENTATIONCOLOUR, reinterpret_cast<WPARAM>("\r"), eolCustomColor);
    }

    const char* crlf = "CRLF";
    const char* lf = "LF";
    const char* cr = "CR";

    if (svp._eolMode == svp.roundedRectangleText)
    {
        crlf = "";
        lf = "";
        cr = "";
    }

    execute(SCI_SETREPRESENTATION, reinterpret_cast<WPARAM>("\r\n"), reinterpret_cast<LPARAM>(crlf));
    execute(SCI_SETREPRESENTATION, reinterpret_cast<WPARAM>("\n"), reinterpret_cast<LPARAM>(lf));
    execute(SCI_SETREPRESENTATION, reinterpret_cast<WPARAM>("\r"), reinterpret_cast<LPARAM>(cr));
}

void ScintillaEditView::updateLineNumberWidth()
{
    const ScintillaViewParams& svp = NppParameters::getInstance().getSVP();
    if (svp._lineNumberMarginShow)
    {
        auto linesVisible = execute(SCI_LINESONSCREEN);
        if (linesVisible)
        {
            int nbDigits = 0;

            if (svp._lineNumberMarginDynamicWidth)
            {
                auto firstVisibleLineVis = execute(SCI_GETFIRSTVISIBLELINE);
                auto lastVisibleLineVis = linesVisible + firstVisibleLineVis + 1;
                auto lastVisibleLineDoc = execute(SCI_DOCLINEFROMVISIBLE, lastVisibleLineVis);

                nbDigits = nbDigitsFromNbLines(lastVisibleLineDoc);
                nbDigits = nbDigits < 3 ? 3 : nbDigits;
            }
            else
            {
                auto nbLines = execute(SCI_GETLINECOUNT);
                nbDigits = nbDigitsFromNbLines(nbLines);
                nbDigits = nbDigits < 4 ? 4 : nbDigits;
            }

            auto pixelWidth = 8 + nbDigits * execute(SCI_TEXTWIDTH, STYLE_LINENUMBER, reinterpret_cast<LPARAM>("8"));
            execute(SCI_SETMARGINWIDTHN, _SC_MARGE_LINENUMBER, pixelWidth);
        }
    }
}

// ============================================================================
// Complex Lexers
// ============================================================================

void ScintillaEditView::setCppLexer(LangType langType)
{
    const char* cppInstrs;
    const char* cppTypes;
    const char* cppGlobalclass;
    const wchar_t* doxygenKeyWords = NppParameters::getInstance().getWordList(L_CPP, LANG_INDEX_TYPE2);

    setLexerFromLangID(L_CPP);

    if (langType == L_GOLANG)
    {
        execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.cpp.backquoted.strings"), reinterpret_cast<LPARAM>("1"));
    }

    if (langType != L_RC)
    {
        if (doxygenKeyWords)
        {
            WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
            const char* doxygenKeyWords_char = wmc.wchar2char(doxygenKeyWords, CP_ACP);
            execute(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(doxygenKeyWords_char));
        }
    }

    const wchar_t* pKwArray[NB_LIST] = {NULL};
    makeStyle(langType, pKwArray);

    basic_string<char> keywordListInstruction("");
    basic_string<char> keywordListType("");
    basic_string<char> keywordListGlobalclass("");
    if (pKwArray[LANG_INDEX_INSTR])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
        keywordListInstruction = wstring2string(kwlW, CP_ACP);
    }
    cppInstrs = concatToBuildKeywordList(keywordListInstruction, langType, LANG_INDEX_INSTR);

    if (pKwArray[LANG_INDEX_TYPE])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE];
        keywordListType = wstring2string(kwlW, CP_ACP);
    }
    cppTypes = concatToBuildKeywordList(keywordListType, langType, LANG_INDEX_TYPE);

    if (pKwArray[LANG_INDEX_INSTR2])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR2];
        keywordListGlobalclass = wstring2string(kwlW, CP_ACP);
    }
    cppGlobalclass = concatToBuildKeywordList(keywordListGlobalclass, langType, LANG_INDEX_INSTR2);

    execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(cppInstrs));
    execute(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(cppTypes));
    execute(SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(cppGlobalclass));

    populateSubStyleKeywords(langType, SCE_C_IDENTIFIER, 8, LANG_INDEX_SUBSTYLE1, pKwArray);

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.cpp.comment.explicit"), reinterpret_cast<LPARAM>("0"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.cpp.track.preprocessor"), reinterpret_cast<LPARAM>("0"));
}

void ScintillaEditView::setJsLexer()
{
    const wchar_t* doxygenKeyWords = NppParameters::getInstance().getWordList(L_CPP, LANG_INDEX_TYPE2);

    setLexerFromLangID(L_JAVASCRIPT);
    const wchar_t* pKwArray[NB_LIST] = {NULL};
    makeStyle(L_JAVASCRIPT, pKwArray);

    if (doxygenKeyWords)
    {
        WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
        const char* doxygenKeyWords_char = wmc.wchar2char(doxygenKeyWords, CP_ACP);
        execute(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(doxygenKeyWords_char));
    }

    const wchar_t* newLexerName = ScintillaEditView::_langNameInfoArray[L_JAVASCRIPT]._langName;
    LexerStyler* pNewStyler = (NppParameters::getInstance().getLStylerArray()).getLexerStylerByName(newLexerName);
    if (pNewStyler)
    {
        for (const Style& style : *pNewStyler)
        {
            setStyle(style);
        }

        basic_string<char> keywordListInstruction("");
        basic_string<char> keywordListType("");
        basic_string<char> keywordListInstruction2("");

        if (pKwArray[LANG_INDEX_INSTR])
        {
            basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
            keywordListInstruction = wstring2string(kwlW, CP_ACP);
        }
        const char* jsInstrs = concatToBuildKeywordList(keywordListInstruction, L_JAVASCRIPT, LANG_INDEX_INSTR);

        if (pKwArray[LANG_INDEX_TYPE])
        {
            basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE];
            keywordListType = wstring2string(kwlW, CP_ACP);
        }
        const char* jsTypes = concatToBuildKeywordList(keywordListType, L_JAVASCRIPT, LANG_INDEX_TYPE);

        if (pKwArray[LANG_INDEX_INSTR2])
        {
            basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR2];
            keywordListInstruction2 = wstring2string(kwlW, CP_ACP);
        }
        const char* jsInstrs2 = concatToBuildKeywordList(keywordListInstruction2, L_JAVASCRIPT, LANG_INDEX_INSTR2);

        execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(jsInstrs));
        execute(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(jsTypes));
        execute(SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(jsInstrs2));

        populateSubStyleKeywords(L_JAVASCRIPT, SCE_C_IDENTIFIER, 8, LANG_INDEX_SUBSTYLE1, pKwArray);

    }
    else
    {
        const wchar_t* lexerName = ScintillaEditView::_langNameInfoArray[L_JS_EMBEDDED]._langName;
        LexerStyler* pOldStyler = (NppParameters::getInstance().getLStylerArray()).getLexerStylerByName(lexerName);

        if (pOldStyler)
        {
            for (Style style : *pOldStyler)
            {
                int cppID = style._styleID;

                switch (style._styleID)
                {
                    case SCE_HJ_DEFAULT: cppID = SCE_C_DEFAULT; break;
                    case SCE_HJ_WORD: cppID = SCE_C_IDENTIFIER; break;
                    case SCE_HJ_SYMBOLS: cppID = SCE_C_OPERATOR; break;
                    case SCE_HJ_COMMENT: cppID = SCE_C_COMMENT; break;
                    case SCE_HJ_COMMENTLINE: cppID = SCE_C_COMMENTLINE; break;
                    case SCE_HJ_COMMENTDOC: cppID = SCE_C_COMMENTDOC; break;
                    case SCE_HJ_NUMBER: cppID = SCE_C_NUMBER; break;
                    case SCE_HJ_KEYWORD: cppID = SCE_C_WORD; break;
                    case SCE_HJ_DOUBLESTRING: cppID = SCE_C_STRING; break;
                    case SCE_HJ_SINGLESTRING: cppID = SCE_C_CHARACTER; break;
                    case SCE_HJ_REGEX: cppID = SCE_C_REGEX; break;
                }
                style._styleID = cppID;
                setStyle(style);
            }
        }
        execute(SCI_STYLESETEOLFILLED, SCE_C_DEFAULT, true);
        execute(SCI_STYLESETEOLFILLED, SCE_C_COMMENTLINE, true);
        execute(SCI_STYLESETEOLFILLED, SCE_C_COMMENT, true);
        execute(SCI_STYLESETEOLFILLED, SCE_C_COMMENTDOC, true);

        makeStyle(L_JS_EMBEDDED, pKwArray);

        basic_string<char> keywordListInstruction("");
        if (pKwArray[LANG_INDEX_INSTR])
        {
            basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
            keywordListInstruction = wstring2string(kwlW, CP_ACP);
        }

        execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(concatToBuildKeywordList(keywordListInstruction, L_JS_EMBEDDED, LANG_INDEX_INSTR)));
        populateSubStyleKeywords(L_JS_EMBEDDED, SCE_C_IDENTIFIER, 8, LANG_INDEX_SUBSTYLE1, pKwArray);
    }

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.cpp.comment.explicit"), reinterpret_cast<LPARAM>("0"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.cpp.track.preprocessor"), reinterpret_cast<LPARAM>("0"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.cpp.backquoted.strings"), reinterpret_cast<LPARAM>("2"));
}

void ScintillaEditView::setTclLexer()
{
    setLexerFromLangID(L_TCL);

    const wchar_t* pKwArray[NB_LIST] = {NULL};
    makeStyle(L_TCL, pKwArray);

    basic_string<char> keywordList_TCL_KW("");
    basic_string<char> keywordList_TK_KW("");
    basic_string<char> keywordList_TK_CMD("");
    basic_string<char> keywordList_iTCL_KW("");
    basic_string<char> keywordList_EXPAND("");
    basic_string<char> keywordList_USER1("");
    basic_string<char> keywordList_USER2("");
    basic_string<char> keywordList_USER3("");
    basic_string<char> keywordList_USER4("");

    if (pKwArray[LANG_INDEX_INSTR])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
        keywordList_TCL_KW = wstring2string(kwlW, CP_ACP);
    }
    const char* kw_TCL_KW = concatToBuildKeywordList(keywordList_TCL_KW, L_TCL, LANG_INDEX_INSTR);

    if (pKwArray[LANG_INDEX_INSTR2])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR2];
        keywordList_TK_KW = wstring2string(kwlW, CP_ACP);
    }
    const char* kw_TK_KW = concatToBuildKeywordList(keywordList_TK_KW, L_TCL, LANG_INDEX_INSTR2);

    if (pKwArray[LANG_INDEX_TYPE])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE];
        keywordList_iTCL_KW = wstring2string(kwlW, CP_ACP);
    }
    const char* kw_iTCL_KW = concatToBuildKeywordList(keywordList_iTCL_KW, L_TCL, LANG_INDEX_TYPE);

    if (pKwArray[LANG_INDEX_TYPE2])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE2];
        keywordList_TK_CMD = wstring2string(kwlW, CP_ACP);
    }
    const char* kw_TK_CMD = concatToBuildKeywordList(keywordList_TK_CMD, L_TCL, LANG_INDEX_TYPE2);

    if (pKwArray[LANG_INDEX_TYPE3])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE3];
        keywordList_EXPAND = wstring2string(kwlW, CP_ACP);
    }
    const char* kw_EXPAND = concatToBuildKeywordList(keywordList_EXPAND, L_TCL, LANG_INDEX_TYPE3);

    if (pKwArray[LANG_INDEX_TYPE4])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE4];
        keywordList_USER1 = wstring2string(kwlW, CP_ACP);
    }
    const char* kw_USER1 = concatToBuildKeywordList(keywordList_USER1, L_TCL, LANG_INDEX_TYPE4);

    if (pKwArray[LANG_INDEX_TYPE5])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE5];
        keywordList_USER2 = wstring2string(kwlW, CP_ACP);
    }
    const char* kw_USER2 = concatToBuildKeywordList(keywordList_USER2, L_TCL, LANG_INDEX_TYPE5);

    if (pKwArray[LANG_INDEX_TYPE6])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE6];
        keywordList_USER3 = wstring2string(kwlW, CP_ACP);
    }
    const char* kw_USER3 = concatToBuildKeywordList(keywordList_USER3, L_TCL, LANG_INDEX_TYPE6);

    if (pKwArray[LANG_INDEX_TYPE7])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_TYPE7];
        keywordList_USER4 = wstring2string(kwlW, CP_ACP);
    }
    const char* kw_USER4 = concatToBuildKeywordList(keywordList_USER4, L_TCL, LANG_INDEX_TYPE7);

    execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(kw_TCL_KW));
    execute(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(kw_iTCL_KW));
    execute(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(kw_TK_KW));
    execute(SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(kw_TK_CMD));
    execute(SCI_SETKEYWORDS, 4, reinterpret_cast<LPARAM>(kw_EXPAND));
    execute(SCI_SETKEYWORDS, 5, reinterpret_cast<LPARAM>(kw_USER1));
    execute(SCI_SETKEYWORDS, 6, reinterpret_cast<LPARAM>(kw_USER2));
    execute(SCI_SETKEYWORDS, 7, reinterpret_cast<LPARAM>(kw_USER3));
    execute(SCI_SETKEYWORDS, 8, reinterpret_cast<LPARAM>(kw_USER4));
}

void ScintillaEditView::setObjCLexer(LangType langType)
{
    setLexerFromLangID(L_OBJC);

    const wchar_t* pKwArray[NB_LIST] = {NULL};

    makeStyle(langType, pKwArray);

    basic_string<char> objcInstr1Kwl("");
    if (pKwArray[LANG_INDEX_INSTR])
    {
        objcInstr1Kwl = wstring2string(pKwArray[LANG_INDEX_INSTR], CP_ACP);
    }
    const char* objcInstrs = concatToBuildKeywordList(objcInstr1Kwl, langType, LANG_INDEX_INSTR);

    basic_string<char> objcInstr2Kwl("");
    if (pKwArray[LANG_INDEX_INSTR2])
    {
        objcInstr2Kwl = wstring2string(pKwArray[LANG_INDEX_INSTR2], CP_ACP);
    }
    const char* objCDirective = concatToBuildKeywordList(objcInstr2Kwl, langType, LANG_INDEX_INSTR2);

    basic_string<char> objcTypeKwl("");
    if (pKwArray[LANG_INDEX_TYPE])
    {
        objcTypeKwl = wstring2string(pKwArray[LANG_INDEX_TYPE], CP_ACP);
    }
    const char* objcTypes = concatToBuildKeywordList(objcTypeKwl, langType, LANG_INDEX_TYPE);

    basic_string<char> objcType2Kwl("");
    if (pKwArray[LANG_INDEX_TYPE2])
    {
        objcType2Kwl = wstring2string(pKwArray[LANG_INDEX_TYPE2], CP_ACP);
    }
    const char* objCQualifier = concatToBuildKeywordList(objcType2Kwl, langType, LANG_INDEX_TYPE2);

    basic_string<char> doxygenKeyWordsString("");
    const wchar_t* doxygenKeyWordsW = NppParameters::getInstance().getWordList(L_CPP, LANG_INDEX_TYPE2);
    if (doxygenKeyWordsW)
    {
        doxygenKeyWordsString = wstring2string(doxygenKeyWordsW, CP_ACP);
    }
    const char* doxygenKeyWords = doxygenKeyWordsString.c_str();

    execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(objcInstrs));
    execute(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(objcTypes));
    execute(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(doxygenKeyWords));
    execute(SCI_SETKEYWORDS, 3, reinterpret_cast<LPARAM>(objCDirective));
    execute(SCI_SETKEYWORDS, 4, reinterpret_cast<LPARAM>(objCQualifier));

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.cpp.comment.explicit"), reinterpret_cast<LPARAM>("0"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));
}

void ScintillaEditView::setTypeScriptLexer()
{
    const wchar_t* doxygenKeyWords = NppParameters::getInstance().getWordList(L_CPP, LANG_INDEX_TYPE2);
    setLexerFromLangID(L_TYPESCRIPT);

    if (doxygenKeyWords)
    {
        WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
        const char* doxygenKeyWords_char = wmc.wchar2char(doxygenKeyWords, CP_ACP);
        execute(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(doxygenKeyWords_char));
    }

    const wchar_t* pKwArray[NB_LIST] = {NULL};
    makeStyle(L_TYPESCRIPT, pKwArray);

    auto getKeywordList = [&pKwArray](const int i)
    {
        if (pKwArray[i])
        {
            basic_string<wchar_t> kwlW = pKwArray[i];
            return wstring2string(kwlW, CP_ACP);
        }
        return basic_string<char>("");
    };

    std::string keywordListInstruction = getKeywordList(LANG_INDEX_INSTR);
    const char* tsInstructions = concatToBuildKeywordList(keywordListInstruction, L_TYPESCRIPT, LANG_INDEX_INSTR);

    string keywordListType = getKeywordList(LANG_INDEX_TYPE);
    const char* tsTypes = concatToBuildKeywordList(keywordListType, L_TYPESCRIPT, LANG_INDEX_TYPE);

    execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(tsInstructions));
    execute(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(tsTypes));

    populateSubStyleKeywords(L_TYPESCRIPT, SCE_C_IDENTIFIER, 8, LANG_INDEX_SUBSTYLE1, pKwArray);

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.cpp.comment.explicit"), reinterpret_cast<LPARAM>("0"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.cpp.track.preprocessor"), reinterpret_cast<LPARAM>("0"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.cpp.backquoted.strings"), reinterpret_cast<LPARAM>("1"));
}

void ScintillaEditView::setXmlLexer(LangType type)
{
    if (type == L_XML)
    {
        const wchar_t* pKwArray[NB_LIST] = {NULL};

        setLexerFromLangID(L_XML);
        makeStyle(type, pKwArray);

        basic_string<char> keywordList("");
        if (pKwArray[LANG_INDEX_INSTR])
        {
            basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
            keywordList = wstring2string(kwlW, CP_ACP);
        }

        execute(SCI_SETKEYWORDS, 5, reinterpret_cast<LPARAM>(concatToBuildKeywordList(keywordList, L_XML, LANG_INDEX_INSTR)));

        populateSubStyleKeywords(type, SCE_H_ATTRIBUTE, 8, LANG_INDEX_SUBSTYLE1, pKwArray);

        execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.xml.allow.scripts"), reinterpret_cast<LPARAM>("0"));
    }
    else if ((type == L_HTML) || (type == L_PHP) || (type == L_ASP) || (type == L_JSP))
    {
        setLexerFromLangID(L_HTML);

        setHTMLLexer();
        setEmbeddedJSLexer();
        setEmbeddedPhpLexer();
        setEmbeddedAspLexer();
    }
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.html"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.hypertext.comment"), reinterpret_cast<LPARAM>("1"));
}

void ScintillaEditView::setHTMLLexer()
{
    const wchar_t* pKwArray[NB_LIST]{};
    makeStyle(L_HTML, pKwArray);

    std::string keywordList;
    if (pKwArray[LANG_INDEX_INSTR])
    {
        std::wstring kwlW = pKwArray[LANG_INDEX_INSTR];
        keywordList = wstring2string(kwlW, CP_ACP);
    }

    execute(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(concatToBuildKeywordList(keywordList, L_HTML, LANG_INDEX_INSTR)));

    std::string keywordList2;
    if (pKwArray[LANG_INDEX_INSTR2])
    {
        std::wstring kwlW = pKwArray[LANG_INDEX_INSTR2];
        keywordList2 = wstring2string(kwlW, CP_ACP);
    }

    execute(SCI_SETKEYWORDS, 5, reinterpret_cast<LPARAM>(concatToBuildKeywordList(keywordList2, L_HTML, LANG_INDEX_INSTR2)));

    populateSubStyleKeywords(L_HTML, SCE_H_TAG, 4, LANG_INDEX_SUBSTYLE1, pKwArray);
    populateSubStyleKeywords(L_HTML, SCE_H_ATTRIBUTE, 4, LANG_INDEX_SUBSTYLE5, pKwArray);
}

void ScintillaEditView::setEmbeddedJSLexer()
{
    const wchar_t* pKwArray[NB_LIST] = {NULL};
    makeStyle(L_JS_EMBEDDED, pKwArray);

    basic_string<char> keywordList("");
    if (pKwArray[LANG_INDEX_INSTR])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
        keywordList = wstring2string(kwlW, CP_ACP);
    }

    execute(SCI_SETKEYWORDS, 1, reinterpret_cast<LPARAM>(concatToBuildKeywordList(keywordList, L_JS_EMBEDDED, LANG_INDEX_INSTR)));
    populateSubStyleKeywords(L_JS_EMBEDDED, SCE_HJ_WORD, 8, LANG_INDEX_SUBSTYLE1, pKwArray);
    execute(SCI_STYLESETEOLFILLED, SCE_HJ_DEFAULT, true);
    execute(SCI_STYLESETEOLFILLED, SCE_HJ_COMMENT, true);
    execute(SCI_STYLESETEOLFILLED, SCE_HJ_COMMENTDOC, true);
    execute(SCI_STYLESETEOLFILLED, SCE_HJ_TEMPLATELITERAL, true);
    execute(SCI_STYLESETEOLFILLED, SCE_HJA_TEMPLATELITERAL, true);
}

void ScintillaEditView::setEmbeddedPhpLexer()
{
    const wchar_t* pKwArray[NB_LIST] = {NULL};
    makeStyle(L_PHP, pKwArray);

    basic_string<char> keywordList("");
    if (pKwArray[LANG_INDEX_INSTR])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
        keywordList = wstring2string(kwlW, CP_ACP);
    }

    execute(SCI_SETKEYWORDS, 4, reinterpret_cast<LPARAM>(concatToBuildKeywordList(keywordList, L_PHP, LANG_INDEX_INSTR)));
    populateSubStyleKeywords(L_PHP, SCE_HPHP_WORD, 8, LANG_INDEX_SUBSTYLE1, pKwArray);

    execute(SCI_STYLESETEOLFILLED, SCE_HPHP_DEFAULT, true);
    execute(SCI_STYLESETEOLFILLED, SCE_HPHP_COMMENT, true);
}

void ScintillaEditView::setEmbeddedAspLexer()
{
    const wchar_t* pKwArray[NB_LIST] = {NULL};
    makeStyle(L_ASP, pKwArray);

    basic_string<char> keywordList("");
    if (pKwArray[LANG_INDEX_INSTR])
    {
        basic_string<wchar_t> kwlW = pKwArray[LANG_INDEX_INSTR];
        keywordList = wstring2string(kwlW, CP_ACP);
    }

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("asp.default.language"), reinterpret_cast<LPARAM>("2"));

    execute(SCI_SETKEYWORDS, 2, reinterpret_cast<LPARAM>(concatToBuildKeywordList(keywordList, L_VB, LANG_INDEX_INSTR)));

    populateSubStyleKeywords(L_ASP, SCE_HB_WORD, 8, LANG_INDEX_SUBSTYLE1, pKwArray);

    execute(SCI_STYLESETEOLFILLED, SCE_HBA_DEFAULT, true);
}

void ScintillaEditView::setUserLexer(const wchar_t* userLangName)
{
    setLexerFromLangID(L_USER);

    const UserLangContainer* userLangContainer = userLangName ? NppParameters::getInstance().getULCFromName(userLangName) : nullptr;

    if (!userLangContainer)
        return;

    UINT codepage = CP_ACP;
    UniMode unicodeMode = static_cast<UniMode>(_currentBuffer->getUnicodeMode());
    int encoding = _currentBuffer->getEncodingNumber();
    if (encoding == -1)
    {
        if (unicodeMode == uniUTF8 || unicodeMode == uniUTF8_NoBOM)
            codepage = CP_UTF8;
    }
    else
    {
        codepage = CP_OEMCP;
    }

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("userDefine.isCaseIgnored"), reinterpret_cast<LPARAM>(userLangContainer->_isCaseIgnored ? "1":"0"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("userDefine.allowFoldOfComments"), reinterpret_cast<LPARAM>(userLangContainer->_allowFoldOfComments ? "1":"0"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("userDefine.foldCompact"), reinterpret_cast<LPARAM>(userLangContainer->_foldCompact ? "1":"0"));

    for (const Style& style : userLangContainer->_styles)
    {
        if (style._styleID == STYLE_NOT_USED)
            continue;

        setStyle(style);
    }
}

void ScintillaEditView::setExternalLexer(LangType typeDoc)
{
    int id = typeDoc - L_EXTERNAL;

    const ExternalLangContainer* externalLexer = NppParameters::getInstance().getELCFromIndex(id);
    if (!externalLexer || !externalLexer->fnCL)
        return;
    ILexer5* iLex5 = externalLexer->fnCL(externalLexer->_name.c_str());
    if (!iLex5)
        return;
    execute(SCI_SETILEXER, 0, reinterpret_cast<LPARAM>(iLex5));

    WcharMbcsConvertor& wmc = WcharMbcsConvertor::getInstance();
    const wchar_t* lexerNameW = wmc.char2wchar(externalLexer->_name.c_str(), CP_UTF8);
    LexerStyler* pStyler = (NppParameters::getInstance().getLStylerArray()).getLexerStylerByName(lexerNameW);
    if (pStyler)
    {
        for (const Style& style : *pStyler)
        {
            setStyle(style);

            if (style._keywordClass >= 0 && style._keywordClass <= KEYWORDSET_MAX)
            {
                basic_string<char> keywordList("");
                if (!style._keywords.empty())
                {
                    keywordList = wstring2string(style._keywords, CP_ACP);
                }
                execute(SCI_SETKEYWORDS, style._keywordClass, reinterpret_cast<LPARAM>(concatToBuildKeywordList(keywordList, typeDoc, style._keywordClass)));
            }
        }
    }
}

// ============================================================================
// Simple Lexers
// ============================================================================
// Note: setCssLexer, setLuaLexer, setMakefileLexer, setPropsLexer, setSqlLexer,
// setBashLexer, and setVBLexer are defined inline in ScintillaEditView.h

// ============================================================================
// Initialization
// ============================================================================
void ScintillaEditView::init(HINSTANCE hInst, HWND hPere)
{
    (void)hInst;
    (void)hPere;
    // TODO: Implement Qt-specific initialization
    // This is a stub - proper implementation needs Scintilla Qt initialization
    throw std::runtime_error("ScintillaEditView::init: Qt implementation not yet complete");
}

// ============================================================================
// Destructor
// ============================================================================
// Out-of-line destructor definition to provide a key function for the vtable.
// This ensures the vtable is emitted in this translation unit, preventing
// "undefined reference to vtable for ScintillaEditView" linker errors.
ScintillaEditView::~ScintillaEditView()
{
    --_refCount;

    if (!_refCount && _SciInit)
    {
        // Scintilla_ReleaseResources() is Windows-only
    }
}

#endif // NPP_LINUX
