// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../../ScintillaComponent/ScintillaEditView.h"
#include <memory>
#include <vector>
#include <map>

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

// Forward declarations
class QTreeWidget;
class QTreeWidgetItem;
class QToolBar;
class QAction;
class QMenu;
class QFileSystemWatcher;
class QLineEdit;
class QPushButton;

namespace QtControls {

class TreeView;

// ============================================================================
// Workspace Item Types and Structures
// ============================================================================

enum class NodeType {
    Root = 0,
    Project = 1,
    Folder = 2,
    File = 3
};

enum IconIndex {
    INDEX_CLEAN_ROOT = 0,
    INDEX_DIRTY_ROOT = 1,
    INDEX_PROJECT = 2,
    INDEX_OPEN_NODE = 3,
    INDEX_CLOSED_NODE = 4,
    INDEX_LEAF = 5,
    INDEX_LEAF_INVALID = 6
};

struct WorkspaceItem {
    enum Type { File, Folder, Root };
    Type type;
    QString name;
    QString path;
    std::vector<WorkspaceItem> children;
    bool isExpanded = false;
    bool isDirty = false;
};

// ============================================================================
// ProjectPanel - Dockable panel for managing project workspaces
// ============================================================================

class ProjectPanel : public StaticDialog {
    Q_OBJECT

public:
    explicit ProjectPanel(QWidget* parent = nullptr);
    ~ProjectPanel() override;

    // Initialize the panel
    void init(ScintillaEditView** ppEditView);

    // Show the panel
    void doDialog();

    // Event processing override
    bool run_dlgProc(QEvent* event) override;

    // Bring getDialog into public scope for getWidget
    using StaticDialog::getDialog;

    // Get widget for docking integration
    QWidget* getWidget() const { return getDialog(); }

    // Workspace operations
    void newWorkspace();
    void openWorkspace(const QString& filePath);
    void saveWorkspace();
    void saveWorkspaceAs(const QString& filePath);

    // Panel configuration
    void setPanelTitle(const QString& title) { _panelTitle = title; }
    QString getPanelTitle() const { return _panelTitle; }

    // Check if workspace has unsaved changes
    bool isDirty() const { return _isDirty; }

    // Get workspace file path
    QString getWorkspaceFilePath() const { return _workspaceFile; }

    // Set workspace file path
    void setWorkspaceFilePath(const QString& filePath) { _workspaceFile = filePath; }

    // Check if save is needed before closing
    bool checkIfNeedSave();

    // Set background/foreground colors
    void setBackgroundColor(const QColor& color);
    void setForegroundColor(const QColor& color);

public slots:
    // Tree interaction slots
    void onItemDoubleClicked(int itemId, int column);
    void onContextMenu(const QPoint& pos);
    void onItemExpanded(int itemId);
    void onItemCollapsed(int itemId);
    void onItemChanged(int itemId, int column);
    void onCurrentItemChanged(int currentItemId, int previousItemId);

    // Toolbar/menu actions
    void onNewWorkspace();
    void onOpenWorkspace();
    void onReloadWorkspace();
    void onSaveWorkspace();
    void onSaveWorkspaceAs();
    void onSaveACopyAs();
    void onNewProject();
    void onFindInProjects();

    // Edit actions
    void onNewFile();
    void onNewFolder();
    void onAddFiles();
    void onAddFilesFromDirectory();
    void onRemoveItem();
    void onRenameItem();
    void onMoveUp();
    void onMoveDown();
    void onModifyFilePath();

    // Other actions
    void onRefresh();
    void onOpenSelectedFile();

protected:
    void setupUI() override;
    void connectSignals() override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    // UI Components
    TreeView* _treeView = nullptr;
    QToolBar* _toolbar = nullptr;

    // Toolbar actions
    QAction* _workspaceAction = nullptr;
    QAction* _editAction = nullptr;

    // Context menus
    QMenu* _workspaceMenu = nullptr;
    QMenu* _projectMenu = nullptr;
    QMenu* _folderMenu = nullptr;
    QMenu* _fileMenu = nullptr;

    // State
    QString _workspaceFile;
    QString _workspaceName;
    QString _panelTitle;
    bool _isDirty = false;
    int _panelID = 0;

    // Item path mapping (maps tree item IDs to file paths)
    std::map<int, QString> _itemPaths;

    // Scintilla edit view pointer
    ScintillaEditView** _ppEditView = nullptr;

    // File system watcher for monitoring file changes
    QFileSystemWatcher* _fileWatcher = nullptr;

    // Last selected directory for file dialogs
    QString _lastSelectedDir;

    // Setup methods
    void setupToolbar();
    void setupMenus();
    void setupTreeView();

    // Tree population methods
    void populateTree();
    void buildTreeFromWorkspace(const WorkspaceItem& item, int parentId);
    int addFileToTree(const QString& filePath, int parentId = -1);
    int addFolderToTree(const QString& folderName, int parentId = -1);
    int addProjectToTree(const QString& projectName, int parentId = -1);

    // Workspace operations
    bool writeWorkspace(const QString& filePath = QString(), bool doUpdateGUI = true);
    bool readWorkspace(const QString& filePath);
    void buildProjectXml(QXmlStreamWriter& writer, int itemId);
    bool buildTreeFromXml(QXmlStreamReader& reader, int parentId);

    // File operations
    void openFile(const QString& filePath);
    QString getAbsoluteFilePath(const QString& relativePath) const;
    QString getRelativePath(const QString& filePath) const;

    // Node type detection
    NodeType getNodeType(int itemId) const;
    NodeType getNodeTypeFromIcon(int iconIndex) const;

    // Context menu helpers
    void showContextMenuAt(const QPoint& pos, int itemId);
    QMenu* getContextMenuForItem(int itemId);

    // Drag and drop helpers
    void handleDragDrop(int sourceId, int targetId);

    // Dirty state management
    void setWorkspaceDirty(bool dirty);

    // Recursive file addition
    void recursiveAddFilesFrom(const QString& folderPath, int parentId);

    // File validation
    bool doesFileExist(const QString& filePath) const;
    void updateFileIcon(int itemId, bool exists);

    // Workspace item creation
    WorkspaceItem createWorkspaceItemFromTree(int itemId) const;

    // Dialog helpers
    bool saveWorkspaceRequest();
    void setFileExtFilter(QString& filter);
};

// ============================================================================
// FileRelocalizerDlg - Dialog for modifying file paths
// ============================================================================

class FileRelocalizerDlg : public StaticDialog {
    Q_OBJECT

public:
    explicit FileRelocalizerDlg(QWidget* parent = nullptr);
    ~FileRelocalizerDlg() override;

    int doDialog(const QString& filePath, bool isRTL = false);

    QString getFullFilePath() const { return _fullFilePath; }

protected:
    void setupUI() override;
    void connectSignals() override;
    bool run_dlgProc(QEvent* event) override {
        (void)event;
        return false;
    }

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onBrowseClicked();

private:
    QString _fullFilePath;

    // UI components
    QLineEdit* _pathEdit = nullptr;
    QPushButton* _browseButton = nullptr;
    QPushButton* _okButton = nullptr;
    QPushButton* _cancelButton = nullptr;
};

} // namespace QtControls
