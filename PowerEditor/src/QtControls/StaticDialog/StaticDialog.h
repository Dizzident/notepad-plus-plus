// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <QDialog>
#include <QCheckBox>

namespace QtControls {

enum class PosAlign { left, right, top, bottom };

class StaticDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StaticDialog(QWidget* parent = nullptr);
    ~StaticDialog() override;

    // Initialize the dialog with a parent widget (compatibility with Window class)
    virtual void init(QWidget* parent);

    virtual void create(const QString& title = QString(), bool isRTL = false);

    virtual bool isCreated() const {
        return _isCreated;
    }

    // Get the underlying widget (compatibility with Window class)
    QWidget* getWidget() const { return _widget; }

    void getMappedChildRect(QWidget* child, QRect& rcChild) const;
    void getMappedChildRect(int idChild, QRect& rcChild) const;
    void getMappedChildRect(const QString& objectName, QRect& rcChild) const;
    void redrawDlgItem(const QString& objectName, bool forceUpdate = false) const;

    // Redraw the entire dialog (compatibility with Windows code)
    void redraw(bool forceUpdate = false) {
        update();
        if (forceUpdate) {
            repaint();
        }
    }

    void goToCenter();
    void goToCenter(unsigned int swpFlags);
    bool moveForDpiChange();

    void display(bool toShow = true, bool enhancedPositioningCheckWhenShowing = false);

    QRect getViewablePositionRect(QRect testRc) const;

    QPoint getTopPoint(QWidget* widget, bool isLeft = true) const;

    bool isCheckedOrNot(const QString& checkControlName) const;
    bool isCheckedOrNot(int checkControlID) const;

    void setChecked(const QString& checkControlName, bool checkOrNot = true) const;
    void setChecked(int checkControlID, bool checkOrNot = true) const;

    void destroy();

protected:
    QWidget* _parent = nullptr;
    QWidget* _widget = nullptr;
    QRect _rc{};
    bool _isCreated = false;

    QDialog* getDialog() const { return const_cast<StaticDialog*>(this); }

    virtual void setupUI() {}
    virtual void connectSignals() {}

    static bool dlgProc(QWidget* hwnd, QEvent* event);
    virtual bool run_dlgProc(QEvent* event);

    void setupDialog(bool isRTL);
};

} // namespace QtControls
