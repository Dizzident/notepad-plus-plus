// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Window.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QIcon>
#include <QHash>
#include <QVariant>

namespace QtControls {

struct TreeStateNode {
    QString _label;
    QString _extraData;
    bool _isExpanded = false;
    bool _isSelected = false;
    std::vector<TreeStateNode> _children;
};

class TreeView : public Window
{
    Q_OBJECT

public:
    TreeView() = default;
    ~TreeView() override = default;

    virtual void init(QWidget* parent) override;
    void destroy() override;

    // Item management
    int addItem(const QString& text, int parentId = -1);
    int addItem(const QString& text, int parentId, int iconIndex);
    void removeItem(int itemId);
    void clear();

    // Item properties
    bool setItemText(int itemId, const QString& text);
    QString getItemText(int itemId) const;
    bool setItemData(int itemId, const QVariant& data);
    QVariant getItemData(int itemId) const;

    // Selection handling
    int getSelectedItem() const;
    void setSelectedItem(int itemId);
    void clearSelection();

    // Expansion control
    void expand(int itemId);
    void collapse(int itemId);
    void expandAll();
    void collapseAll();
    void toggleExpandCollapse(int itemId);
    bool isExpanded(int itemId) const;

    // Checkbox support
    void setCheckable(int itemId, bool checkable);
    bool isCheckable(int itemId) const;
    bool isItemChecked(int itemId) const;
    void setItemChecked(int itemId, bool checked);
    void setCheckState(int itemId, Qt::CheckState state);
    Qt::CheckState getCheckState(int itemId) const;

    // Icon support
    void setItemIcon(int itemId, const QIcon& icon);
    void setItemIcon(int itemId, int column, const QIcon& icon);
    void setIconSize(const QSize& size);

    // Column handling
    void setColumnCount(int columns);
    int getColumnCount() const;
    void setColumnText(int column, const QString& text);
    QString getColumnText(int column) const;
    void setColumnWidth(int column, int width);
    int getColumnWidth(int column) const;

    // Tree navigation
    int getRootItem() const;
    int getParentItem(int itemId) const;
    int getChildItem(int itemId) const;
    int getNextSibling(int itemId) const;
    int getPrevSibling(int itemId) const;

    // Drag and drop
    void setDragEnabled(bool enabled);
    void setAcceptDrops(bool enabled);
    void setDropIndicatorShown(bool shown);
    bool isDragging() const { return _isItemDragged; }

    // Sorting
    void sortItems(int column, Qt::SortOrder order = Qt::AscendingOrder);
    void setSortingEnabled(bool enabled);

    // Search
    int findItem(const QString& text, int startItem = -1) const;
    int findItemByData(const QVariant& data, int role = Qt::UserRole + 1) const;

    // State persistence
    bool restoreFoldingStateFrom(const TreeStateNode& treeState2Compare, int treeviewNodeId);
    bool retrieveFoldingStateTo(TreeStateNode& treeState2Construct, int treeviewNodeId);

    // Edit mode
    void makeLabelEditable(bool toBeEnabled);
    void editItem(int itemId);

    // Direct access to underlying widget
    QTreeWidget* getTreeWidget() const { return qobject_cast<QTreeWidget*>(_widget); }

signals:
    void itemSelected(int itemId);
    void itemClicked(int itemId, int column);
    void itemDoubleClicked(int itemId, int column);
    void itemChanged(int itemId, int column);
    void itemExpanded(int itemId);
    void itemCollapsed(int itemId);
    void itemChecked(int itemId, bool checked);
    void currentItemChanged(int currentItemId, int previousItemId);

protected:
    QHash<int, QTreeWidgetItem*> _itemMap;
    int _nextItemId = 0;
    bool _isItemDragged = false;
    int _draggedItemId = -1;

    QTreeWidgetItem* getItemById(int itemId) const;
    int getIdByItem(QTreeWidgetItem* item) const;
    void collectItemIds(QTreeWidgetItem* item, QHash<QTreeWidgetItem*, int>& reverseMap) const;

    void foldExpandRecursively(QTreeWidgetItem* item, bool fold);
    bool restoreFoldingStateRecursive(const TreeStateNode& stateNode, QTreeWidgetItem* treeItem);
    bool retrieveFoldingStateRecursive(TreeStateNode& stateNode, QTreeWidgetItem* treeItem);

protected slots:
    void onItemClicked(QTreeWidgetItem* item, int column);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onItemExpanded(QTreeWidgetItem* item);
    void onItemCollapsed(QTreeWidgetItem* item);
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onItemSelectionChanged();
};

} // namespace QtControls
