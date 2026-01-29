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
// NativeLangSpeaker.cpp - Linux/Qt implementation
// ============================================================================
// This file provides Linux-compatible implementations of NativeLangSpeaker
// methods that are needed for the Qt6 port. These are stub implementations
// that provide basic functionality.
// ============================================================================

#include "localization.h"

using namespace std;

wstring NativeLangSpeaker::getShortcutMapperLangStr(const char *nodeName, const wchar_t* defaultStr) const
{
    (void)nodeName;
    // Return default string for now
    if (defaultStr)
        return wstring(defaultStr);
    return L"";
}

void NativeLangSpeaker::resizeCheckboxRadioBtn(HWND hWnd)
{
    (void)hWnd;
    // Stub implementation - no-op for Linux/Qt
    // This function is used on Windows to resize checkboxes/radio buttons
    // based on localized text length. On Qt, layout management handles this.
}
