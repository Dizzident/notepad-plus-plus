// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QWidget>
#include <QRect>
#include <QString>

namespace QtControls {

class Window
{
public:
    //! \name Constructors & Destructor
    //@{
    Window() = default;
    Window(const Window&) = delete;
    virtual ~Window() = default;
    //@}

    virtual void init(QWidget* parent) {
        _parent = parent;
    }

    virtual void destroy() = 0;

    virtual void display(bool toShow = true) {
        if (_widget) {
            _widget->setVisible(toShow);
        }
    }

    virtual void reSizeTo(QRect& rc) {
        if (_widget) {
            _widget->setGeometry(rc);
            redraw();
        }
    }

    virtual void reSizeToWH(QRect& rc) {
        if (_widget) {
            _widget->setGeometry(rc.left(), rc.top(), rc.width(), rc.height());
            redraw();
        }
    }

    virtual void redraw(bool forceUpdate = false) {
        if (_widget) {
            _widget->update();
            if (forceUpdate) {
                // Qt doesn't have exact equivalent to UpdateWindow
                // but repaint() forces immediate repaint
                _widget->repaint();
            }
        }
    }

    virtual void getClientRect(QRect& rc) const {
        if (_widget) {
            rc = _widget->rect();
        }
    }

    virtual void getWindowRect(QRect& rc) const {
        if (_widget) {
            rc = _widget->frameGeometry();
        }
    }

    virtual int getWidth() const {
        if (_widget) {
            return _widget->width();
        }
        return 0;
    }

    virtual int getHeight() const {
        if (_widget) {
            if (_widget->isVisible())
                return _widget->height();
        }
        return 0;
    }

    virtual bool isVisible() const {
        if (_widget) {
            return _widget->isVisible();
        }
        return false;
    }

    QWidget* getWidget() const {
        return _widget;
    }

    QWidget* getParent() const {
        return _parent;
    }

    void grabFocus() const {
        if (_widget) {
            _widget->setFocus();
        }
    }

    Window& operator = (const Window&) = delete;

protected:
    QWidget* _parent = nullptr;
    QWidget* _widget = nullptr;
};

} // namespace QtControls
