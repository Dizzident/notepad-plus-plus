// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "SplitterContainer.h"
#include <QDebug>

namespace QtControls {

SplitterContainer::SplitterContainer()
{
}

SplitterContainer::~SplitterContainer()
{
    destroy();
}

void SplitterContainer::create(Window* pWin0, Window* pWin1, int splitterSize,
                               SplitterMode mode, int ratio, bool isVertical)
{
    _pWin0 = pWin0;
    _pWin1 = pWin1;
    _splitterSize = splitterSize;
    _splitterMode = mode;
    _ratio = ratio;
    _isVertical = isVertical;

    if (_parent) {
        _splitter = new QSplitter(isVertical ? Qt::Vertical : Qt::Horizontal, _parent);
        _widget = _splitter;

        // Set handle width
        _splitter->setHandleWidth(splitterSize);

        // Configure based on mode
        if (mode == SplitterMode::DYNAMIC) {
            _splitter->setOpaqueResize(true);
        } else {
            // For fixed modes, make one side non-collapsible
            _splitter->setOpaqueResize(false);
        }

        updateSplitterWidgets();
        applySplitterSizes();

        _splitter->show();
    }
}

void SplitterContainer::destroy()
{
    if (_splitter) {
        // Remove widgets but don't delete them (they're managed elsewhere)
        while (_splitter->count() > 0) {
            QWidget* w = _splitter->widget(0);
            if (w) {
                w->setParent(nullptr);
            }
        }
        delete _splitter;
        _splitter = nullptr;
        _widget = nullptr;
    }
}

void SplitterContainer::reSizeTo(QRect& rc)
{
    if (_widget) {
        _widget->setGeometry(rc);
        applySplitterSizes();
    }
}

void SplitterContainer::display(bool toShow)
{
    if (_widget) {
        _widget->setVisible(toShow);
    }
    if (_pWin0) {
        _pWin0->display(toShow);
    }
    if (_pWin1) {
        _pWin1->display(toShow);
    }
}

void SplitterContainer::redraw(bool forceUpdate)
{
    if (_pWin0) {
        _pWin0->redraw(forceUpdate);
    }
    if (_pWin1) {
        _pWin1->redraw(forceUpdate);
    }
    if (_widget) {
        _widget->update();
        if (forceUpdate) {
            _widget->repaint();
        }
    }
}

void SplitterContainer::rotate()
{
    rotateTo(DIRECTION::RIGHT);
}

void SplitterContainer::rotateTo(DIRECTION direction)
{
    (void)direction;

    if (!_splitter) {
        return;
    }

    // Toggle orientation
    _isVertical = !_isVertical;
    _splitter->setOrientation(_isVertical ? Qt::Vertical : Qt::Horizontal);

    // Update sizes after rotation
    applySplitterSizes();
}

void SplitterContainer::updateSplitterWidgets()
{
    if (!_splitter) {
        return;
    }

    // Clear existing widgets
    while (_splitter->count() > 0) {
        QWidget* w = _splitter->widget(0);
        if (w) {
            w->setParent(nullptr);
        }
    }

    // Add windows to splitter
    if (_pWin0 && _pWin0->getWidget()) {
        _splitter->addWidget(_pWin0->getWidget());
    }
    if (_pWin1 && _pWin1->getWidget()) {
        _splitter->addWidget(_pWin1->getWidget());
    }

    // Set stretch factors
    if (_splitter->count() >= 2) {
        _splitter->setStretchFactor(0, 1);
        _splitter->setStretchFactor(1, 1);
    }
}

void SplitterContainer::applySplitterSizes()
{
    if (!_splitter || _splitter->count() < 2) {
        return;
    }

    QList<int> sizes;
    int totalSize = _isVertical ? _splitter->height() : _splitter->width();

    if (totalSize <= 0) {
        // Widget not yet shown, use ratio
        int size0 = (_ratio * 1000) / 100;
        int size1 = 1000 - size0;
        sizes << size0 << size1;
    } else {
        int size0 = (totalSize * _ratio) / 100;
        int size1 = totalSize - size0 - _splitterSize;
        if (size1 < 0) {
            size1 = 0;
        }
        sizes << size0 << size1;
    }

    _splitter->setSizes(sizes);
}

} // namespace QtControls
