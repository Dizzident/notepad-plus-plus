// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Window.h"
#include <QToolBar>
#include <QAction>
#include <vector>
#include <memory>

namespace QtControls {

enum toolBarStatusType {TB_SMALL, TB_LARGE, TB_SMALL2, TB_LARGE2, TB_STANDARD};

struct ToolBarButtonUnit {
    int idCommand;
    int idResource;
    int style;
    QString tooltip;
};

struct iconLocator {
    size_t _listIndex = 0;
    size_t _iconIndex = 0;
    QString _iconLocation;

    iconLocator(size_t iList, size_t iIcon, const QString& iconLoc)
        : _listIndex(iList), _iconIndex(iIcon), _iconLocation(iconLoc) {}
};

struct ToolbarPluginButtonsConf
{
    bool _isHideAll = false;
    std::vector<bool> _showPluginButtonsArray;
};

class ReBar;

class ToolBar : public Window
{
public:
    ToolBar() = default;
    ~ToolBar() override = default;

    virtual bool init(QWidget* parent, toolBarStatusType type,
                      const ToolBarButtonUnit* buttonUnitArray, int arraySize);

    void destroy() override;

    void enable(int cmdID, bool doEnable) const;

    int getWidth() const override;
    int getHeight() const override;

    void reduce();
    void enlarge();
    void reduceToSet2();
    void enlargeToSet2();
    void setToBmpIcons();

    bool getCheckState(int ID2Check) const;
    void setCheck(int ID2Check, bool willBeChecked) const;

    toolBarStatusType getState() const {
        return _state;
    }

    void registerDynBtn(int messageId, void* iconHandles);

    void doPopup(QPoint chevPoint);

    void addToRebar(ReBar* rebar);

    void resizeIconsDpi(int dpi);

    QToolBar* getToolBar() const { return qobject_cast<QToolBar*>(_widget); }

private:
    std::vector<QAction*> _actions;
    toolBarStatusType _state = TB_SMALL;
    std::vector<iconLocator> _customIconVect;
    size_t _nbButtons = 0;
    size_t _nbDynButtons = 0;
    size_t _nbTotalButtons = 0;
    int _dpi = 96;

    void setupIcons(toolBarStatusType type);
    void fillToolbar();
};

class ReBar : public Window
{
public:
    void init(QWidget* parent);
    void addToolBar(ToolBar* toolBar);
    void reNew(int id, int width, int height);
    void resize(QWidget parent);
};

} // namespace QtControls
