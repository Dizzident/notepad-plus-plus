// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtGui/QColor>
#include <vector>

// Forward declarations
class QEvent;
class ScintillaEditView;

namespace QtControls {

// ============================================================================
// AsciiCharItem - Structure to store ASCII character information
// ============================================================================
struct AsciiCharItem {
    int value = 0;
    QString hex;
    QString character;
    QString htmlName;
    QString htmlNumber;
    QString htmlHexNumber;

    AsciiCharItem() = default;
    AsciiCharItem(int val, const QString& hexStr, const QString& charStr,
                  const QString& htmlNameStr, const QString& htmlNumStr,
                  const QString& htmlHexStr)
        : value(val), hex(hexStr), character(charStr),
          htmlName(htmlNameStr), htmlNumber(htmlNumStr), htmlHexNumber(htmlHexStr) {}
};

// ============================================================================
// AnsiCharPanel - Dockable panel showing ASCII characters and their codes
// ============================================================================
class AnsiCharPanel : public StaticDialog {
    Q_OBJECT

public:
    explicit AnsiCharPanel(QWidget* parent = nullptr);
    ~AnsiCharPanel() override;

    void init(ScintillaEditView** ppEditView);
    void doDialog();

    using StaticDialog::getDialog;
    QWidget* getWidget() const { return getDialog(); }

    void switchEncoding();
    void setBackgroundColor(const QColor& bgColour);
    void setForegroundColor(const QColor& fgColour);

public slots:
    void onItemDoubleClicked(int row, int column);
    void onItemClicked(int row, int column);
    void onFilterChanged(const QString& text);
    void onHeaderClicked(int logicalIndex);
    void onItemActivated(int row, int column);

protected:
    void setupUI();
    void connectSignals();
    bool run_dlgProc(QEvent* event) override;

private:
    void populateTable(int codepage = 0);
    void clearTable();
    void insertChar(unsigned char char2insert) const;
    void insertString(const QString& string2insert) const;
    void filterItems(const QString& filter);

    QString getAsciiName(unsigned char value) const;
    QString getHtmlName(unsigned char value) const;
    int getHtmlNumber(unsigned char value) const;

    QTableWidget* _tableWidget = nullptr;
    QLineEdit* _filterEdit = nullptr;
    QLabel* _statusLabel = nullptr;
    QVBoxLayout* _mainLayout = nullptr;
    QHBoxLayout* _filterLayout = nullptr;

    std::vector<AsciiCharItem> _charItems;
    std::vector<AsciiCharItem> _filteredItems;
    int _codepage = 0;
    int _currentSortColumn = 0;
    Qt::SortOrder _currentSortOrder = Qt::AscendingOrder;

    ScintillaEditView** _ppEditView = nullptr;

    static constexpr int COLUMN_VALUE = 0;
    static constexpr int COLUMN_HEX = 1;
    static constexpr int COLUMN_CHAR = 2;
    static constexpr int COLUMN_HTML_NAME = 3;
    static constexpr int COLUMN_HTML_NUMBER = 4;
    static constexpr int COLUMN_HTML_HEX = 5;
    static constexpr int NUM_COLUMNS = 6;
};

} // namespace QtControls
