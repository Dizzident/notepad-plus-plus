// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.

#pragma once

#include <QString>
#include <QStringList>
#include <QColor>
#include <QPalette>
#include <QFont>

namespace QtControls {
namespace KDE {

// ============================================================================
// KDE Detection Utilities
// ============================================================================

//! Check if running under KDE Plasma desktop environment
bool IsKDEPlasma();

//! Check if running in a full KDE session
bool IsKDESession();

//! Get the KDE Plasma version string
QString GetKDEVersion();

//! Get the KDE Plasma major version number (5 or 6)
int GetKDEMajorVersion();

// ============================================================================
// Theme Detection Utilities
// ============================================================================

//! Check if the current Plasma theme is a dark theme
bool IsDarkTheme();

//! Get the name of the current Plasma color scheme
QString GetCurrentColorScheme();

//! Get the name of the current Plasma icon theme
QString GetCurrentIconTheme();

//! Get the name of the current Plasma widget style
QString GetCurrentWidgetStyle();

// ============================================================================
// Color Utilities
// ============================================================================

//! Get the accent color from KDE settings
QColor GetAccentColor();

//! Get the window background color from KDE settings
QColor GetWindowBackgroundColor();

//! Get the view/background color from KDE settings
QColor GetViewBackgroundColor();

//! Get the selection background color from KDE settings
QColor GetSelectionBackgroundColor();

//! Get the text color from KDE settings
QColor GetTextColor();

//! Get the disabled text color from KDE settings
QColor GetDisabledTextColor();

//! Get the border color from KDE settings
QColor GetBorderColor();

//! Get a complete QPalette from KDE settings
QPalette GetKDEPalette();

// ============================================================================
// Font Utilities
// ============================================================================

//! Get the general/application font from KDE settings
QFont GetGeneralFont();

//! Get the fixed-width font from KDE settings (for editor)
QFont GetFixedFont();

//! Get the menu font from KDE settings
QFont GetMenuFont();

//! Get the toolbar font from KDE settings
QFont GetToolbarFont();

//! Get the small font from KDE settings
QFont GetSmallFont();

//! Get the DPI setting from KDE
int GetFontDPI();

// ============================================================================
// Icon Utilities
// ============================================================================

//! Get the path to an icon from the current theme
QString GetIconPath(const QString& iconName, int size = 22);

//! Get the list of fallback icon themes
QStringList GetIconFallbackThemes();

//! Get the list of icon search paths
QStringList GetIconSearchPaths();

// ============================================================================
// Animation Utilities
// ============================================================================

//! Check if animations are enabled in KDE settings
bool AnimationsEnabled();

//! Get the animation speed setting (0-100)
int GetAnimationSpeed();

//! Get the animation duration for a specific type of animation
int GetAnimationDuration(const QString& animationType);

// ============================================================================
// High DPI Utilities
// ============================================================================

//! Get the device pixel ratio from KDE settings
qreal GetDevicePixelRatio();

//! Check if high DPI scaling is enabled
bool IsHighDPIEnabled();

//! Get the scale factor from KDE settings
qreal GetScaleFactor();

// ============================================================================
// Configuration Path Utilities
// ============================================================================

//! Get the KDE configuration directory
QString GetKDEConfigDir();

//! Get the KDE data directory
QString GetKDEDataDir();

//! Get the path to a specific KDE config file
QString GetKDEConfigFile(const QString& filename);

//! Get the path to a color scheme file
QString GetColorSchemePath(const QString& schemeName);

//! Get the path to an icon theme directory
QString GetIconThemePath(const QString& themeName);

// ============================================================================
// Quick Apply Functions
// ============================================================================

//! Apply all KDE settings to the application (palette, fonts, icons)
void ApplyKDEStyle();

//! Apply only the KDE color palette
void ApplyKDEPalette();

//! Apply only the KDE fonts
void ApplyKDEFonts();

//! Apply only the KDE icons
void ApplyKDEIcons();

//! Apply KDE high DPI settings
void ApplyKDEHighDPI();

// ============================================================================
// Convenience Macros
// ============================================================================

#define KDE_STYLE_INIT() \
    do { \
        if (QtControls::KDE::IsKDEPlasma()) { \
            QtControls::KDE::ApplyKDEStyle(); \
        } \
    } while(0)

#define KDE_PALETTE_INIT() \
    do { \
        if (QtControls::KDE::IsKDEPlasma()) { \
            QtControls::KDE::ApplyKDEPalette(); \
        } \
    } while(0)

} // namespace KDE
} // namespace QtControls
