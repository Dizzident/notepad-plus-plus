// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include <vector>

// Forward declarations
class QComboBox;
class QLineEdit;
class QTextEdit;
class QPushButton;
class QLabel;

namespace QtControls {

namespace RunDlg {

// ============================================================================
// RunDlg - Qt implementation for running external programs
// ============================================================================
class RunDlg : public StaticDialog {
    Q_OBJECT

public:
    explicit RunDlg(QWidget* parent = nullptr);
    ~RunDlg() override;

    // Show the dialog
    void doDialog();
    void doDialog(bool isRTL);

    // Initialize macro list (stub for compatibility)
    void initMacroList() {}

    // Get the current command
    QString getCommand() const;

    // Set the current command
    void setCommand(const QString& command);

    // Set the command history
    void setHistory(const std::vector<QString>& history);

    // Get the command history
    std::vector<QString> getHistory() const;

protected:
    void setupUI() override;
    void connectSignals() override;
    bool run_dlgProc(QEvent* event) override;

private slots:
    void onRunClicked();
    void onCancelClicked();
    void onBrowseClicked();
    void onSaveClicked();
    void onCommandChanged(const QString& text);

private:
    // UI Components
    QLabel* _programLabel = nullptr;
    QComboBox* _commandCombo = nullptr;
    QLineEdit* _commandEdit = nullptr;
    QTextEdit* _helpText = nullptr;
    QPushButton* _browseButton = nullptr;
    QPushButton* _saveButton = nullptr;
    QPushButton* _runButton = nullptr;
    QPushButton* _cancelButton = nullptr;

    // State
    std::vector<QString> _commandHistory;
    QString _currentCommand;

    // Helper methods
    void addCommandToHistory(const QString& command);
    void updateComboHistory();
    QString expandVariables(const QString& command);
    void executeCommand();
};

} // namespace RunDlg

} // namespace QtControls
