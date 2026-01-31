// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "TreeView.h"
#include <QHeaderView>

namespace QtControls {

void TreeView::init(QWidget* parent)
{
    if (!parent) return;

    _parent = parent;

    QTreeWidget* treeWidget = new QTreeWidget(parent);
    _widget = treeWidget;

    // Set up default properties
    treeWidget->setColumnCount(1);
    treeWidget->setHeaderHidden(true);
    treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Connect signals
    connect(treeWidget, &QTreeWidget::itemClicked,
            this, &TreeView::onItemClicked);
    connect(treeWidget, &QTreeWidget::itemDoubleClicked,
            this, &TreeView::onItemDoubleClicked);
    connect(treeWidget, &QTreeWidget::itemChanged,
            this, &TreeView::onItemChanged);
    connect(treeWidget, &QTreeWidget::itemExpanded,
            this, &TreeView::onItemExpanded);
    connect(treeWidget, &QTreeWidget::itemCollapsed,
            this, &TreeView::onItemCollapsed);
    connect(treeWidget, &QTreeWidget::currentItemChanged,
            this, &TreeView::onCurrentItemChanged);
    connect(treeWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &TreeView::onItemSelectionChanged);
}

void TreeView::destroy()
{
    _itemMap.clear();
    _nextItemId = 0;
    if (_widget) {
        delete _widget;
        _widget = nullptr;
    }
}

int TreeView::addItem(const QString& text, int parentId)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return -1;

    QTreeWidgetItem* parentItem = nullptr;
    if (parentId >= 0) {
        parentItem = getItemById(parentId);
        if (!parentItem) return -1;
    }

    QTreeWidgetItem* newItem;
    if (parentItem) {
        newItem = new QTreeWidgetItem(parentItem);
    } else {
        newItem = new QTreeWidgetItem(treeWidget);
    }

    newItem->setText(0, text);

    // Ensure item is not checkable by default (Qt 6 may set this flag by default)
    newItem->setFlags(newItem->flags() & ~Qt::ItemIsUserCheckable);

    int itemId = _nextItemId++;
    _itemMap[itemId] = newItem;
    newItem->setData(0, Qt::UserRole, itemId);

    return itemId;
}

int TreeView::addItem(const QString& text, int parentId, int iconIndex)
{
    (void)iconIndex; // Icon index handling can be extended with icon list support
    return addItem(text, parentId);
}

void TreeView::removeItem(int itemId)
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return;

    // Remove all children recursively first
    while (item->childCount() > 0) {
        QTreeWidgetItem* child = item->child(0);
        int childId = getIdByItem(child);
        if (childId >= 0) {
            removeItem(childId);
        } else {
            delete child;
        }
    }

    _itemMap.remove(itemId);
    delete item;
}

void TreeView::clear()
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->clear();
    _itemMap.clear();
    _nextItemId = 0;
}

bool TreeView::setItemText(int itemId, const QString& text)
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return false;

    item->setText(0, text);
    return true;
}

QString TreeView::getItemText(int itemId) const
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return QString();

    return item->text(0);
}

bool TreeView::setItemData(int itemId, const QVariant& data)
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return false;

    item->setData(0, Qt::UserRole + 1, data);
    return true;
}

QVariant TreeView::getItemData(int itemId) const
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return QVariant();

    return item->data(0, Qt::UserRole + 1);
}

int TreeView::getSelectedItem() const
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return -1;

    QTreeWidgetItem* currentItem = treeWidget->currentItem();
    if (!currentItem) return -1;

    return getIdByItem(currentItem);
}

void TreeView::setSelectedItem(int itemId)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return;

    treeWidget->setCurrentItem(item);
    item->setSelected(true);
}

void TreeView::clearSelection()
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->clearSelection();
}

void TreeView::expand(int itemId)
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return;

    item->setExpanded(true);
}

void TreeView::collapse(int itemId)
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return;

    item->setExpanded(false);
}

void TreeView::expandAll()
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->expandAll();
}

void TreeView::collapseAll()
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->collapseAll();
}

void TreeView::toggleExpandCollapse(int itemId)
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return;

    item->setExpanded(!item->isExpanded());
}

bool TreeView::isExpanded(int itemId) const
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return false;

    return item->isExpanded();
}

void TreeView::setCheckable(int itemId, bool checkable)
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return;

    if (checkable) {
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Unchecked);
    } else {
        item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
        item->setData(0, Qt::CheckStateRole, QVariant());
    }
}

bool TreeView::isCheckable(int itemId) const
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return false;

    return item->flags() & Qt::ItemIsUserCheckable;
}

bool TreeView::isItemChecked(int itemId) const
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return false;

    return item->checkState(0) == Qt::Checked;
}

void TreeView::setItemChecked(int itemId, bool checked)
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return;

    item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
}

void TreeView::setCheckState(int itemId, Qt::CheckState state)
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return;

    item->setCheckState(0, state);
}

Qt::CheckState TreeView::getCheckState(int itemId) const
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return Qt::Unchecked;

    return item->checkState(0);
}

void TreeView::setItemIcon(int itemId, const QIcon& icon)
{
    setItemIcon(itemId, 0, icon);
}

void TreeView::setItemIcon(int itemId, int column, const QIcon& icon)
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return;

    item->setIcon(column, icon);
}

void TreeView::setIconSize(const QSize& size)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->setIconSize(size);
}

void TreeView::setColumnCount(int columns)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->setColumnCount(columns);
}

int TreeView::getColumnCount() const
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return 0;

    return treeWidget->columnCount();
}

void TreeView::setColumnText(int column, const QString& text)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    QTreeWidgetItem* header = treeWidget->headerItem();
    if (header) {
        header->setText(column, text);
    }
}

QString TreeView::getColumnText(int column) const
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return QString();

    QTreeWidgetItem* header = treeWidget->headerItem();
    if (header) {
        return header->text(column);
    }
    return QString();
}

void TreeView::setColumnWidth(int column, int width)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->setColumnWidth(column, width);
}

int TreeView::getColumnWidth(int column) const
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return 0;

    return treeWidget->columnWidth(column);
}

int TreeView::getRootItem() const
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return -1;

    QTreeWidgetItem* root = treeWidget->invisibleRootItem();
    if (root && root->childCount() > 0) {
        return getIdByItem(root->child(0));
    }
    return -1;
}

int TreeView::getParentItem(int itemId) const
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return -1;

    QTreeWidgetItem* parent = item->parent();
    if (!parent) return -1;

    return getIdByItem(parent);
}

int TreeView::getChildItem(int itemId) const
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return -1;

    if (item->childCount() > 0) {
        return getIdByItem(item->child(0));
    }
    return -1;
}

int TreeView::getNextSibling(int itemId) const
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return -1;

    QTreeWidgetItem* parent = item->parent();
    int index;
    if (parent) {
        index = parent->indexOfChild(item);
        if (index >= 0 && index + 1 < parent->childCount()) {
            return getIdByItem(parent->child(index + 1));
        }
    } else {
        QTreeWidget* treeWidget = getTreeWidget();
        if (treeWidget) {
            index = treeWidget->indexOfTopLevelItem(item);
            if (index >= 0 && index + 1 < treeWidget->topLevelItemCount()) {
                return getIdByItem(treeWidget->topLevelItem(index + 1));
            }
        }
    }
    return -1;
}

int TreeView::getPrevSibling(int itemId) const
{
    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return -1;

    QTreeWidgetItem* parent = item->parent();
    int index;
    if (parent) {
        index = parent->indexOfChild(item);
        if (index > 0) {
            return getIdByItem(parent->child(index - 1));
        }
    } else {
        QTreeWidget* treeWidget = getTreeWidget();
        if (treeWidget) {
            index = treeWidget->indexOfTopLevelItem(item);
            if (index > 0) {
                return getIdByItem(treeWidget->topLevelItem(index - 1));
            }
        }
    }
    return -1;
}

void TreeView::setDragEnabled(bool enabled)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->setDragEnabled(enabled);
}

void TreeView::setAcceptDrops(bool enabled)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->setAcceptDrops(enabled);
}

void TreeView::setDropIndicatorShown(bool shown)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->setDropIndicatorShown(shown);
}

void TreeView::sortItems(int column, Qt::SortOrder order)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->sortItems(column, order);
}

void TreeView::setSortingEnabled(bool enabled)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    treeWidget->setSortingEnabled(enabled);
}

int TreeView::findItem(const QString& text, int startItem) const
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return -1;

    // Build reverse map for efficient lookup
    QHash<QTreeWidgetItem*, int> reverseMap;
    for (auto it = _itemMap.constBegin(); it != _itemMap.constEnd(); ++it) {
        reverseMap[it.value()] = it.key();
    }

    // Search all items
    QTreeWidgetItemIterator it(treeWidget);
    bool foundStart = (startItem < 0);

    while (*it) {
        QTreeWidgetItem* item = *it;
        if (!foundStart) {
            if (reverseMap.value(item, -1) == startItem) {
                foundStart = true;
            }
        } else if (item->text(0).contains(text, Qt::CaseInsensitive)) {
            return reverseMap.value(item, -1);
        }
        ++it;
    }
    return -1;
}

int TreeView::findItemByData(const QVariant& data, int role) const
{
    for (auto it = _itemMap.constBegin(); it != _itemMap.constEnd(); ++it) {
        QTreeWidgetItem* item = it.value();
        if (item->data(0, role) == data) {
            return it.key();
        }
    }
    return -1;
}

bool TreeView::restoreFoldingStateFrom(const TreeStateNode& treeState2Compare, int treeviewNodeId)
{
    QTreeWidgetItem* item = getItemById(treeviewNodeId);
    if (!item) return false;

    // Check if labels match
    if (item->text(0) != treeState2Compare._label) {
        return false;
    }

    // Restore expansion state
    item->setExpanded(treeState2Compare._isExpanded);

    // Restore selection state
    if (treeState2Compare._isSelected) {
        setSelectedItem(treeviewNodeId);
    }

    // Restore data if present
    if (!treeState2Compare._extraData.isEmpty()) {
        setItemData(treeviewNodeId, treeState2Compare._extraData);
    }

    // Recursively restore children
    return restoreFoldingStateRecursive(treeState2Compare, item);
}

bool TreeView::retrieveFoldingStateTo(TreeStateNode& treeState2Construct, int treeviewNodeId)
{
    QTreeWidgetItem* item = getItemById(treeviewNodeId);
    if (!item) return false;

    treeState2Construct._label = item->text(0);
    treeState2Construct._isExpanded = item->isExpanded();
    treeState2Construct._isSelected = item->isSelected();
    treeState2Construct._extraData = getItemData(treeviewNodeId).toString();

    return retrieveFoldingStateRecursive(treeState2Construct, item);
}

void TreeView::makeLabelEditable(bool toBeEnabled)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    if (toBeEnabled) {
        treeWidget->setEditTriggers(QAbstractItemView::DoubleClicked |
                                       QAbstractItemView::EditKeyPressed);
    } else {
        treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

void TreeView::editItem(int itemId)
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    QTreeWidgetItem* item = getItemById(itemId);
    if (!item) return;

    treeWidget->editItem(item, 0);
}

QTreeWidgetItem* TreeView::getItemById(int itemId) const
{
    return _itemMap.value(itemId, nullptr);
}

int TreeView::getIdByItem(QTreeWidgetItem* item) const
{
    if (!item) return -1;

    // First try to get from item data (faster)
    QVariant data = item->data(0, Qt::UserRole);
    if (data.isValid()) {
        return data.toInt();
    }

    // Fallback to map lookup
    for (auto it = _itemMap.constBegin(); it != _itemMap.constEnd(); ++it) {
        if (it.value() == item) {
            return it.key();
        }
    }
    return -1;
}

void TreeView::foldExpandRecursively(QTreeWidgetItem* item, bool fold)
{
    if (!item) return;

    item->setExpanded(!fold);

    for (int i = 0; i < item->childCount(); ++i) {
        foldExpandRecursively(item->child(i), fold);
    }
}

bool TreeView::restoreFoldingStateRecursive(const TreeStateNode& stateNode, QTreeWidgetItem* treeItem)
{
    if (!treeItem) return false;

    for (const auto& childState : stateNode._children) {
        // Find matching child by label
        QTreeWidgetItem* matchingChild = nullptr;
        for (int i = 0; i < treeItem->childCount(); ++i) {
            QTreeWidgetItem* child = treeItem->child(i);
            if (child->text(0) == childState._label) {
                matchingChild = child;
                break;
            }
        }

        if (matchingChild) {
            int childId = getIdByItem(matchingChild);
            if (childId >= 0) {
                restoreFoldingStateFrom(childState, childId);
            }
        }
    }
    return true;
}

bool TreeView::retrieveFoldingStateRecursive(TreeStateNode& stateNode, QTreeWidgetItem* treeItem)
{
    if (!treeItem) return false;

    stateNode._children.clear();
    for (int i = 0; i < treeItem->childCount(); ++i) {
        QTreeWidgetItem* child = treeItem->child(i);
        TreeStateNode childState;
        int childId = getIdByItem(child);
        if (childId >= 0) {
            retrieveFoldingStateTo(childState, childId);
            stateNode._children.push_back(childState);
        }
    }
    return true;
}

void TreeView::onItemClicked(QTreeWidgetItem* item, int column)
{
    int itemId = getIdByItem(item);
    if (itemId >= 0) {
        emit itemClicked(itemId, column);
    }
}

void TreeView::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    int itemId = getIdByItem(item);
    if (itemId >= 0) {
        emit itemDoubleClicked(itemId, column);
    }
}

void TreeView::onItemChanged(QTreeWidgetItem* item, int column)
{
    int itemId = getIdByItem(item);
    if (itemId >= 0) {
        emit itemChanged(itemId, column);

        // Emit checked signal if checkbox state changed
        if (isCheckable(itemId)) {
            emit itemChecked(itemId, isItemChecked(itemId));
        }
    }
}

void TreeView::onItemExpanded(QTreeWidgetItem* item)
{
    int itemId = getIdByItem(item);
    if (itemId >= 0) {
        emit itemExpanded(itemId);
    }
}

void TreeView::onItemCollapsed(QTreeWidgetItem* item)
{
    int itemId = getIdByItem(item);
    if (itemId >= 0) {
        emit itemCollapsed(itemId);
    }
}

void TreeView::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    int currentId = getIdByItem(current);
    int previousId = getIdByItem(previous);
    emit currentItemChanged(currentId, previousId);
}

void TreeView::onItemSelectionChanged()
{
    QTreeWidget* treeWidget = getTreeWidget();
    if (!treeWidget) return;

    QTreeWidgetItem* current = treeWidget->currentItem();
    if (current) {
        int itemId = getIdByItem(current);
        if (itemId >= 0) {
            emit itemSelected(itemId);
        }
    }
}

} // namespace QtControls
