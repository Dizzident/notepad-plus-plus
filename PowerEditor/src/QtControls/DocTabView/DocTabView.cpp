// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "DocTabView.h"
#include "../../QtCore/Buffer.h"

#include <QTabWidget>
#include <QFileInfo>
#include <QDebug>

#include <iostream>

namespace QtControls {

DocTabView::DocTabView()
    : TabBarPlus()
    , _pView(nullptr)
{
}

DocTabView::~DocTabView()
{
}

void DocTabView::init(QWidget* parent, ScintillaEditView* pView, unsigned char indexChoice, unsigned char buttonsStatus)
{
    std::cout << "[DocTabView::init] Initializing DocTabView..." << std::endl;
    std::cout << "[DocTabView::init] Parent: " << parent << ", View: " << pView << std::endl;

    (void)buttonsStatus; // Not used in Qt version

    TabBarPlus::init(parent, false, false);
    _pView = pView;

    createIconSets();

    if (indexChoice < static_cast<unsigned char>(_iconSets.size())) {
        _iconListIndexChoice = indexChoice;
    } else {
        _iconListIndexChoice = 0;
    }

    std::cout << "[DocTabView::init] Initialization complete. Widget: " << getWidget() << std::endl;
}

void DocTabView::createIconSets()
{
    // Create icon sets for different states
    // In a full implementation, these would load actual icon resources
    // For now, we create placeholder icons

    _iconSets.clear();

    // Default icon set
    QList<QIcon> defaultSet;
    defaultSet.append(QIcon()); // SAVED_IMG_INDEX - no icon for saved
    defaultSet.append(QIcon::fromTheme("document-save", QIcon())); // UNSAVED_IMG_INDEX
    defaultSet.append(QIcon::fromTheme("emblem-readonly", QIcon())); // READONLY_IMG_INDEX
    defaultSet.append(QIcon::fromTheme("emblem-readonly", QIcon())); // READONLYSYS_IMG_INDEX
    defaultSet.append(QIcon::fromTheme("view-refresh", QIcon())); // MONITORING_IMG_INDEX
    _iconSets.append(defaultSet);

    // Alternative icon set
    QList<QIcon> altSet;
    altSet.append(QIcon());
    altSet.append(QIcon::fromTheme("document-save", QIcon()));
    altSet.append(QIcon::fromTheme("emblem-readonly", QIcon()));
    altSet.append(QIcon::fromTheme("emblem-readonly", QIcon()));
    altSet.append(QIcon::fromTheme("view-refresh", QIcon()));
    _iconSets.append(altSet);

    // Dark mode icon set
    QList<QIcon> darkSet;
    darkSet.append(QIcon());
    darkSet.append(QIcon::fromTheme("document-save", QIcon()));
    darkSet.append(QIcon::fromTheme("emblem-readonly", QIcon()));
    darkSet.append(QIcon::fromTheme("emblem-readonly", QIcon()));
    darkSet.append(QIcon::fromTheme("view-refresh", QIcon()));
    _iconSets.append(darkSet);
}

void DocTabView::changeIconSet(unsigned char choice)
{
    if (choice >= static_cast<unsigned char>(_iconSets.size()))
        return;

    _iconListIndexChoice = choice;

    // Update all tabs with new icons
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    for (int i = 0; i < tabWidget->count(); ++i) {
        BufferID id = getBufferByIndex(i);
        if (id) {
            int imgIndex = getImageIndexForBuffer(id);
            if (imgIndex >= 0 && imgIndex < _iconSets[_iconListIndexChoice].size()) {
                tabWidget->setTabIcon(i, _iconSets[_iconListIndexChoice][imgIndex]);
            }
        }
    }
}

void DocTabView::addBuffer(BufferID buffer)
{
    if (buffer == BUFFER_INVALID)
        return;

    if (getIndexByBuffer(buffer) != -1)
        return; // No duplicates

    // Get the buffer title
    QString title = buffer->getFileNameQString();
    if (title.isEmpty()) {
        title = QString("new %1").arg(getItemCount() + 1);
    }

    // Add tab
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    QWidget* page = new QWidget();
    int index = tabWidget->addTab(page, title);

    // Store buffer mapping
    _bufferToIndex[buffer] = index;

    // Update tab appearance
    bufferUpdated(buffer, DocTabBufferChangeMask);

    // Make sure tab widget is visible - critical for initial tab display
    tabWidget->show();

    // Update parent layout
    if (_parent) {
        _parent->updateGeometry();
    }
}

void DocTabView::closeBuffer(BufferID buffer)
{
    int indexToClose = getIndexByBuffer(buffer);
    if (indexToClose == -1)
        return;

    deleteItemAt(static_cast<size_t>(indexToClose));
    _bufferToIndex.remove(buffer);

    // Rebuild index mapping
    QTabWidget* tabWidget = getTabWidget();
    if (tabWidget) {
        _bufferToIndex.clear();
        for (int i = 0; i < tabWidget->count(); ++i) {
            BufferID id = getBufferByIndex(i);
            if (id) {
                _bufferToIndex[id] = i;
            }
        }
    }

    if (_parent) {
        _parent->updateGeometry();
    }
}

bool DocTabView::activateBuffer(BufferID buffer)
{
    int indexToActivate = getIndexByBuffer(buffer);
    if (indexToActivate == -1)
        return false;

    // Make sure tab widget is visible before activating
    QTabWidget* tabWidget = getTabWidget();
    if (tabWidget) {
        tabWidget->show();
    }

    activateAt(indexToActivate);
    return true;
}

BufferID DocTabView::activeBuffer()
{
    int index = getCurrentTabIndex();
    return getBufferByIndex(index);
}

BufferID DocTabView::findBufferByName(const wchar_t* fullfilename)
{
    if (!fullfilename)
        return BUFFER_INVALID;

    QString filePath = QString::fromWCharArray(fullfilename);

    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return BUFFER_INVALID;

    for (int i = 0; i < tabWidget->count(); ++i) {
        BufferID id = getBufferByIndex(i);
        if (id) {
            QString bufferPath = id->getFilePath();
            if (bufferPath.compare(filePath, Qt::CaseInsensitive) == 0) {
                return id;
            }
        }
    }

    return BUFFER_INVALID;
}

int DocTabView::getIndexByBuffer(BufferID id)
{
    if (!id)
        return -1;

    auto it = _bufferToIndex.find(id);
    if (it != _bufferToIndex.end()) {
        // Verify the index is still valid
        QTabWidget* tabWidget = getTabWidget();
        if (tabWidget && it.value() < tabWidget->count()) {
            return it.value();
        }
    }

    // Fallback: search through tabs
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return -1;

    for (int i = 0; i < tabWidget->count(); ++i) {
        BufferID tabBuffer = getBufferByIndex(i);
        if (tabBuffer == id) {
            _bufferToIndex[id] = i;
            return i;
        }
    }

    return -1;
}

BufferID DocTabView::getBufferByIndex(size_t index)
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return BUFFER_INVALID;

    if (index >= static_cast<size_t>(tabWidget->count()))
        return BUFFER_INVALID;

    // Find buffer by index in our mapping
    for (auto it = _bufferToIndex.begin(); it != _bufferToIndex.end(); ++it) {
        if (static_cast<size_t>(it.value()) == index) {
            return it.key();
        }
    }

    return BUFFER_INVALID;
}

size_t DocTabView::nbItem() const
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return 0;

    return static_cast<size_t>(tabWidget->count());
}

void DocTabView::setBuffer(size_t index, BufferID id)
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    if (index >= static_cast<size_t>(tabWidget->count()))
        return;

    // Remove old mapping for this index
    for (auto it = _bufferToIndex.begin(); it != _bufferToIndex.end(); ) {
        if (static_cast<size_t>(it.value()) == index) {
            it = _bufferToIndex.erase(it);
        } else {
            ++it;
        }
    }

    // Add new mapping
    if (id != BUFFER_INVALID) {
        _bufferToIndex[id] = static_cast<int>(index);
    }

    // Update tab appearance
    if (id) {
        bufferUpdated(id, DocTabBufferChangeMask);
    }

    if (_parent) {
        _parent->updateGeometry();
    }
}

void DocTabView::bufferUpdated(Buffer* buffer, int mask)
{
    int index = getIndexByBuffer(buffer);
    if (index == -1)
        return;

    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    // Update icon if state changed (check dirty and readonly bits)
    // BufferChangeDirty = 0x002, BufferChangeReadonly = 0x010
    if (mask & 0x012) {
        int imgIndex = getImageIndexForBuffer(buffer);
        if (imgIndex >= 0 && _iconListIndexChoice < _iconSets.size() &&
            imgIndex < _iconSets[_iconListIndexChoice].size()) {
            tabWidget->setTabIcon(index, _iconSets[_iconListIndexChoice][imgIndex]);
        }
    }

    // Update text if filename changed (BufferChangeFilename = 0x080)
    if (mask & 0x080) {
        updateTabText(index, buffer);
    }

    // Update tab text color based on state
    if (buffer->isDirty()) {
        tabWidget->tabBar()->setTabTextColor(index, QColor(255, 128, 0)); // Orange for unsaved
    } else {
        tabWidget->tabBar()->setTabTextColor(index, QColor());
    }
}

void DocTabView::setIndividualTabColour(BufferID bufferId, int colorId)
{
    if (!bufferId)
        return;

    bufferId->setDocColorId(colorId);

    // Apply color to tab
    int index = getIndexByBuffer(bufferId);
    if (index == -1)
        return;

    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget) return;

    // Map colorId to actual color
    QColor color;
    switch (colorId) {
        case 0: color = QColor(255, 102, 102); break; // Red
        case 1: color = QColor(255, 178, 102); break; // Orange
        case 2: color = QColor(255, 255, 102); break; // Yellow
        case 3: color = QColor(178, 255, 102); break; // Green
        case 4: color = QColor(102, 255, 255); break; // Cyan
        case 5: color = QColor(102, 178, 255); break; // Blue
        case 6: color = QColor(178, 102, 255); break; // Purple
        default: color = QColor(); break;
    }

    tabWidget->tabBar()->setTabTextColor(index, color);
}

int DocTabView::getIndividualTabColourId(int tabIndex)
{
    BufferID bufferId = getBufferByIndex(tabIndex);
    if (!bufferId)
        return -1;

    return bufferId->getDocColorId();
}

int DocTabView::getImageIndexForBuffer(Buffer* buffer)
{
    if (!buffer)
        return SAVED_IMG_INDEX;

    if (buffer->isFileMonitoringEnabled()) {
        return MONITORING_IMG_INDEX;
    }

    if (buffer->isFileReadOnly()) {
        return READONLYSYS_IMG_INDEX;
    }

    if (buffer->isUserReadOnly()) {
        return READONLY_IMG_INDEX;
    }

    if (buffer->isDirty()) {
        return UNSAVED_IMG_INDEX;
    }

    return SAVED_IMG_INDEX;
}

void DocTabView::updateTabText(int index, Buffer* buffer)
{
    QTabWidget* tabWidget = getTabWidget();
    if (!tabWidget || !buffer) return;

    QString title = buffer->getFileNameQString();
    if (title.isEmpty()) {
        title = QString("new %1").arg(index + 1);
    }

    // Encode ampersands (double them for display)
    title.replace("&", "&&");

    tabWidget->setTabText(index, title);
}

void DocTabView::reSizeTo(QRect& rc)
{
    if (_widget) {
        _widget->setGeometry(rc);
    }

    if (_pView) {
        // Adjust rect for border/padding if needed
        // Note: _pView is ScintillaEditView*, getWidget() would need proper interface
        // For now, we assume the view handles its own geometry
    }
}

} // namespace QtControls
