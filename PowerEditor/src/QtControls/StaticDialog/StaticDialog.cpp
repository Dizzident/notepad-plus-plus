// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "StaticDialog.h"
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>

namespace QtControls {

StaticDialog::StaticDialog(QWidget* parent) : QDialog(parent)
{
    _widget = this;
}

StaticDialog::~StaticDialog()
{
    // Don't call destroy() here - it would delete this which is already being destroyed
    // Just clean up any resources that need explicit cleanup
    _widget = nullptr;
}

void StaticDialog::init(QWidget* parent)
{
    _parent = parent;
    if (parent) {
        setParent(parent);
    }
}

void StaticDialog::create(const QString& title, bool isRTL)
{
    _widget = this;

    QDialog* dialog = getDialog();
    if (dialog) {
        dialog->setWindowTitle(title);
        setupDialog(isRTL);
    }

    _isCreated = true;
}

void StaticDialog::setupDialog(bool isRTL)
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    if (isRTL) {
        dialog->setLayoutDirection(Qt::RightToLeft);
    }

    // Store initial rect
    _rc = dialog->geometry();
}

void StaticDialog::destroy()
{
    // Close the dialog and mark as not created
    // Don't delete _widget here since it could be 'this'
    if (_isCreated) {
        close();
        _isCreated = false;
    }
    _widget = nullptr;
}

void StaticDialog::getMappedChildRect(QWidget* child, QRect& rcChild) const
{
    if (child && _widget) {
        QPoint topLeft = child->mapTo(_widget, QPoint(0, 0));
        rcChild = QRect(topLeft, child->size());
    }
}

void StaticDialog::getMappedChildRect(int idChild, QRect& rcChild) const
{
    if (!_widget) return;

    QWidget* child = _widget->findChild<QWidget*>(QString::number(idChild));
    if (child) {
        getMappedChildRect(child, rcChild);
    }
}

void StaticDialog::getMappedChildRect(const QString& objectName, QRect& rcChild) const
{
    if (!_widget) return;

    QWidget* child = _widget->findChild<QWidget*>(objectName);
    if (child) {
        getMappedChildRect(child, rcChild);
    }
}

void StaticDialog::redrawDlgItem(const QString& objectName, bool forceUpdate) const
{
    if (!_widget) return;

    QWidget* item = _widget->findChild<QWidget*>(objectName);
    if (item) {
        item->update();
        if (forceUpdate) {
            item->repaint();
        }
    }
}

void StaticDialog::goToCenter()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    // Center on parent or screen
    if (dialog->parentWidget()) {
        dialog->move(dialog->parentWidget()->frameGeometry().center()
                     - dialog->rect().center());
    } else {
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            dialog->move(screen->geometry().center() - dialog->rect().center());
        }
    }

    dialog->show();
}

void StaticDialog::goToCenter(unsigned int /*swpFlags*/)
{
    // The swpFlags parameter is Windows-specific (SetWindowPos flags)
    // For Qt, we ignore it and use the standard centering behavior
    goToCenter();
}

bool StaticDialog::moveForDpiChange()
{
    // Qt handles DPI changes automatically
    // This method is for Windows-specific DPI handling
    return true;
}

void StaticDialog::display(bool toShow, bool enhancedPositioningCheckWhenShowing)
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    if (toShow) {
        if (enhancedPositioningCheckWhenShowing) {
            goToCenter();
        } else {
            dialog->show();
        }
    } else {
        dialog->hide();
    }
}

QRect StaticDialog::getViewablePositionRect(QRect testRc) const
{
    QScreen* screen = QApplication::primaryScreen();
    if (!screen) return testRc;

    QRect screenRect = screen->availableGeometry();

    // Ensure the rect is within screen bounds
    if (testRc.right() > screenRect.right()) {
        testRc.moveRight(screenRect.right());
    }
    if (testRc.bottom() > screenRect.bottom()) {
        testRc.moveBottom(screenRect.bottom());
    }
    if (testRc.left() < screenRect.left()) {
        testRc.moveLeft(screenRect.left());
    }
    if (testRc.top() < screenRect.top()) {
        testRc.moveTop(screenRect.top());
    }

    return testRc;
}

QPoint StaticDialog::getTopPoint(QWidget* widget, bool isLeft) const
{
    if (!widget) return QPoint();

    QPoint pos = widget->mapToGlobal(QPoint(0, 0));
    if (!isLeft) {
        pos.setX(pos.x() + widget->width());
    }
    return pos;
}

bool StaticDialog::isCheckedOrNot(const QString& checkControlName) const
{
    if (!_widget) return false;

    QCheckBox* check = _widget->findChild<QCheckBox*>(checkControlName);
    if (check) {
        return check->isChecked();
    }
    return false;
}

bool StaticDialog::isCheckedOrNot(int checkControlID) const
{
    return isCheckedOrNot(QString::number(checkControlID));
}

void StaticDialog::setChecked(const QString& checkControlName, bool checkOrNot) const
{
    if (!_widget) return;

    QCheckBox* check = _widget->findChild<QCheckBox*>(checkControlName);
    if (check) {
        check->setChecked(checkOrNot);
    }
}

void StaticDialog::setChecked(int checkControlID, bool checkOrNot) const
{
    setChecked(QString::number(checkControlID), checkOrNot);
}

bool StaticDialog::dlgProc(QWidget* hwnd, QEvent* event)
{
    // Base implementation - subclasses should override
    (void)hwnd;
    (void)event;
    return false;
}

bool StaticDialog::run_dlgProc(QEvent* event)
{
    // Base implementation - no longer pure virtual so class can be instantiated
    (void)event;
    return false;
}

} // namespace QtControls
