// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "TabBar.h"
#include <QApplication>
#include <QDrag>
#include <QMimeData>

namespace QtControls {

bool TabBar::init(QWidget* parent, bool isVertical, bool isMultiLine)
{
    (void)isMultiLine; // Qt handles multi-line automatically

    if (!parent) return false;

    _parent = parent;
    _isVertical = isVertical;

    QTabWidget* tabWidget = new QTabWidget(parent);
    _widget = tabWidget;

    if (isVertical) {
        tabWidget->setTabPosition(QTabWidget::West);
    }

    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);

    return true;
}

void TabBar::destroy()
{
    _tabs.clear();
    if (_widget) {
        delete _widget;
        _widget = nullptr;
    }
}

int TabBar::insertAtEnd(const QString& title)
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return -1;

    // Create empty widget as tab content
    QWidget* page = new QWidget();
    int index = tabWidget->addTab(page, title);

    TabInfo info;
    info.title = title;
    info.changeState = FileChangeState::None;
    info.index = index;

    if (index >= static_cast<int>(_tabs.size())) {
        _tabs.push_back(info);
    } else {
        _tabs[index] = info;
    }

    updateTabAppearance(index);
    return index;
}

int TabBar::insertAtEnd(const wchar_t* title)
{
    return insertAtEnd(QString::fromWCharArray(title));
}

void TabBar::activateAt(int index)
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    if (index >= 0 && index < tabWidget->count()) {
        tabWidget->setCurrentIndex(index);
    }
}

void TabBar::deactivateAt(int index)
{
    (void)index;
    // In Qt, we don't really deactivate tabs
    // We just don't show them as active
}

QString TabBar::getCurrentTitle() const
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return QString();

    int index = tabWidget->currentIndex();
    if (index >= 0) {
        return tabWidget->tabText(index);
    }
    return QString();
}

int TabBar::getCurrentTabIndex() const
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return -1;

    return tabWidget->currentIndex();
}

int TabBar::getItemCount() const
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return 0;

    return tabWidget->count();
}

void TabBar::deleteItemAt(size_t index)
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    if (index < static_cast<size_t>(tabWidget->count())) {
        tabWidget->removeTab(static_cast<int>(index));

        // Update our tracking vector
        if (index < _tabs.size()) {
            _tabs.erase(_tabs.begin() + index);
        }

        // Re-index remaining tabs
        for (size_t i = index; i < _tabs.size(); ++i) {
            _tabs[i].index = static_cast<int>(i);
        }
    }
}

void TabBar::deleteAllItems()
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    while (tabWidget->count() > 0) {
        tabWidget->removeTab(0);
    }

    _tabs.clear();
}

void TabBar::setImageList(void* himl)
{
    (void)himl;
    // Windows-specific image list
    // Qt uses QIcon instead
}

void TabBar::setColour(FileChangeState state, QColor colour)
{
    switch (state) {
        case FileChangeState::Modified:
            _modifiedColour = colour;
            break;
        case FileChangeState::ModifiedAndUnsaved:
            _unsavedColour = colour;
            break;
        default:
            break;
    }
}

void TabBar::updateTabAppearance(int index)
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget || index < 0 || index >= tabWidget->count()) return;

    if (index >= static_cast<int>(_tabs.size())) return;

    QString title = _tabs[index].title;

    switch (_tabs[index].changeState) {
        case FileChangeState::Modified:
            title = "* " + title;
            tabWidget->setTabText(index, title);
            if (_modifiedColour.isValid()) {
                tabWidget->tabBar()->setTabTextColor(index, _modifiedColour);
            }
            break;
        case FileChangeState::ModifiedAndUnsaved:
            title = "+ " + title;
            tabWidget->setTabText(index, title);
            if (_unsavedColour.isValid()) {
                tabWidget->tabBar()->setTabTextColor(index, _unsavedColour);
            }
            break;
        default:
            tabWidget->setTabText(index, title);
            tabWidget->tabBar()->setTabTextColor(index, QColor());
            break;
    }
}

// ============================================================================
// TabBarPlus implementation
// ============================================================================

bool TabBarPlus::init(QWidget* parent, bool isVertical, bool isMultiLine)
{
    if (!TabBar::init(parent, isVertical, isMultiLine)) return false;

    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return false;

    // Enable close buttons on tabs
    tabWidget->setTabsClosable(true);

    // Connect signals
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &TabBarPlus::tabCloseRequested);

    return true;
}

void TabBarPlus::tabToStart()
{
    int current = getCurrentTabIndex();
    if (current <= 0) return;

    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    // Move tab to position 0
    QWidget* widget = tabWidget->widget(current);
    QString text = tabWidget->tabText(current);

    tabWidget->removeTab(current);
    tabWidget->insertTab(0, widget, text);
    tabWidget->setCurrentIndex(0);
}

void TabBarPlus::tabToEnd()
{
    int current = getCurrentTabIndex();
    int count = getItemCount();
    if (current < 0 || current >= count - 1) return;

    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    // Move tab to last position
    QWidget* widget = tabWidget->widget(current);
    QString text = tabWidget->tabText(current);

    tabWidget->removeTab(current);
    tabWidget->addTab(widget, text);
    tabWidget->setCurrentIndex(count - 1);
}

void TabBarPlus::tabToPrev()
{
    int current = getCurrentTabIndex();
    if (current <= 0) return;

    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    tabWidget->setCurrentIndex(current - 1);
}

void TabBarPlus::tabToNext()
{
    int current = getCurrentTabIndex();
    int count = getItemCount();
    if (current < 0 || current >= count - 1) return;

    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    tabWidget->setCurrentIndex(current + 1);
}

void TabBarPlus::setCloseBtnImageList(void* himl)
{
    (void)himl;
    // Windows-specific
    // Qt uses native close buttons
}

void TabBarPlus::setPinBtnImageList(void* himl)
{
    (void)himl;
    // Windows-specific
}

void TabBarPlus::setColour(FileChangeState state, QColor colour)
{
    TabBar::setColour(state, colour);
}

void TabBarPlus::triggerOwnerDrawTabbar()
{
    // Qt uses stylesheets or custom QStyle for custom drawing
}

void TabBarPlus::doVertical()
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    tabWidget->setTabPosition(QTabWidget::West);
    _isVertical = true;
}

void TabBarPlus::doMultiLine()
{
    // Qt handles multi-line tabs automatically when needed
}

void TabBarPlus::setIndividualTabColour(int tabIndex, QColor colour)
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    tabWidget->tabBar()->setTabTextColor(tabIndex, colour);
}

void TabBarPlus::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _isDragging = true;
        _dragStartPos = event->pos();

        QTabWidget* tabWidget = getTabWidget();
        if (tabWidget) {
            _dragStartIndex = tabWidget->currentIndex();
        }
    }
}

void TabBarPlus::mouseMoveEvent(QMouseEvent* event)
{
    if (!_isDragging) return;

    if ((event->pos() - _dragStartPos).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    // Handle drag
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    // Qt's QTabWidget handles tab moving internally when setMovable(true)
}

void TabBarPlus::mouseReleaseEvent(QMouseEvent* event)
{
    (void)event;
    _isDragging = false;
    _dragStartIndex = -1;
}

void TabBarPlus::mouseDoubleClickEvent(QMouseEvent* event)
{
    (void)event;
    // Emit pin request on double click
    emit tabPinRequested(getCurrentTabIndex());
}

} // namespace QtControls
