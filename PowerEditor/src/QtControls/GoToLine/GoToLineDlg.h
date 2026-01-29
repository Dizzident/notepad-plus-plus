// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "ScintillaEditView.h"

// Forward declarations
class QSpinBox;
class QLabel;
class QRadioButton;
class QPushButton;

namespace QtControls {

// ============================================================================
// GoToLineDlg - Qt implementation of Go To Line dialog
// ============================================================================
class GoToLineDlg : public StaticDialog {
    Q_OBJECT

public:
    GoToLineDlg(QWidget* parent = nullptr);
    ~GoToLineDlg() override;

    // Initialize the dialog (matches Windows interface)
    void init(HINSTANCE hInst, HWND hPere, ScintillaEditView **ppEditView);

    // Show the dialog (matches Windows interface)
    void doDialog(bool isRTL = false);

    // Display the dialog (Qt-specific interface)
    void display(bool toShow = true, bool enhancedPositioningCheckWhenShowing = false);

    // Update line numbers display (matches Windows interface)
    void updateLinesNumbers() const;

    // Get the target line number (1-based)
    long long getLine() const;

protected:
    void setupUI() override;
    void connectSignals() override;
    bool run_dlgProc(QEvent* event) override;

private slots:
    void onGoClicked();
    void onCancelClicked();
    void onModeChanged();
    void updateRangeLabel();

private:
    QSpinBox* _lineSpinBox = nullptr;
    QLabel* _rangeLabel = nullptr;
    QRadioButton* _lineModeRadio = nullptr;
    QRadioButton* _offsetModeRadio = nullptr;
    QPushButton* _goButton = nullptr;
    QPushButton* _cancelButton = nullptr;

    // Mode enum matching Windows version
    enum mode {go2line, go2offset};
    mode _mode = go2line;

    // Store the edit view pointer (matches Windows interface)
    ScintillaEditView **_ppEditView = nullptr;

    // Cached values for display
    mutable int _currentLine = 0;
    mutable int _totalLines = 0;
    mutable int _currentPos = 0;
};

} // namespace QtControls
