// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Window.h"
#include <QListWidget>
#include <QIcon>
#include <QVariant>
#include <vector>

namespace QtControls {

// Sort direction for list items
enum class SortDirection {
    Ascending,
    Descending
};

// Selection mode for the list view
enum class ListViewSelectionMode {
    Single,
    Multi,
    Extended,
    None
};

class ListView : public Window
{
    Q_OBJECT

public:
    ListView() = default;
    ~ListView() override = default;

    // Initialization
    virtual void init(QWidget* parent) override;
    void destroy() override;

    // Item management
    void addItem(const QString& text);
    void addItem(const QString& text, const QIcon& icon);
    void insertItem(int index, const QString& text);
    void insertItem(int index, const QString& text, const QIcon& icon);
    void removeItem(int index);
    void clear();

    // Item count
    int getItemCount() const;

    // Item text
    QString getItemText(int index) const;
    void setItemText(int index, const QString& text);

    // Item data (user data associated with items)
    void setItemData(int index, const QVariant& data, int role = Qt::UserRole);
    QVariant getItemData(int index, int role = Qt::UserRole) const;

    // Item icons
    void setItemIcon(int index, const QIcon& icon);
    QIcon getItemIcon(int index) const;

    // Selection handling - single selection
    int getSelectedIndex() const;
    void setSelectedIndex(int index);

    // Selection handling - multi-selection
    std::vector<int> getSelectedIndexes() const;
    void setSelectedIndexes(const std::vector<int>& indexes);
    void selectItem(int index, bool select = true);
    bool isItemSelected(int index) const;

    // Selection mode
    void setSelectionMode(ListViewSelectionMode mode);
    ListViewSelectionMode getSelectionMode() const;

    // Current item (focused item)
    int getCurrentIndex() const;
    void setCurrentIndex(int index);

    // Sorting
    void setSortingEnabled(bool enabled);
    bool isSortingEnabled() const;
    void sortItems(SortDirection direction = SortDirection::Ascending);

    // Item visibility
    void ensureItemVisible(int index);

    // Edit mode
    void setEditTriggers(QAbstractItemView::EditTriggers triggers);
    void openPersistentEditor(int index);
    void closePersistentEditor(int index);

    // Check state support
    void setItemCheckState(int index, Qt::CheckState state);
    Qt::CheckState getItemCheckState(int index) const;
    std::vector<int> getCheckedIndexes() const;

    // Search
    int findItem(const QString& text, Qt::MatchFlags flags = Qt::MatchExactly) const;
    std::vector<int> findItems(const QString& text, Qt::MatchFlags flags = Qt::MatchExactly) const;

    // Direct access to QListWidget
    QListWidget* getListWidget() const { return qobject_cast<QListWidget*>(_widget); }

signals:
    // Item interaction signals
    void itemSelected(int index);
    void itemClicked(int index);
    void itemDoubleClicked(int index);
    void itemActivated(int index);
    void itemPressed(int index);

    // Selection change
    void selectionChanged();

    // Item editing
    void itemTextChanged(int index, const QString& newText);

    // Context menu
    void contextMenuRequested(int index, const QPoint& pos);

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onItemDoubleClicked(QListWidgetItem* item);
    void onItemActivated(QListWidgetItem* item);
    void onItemPressed(QListWidgetItem* item);
    void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onItemChanged(QListWidgetItem* item);
    void onCustomContextMenuRequested(const QPoint& pos);

private:
    QListWidgetItem* getItem(int index) const;
    int getItemIndex(QListWidgetItem* item) const;

    bool _sortingEnabled = false;
    ListViewSelectionMode _selectionMode = ListViewSelectionMode::Single;
};

} // namespace QtControls
