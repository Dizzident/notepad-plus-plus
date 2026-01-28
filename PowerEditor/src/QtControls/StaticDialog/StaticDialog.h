// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Window.h"
#include <QDialog>
#include <QCheckBox>

namespace QtControls {

enum class PosAlign { left, right, top, bottom };

class StaticDialog : public Window
{
public:
    ~StaticDialog() override;

    virtual void create(const QString& title = QString(), bool isRTL = false);

    virtual bool isCreated() const {
        return (_widget != nullptr);
    }

    void getMappedChildRect(QWidget* child, QRect& rcChild) const;
    void getMappedChildRect(int idChild, QRect& rcChild) const;
    void redrawDlgItem(const QString& objectName, bool forceUpdate = false) const;

    void goToCenter();
    bool moveForDpiChange();

    void display(bool toShow = true, bool enhancedPositioningCheckWhenShowing = false);

    QRect getViewablePositionRect(QRect testRc) const;

    QPoint getTopPoint(QWidget* widget, bool isLeft = true) const;

    bool isCheckedOrNot(const QString& checkControlName) const;
    bool isCheckedOrNot(int checkControlID) const;

    void setChecked(const QString& checkControlName, bool checkOrNot = true) const;
    void setChecked(int checkControlID, bool checkOrNot = true) const;

    void destroy() override;

protected:
    QRect _rc{};

    QDialog* getDialog() const { return qobject_cast<QDialog*>(_widget); }

    static bool dlgProc(QWidget* hwnd, QEvent* event);
    virtual bool run_dlgProc(QEvent* event) = 0;

    void setupDialog(bool isRTL);
};

} // namespace QtControls
