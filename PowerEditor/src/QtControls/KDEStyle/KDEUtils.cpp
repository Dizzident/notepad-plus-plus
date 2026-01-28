// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.

#include "KDEUtils.h"
#include "KDEStyleManager.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace QtControls {
namespace KDE {

// ============================================================================
// KDE Detection Utilities
// ============================================================================

bool IsKDEPlasma()
{
    return KDEStyleManager::isKDEPlasma();
}

bool IsKDESession()
{
    return KDEStyleManager::isKDESession();
}

QString GetKDEVersion()
{
    return KDEStyleManager::getKDEVersion();
}

int GetKDEMajorVersion()
{
    QString version = GetKDEVersion();
    if (version.isEmpty()) {
        return 0;
    }
    return version.split('.').first().toInt();
}

// ============================================================================
// Theme Detection Utilities
// ============================================================================

bool IsDarkTheme()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return false;
    }
    return mgr->isDarkMode();
}

QString GetCurrentColorScheme()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QString();
    }
    return mgr->getCurrentThemeName();
}

QString GetCurrentIconTheme()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QString();
    }
    return mgr->getIconTheme();
}

QString GetCurrentWidgetStyle()
{
    // Read from kdeglobals
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QString();
    }
    return mgr->readKConfigValue("kdeglobals", "General", "widgetStyle", "Breeze").toString();
}

// ============================================================================
// Color Utilities
// ============================================================================

QColor GetAccentColor()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QColor(61, 174, 233);  // Default Breeze blue
    }
    return mgr->getAccentColor();
}

QColor GetWindowBackgroundColor()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QColor(239, 240, 241);  // Default Breeze light
    }
    return mgr->getWindowBackground();
}

QColor GetViewBackgroundColor()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QColor(252, 252, 252);  // Default Breeze view
    }
    return mgr->getViewBackground();
}

QColor GetSelectionBackgroundColor()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QColor(61, 174, 233);  // Default Breeze accent
    }
    return mgr->getSelectionBackground();
}

QColor GetTextColor()
{
    KDEColorScheme scheme = KDEStyleManager::instance()->getColorScheme();
    if (!scheme.isValid) {
        return QColor(49, 54, 59);  // Default Breeze text
    }
    return scheme.windowForeground;
}

QColor GetDisabledTextColor()
{
    KDEColorScheme scheme = KDEStyleManager::instance()->getColorScheme();
    if (!scheme.isValid) {
        return QColor(189, 195, 199);  // Default Breeze disabled
    }
    return scheme.disabledColor;
}

QColor GetBorderColor()
{
    KDEColorScheme scheme = KDEStyleManager::instance()->getColorScheme();
    if (!scheme.isValid) {
        return QColor(160, 160, 160);  // Default Breeze border
    }
    return scheme.borderColor;
}

QPalette GetKDEPalette()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QApplication::palette();
    }
    return mgr->getKDEPalette();
}

// ============================================================================
// Font Utilities
// ============================================================================

QFont GetGeneralFont()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QApplication::font();
    }
    return mgr->getGeneralFont();
}

QFont GetFixedFont()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        QFont font("monospace");
        font.setFixedPitch(true);
        return font;
    }
    return mgr->getFixedFont();
}

QFont GetMenuFont()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QApplication::font();
    }
    KDEFontSettings settings = mgr->getFontSettings();
    return settings.menuFont;
}

QFont GetToolbarFont()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QApplication::font();
    }
    KDEFontSettings settings = mgr->getFontSettings();
    return settings.toolbarFont;
}

QFont GetSmallFont()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        QFont font = QApplication::font();
        font.setPointSize(font.pointSize() - 2);
        return font;
    }
    KDEFontSettings settings = mgr->getFontSettings();
    return settings.smallFont;
}

int GetFontDPI()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return 96;
    }
    KDEFontSettings settings = mgr->getFontSettings();
    return settings.dpi;
}

// ============================================================================
// Icon Utilities
// ============================================================================

QString GetIconPath(const QString& iconName, int size)
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QString();
    }
    return mgr->getIconPath(iconName, size);
}

QStringList GetIconFallbackThemes()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QStringList() << "hicolor";
    }
    KDEIconSettings settings = mgr->getIconSettings();
    return settings.fallbackThemes;
}

QStringList GetIconSearchPaths()
{
    return KDEIcons::getIconSearchPaths();
}

// ============================================================================
// Animation Utilities
// ============================================================================

bool AnimationsEnabled()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return true;  // Default to enabled
    }
    return mgr->animationsEnabled();
}

int GetAnimationSpeed()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return 100;
    }
    return mgr->getAnimationSpeed();
}

int GetAnimationDuration(const QString& animationType)
{
    if (!AnimationsEnabled()) {
        return 0;
    }

    int speed = GetAnimationSpeed();
    int baseDuration = 150;  // Default base duration in ms

    // Different animation types have different base durations
    if (animationType == "tooltip") {
        baseDuration = 100;
    } else if (animationType == "menu") {
        baseDuration = 150;
    } else if (animationType == "dialog") {
        baseDuration = 200;
    } else if (animationType == "fade") {
        baseDuration = 120;
    } else if (animationType == "slide") {
        baseDuration = 200;
    }

    // Adjust by speed percentage
    return (baseDuration * speed) / 100;
}

// ============================================================================
// High DPI Utilities
// ============================================================================

qreal GetDevicePixelRatio()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return 1.0;
    }
    return mgr->getDevicePixelRatio();
}

bool IsHighDPIEnabled()
{
    // Check if Qt high DPI scaling is enabled
    return qApp->testAttribute(Qt::AA_EnableHighDpiScaling);
}

qreal GetScaleFactor()
{
    // Read from kcmfonts or kwinrc
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return 1.0;
    }

    int dpi = GetFontDPI();
    return dpi / 96.0;
}

// ============================================================================
// Configuration Path Utilities
// ============================================================================

QString GetKDEConfigDir()
{
    QString home = QDir::homePath();
    return qEnvironmentVariable("XDG_CONFIG_HOME", home + "/.config");
}

QString GetKDEDataDir()
{
    QString home = QDir::homePath();
    return qEnvironmentVariable("XDG_DATA_HOME", home + "/.local/share");
}

QString GetKDEConfigFile(const QString& filename)
{
    return GetKDEConfigDir() + "/" + filename;
}

QString GetColorSchemePath(const QString& schemeName)
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QString();
    }
    return mgr->getColorSchemePath(schemeName);
}

QString GetIconThemePath(const QString& themeName)
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return QString();
    }
    return mgr->getIconThemePath(themeName);
}

// ============================================================================
// Quick Apply Functions
// ============================================================================

void ApplyKDEStyle()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return;
    }

    mgr->initialize();
    mgr->applyKDEStyle();
    mgr->startWatching();
}

void ApplyKDEPalette()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return;
    }

    mgr->initialize();
    QPalette palette = mgr->getKDEPalette();
    QApplication::setPalette(palette);
}

void ApplyKDEFonts()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return;
    }

    mgr->initialize();
    mgr->applyKDEFonts();
}

void ApplyKDEIcons()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return;
    }

    mgr->initialize();
    mgr->applyKDEIcons();
}

void ApplyKDEHighDPI()
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    if (!mgr) {
        return;
    }

    mgr->applyHighDPISettings();
}

} // namespace KDE
} // namespace QtControls
