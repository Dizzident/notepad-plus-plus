// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Window.h"
#include <QSplitter>
#include <QWidget>
#include <cstdint>

namespace QtControls {

// SplitterMode enum - must match Windows version
enum class SplitterMode : std::uint8_t
{
    DYNAMIC, LEFT_FIX, RIGHT_FIX
};

enum class DIRECTION
{
    RIGHT,
    LEFT
};

class SplitterContainer : public Window
{
    Q_OBJECT

public:
    SplitterContainer();
    ~SplitterContainer() override;

    void create(Window* pWin0, Window* pWin1, int splitterSize,
                SplitterMode mode = SplitterMode::DYNAMIC, int ratio = 50, bool isVertical = true);

    void destroy() override;

    void reSizeTo(QRect& rc) override;

    void display(bool toShow = true) override;

    void redraw(bool forceUpdate = false) override;

    void setWin0(Window* pWin)
    {
        _pWin0 = pWin;
        updateSplitterWidgets();
    }

    void setWin1(Window* pWin)
    {
        _pWin1 = pWin;
        updateSplitterWidgets();
    }

    bool isVertical() const
    {
        return _isVertical;
    }

    void rotate();

private:
    Window* _pWin0 = nullptr; // left or top window
    Window* _pWin1 = nullptr; // right or bottom window

    QSplitter* _splitter = nullptr;
    int _splitterSize = 0;
    int _ratio = 50;
    SplitterMode _splitterMode = SplitterMode::DYNAMIC;
    bool _isVertical = true;

    void updateSplitterWidgets();
    void rotateTo(DIRECTION direction);
    void applySplitterSizes();
};

} // namespace QtControls
