// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ShortcutManager.h"

#include <QAction>
#include <QMenu>
#include <QDebug>

#include "../../Parameters.h"
#include "../../menuCmdID.h"
#include "../../MISC/Common/Common.h"

// Define missing VK_ constants for Linux
#ifndef VK_NUMPAD0
#define VK_NUMPAD0 0x60
#endif
#ifndef VK_NUMPAD9
#define VK_NUMPAD9 0x69
#endif
#ifndef VK_MULTIPLY
#define VK_MULTIPLY 0x6A
#endif
#ifndef VK_ADD
#define VK_ADD 0x6B
#endif
#ifndef VK_SUBTRACT
#define VK_SUBTRACT 0x6D
#endif
#ifndef VK_DECIMAL
#define VK_DECIMAL 0x6E
#endif
#ifndef VK_DIVIDE
#define VK_DIVIDE 0x6F
#endif
#ifndef VK_OEM_PLUS
#define VK_OEM_PLUS 0xBB
#endif
#ifndef VK_OEM_MINUS
#define VK_OEM_MINUS 0xBD
#endif
#ifndef VK_OEM_COMMA
#define VK_OEM_COMMA 0xBC
#endif
#ifndef VK_OEM_PERIOD
#define VK_OEM_PERIOD 0xBE
#endif
#ifndef VK_OEM_1
#define VK_OEM_1 0xBA
#endif

namespace QtControls {

// Static instance
ShortcutManager* ShortcutManager::_instance = nullptr;

ShortcutManager::ShortcutManager(QObject* parent)
    : QObject(parent)
{
}

ShortcutManager::~ShortcutManager() = default;

ShortcutManager* ShortcutManager::getInstance()
{
    if (!_instance) {
        _instance = new ShortcutManager();
    }
    return _instance;
}

void ShortcutManager::initialize()
{
    loadDefaultShortcuts();
    applyShortcuts();
}

void ShortcutManager::loadDefaultShortcuts()
{
    // Load default shortcuts from NppParameters
    NppParameters& nppParams = NppParameters::getInstance();

    // Note: On Linux, the default shortcuts are loaded from winKeyDefs
    // in QtCore/Parameters.cpp during NppParameters::load()
}

void ShortcutManager::registerAction(int commandId, QAction* action, const QString& category)
{
    if (!action) {
        return;
    }

    // Unregister if already registered
    if (_commands.contains(commandId)) {
        unregisterAction(commandId);
    }

    CommandInfo info;
    info.commandId = commandId;
    info.name = action->text();
    info.category = category;
    info.action = action;

    _commands[commandId] = info;
    _actionToCommandId[action] = commandId;

    // Apply shortcut if one exists in NppParameters
    applyShortcut(commandId, getShortcut(commandId));
}

void ShortcutManager::registerMenuActions(QMenu* menu, const QString& category)
{
    if (!menu) {
        return;
    }

    for (QAction* action : menu->actions()) {
        if (action->isSeparator()) {
            continue;
        }

        if (action->menu()) {
            // Recursively register submenu actions
            registerMenuActions(action->menu(), category.isEmpty() ? action->text() : category);
        } else {
            // Try to determine command ID from action data or property
            QVariant cmdIdVar = action->property("commandId");
            if (cmdIdVar.isValid()) {
                int commandId = cmdIdVar.toInt();
                registerAction(commandId, action, category);
            }
        }
    }
}

void ShortcutManager::unregisterAction(int commandId)
{
    auto it = _commands.find(commandId);
    if (it != _commands.end()) {
        _actionToCommandId.remove(it.value().action);
        _commands.erase(it);
    }
}

void ShortcutManager::unregisterAction(QAction* action)
{
    auto it = _actionToCommandId.find(action);
    if (it != _actionToCommandId.end()) {
        unregisterAction(it.value());
    }
}

void ShortcutManager::applyShortcuts()
{
    NppParameters& nppParams = NppParameters::getInstance();

    // Apply main menu shortcuts
    std::vector<CommandShortcut>& shortcuts = nppParams.getUserShortcuts();
    for (const CommandShortcut& sc : shortcuts) {
        int cmdId = sc.getID();
        KeyCombo combo = sc.getKeyCombo();

        auto it = _commands.find(cmdId);
        if (it != _commands.end()) {
            it.value().keyCombo = combo;
            it.value().isEnabled = sc.isValid();
            applyShortcut(cmdId, combo);
        }
    }

    // Apply macro shortcuts
    std::vector<MacroShortcut>& macros = nppParams.getMacroList();
    for (const MacroShortcut& sc : macros) {
        int cmdId = sc.getID();
        KeyCombo combo = sc.getKeyCombo();

        auto it = _commands.find(cmdId);
        if (it != _commands.end()) {
            it.value().keyCombo = combo;
            it.value().isEnabled = sc.isValid();
            applyShortcut(cmdId, combo);
        }
    }

    // Apply user command shortcuts
    std::vector<UserCommand>& userCommands = nppParams.getUserCommandList();
    for (const UserCommand& sc : userCommands) {
        int cmdId = sc.getID();
        KeyCombo combo = sc.getKeyCombo();

        auto it = _commands.find(cmdId);
        if (it != _commands.end()) {
            it.value().keyCombo = combo;
            it.value().isEnabled = sc.isValid();
            applyShortcut(cmdId, combo);
        }
    }

    // Apply plugin command shortcuts
    std::vector<PluginCmdShortcut>& pluginCommands = nppParams.getPluginCommandList();
    for (const PluginCmdShortcut& sc : pluginCommands) {
        int cmdId = static_cast<int>(sc.getID());
        KeyCombo combo = sc.getKeyCombo();

        auto it = _commands.find(cmdId);
        if (it != _commands.end()) {
            it.value().keyCombo = combo;
            it.value().isEnabled = sc.isValid();
            applyShortcut(cmdId, combo);
        }
    }

    emit shortcutsReloaded();
}

void ShortcutManager::applyShortcut(int commandId, const KeyCombo& combo)
{
    auto it = _commands.find(commandId);
    if (it == _commands.end()) {
        return;
    }

    QAction* action = it.value().action;
    if (!action) {
        return;
    }

    if (isValidKeyCombo(combo)) {
        QKeySequence seq = keyComboToQKeySequence(combo);
        action->setShortcut(seq);
        action->setShortcutContext(Qt::ApplicationShortcut);
    } else {
        action->setShortcut(QKeySequence());
    }

    it.value().keyCombo = combo;
    it.value().isEnabled = isValidKeyCombo(combo);
}

KeyCombo ShortcutManager::getShortcut(int commandId) const
{
    // First check registered commands
    auto it = _commands.find(commandId);
    if (it != _commands.end() && it.value().isEnabled) {
        return it.value().keyCombo;
    }

    // Otherwise, look up in NppParameters
    NppParameters& nppParams = const_cast<NppParameters&>(NppParameters::getInstance());

    // Check main menu shortcuts
    std::vector<CommandShortcut>& shortcuts = nppParams.getUserShortcuts();
    for (const CommandShortcut& sc : shortcuts) {
        if (sc.getID() == commandId) {
            return sc.getKeyCombo();
        }
    }

    // Check macro shortcuts
    std::vector<MacroShortcut>& macros = nppParams.getMacroList();
    for (const MacroShortcut& sc : macros) {
        if (sc.getID() == commandId) {
            return sc.getKeyCombo();
        }
    }

    // Check user commands
    std::vector<UserCommand>& userCommands = nppParams.getUserCommandList();
    for (const UserCommand& sc : userCommands) {
        if (sc.getID() == commandId) {
            return sc.getKeyCombo();
        }
    }

    // Check plugin commands
    std::vector<PluginCmdShortcut>& pluginCommands = nppParams.getPluginCommandList();
    for (const PluginCmdShortcut& sc : pluginCommands) {
        if (static_cast<int>(sc.getID()) == commandId) {
            return sc.getKeyCombo();
        }
    }

    return KeyCombo();
}

QString ShortcutManager::keyComboToString(const KeyCombo& combo)
{
    if (combo._key == 0) {
        return QString();
    }

    QStringList parts;
    if (combo._isCtrl) parts << "Ctrl";
    if (combo._isAlt) parts << "Alt";
    if (combo._isShift) parts << "Shift";

    // Convert key code to string
    QString keyStr;
    if (combo._key >= 'A' && combo._key <= 'Z') {
        keyStr = QString(QChar::fromLatin1(combo._key));
    } else if (combo._key >= '0' && combo._key <= '9') {
        keyStr = QString(QChar::fromLatin1(combo._key));
    } else if (combo._key >= VK_F1 && combo._key <= VK_F24) {
        keyStr = QString("F%1").arg(combo._key - VK_F1 + 1);
    } else {
        switch (combo._key) {
            case VK_SPACE: keyStr = QString("Space"); break;
            case VK_RETURN: keyStr = QString("Enter"); break;
            case VK_ESCAPE: keyStr = QString("Esc"); break;
            case VK_TAB: keyStr = QString("Tab"); break;
            case VK_BACK: keyStr = QString("Backspace"); break;
            case VK_DELETE: keyStr = QString("Delete"); break;
            case VK_INSERT: keyStr = QString("Insert"); break;
            case VK_HOME: keyStr = QString("Home"); break;
            case VK_END: keyStr = QString("End"); break;
            case VK_PRIOR: keyStr = QString("PageUp"); break;
            case VK_NEXT: keyStr = QString("PageDown"); break;
            case VK_LEFT: keyStr = QString("Left"); break;
            case VK_RIGHT: keyStr = QString("Right"); break;
            case VK_UP: keyStr = QString("Up"); break;
            case VK_DOWN: keyStr = QString("Down"); break;
            default: keyStr = QString("Key%1").arg(combo._key); break;
        }
    }

    parts << keyStr;
    return parts.join("+");
}

QKeySequence ShortcutManager::keyComboToQKeySequence(const KeyCombo& combo)
{
    if (combo._key == 0) {
        return QKeySequence();
    }

    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    if (combo._isCtrl) modifiers |= Qt::ControlModifier;
    if (combo._isAlt) modifiers |= Qt::AltModifier;
    if (combo._isShift) modifiers |= Qt::ShiftModifier;

    Qt::Key key = virtualKeyToQtKey(combo._key);

    return QKeySequence(key | modifiers);
}

KeyCombo ShortcutManager::qKeySequenceToKeyCombo(const QKeySequence& seq)
{
    if (seq.isEmpty()) {
        return KeyCombo();
    }

    KeyCombo combo;
    int key = seq[0].toCombined();

    combo._isCtrl = (key & Qt::ControlModifier) != 0;
    combo._isAlt = (key & Qt::AltModifier) != 0;
    combo._isShift = (key & Qt::ShiftModifier) != 0;

    Qt::Key qtKey = static_cast<Qt::Key>(key & ~Qt::KeyboardModifierMask);
    combo._key = qtKeyToVirtualKey(qtKey);

    return combo;
}

bool ShortcutManager::isValidKeyCombo(const KeyCombo& combo)
{
    return combo._key != 0;
}

void ShortcutManager::updateShortcutsFromParameters()
{
    applyShortcuts();
}

void ShortcutManager::updateCommandShortcut(int commandId, const KeyCombo& newCombo)
{
    // Apply the new shortcut to the action
    applyShortcut(commandId, newCombo);

    // Update NppParameters
    NppParameters& nppParams = NppParameters::getInstance();
    bool found = false;

    // Try to find and update in main menu shortcuts
    std::vector<CommandShortcut>& shortcuts = nppParams.getUserShortcuts();
    for (size_t i = 0; i < shortcuts.size(); ++i) {
        if (shortcuts[i].getID() == commandId) {
            shortcuts[i].setKeyCombo(newCombo);
            nppParams.addUserModifiedIndex(i);
            found = true;
            break;
        }
    }

    // If not found, we may need to add it (this would be a new custom shortcut)
    // For now, we only update existing shortcuts

    // Mark shortcuts as dirty
    if (found) {
        nppParams.setShortcutDirty();
    }

    emit shortcutChanged(commandId, newCombo);
}

void ShortcutManager::clearCommandShortcut(int commandId)
{
    // Clear the shortcut by applying an empty KeyCombo
    KeyCombo emptyCombo;
    applyShortcut(commandId, emptyCombo);

    // Update NppParameters
    NppParameters& nppParams = NppParameters::getInstance();

    std::vector<CommandShortcut>& shortcuts = nppParams.getUserShortcuts();
    for (size_t i = 0; i < shortcuts.size(); ++i) {
        if (shortcuts[i].getID() == commandId) {
            shortcuts[i].setKeyCombo(emptyCombo);
            nppParams.addUserModifiedIndex(i);
            nppParams.setShortcutDirty();
            break;
        }
    }

    emit shortcutChanged(commandId, emptyCombo);
}

void ShortcutManager::saveShortcutsToParameters()
{
    // This method ensures all current shortcuts are synchronized to NppParameters
    // and marks them for saving
    NppParameters& nppParams = NppParameters::getInstance();

    for (auto it = _commands.begin(); it != _commands.end(); ++it) {
        const CommandInfo& info = it.value();
        if (!info.isEnabled) continue;

        // Find in NppParameters and update
        std::vector<CommandShortcut>& shortcuts = nppParams.getUserShortcuts();
        for (size_t i = 0; i < shortcuts.size(); ++i) {
            if (shortcuts[i].getID() == info.commandId) {
                shortcuts[i].setKeyCombo(info.keyCombo);
                nppParams.addUserModifiedIndex(i);
                break;
            }
        }
    }

    nppParams.setShortcutDirty();
}

QList<ShortcutManager::CommandInfo> ShortcutManager::getAllCommands() const
{
    return _commands.values();
}

QList<ShortcutManager::CommandInfo> ShortcutManager::getCommandsByCategory(const QString& category) const
{
    QList<CommandInfo> result;
    for (const CommandInfo& info : _commands) {
        if (info.category == category) {
            result.append(info);
        }
    }
    return result;
}

ShortcutManager::CommandInfo ShortcutManager::getCommand(int commandId) const
{
    auto it = _commands.find(commandId);
    if (it != _commands.end()) {
        return it.value();
    }
    return CommandInfo();
}

bool ShortcutManager::isCommandRegistered(int commandId) const
{
    return _commands.contains(commandId);
}

void ShortcutManager::setCommandCallback(CommandCallback callback)
{
    _commandCallback = callback;
}

void ShortcutManager::executeCommand(int commandId)
{
    // First try to trigger the action directly
    auto it = _commands.find(commandId);
    if (it != _commands.end() && it.value().action) {
        it.value().action->trigger();
        return;
    }

    // Otherwise, use the callback
    if (_commandCallback) {
        _commandCallback(commandId);
    }
}

int ShortcutManager::getCommandId(QAction* action) const
{
    auto it = _actionToCommandId.find(action);
    if (it != _actionToCommandId.end()) {
        return it.value();
    }
    return -1;
}

Qt::Key ShortcutManager::virtualKeyToQtKey(unsigned char vk)
{
    // Map virtual key codes to Qt keys
    if (vk >= 'A' && vk <= 'Z') {
        return static_cast<Qt::Key>(Qt::Key_A + (vk - 'A'));
    }
    if (vk >= '0' && vk <= '9') {
        return static_cast<Qt::Key>(Qt::Key_0 + (vk - '0'));
    }
    if (vk >= VK_F1 && vk <= VK_F24) {
        return static_cast<Qt::Key>(Qt::Key_F1 + (vk - VK_F1));
    }
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) {
        return static_cast<Qt::Key>(Qt::Key_0 + (vk - VK_NUMPAD0));
    }

    switch (vk) {
        case VK_SPACE: return Qt::Key_Space;
        case VK_RETURN: return Qt::Key_Return;
        case VK_ESCAPE: return Qt::Key_Escape;
        case VK_TAB: return Qt::Key_Tab;
        case VK_BACK: return Qt::Key_Backspace;
        case VK_DELETE: return Qt::Key_Delete;
        case VK_INSERT: return Qt::Key_Insert;
        case VK_HOME: return Qt::Key_Home;
        case VK_END: return Qt::Key_End;
        case VK_PRIOR: return Qt::Key_PageUp;
        case VK_NEXT: return Qt::Key_PageDown;
        case VK_LEFT: return Qt::Key_Left;
        case VK_RIGHT: return Qt::Key_Right;
        case VK_UP: return Qt::Key_Up;
        case VK_DOWN: return Qt::Key_Down;
        case VK_MULTIPLY: return Qt::Key_Asterisk;
        case VK_ADD: return Qt::Key_Plus;
        case VK_SUBTRACT: return Qt::Key_Minus;
        case VK_DECIMAL: return Qt::Key_Period;
        case VK_DIVIDE: return Qt::Key_Slash;
        case VK_OEM_PLUS: return Qt::Key_Plus;
        case VK_OEM_MINUS: return Qt::Key_Minus;
        case VK_OEM_COMMA: return Qt::Key_Comma;
        case VK_OEM_PERIOD: return Qt::Key_Period;
        case VK_OEM_1: return Qt::Key_Semicolon;
        case VK_OEM_2: return Qt::Key_Slash;
        case VK_OEM_3: return Qt::Key_QuoteLeft;
        case VK_OEM_4: return Qt::Key_BracketLeft;
        case VK_OEM_5: return Qt::Key_Backslash;
        case VK_OEM_6: return Qt::Key_BracketRight;
        case VK_OEM_7: return Qt::Key_Apostrophe;
        default: return Qt::Key_unknown;
    }
}

unsigned char ShortcutManager::qtKeyToVirtualKey(Qt::Key key)
{
    // Map Qt keys to virtual key codes
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        return 'A' + (key - Qt::Key_A);
    }
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        return '0' + (key - Qt::Key_0);
    }
    if (key >= Qt::Key_F1 && key <= Qt::Key_F24) {
        return VK_F1 + (key - Qt::Key_F1);
    }

    switch (key) {
        case Qt::Key_Space: return VK_SPACE;
        case Qt::Key_Return: return VK_RETURN;
        case Qt::Key_Enter: return VK_RETURN;
        case Qt::Key_Escape: return VK_ESCAPE;
        case Qt::Key_Tab: return VK_TAB;
        case Qt::Key_Backspace: return VK_BACK;
        case Qt::Key_Delete: return VK_DELETE;
        case Qt::Key_Insert: return VK_INSERT;
        case Qt::Key_Home: return VK_HOME;
        case Qt::Key_End: return VK_END;
        case Qt::Key_PageUp: return VK_PRIOR;
        case Qt::Key_PageDown: return VK_NEXT;
        case Qt::Key_Left: return VK_LEFT;
        case Qt::Key_Right: return VK_RIGHT;
        case Qt::Key_Up: return VK_UP;
        case Qt::Key_Down: return VK_DOWN;
        case Qt::Key_Plus: return VK_ADD;
        case Qt::Key_Minus: return VK_SUBTRACT;
        case Qt::Key_Asterisk: return VK_MULTIPLY;
        case Qt::Key_Slash: return VK_DIVIDE;
        case Qt::Key_Period: return VK_DECIMAL;
        case Qt::Key_Comma: return VK_OEM_COMMA;
        case Qt::Key_Semicolon: return VK_OEM_1;
        case Qt::Key_BracketLeft: return VK_OEM_4;
        case Qt::Key_BracketRight: return VK_OEM_6;
        case Qt::Key_Backslash: return VK_OEM_5;
        case Qt::Key_Apostrophe: return VK_OEM_7;
        case Qt::Key_QuoteLeft: return VK_OEM_3;
        default: return 0;
    }
}

} // namespace QtControls
