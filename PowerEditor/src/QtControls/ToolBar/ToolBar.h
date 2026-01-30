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
#include <QIcon>
#include <vector>
#include <memory>
#include <map>

// Enums matching Windows version - at global scope for compatibility
#ifndef TOOLBAR_STATUS_TYPE_DEFINED
enum toolBarStatusType {TB_SMALL, TB_LARGE, TB_SMALL2, TB_LARGE2, TB_STANDARD};
#define TOOLBAR_STATUS_TYPE_DEFINED
#endif

// Constants - at global scope for compatibility
constexpr int REBAR_BAR_TOOLBAR = 0;
constexpr int REBAR_BAR_SEARCH = 1;
constexpr int REBAR_BAR_EXTERNAL = 10;

namespace QtControls {

// Forward declarations
class ReBar;

// Toolbar button unit structure
struct ToolBarButtonUnit {
    int _cmdID = 0;
    int _defaultIcon = 0;
    int _grayIcon = 0;
    int _defaultIcon2 = 0;
    int _grayIcon2 = 0;
    int _defaultDarkModeIcon = 0;
    int _grayDarkModeIcon = 0;
    int _defaultDarkModeIcon2 = 0;
    int _grayDarkModeIcon2 = 0;
    int _stdIcon = 0;
};

// Icon locator for custom icons
struct iconLocator {
    size_t _listIndex = 0;
    size_t _iconIndex = 0;
    QString _iconLocation;

    iconLocator(size_t iList, size_t iIcon, const QString& iconLoc)
        : _listIndex(iList), _iconIndex(iIcon), _iconLocation(iconLoc) {}
};

// Plugin button configuration
struct ToolbarPluginButtonsConf
{
    bool _isHideAll = false;
    std::vector<bool> _showPluginButtonsArray;
};

// Dynamic button registration info
struct DynamicCmdIcoBmp {
    int _message = 0;
    QIcon _icon;
    QIcon _iconDarkMode;
    QString _tooltip;
};

// ReBar band info structure (simplified for Qt)
struct ReBarBandInfo {
    int wID = 0;
    int cx = 0;
    int cy = 0;
    int cxMinChild = 0;
    int cyMinChild = 0;
    int cyMaxChild = 0;
    int cxIdeal = 0;
    int cyIntegral = 1;
    QWidget* hwndChild = nullptr;
    int fStyle = 0;
    int fMask = 0;
};

// ToolBar class
class ToolBar : public Window
{
    Q_OBJECT

public:
    ToolBar();
    ~ToolBar() override;

    // Initialization
    void initTheme(void* toolIconsDocRoot);  // NppXml::Document
    void initHideButtonsConf(void* toolButtonsDocRoot, const ToolBarButtonUnit* buttonUnitArray, int arraySize);

    virtual bool init(QWidget* parent, toolBarStatusType type,
                      const ToolBarButtonUnit* buttonUnitArray, int arraySize);

    void destroy() override;

    // Button state
    void enable(int cmdID, bool doEnable) const;
    bool getCheckState(int ID2Check) const;
    void setCheck(int ID2Check, bool willBeChecked) const;

    // Size
    int getWidth() const override;
    int getHeight() const override;

    // Icon size changes
    void reduce();
    void enlarge();
    void reduceToSet2();
    void enlargeToSet2();
    void setToBmpIcons();

    // State
    toolBarStatusType getState() const { return _state; }

    // Custom icons
    bool change2CustomIconsIfAny();
    bool changeIcons(size_t whichLst, size_t iconIndex, const wchar_t* iconLocation) const;

    // Dynamic buttons (for plugins)
    void registerDynBtn(unsigned int message, void* iconHandles, void* absentIco = nullptr);
    void registerDynBtnDM(unsigned int message, void* iconHandles);

    // Popup for overflow
    void doPopup(QPoint chevPoint);

    // ReBar integration
    void addToRebar(ReBar* rebar);

    // DPI handling
    void resizeIconsDpi(int dpi);

    // Get underlying toolbar
    QToolBar* getToolBar() const { return qobject_cast<QToolBar*>(_widget); }

private:
    std::vector<std::unique_ptr<ToolBarButtonUnit>> _pTBB;
    std::vector<QAction*> _actions;
    std::map<int, QAction*> _cmdToAction;
    toolBarStatusType _state = TB_SMALL;
    std::vector<DynamicCmdIcoBmp> _vDynBtnReg;
    size_t _nbButtons = 0;
    size_t _nbDynButtons = 0;
    size_t _nbTotalButtons = 0;
    size_t _nbCurrentButtons = 0;
    ReBar* _pRebar = nullptr;
    ReBarBandInfo _rbBand;
    std::vector<iconLocator> _customIconVect;
    std::unique_ptr<bool[]> _toolbarStdButtonsConfArray;
    ToolbarPluginButtonsConf _toolbarPluginButtonsConf;
    void* _toolIcons = nullptr;  // NppXml::Element equivalent
    int _dpi = 96;

    // Icon lists for different states
    std::vector<QIcon> _defaultIcons;
    std::vector<QIcon> _disabledIcons;
    std::vector<QIcon> _defaultIcons2;
    std::vector<QIcon> _disabledIcons2;
    std::vector<QIcon> _defaultIconsDM;
    std::vector<QIcon> _disabledIconsDM;
    std::vector<QIcon> _defaultIconsDM2;
    std::vector<QIcon> _disabledIconsDM2;

    void reset(bool create = false);
    void setState(toolBarStatusType state);
    void setDefaultImageList();
    void setDisableImageList();
    void setDefaultImageList2();
    void setDisableImageList2();
    void setDefaultImageListDM();
    void setDisableImageListDM();
    void setDefaultImageListDM2();
    void setDisableImageListDM2();
    void setupIcons(toolBarStatusType type);
    void fillToolbar();
    void updateButtonImages();
};

// ReBar class - container for toolbars
class ReBar : public Window
{
    Q_OBJECT

public:
    ReBar();
    ~ReBar() override;

    void destroy() override;

    void init(QWidget* parent);
    bool addBand(ReBarBandInfo* rBand, bool useID);
    void reNew(int id, ReBarBandInfo* rBand);
    void removeBand(int id);

    void setIDVisible(int id, bool show);
    bool getIDVisible(int id);
    void setGrayBackground(int id);

private:
    std::vector<int> _usedIDs;
    std::map<int, ReBarBandInfo> _bands;
    std::map<int, QWidget*> _bandWidgets;

    int getNewID();
    void releaseID(int id);
    bool isIDTaken(int id);
};

} // namespace QtControls

// For compatibility with existing code that expects ToolBar/ReBar at global scope
using QtControls::ToolBar;
using QtControls::ReBar;
using QtControls::ToolBarButtonUnit;
using QtControls::iconLocator;
using QtControls::ToolbarPluginButtonsConf;
using QtControls::DynamicCmdIcoBmp;
using QtControls::ReBarBandInfo;
