// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Window.h"
#include <QTabWidget>
#include <QTabBar>
#include <QMouseEvent>
#include <vector>

namespace QtControls {

// File change state for tab indicator
enum class FileChangeState {
    None,
    Modified,
    ModifiedAndUnsaved
};

struct TabInfo {
    QString title;
    FileChangeState changeState;
    int index;
};

class TabBar : public Window
{
public:
    TabBar() = default;
    ~TabBar() override = default;

    virtual bool init(QWidget* parent, bool isVertical = false, bool isMultiLine = false);

    void destroy() override;

    int insertAtEnd(const QString& title);
    int insertAtEnd(const wchar_t* title);

    void activateAt(int index);
    void deactivateAt(int index);

    QString getCurrentTitle() const;
    int getCurrentTabIndex() const;

    int getItemCount() const;

    void deleteItemAt(size_t index);
    void deletItemAt(size_t index) { deleteItemAt(index); }  // Compatibility wrapper for original typo
    void deleteAllItems();

    void setImageList(void* himl);

    void setColour(FileChangeState state, QColor colour);

    QTabWidget* getTabWidget() const { return qobject_cast<QTabWidget*>(_widget); }

protected:
    std::vector<TabInfo> _tabs;
    QColor _modifiedColour;
    QColor _unsavedColour;
    bool _isVertical = false;

    void updateTabAppearance(int index);
};

class TabBarPlus : public TabBar
{
    Q_OBJECT
public:
    TabBarPlus() = default;
    ~TabBarPlus() override = default;

    bool init(QWidget* parent, bool isVertical = false, bool isMultiLine = false) override;

    // Drag and drop
    void tabToStart();
    void tabToEnd();
    void tabToPrev();
    void tabToNext();

    // Tab buttons
    void setCloseBtnImageList(void* himl);
    void setPinBtnImageList(void* himl);

    // Colors
    void setColour(FileChangeState state, QColor colour);

    // Drawing modes
    void triggerOwnerDrawTabbar();
    void doVertical();
    void doMultiLine();

    void setIndividualTabColour(int tabIndex, QColor colour);

signals:
    void tabCloseRequested(int index);
    void tabPinRequested(int index);
    void tabMoved(int fromIndex, int toIndex);

protected:
    bool _isDragging = false;
    int _dragStartIndex = -1;
    QPoint _dragStartPos;

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
};

} // namespace QtControls
