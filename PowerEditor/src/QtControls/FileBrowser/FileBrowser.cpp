// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "FileBrowser.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtGui/QAction>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QApplication>
#include <QtGui/QDesktopServices>
#include <QtGui/QClipboard>
#include <QtCore/QUrl>
#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>

namespace QtControls {

// ============================================================================
// Constructor/Destructor
// ============================================================================

FileBrowser::FileBrowser(QWidget* parent)
    : StaticDialog(parent)
{
    setWindowTitle(tr("Folder as Workspace"));
    resize(300, 500);
}

FileBrowser::~FileBrowser()
{
    if (_watcher) {
        delete _watcher;
    }
}

// ============================================================================
// Initialization
// ============================================================================

void FileBrowser::init(ScintillaEditView** ppEditView)
{
    _ppEditView = ppEditView;
    setupUI();
    setupIcons();
    connectSignals();
    setupDirectoryWatcher();
}

void FileBrowser::doDialog()
{
    display(true);
    raise();
    activateWindow();
}

bool FileBrowser::run_dlgProc(QEvent* event)
{
    (void)event;
    return false;
}

void FileBrowser::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // Toolbar
    _toolbar = new QToolBar(this);
    _toolbar->setIconSize(QSize(16, 16));

    _navigateUpAction = _toolbar->addAction(tr("Up"));
    _navigateUpAction->setToolTip(tr("Navigate to parent directory"));

    _refreshAction = _toolbar->addAction(tr("Refresh"));
    _refreshAction->setToolTip(tr("Refresh file list"));

    _goToCurrentAction = _toolbar->addAction(tr("Locate"));
    _goToCurrentAction->setToolTip(tr("Locate current file in tree"));

    _toolbar->addSeparator();

    _addFolderAction = _toolbar->addAction(tr("Add"));
    _addFolderAction->setToolTip(tr("Add folder to workspace"));

    _removeFolderAction = _toolbar->addAction(tr("Remove"));
    _removeFolderAction->setToolTip(tr("Remove folder from workspace"));

    _toolbar->addSeparator();

    _collapseAllAction = _toolbar->addAction(tr("Collapse"));
    _collapseAllAction->setToolTip(tr("Collapse all folders"));

    _expandAllAction = _toolbar->addAction(tr("Expand"));
    _expandAllAction->setToolTip(tr("Expand all folders"));

    mainLayout->addWidget(_toolbar);

    // Path display
    _pathEdit = new QLineEdit(this);
    _pathEdit->setReadOnly(true);
    _pathEdit->setPlaceholderText(tr("No folder selected"));
    mainLayout->addWidget(_pathEdit);

    // Filter
    auto* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel(tr("Filter:"), this));
    _filterEdit = new QLineEdit(this);
    _filterEdit->setPlaceholderText(tr("e.g. *.cpp *.h"));
    filterLayout->addWidget(_filterEdit);
    mainLayout->addLayout(filterLayout);

    // Options
    auto* optionsLayout = new QHBoxLayout();
    _showHiddenCheck = new QCheckBox(tr("Show hidden"), this);
    optionsLayout->addWidget(_showHiddenCheck);

    _followCurrentCheck = new QCheckBox(tr("Follow current"), this);
    _followCurrentCheck->setToolTip(tr("Auto-navigate to current document"));
    optionsLayout->addWidget(_followCurrentCheck);

    optionsLayout->addStretch();
    mainLayout->addLayout(optionsLayout);

    // Tree view
    _treeView = new TreeView();
    _treeView->init(this);
    _treeView->setColumnCount(1);
    _treeView->setSortingEnabled(true);
    mainLayout->addWidget(_treeView->getWidget(), 1);

    // Status label
    _statusLabel = new QLabel(this);
    _statusLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mainLayout->addWidget(_statusLabel);

    // Setup context menus
    _contextMenu = new QMenu(this);
    _openAction = _contextMenu->addAction(tr("Open"));
    _contextMenu->addSeparator();
    _openFolderAction = _contextMenu->addAction(tr("Open Containing Folder"));
    _contextMenu->addSeparator();
    _copyPathAction = _contextMenu->addAction(tr("Copy Path"));
    _copyNameAction = _contextMenu->addAction(tr("Copy File Name"));
    _contextMenu->addSeparator();
    _findInFilesAction = _contextMenu->addAction(tr("Find in Files..."));
    _contextMenu->addSeparator();
    _renameAction = _contextMenu->addAction(tr("Rename..."));
    _deleteAction = _contextMenu->addAction(tr("Delete"));

    _rootContextMenu = new QMenu(this);
    _rootContextMenu->addAction(tr("Remove from Workspace"), this, &FileBrowser::onRemoveRootFolder);
    _rootContextMenu->addSeparator();
    _rootContextMenu->addAction(tr("Open Containing Folder"), this, &FileBrowser::onOpenContainingFolder);
    _rootContextMenu->addSeparator();
    _rootContextMenu->addAction(tr("Copy Path"), this, &FileBrowser::onCopyPath);
    _rootContextMenu->addAction(tr("Find in Files..."), this, &FileBrowser::onFindInFiles);

    updateStatusLabel();
}

void FileBrowser::setupIcons()
{
    _folderOpenIcon = _iconProvider.icon(QFileIconProvider::Folder);
    _folderClosedIcon = _iconProvider.icon(QFileIconProvider::Folder);
    _fileIcon = _iconProvider.icon(QFileIconProvider::File);
}

void FileBrowser::connectSignals()
{
    // Toolbar actions
    connect(_navigateUpAction, &QAction::triggered, this, &FileBrowser::onNavigateUp);
    connect(_refreshAction, &QAction::triggered, this, &FileBrowser::onRefresh);
    connect(_goToCurrentAction, &QAction::triggered, this, &FileBrowser::onGoToCurrentFile);
    connect(_addFolderAction, &QAction::triggered, this, &FileBrowser::onAddRootFolder);
    connect(_removeFolderAction, &QAction::triggered, this, &FileBrowser::onRemoveRootFolder);
    connect(_collapseAllAction, &QAction::triggered, this, &FileBrowser::onCollapseAll);
    connect(_expandAllAction, &QAction::triggered, this, &FileBrowser::onExpandAll);

    // Tree view signals
    connect(_treeView, &TreeView::itemExpanded, this, &FileBrowser::onItemExpanded);
    connect(_treeView, &TreeView::itemCollapsed, this, &FileBrowser::onItemCollapsed);
    connect(_treeView, &TreeView::itemDoubleClicked, this, &FileBrowser::onItemDoubleClicked);

    // Context menu
    _treeView->getWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_treeView->getWidget(), &QWidget::customContextMenuRequested,
            this, &FileBrowser::onContextMenu);

    // Context menu actions
    connect(_openAction, &QAction::triggered, this, &FileBrowser::onOpenFile);
    connect(_openFolderAction, &QAction::triggered, this, &FileBrowser::onOpenContainingFolder);
    connect(_copyPathAction, &QAction::triggered, this, &FileBrowser::onCopyPath);
    connect(_copyNameAction, &QAction::triggered, this, &FileBrowser::onCopyFileName);
    connect(_deleteAction, &QAction::triggered, this, &FileBrowser::onDeleteFile);
    connect(_renameAction, &QAction::triggered, this, &FileBrowser::onRenameFile);
    connect(_findInFilesAction, &QAction::triggered, this, &FileBrowser::onFindInFiles);

    // Options
    connect(_showHiddenCheck, &QCheckBox::toggled, this, &FileBrowser::onShowHiddenToggled);
    connect(_followCurrentCheck, &QCheckBox::toggled, this, &FileBrowser::onFollowCurrentToggled);
    connect(_filterEdit, &QLineEdit::textChanged, this, &FileBrowser::onFilterChanged);
}

void FileBrowser::setupDirectoryWatcher()
{
    _watcher = new QFileSystemWatcher(this);
    connect(_watcher, &QFileSystemWatcher::directoryChanged,
            this, &FileBrowser::onDirectoryChanged);
}

// ============================================================================
// Tree Population
// ============================================================================

void FileBrowser::setRootPath(const QString& path)
{
    if (path.isEmpty() || !QDir(path).exists()) {
        return;
    }

    _rootPath = path;
    _pathEdit->setText(_rootPath);

    refreshTree();

    // Watch the root directory
    if (_watcher) {
        _watcher->addPath(_rootPath);
    }
}

void FileBrowser::addRootFolder(const QString& folderPath)
{
    if (folderPath.isEmpty() || !QDir(folderPath).exists()) {
        return;
    }

    // Check if already added
    auto it = _pathToItemId.find(folderPath);
    if (it != _pathToItemId.end()) {
        // Already exists, select it
        _treeView->setSelectedItem(it->second);
        return;
    }

    QFileInfo dirInfo(folderPath);
    QString displayName = dirInfo.fileName();
    if (displayName.isEmpty()) {
        displayName = folderPath;
    }

    // Add to tree as root item
    int itemId = _treeView->addItem(displayName, -1);
    _treeView->setItemIcon(itemId, _folderClosedIcon);
    _treeView->setItemData(itemId, folderPath);

    _itemPaths[itemId] = folderPath;
    _itemIsDirectory[itemId] = true;
    _pathToItemId[folderPath] = itemId;

    // Populate the directory
    populateDirectoryNode(itemId);

    // Expand the root
    _treeView->expand(itemId);

    // Watch this directory
    if (_watcher) {
        _watcher->addPath(folderPath);
    }

    updateStatusLabel();
}

void FileBrowser::removeAllRootFolders()
{
    _treeView->clear();
    _itemPaths.clear();
    _itemIsDirectory.clear();
    _pathToItemId.clear();

    if (_watcher) {
        _watcher->removePaths(_watcher->directories());
    }

    updateStatusLabel();
}

void FileBrowser::populateTree(const QString& path, int parentId)
{
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    // Set filter options
    QDir::Filters filters = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot;
    if (_showHiddenFiles) {
        filters |= QDir::Hidden;
    }

    // Get entries sorted (directories first, then files)
    dir.setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

    QFileInfoList entries = dir.entryInfoList(filters);

    for (const QFileInfo& info : entries) {
        if (info.isDir()) {
            addDirectoryToTree(info.filePath(), parentId);
        } else {
            addFileToTree(info, parentId);
        }
    }
}

void FileBrowser::addDirectoryToTree(const QString& dirPath, int parentId)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        return;
    }

    QString dirName = dir.dirName();
    if (dirName.isEmpty()) {
        dirName = dirPath;
    }

    int itemId = _treeView->addItem(dirName, parentId);
    _treeView->setItemIcon(itemId, _folderClosedIcon);
    _treeView->setItemData(itemId, dirPath);

    _itemPaths[itemId] = dirPath;
    _itemIsDirectory[itemId] = true;

    // Add a placeholder child to show the expand arrow
    // The actual contents will be loaded when expanded
    QDir subDir(dirPath);
    QDir::Filters filters = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot;
    if (_showHiddenFiles) {
        filters |= QDir::Hidden;
    }

    if (subDir.entryList(filters).isEmpty()) {
        // Empty directory - no placeholder needed
    }
}

void FileBrowser::addFileToTree(const QFileInfo& fileInfo, int parentId)
{
    // Check filter
    if (!matchesFilter(fileInfo.fileName())) {
        return;
    }

    int itemId = _treeView->addItem(fileInfo.fileName(), parentId);
    _treeView->setItemIcon(itemId, getFileIcon(fileInfo.filePath()));
    _treeView->setItemData(itemId, fileInfo.filePath());

    _itemPaths[itemId] = fileInfo.filePath();
    _itemIsDirectory[itemId] = false;
}

void FileBrowser::populateDirectoryNode(int itemId)
{
    if (!isDirectory(itemId)) {
        return;
    }

    QString path = getNodePath(itemId);
    if (path.isEmpty()) {
        return;
    }

    // Check if already populated by looking for children
    int childId = _treeView->getChildItem(itemId);
    if (childId >= 0) {
        // Already has children, might be populated
        // Check if it's just a placeholder (no path data)
        if (_itemPaths.find(childId) == _itemPaths.end()) {
            // Remove placeholder and populate
            _treeView->removeItem(childId);
        } else {
            // Already populated
            return;
        }
    }

    // Populate the directory
    populateTree(path, itemId);

    // Watch this directory
    if (_watcher && !path.isEmpty()) {
        _watcher->addPath(path);
    }
}

bool FileBrowser::isNodePopulated(int itemId) const
{
    int childId = _treeView->getChildItem(itemId);
    return childId >= 0 && _itemPaths.find(childId) != _itemPaths.end();
}

void FileBrowser::refreshTree()
{
    _treeView->clear();
    _itemPaths.clear();
    _itemIsDirectory.clear();

    if (!_rootPath.isEmpty()) {
        populateTree(_rootPath);
    }
}

void FileBrowser::clearTree()
{
    _treeView->clear();
    _itemPaths.clear();
    _itemIsDirectory.clear();
}

// ============================================================================
// Event Handlers
// ============================================================================

void FileBrowser::onItemExpanded(int itemId)
{
    _treeView->setItemIcon(itemId, _folderOpenIcon);

    // Lazy load directory contents
    if (isDirectory(itemId)) {
        populateDirectoryNode(itemId);
    }
}

void FileBrowser::onItemCollapsed(int itemId)
{
    _treeView->setItemIcon(itemId, _folderClosedIcon);
}

void FileBrowser::onItemDoubleClicked(int itemId, int column)
{
    (void)column;

    QString path = getNodePath(itemId);
    if (path.isEmpty()) {
        return;
    }

    if (isDirectory(itemId)) {
        // Toggle expansion
        if (_treeView->isExpanded(itemId)) {
            _treeView->collapse(itemId);
        } else {
            _treeView->expand(itemId);
        }
    } else {
        // Open file
        openFile(path);
    }
}

void FileBrowser::onContextMenu(const QPoint& pos)
{
    int itemId = _treeView->getSelectedItem();
    if (itemId < 0) {
        // Clicked on empty area - show root context menu if we have roots
        if (!_pathToItemId.empty()) {
            _rootContextMenu->exec(_treeView->getWidget()->mapToGlobal(pos));
        }
        return;
    }

    _contextMenuItemId = itemId;

    // Determine if this is a root item
    QString path = getNodePath(itemId);
    bool isRoot = _pathToItemId.find(path) != _pathToItemId.end();
    bool isDir = isDirectory(itemId);

    // Configure menu based on item type
    _openAction->setVisible(!isDir);
    _findInFilesAction->setVisible(isDir || isRoot);

    if (isRoot) {
        _rootContextMenu->exec(_treeView->getWidget()->mapToGlobal(pos));
    } else {
        _contextMenu->exec(_treeView->getWidget()->mapToGlobal(pos));
    }
}

void FileBrowser::onNavigateUp()
{
    int currentItem = _treeView->getSelectedItem();
    if (currentItem < 0) {
        return;
    }

    int parentId = _treeView->getParentItem(currentItem);
    if (parentId >= 0) {
        _treeView->setSelectedItem(parentId);
    }
}

void FileBrowser::onRefresh()
{
    refreshTree();
}

void FileBrowser::onGoToCurrentFile()
{
    selectCurrentEditingFile();
}

void FileBrowser::onShowHiddenToggled(bool show)
{
    _showHiddenFiles = show;
    refreshTree();
}

void FileBrowser::onFollowCurrentToggled(bool follow)
{
    _followCurrentDocument = follow;
}

void FileBrowser::onFilterChanged(const QString& filter)
{
    _currentFilter = filter.trimmed();
    applyFilter();
}

void FileBrowser::onAddRootFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"),
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty()) {
        addRootFolder(dir);
    }
}

void FileBrowser::onRemoveRootFolder()
{
    int itemId = _treeView->getSelectedItem();
    if (itemId < 0) {
        return;
    }

    QString path = getNodePath(itemId);
    auto it = _pathToItemId.find(path);
    if (it != _pathToItemId.end()) {
        // Remove from watcher
        if (_watcher) {
            _watcher->removePath(path);
        }

        // Remove from tree
        _treeView->removeItem(itemId);

        // Clean up maps
        _pathToItemId.erase(it);

        // Remove all children from maps
        std::vector<int> toRemove;
        for (const auto& pair : _itemPaths) {
            if (pair.second.startsWith(path + "/") || pair.second.startsWith(path + "\\")) {
                toRemove.push_back(pair.first);
            }
        }
        for (int id : toRemove) {
            _itemPaths.erase(id);
            _itemIsDirectory.erase(id);
        }
        _itemPaths.erase(itemId);
        _itemIsDirectory.erase(itemId);

        updateStatusLabel();
    }
}

void FileBrowser::onCollapseAll()
{
    _treeView->collapseAll();
}

void FileBrowser::onExpandAll()
{
    _treeView->expandAll();
}

// ============================================================================
// Context Menu Actions
// ============================================================================

void FileBrowser::onOpenFile()
{
    QString path = getNodePath(_contextMenuItemId);
    if (!path.isEmpty() && !isDirectory(_contextMenuItemId)) {
        openFile(path);
    }
}

void FileBrowser::onOpenContainingFolder()
{
    QString path = getNodePath(_contextMenuItemId);
    if (path.isEmpty()) {
        return;
    }

    openContainingFolder(path);
}

void FileBrowser::onCopyPath()
{
    QString path = getNodePath(_contextMenuItemId);
    if (!path.isEmpty()) {
        QApplication::clipboard()->setText(path);
    }
}

void FileBrowser::onCopyFileName()
{
    QString name = getNodeName(_contextMenuItemId);
    if (!name.isEmpty()) {
        QApplication::clipboard()->setText(name);
    }
}

void FileBrowser::onDeleteFile()
{
    QString path = getNodePath(_contextMenuItemId);
    if (path.isEmpty()) {
        return;
    }

    QString itemType = isDirectory(_contextMenuItemId) ? tr("folder") : tr("file");
    QString name = getNodeName(_contextMenuItemId);

    int ret = QMessageBox::question(this, tr("Confirm Delete"),
                                    tr("Are you sure you want to delete %1 '%2'?")
                                        .arg(itemType, name),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (deleteFile(path)) {
            // Remove from tree
            _treeView->removeItem(_contextMenuItemId);
            _itemPaths.erase(_contextMenuItemId);
            _itemIsDirectory.erase(_contextMenuItemId);
        } else {
            QMessageBox::warning(this, tr("Delete Failed"),
                                 tr("Failed to delete %1 '%2'.")
                                     .arg(itemType, name));
        }
    }
}

void FileBrowser::onRenameFile()
{
    QString path = getNodePath(_contextMenuItemId);
    if (path.isEmpty()) {
        return;
    }

    QString oldName = getNodeName(_contextMenuItemId);
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename"),
                                            tr("New name:"),
                                            QLineEdit::Normal,
                                            oldName, &ok);

    if (ok && !newName.isEmpty() && newName != oldName) {
        if (renameFile(path, newName)) {
            // Update tree
            _treeView->setItemText(_contextMenuItemId, newName);

            // Update path
            QFileInfo newPathInfo(QFileInfo(path).dir(), newName);
            _itemPaths[_contextMenuItemId] = newPathInfo.filePath();
        } else {
            QMessageBox::warning(this, tr("Rename Failed"),
                                 tr("Failed to rename '%1' to '%2'.")
                                     .arg(oldName, newName));
        }
    }
}

void FileBrowser::onFindInFiles()
{
    QString path = getNodePath(_contextMenuItemId);
    if (!path.isEmpty()) {
        // Emit signal or call main window to open Find in Files dialog
        // For now, just a placeholder
        QMessageBox::information(this, tr("Find in Files"),
                                 tr("Find in Files for:\n%1").arg(path));
    }
}

// ============================================================================
// File Operations
// ============================================================================

void FileBrowser::openFile(const QString& filePath)
{
    if (_ppEditView && *_ppEditView) {
        // TODO: Send message to main window to open file
        // For now, emit a signal that the main window can connect to
        // This would typically use Scintilla's file opening mechanism
    }

    // Alternative: Use QDesktopServices for files we can't open internally
    QFileInfo info(filePath);
    if (info.suffix().compare("exe", Qt::CaseInsensitive) == 0 ||
        info.suffix().compare("bin", Qt::CaseInsensitive) == 0) {
        // Don't try to open executables
        return;
    }

    // Signal to main window would go here
}

void FileBrowser::openContainingFolder(const QString& filePath)
{
    QFileInfo info(filePath);
    QString dirPath = info.dir().absolutePath();

#if defined(Q_OS_WIN)
    QProcess::startDetached("explorer", QStringList() << "/select," << QDir::toNativeSeparators(filePath));
#elif defined(Q_OS_MAC)
    QProcess::startDetached("open", QStringList() << "-R" << filePath);
#else
    // Linux - open folder, can't easily select file
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
#endif
}

bool FileBrowser::deleteFile(const QString& filePath)
{
    QFileInfo info(filePath);
    if (info.isDir()) {
        QDir dir(filePath);
        return dir.removeRecursively();
    } else {
        return QFile::remove(filePath);
    }
}

bool FileBrowser::renameFile(const QString& oldPath, const QString& newName)
{
    QFileInfo info(oldPath);
    QString newPath = info.dir().absoluteFilePath(newName);

    if (info.isDir()) {
        return QDir().rename(oldPath, newPath);
    } else {
        return QFile::rename(oldPath, newPath);
    }
}

// ============================================================================
// Path Utilities
// ============================================================================

QString FileBrowser::getNodePath(int itemId) const
{
    auto it = _itemPaths.find(itemId);
    if (it != _itemPaths.end()) {
        return it->second;
    }
    return QString();
}

QString FileBrowser::getNodeName(int itemId) const
{
    return _treeView->getItemText(itemId);
}

bool FileBrowser::isDirectory(int itemId) const
{
    auto it = _itemIsDirectory.find(itemId);
    if (it != _itemIsDirectory.end()) {
        return it->second;
    }
    return false;
}

int FileBrowser::findRootForPath(const QString& path) const
{
    for (const auto& pair : _pathToItemId) {
        if (path.startsWith(pair.first + "/") ||
            path.startsWith(pair.first + "\\") ||
            path == pair.first) {
            return pair.second;
        }
    }
    return -1;
}

// ============================================================================
// Navigation
// ============================================================================

void FileBrowser::navigateToFile(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo info(filePath);
    if (!info.exists()) {
        return;
    }

    // Find which root this file belongs to
    int rootId = findRootForPath(filePath);
    if (rootId < 0) {
        return;
    }

    // Build path components
    QString rootPath = getNodePath(rootId);
    QString relativePath = filePath.mid(rootPath.length() + 1);
    QStringList components = relativePath.split(QDir::separator(), Qt::SkipEmptyParts);

    // Navigate through tree
    int currentId = rootId;
    _treeView->expand(currentId);

    for (const QString& component : components) {
        // Find child with this name
        bool found = false;
        int childId = _treeView->getChildItem(currentId);

        while (childId >= 0) {
            if (getNodeName(childId) == component) {
                currentId = childId;
                _treeView->expand(currentId);
                found = true;
                break;
            }
            childId = _treeView->getNextSibling(childId);
        }

        if (!found) {
            return; // Path component not found
        }
    }

    // Select the final item
    _treeView->setSelectedItem(currentId);
}

bool FileBrowser::selectCurrentEditingFile()
{
    if (!_ppEditView || !*_ppEditView) {
        return false;
    }

    // TODO: Get current file path from ScintillaEditView
    // This would need to be implemented based on how the edit view tracks file paths
    // For now, placeholder implementation

    return false;
}

bool FileBrowser::selectItemFromPath(const QString& itemPath)
{
    navigateToFile(itemPath);
    return true;
}

// ============================================================================
// Icons
// ============================================================================

QIcon FileBrowser::getFileIcon(const QString& filePath) const
{
    QFileInfo info(filePath);
    return _iconProvider.icon(info);
}

QIcon FileBrowser::getDirectoryIcon() const
{
    return _iconProvider.icon(QFileIconProvider::Folder);
}

// ============================================================================
// Filtering
// ============================================================================

bool FileBrowser::matchesFilter(const QString& fileName) const
{
    if (_currentFilter.isEmpty()) {
        return true;
    }

    // Parse filter patterns (space-separated)
    QStringList patterns = _currentFilter.split(' ', Qt::SkipEmptyParts);
    if (patterns.isEmpty()) {
        return true;
    }

    for (const QString& pattern : patterns) {
        QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(pattern),
                                 QRegularExpression::CaseInsensitiveOption);
        if (regex.match(fileName).hasMatch()) {
            return true;
        }
    }

    return false;
}

void FileBrowser::applyFilter()
{
    // Refresh tree to apply filter
    refreshTree();
}

// ============================================================================
// Directory Watching
// ============================================================================

void FileBrowser::onDirectoryChanged(const QString& path)
{
    // Find the item for this path
    for (const auto& pair : _itemPaths) {
        if (pair.second == path) {
            // Refresh this node's children
            // For now, just refresh the entire tree
            // A more optimized approach would only refresh the changed directory
            refreshTree();
            break;
        }
    }
}

// ============================================================================
// Status
// ============================================================================

void FileBrowser::updateStatusLabel()
{
    int rootCount = static_cast<int>(_pathToItemId.size());
    if (rootCount == 0) {
        _statusLabel->setText(tr("No folders in workspace"));
    } else if (rootCount == 1) {
        _statusLabel->setText(tr("1 folder in workspace"));
    } else {
        _statusLabel->setText(tr("%1 folders in workspace").arg(rootCount));
    }
}

// ============================================================================
// Resize Event
// ============================================================================

void FileBrowser::resizeEvent(QResizeEvent* event)
{
    StaticDialog::resizeEvent(event);
    // Layout is handled by QVBoxLayout, but we can adjust if needed
}

} // namespace QtControls
