// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "AnsiCharPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTextCodec>

#include "../../ScintillaComponent/ScintillaEditView.h"
#include "../../ScintillaComponent/Buffer.h"

namespace QtControls {

// ============================================================================
// Constructor/Destructor
// ============================================================================

AnsiCharPanel::AnsiCharPanel(QWidget* parent)
    : StaticDialog(parent)
{
}

AnsiCharPanel::~AnsiCharPanel()
{
    destroy();
}

// ============================================================================
// Initialization
// ============================================================================

void AnsiCharPanel::init(ScintillaEditView** ppEditView)
{
    _ppEditView = ppEditView;

    // Create the dialog widget
    create(tr("ASCII Codes Insertion Panel"), false);

    // Setup UI
    setupUI();
    connectSignals();

    // Populate table with default encoding
    populateTable(0);
}

void AnsiCharPanel::doDialog()
{
    display(true);
}

// ============================================================================
// UI Setup
// ============================================================================

void AnsiCharPanel::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setMinimumSize(450, 400);

    // Main layout
    _mainLayout = new QVBoxLayout(dialog);
    _mainLayout->setSpacing(6);
    _mainLayout->setContentsMargins(8, 8, 8, 8);

    // Filter/search box
    _filterLayout = new QHBoxLayout();
    QLabel* filterLabel = new QLabel(tr("Filter:"), dialog);
    _filterEdit = new QLineEdit(dialog);
    _filterEdit->setPlaceholderText(tr("Search characters..."));
    _filterLayout->addWidget(filterLabel);
    _filterLayout->addWidget(_filterEdit);
    _mainLayout->addLayout(_filterLayout);

    // Table widget for ASCII characters
    _tableWidget = new QTableWidget(dialog);
    _tableWidget->setColumnCount(NUM_COLUMNS);
    _tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    _tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    _tableWidget->setAlternatingRowColors(true);
    _tableWidget->setSortingEnabled(true);

    // Set column headers
    QStringList headers;
    headers << tr("Value") << tr("Hex") << tr("Character")
            << tr("HTML Name") << tr("HTML Decimal") << tr("HTML Hexadecimal");
    _tableWidget->setHorizontalHeaderLabels(headers);

    // Configure header
    QHeaderView* header = _tableWidget->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setDefaultSectionSize(70);
    header->setSectionResizeMode(COLUMN_VALUE, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(COLUMN_HEX, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(COLUMN_CHAR, QHeaderView::ResizeToContents);

    // Hide vertical header (row numbers)
    _tableWidget->verticalHeader()->setVisible(false);

    // Set table to expand
    _tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _mainLayout->addWidget(_tableWidget, 1);

    // Status label
    _statusLabel = new QLabel(dialog);
    _statusLabel->setText(tr("Characters: 256"));
    _mainLayout->addWidget(_statusLabel);

    dialog->setLayout(_mainLayout);
}

void AnsiCharPanel::connectSignals()
{
    if (!_tableWidget) return;

    // Table signals
    connect(_tableWidget, &QTableWidget::cellDoubleClicked,
            this, &AnsiCharPanel::onItemDoubleClicked);
    connect(_tableWidget, &QTableWidget::cellClicked,
            this, &AnsiCharPanel::onItemClicked);
    connect(_tableWidget, &QTableWidget::cellActivated,
            this, &AnsiCharPanel::onItemActivated);
    connect(_tableWidget->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &AnsiCharPanel::onHeaderClicked);

    // Filter signal
    if (_filterEdit) {
        connect(_filterEdit, &QLineEdit::textChanged,
                this, &AnsiCharPanel::onFilterChanged);
    }
}

// ============================================================================
// Event Handling
// ============================================================================

bool AnsiCharPanel::run_dlgProc(QEvent* event)
{
    (void)event;
    return false;
}

// ============================================================================
// Color Handling (Dark Mode Support)
// ============================================================================

void AnsiCharPanel::setBackgroundColor(const QColor& bgColour)
{
    if (_tableWidget) {
        QPalette palette = _tableWidget->palette();
        palette.setColor(QPalette::Base, bgColour);
        palette.setColor(QPalette::AlternateBase, bgColour.darker(110));
        _tableWidget->setPalette(palette);
        _tableWidget->setStyleSheet(QString("QTableWidget { background-color: %1; }")
                                    .arg(bgColour.name()));
    }
}

void AnsiCharPanel::setForegroundColor(const QColor& fgColour)
{
    if (_tableWidget) {
        QPalette palette = _tableWidget->palette();
        palette.setColor(QPalette::Text, fgColour);
        _tableWidget->setPalette(palette);
    }
}

// ============================================================================
// Encoding Switch
// ============================================================================

void AnsiCharPanel::switchEncoding()
{
    if (!_ppEditView || !*_ppEditView) return;

    Buffer* buffer = (*_ppEditView)->getCurrentBuffer();
    if (!buffer) return;

    int codepage = buffer->getEncoding();
    if (codepage == -1) {
        codepage = 0;
    }

    if (_codepage != codepage) {
        populateTable(codepage);
    }
}

// ============================================================================
// Table Population
// ============================================================================

void AnsiCharPanel::populateTable(int codepage)
{
    _codepage = codepage;
    clearTable();
    _charItems.clear();
    _charItems.reserve(256);

    for (int i = 0; i < 256; ++i) {
        unsigned char value = static_cast<unsigned char>(i);

        // Decimal value
        QString dec = QString::number(i);

        // Hex value
        QString hex = QString("%1").arg(i, 2, 16, QChar('0')).toUpper();

        // Character representation
        QString charStr = getAsciiName(value);

        // HTML entities (only for codepage 0 or 1252)
        QString htmlName;
        QString htmlNumber;
        QString htmlHexNumber;

        if (codepage == 0 || codepage == 1252) {
            if ((i >= 32 && i <= 126 && i != 45) || (i >= 160 && i <= 255)) {
                htmlNumber = QString("&#%1;").arg(i);
                htmlHexNumber = QString("&#x%1;").arg(i, 0, 16);
            } else {
                int n = getHtmlNumber(value);
                if (n > -1) {
                    htmlNumber = QString("&#%1;").arg(n);
                    htmlHexNumber = QString("&#x%1;").arg(n, 0, 16);
                }
            }
            htmlName = getHtmlName(value);
        }

        _charItems.emplace_back(i, hex, charStr, htmlName, htmlNumber, htmlHexNumber);
    }

    // Apply filter if any
    if (_filterEdit && !_filterEdit->text().isEmpty()) {
        filterItems(_filterEdit->text());
    } else {
        _filteredItems = _charItems;
    }

    // Populate table
    _tableWidget->setRowCount(static_cast<int>(_filteredItems.size()));

    for (size_t row = 0; row < _filteredItems.size(); ++row) {
        const auto& item = _filteredItems[row];

        // Value column
        QTableWidgetItem* valueItem = new QTableWidgetItem(QString::number(item.value));
        valueItem->setData(Qt::UserRole, item.value);
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_VALUE, valueItem);

        // Hex column
        QTableWidgetItem* hexItem = new QTableWidgetItem(item.hex);
        hexItem->setFlags(hexItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_HEX, hexItem);

        // Character column
        QTableWidgetItem* charItem = new QTableWidgetItem(item.character);
        charItem->setFlags(charItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_CHAR, charItem);

        // HTML Name column
        QTableWidgetItem* htmlNameItem = new QTableWidgetItem(item.htmlName);
        htmlNameItem->setFlags(htmlNameItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_HTML_NAME, htmlNameItem);

        // HTML Number column
        QTableWidgetItem* htmlNumItem = new QTableWidgetItem(item.htmlNumber);
        htmlNumItem->setFlags(htmlNumItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_HTML_NUMBER, htmlNumItem);

        // HTML Hex column
        QTableWidgetItem* htmlHexItem = new QTableWidgetItem(item.htmlHexNumber);
        htmlHexItem->setFlags(htmlHexItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_HTML_HEX, htmlHexItem);
    }

    // Update status
    if (_statusLabel) {
        _statusLabel->setText(tr("Characters: %1").arg(_filteredItems.size()));
    }
}

void AnsiCharPanel::clearTable()
{
    if (_tableWidget) {
        _tableWidget->clearContents();
        _tableWidget->setRowCount(0);
    }
}

// ============================================================================
// Filtering
// ============================================================================

void AnsiCharPanel::onFilterChanged(const QString& text)
{
    filterItems(text);
}

void AnsiCharPanel::filterItems(const QString& filter)
{
    if (filter.isEmpty()) {
        _filteredItems = _charItems;
    } else {
        _filteredItems.clear();
        QString lowerFilter = filter.toLower();

        for (const auto& item : _charItems) {
            if (item.character.toLower().contains(lowerFilter) ||
                item.hex.toLower().contains(lowerFilter) ||
                QString::number(item.value).contains(filter) ||
                item.htmlName.toLower().contains(lowerFilter) ||
                item.htmlNumber.contains(filter) ||
                item.htmlHexNumber.toLower().contains(lowerFilter)) {
                _filteredItems.push_back(item);
            }
        }
    }

    // Repopulate table with filtered items
    _tableWidget->setRowCount(static_cast<int>(_filteredItems.size()));

    for (size_t row = 0; row < _filteredItems.size(); ++row) {
        const auto& item = _filteredItems[row];

        QTableWidgetItem* valueItem = new QTableWidgetItem(QString::number(item.value));
        valueItem->setData(Qt::UserRole, item.value);
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_VALUE, valueItem);

        QTableWidgetItem* hexItem = new QTableWidgetItem(item.hex);
        hexItem->setFlags(hexItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_HEX, hexItem);

        QTableWidgetItem* charItem = new QTableWidgetItem(item.character);
        charItem->setFlags(charItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_CHAR, charItem);

        QTableWidgetItem* htmlNameItem = new QTableWidgetItem(item.htmlName);
        htmlNameItem->setFlags(htmlNameItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_HTML_NAME, htmlNameItem);

        QTableWidgetItem* htmlNumItem = new QTableWidgetItem(item.htmlNumber);
        htmlNumItem->setFlags(htmlNumItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_HTML_NUMBER, htmlNumItem);

        QTableWidgetItem* htmlHexItem = new QTableWidgetItem(item.htmlHexNumber);
        htmlHexItem->setFlags(htmlHexItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(static_cast<int>(row), COLUMN_HTML_HEX, htmlHexItem);
    }

    // Update status
    if (_statusLabel) {
        int total = static_cast<int>(_charItems.size());
        int shown = static_cast<int>(_filteredItems.size());
        if (shown != total) {
            _statusLabel->setText(tr("Characters: %1 (filtered from %2)").arg(shown).arg(total));
        } else {
            _statusLabel->setText(tr("Characters: %1").arg(total));
        }
    }
}

void AnsiCharPanel::onHeaderClicked(int logicalIndex)
{
    (void)logicalIndex;
    // Sorting is handled automatically by QTableWidget
}

// ============================================================================
// Item Actions
// ============================================================================

void AnsiCharPanel::onItemClicked(int row, int column)
{
    (void)row;
    (void)column;
    // Single click just selects the item
}

void AnsiCharPanel::onItemActivated(int row, int column)
{
    // Activated is triggered by Enter key or double-click
    onItemDoubleClicked(row, column);
}

void AnsiCharPanel::onItemDoubleClicked(int row, int column)
{
    if (row < 0 || row >= static_cast<int>(_filteredItems.size())) return;

    const AsciiCharItem& item = _filteredItems[static_cast<size_t>(row)];

    if (column == COLUMN_CHAR) {
        // Insert the actual character
        insertChar(static_cast<unsigned char>(item.value));
    } else {
        // Insert the text from the clicked column
        QTableWidgetItem* tableItem = _tableWidget->item(row, column);
        if (tableItem) {
            insertString(tableItem->text());
        }
    }
}

void AnsiCharPanel::insertChar(unsigned char char2insert) const
{
    if (!_ppEditView || !*_ppEditView) return;

    ScintillaEditView* editView = *_ppEditView;

    // Convert character to string based on encoding
    char charStr[2] = { static_cast<char>(char2insert), '\0' };
    QString textToInsert;

    Buffer* buffer = editView->getCurrentBuffer();
    int codepage = buffer ? buffer->getEncoding() : -1;

    if (codepage == -1) {
        // Check if document is UTF-8
        bool isUnicode = (editView->execute(SCI_GETCODEPAGE) == SC_CP_UTF8);
        if (isUnicode) {
            // Convert from Latin-1 to UTF-8
            QByteArray latin1Bytes;
            latin1Bytes.append(static_cast<char>(char2insert));
            textToInsert = QString::fromLatin1(latin1Bytes);
        } else {
            // ANSI - use as-is
            textToInsert = QString(QChar(static_cast<char>(char2insert)));
        }
    } else {
        // Use buffer's encoding
        QTextCodec* codec = QTextCodec::codecForMib(codepage);
        if (codec) {
            QByteArray encoded;
            encoded.append(static_cast<char>(char2insert));
            textToInsert = codec->toUnicode(encoded);
        } else {
            textToInsert = QString(QChar(static_cast<char>(char2insert)));
        }
    }

    // Insert the text
    std::string utf8Text = textToInsert.toUtf8().constData();
    editView->replaceSelWith(utf8Text.c_str());
    editView->grabFocus();
}

void AnsiCharPanel::insertString(const QString& string2insert) const
{
    if (!_ppEditView || !*_ppEditView || string2insert.isEmpty()) return;

    ScintillaEditView* editView = *_ppEditView;

    // Convert to UTF-8 for Scintilla
    std::string utf8Text = string2insert.toUtf8().constData();

    editView->replaceSelWith(utf8Text.c_str());
    editView->grabFocus();
}

// ============================================================================
// ASCII Name Helpers
// ============================================================================

QString AnsiCharPanel::getAsciiName(unsigned char value) const
{
    switch (value) {
        case 0: return "NULL";
        case 1: return "SOH";
        case 2: return "STX";
        case 3: return "ETX";
        case 4: return "EOT";
        case 5: return "ENQ";
        case 6: return "ACK";
        case 7: return "BEL";
        case 8: return "BS";
        case 9: return "TAB";
        case 10: return "LF";
        case 11: return "VT";
        case 12: return "FF";
        case 13: return "CR";
        case 14: return "SO";
        case 15: return "SI";
        case 16: return "DLE";
        case 17: return "DC1";
        case 18: return "DC2";
        case 19: return "DC3";
        case 20: return "DC4";
        case 21: return "NAK";
        case 22: return "SYN";
        case 23: return "ETB";
        case 24: return "CAN";
        case 25: return "EM";
        case 26: return "SUB";
        case 27: return "ESC";
        case 28: return "FS";
        case 29: return "GS";
        case 30: return "RS";
        case 31: return "US";
        case 32: return "Space";
        case 127: return "DEL";
        default: {
            // For printable characters, return the actual character
            if (value >= 33 && value <= 126) {
                return QString(QChar(static_cast<char>(value)));
            }
            // For extended ASCII, try to convert using current codepage
            if (_codepage == 0 || _codepage == 1252) {
                QByteArray bytes;
                bytes.append(static_cast<char>(value));
                return QString::fromLatin1(bytes);
            }
            QTextCodec* codec = QTextCodec::codecForMib(_codepage);
            if (codec) {
                QByteArray bytes;
                bytes.append(static_cast<char>(value));
                return codec->toUnicode(bytes);
            }
            return QString(QChar(static_cast<char>(value)));
        }
    }
}

QString AnsiCharPanel::getHtmlName(unsigned char value) const
{
    switch (value) {
        case 33: return "&excl;";
        case 34: return "&quot;";
        case 35: return "&num;";
        case 36: return "&dollar;";
        case 37: return "&percnt;";
        case 38: return "&amp;";
        case 39: return "&apos;";
        case 40: return "&lpar;";
        case 41: return "&rpar;";
        case 42: return "&ast;";
        case 43: return "&plus;";
        case 44: return "&comma;";
        case 45: return "&minus;";
        case 46: return "&period;";
        case 47: return "&sol;";
        case 58: return "&colon;";
        case 59: return "&semi;";
        case 60: return "&lt;";
        case 61: return "&equals;";
        case 62: return "&gt;";
        case 63: return "&quest;";
        case 64: return "&commat;";
        case 91: return "&lbrack;";
        case 92: return "&bsol;";
        case 93: return "&rbrack;";
        case 94: return "&Hat;";
        case 95: return "&lowbar;";
        case 96: return "&grave;";
        case 123: return "&lbrace;";
        case 124: return "&vert;";
        case 125: return "&rbrace;";
        case 126: return "";  // ascii tilde
        case 128: return "&euro;";
        case 130: return "&sbquo;";
        case 131: return "&fnof;";
        case 132: return "&bdquo;";
        case 133: return "&hellip;";
        case 134: return "&dagger;";
        case 135: return "&Dagger;";
        case 136: return "&circ;";
        case 137: return "&permil;";
        case 138: return "&Scaron;";
        case 139: return "&lsaquo;";
        case 140: return "&OElig;";
        case 142: return "&Zcaron;";
        case 145: return "&lsquo;";
        case 146: return "&rsquo;";
        case 147: return "&ldquo;";
        case 148: return "&rdquo;";
        case 149: return "&bull;";
        case 150: return "&ndash;";
        case 151: return "&mdash;";
        case 152: return "&tilde;";
        case 153: return "&trade;";
        case 154: return "&scaron;";
        case 155: return "&rsaquo;";
        case 156: return "&oelig;";
        case 158: return "&zcaron;";
        case 159: return "&Yuml;";
        case 160: return "&nbsp;";
        case 161: return "&iexcl;";
        case 162: return "&cent;";
        case 163: return "&pound;";
        case 164: return "&curren;";
        case 165: return "&yen;";
        case 166: return "&brvbar;";
        case 167: return "&sect;";
        case 168: return "&uml;";
        case 169: return "&copy;";
        case 170: return "&ordf;";
        case 171: return "&laquo;";
        case 172: return "&not;";
        case 173: return "&shy;";
        case 174: return "&reg;";
        case 175: return "&macr;";
        case 176: return "&deg;";
        case 177: return "&plusmn;";
        case 178: return "&sup2;";
        case 179: return "&sup3;";
        case 180: return "&acute;";
        case 181: return "&micro;";
        case 182: return "&para;";
        case 183: return "&middot;";
        case 184: return "&cedil;";
        case 185: return "&sup1;";
        case 186: return "&ordm;";
        case 187: return "&raquo;";
        case 188: return "&frac14;";
        case 189: return "&frac12;";
        case 190: return "&frac34;";
        case 191: return "&iquest;";
        case 192: return "&Agrave;";
        case 193: return "&Aacute;";
        case 194: return "&Acirc;";
        case 195: return "&Atilde;";
        case 196: return "&Auml;";
        case 197: return "&Aring;";
        case 198: return "&AElig;";
        case 199: return "&Ccedil;";
        case 200: return "&Egrave;";
        case 201: return "&Eacute;";
        case 202: return "&Ecirc;";
        case 203: return "&Euml;";
        case 204: return "&Igrave;";
        case 205: return "&Iacute;";
        case 206: return "&Icirc;";
        case 207: return "&Iuml;";
        case 208: return "&ETH;";
        case 209: return "&Ntilde;";
        case 210: return "&Ograve;";
        case 211: return "&Oacute;";
        case 212: return "&Ocirc;";
        case 213: return "&Otilde;";
        case 214: return "&Ouml;";
        case 215: return "&times;";
        case 216: return "&Oslash;";
        case 217: return "&Ugrave;";
        case 218: return "&Uacute;";
        case 219: return "&Ucirc;";
        case 220: return "&Uuml;";
        case 221: return "&Yacute;";
        case 222: return "&THORN;";
        case 223: return "&szlig;";
        case 224: return "&agrave;";
        case 225: return "&aacute;";
        case 226: return "&acirc;";
        case 227: return "&atilde;";
        case 228: return "&auml;";
        case 229: return "&aring;";
        case 230: return "&aelig;";
        case 231: return "&ccedil;";
        case 232: return "&egrave;";
        case 233: return "&eacute;";
        case 234: return "&ecirc;";
        case 235: return "&euml;";
        case 236: return "&igrave;";
        case 237: return "&iacute;";
        case 238: return "&icirc;";
        case 239: return "&iuml;";
        case 240: return "&eth;";
        case 241: return "&ntilde;";
        case 242: return "&ograve;";
        case 243: return "&oacute;";
        case 244: return "&ocirc;";
        case 245: return "&otilde;";
        case 246: return "&ouml;";
        case 247: return "&divide;";
        case 248: return "&oslash;";
        case 249: return "&ugrave;";
        case 250: return "&uacute;";
        case 251: return "&ucirc;";
        case 252: return "&uuml;";
        case 253: return "&yacute;";
        case 254: return "&thorn;";
        case 255: return "&yuml;";
        default: return "";
    }
}

int AnsiCharPanel::getHtmlNumber(unsigned char value) const
{
    switch (value) {
        case 45: return 8722;
        case 128: return 8364;
        case 130: return 8218;
        case 131: return 402;
        case 132: return 8222;
        case 133: return 8230;
        case 134: return 8224;
        case 135: return 8225;
        case 136: return 710;
        case 137: return 8240;
        case 138: return 352;
        case 139: return 8249;
        case 140: return 338;
        case 142: return 381;
        case 145: return 8216;
        case 146: return 8217;
        case 147: return 8220;
        case 148: return 8221;
        case 149: return 8226;
        case 150: return 8211;
        case 151: return 8212;
        case 152: return 732;
        case 153: return 8482;
        case 154: return 353;
        case 155: return 8250;
        case 156: return 339;
        case 158: return 382;
        case 159: return 376;
        default: return -1;
    }
}

} // namespace QtControls
