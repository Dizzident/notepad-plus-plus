// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ListView.h"
#include <QVBoxLayout>

namespace QtControls {

void ListView::init(QWidget* parent)
{
    if (!parent) return;

    _parent = parent;

    QListWidget* listWidget = new QListWidget(parent);
    _widget = listWidget;

    // Set default selection mode
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Enable context menu
    listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect signals
    connect(listWidget, &QListWidget::itemClicked, this, &ListView::onItemClicked);
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &ListView::onItemDoubleClicked);
    connect(listWidget, &QListWidget::itemActivated, this, &ListView::onItemActivated);
    connect(listWidget, &QListWidget::itemPressed, this, &ListView::onItemPressed);
    connect(listWidget, &QListWidget::currentItemChanged, this, &ListView::onCurrentItemChanged);
    connect(listWidget, &QListWidget::itemChanged, this, &ListView::onItemChanged);
    connect(listWidget, &QListWidget::customContextMenuRequested, this, &ListView::onCustomContextMenuRequested);
}

void ListView::destroy()
{
    if (_widget) {
        delete _widget;
        _widget = nullptr;
    }
}

// ============================================================================
// Item management
// ============================================================================

void ListView::addItem(const QString& text)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    listWidget->addItem(text);
}

void ListView::addItem(const QString& text, const QIcon& icon)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    QListWidgetItem* item = new QListWidgetItem(icon, text);
    listWidget->addItem(item);
}

void ListView::insertItem(int index, const QString& text)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    listWidget->insertItem(index, text);
}

void ListView::insertItem(int index, const QString& text, const QIcon& icon)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    QListWidgetItem* item = new QListWidgetItem(icon, text);
    listWidget->insertItem(index, item);
}

void ListView::removeItem(int index)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    if (index >= 0 && index < listWidget->count()) {
        delete listWidget->takeItem(index);
    }
}

void ListView::clear()
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    listWidget->clear();
}

// ============================================================================
// Item count
// ============================================================================

int ListView::getItemCount() const
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return 0;

    return listWidget->count();
}

// ============================================================================
// Item text
// ============================================================================

QString ListView::getItemText(int index) const
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        return item->text();
    }
    return QString();
}

void ListView::setItemText(int index, const QString& text)
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        item->setText(text);
    }
}

// ============================================================================
// Item data
// ============================================================================

void ListView::setItemData(int index, const QVariant& data, int role)
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        item->setData(role, data);
    }
}

QVariant ListView::getItemData(int index, int role) const
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        return item->data(role);
    }
    return QVariant();
}

// ============================================================================
// Item icons
// ============================================================================

void ListView::setItemIcon(int index, const QIcon& icon)
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        item->setIcon(icon);
    }
}

QIcon ListView::getItemIcon(int index) const
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        return item->icon();
    }
    return QIcon();
}

// ============================================================================
// Selection handling - single selection
// ============================================================================

int ListView::getSelectedIndex() const
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return -1;

    QListWidgetItem* item = listWidget->currentItem();
    if (item) {
        return listWidget->row(item);
    }
    return -1;
}

void ListView::setSelectedIndex(int index)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    if (index >= 0 && index < listWidget->count()) {
        listWidget->setCurrentRow(index);
        listWidget->item(index)->setSelected(true);
    }
}

// ============================================================================
// Selection handling - multi-selection
// ============================================================================

std::vector<int> ListView::getSelectedIndexes() const
{
    std::vector<int> indexes;
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return indexes;

    QList<QListWidgetItem*> selectedItems = listWidget->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        indexes.push_back(listWidget->row(item));
    }

    return indexes;
}

void ListView::setSelectedIndexes(const std::vector<int>& indexes)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    // Clear current selection
    listWidget->clearSelection();

    // Select specified items
    for (int index : indexes) {
        if (index >= 0 && index < listWidget->count()) {
            listWidget->item(index)->setSelected(true);
        }
    }
}

void ListView::selectItem(int index, bool select)
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        item->setSelected(select);
    }
}

bool ListView::isItemSelected(int index) const
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        return item->isSelected();
    }
    return false;
}

// ============================================================================
// Selection mode
// ============================================================================

void ListView::setSelectionMode(ListViewSelectionMode mode)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    _selectionMode = mode;

    switch (mode) {
        case ListViewSelectionMode::Single:
            listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
            break;
        case ListViewSelectionMode::Multi:
            listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
            break;
        case ListViewSelectionMode::Extended:
            listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
            break;
        case ListViewSelectionMode::None:
            listWidget->setSelectionMode(QAbstractItemView::NoSelection);
            break;
    }
}

ListViewSelectionMode ListView::getSelectionMode() const
{
    return _selectionMode;
}

// ============================================================================
// Current item
// ============================================================================

int ListView::getCurrentIndex() const
{
    return getSelectedIndex();
}

void ListView::setCurrentIndex(int index)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    if (index >= 0 && index < listWidget->count()) {
        listWidget->setCurrentRow(index);
    }
}

// ============================================================================
// Sorting
// ============================================================================

void ListView::setSortingEnabled(bool enabled)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    _sortingEnabled = enabled;
    listWidget->setSortingEnabled(enabled);
}

bool ListView::isSortingEnabled() const
{
    return _sortingEnabled;
}

void ListView::sortItems(SortDirection direction)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    Qt::SortOrder order = (direction == SortDirection::Ascending) ?
                          Qt::AscendingOrder : Qt::DescendingOrder;
    listWidget->sortItems(order);
}

// ============================================================================
// Item visibility
// ============================================================================

void ListView::ensureItemVisible(int index)
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        QListWidget* listWidget = getListWidget();
        if (listWidget) {
            listWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
        }
    }
}

// ============================================================================
// Edit mode
// ============================================================================

void ListView::setEditTriggers(QAbstractItemView::EditTriggers triggers)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    listWidget->setEditTriggers(triggers);
}

void ListView::openPersistentEditor(int index)
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        QListWidget* listWidget = getListWidget();
        if (listWidget) {
            listWidget->openPersistentEditor(item);
        }
    }
}

void ListView::closePersistentEditor(int index)
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        QListWidget* listWidget = getListWidget();
        if (listWidget) {
            listWidget->closePersistentEditor(item);
        }
    }
}

// ============================================================================
// Check state support
// ============================================================================

void ListView::setItemCheckState(int index, Qt::CheckState state)
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        item->setCheckState(state);
    }
}

Qt::CheckState ListView::getItemCheckState(int index) const
{
    QListWidgetItem* item = getItem(index);
    if (item) {
        return item->checkState();
    }
    return Qt::Unchecked;
}

std::vector<int> ListView::getCheckedIndexes() const
{
    std::vector<int> checkedIndexes;
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return checkedIndexes;

    int count = listWidget->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem* item = listWidget->item(i);
        if (item && item->checkState() == Qt::Checked) {
            checkedIndexes.push_back(i);
        }
    }

    return checkedIndexes;
}

// ============================================================================
// Search
// ============================================================================

int ListView::findItem(const QString& text, Qt::MatchFlags flags) const
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return -1;

    QList<QListWidgetItem*> items = listWidget->findItems(text, flags);
    if (!items.isEmpty()) {
        return listWidget->row(items.first());
    }
    return -1;
}

std::vector<int> ListView::findItems(const QString& text, Qt::MatchFlags flags) const
{
    std::vector<int> indexes;
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return indexes;

    QList<QListWidgetItem*> items = listWidget->findItems(text, flags);
    for (QListWidgetItem* item : items) {
        indexes.push_back(listWidget->row(item));
    }

    return indexes;
}

// ============================================================================
// Private helper methods
// ============================================================================

QListWidgetItem* ListView::getItem(int index) const
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return nullptr;

    if (index >= 0 && index < listWidget->count()) {
        return listWidget->item(index);
    }
    return nullptr;
}

int ListView::getItemIndex(QListWidgetItem* item) const
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget || !item) return -1;

    return listWidget->row(item);
}

// ============================================================================
// Private slots
// ============================================================================

void ListView::onItemClicked(QListWidgetItem* item)
{
    int index = getItemIndex(item);
    emit itemClicked(index);
}

void ListView::onItemDoubleClicked(QListWidgetItem* item)
{
    int index = getItemIndex(item);
    emit itemDoubleClicked(index);
}

void ListView::onItemActivated(QListWidgetItem* item)
{
    int index = getItemIndex(item);
    emit itemActivated(index);
}

void ListView::onItemPressed(QListWidgetItem* item)
{
    int index = getItemIndex(item);
    emit itemPressed(index);
}

void ListView::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    (void)previous;
    if (current) {
        int index = getItemIndex(current);
        emit itemSelected(index);
    }
    emit selectionChanged();
}

void ListView::onItemChanged(QListWidgetItem* item)
{
    if (item) {
        int index = getItemIndex(item);
        emit itemTextChanged(index, item->text());
    }
}

void ListView::onCustomContextMenuRequested(const QPoint& pos)
{
    QListWidget* listWidget = getListWidget();
    if (!listWidget) return;

    QListWidgetItem* item = listWidget->itemAt(pos);
    int index = getItemIndex(item);
    emit contextMenuRequested(index, pos);
}

} // namespace QtControls
