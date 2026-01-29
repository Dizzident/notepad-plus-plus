// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

// Linux/Qt implementation of LastRecentFileList
// This is a stub implementation for the Linux port

#include "lastRecentFileList.h"
#include "menuCmdID.h"
#include "localization.h"
#include <algorithm>

using namespace std;

void LastRecentFileList::initMenu(HMENU hMenu, int idBase, int posBase, Accelerator *pAccelerator, bool doSubMenu)
{
    (void)hMenu;
    (void)idBase;
    (void)posBase;
    (void)pAccelerator;
    (void)doSubMenu;
    // Qt/Linux: Menu handling is done through Qt's menu system
    // This is a stub for compatibility
    _idBase = idBase;
    _posBase = posBase;
    _pAccelerator = pAccelerator;
    _nativeLangEncoding = 0;

    for (size_t i = 0; i < sizeof(_idFreeArray); ++i)
        _idFreeArray[i] = true;
}

void LastRecentFileList::switchMode()
{
    // Qt/Linux: Menu mode switching not applicable
    _hasSeparators = false;
}

void LastRecentFileList::updateMenu()
{
    // Qt/Linux: Menu updates are handled by the Qt menu system
    // This is a stub for compatibility
    if (_pAccelerator)
        _pAccelerator->updateFullMenu();
}

void LastRecentFileList::add(const wchar_t *fn)
{
    if (_userMax == 0 || _locked)
        return;

    RecentItem itemToAdd(fn);

    int index = find(fn);
    if (index != -1)
    {
        // Already in list, bump upwards
        remove(index);
    }

    if (_size == _userMax)
    {
        itemToAdd._id = _lrfl.back()._id;
        _lrfl.pop_back(); // Remove oldest
    }
    else
    {
        itemToAdd._id = popFirstAvailableID();
        ++_size;
    }
    _lrfl.push_front(itemToAdd);
    updateMenu();
}

void LastRecentFileList::remove(const wchar_t *fn)
{
    int index = find(fn);
    if (index != -1)
        remove(index);
}

void LastRecentFileList::remove(size_t index)
{
    if (_size == 0 || _locked)
        return;
    if (index < _lrfl.size())
    {
        setAvailable(_lrfl.at(index)._id);
        _lrfl.erase(_lrfl.begin() + index);
        --_size;
        updateMenu();
    }
}

void LastRecentFileList::clear()
{
    if (_size == 0)
        return;

    for (int i = (_size - 1); i >= 0; i--)
    {
        setAvailable(_lrfl.at(i)._id);
        _lrfl.erase(_lrfl.begin() + i);
    }
    _size = 0;
    updateMenu();
}

wstring & LastRecentFileList::getItem(int id)
{
    int i = 0;
    for (; i < _size; ++i)
    {
        if (_lrfl.at(i)._id == id)
            break;
    }

    if (i == _size)
        i = 0;

    return _lrfl.at(i)._name; // If not found, return first
}

wstring & LastRecentFileList::getIndex(int index)
{
    return _lrfl.at(index)._name; // If not found, return first
}

void LastRecentFileList::setUserMaxNbLRF(int size)
{
    _userMax = size;
    if (_size > _userMax)
    {
        // Start popping items
        int toPop = _size - _userMax;
        while (toPop > 0)
        {
            setAvailable(_lrfl.back()._id);
            _lrfl.pop_back();
            toPop--;
            _size--;
        }

        updateMenu();
        _size = _userMax;
    }
}

void LastRecentFileList::saveLRFL()
{
    NppParameters& nppParams = NppParameters::getInstance();
    if (nppParams.writeRecentFileHistorySettings(_userMax))
    {
        for (int i = _size - 1; i >= 0; i--) // Reverse order: so loading goes in correct order
        {
            nppParams.writeHistory(_lrfl.at(i)._name.c_str());
        }
    }
}

int LastRecentFileList::find(const wchar_t *fn)
{
    for (int i = 0; i < _size; ++i)
    {
        if (wcscmp(_lrfl.at(i)._name.c_str(), fn) == 0)
        {
            return i;
        }
    }
    return -1;
}

int LastRecentFileList::popFirstAvailableID()
{
    for (int i = 0; i < NB_MAX_LRF_FILE; ++i)
    {
        if (_idFreeArray[i])
        {
            _idFreeArray[i] = false;
            return i + _idBase;
        }
    }
    return 0;
}

void LastRecentFileList::setAvailable(int id)
{
    int index = id - _idBase;
    if (index >= 0 && index < NB_MAX_LRF_FILE)
        _idFreeArray[index] = true;
}
