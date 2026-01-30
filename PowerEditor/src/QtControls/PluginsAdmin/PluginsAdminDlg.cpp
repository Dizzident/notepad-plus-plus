// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "PluginsAdminDlg.h"
#include "../../MISC/PluginsManager/PluginsManager.h"
#include "../../WinControls/PluginsAdmin/pluginsAdminRes.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtGui/QResizeEvent>

namespace QtControls {

PluginsAdminDlg::PluginsAdminDlg(QWidget* parent)
    : StaticDialog(parent)
    , _availableList(this)
    , _updateList(this)
    , _installedList(this)
    , _incompatibleList(this)
{
    setWindowTitle(tr("Plugins Admin"));
    resize(700, 500);

    // Initialize status
    _nppCurrentStatus = new NppCurrentStatus();
}

PluginsAdminDlg::~PluginsAdminDlg()
{
    delete _nppCurrentStatus;
}

void PluginsAdminDlg::create(int dialogID, bool isRTL, bool msgDestParent)
{
    Q_UNUSED(dialogID)
    Q_UNUSED(msgDestParent)

    setupUI();
    connectSignals();
    setupDialog(isRTL);
    _isCreated = true;
}

void PluginsAdminDlg::doDialog(bool isRTL)
{
    if (!isCreated()) {
        create(IDD_PLUGINSADMIN_DLG, isRTL);
    }
    display();
    raise();
    activateWindow();
}

void PluginsAdminDlg::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // Search section
    auto* searchLayout = new QHBoxLayout();
    _searchLabel = new QLabel(tr("Search:"), this);
    searchLayout->addWidget(_searchLabel);

    _searchEdit = new QLineEdit(this);
    _searchEdit->setPlaceholderText(tr("Search for plugins..."));
    searchLayout->addWidget(_searchEdit, 1);

    _searchNextButton = new QPushButton(tr("Find Next"), this);
    searchLayout->addWidget(_searchNextButton);

    mainLayout->addLayout(searchLayout);

    // Tab widget
    _tabWidget = new QTabWidget(this);
    mainLayout->addWidget(_tabWidget, 1);

    createTabs();

    // Version info section
    auto* versionLayout = new QHBoxLayout();
    _versionLabel = new QLabel(tr("Plugin List Version:"), this);
    versionLayout->addWidget(_versionLabel);

    _versionNumberLabel = new QLabel("-", this);
    versionLayout->addWidget(_versionNumberLabel);

    versionLayout->addStretch();

    _repoLinkLabel = new QLabel(tr("<a href=\"https://github.com/notepad-plus-plus/nppPluginList\">Plugin List Repository</a>"), this);
    _repoLinkLabel->setOpenExternalLinks(true);
    versionLayout->addWidget(_repoLinkLabel);

    mainLayout->addLayout(versionLayout);

    // Button section
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _installButton = new QPushButton(tr("Install"), this);
    buttonLayout->addWidget(_installButton);

    _updateButton = new QPushButton(tr("Update"), this);
    buttonLayout->addWidget(_updateButton);

    _removeButton = new QPushButton(tr("Remove"), this);
    buttonLayout->addWidget(_removeButton);

    buttonLayout->addSpacing(20);

    _closeButton = new QPushButton(tr("Close"), this);
    buttonLayout->addWidget(_closeButton);

    mainLayout->addLayout(buttonLayout);

    updateButtonStates();
}

void PluginsAdminDlg::createTabs()
{
    // Available tab
    _availableTab = new QWidget();
    auto* availLayout = new QVBoxLayout(_availableTab);
    availLayout->setContentsMargins(4, 4, 4, 4);
    availLayout->addWidget(&_availableList);
    _tabWidget->addTab(_availableTab, tr("Available"));

    // Updates tab
    _updatesTab = new QWidget();
    auto* updateLayout = new QVBoxLayout(_updatesTab);
    updateLayout->setContentsMargins(4, 4, 4, 4);
    updateLayout->addWidget(&_updateList);
    _tabWidget->addTab(_updatesTab, tr("Updates"));

    // Installed tab
    _installedTab = new QWidget();
    auto* installedLayout = new QVBoxLayout(_installedTab);
    installedLayout->setContentsMargins(4, 4, 4, 4);
    installedLayout->addWidget(&_installedList);
    _tabWidget->addTab(_installedTab, tr("Installed"));

    // Incompatible tab
    _incompatibleTab = new QWidget();
    auto* incompatibleLayout = new QVBoxLayout(_incompatibleTab);
    incompatibleLayout->setContentsMargins(4, 4, 4, 4);
    incompatibleLayout->addWidget(&_incompatibleList);
    _tabWidget->addTab(_incompatibleTab, tr("Incompatible"));
}

void PluginsAdminDlg::connectSignals()
{
    connect(_tabWidget, &QTabWidget::currentChanged, this, &PluginsAdminDlg::onTabChanged);
    connect(_searchEdit, &QLineEdit::textChanged, this, &PluginsAdminDlg::onSearchTextChanged);
    connect(_searchNextButton, &QPushButton::clicked, this, &PluginsAdminDlg::onSearchNextClicked);
    connect(_installButton, &QPushButton::clicked, this, &PluginsAdminDlg::onInstallClicked);
    connect(_updateButton, &QPushButton::clicked, this, &PluginsAdminDlg::onUpdateClicked);
    connect(_removeButton, &QPushButton::clicked, this, &PluginsAdminDlg::onRemoveClicked);
    connect(_closeButton, &QPushButton::clicked, this, &PluginsAdminDlg::onCloseClicked);

    // Connect list signals
    connect(&_availableList, &PluginViewList::itemSelectionChanged,
            this, &PluginsAdminDlg::onItemSelectionChanged);
    connect(&_availableList, &PluginViewList::itemDoubleClicked,
            this, &PluginsAdminDlg::onItemDoubleClicked);
    connect(&_updateList, &PluginViewList::itemSelectionChanged,
            this, &PluginsAdminDlg::onItemSelectionChanged);
    connect(&_installedList, &PluginViewList::itemSelectionChanged,
            this, &PluginsAdminDlg::onItemSelectionChanged);
    connect(&_incompatibleList, &PluginViewList::itemSelectionChanged,
            this, &PluginsAdminDlg::onItemSelectionChanged);
}

void PluginsAdminDlg::onTabChanged(int index)
{
    Q_UNUSED(index)
    updateButtonStates();

    // Apply current search filter to the new tab's list
    QString searchText = _searchEdit->text();
    if (!searchText.isEmpty()) {
        onSearchTextChanged(searchText);
    }
}

void PluginsAdminDlg::onSearchTextChanged(const QString& text)
{
    _lastSearchText = text;

    PluginViewList* currentList = getCurrentList();
    if (currentList) {
        currentList->filterItems(text, _searchInNames, _searchInDescs);
    }
}

void PluginsAdminDlg::onSearchNextClicked()
{
    searchInPlugins(true);
}

void PluginsAdminDlg::onInstallClicked()
{
    installPlugins();
}

void PluginsAdminDlg::onUpdateClicked()
{
    updatePlugins();
}

void PluginsAdminDlg::onRemoveClicked()
{
    removePlugins();
}

void PluginsAdminDlg::onCloseClicked()
{
    hide();
}

void PluginsAdminDlg::onItemSelectionChanged(int index)
{
    Q_UNUSED(index)
    updateButtonStates();
}

void PluginsAdminDlg::onItemDoubleClicked(int index)
{
    Q_UNUSED(index)
    // Show plugin details or toggle selection
}

void PluginsAdminDlg::updateButtonStates()
{
    int currentTab = _tabWidget->currentIndex();

    // Enable/disable buttons based on current tab
    _installButton->setEnabled(currentTab == AVAILABLE_LIST);
    _updateButton->setEnabled(currentTab == UPDATES_LIST);
    _removeButton->setEnabled(currentTab == INSTALLED_LIST);

    // Check if items are selected/checked for additional button state logic
    PluginViewList* currentList = getCurrentList();
    if (currentList) {
        auto checked = currentList->getCheckedIndexes();
        bool hasChecked = !checked.empty();

        switch (currentTab) {
            case AVAILABLE_LIST:
                _installButton->setEnabled(hasChecked);
                break;
            case UPDATES_LIST:
                _updateButton->setEnabled(hasChecked);
                break;
            case INSTALLED_LIST:
                _removeButton->setEnabled(hasChecked);
                break;
            default:
                break;
        }
    }
}

PluginViewList* PluginsAdminDlg::getCurrentList()
{
    switch (_tabWidget->currentIndex()) {
        case AVAILABLE_LIST:
            return &_availableList;
        case UPDATES_LIST:
            return &_updateList;
        case INSTALLED_LIST:
            return &_installedList;
        case INCOMPATIBLE_LIST:
            return &_incompatibleList;
        default:
            return nullptr;
    }
}

const PluginViewList* PluginsAdminDlg::getCurrentList() const
{
    switch (_tabWidget->currentIndex()) {
        case AVAILABLE_LIST:
            return &_availableList;
        case UPDATES_LIST:
            return &_updateList;
        case INSTALLED_LIST:
            return &_installedList;
        case INCOMPATIBLE_LIST:
            return &_incompatibleList;
        default:
            return nullptr;
    }
}

bool PluginsAdminDlg::initFromJson()
{
    // TODO: Load plugin list from JSON file
    // This is a placeholder implementation

    // Example JSON parsing structure:
    // QFile file(QString::fromStdWString(_pluginListFullPath));
    // if (!file.open(QIODevice::ReadOnly)) return false;
    //
    // QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    // QJsonObject root = doc.object();
    // QJsonArray plugins = root["plugins"].toArray();
    //
    // for (const auto& plugin : plugins) {
    //     QJsonObject obj = plugin.toObject();
    //     auto* pi = new PluginUpdateInfo();
    //     pi->_folderName = obj["folderName"].toString().toStdWString();
    //     pi->_displayName = obj["displayName"].toString().toStdWString();
    //     // ... parse other fields
    //     _availableList.pushBack(pi);
    // }

    return true;
}

void PluginsAdminDlg::switchDialog(int indexToSwitch)
{
    if (indexToSwitch >= 0 && indexToSwitch < _tabWidget->count()) {
        _tabWidget->setCurrentIndex(indexToSwitch);
    }
}

void PluginsAdminDlg::setPluginsManager(PluginsManager* pluginsManager)
{
    _pPluginsManager = pluginsManager;
}

bool PluginsAdminDlg::updateList()
{
    // Clear existing lists
    _availableList.clear();
    _updateList.clear();
    _installedList.clear();
    _incompatibleList.clear();

    // Load from JSON
    if (!initFromJson()) {
        return false;
    }

    // Load installed plugins
    loadFromPluginInfos();

    // Check for updates
    checkUpdates();

    // Build incompatible list
    initIncompatiblePluginList();

    return true;
}

void PluginsAdminDlg::setAdminMode(bool isAdm)
{
    if (_nppCurrentStatus) {
        _nppCurrentStatus->_isAdminMode = isAdm;
    }
}

bool PluginsAdminDlg::installPlugins()
{
    auto checked = _availableList.getCheckedPlugins();
    if (checked.empty()) {
        QMessageBox::information(this, tr("No Plugins Selected"),
                                 tr("Please select plugins to install."));
        return false;
    }

    // TODO: Implement installation logic
    // This would typically involve:
    // 1. Downloading the plugin package
    // 2. Extracting to the plugins directory
    // 3. Refreshing the installed list

    return exitToInstallRemovePlugins(pa_install, checked);
}

bool PluginsAdminDlg::updatePlugins()
{
    auto checked = _updateList.getCheckedPlugins();
    if (checked.empty()) {
        QMessageBox::information(this, tr("No Plugins Selected"),
                                 tr("Please select plugins to update."));
        return false;
    }

    // TODO: Implement update logic
    return exitToInstallRemovePlugins(pa_update, checked);
}

bool PluginsAdminDlg::removePlugins()
{
    auto checked = _installedList.getCheckedPlugins();
    if (checked.empty()) {
        QMessageBox::information(this, tr("No Plugins Selected"),
                                 tr("Please select plugins to remove."));
        return false;
    }

    // Confirm removal
    auto reply = QMessageBox::question(this, tr("Confirm Removal"),
                                       tr("Are you sure you want to remove the selected plugins?"),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return false;
    }

    // TODO: Implement removal logic
    return exitToInstallRemovePlugins(pa_remove, checked);
}

void PluginsAdminDlg::changeTabName(LIST_TYPE index, const wchar_t* name2change)
{
    if (!name2change) return;

    QString name = QString::fromWCharArray(name2change);
    int tabIndex = -1;

    switch (index) {
        case AVAILABLE_LIST:
            tabIndex = 0;
            break;
        case UPDATES_LIST:
            tabIndex = 1;
            break;
        case INSTALLED_LIST:
            tabIndex = 2;
            break;
        case INCOMPATIBLE_LIST:
            tabIndex = 3;
            break;
        default:
            return;
    }

    if (tabIndex >= 0) {
        _tabWidget->setTabText(tabIndex, name);
    }
}

void PluginsAdminDlg::changeColumnName(COLUMN_TYPE index, const wchar_t* name2change)
{
    _availableList.changeColumnName(index, name2change);
    _updateList.changeColumnName(index, name2change);
    _installedList.changeColumnName(index, name2change);
    _incompatibleList.changeColumnName(index, name2change);
}

std::wstring PluginsAdminDlg::getPluginListVerStr() const
{
    return _pluginListVersion;
}

void PluginsAdminDlg::collectNppCurrentStatusInfos()
{
    // TODO: Collect Notepad++ current status information
    // This includes:
    // - Admin mode status
    // - Installation path (Program Files vs user directory)
    // - AppData plugins allowed
}

bool PluginsAdminDlg::searchInPlugins(bool isNextMode)
{
    Q_UNUSED(isNextMode)

    QString searchText = _searchEdit->text();
    if (searchText.isEmpty()) {
        return false;
    }

    PluginViewList* currentList = getCurrentList();
    if (!currentList) {
        return false;
    }

    // Search in names first, then descriptions
    long result = searchInNamesFromCurrentSel(*currentList, searchText.toStdWString(), isNextMode);
    if (result == -1 && _searchInDescs) {
        result = searchInDescsFromCurrentSel(*currentList, searchText.toStdWString(), isNextMode);
    }

    return result != -1;
}

bool PluginsAdminDlg::isFoundInListFromIndex(const PluginViewList& inWhichList, int index,
                                              const std::wstring& str2search, bool inWhichPart) const
{
    PluginUpdateInfo* pi = inWhichList.getPluginInfoFromUiIndex(static_cast<size_t>(index));
    if (!pi) return false;

    QString search = QString::fromStdWString(str2search).toLower();

    if (inWhichPart) {  // Search in names
        QString name = QString::fromStdWString(pi->_displayName).toLower();
        return name.contains(search);
    } else {  // Search in descriptions
        QString desc = QString::fromStdWString(pi->_description).toLower();
        return desc.contains(search);
    }
}

long PluginsAdminDlg::searchFromCurrentSel(const PluginViewList& inWhichList,
                                            const std::wstring& str2search,
                                            bool inWhichPart, bool isNextMode) const
{
    Q_UNUSED(isNextMode)

    long startIndex = inWhichList.getSelectedIndex();
    if (startIndex < 0) startIndex = 0;

    size_t count = inWhichList.nbItem();
    for (size_t i = 0; i < count; ++i) {
        long index = static_cast<long>((startIndex + i + 1) % count);
        if (isFoundInListFromIndex(inWhichList, index, str2search, inWhichPart)) {
            const_cast<PluginViewList&>(inWhichList).setSelection(index);
            return index;
        }
    }

    return -1;
}

long PluginsAdminDlg::searchInNamesFromCurrentSel(const PluginViewList& inWhichList,
                                                   const std::wstring& str2search,
                                                   bool isNextMode) const
{
    return searchFromCurrentSel(inWhichList, str2search, true, isNextMode);
}

long PluginsAdminDlg::searchInDescsFromCurrentSel(const PluginViewList& inWhichList,
                                                   const std::wstring& str2search,
                                                   bool isNextMode) const
{
    return searchFromCurrentSel(inWhichList, str2search, false, isNextMode);
}

bool PluginsAdminDlg::initAvailablePluginsViewFromList()
{
    // Initialize available plugins view from loaded list
    return true;
}

bool PluginsAdminDlg::initIncompatiblePluginList()
{
    // Build list of incompatible plugins based on version checks
    _incompatibleList.clear();

    // TODO: Check each plugin's compatibility with current Notepad++ version
    // and add incompatible ones to _incompatibleList

    return true;
}

bool PluginsAdminDlg::loadFromPluginInfos()
{
    // Load installed plugins from PluginsManager
    if (!_pPluginsManager) {
        return false;
    }

    _installedList.clear();

    // TODO: Iterate through loaded plugins and add to _installedList
    // This would involve getting the list from _pPluginsManager

    return true;
}

bool PluginsAdminDlg::checkUpdates()
{
    // Check for available updates by comparing installed versions
    // with available versions
    _updateList.clear();

    // TODO: Compare versions and populate _updateList

    return true;
}

bool PluginsAdminDlg::exitToInstallRemovePlugins(Operation op, const std::vector<PluginUpdateInfo*>& puis)
{
    Q_UNUSED(op)
    Q_UNUSED(puis)

    // TODO: Implement the actual installation/update/removal process
    // This may involve:
    // 1. Creating a batch script or external process
    // 2. Closing Notepad++
    // 3. Performing the operations
    // 4. Restarting Notepad++

    return true;
}

bool PluginsAdminDlg::run_dlgProc(QEvent* event)
{
    Q_UNUSED(event)
    return true;
}

void PluginsAdminDlg::resizeEvent(QResizeEvent* event)
{
    StaticDialog::resizeEvent(event);

    // Adjust column widths on resize
    if (_availableList.getTableWidget()) {
        _availableList.getTableWidget()->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    }
}

} // namespace QtControls
