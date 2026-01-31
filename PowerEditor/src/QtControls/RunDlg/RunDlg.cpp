// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "RunDlg.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QProcess>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

namespace QtControls {

namespace RunDlg {

RunDlg::RunDlg(QWidget* parent)
    : StaticDialog(parent) {
}

RunDlg::~RunDlg() = default;

void RunDlg::doDialog() {
    doDialog(false);
}

void RunDlg::doDialog(bool /*isRTL*/) {
    if (!isCreated()) {
        create(tr("Run"), false);
        setupUI();
        connectSignals();
    }

    display(true, true);
    goToCenter();

    // Focus the command input
    if (_commandEdit) {
        _commandEdit->setFocus();
        _commandEdit->selectAll();
    }
}

void RunDlg::setupUI() {
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setWindowTitle(tr("Run..."));
    dialog->resize(450, 350);

    auto* mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // Program to run label and combo
    auto* programLayout = new QHBoxLayout();
    _programLabel = new QLabel(tr("The Program to Run:"), dialog);
    programLayout->addWidget(_programLabel);
    programLayout->addStretch();
    mainLayout->addLayout(programLayout);

    // Command input with browse button
    auto* commandLayout = new QHBoxLayout();
    _commandCombo = new QComboBox(dialog);
    _commandCombo->setEditable(true);
    _commandCombo->setMaxCount(20);
    _commandEdit = _commandCombo->lineEdit();
    _commandEdit->setPlaceholderText(tr("Enter command or select from history..."));
    _commandEdit->setText(_currentCommand);
    commandLayout->addWidget(_commandCombo, 1);

    _browseButton = new QPushButton(tr("..."), dialog);
    _browseButton->setToolTip(tr("Browse for executable"));
    _browseButton->setFixedWidth(32);
    commandLayout->addWidget(_browseButton);

    mainLayout->addLayout(commandLayout);

    // Help text area with available variables
    auto* helpLabel = new QLabel(tr("Available variables:"), dialog);
    mainLayout->addWidget(helpLabel);

    _helpText = new QTextEdit(dialog);
    _helpText->setReadOnly(true);
    _helpText->setMaximumHeight(140);
    _helpText->setPlainText(
        tr("$(FULL_CURRENT_PATH)  : The full path to the current document\n"
           "$(CURRENT_DIRECTORY)  : The directory of the current document\n"
           "$(FILE_NAME)          : The filename of the current document\n"
           "$(NAME_PART)          : The filename without extension\n"
           "$(EXT_PART)           : The extension of the current document\n"
           "$(CURRENT_WORD)       : The current selected text\n"
           "$(CURRENT_LINE)       : The current line number\n"
           "$(CURRENT_COLUMN)     : The current column number\n"
           "$(NPP_DIRECTORY)      : The directory of Notepad++ executable")
    );
    mainLayout->addWidget(_helpText);

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _saveButton = new QPushButton(tr("Save..."), dialog);
    _saveButton->setToolTip(tr("Save this command to the Run menu"));
    buttonLayout->addWidget(_saveButton);

    buttonLayout->addSpacing(20);

    _runButton = new QPushButton(tr("Run"), dialog);
    _runButton->setDefault(true);
    buttonLayout->addWidget(_runButton);

    _cancelButton = new QPushButton(tr("Cancel"), dialog);
    buttonLayout->addWidget(_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Store initial rect
    _rc = dialog->geometry();
}

void RunDlg::connectSignals() {
    // Button clicks
    connect(_runButton, &QPushButton::clicked, this, &RunDlg::onRunClicked);
    connect(_cancelButton, &QPushButton::clicked, this, &RunDlg::onCancelClicked);
    connect(_browseButton, &QPushButton::clicked, this, &RunDlg::onBrowseClicked);
    connect(_saveButton, &QPushButton::clicked, this, &RunDlg::onSaveClicked);

    // Command text changes
    connect(_commandEdit, &QLineEdit::textChanged, this, &RunDlg::onCommandChanged);

    // Enable/disable run button based on command text
    onCommandChanged(_commandEdit->text());
}

QString RunDlg::getCommand() const {
    if (_commandEdit) {
        return _commandEdit->text().trimmed();
    }
    return _currentCommand;
}

void RunDlg::setCommand(const QString& command) {
    _currentCommand = command;
    if (_commandEdit) {
        _commandEdit->setText(command);
    }
}

void RunDlg::setHistory(const std::vector<QString>& history) {
    _commandHistory = history;
    updateComboHistory();
}

std::vector<QString> RunDlg::getHistory() const {
    return _commandHistory;
}

void RunDlg::addCommandToHistory(const QString& command) {
    if (command.isEmpty()) return;

    // Remove if already exists
    auto it = std::find(_commandHistory.begin(), _commandHistory.end(), command);
    if (it != _commandHistory.end()) {
        _commandHistory.erase(it);
    }

    // Add to front
    _commandHistory.insert(_commandHistory.begin(), command);

    // Limit size
    if (_commandHistory.size() > 20) {
        _commandHistory.resize(20);
    }

    updateComboHistory();
}

void RunDlg::updateComboHistory() {
    if (!_commandCombo) return;

    QString currentText = _commandCombo->currentText();
    _commandCombo->clear();

    for (const auto& item : _commandHistory) {
        _commandCombo->addItem(item);
    }

    _commandCombo->setCurrentText(currentText);
}

QString RunDlg::expandVariables(const QString& command) {
    // TODO: Implement variable expansion
    // This should expand:
    // $(FULL_CURRENT_PATH), $(CURRENT_DIRECTORY), $(FILE_NAME)
    // $(NAME_PART), $(EXT_PART), $(CURRENT_WORD)
    // $(CURRENT_LINE), $(CURRENT_COLUMN), $(NPP_DIRECTORY)
    //
    // For now, return the command as-is
    // The actual implementation should query the editor for these values
    return command;
}

void RunDlg::executeCommand() {
    QString command = getCommand();
    if (command.isEmpty()) {
        QMessageBox::warning(getDialog(), tr("Error"), tr("Please enter a command to run."));
        return;
    }

    // Expand variables in the command
    QString expandedCommand = expandVariables(command);

    // Parse command and arguments
    QString program;
    QStringList arguments;

    // Handle quoted paths
    if (expandedCommand.startsWith('"')) {
        int endQuote = expandedCommand.indexOf('"', 1);
        if (endQuote > 0) {
            program = expandedCommand.mid(1, endQuote - 1);
            QString args = expandedCommand.mid(endQuote + 1).trimmed();
            if (!args.isEmpty()) {
                arguments = args.split(' ', Qt::SkipEmptyParts);
            }
        } else {
            program = expandedCommand;
        }
    } else {
        int spaceIdx = expandedCommand.indexOf(' ');
        if (spaceIdx > 0) {
            program = expandedCommand.left(spaceIdx);
            QString args = expandedCommand.mid(spaceIdx + 1).trimmed();
            arguments = args.split(' ', Qt::SkipEmptyParts);
        } else {
            program = expandedCommand;
        }
    }

    // Execute the command
    bool started = QProcess::startDetached(program, arguments);

    if (started) {
        // Add to history on successful execution
        addCommandToHistory(command);
        display(false);
    } else {
        QMessageBox::critical(getDialog(), tr("Error"),
            tr("Failed to execute command:\n%1").arg(expandedCommand));
    }
}

void RunDlg::onRunClicked() {
    executeCommand();
}

void RunDlg::onCancelClicked() {
    display(false);
}

void RunDlg::onBrowseClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        getDialog(),
        tr("Select Executable"),
        QString(),
        tr("Executable Files (*);;All Files (*)")
    );

    if (!fileName.isEmpty()) {
        // Quote the path if it contains spaces
        if (fileName.contains(' ')) {
            fileName = QString("\"%1\"").arg(fileName);
        }

        if (_commandEdit) {
            _commandEdit->setText(fileName);
        }
    }
}

void RunDlg::onSaveClicked() {
    QString command = getCommand();
    if (command.isEmpty()) {
        QMessageBox::warning(getDialog(), tr("Error"), tr("Please enter a command to save."));
        return;
    }

    // TODO: Implement saving command to Run menu
    // This should open a dialog to save the command with a name
    // and add it to the Run menu for quick access
    QMessageBox::information(getDialog(), tr("Not Implemented"),
        tr("Save command to Run menu is not yet implemented."));
}

void RunDlg::onCommandChanged(const QString& text) {
    // Enable/disable run button based on whether there's text
    bool hasText = !text.trimmed().isEmpty();
    if (_runButton) {
        _runButton->setEnabled(hasText);
    }
}

bool RunDlg::run_dlgProc(QEvent* /*event*/) {
    // Handle any custom event processing here
    // Return true if event was handled, false otherwise
    return false;
}

} // namespace RunDlg

} // namespace QtControls
