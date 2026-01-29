// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "ProjectPanel.h"
#include "../TreeView/TreeView.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFileSystemWatcher>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDir>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QApplication>
#include <QDebug>

namespace QtControls {

// ============================================================================
// Constants
// ============================================================================

static const char* PM_WORKSPACEROOTNAME = "Workspace";
static const char* PM_NEWFOLDERNAME = "Folder Name";
static const char* PM_NEWPROJECTNAME = "Project Name";

static const char* PM_NEWWORKSPACE = "New Workspace";
static const char* PM_OPENWORKSPACE = "Open Workspace";
static const char* PM_RELOADWORKSPACE = "Reload Workspace";
static const char* PM_SAVEWORKSPACE = "Save";
static const char* PM_SAVEASWORKSPACE = "Save As...";
static const char* PM_SAVEACOPYASWORKSPACE = "Save a Copy As...";
static const char* PM_NEWPROJECTWORKSPACE = "Add New Project";
static const char* PM_FINDINFILESWORKSPACE = "Find in Projects...";

static const char* PM_EDITRENAME = "Rename";
static const char* PM_EDITNEWFOLDER = "Add Folder";
static const char* PM_EDITADDFILES = "Add Files...";
static const char* PM_EDITADDFILESRECUSIVELY = "Add Files from Directory...";
static const char* PM_EDITREMOVE = "Remove\tDEL";
static const char* PM_EDITMODIFYFILE = "Modify File Path";

static const char* PM_WORKSPACEMENUENTRY = "Workspace";
static const char* PM_EDITMENUENTRY = "Edit";

static const char* PM_MOVEUPENTRY = "Move Up\tCtrl+Up";
static const char* PM_MOVEDOWNENTRY = "Move Down\tCtrl+Down";

// ============================================================================
// ProjectPanel Implementation
// ============================================================================

ProjectPanel::ProjectPanel(QWidget* parent)
    : StaticDialog(parent)
{
    setupUI();
    setupMenus();
    connectSignals();
}

ProjectPanel::~ProjectPanel() = default;

void ProjectPanel::init(ScintillaEditView** ppEditView)
{
    _ppEditView = ppEditView;
}

void ProjectPanel::doDialog()
{
    display(true);
}

bool ProjectPanel::run_dlgProc(QEvent* event)
{
    (void)event;
    return false;
}

void ProjectPanel::setupUI()
{
    _widget = new QWidget(this);
    auto* layout = new QVBoxLayout(_widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    // Create toolbar
    setupToolbar();
    layout->addWidget(_toolbar);

    // Create tree view
    setupTreeView();
    layout->addWidget(_treeView->getWidget(), 1);

    // Set initial size
    _widget->setMinimumSize(200, 300);

    // Initialize with empty workspace
    newWorkspace();
}

void ProjectPanel::setupToolbar()
{
    _toolbar = new QToolBar(_widget);
    _toolbar->setMovable(false);
    _toolbar->setFloatable(false);

    _workspaceAction = _toolbar->addAction(tr(PM_WORKSPACEMENUENTRY));
    _editAction = _toolbar->addAction(tr(PM_EDITMENUENTRY));

    connect(_workspaceAction, &QAction::triggered, this, &ProjectPanel::onNewWorkspace);
    connect(_editAction, &QAction::triggered, this, [this]() {
        int selectedItem = _treeView->getSelectedItem();
        if (selectedItem >= 0) {
            QPoint pos = _toolbar->mapToGlobal(QPoint(0, _toolbar->height()));
            showContextMenuAt(pos, selectedItem);
        }
    });
}

void ProjectPanel::setupMenus()
{
    // Workspace menu
    _workspaceMenu = new QMenu(_widget);
    _workspaceMenu->addAction(tr(PM_NEWWORKSPACE), this, &ProjectPanel::onNewWorkspace);
    _workspaceMenu->addAction(tr(PM_OPENWORKSPACE), this, &ProjectPanel::onOpenWorkspace);
    _workspaceMenu->addAction(tr(PM_RELOADWORKSPACE), this, &ProjectPanel::onReloadWorkspace);
    _workspaceMenu->addSeparator();
    _workspaceMenu->addAction(tr(PM_SAVEWORKSPACE), this, &ProjectPanel::onSaveWorkspace);
    _workspaceMenu->addAction(tr(PM_SAVEASWORKSPACE), this, &ProjectPanel::onSaveWorkspaceAs);
    _workspaceMenu->addAction(tr(PM_SAVEACOPYASWORKSPACE), this, &ProjectPanel::onSaveACopyAs);
    _workspaceMenu->addSeparator();
    _workspaceMenu->addAction(tr(PM_NEWPROJECTWORKSPACE), this, &ProjectPanel::onNewProject);
    _workspaceMenu->addSeparator();
    _workspaceMenu->addAction(tr(PM_FINDINFILESWORKSPACE), this, &ProjectPanel::onFindInProjects);

    // Project menu
    _projectMenu = new QMenu(_widget);
    _projectMenu->addAction(tr(PM_MOVEUPENTRY), this, &ProjectPanel::onMoveUp);
    _projectMenu->addAction(tr(PM_MOVEDOWNENTRY), this, &ProjectPanel::onMoveDown);
    _projectMenu->addSeparator();
    _projectMenu->addAction(tr(PM_EDITRENAME), this, &ProjectPanel::onRenameItem);
    _projectMenu->addAction(tr(PM_EDITNEWFOLDER), this, &ProjectPanel::onNewFolder);
    _projectMenu->addAction(tr(PM_EDITADDFILES), this, &ProjectPanel::onAddFiles);
    _projectMenu->addAction(tr(PM_EDITADDFILESRECUSIVELY), this, &ProjectPanel::onAddFilesFromDirectory);
    _projectMenu->addAction(tr(PM_EDITREMOVE), this, &ProjectPanel::onRemoveItem);

    // Folder menu
    _folderMenu = new QMenu(_widget);
    _folderMenu->addAction(tr(PM_MOVEUPENTRY), this, &ProjectPanel::onMoveUp);
    _folderMenu->addAction(tr(PM_MOVEDOWNENTRY), this, &ProjectPanel::onMoveDown);
    _folderMenu->addSeparator();
    _folderMenu->addAction(tr(PM_EDITRENAME), this, &ProjectPanel::onRenameItem);
    _folderMenu->addAction(tr(PM_EDITNEWFOLDER), this, &ProjectPanel::onNewFolder);
    _folderMenu->addAction(tr(PM_EDITADDFILES), this, &ProjectPanel::onAddFiles);
    _folderMenu->addAction(tr(PM_EDITADDFILESRECUSIVELY), this, &ProjectPanel::onAddFilesFromDirectory);
    _folderMenu->addAction(tr(PM_EDITREMOVE), this, &ProjectPanel::onRemoveItem);

    // File menu
    _fileMenu = new QMenu(_widget);
    _fileMenu->addAction(tr(PM_MOVEUPENTRY), this, &ProjectPanel::onMoveUp);
    _fileMenu->addAction(tr(PM_MOVEDOWNENTRY), this, &ProjectPanel::onMoveDown);
    _fileMenu->addSeparator();
    _fileMenu->addAction(tr(PM_EDITRENAME), this, &ProjectPanel::onRenameItem);
    _fileMenu->addAction(tr(PM_EDITREMOVE), this, &ProjectPanel::onRemoveItem);
    _fileMenu->addAction(tr(PM_EDITMODIFYFILE), this, &ProjectPanel::onModifyFilePath);
}

void ProjectPanel::setupTreeView()
{
    _treeView = new TreeView();
    _treeView->init(_widget);

    // Enable drag and drop
    _treeView->setDragEnabled(true);
    _treeView->setAcceptDrops(true);
    _treeView->setDropIndicatorShown(true);

    // Set single column
    _treeView->setColumnCount(1);
    _treeView->getTreeWidget()->header()->hide();
}

void ProjectPanel::connectSignals()
{
    connect(_treeView, &TreeView::itemDoubleClicked,
            this, &ProjectPanel::onItemDoubleClicked);
    connect(_treeView, &TreeView::itemExpanded,
            this, &ProjectPanel::onItemExpanded);
    connect(_treeView, &TreeView::itemCollapsed,
            this, &ProjectPanel::onItemCollapsed);
    connect(_treeView, &TreeView::itemChanged,
            this, &ProjectPanel::onItemChanged);
    connect(_treeView, &TreeView::currentItemChanged,
            this, &ProjectPanel::onCurrentItemChanged);

    // Enable custom context menu on tree
    _treeView->getTreeWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_treeView->getTreeWidget(), &QTreeWidget::customContextMenuRequested,
            this, &ProjectPanel::onContextMenu);
}

void ProjectPanel::resizeEvent(QResizeEvent* event)
{
    StaticDialog::resizeEvent(event);
}

void ProjectPanel::closeEvent(QCloseEvent* event)
{
    if (checkIfNeedSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

// ============================================================================
// Workspace Operations
// ============================================================================

void ProjectPanel::newWorkspace()
{
    if (_treeView) {
        _treeView->clear();
    }

    _workspaceFile.clear();
    _workspaceName = tr(PM_WORKSPACEROOTNAME);
    _panelTitle = _workspaceName;

    // Add root item
    int rootId = _treeView->addItem(_workspaceName, -1, INDEX_CLEAN_ROOT);
    _itemPaths[rootId] = QString();

    setWorkspaceDirty(false);
}

void ProjectPanel::openWorkspace(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    if (!QFile::exists(filePath)) {
        QMessageBox::warning(_widget, tr("Open Workspace"),
                             tr("The workspace file does not exist."));
        return;
    }

    if (!checkIfNeedSave()) {
        return;
    }

    if (readWorkspace(filePath)) {
        _workspaceFile = filePath;
        QFileInfo fi(filePath);
        _workspaceName = fi.fileName();
        setWorkspaceDirty(false);
    } else {
        QMessageBox::warning(_widget, tr("Open Workspace"),
                             tr("The workspace could not be opened.\n"
                                "It seems the file to open is not a valid project file."));
    }
}

void ProjectPanel::saveWorkspace()
{
    if (_workspaceFile.isEmpty()) {
        saveWorkspaceAs(QString());
    } else {
        if (writeWorkspace()) {
            setWorkspaceDirty(false);
        }
    }
}

void ProjectPanel::saveWorkspaceAs(const QString& filePath)
{
    QString path = filePath;
    if (path.isEmpty()) {
        QString filter;
        setFileExtFilter(filter);

        path = QFileDialog::getSaveFileName(_widget, tr("Save Workspace"),
                                            _workspaceFile.isEmpty() ? QString() : _workspaceFile,
                                            filter);
    }

    if (!path.isEmpty()) {
        if (writeWorkspace(path, true)) {
            _workspaceFile = path;
            QFileInfo fi(path);
            _workspaceName = fi.fileName();
            setWorkspaceDirty(false);
        }
    }
}

bool ProjectPanel::writeWorkspace(const QString& filePath, bool doUpdateGUI)
{
    QString path = filePath.isEmpty() ? _workspaceFile : filePath;
    if (path.isEmpty()) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(_widget, tr("Save Workspace"),
                              tr("An error occurred while writing your workspace file.\n"
                                 "Your workspace has not been saved."));
        return false;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);

    writer.writeStartDocument();
    writer.writeStartElement("NotepadPlus");

    // Write all projects (children of root)
    int rootId = _treeView->getRootItem();
    if (rootId >= 0) {
        int childId = _treeView->getChildItem(rootId);
        while (childId >= 0) {
            QString projectName = _treeView->getItemText(childId);
            writer.writeStartElement("Project");
            writer.writeAttribute("name", projectName);
            buildProjectXml(writer, childId);
            writer.writeEndElement();
            childId = _treeView->getNextSibling(childId);
        }
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    file.close();

    // Update root item name to match file name
    if (doUpdateGUI && !filePath.isEmpty()) {
        QFileInfo fi(filePath);
        int rootId = _treeView->getRootItem();
        if (rootId >= 0) {
            _treeView->setItemText(rootId, fi.fileName());
        }
    }

    return true;
}

bool ProjectPanel::readWorkspace(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QXmlStreamReader reader(&file);

    // Clear existing items
    _treeView->clear();
    _itemPaths.clear();

    // Add root item with file name
    QFileInfo fi(filePath);
    int rootId = _treeView->addItem(fi.fileName(), -1, INDEX_CLEAN_ROOT);
    _itemPaths[rootId] = filePath;

    bool foundNotepadPlus = false;

    while (!reader.atEnd() && !reader.hasError()) {
        reader.readNext();

        if (reader.isStartElement()) {
            if (reader.name() == QStringLiteral("NotepadPlus")) {
                foundNotepadPlus = true;
            } else if (reader.name() == QStringLiteral("Project")) {
                QString projectName = reader.attributes().value("name").toString();
                int projectId = addProjectToTree(projectName, rootId);
                buildTreeFromXml(reader, projectId);
            }
        }
    }

    file.close();

    if (reader.hasError()) {
        return false;
    }

    if (!foundNotepadPlus) {
        return false;
    }

    // Expand root
    _treeView->expand(rootId);

    return true;
}

void ProjectPanel::buildProjectXml(QXmlStreamWriter& writer, int itemId)
{
    int childId = _treeView->getChildItem(itemId);
    while (childId >= 0) {
        NodeType type = getNodeType(childId);

        if (type == NodeType::File) {
            QString filePath = _itemPaths[childId];
            QString relativePath = getRelativePath(filePath);
            writer.writeStartElement("File");
            writer.writeAttribute("name", relativePath);
            writer.writeEndElement();
        } else if (type == NodeType::Folder) {
            QString folderName = _treeView->getItemText(childId);
            writer.writeStartElement("Folder");
            writer.writeAttribute("name", folderName);
            buildProjectXml(writer, childId);
            writer.writeEndElement();
        }

        childId = _treeView->getNextSibling(childId);
    }
}

bool ProjectPanel::buildTreeFromXml(QXmlStreamReader& reader, int parentId)
{
    while (!reader.atEnd() && !reader.hasError()) {
        reader.readNext();

        if (reader.isEndElement() && reader.name() == QStringLiteral("Project")) {
            break;
        }

        if (reader.isStartElement()) {
            if (reader.name() == QStringLiteral("Folder")) {
                QString folderName = reader.attributes().value("name").toString();
                int folderId = addFolderToTree(folderName, parentId);

                // Recursively process folder contents
                while (!reader.atEnd() && !reader.hasError()) {
                    reader.readNext();

                    if (reader.isEndElement() && reader.name() == QStringLiteral("Folder")) {
                        break;
                    }

                    if (reader.isStartElement()) {
                        if (reader.name() == QStringLiteral("Folder")) {
                            // Nested folder - process recursively
                            QString nestedName = reader.attributes().value("name").toString();
                            int nestedId = addFolderToTree(nestedName, folderId);
                            buildTreeFromXml(reader, nestedId);
                        } else if (reader.name() == QStringLiteral("File")) {
                            QString filePath = reader.attributes().value("name").toString();
                            QString absolutePath = getAbsoluteFilePath(filePath);
                            addFileToTree(absolutePath, folderId);
                        }
                    }
                }
            } else if (reader.name() == QStringLiteral("File")) {
                QString filePath = reader.attributes().value("name").toString();
                QString absolutePath = getAbsoluteFilePath(filePath);
                addFileToTree(absolutePath, parentId);
            }
        }
    }

    return !reader.hasError();
}

// ============================================================================
// Tree Operations
// ============================================================================

int ProjectPanel::addFileToTree(const QString& filePath, int parentId)
{
    QFileInfo fi(filePath);
    QString fileName = fi.fileName();

    bool exists = fi.exists();
    int iconIndex = exists ? INDEX_LEAF : INDEX_LEAF_INVALID;

    int itemId = _treeView->addItem(fileName, parentId, iconIndex);
    _itemPaths[itemId] = filePath;

    // Expand parent
    if (parentId >= 0) {
        _treeView->expand(parentId);
    }

    return itemId;
}

int ProjectPanel::addFolderToTree(const QString& folderName, int parentId)
{
    int itemId = _treeView->addItem(folderName, parentId, INDEX_CLOSED_NODE);
    _itemPaths[itemId] = QString();  // Folders don't have paths stored

    // Expand parent
    if (parentId >= 0) {
        _treeView->expand(parentId);
    }

    return itemId;
}

int ProjectPanel::addProjectToTree(const QString& projectName, int parentId)
{
    int itemId = _treeView->addItem(projectName, parentId, INDEX_PROJECT);
    _itemPaths[itemId] = QString();
    return itemId;
}

void ProjectPanel::populateTree()
{
    // This is called when loading a workspace
    // The actual population is done in readWorkspace
}

NodeType ProjectPanel::getNodeType(int itemId) const
{
    if (itemId < 0) {
        return NodeType::Root;
    }

    // Check if it's root (no parent)
    int parentId = _treeView->getParentItem(itemId);
    if (parentId < 0) {
        return NodeType::Root;
    }

    // Check if it has children
    int childId = _treeView->getChildItem(itemId);

    // Check stored path - files have paths, folders don't
    auto it = _itemPaths.find(itemId);
    if (it != _itemPaths.end() && !it->second.isEmpty()) {
        // This is a file
        return NodeType::File;
    }

    // Check parent to determine if it's a project or folder
    int grandParentId = _treeView->getParentItem(parentId);
    if (grandParentId < 0) {
        // Parent is root, so this is a project
        return NodeType::Project;
    }

    return NodeType::Folder;
}

// ============================================================================
// File Operations
// ============================================================================

void ProjectPanel::openFile(const QString& filePath)
{
    if (_ppEditView && *_ppEditView) {
        // Emit signal or call method to open file in editor
        // For now, we'll use a simple approach
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString content = QString::fromUtf8(file.readAll());
            file.close();

            // Open in Scintilla
            (*_ppEditView)->execute(SCI_CLEARALL);
            (*_ppEditView)->execute(SCI_ADDTEXT, content.length(),
                                    reinterpret_cast<sptr_t>(content.toUtf8().constData()));
            (*_ppEditView)->execute(SCI_SETSAVEPOINT);
        }
    }
}

QString ProjectPanel::getAbsoluteFilePath(const QString& relativePath) const
{
    if (QFileInfo(relativePath).isAbsolute()) {
        return relativePath;
    }

    if (_workspaceFile.isEmpty()) {
        return relativePath;
    }

    QFileInfo wsFi(_workspaceFile);
    QString wsDir = wsFi.absolutePath();
    return QDir(wsDir).absoluteFilePath(relativePath);
}

QString ProjectPanel::getRelativePath(const QString& filePath) const
{
    if (_workspaceFile.isEmpty()) {
        return filePath;
    }

    QFileInfo wsFi(_workspaceFile);
    QString wsDir = wsFi.absolutePath();

    QDir dir(wsDir);
    return dir.relativeFilePath(filePath);
}

bool ProjectPanel::doesFileExist(const QString& filePath) const
{
    return QFile::exists(filePath);
}

void ProjectPanel::updateFileIcon(int itemId, bool exists)
{
    int iconIndex = exists ? INDEX_LEAF : INDEX_LEAF_INVALID;
    // Note: TreeView doesn't have a direct setItemImage method in the current interface
    // This would need to be implemented in TreeView
}

// ============================================================================
// Context Menu
// ============================================================================

void ProjectPanel::onContextMenu(const QPoint& pos)
{
    QTreeWidget* tree = _treeView->getTreeWidget();
    QTreeWidgetItem* item = tree->itemAt(pos);

    if (item) {
        int itemId = _treeView->getSelectedItem();
        QPoint globalPos = tree->mapToGlobal(pos);
        showContextMenuAt(globalPos, itemId);
    }
}

void ProjectPanel::showContextMenuAt(const QPoint& pos, int itemId)
{
    QMenu* menu = getContextMenuForItem(itemId);
    if (menu) {
        menu->exec(pos);
    }
}

QMenu* ProjectPanel::getContextMenuForItem(int itemId)
{
    NodeType type = getNodeType(itemId);

    switch (type) {
        case NodeType::Root:
            return _workspaceMenu;
        case NodeType::Project:
            return _projectMenu;
        case NodeType::Folder:
            return _folderMenu;
        case NodeType::File:
            return _fileMenu;
    }

    return nullptr;
}

// ============================================================================
// Slot Implementations
// ============================================================================

void ProjectPanel::onItemDoubleClicked(int itemId, int column)
{
    Q_UNUSED(column)

    NodeType type = getNodeType(itemId);
    if (type == NodeType::File) {
        QString filePath = _itemPaths[itemId];
        if (doesFileExist(filePath)) {
            openFile(filePath);
        } else {
            QMessageBox::warning(_widget, tr("Open File"),
                                 tr("The file does not exist:\n%1").arg(filePath));
        }
    } else {
        // Toggle expand/collapse for folders
        if (_treeView->isExpanded(itemId)) {
            _treeView->collapse(itemId);
        } else {
            _treeView->expand(itemId);
        }
    }
}

void ProjectPanel::onItemExpanded(int itemId)
{
    NodeType type = getNodeType(itemId);
    if (type == NodeType::Folder) {
        // Update icon to open folder
        // Note: This would require TreeView to support icon changes
    }
}

void ProjectPanel::onItemCollapsed(int itemId)
{
    NodeType type = getNodeType(itemId);
    if (type == NodeType::Folder) {
        // Update icon to closed folder
    }
}

void ProjectPanel::onItemChanged(int itemId, int column)
{
    Q_UNUSED(column)

    // Item was renamed
    setWorkspaceDirty(true);

    // If it's a file, update the path
    QString newName = _treeView->getItemText(itemId);
    if (_itemPaths.contains(itemId)) {
        QString oldPath = _itemPaths[itemId];
        if (!oldPath.isEmpty()) {
            QFileInfo fi(oldPath);
            QString newPath = fi.absolutePath() + "/" + newName;
            _itemPaths[itemId] = newPath;
        }
    }
}

void ProjectPanel::onCurrentItemChanged(int currentItemId, int previousItemId)
{
    Q_UNUSED(previousItemId)
    Q_UNUSED(currentItemId)
    // Update UI based on selection if needed
}

// ============================================================================
// Action Implementations
// ============================================================================

void ProjectPanel::onNewWorkspace()
{
    if (!checkIfNeedSave()) {
        return;
    }

    _treeView->clear();
    _itemPaths.clear();
    newWorkspace();
}

void ProjectPanel::onOpenWorkspace()
{
    QString filter;
    setFileExtFilter(filter);

    QString filePath = QFileDialog::getOpenFileName(_widget, tr("Open Workspace"),
                                                    QString(), filter);
    if (!filePath.isEmpty()) {
        openWorkspace(filePath);
    }
}

void ProjectPanel::onReloadWorkspace()
{
    if (_workspaceFile.isEmpty()) {
        return;
    }

    if (_isDirty) {
        int res = QMessageBox::question(_widget, tr("Reload Workspace"),
                                        tr("The current workspace was modified. Reloading will discard all modifications.\n"
                                           "Do you want to continue?"),
                                        QMessageBox::Yes | QMessageBox::No);
        if (res != QMessageBox::Yes) {
            return;
        }
    }

    if (doesFileExist(_workspaceFile)) {
        openWorkspace(_workspaceFile);
    } else {
        QMessageBox::warning(_widget, tr("Reload Workspace"),
                             tr("Cannot find the file to reload."));
    }
}

void ProjectPanel::onSaveWorkspace()
{
    saveWorkspace();
}

void ProjectPanel::onSaveWorkspaceAs()
{
    saveWorkspaceAs(QString());
}

void ProjectPanel::onSaveACopyAs()
{
    QString filter;
    setFileExtFilter(filter);

    QString filePath = QFileDialog::getSaveFileName(_widget, tr("Save a Copy As"),
                                                    _workspaceFile, filter);
    if (!filePath.isEmpty()) {
        writeWorkspace(filePath, false);
    }
}

void ProjectPanel::onNewProject()
{
    int rootId = _treeView->getRootItem();
    if (rootId < 0) {
        return;
    }

    bool ok;
    QString projectName = QInputDialog::getText(_widget, tr("New Project"),
                                                tr("Project name:"),
                                                QLineEdit::Normal,
                                                tr(PM_NEWPROJECTNAME), &ok);
    if (ok && !projectName.isEmpty()) {
        addProjectToTree(projectName, rootId);
        _treeView->expand(rootId);
        setWorkspaceDirty(true);
    }
}

void ProjectPanel::onFindInProjects()
{
    // This would integrate with the FindReplaceDlg
    // For now, just emit a signal or call a callback
}

void ProjectPanel::onNewFile()
{
    int selectedItem = _treeView->getSelectedItem();
    if (selectedItem < 0) {
        return;
    }

    NodeType type = getNodeType(selectedItem);
    int parentId = selectedItem;

    // If a file is selected, add to its parent
    if (type == NodeType::File) {
        parentId = _treeView->getParentItem(selectedItem);
    }

    QString filter = tr("All Files (*.*)");
    QStringList filePaths = QFileDialog::getOpenFileNames(_widget, tr("Add Files"),
                                                          _lastSelectedDir, filter);

    if (!filePaths.isEmpty()) {
        _lastSelectedDir = QFileInfo(filePaths.first()).absolutePath();
        for (const QString& filePath : filePaths) {
            addFileToTree(filePath, parentId);
        }
        setWorkspaceDirty(true);
    }
}

void ProjectPanel::onNewFolder()
{
    int selectedItem = _treeView->getSelectedItem();
    if (selectedItem < 0) {
        return;
    }

    NodeType type = getNodeType(selectedItem);
    int parentId = selectedItem;

    // If a file is selected, add to its parent
    if (type == NodeType::File) {
        parentId = _treeView->getParentItem(selectedItem);
    }

    bool ok;
    QString folderName = QInputDialog::getText(_widget, tr("New Folder"),
                                               tr("Folder name:"),
                                               QLineEdit::Normal,
                                               tr(PM_NEWFOLDERNAME), &ok);
    if (ok && !folderName.isEmpty()) {
        int folderId = addFolderToTree(folderName, parentId);
        _treeView->expand(parentId);

        // Enable editing of the new folder name
        _treeView->makeLabelEditable(true);
        _treeView->editItem(folderId);

        setWorkspaceDirty(true);
    }
}

void ProjectPanel::onAddFiles()
{
    onNewFile();  // Same operation
}

void ProjectPanel::onAddFilesFromDirectory()
{
    int selectedItem = _treeView->getSelectedItem();
    if (selectedItem < 0) {
        return;
    }

    NodeType type = getNodeType(selectedItem);
    int parentId = selectedItem;

    // If a file is selected, add to its parent
    if (type == NodeType::File) {
        parentId = _treeView->getParentItem(selectedItem);
    }

    QString dirPath = QFileDialog::getExistingDirectory(_widget, tr("Add Files from Directory"),
                                                        _lastSelectedDir);
    if (!dirPath.isEmpty()) {
        _lastSelectedDir = dirPath;
        recursiveAddFilesFrom(dirPath, parentId);
        setWorkspaceDirty(true);
    }
}

void ProjectPanel::onRemoveItem()
{
    int selectedItem = _treeView->getSelectedItem();
    if (selectedItem < 0) {
        return;
    }

    NodeType type = getNodeType(selectedItem);

    // Check if folder has children
    int childId = _treeView->getChildItem(selectedItem);
    if (type == NodeType::Folder && childId >= 0) {
        int res = QMessageBox::question(_widget, tr("Remove Folder"),
                                        tr("All the sub-items will be removed.\n"
                                           "Are you sure you want to remove this folder from the project?"),
                                        QMessageBox::Yes | QMessageBox::No);
        if (res != QMessageBox::Yes) {
            return;
        }
    } else if (type == NodeType::File) {
        int res = QMessageBox::question(_widget, tr("Remove File"),
                                        tr("Are you sure you want to remove this file from the project?"),
                                        QMessageBox::Yes | QMessageBox::No);
        if (res != QMessageBox::Yes) {
            return;
        }
    }

    _treeView->removeItem(selectedItem);
    _itemPaths.erase(selectedItem);
    setWorkspaceDirty(true);
}

void ProjectPanel::onRenameItem()
{
    int selectedItem = _treeView->getSelectedItem();
    if (selectedItem < 0) {
        return;
    }

    NodeType type = getNodeType(selectedItem);
    if (type == NodeType::Root) {
        return;  // Can't rename root
    }

    _treeView->makeLabelEditable(true);
    _treeView->editItem(selectedItem);
}

void ProjectPanel::onMoveUp()
{
    int selectedItem = _treeView->getSelectedItem();
    if (selectedItem < 0) {
        return;
    }

    // Get previous sibling
    int prevSibling = _treeView->getPrevSibling(selectedItem);
    if (prevSibling >= 0) {
        // Swap items - this is a simplified implementation
        // A full implementation would need to handle the tree structure properly
        setWorkspaceDirty(true);
    }
}

void ProjectPanel::onMoveDown()
{
    int selectedItem = _treeView->getSelectedItem();
    if (selectedItem < 0) {
        return;
    }

    // Get next sibling
    int nextSibling = _treeView->getNextSibling(selectedItem);
    if (nextSibling >= 0) {
        // Swap items
        setWorkspaceDirty(true);
    }
}

void ProjectPanel::onModifyFilePath()
{
    int selectedItem = _treeView->getSelectedItem();
    if (selectedItem < 0) {
        return;
    }

    NodeType type = getNodeType(selectedItem);
    if (type != NodeType::File) {
        return;
    }

    QString currentPath = _itemPaths[selectedItem];

    FileRelocalizerDlg dlg(_widget);
    if (dlg.doDialog(currentPath) == QDialog::Accepted) {
        QString newPath = dlg.getFullFilePath();
        if (newPath != currentPath) {
            _itemPaths[selectedItem] = newPath;
            QFileInfo fi(newPath);
            _treeView->setItemText(selectedItem, fi.fileName());
            updateFileIcon(selectedItem, doesFileExist(newPath));
            setWorkspaceDirty(true);
        }
    }
}

void ProjectPanel::onRefresh()
{
    // Refresh file existence status
    for (auto it = _itemPaths.begin(); it != _itemPaths.end(); ++it) {
        if (!it->second.isEmpty()) {
            updateFileIcon(it->first, doesFileExist(it->second));
        }
    }
}

void ProjectPanel::onOpenSelectedFile()
{
    int selectedItem = _treeView->getSelectedItem();
    if (selectedItem < 0) {
        return;
    }

    NodeType type = getNodeType(selectedItem);
    if (type == NodeType::File) {
        QString filePath = _itemPaths[selectedItem];
        if (doesFileExist(filePath)) {
            openFile(filePath);
        }
    }
}

// ============================================================================
// Helper Methods
// ============================================================================

void ProjectPanel::recursiveAddFilesFrom(const QString& folderPath, int parentId)
{
    QDir dir(folderPath);
    if (!dir.exists()) {
        return;
    }

    // Get all entries
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

    // First, process directories
    for (const QFileInfo& entry : entries) {
        if (entry.isDir() && !entry.isHidden()) {
            QString subDirName = entry.fileName();
            int folderId = addFolderToTree(subDirName, parentId);
            recursiveAddFilesFrom(entry.absoluteFilePath(), folderId);
        }
    }

    // Then, process files
    for (const QFileInfo& entry : entries) {
        if (entry.isFile()) {
            addFileToTree(entry.absoluteFilePath(), parentId);
        }
    }

    _treeView->expand(parentId);
}

void ProjectPanel::setWorkspaceDirty(bool dirty)
{
    _isDirty = dirty;

    // Update root icon
    int rootId = _treeView->getRootItem();
    if (rootId >= 0) {
        int iconIndex = dirty ? INDEX_DIRTY_ROOT : INDEX_CLEAN_ROOT;
        // Note: TreeView would need setItemImage method
        Q_UNUSED(iconIndex)
    }
}

bool ProjectPanel::checkIfNeedSave()
{
    if (!_isDirty) {
        return true;
    }

    QString title = _workspaceFile.isEmpty() ? _panelTitle : QFileInfo(_workspaceFile).fileName();

    int res = QMessageBox::question(_widget, tr("Save Workspace"),
                                    tr("The workspace was modified. Do you want to save it?"),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                    QMessageBox::Yes);

    if (res == QMessageBox::Yes) {
        saveWorkspace();
        return !_isDirty;  // If save failed, _isDirty will still be true
    } else if (res == QMessageBox::No) {
        return true;
    } else {
        return false;  // Cancel
    }
}

bool ProjectPanel::saveWorkspaceRequest()
{
    return checkIfNeedSave();
}

void ProjectPanel::setFileExtFilter(QString& filter)
{
    // Default workspace extension filter
    filter = tr("Workspace files (*.workspace);;All files (*.*)");
}

void ProjectPanel::setBackgroundColor(const QColor& color)
{
    if (_treeView && _treeView->getTreeWidget()) {
        QPalette palette = _treeView->getTreeWidget()->palette();
        palette.setColor(QPalette::Base, color);
        _treeView->getTreeWidget()->setPalette(palette);
    }
}

void ProjectPanel::setForegroundColor(const QColor& color)
{
    if (_treeView && _treeView->getTreeWidget()) {
        QPalette palette = _treeView->getTreeWidget()->palette();
        palette.setColor(QPalette::Text, color);
        _treeView->getTreeWidget()->setPalette(palette);
    }
}

// ============================================================================
// FileRelocalizerDlg Implementation
// ============================================================================

FileRelocalizerDlg::FileRelocalizerDlg(QWidget* parent)
    : StaticDialog(parent)
{
    setupUI();
    connectSignals();
}

FileRelocalizerDlg::~FileRelocalizerDlg() = default;

void FileRelocalizerDlg::setupUI()
{
    _widget = new QDialog(this);
    _widget->setWindowTitle(tr("Modify File Path"));

    auto* layout = new QVBoxLayout(_widget);

    auto* label = new QLabel(tr("Full file path:"), _widget);
    layout->addWidget(label);

    _pathEdit = new QLineEdit(_widget);
    layout->addWidget(_pathEdit);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    _browseButton = new QPushButton(tr("Browse..."), _widget);
    buttonLayout->addWidget(_browseButton);

    _okButton = new QPushButton(tr("OK"), _widget);
    _okButton->setDefault(true);
    buttonLayout->addWidget(_okButton);

    _cancelButton = new QPushButton(tr("Cancel"), _widget);
    buttonLayout->addWidget(_cancelButton);

    layout->addLayout(buttonLayout);

    _widget->setMinimumWidth(400);
}

void FileRelocalizerDlg::connectSignals()
{
    connect(_okButton, &QPushButton::clicked, this, &FileRelocalizerDlg::onOkClicked);
    connect(_cancelButton, &QPushButton::clicked, this, &FileRelocalizerDlg::onCancelClicked);
    connect(_browseButton, &QPushButton::clicked, this, &FileRelocalizerDlg::onBrowseClicked);
}

int FileRelocalizerDlg::doDialog(const QString& filePath, bool isRTL)
{
    Q_UNUSED(isRTL)

    _fullFilePath = filePath;
    _pathEdit->setText(filePath);

    if (auto* dlg = qobject_cast<QDialog*>(_widget)) {
        return dlg->exec();
    }
    return QDialog::Rejected;
}

void FileRelocalizerDlg::onOkClicked()
{
    _fullFilePath = _pathEdit->text();

    if (auto* dlg = qobject_cast<QDialog*>(_widget)) {
        dlg->accept();
    }
}

void FileRelocalizerDlg::onCancelClicked()
{
    if (auto* dlg = qobject_cast<QDialog*>(_widget)) {
        dlg->reject();
    }
}

void FileRelocalizerDlg::onBrowseClicked()
{
    QString filePath = QFileDialog::getOpenFileName(_widget, tr("Select File"),
                                                    _pathEdit->text());
    if (!filePath.isEmpty()) {
        _pathEdit->setText(filePath);
    }
}

} // namespace QtControls
