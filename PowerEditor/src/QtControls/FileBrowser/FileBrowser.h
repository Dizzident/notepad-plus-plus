// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../TreeView/TreeView.h"
#include "../../ScintillaComponent/ScintillaEditView.h"

#include <QFileSystemWatcher>
#include <QFileIconProvider>
#include <QDir>
#include <QFileInfo>
#include <map>
#include <memory>

// Forward declarations
class QLineEdit;
class QToolBar;
class QCheckBox;
class QPushButton;
class QMenu;
class QAction;
class QLabel;

namespace QtControls {

// ============================================================================
// FileBrowser - Dockable panel for browsing the file system
// ============================================================================
class FileBrowser : public StaticDialog {
    Q_OBJECT

public:
    explicit FileBrowser(QWidget* parent = nullptr);
    ~FileBrowser() override;

    void init(ScintillaEditView** ppEditView);
    void doDialog();

    // Event processing override
    bool run_dlgProc(QEvent* event) override;

    // Bring getDialog into public scope for getWidget
    using StaticDialog::getDialog;

    // Get widget for docking integration
    QWidget* getWidget() const { return getDialog(); }

    void setRootPath(const QString& path);
    QString getRootPath() const { return _rootPath; }
    void navigateToFile(const QString& filePath);

    // Add a folder to the workspace (similar to Windows "Folder as Workspace")
    void addRootFolder(const QString& folderPath);
    void removeAllRootFolders();

    // Select/locate current file in tree
    bool selectCurrentEditingFile();
    bool selectItemFromPath(const QString& itemPath);

public slots:
    void onItemExpanded(int itemId);
    void onItemCollapsed(int itemId);
    void onItemDoubleClicked(int itemId, int column);
    void onContextMenu(const QPoint& pos);
    void onNavigateUp();
    void onRefresh();
    void onGoToCurrentFile();
    void onShowHiddenToggled(bool show);
    void onFollowCurrentToggled(bool follow);
    void onFilterChanged(const QString& filter);
    void onAddRootFolder();
    void onRemoveRootFolder();
    void onCollapseAll();
    void onExpandAll();

    // Context menu actions
    void onOpenFile();
    void onOpenContainingFolder();
    void onCopyPath();
    void onCopyFileName();
    void onDeleteFile();
    void onRenameFile();
    void onFindInFiles();

protected:
    void setupUI();
    void connectSignals();
    void resizeEvent(QResizeEvent* event) override;

private:
    // Tree population
    void populateTree(const QString& path, int parentId = -1);
    void addDirectoryToTree(const QString& dirPath, int parentId);
    void addFileToTree(const QFileInfo& fileInfo, int parentId);
    void refreshTree();
    void clearTree();

    // File operations
    void openFile(const QString& filePath);
    void openContainingFolder(const QString& filePath);
    bool deleteFile(const QString& filePath);
    bool renameFile(const QString& oldPath, const QString& newName);

    // Path utilities
    QString getNodePath(int itemId) const;
    QString getNodeName(int itemId) const;
    bool isDirectory(int itemId) const;
    int findRootForPath(const QString& path) const;

    // Icons
    QIcon getFileIcon(const QString& filePath) const;
    QIcon getDirectoryIcon() const;
    void setupIcons();

    // Filtering
    bool matchesFilter(const QString& fileName) const;
    void applyFilter();

    // Directory watching
    void setupDirectoryWatcher();
    void onDirectoryChanged(const QString& path);

    // Lazy loading
    void populateDirectoryNode(int itemId);
    bool isNodePopulated(int itemId) const;

    // Status
    void updateStatusLabel();

    // UI Components
    TreeView* _treeView = nullptr;
    QLineEdit* _pathEdit = nullptr;
    QLineEdit* _filterEdit = nullptr;
    QToolBar* _toolbar = nullptr;
    QCheckBox* _showHiddenCheck = nullptr;
    QCheckBox* _followCurrentCheck = nullptr;
    QLabel* _statusLabel = nullptr;

    // Toolbar actions
    QAction* _navigateUpAction = nullptr;
    QAction* _refreshAction = nullptr;
    QAction* _goToCurrentAction = nullptr;
    QAction* _addFolderAction = nullptr;
    QAction* _removeFolderAction = nullptr;
    QAction* _collapseAllAction = nullptr;
    QAction* _expandAllAction = nullptr;

    // Context menu
    QMenu* _contextMenu = nullptr;
    QMenu* _rootContextMenu = nullptr;
    QAction* _openAction = nullptr;
    QAction* _openFolderAction = nullptr;
    QAction* _copyPathAction = nullptr;
    QAction* _copyNameAction = nullptr;
    QAction* _deleteAction = nullptr;
    QAction* _renameAction = nullptr;
    QAction* _findInFilesAction = nullptr;

    // State
    QString _rootPath;
    std::map<int, QString> _itemPaths;      // itemId -> full path
    std::map<int, bool> _itemIsDirectory;   // itemId -> is directory
    std::map<QString, int> _pathToItemId;   // path -> itemId (for roots)
    ScintillaEditView** _ppEditView = nullptr;

    // File system
    QFileSystemWatcher* _watcher = nullptr;
    QFileIconProvider _iconProvider;
    mutable QIcon _folderOpenIcon;
    mutable QIcon _folderClosedIcon;
    mutable QIcon _fileIcon;

    // Settings
    bool _showHiddenFiles = false;
    bool _followCurrentDocument = false;
    QString _currentFilter;

    // Context menu state
    int _contextMenuItemId = -1;
};

} // namespace QtControls
