// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <string>
#include <vector>

// MenuItemUnit - same as Windows version
struct MenuItemUnit final {
    unsigned long _cmdID = 0;
    std::wstring _itemName;
    std::wstring _parentFolderName;

    MenuItemUnit() = default;
    MenuItemUnit(unsigned long cmdID, const std::wstring& itemName, const std::wstring& parentFolderName = std::wstring())
        : _cmdID(cmdID), _itemName(itemName), _parentFolderName(parentFolderName) {}
    MenuItemUnit(unsigned long cmdID, const wchar_t* itemName, const wchar_t* parentFolderName = nullptr);
};

// Stub ContextMenu class for Linux
// The actual context menu implementation is in the Qt MainWindow
class ContextMenu final {
public:
    ~ContextMenu() {
        destroy();
    }

    // Stubs - actual implementation uses Qt's QMenu
    void create(void* hParent, const std::vector<MenuItemUnit> & menuItemArray, const void* mainMenuHandle = nullptr, bool copyLink = false) {
        (void)hParent;
        (void)menuItemArray;
        (void)mainMenuHandle;
        (void)copyLink;
    }

    bool isCreated() const { return _isCreated; }

    void display(const void* p) const {
        (void)p;
        // Qt implementation would show QMenu here
    }

    void display(void* hwnd) const {
        (void)hwnd;
    }

    void enableItem(int cmdID, bool doEnable) const {
        (void)cmdID;
        (void)doEnable;
    }

    void checkItem(int cmdID, bool doCheck) const {
        (void)cmdID;
        (void)doCheck;
    }

    void destroy() {
        _isCreated = false;
    }

private:
    bool _isCreated = false;
};
