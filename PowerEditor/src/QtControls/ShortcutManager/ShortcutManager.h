// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QObject>
#include <QKeySequence>
#include <QShortcut>
#include <QMap>
#include <QHash>
#include <functional>

#include "../Shortcut/Shortcut.h"

// Forward declarations
class QAction;
class QWidget;
class QMenu;

namespace QtControls {

// ============================================================================
// ShortcutManager - Global shortcut handling for Qt6
//
// This class manages keyboard shortcuts for the Notepad++ Qt6 port.
// It loads shortcuts from NppParameters and applies them to QActions.
// It also handles dynamic shortcut updates via the ShortcutMapper.
// ============================================================================
class ShortcutManager : public QObject {
    Q_OBJECT

public:
    // Callback type for command execution
    using CommandCallback = std::function<void(int)>;

    // Structure to hold command information
    struct CommandInfo {
        int commandId = 0;
        QString name;
        QString category;
        KeyCombo keyCombo;
        bool isEnabled = false;
        QAction* action = nullptr;
    };

    explicit ShortcutManager(QObject* parent = nullptr);
    ~ShortcutManager() override;

    // Singleton access
    static ShortcutManager* getInstance();

    // Initialize shortcuts from NppParameters
    void initialize();

    // Register a QAction with a command ID
    void registerAction(int commandId, QAction* action, const QString& category = QString());

    // Register multiple actions from a menu recursively
    void registerMenuActions(QMenu* menu, const QString& category = QString());

    // Unregister an action
    void unregisterAction(int commandId);
    void unregisterAction(QAction* action);

    // Apply shortcuts from NppParameters to registered actions
    void applyShortcuts();

    // Apply a specific shortcut to an action
    void applyShortcut(int commandId, const KeyCombo& combo);

    // Get the current shortcut for a command
    KeyCombo getShortcut(int commandId) const;

    // Get the display string for a shortcut
    static QString keyComboToString(const KeyCombo& combo);
    static QKeySequence keyComboToQKeySequence(const KeyCombo& combo);

    // Convert QKeySequence to KeyCombo
    static KeyCombo qKeySequenceToKeyCombo(const QKeySequence& seq);

    // Check if a key combo is valid
    static bool isValidKeyCombo(const KeyCombo& combo);

    // Update shortcuts from NppParameters (call after ShortcutMapper changes)
    void updateShortcutsFromParameters();

    // Update a specific command shortcut (used by ShortcutMapper)
    void updateCommandShortcut(int commandId, const KeyCombo& newCombo);

    // Clear a command shortcut (used by ShortcutMapper)
    void clearCommandShortcut(int commandId);

    // Save current shortcuts to NppParameters (call after ShortcutMapper modifications)
    void saveShortcutsToParameters();

    // Get all registered commands
    QList<CommandInfo> getAllCommands() const;

    // Get commands by category
    QList<CommandInfo> getCommandsByCategory(const QString& category) const;

    // Find command by ID
    CommandInfo getCommand(int commandId) const;

    // Check if a command is registered
    bool isCommandRegistered(int commandId) const;

    // Set command callback (for handling command execution)
    void setCommandCallback(CommandCallback callback);

    // Execute a command by ID
    void executeCommand(int commandId);

    // Get command ID from action
    int getCommandId(QAction* action) const;

signals:
    // Emitted when a shortcut is changed
    void shortcutChanged(int commandId, const KeyCombo& newCombo);

    // Emitted when shortcuts are reloaded
    void shortcutsReloaded();

private:
    // Map command IDs to command info
    QHash<int, CommandInfo> _commands;

    // Map actions to command IDs (for reverse lookup)
    QHash<QAction*, int> _actionToCommandId;

    // Command callback
    CommandCallback _commandCallback;

    // Static instance for singleton pattern
    static ShortcutManager* _instance;

    // Helper to create QKeySequence from KeyCombo
    static Qt::Key virtualKeyToQtKey(unsigned char vk);
    static unsigned char qtKeyToVirtualKey(Qt::Key key);

    // Load default shortcuts
    void loadDefaultShortcuts();
};

} // namespace QtControls
