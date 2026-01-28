// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "StatusBar.h"
#include <QMainWindow>

namespace QtControls {

bool StatusBar::init(QWidget* parent, int nbParts)
{
    if (!parent) return false;

    _parent = parent;
    _nbParts = nbParts;

    // Create status bar
    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(parent);
    if (mainWindow) {
        _widget = mainWindow->statusBar();
    } else {
        _widget = new QStatusBar(parent);
    }

    QStatusBar* statusBar = getStatusBar();
    if (!statusBar) return false;

    // Create labels for each part
    _partLabels.clear();
    _partWidths.clear();

    for (int i = 0; i < nbParts; ++i) {
        QLabel* label = new QLabel(statusBar);
        label->setFrameStyle(QFrame::NoFrame);

        if (i == 0) {
            // First part stretches
            statusBar->addWidget(label, 1);
        } else {
            // Other parts have fixed size
            statusBar->addPermanentWidget(label);
        }

        _partLabels.push_back(label);
        _partWidths.push_back(100); // Default width
    }

    return true;
}

void StatusBar::destroy()
{
    _partLabels.clear();
    _partWidths.clear();
    _nbParts = 0;
    // Status bar is owned by QMainWindow, don't delete
    _widget = nullptr;
}

void StatusBar::setPartWidth(int index, int width)
{
    if (index < 0 || index >= _nbParts) return;

    _partWidths[index] = width;

    if (index < static_cast<int>(_partLabels.size())) {
        _partLabels[index]->setMinimumWidth(width);
        _partLabels[index]->setMaximumWidth(width);
    }
}

void StatusBar::setParts(int nbParts, const int* partWidths)
{
    if (!partWidths || nbParts <= 0) return;

    // Adjust number of parts if needed
    while (_nbParts < nbParts) {
        QLabel* label = new QLabel(getStatusBar());
        getStatusBar()->addPermanentWidget(label);
        _partLabels.push_back(label);
        _partWidths.push_back(100);
        _nbParts++;
    }

    while (_nbParts > nbParts) {
        delete _partLabels.back();
        _partLabels.pop_back();
        _partWidths.pop_back();
        _nbParts--;
    }

    // Set widths
    for (int i = 0; i < nbParts; ++i) {
        setPartWidth(i, partWidths[i]);
    }
}

int StatusBar::getHeight() const
{
    QStatusBar* statusBar = getStatusBar();
    if (!statusBar) return 0;
    return statusBar->height();
}

void StatusBar::setText(const QString& text, int partNumber)
{
    if (partNumber < 0 || partNumber >= _nbParts) return;
    if (partNumber >= static_cast<int>(_partLabels.size())) return;

    _partLabels[partNumber]->setText(text);
}

void StatusBar::setText(const wchar_t* text, int partNumber)
{
    setText(QString::fromWCharArray(text), partNumber);
}

QString StatusBar::getText(int partNumber) const
{
    if (partNumber < 0 || partNumber >= _nbParts) return QString();
    if (partNumber >= static_cast<int>(_partLabels.size())) return QString();

    return _partLabels[partNumber]->text();
}

void StatusBar::adjustParts()
{
    // Adjust sizes based on content
    for (size_t i = 0; i < _partLabels.size(); ++i) {
        if (i < _partWidths.size()) {
            _partLabels[i]->setMinimumWidth(_partWidths[i]);
        }
    }
}

} // namespace QtControls
