// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

// On Linux, this header provides minimal stub definitions for shortcut-related
// classes that are needed by the core code but are Windows-specific.
// The full Qt implementation of shortcut management is in ShortcutMapper.

#include <cstring>
#include <string>
#include <vector>

// Forward declarations for types needed from Scintilla
#include <Scintilla.h>

// KeyCombo - same as Windows version
struct KeyCombo {
    bool _isCtrl = false;
    bool _isAlt = false;
    bool _isShift = false;
    unsigned char _key = 0;
};

// Minimal Shortcut class for Linux - data only, no UI
class Shortcut {
public:
    Shortcut() : _canModifyName(false) {
        setName("");
        _keyCombo._isCtrl = false;
        _keyCombo._isAlt = false;
        _keyCombo._isShift = false;
        _keyCombo._key = 0;
    }

    Shortcut(const char* name, bool isCtrl, bool isAlt, bool isShift, unsigned char key)
        : _canModifyName(false) {
        _name[0] = '\0';
        if (name) {
            setName(name);
        } else {
            setName("");
        }
        _keyCombo._isCtrl = isCtrl;
        _keyCombo._isAlt = isAlt;
        _keyCombo._isShift = isShift;
        _keyCombo._key = key;
    }

    virtual ~Shortcut() = default;

    const char* getName() const { return _name; }
    void setName(const char* name) {
        if (name) {
            strncpy(_name, name, sizeof(_name) - 1);
            _name[sizeof(_name) - 1] = '\0';
        } else {
            _name[0] = '\0';
        }
    }

    KeyCombo getKeyCombo() const { return _keyCombo; }
    void setKeyCombo(const KeyCombo& combo) { _keyCombo = combo; }

    virtual bool isValid() const {
        return (_keyCombo._key != 0);
    }

    bool canModifyName() const { return _canModifyName; }

    void clear() {
        _keyCombo._isCtrl = false;
        _keyCombo._isAlt = false;
        _keyCombo._isShift = false;
        _keyCombo._key = 0;
    }

protected:
    char _name[128] = {0};  // menuItemStrLenMax equivalent
    KeyCombo _keyCombo;
    bool _canModifyName = false;
};

// CommandShortcut extends Shortcut with command ID
class CommandShortcut : public Shortcut {
public:
    CommandShortcut() : Shortcut(), _id(0) {}
    CommandShortcut(const Shortcut& sc, int id) : Shortcut(sc), _id(id) {}
    CommandShortcut(const Shortcut& sc, int id, bool isDuplicate) : Shortcut(sc), _id(id), _isDuplicate(isDuplicate) {}

    int getID() const { return _id; }
    bool isDuplicate() const { return _isDuplicate; }

protected:
    int _id = 0;
    bool _isDuplicate = false;
};

// Accelerator stub for Linux
class Accelerator {
public:
    Accelerator() = default;
    ~Accelerator() = default;

    // Stub implementations - Qt handles accelerators natively
    void updateShortcuts() {}
    void updateFullMenu() {}
};

// ScintillaAccelerator stub for Linux
class ScintillaAccelerator {
public:
    ScintillaAccelerator() = default;
    ~ScintillaAccelerator() = default;
};

// ScintillaKeyMap stub
class ScintillaKeyMap {
public:
    ScintillaKeyMap() = default;
    ~ScintillaKeyMap() = default;
};

// recordedMacroStep for macro support
struct recordedMacroStep {
    enum MacroTypeIndex {mtUseLParameter, mtUseSParameter, mtMenuCommand, mtSavedSnR};

    int _message = 0;
    uptr_t _wParameter = 0;
    uptr_t _lParameter = 0;
    std::string _sParameter;
    MacroTypeIndex _macroType = mtMenuCommand;

    recordedMacroStep(int iMessage, uptr_t wParam, uptr_t lParam) :
        _message(iMessage), _wParameter(wParam), _lParameter(lParam) {}

    explicit recordedMacroStep(int iCommandID) : _wParameter(iCommandID) {}

    recordedMacroStep(int iMessage, uptr_t wParam, uptr_t lParam, const char* sParam, int type)
        : _message(iMessage), _wParameter(wParam), _lParameter(lParam),
          _sParameter((sParam != nullptr) ? sParam : ""),
          _macroType(static_cast<MacroTypeIndex>(type)) {}

    bool isScintillaMacro() const { return _macroType <= mtMenuCommand; }
    bool isMacroable() const;
};

typedef std::vector<recordedMacroStep> Macro;

// MacroShortcut
class MacroShortcut : public CommandShortcut {
public:
    MacroShortcut(const Shortcut& sc, const Macro& macro, int id) :
        CommandShortcut(sc, id), _macro(macro) { _canModifyName = true; }
    Macro& getMacro() { return _macro; }
private:
    Macro _macro;
};

// UserCommand
class UserCommand : public CommandShortcut {
public:
    UserCommand(const Shortcut& sc, const char* cmd, int id) :
        CommandShortcut(sc, id), _cmd(cmd) { _canModifyName = true; }
    const char* getCmd() const { return _cmd.c_str(); }
private:
    std::string _cmd;
};

// PluginCmdShortcut
class PluginCmdShortcut : public CommandShortcut {
public:
    PluginCmdShortcut(const Shortcut& sc, int id, const char* moduleName, unsigned short internalID) :
        CommandShortcut(sc, id), _id(id), _moduleName(moduleName), _internalID(internalID) {}

    bool isValid() const override {
        if (!Shortcut::isValid())
            return false;
        if ((!_moduleName[0]) || (_internalID == -1))
            return false;
        return true;
    }

    const char* getModuleName() const { return _moduleName.c_str(); }
    int getInternalID() const { return _internalID; }
    unsigned long getID() const { return _id; }

private:
    unsigned long _id = 0;
    std::string _moduleName;
    int _internalID = 0;
};
