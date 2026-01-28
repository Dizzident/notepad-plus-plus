// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Window.h"
#include <QStatusBar>
#include <QLabel>
#include <vector>

namespace QtControls {

class StatusBar : public Window
{
public:
    StatusBar() = default;
    ~StatusBar() override = default;

    virtual bool init(QWidget* parent, int nbParts);

    void destroy() override;

    void setPartWidth(int index, int width);
    void setParts(int nbParts, const int* partWidths);

    int getHeight() const override;

    void setText(const QString& text, int partNumber = 0);
    void setText(const wchar_t* text, int partNumber = 0);

    QString getText(int partNumber = 0) const;

    void adjustParts();

    QStatusBar* getStatusBar() const { return qobject_cast<QStatusBar*>(_widget); }

private:
    std::vector<QLabel*> _partLabels;
    std::vector<int> _partWidths;
    int _nbParts = 0;
};

} // namespace QtControls
