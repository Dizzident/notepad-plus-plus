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

#include "ScintillaEditView.h"
#include "Parameters.h"
#include "Common.h"

#include <algorithm>
#include <vector>
#include <string>
#include <unordered_set>

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
