// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"

// Forward declarations
class QComboBox;
class QLineEdit;
class QRadioButton;
class QPushButton;
class QLabel;

namespace QtControls {

// ============================================================================
// RunMacroDlg - Qt implementation for running macros multiple times
// ============================================================================
class RunMacroDlg : public StaticDialog {
    Q_OBJECT

public:
    explicit RunMacroDlg(QWidget* parent = nullptr);
    ~RunMacroDlg() override = default;

    // Show the dialog
    void doDialog(bool isRTL = false);

    // Initialize the macro list
    void initMacroList();

    // Getters for dialog state
    int isMulti() const;
    int getTimes() const { return _times; }
    int getMacro2Exec() const;

protected:
    void setupUI() override;
    void connectSignals() override;
    bool run_dlgProc(QEvent* event) override;

private slots:
    void onOKClicked();
    void onCancelClicked();
    void onRunMultiToggled(bool checked);
    void onRunEOFToggled(bool checked);
    void onTimesChanged(const QString& text);
    void onMacroSelectionChanged(int index);

private:
    // UI Components
    QLabel* _macroLabel = nullptr;
    QComboBox* _macroCombo = nullptr;

    QRadioButton* _runMultiRadio = nullptr;
    QRadioButton* _runEOFRadio = nullptr;

    QLabel* _timesLabel = nullptr;
    QLineEdit* _timesEdit = nullptr;

    QPushButton* _okButton = nullptr;
    QPushButton* _cancelButton = nullptr;

    // State
    int _times = 1;
    int _macroIndex = 0;
    bool _runUntilEOF = false;
};

} // namespace QtControls
