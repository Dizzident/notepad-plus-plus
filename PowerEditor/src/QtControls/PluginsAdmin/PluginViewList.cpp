// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "PluginViewList.h"
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <algorithm>

namespace QtControls {

// ============================================================================
// PluginListItem implementation
// ============================================================================

PluginListItem::PluginListItem(const QString& displayName, const QString& version, QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(8);

    _checkBox = new QCheckBox(this);
    layout->addWidget(_checkBox);

    _nameLabel = new QLabel(displayName, this);
    _nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(_nameLabel);

    _versionLabel = new QLabel(version, this);
    _versionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(_versionLabel);

    connect(_checkBox, &QCheckBox::toggled, this, &PluginListItem::checkedChanged);
}

PluginListItem::~PluginListItem() = default;

bool PluginListItem::isChecked() const
{
    return _checkBox ? _checkBox->isChecked() : false;
}

void PluginListItem::setChecked(bool checked)
{
    if (_checkBox) {
        _checkBox->setChecked(checked);
    }
}

QString PluginListItem::displayName() const
{
    return _nameLabel ? _nameLabel->text() : QString();
}

QString PluginListItem::version() const
{
    return _versionLabel ? _versionLabel->text() : QString();
}

// ============================================================================
// PluginViewList implementation
// ============================================================================

PluginViewList::PluginViewList(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

PluginViewList::~PluginViewList() = default;

void PluginViewList::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    _tableWidget = new QTableWidget(this);
    _tableWidget->setColumnCount(3);
    _tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Select") << tr("Plugin") << tr("Version"));
    _tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    _tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    _tableWidget->setAlternatingRowColors(true);
    _tableWidget->setShowGrid(false);
    _tableWidget->verticalHeader()->setVisible(false);
    _tableWidget->horizontalHeader()->setStretchLastSection(true);
    _tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    layout->addWidget(_tableWidget);

    connect(_tableWidget, &QTableWidget::itemSelectionChanged,
            this, &PluginViewList::onItemSelectionChanged);
    connect(_tableWidget, &QTableWidget::itemDoubleClicked,
            this, &PluginViewList::onItemDoubleClicked);
    connect(_tableWidget->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &PluginViewList::onHeaderClicked);
}

void PluginViewList::pushBack(PluginUpdateInfo* pi)
{
    if (!pi) return;

    _list.push_back(pi);

    int row = _tableWidget->rowCount();
    _tableWidget->insertRow(row);

    // Checkbox item
    auto* checkItem = new QTableWidgetItem();
    checkItem->setFlags(checkItem->flags() | Qt::ItemIsUserCheckable);
    checkItem->setCheckState(Qt::Unchecked);
    _tableWidget->setItem(row, 0, checkItem);

    // Plugin name
    QString displayName = QString::fromStdWString(pi->_displayName);
    auto* nameItem = new QTableWidgetItem(displayName);
    nameItem->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<qintptr>(pi)));
    _tableWidget->setItem(row, 1, nameItem);

    // Version
    QString version = QString::fromStdWString(pi->_version.toString());
    auto* versionItem = new QTableWidgetItem(version);
    _tableWidget->setItem(row, 2, versionItem);
}

std::vector<size_t> PluginViewList::getCheckedIndexes() const
{
    std::vector<size_t> checked;
    for (int i = 0; i < _tableWidget->rowCount(); ++i) {
        auto* item = _tableWidget->item(i, 0);
        if (item && item->checkState() == Qt::Checked) {
            checked.push_back(static_cast<size_t>(i));
        }
    }
    return checked;
}

std::vector<PluginUpdateInfo*> PluginViewList::getCheckedPlugins() const
{
    std::vector<PluginUpdateInfo*> checked;
    for (int i = 0; i < _tableWidget->rowCount(); ++i) {
        auto* item = _tableWidget->item(i, 0);
        if (item && item->checkState() == Qt::Checked) {
            if (i < static_cast<int>(_list.size())) {
                checked.push_back(_list[i]);
            }
        }
    }
    return checked;
}

long PluginViewList::getSelectedIndex() const
{
    auto selected = _tableWidget->selectedItems();
    if (selected.isEmpty()) return -1;

    int row = selected.first()->row();
    return static_cast<long>(row);
}

void PluginViewList::setSelection(int index)
{
    if (index < 0 || index >= _tableWidget->rowCount()) return;

    _tableWidget->selectRow(index);
}

PluginUpdateInfo* PluginViewList::getPluginInfoFromUiIndex(size_t index) const
{
    if (index >= _list.size()) return nullptr;
    return _list[index];
}

PluginUpdateInfo* PluginViewList::findPluginInfoFromFolderName(const std::wstring& folderName, int& index) const
{
    for (size_t i = 0; i < _list.size(); ++i) {
        if (_list[i] && _list[i]->_folderName == folderName) {
            index = static_cast<int>(i);
            return _list[i];
        }
    }
    index = -1;
    return nullptr;
}

bool PluginViewList::removeFromListIndex(size_t index2remove)
{
    if (index2remove >= _list.size()) return false;

    _list.erase(_list.begin() + index2remove);
    _tableWidget->removeRow(static_cast<int>(index2remove));
    return true;
}

bool PluginViewList::hideFromListIndex(size_t index2Hide)
{
    if (index2Hide >= _list.size()) return false;

    // Store the hidden item
    _hiddenItems.push_back(std::make_pair(index2Hide, _list[index2Hide]));

    // Remove from visible list
    _list.erase(_list.begin() + index2Hide);
    _tableWidget->removeRow(static_cast<int>(index2Hide));

    return true;
}

bool PluginViewList::removeFromFolderName(const std::wstring& folderName)
{
    for (size_t i = 0; i < _list.size(); ++i) {
        if (_list[i] && _list[i]->_folderName == folderName) {
            return removeFromListIndex(i);
        }
    }
    return false;
}

bool PluginViewList::removeFromUiIndex(size_t index2remove)
{
    return removeFromListIndex(static_cast<size_t>(index2remove));
}

bool PluginViewList::hideFromPluginInfoPtr(PluginUpdateInfo* pluginInfo2hide)
{
    for (size_t i = 0; i < _list.size(); ++i) {
        if (_list[i] == pluginInfo2hide) {
            return hideFromListIndex(i);
        }
    }
    return false;
}

bool PluginViewList::restore(const std::wstring& folderName)
{
    // Find in hidden items
    for (auto it = _hiddenItems.begin(); it != _hiddenItems.end(); ++it) {
        if (it->second && it->second->_folderName == folderName) {
            // Re-add to the list at the original position if possible
            size_t originalIndex = it->first;
            if (originalIndex <= _list.size()) {
                _list.insert(_list.begin() + originalIndex, it->second);
            } else {
                _list.push_back(it->second);
                originalIndex = _list.size() - 1;
            }

            // Re-add to table
            int row = static_cast<int>(originalIndex);
            _tableWidget->insertRow(row);

            auto* checkItem = new QTableWidgetItem();
            checkItem->setFlags(checkItem->flags() | Qt::ItemIsUserCheckable);
            checkItem->setCheckState(Qt::Unchecked);
            _tableWidget->setItem(row, 0, checkItem);

            QString displayName = QString::fromStdWString(it->second->_displayName);
            auto* nameItem = new QTableWidgetItem(displayName);
            nameItem->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<qintptr>(it->second)));
            _tableWidget->setItem(row, 1, nameItem);

            QString version = QString::fromStdWString(it->second->_version.toString());
            auto* versionItem = new QTableWidgetItem(version);
            _tableWidget->setItem(row, 2, versionItem);

            _hiddenItems.erase(it);
            return true;
        }
    }
    return false;
}

bool PluginViewList::removeFromPluginInfoPtr(PluginUpdateInfo* pluginInfo2remove)
{
    for (size_t i = 0; i < _list.size(); ++i) {
        if (_list[i] == pluginInfo2remove) {
            return removeFromListIndex(i);
        }
    }
    return false;
}

void PluginViewList::changeColumnName(COLUMN_TYPE index, const wchar_t* name2change)
{
    if (!name2change) return;

    QString name = QString::fromWCharArray(name2change);
    int column = -1;

    switch (index) {
        case COLUMN_PLUGIN:
            column = 1;
            break;
        case COLUMN_VERSION:
            column = 2;
            break;
        default:
            return;
    }

    if (column >= 0) {
        _tableWidget->horizontalHeaderItem(column)->setText(name);
    }
}

void PluginViewList::filterItems(const QString& searchText, bool searchInNames, bool searchInDescs)
{
    QString lowerSearch = searchText.toLower();

    for (int i = 0; i < _tableWidget->rowCount(); ++i) {
        bool visible = false;

        if (i < static_cast<int>(_list.size())) {
            PluginUpdateInfo* pi = _list[i];
            if (pi) {
                if (searchInNames) {
                    QString name = QString::fromStdWString(pi->_displayName).toLower();
                    if (name.contains(lowerSearch)) {
                        visible = true;
                    }
                }
                if (searchInDescs && !visible) {
                    QString desc = QString::fromStdWString(pi->_description).toLower();
                    if (desc.contains(lowerSearch)) {
                        visible = true;
                    }
                }
            }
        }

        _tableWidget->setRowHidden(i, !visible);
    }
}

void PluginViewList::clear()
{
    _list.clear();
    _hiddenItems.clear();
    _tableWidget->setRowCount(0);
}

void PluginViewList::onItemSelectionChanged()
{
    long index = getSelectedIndex();
    emit itemSelectionChanged(static_cast<int>(index));
}

void PluginViewList::onItemDoubleClicked(QTableWidgetItem* item)
{
    if (!item) return;
    emit itemDoubleClicked(item->row());
}

void PluginViewList::onHeaderClicked(int column)
{
    // Toggle sort direction
    if (_sortColumn == column) {
        _sortType = (_sortType == DISPLAY_NAME_ALPHABET_ENCREASE)
                        ? DISPLAY_NAME_ALPHABET_DECREASE
                        : DISPLAY_NAME_ALPHABET_ENCREASE;
    } else {
        _sortColumn = column;
        _sortType = DISPLAY_NAME_ALPHABET_ENCREASE;
    }

    // Sort by plugin name (column 1)
    if (column == 1) {
        Qt::SortOrder order = (_sortType == DISPLAY_NAME_ALPHABET_ENCREASE)
                                  ? Qt::AscendingOrder
                                  : Qt::DescendingOrder;
        _tableWidget->sortItems(column, order);
    }
}

void PluginViewList::updateRow(int row)
{
    Q_UNUSED(row)
    // Update row display if needed
}

} // namespace QtControls
