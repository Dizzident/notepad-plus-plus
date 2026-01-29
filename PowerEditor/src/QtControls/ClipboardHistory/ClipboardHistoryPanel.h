// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../ListView/ListView.h"

#include <QDateTime>
#include <QString>
#include <vector>

// Forward declarations
class QLineEdit;
class QTextEdit;
class QPushButton;
class QClipboard;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;

class ScintillaEditView;

namespace QtControls {

// ============================================================================
// ClipboardItem - Structure to store clipboard history entry
// ============================================================================
struct ClipboardItem {
    QString text;
    QDateTime timestamp;
    QString displayText; // Truncated for display

    ClipboardItem() = default;
    explicit ClipboardItem(const QString& t)
        : text(t), timestamp(QDateTime::currentDateTime()) {
        updateDisplayText();
    }

    void updateDisplayText(size_t maxLength = 64) {
        if (text.length() <= static_cast<int>(maxLength)) {
            displayText = text;
        } else {
            displayText = text.left(static_cast<int>(maxLength) - 3) + "...";
        }
        // Replace newlines with visible representation for display
        displayText.replace("\n", "\xE2\x8F\x8E"); // Return symbol
        displayText.replace("\r", "");
        displayText.replace("\t", "\xE2\x86\xB9"); // Tab symbol
    }

    bool operator==(const ClipboardItem& other) const {
        return text == other.text;
    }
};

// ============================================================================
// ClipboardHistoryPanel - Dockable panel showing clipboard history
// ============================================================================
class ClipboardHistoryPanel : public StaticDialog {
    Q_OBJECT

public:
    explicit ClipboardHistoryPanel(QWidget* parent = nullptr);
    ~ClipboardHistoryPanel() override;

    // Initialize the panel
    void init(ScintillaEditView** ppEditView);

    // Show the panel
    void doDialog();

    // Bring getDialog into public scope for getWidget
    using StaticDialog::getDialog;

    // Get widget for docking integration
    QWidget* getWidget() const { return getDialog(); }

    // Add text to history
    void addToHistory(const QString& text);

    // Clear all history
    void clearHistory();

    // Get/set max history size
    int getMaxHistorySize() const { return _maxHistorySize; }
    void setMaxHistorySize(int size);

    // Enable/disable clipboard tracking
    bool trackClipboardOps(bool track) {
        bool previousState = _isTrackingClipboardOps;
        _isTrackingClipboardOps = track;
        return previousState;
    }

    // Check if tracking is enabled
    bool isTrackingClipboardOps() const { return _isTrackingClipboardOps; }

public slots:
    void onItemClicked(int index);
    void onItemDoubleClicked(int index);
    void onDeleteItem();
    void onClearAll();
    void onFilterChanged(const QString& text);
    void onClipboardChanged();
    void onSelectionChanged();
    void onPasteClicked();

protected:
    void setupUI();
    void connectSignals();
    bool run_dlgProc(QEvent* event) override;

private:
    void updateList();
    void updatePreview();
    void pasteItem(int index);
    void deleteItem(int index);
    int findItemIndex(const QString& text) const;
    void trimHistory();
    void loadHistory();
    void saveHistory();

    QString getClipboardText() const;

    // UI Components
    ListView* _listView = nullptr;
    QLineEdit* _filterEdit = nullptr;
    QTextEdit* _preview = nullptr;
    QPushButton* _pasteBtn = nullptr;
    QPushButton* _deleteBtn = nullptr;
    QPushButton* _clearBtn = nullptr;
    QLabel* _statusLabel = nullptr;

    // Layouts
    QVBoxLayout* _mainLayout = nullptr;
    QHBoxLayout* _buttonLayout = nullptr;

    // Data
    std::vector<ClipboardItem> _history;
    std::vector<ClipboardItem> _filteredHistory;
    int _maxHistorySize = 20;
    int _currentIndex = -1;

    // Scintilla edit view pointer
    ScintillaEditView** _ppEditView = nullptr;

    // Clipboard monitoring
    QClipboard* _clipboard = nullptr;
    bool _isTrackingClipboardOps = true;
    bool _isInternalClipboardChange = false;

    // Constants
    static constexpr int MAX_DISPLAY_LENGTH = 64;
    static constexpr int MAX_PREVIEW_LENGTH = 1000;
};

} // namespace QtControls
