// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "VerticalFileSwitcher.h"
#include "../ListView/ListView.h"
#include "../../ScintillaComponent/ScintillaEditView.h"
#include "../../Parameters.h"
#include "../../MISC/PluginsManager/Notepad_plus_msgs.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QTreeWidget>
#include <QContextMenuEvent>
#include <QColor>
#include <QFileInfo>
#include <QDir>
#include <algorithm>

namespace QtControls {

// ============================================================================
// VerticalFileSwitcher Implementation
// ============================================================================

VerticalFileSwitcher::VerticalFileSwitcher(QWidget* parent)
    : StaticDialog(parent)
{
}

VerticalFileSwitcher::~VerticalFileSwitcher()
{
    // Clean up doc item data
    removeAll();
}

void VerticalFileSwitcher::init(ScintillaEditView** ppEditView)
{
    _ppEditView = ppEditView;
}

void VerticalFileSwitcher::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setWindowTitle(tr("Document List"));
    dialog->resize(250, 400);

    _mainLayout = new QVBoxLayout(dialog);
    _mainLayout->setSpacing(0);
    _mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create the list view
    setupListView();

    // Create context menu
    setupContextMenu();

    // Store initial rect
    _rc = dialog->geometry();
}

void VerticalFileSwitcher::setupListView()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    _fileListView = new ListView();
    _fileListView->init(dialog);

    // Configure list view
    _fileListView->setSelectionMode(ListViewSelectionMode::Extended);

    // Get the underlying QListWidget to configure headers
    QListWidget* listWidget = _fileListView->getListWidget();
    if (listWidget) {
        listWidget->setUniformItemSizes(true);
        listWidget->setAlternatingRowColors(true);
    }

    _mainLayout->addWidget(_fileListView->getWidget(), 1);
}

void VerticalFileSwitcher::setupContextMenu()
{
    _contextMenu = new QMenu(this);

    _extColumnAction = new QAction(tr("Ext."), this);
    _extColumnAction->setCheckable(true);
    _extColumnAction->setChecked(true);
    connect(_extColumnAction, &QAction::triggered, this, &VerticalFileSwitcher::onToggleExtColumn);
    _contextMenu->addAction(_extColumnAction);

    _pathColumnAction = new QAction(tr("Path"), this);
    _pathColumnAction->setCheckable(true);
    _pathColumnAction->setChecked(true);
    connect(_pathColumnAction, &QAction::triggered, this, &VerticalFileSwitcher::onTogglePathColumn);
    _contextMenu->addAction(_pathColumnAction);

    _contextMenu->addSeparator();

    _groupByViewAction = new QAction(tr("Group by View"), this);
    _groupByViewAction->setCheckable(true);
    _groupByViewAction->setChecked(true);
    connect(_groupByViewAction, &QAction::triggered, this, &VerticalFileSwitcher::onToggleGroupByView);
    _contextMenu->addAction(_groupByViewAction);
}

void VerticalFileSwitcher::connectSignals()
{
    if (_fileListView) {
        connect(_fileListView, &ListView::itemClicked,
                this, &VerticalFileSwitcher::onItemClicked);
        connect(_fileListView, &ListView::itemDoubleClicked,
                this, &VerticalFileSwitcher::onItemDoubleClicked);
        connect(_fileListView, &ListView::contextMenuRequested,
                this, &VerticalFileSwitcher::onContextMenuRequested);
        connect(_fileListView, &ListView::selectionChanged,
                this, &VerticalFileSwitcher::onSelectionChanged);
    }
}

void VerticalFileSwitcher::doDialog()
{
    if (!isCreated()) {
        create(tr("Document List"), false);
        setupUI();
        connectSignals();
        initList();
    }

    display(true);
    ensureVisibleCurrentItem();
}

bool VerticalFileSwitcher::run_dlgProc(QEvent* event)
{
    (void)event;
    return false;
}

void VerticalFileSwitcher::initList()
{
    // Clear existing items
    removeAll();

    // Get settings from NppParameters
    NppParameters& nppParams = NppParameters::getInstance();
    const NppGUI& nppGUI = nppParams.getNppGUI();

    _showExtColumn = !nppGUI._fileSwitcherWithoutExtColumn;
    _showPathColumn = !nppGUI._fileSwitcherWithoutPathColumn;
    _groupByView = !nppGUI._fileSwitcherDisableListViewGroups;

    // Update menu check states
    if (_extColumnAction) {
        _extColumnAction->setChecked(_showExtColumn);
    }
    if (_pathColumnAction) {
        _pathColumnAction->setChecked(_showPathColumn);
    }
    if (_groupByViewAction) {
        _groupByViewAction->setChecked(_groupByView);
    }

    // Get buffer information from main window
    // This would typically be done via messages to the parent
    // For now, we'll rely on external calls to newItem()

    // Apply colors if set
    if (_bgColor.isValid()) {
        setBackgroundColor(_bgColor);
    }
    if (_fgColor.isValid()) {
        setForegroundColor(_fgColor);
    }
}

void VerticalFileSwitcher::removeAll()
{
    if (!_fileListView) return;

    // Delete all doc item data
    int count = _fileListView->getItemCount();
    for (int i = 0; i < count; ++i) {
        QVariant data = _fileListView->getItemData(i, Qt::UserRole);
        DocItemData* docData = data.value<DocItemData*>();
        delete docData;
    }

    _fileListView->clear();
}

int VerticalFileSwitcher::findItem(BufferID bufferID, int iView) const
{
    if (!_fileListView) return -1;

    int count = _fileListView->getItemCount();
    for (int i = 0; i < count; ++i) {
        QVariant data = _fileListView->getItemData(i, Qt::UserRole);
        DocItemData* docData = data.value<DocItemData*>();
        if (docData && docData->_bufID == bufferID && docData->_iView == iView) {
            return i;
        }
    }
    return -1;
}

int VerticalFileSwitcher::addItem(BufferID bufferID, int iView)
{
    if (!_fileListView || !bufferID) return -1;

    Buffer* buf = bufferID;
    QString filePath = QString::fromWCharArray(buf->getFullPathName());
    QString fileName = QString::fromWCharArray(buf->getFileName());

    // Get status icon index
    int status = getStatusIconIndex(bufferID);

    // Create doc item data
    DocItemData* docData = new DocItemData(iView, _fileListView->getItemCount(), filePath, status, bufferID, buf->getDocColorId());

    // Prepare display text
    QString displayText;
    if (_showExtColumn) {
        displayText = getFileNameOnly(filePath);
    } else {
        displayText = fileName;
    }

    // Add to list
    int index = _fileListView->getItemCount();
    _fileListView->addItem(displayText);

    // Store data
    _fileListView->setItemData(index, QVariant::fromValue(docData), Qt::UserRole);

    // Set tooltip to full path
    QListWidget* listWidget = _fileListView->getListWidget();
    if (listWidget) {
        QListWidgetItem* item = listWidget->item(index);
        if (item) {
            item->setToolTip(filePath);
        }
    }

    return index;
}

void VerticalFileSwitcher::removeItem(int index)
{
    if (!_fileListView) return;

    QVariant data = _fileListView->getItemData(index, Qt::UserRole);
    DocItemData* docData = data.value<DocItemData*>();
    delete docData;

    _fileListView->removeItem(index);
}

int VerticalFileSwitcher::newItem(BufferID bufferID, int iView)
{
    int index = findItem(bufferID, iView);
    if (index == -1) {
        index = addItem(bufferID, iView);
    }
    return index;
}

int VerticalFileSwitcher::closeItem(BufferID bufferID, int iView)
{
    int index = findItem(bufferID, iView);
    if (index != -1) {
        removeItem(index);
    }
    return index;
}

void VerticalFileSwitcher::activateItem(BufferID bufferID, int iView)
{
    if (!_fileListView) return;

    // Clear current selection
    int count = _fileListView->getItemCount();
    for (int i = 0; i < count; ++i) {
        _fileListView->selectItem(i, false);
    }

    // Find and select the item
    int index = findItem(bufferID, iView);
    if (index != -1) {
        _fileListView->setSelectedIndex(index);
        _fileListView->ensureItemVisible(index);
    }
}

void VerticalFileSwitcher::setItemIconStatus(BufferID bufferID)
{
    if (!_fileListView || !bufferID) return;

    Buffer* buf = bufferID;
    QString filePath = QString::fromWCharArray(buf->getFullPathName());
    int status = getStatusIconIndex(bufferID);

    int count = _fileListView->getItemCount();
    for (int i = 0; i < count; ++i) {
        QVariant data = _fileListView->getItemData(i, Qt::UserRole);
        DocItemData* docData = data.value<DocItemData*>();
        if (docData && docData->_bufID == bufferID) {
            docData->_status = status;
            docData->_filePath = filePath;

            // Update display text
            QString displayText;
            if (_showExtColumn) {
                displayText = getFileNameOnly(filePath);
            } else {
                displayText = QString::fromWCharArray(buf->getFileName());
            }
            _fileListView->setItemText(i, displayText);

            // Update tooltip
            QListWidget* listWidget = _fileListView->getListWidget();
            if (listWidget) {
                QListWidgetItem* item = listWidget->item(i);
                if (item) {
                    item->setToolTip(filePath);
                }
            }
            break;
        }
    }
}

void VerticalFileSwitcher::setItemColor(BufferID bufferID)
{
    if (!_fileListView || !bufferID) return;

    Buffer* buf = bufferID;
    int docColor = buf->getDocColorId();

    int count = _fileListView->getItemCount();
    for (int i = 0; i < count; ++i) {
        QVariant data = _fileListView->getItemData(i, Qt::UserRole);
        DocItemData* docData = data.value<DocItemData*>();
        if (docData && docData->_bufID == bufferID) {
            docData->_docColor = docColor;
            break;
        }
    }

    // Trigger redraw
    QListWidget* listWidget = _fileListView->getListWidget();
    if (listWidget) {
        listWidget->update();
    }
}

QString VerticalFileSwitcher::getFullFilePath(int index) const
{
    if (!_fileListView) return QString();

    QVariant data = _fileListView->getItemData(index, Qt::UserRole);
    DocItemData* docData = data.value<DocItemData*>();
    if (docData) {
        return docData->_filePath;
    }
    return QString();
}

int VerticalFileSwitcher::nbSelectedFiles() const
{
    if (!_fileListView) return 0;
    return static_cast<int>(_fileListView->getSelectedIndexes().size());
}

std::vector<DocItemData> VerticalFileSwitcher::getSelectedFiles(bool reverse) const
{
    std::vector<DocItemData> files;
    if (!_fileListView) return files;

    std::vector<int> selectedIndexes = _fileListView->getSelectedIndexes();
    int count = _fileListView->getItemCount();

    if (reverse) {
        // Return unselected files
        for (int i = 0; i < count; ++i) {
            if (std::find(selectedIndexes.begin(), selectedIndexes.end(), i) == selectedIndexes.end()) {
                QVariant data = _fileListView->getItemData(i, Qt::UserRole);
                DocItemData* docData = data.value<DocItemData*>();
                if (docData) {
                    files.push_back(*docData);
                }
            }
        }
    } else {
        // Return selected files
        for (int index : selectedIndexes) {
            QVariant data = _fileListView->getItemData(index, Qt::UserRole);
            DocItemData* docData = data.value<DocItemData*>();
            if (docData) {
                files.push_back(*docData);
            }
        }
    }

    return files;
}

void VerticalFileSwitcher::reload()
{
    // Save current selection
    std::vector<int> selectedIndexes;
    if (_fileListView) {
        selectedIndexes = _fileListView->getSelectedIndexes();
    }

    // Reload the list
    initList();

    // Restore selection if possible
    if (!selectedIndexes.empty() && _fileListView) {
        for (int index : selectedIndexes) {
            if (index < _fileListView->getItemCount()) {
                _fileListView->selectItem(index, true);
            }
        }
    }
}

void VerticalFileSwitcher::updateTabOrder()
{
    if (_lastSortingDirection == SORT_DIRECTION_NONE) {
        reload();
    }
}

int VerticalFileSwitcher::setHeaderOrder(int columnIndex)
{
    // Strip sort indicators from old column
    if (_lastSortingColumn != columnIndex && _lastSortingDirection != SORT_DIRECTION_NONE) {
        _lastSortingDirection = SORT_DIRECTION_NONE;
    }

    if (_lastSortingDirection == SORT_DIRECTION_NONE) {
        return SORT_DIRECTION_UP;
    }

    if (_lastSortingDirection == SORT_DIRECTION_UP) {
        return SORT_DIRECTION_DOWN;
    }

    return SORT_DIRECTION_NONE;
}

void VerticalFileSwitcher::updateHeaderArrow()
{
    // In QListWidget, we don't have column headers
    // This would be implemented if using QTreeWidget with columns
}

void VerticalFileSwitcher::startColumnSort()
{
    // Reset sorting if needed
    if (_lastSortingColumn >= 3) {  // Max 3 columns
        _lastSortingColumn = 0;
        _lastSortingDirection = SORT_DIRECTION_NONE;
    }

    if (_lastSortingDirection != SORT_DIRECTION_NONE) {
        sortItems(_lastSortingColumn, _lastSortingDirection);
    } else {
        reload();
    }

    updateHeaderArrow();
}

void VerticalFileSwitcher::sortItems(int column, int direction)
{
    // For QListWidget, we can sort by text
    // More complex sorting would require custom comparison
    QListWidget* listWidget = _fileListView ? _fileListView->getListWidget() : nullptr;
    if (listWidget) {
        Qt::SortOrder order = (direction == SORT_DIRECTION_UP) ?
                              Qt::AscendingOrder : Qt::DescendingOrder;
        listWidget->sortItems(order);
    }
}

void VerticalFileSwitcher::setBackgroundColor(const QColor& bgColour)
{
    _bgColor = bgColour;

    QListWidget* listWidget = _fileListView ? _fileListView->getListWidget() : nullptr;
    if (listWidget) {
        QPalette palette = listWidget->palette();
        palette.setColor(QPalette::Base, bgColour);
        listWidget->setPalette(palette);
    }
}

void VerticalFileSwitcher::setForegroundColor(const QColor& fgColour)
{
    _fgColor = fgColour;

    QListWidget* listWidget = _fileListView ? _fileListView->getListWidget() : nullptr;
    if (listWidget) {
        QPalette palette = listWidget->palette();
        palette.setColor(QPalette::Text, fgColour);
        listWidget->setPalette(palette);
    }
}

void VerticalFileSwitcher::ensureVisibleCurrentItem()
{
    if (_fileListView) {
        int index = _fileListView->getCurrentIndex();
        if (index >= 0) {
            _fileListView->ensureItemVisible(index);
        }
    }
}

void VerticalFileSwitcher::activateDoc(DocItemData* docData) const
{
    if (!docData || !_ppEditView || !*_ppEditView) return;

    int view = docData->_iView;
    BufferID bufferID = docData->_bufID;

    // Get current state
    // Note: These would need to be implemented via the main window
    // For now, we emit a signal or call back to parent

    // This is a placeholder - actual implementation would send messages
    // to the main Notepad++ window to activate the document
}

void VerticalFileSwitcher::closeDoc(DocItemData* docData) const
{
    if (!docData) return;

    // This is a placeholder - actual implementation would send messages
    // to the main Notepad++ window to close the document
}

int VerticalFileSwitcher::getStatusIconIndex(BufferID bufferID) const
{
    if (!bufferID) return 0;

    Buffer* buf = bufferID;
    if (buf->isMonitoringOn()) return 3;
    if (buf->isReadOnly()) return 2;
    if (buf->isDirty()) return 1;
    return 0;
}

QString VerticalFileSwitcher::getFileNameOnly(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.completeBaseName();
}

QString VerticalFileSwitcher::getFileExtension(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix();
}

QString VerticalFileSwitcher::getFileDirectory(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.path();
}

void VerticalFileSwitcher::resizeColumns()
{
    // QListWidget doesn't have resizable columns like QListView in report mode
    // This would be implemented if using QTreeWidget
}

void VerticalFileSwitcher::selectCurrentItem()
{
    if (_fileListView) {
        int index = _fileListView->getCurrentIndex();
        if (index >= 0) {
            _fileListView->selectItem(index, true);
        }
    }
}

void VerticalFileSwitcher::resizeEvent(QResizeEvent* event)
{
    StaticDialog::resizeEvent(event);
    resizeColumns();
}

void VerticalFileSwitcher::contextMenuEvent(QContextMenuEvent* event)
{
    if (_contextMenu && nbSelectedFiles() == 0) {
        _contextMenu->exec(event->globalPos());
    }
}

// ============================================================================
// Slots
// ============================================================================

void VerticalFileSwitcher::onItemClicked(int index)
{
    if (!_fileListView || index < 0) return;

    // Get the item data
    QVariant data = _fileListView->getItemData(index, Qt::UserRole);
    DocItemData* docData = data.value<DocItemData*>();
    if (docData) {
        activateDoc(docData);
    }
}

void VerticalFileSwitcher::onItemDoubleClicked(int index)
{
    if (index == -1) {
        // Double-click on empty area - create new file
        // This would send IDM_FILE_NEW to parent
    }
}

void VerticalFileSwitcher::onContextMenuRequested(int index, const QPoint& pos)
{
    (void)index;

    if (nbSelectedFiles() >= 1) {
        // Redirect to parent for document context menu
        // This would emit a signal or call back to show the document context menu
    } else if (_contextMenu) {
        _contextMenu->exec(pos);
    }
}

void VerticalFileSwitcher::onSelectionChanged()
{
    // Handle selection change if needed
}

void VerticalFileSwitcher::onToggleExtColumn()
{
    _showExtColumn = !_showExtColumn;
    if (_extColumnAction) {
        _extColumnAction->setChecked(_showExtColumn);
    }

    // Update settings
    NppParameters::getInstance().getNppGUI()._fileSwitcherWithoutExtColumn = !_showExtColumn;

    reload();
}

void VerticalFileSwitcher::onTogglePathColumn()
{
    _showPathColumn = !_showPathColumn;
    if (_pathColumnAction) {
        _pathColumnAction->setChecked(_showPathColumn);
    }

    // Update settings
    NppParameters::getInstance().getNppGUI()._fileSwitcherWithoutPathColumn = !_showPathColumn;

    reload();
}

void VerticalFileSwitcher::onToggleGroupByView()
{
    _groupByView = !_groupByView;
    if (_groupByViewAction) {
        _groupByViewAction->setChecked(_groupByView);
    }

    // Update settings
    NppParameters::getInstance().getNppGUI()._fileSwitcherDisableListViewGroups = !_groupByView;

    reload();
}

} // namespace QtControls
