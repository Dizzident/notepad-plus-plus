// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../../ScintillaComponent/Buffer.h"
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtCore/QPoint>
#include <memory>
#include <vector>

// Forward declarations
class QListWidget;
class QListWidgetItem;
class QMenu;
class QAction;
class QContextMenuEvent;
class QVBoxLayout;
class QEvent;
class QResizeEvent;
class ScintillaEditView;

namespace QtControls {

// Forward declarations
class ListView;

// ============================================================================
// Document Item Data Structure
// ============================================================================
struct DocItemData {
    int _iView = -1;
    int _docIndex = 0;
    QString _filePath;
    int _status = 0;
    BufferID _bufID = nullptr;
    int _docColor = -1;

    DocItemData() = default;
    DocItemData(int iView, int docIndex, const QString& filePath, int status, BufferID bufID, int docColor)
        : _iView(iView), _docIndex(docIndex), _filePath(filePath), _status(status), _bufID(bufID), _docColor(docColor)
    {}
};

// ============================================================================
// VerticalFileSwitcher - Dockable panel showing list of open documents
// ============================================================================
class VerticalFileSwitcher : public StaticDialog {
    Q_OBJECT

public:
    explicit VerticalFileSwitcher(QWidget* parent = nullptr);
    ~VerticalFileSwitcher() override;

    void init(ScintillaEditView** ppEditView);
    void doDialog();
    bool run_dlgProc(QEvent* event) override;

    using StaticDialog::getDialog;
    QWidget* getWidget() const { return getDialog(); }

    void activateDoc(DocItemData* docData) const;
    void closeDoc(DocItemData* docData) const;

    int newItem(BufferID bufferID, int iView);
    int closeItem(BufferID bufferID, int iView);
    void activateItem(BufferID bufferID, int iView);

    void setItemIconStatus(BufferID bufferID);
    void setItemColor(BufferID bufferID);

    QString getFullFilePath(int index) const;
    int nbSelectedFiles() const;
    std::vector<DocItemData> getSelectedFiles(bool reverse = false) const;

    void reload();
    void updateTabOrder();

    int setHeaderOrder(int columnIndex);
    void updateHeaderArrow();
    void startColumnSort();

    void setBackgroundColor(const QColor& bgColour);
    void setForegroundColor(const QColor& fgColour);

    void ensureVisibleCurrentItem();

public slots:
    void onItemClicked(int index);
    void onItemDoubleClicked(int index);
    void onContextMenuRequested(int index, const QPoint& pos);
    void onSelectionChanged();

    void onToggleExtColumn();
    void onTogglePathColumn();
    void onToggleGroupByView();

protected:
    void setupUI() override;
    void connectSignals() override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    ListView* _fileListView = nullptr;
    QVBoxLayout* _mainLayout = nullptr;
    QMenu* _contextMenu = nullptr;

    QAction* _extColumnAction = nullptr;
    QAction* _pathColumnAction = nullptr;
    QAction* _groupByViewAction = nullptr;

    int _lastSortingColumn = 0;
    int _lastSortingDirection = -1;
    bool _colHeaderRClick = false;

    bool _showExtColumn = true;
    bool _showPathColumn = true;
    bool _groupByView = true;

    QColor _bgColor;
    QColor _fgColor;

    ScintillaEditView** _ppEditView = nullptr;

    static constexpr int SORT_DIRECTION_NONE = -1;
    static constexpr int SORT_DIRECTION_UP = 0;
    static constexpr int SORT_DIRECTION_DOWN = 1;

    void setupListView();
    void setupContextMenu();
    void initPopupMenus();

    void initList();
    void removeAll();
    int findItem(BufferID bufferID, int iView) const;
    int addItem(BufferID bufferID, int iView);
    void removeItem(int index);
    void selectCurrentItem();

    void resizeColumns();
    int getStatusIconIndex(BufferID bufferID) const;
    QString getFileNameOnly(const QString& filePath) const;
    QString getFileExtension(const QString& filePath) const;
    QString getFileDirectory(const QString& filePath) const;

    void sortItems(int column, int direction);
};

} // namespace QtControls
