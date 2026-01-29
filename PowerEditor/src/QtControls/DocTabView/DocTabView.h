// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../TabBar/TabBar.h"
#include "../../QtCore/Buffer.h"

#include <QMap>
#include <QIcon>

// Image index constants for tab icons
const int SAVED_IMG_INDEX = 0;
const int UNSAVED_IMG_INDEX = 1;
const int READONLY_IMG_INDEX = 2;
const int READONLYSYS_IMG_INDEX = 3;
const int MONITORING_IMG_INDEX = 4;

// Buffer change mask (mirrors Windows version)
// Note: This is defined in ScintillaComponent/Buffer.h as well
// We use a different name to avoid redefinition errors
const int DocTabBufferChangeMask = 0x3FF;

// Forward declaration
class ScintillaEditView;

namespace QtControls {

// Bring QtCore::Buffer into this namespace for compatibility
using Buffer = QtCore::Buffer;

// BufferID type alias - in Qt version, we use Buffer* directly
using BufferID = Buffer*;
const BufferID BUFFER_INVALID = nullptr;

/**
 * @brief DocTabView - Qt implementation of document tab view
 *
 * This class manages document tabs in the Qt version of Notepad++.
 * It provides tab management functionality including:
 * - Adding/removing buffer tabs
 * - Activating buffers
 * - Finding buffers by name
 * - Updating tab appearance based on buffer state
 */
class DocTabView : public TabBarPlus
{
    Q_OBJECT

public:
    DocTabView();
    ~DocTabView() override;

    // Initialization
    void init(QWidget* parent, ScintillaEditView* pView, unsigned char indexChoice = 0, unsigned char buttonsStatus = 0);

    // Buffer management
    void addBuffer(BufferID buffer);
    void closeBuffer(BufferID buffer);
    void bufferUpdated(Buffer* buffer, int mask);

    bool activateBuffer(BufferID buffer);

    BufferID activeBuffer();
    BufferID findBufferByName(const wchar_t* fullfilename);

    int getIndexByBuffer(BufferID id);
    BufferID getBufferByIndex(size_t index);

    void setBuffer(size_t index, BufferID id);

    // Icon management
    void createIconSets();
    void changeIconSet(unsigned char choice);

    // Tab colors
    void setIndividualTabColour(BufferID bufferId, int colorId);
    int getIndividualTabColourId(int tabIndex);

    // Accessors
    ScintillaEditView* getScintillaEditView() const { return _pView; }

    // Resize override
    void reSizeTo(QRect& rc);

protected:
    ScintillaEditView* _pView = nullptr;

    // Buffer to tab index mapping
    QMap<BufferID, int> _bufferToIndex;

    // Icon sets for different themes - each set is a list of icons
    QList<QList<QIcon>> _iconSets;
    int _iconListIndexChoice = 0;

    // Helper methods
    int getImageIndexForBuffer(Buffer* buffer);
    void updateTabText(int index, Buffer* buffer);
};

} // namespace QtControls
