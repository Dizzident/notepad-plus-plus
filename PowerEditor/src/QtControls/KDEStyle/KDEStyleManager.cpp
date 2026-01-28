// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.

#include "KDEStyleManager.h"

#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QProcess>
#include <QStandardPaths>
#include <QDebug>
#include <cmath>

namespace QtControls {
namespace KDE {

// Static instance initialization
KDEStyleManager* KDEStyleManager::_instance = nullptr;

// ============================================================================
// Singleton Implementation
// ============================================================================
KDEStyleManager* KDEStyleManager::instance()
{
    if (!_instance) {
        _instance = new KDEStyleManager();
    }
    return _instance;
}

// ============================================================================
// Constructor/Destructor
// ============================================================================
KDEStyleManager::KDEStyleManager(QObject* parent)
    : QObject(parent)
    , _configWatcher(std::make_unique<QFileSystemWatcher>())
{
    // Determine KDE config and data directories
    QString home = QDir::homePath();
    _kdeConfigDir = qEnvironmentVariable("XDG_CONFIG_HOME", home + "/.config");
    _kdeDataDir = qEnvironmentVariable("XDG_DATA_HOME", home + "/.local/share");

    // Connect watcher signal
    connect(_configWatcher.get(), &QFileSystemWatcher::fileChanged,
            this, &KDEStyleManager::onKDEConfigChanged);
}

KDEStyleManager::~KDEStyleManager()
{
    stopWatching();
}

// ============================================================================
// Initialization
// ============================================================================
void KDEStyleManager::initialize()
{
    if (_initialized) {
        return;
    }

    if (!isKDEPlasma()) {
        qDebug() << "KDEStyleManager: Not running under KDE Plasma";
        return;
    }

    qDebug() << "KDEStyleManager: Initializing for KDE Plasma";

    // Read all KDE configurations
    readKDEColorScheme();
    readKDEIconTheme();
    readKDEFonts();
    readKDEAnimations();
    readKDEHighDPI();

    _initialized = true;

    emit styleChanged();
}

void KDEStyleManager::applyKDEStyle()
{
    if (!_initialized) {
        initialize();
    }

    applyPalette();
    applyKDEIcons();
    applyKDEFonts();
    applyHighDPISettings();
    applyStyleSheet();

    qDebug() << "KDEStyleManager: Applied KDE style";
}

// ============================================================================
// Detection Methods
// ============================================================================
bool KDEStyleManager::isKDEPlasma()
{
    // Check XDG_CURRENT_DESKTOP
    QString desktop = qEnvironmentVariable(XDG_CURRENT_DESKTOP);
    if (desktop.contains("KDE", Qt::CaseInsensitive)) {
        return true;
    }

    // Check KDE_FULL_SESSION
    if (qEnvironmentVariableIsSet(KDE_FULL_SESSION)) {
        return true;
    }

    // Check KDE_SESSION_VERSION
    if (qEnvironmentVariableIsSet(KDE_SESSION)) {
        return true;
    }

    return false;
}

bool KDEStyleManager::isKDESession()
{
    return qEnvironmentVariableIsSet(KDE_SESSION);
}

QString KDEStyleManager::getKDEVersion()
{
    QString version = qEnvironmentVariable(KDE_SESSION);
    if (!version.isEmpty()) {
        return version;
    }

    // Try to get from dbus
    QDBusInterface interface("org.kde.plasmashell", "/PlasmaShell",
                             "org.kde.PlasmaShell");
    if (interface.isValid()) {
        QDBusMessage reply = interface.call("version");
        if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()) {
            return reply.arguments().first().toString();
        }
    }

    return QString();
}

// ============================================================================
// Color Scheme Methods
// ============================================================================
KDEColorScheme KDEStyleManager::getColorScheme() const
{
    return _colorScheme;
}

QPalette KDEStyleManager::getKDEPalette() const
{
    if (!_colorScheme.isValid) {
        return QApplication::palette();
    }

    QPalette palette;

    // Window colors
    palette.setColor(QPalette::Window, _colorScheme.windowBackground);
    palette.setColor(QPalette::WindowText, _colorScheme.windowForeground);

    // Button colors
    palette.setColor(QPalette::Button, _colorScheme.buttonBackground);
    palette.setColor(QPalette::ButtonText, _colorScheme.buttonForeground);

    // View/Base colors
    palette.setColor(QPalette::Base, _colorScheme.viewBackground);
    palette.setColor(QPalette::AlternateBase, _colorScheme.viewBackground.darker(105));
    palette.setColor(QPalette::Text, _colorScheme.viewForeground);

    // Selection colors
    palette.setColor(QPalette::Highlight, _colorScheme.selectionBackground);
    palette.setColor(QPalette::HighlightedText, _colorScheme.selectionForeground);

    // Tooltip colors
    palette.setColor(QPalette::ToolTipBase, _colorScheme.tooltipBackground);
    palette.setColor(QPalette::ToolTipText, _colorScheme.tooltipForeground);

    // Link colors
    palette.setColor(QPalette::Link, _colorScheme.linkColor);
    palette.setColor(QPalette::LinkVisited, _colorScheme.visitedLinkColor);

    // Disabled colors
    palette.setColor(QPalette::Disabled, QPalette::WindowText, _colorScheme.disabledColor);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, _colorScheme.disabledColor);
    palette.setColor(QPalette::Disabled, QPalette::Text, _colorScheme.disabledColor);

    // Accent color as focus indicator
    palette.setColor(QPalette::Active, QPalette::Highlight, _colorScheme.accentColor);

    return palette;
}

bool KDEStyleManager::isDarkMode() const
{
    return _colorScheme.isDarkTheme;
}

QColor KDEStyleManager::getAccentColor() const
{
    return _colorScheme.accentColor;
}

QColor KDEStyleManager::getWindowBackground() const
{
    return _colorScheme.windowBackground;
}

QColor KDEStyleManager::getViewBackground() const
{
    return _colorScheme.viewBackground;
}

QColor KDEStyleManager::getSelectionBackground() const
{
    return _colorScheme.selectionBackground;
}

// ============================================================================
// Icon Methods
// ============================================================================
KDEIconSettings KDEStyleManager::getIconSettings() const
{
    return _iconSettings;
}

QString KDEStyleManager::getIconTheme() const
{
    return _iconSettings.themeName;
}

QIcon KDEStyleManager::getKDEIcon(const QString& iconName) const
{
    return getKDEIcon(iconName, _iconSettings.preferredSize);
}

QIcon KDEStyleManager::getKDEIcon(const QString& iconName, int size) const
{
    if (iconName.isEmpty()) {
        return QIcon();
    }

    // Try to load from current theme first
    QIcon icon = KDEIcons::loadIconFromTheme(iconName, _iconSettings.themeName);

    // If not found, try fallbacks
    if (icon.isNull()) {
        icon = KDEIcons::loadIconWithFallbacks(iconName, _iconSettings.fallbackThemes);
    }

    // Last resort: use Qt's fallback
    if (icon.isNull()) {
        icon = QIcon::fromTheme(iconName);
    }

    return icon;
}

void KDEStyleManager::applyKDEIcons()
{
    if (!_iconSettings.isValid) {
        return;
    }

    // Set the icon theme for the application
    QIcon::setThemeName(_iconSettings.themeName);

    // Add KDE icon paths
    QStringList searchPaths = KDEIcons::getIconSearchPaths();
    for (const QString& path : searchPaths) {
        QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << path);
    }

    qDebug() << "KDEStyleManager: Applied icon theme:" << _iconSettings.themeName;
}

QString KDEStyleManager::getIconPath(const QString& iconName, int size) const
{
    QString themePath = getIconThemePath(_iconSettings.themeName);
    return KDEIcons::findIconInTheme(iconName, themePath, size);
}

// ============================================================================
// Font Methods
// ============================================================================
KDEFontSettings KDEStyleManager::getFontSettings() const
{
    return _fontSettings;
}

QFont KDEStyleManager::getGeneralFont() const
{
    return _fontSettings.generalFont;
}

QFont KDEStyleManager::getFixedFont() const
{
    return _fontSettings.fixedFont;
}

QFont KDEStyleManager::getMenuFont() const
{
    return _fontSettings.menuFont;
}

void KDEStyleManager::applyKDEFonts()
{
    if (!_fontSettings.isValid) {
        return;
    }

    QApplication::setFont(_fontSettings.generalFont);

    // Apply specific fonts to application components
    qApp->setProperty("menuFont", _fontSettings.menuFont);
    qApp->setProperty("toolbarFont", _fontSettings.toolbarFont);
    qApp->setProperty("fixedFont", _fontSettings.fixedFont);
    qApp->setProperty("smallFont", _fontSettings.smallFont);

    qDebug() << "KDEStyleManager: Applied fonts";
}

// ============================================================================
// Animation Methods
// ============================================================================
KDEAnimationSettings KDEStyleManager::getAnimationSettings() const
{
    return _animationSettings;
}

bool KDEStyleManager::animationsEnabled() const
{
    return _animationSettings.animationsEnabled;
}

int KDEStyleManager::getAnimationSpeed() const
{
    return _animationSettings.animationSpeed;
}

// ============================================================================
// High DPI Methods
// ============================================================================
qreal KDEStyleManager::getDevicePixelRatio() const
{
    return _fontSettings.dpi / 96.0;
}

void KDEStyleManager::applyHighDPISettings()
{
    // Set DPI for the application
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    // Apply font DPI
    if (_fontSettings.dpi > 0) {
        QFont font = QApplication::font();
        font.setPointSizeF(font.pointSizeF() * _fontSettings.dpi / 96.0);
        QApplication::setFont(font);
    }
}

// ============================================================================
// Watching Methods
// ============================================================================
void KDEStyleManager::startWatching()
{
    if (_watching) {
        return;
    }

    setupConfigWatcher();
    _watching = true;

    qDebug() << "KDEStyleManager: Started watching for config changes";
}

void KDEStyleManager::stopWatching()
{
    if (!_watching) {
        return;
    }

    if (_configWatcher) {
        _configWatcher->removePaths(_configWatcher->files());
        _configWatcher->removePaths(_configWatcher->directories());
    }

    _watching = false;

    qDebug() << "KDEStyleManager: Stopped watching for config changes";
}

bool KDEStyleManager::isWatching() const
{
    return _watching;
}

// ============================================================================
// Theme Methods
// ============================================================================
QString KDEStyleManager::getCurrentThemeName() const
{
    return _currentThemeName;
}

QStringList KDEStyleManager::getAvailableThemes() const
{
    QStringList themes;

    // Search in standard locations
    QStringList searchPaths;
    searchPaths << _kdeDataDir + "/color-schemes"
                << "/usr/share/color-schemes"
                << "/usr/local/share/color-schemes";

    for (const QString& path : searchPaths) {
        QDir dir(path);
        if (dir.exists()) {
            QStringList files = dir.entryList(QStringList() << "*.colors", QDir::Files);
            for (const QString& file : files) {
                themes << QFileInfo(file).baseName();
            }
        }
    }

    themes.removeDuplicates();
    return themes;
}

bool KDEStyleManager::loadTheme(const QString& themeName)
{
    QString themePath = getColorSchemePath(themeName);
    if (themePath.isEmpty() || !QFile::exists(themePath)) {
        qWarning() << "KDEStyleManager: Theme not found:" << themeName;
        return false;
    }

    KDEColorScheme newScheme = loadColorSchemeFromFile(themePath);
    if (newScheme.isValid) {
        _colorScheme = newScheme;
        _currentThemeName = themeName;
        emit themeChanged(themeName);
        emit colorSchemeChanged();
        return true;
    }

    return false;
}

// ============================================================================
// Private Slots
// ============================================================================
void KDEStyleManager::onKDEConfigChanged(const QString& path)
{
    qDebug() << "KDEStyleManager: Config file changed:" << path;

    // Determine which config changed and reload
    if (path.contains("kcmfonts")) {
        readKDEFonts();
        emit fontChanged();
    } else if (path.contains("kcmicons")) {
        readKDEIconTheme();
        emit iconThemeChanged();
    } else if (path.contains("kwinrc") || path.contains("kdeglobals")) {
        readKDEColorScheme();
        emit colorSchemeChanged();
    }

    emit styleChanged();
}

void KDEStyleManager::onIconThemeChanged()
{
    readKDEIconTheme();
    applyKDEIcons();
    emit iconThemeChanged();
}

// ============================================================================
// Configuration Reading
// ============================================================================
void KDEStyleManager::readKDEColorScheme()
{
    // Read the current color scheme from kdeglobals
    QString schemeName = readKConfigValue("kdeglobals", "General", "ColorScheme", "Breeze").toString();
    _currentThemeName = schemeName;

    QString schemePath = getColorSchemePath(schemeName);

    if (schemePath.isEmpty() || !QFile::exists(schemePath)) {
        // Try to read from kdeglobals directly
        schemePath = _kdeConfigDir + "/kdeglobals";
    }

    _colorScheme = loadColorSchemeFromFile(schemePath);

    // If we couldn't load the scheme, create a default one
    if (!_colorScheme.isValid) {
        _colorScheme.windowBackground = QColor(239, 240, 241);  // Breeze light
        _colorScheme.windowForeground = QColor(49, 54, 59);
        _colorScheme.buttonBackground = QColor(239, 240, 241);
        _colorScheme.buttonForeground = QColor(49, 54, 59);
        _colorScheme.viewBackground = QColor(252, 252, 252);
        _colorScheme.viewForeground = QColor(49, 54, 59);
        _colorScheme.selectionBackground = QColor(61, 174, 233);
        _colorScheme.selectionForeground = QColor(255, 255, 255);
        _colorScheme.tooltipBackground = QColor(49, 54, 59);
        _colorScheme.tooltipForeground = QColor(239, 240, 241);
        _colorScheme.accentColor = QColor(61, 174, 233);
        _colorScheme.linkColor = QColor(41, 128, 185);
        _colorScheme.visitedLinkColor = QColor(155, 89, 182);
        _colorScheme.focusColor = QColor(61, 174, 233);
        _colorScheme.hoverColor = QColor(147, 206, 233);
        _colorScheme.activeColor = QColor(61, 174, 233);
        _colorScheme.inactiveColor = QColor(127, 140, 141);
        _colorScheme.disabledColor = QColor(189, 195, 199);
        _colorScheme.borderColor = QColor(160, 160, 160);
        _colorScheme.separatorColor = QColor(218, 220, 221);
        _colorScheme.isDarkTheme = false;
        _colorScheme.isValid = true;
    }

    // Detect dark theme
    int bgLuminance = _colorScheme.windowBackground.lightness();
    _colorScheme.isDarkTheme = bgLuminance < 128;
}

void KDEStyleManager::readKDEIconTheme()
{
    QString themeName = readKConfigValue("kcmicons", "Icons", "Theme", "breeze").toString();

    if (themeName.isEmpty()) {
        themeName = readKConfigValue("kdeglobals", "Icons", "Theme", "breeze").toString();
    }

    _iconSettings.themeName = themeName;
    _iconSettings.preferredSize = readKConfigValue("kcmicons", "Icons", "Size", 22).toInt();

    // Read fallback themes
    QString fallbacks = readKConfigValue("kcmicons", "Icons", "FallbackThemes", "hicolor").toString();
    _iconSettings.fallbackThemes = fallbacks.split(',', Qt::SkipEmptyParts);

    _iconSettings.isValid = true;
}

void KDEStyleManager::readKDEFonts()
{
    // General font
    QString generalFontStr = readKConfigValue("kcmfonts", "General", "font",
                                               "Noto Sans,10,-1,5,50,0,0,0,0,0").toString();
    _fontSettings.generalFont = QFont(generalFontStr.split(',').first());
    _fontSettings.generalFont.setPointSize(10);

    // Fixed font
    QString fixedFontStr = readKConfigValue("kcmfonts", "General", "fixed",
                                             "Hack,10,-1,5,50,0,0,0,0,0").toString();
    _fontSettings.fixedFont = QFont(fixedFontStr.split(',').first());
    _fontSettings.fixedFont.setPointSize(10);
    _fontSettings.fixedFont.setFixedPitch(true);

    // Menu font
    QString menuFontStr = readKConfigValue("kcmfonts", "General", "menuFont",
                                            "Noto Sans,10,-1,5,50,0,0,0,0,0").toString();
    _fontSettings.menuFont = QFont(menuFontStr.split(',').first());
    _fontSettings.menuFont.setPointSize(10);

    // Toolbar font
    QString toolbarFontStr = readKConfigValue("kcmfonts", "General", "toolBarFont",
                                               "Noto Sans,10,-1,5,50,0,0,0,0,0").toString();
    _fontSettings.toolbarFont = QFont(toolbarFontStr.split(',').first());
    _fontSettings.toolbarFont.setPointSize(10);

    // Small font
    QString smallFontStr = readKConfigValue("kcmfonts", "General", "smallFont",
                                             "Noto Sans,8,-1,5,50,0,0,0,0,0").toString();
    _fontSettings.smallFont = QFont(smallFontStr.split(',').first());
    _fontSettings.smallFont.setPointSize(8);

    // DPI
    _fontSettings.dpi = readKConfigValue("kcmfonts", "General", "dpi", 96).toInt();

    _fontSettings.isValid = true;
}

void KDEStyleManager::readKDEAnimations()
{
    _animationSettings.animationsEnabled = readKConfigValue("kwinrc", "Compositing",
                                                             "AnimationSpeed", 1).toInt() > 0;
    _animationSettings.animationSpeed = readKConfigValue("kwinrc", "Compositing",
                                                          "AnimationSpeed", 100).toInt();
    _animationSettings.isValid = true;
}

void KDEStyleManager::readKDEHighDPI()
{
    // High DPI settings are typically in kcmfonts or kwinrc
    int scaleFactor = readKConfigValue("kcmfonts", "General", "scaleFactor", 1).toInt();
    if (scaleFactor > 1) {
        _fontSettings.dpi = 96 * scaleFactor;
    }
}

// ============================================================================
// KConfig File Parsing
// ============================================================================
QVariant KDEStyleManager::readKConfigValue(const QString& file, const QString& group,
                                            const QString& key, const QVariant& defaultValue) const
{
    QString filePath = _kdeConfigDir + "/" + file;
    if (!QFile::exists(filePath)) {
        return defaultValue;
    }

    QFile configFile(filePath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return defaultValue;
    }

    QTextStream stream(&configFile);
    bool inTargetGroup = false;
    QRegularExpression groupRegex("^\\[(.+)\\]$");
    QRegularExpression keyValueRegex("^([^=]+)=(.*)$");

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();

        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue;
        }

        // Check for group
        QRegularExpressionMatch groupMatch = groupRegex.match(line);
        if (groupMatch.hasMatch()) {
            inTargetGroup = (groupMatch.captured(1) == group);
            continue;
        }

        // Check for key-value pair
        if (inTargetGroup) {
            QRegularExpressionMatch kvMatch = keyValueRegex.match(line);
            if (kvMatch.hasMatch() && kvMatch.captured(1).trimmed() == key) {
                return QVariant(kvMatch.captured(2).trimmed());
            }
        }
    }

    return defaultValue;
}

QMap<QString, QString> KDEStyleManager::readKConfigGroup(const QString& file, const QString& group) const
{
    QMap<QString, QString> result;

    QString filePath = _kdeConfigDir + "/" + file;
    if (!QFile::exists(filePath)) {
        return result;
    }

    QFile configFile(filePath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return result;
    }

    QTextStream stream(&configFile);
    bool inTargetGroup = false;
    QRegularExpression groupRegex("^\\[(.+)\\]$");
    QRegularExpression keyValueRegex("^([^=]+)=(.*)$");

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue;
        }

        QRegularExpressionMatch groupMatch = groupRegex.match(line);
        if (groupMatch.hasMatch()) {
            if (inTargetGroup) {
                // We've moved to a new group, stop reading
                break;
            }
            inTargetGroup = (groupMatch.captured(1) == group);
            continue;
        }

        if (inTargetGroup) {
            QRegularExpressionMatch kvMatch = keyValueRegex.match(line);
            if (kvMatch.hasMatch()) {
                result[kvMatch.captured(1).trimmed()] = kvMatch.captured(2).trimmed();
            }
        }
    }

    return result;
}

// ============================================================================
// Color Scheme Loading
// ============================================================================
KDEColorScheme KDEStyleManager::loadColorSchemeFromFile(const QString& filePath)
{
    KDEColorScheme scheme;

    if (!QFile::exists(filePath)) {
        return scheme;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return scheme;
    }

    QTextStream stream(&file);
    QString currentGroup;
    QRegularExpression groupRegex("^\\[(.+)\\]$");
    QRegularExpression keyValueRegex("^([^=]+)=(.*)$");

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        QRegularExpressionMatch groupMatch = groupRegex.match(line);
        if (groupMatch.hasMatch()) {
            currentGroup = groupMatch.captured(1);
            continue;
        }

        QRegularExpressionMatch kvMatch = keyValueRegex.match(line);
        if (!kvMatch.hasMatch()) {
            continue;
        }

        QString key = kvMatch.captured(1).trimmed();
        QString value = kvMatch.captured(2).trimmed();

        // Parse colors based on group and key
        if (currentGroup == "Colors:Window") {
            if (key == "BackgroundNormal") scheme.windowBackground = parseKDEColor(value);
            else if (key == "ForegroundNormal") scheme.windowForeground = parseKDEColor(value);
        }
        else if (currentGroup == "Colors:Button") {
            if (key == "BackgroundNormal") scheme.buttonBackground = parseKDEColor(value);
            else if (key == "ForegroundNormal") scheme.buttonForeground = parseKDEColor(value);
        }
        else if (currentGroup == "Colors:View") {
            if (key == "BackgroundNormal") scheme.viewBackground = parseKDEColor(value);
            else if (key == "ForegroundNormal") scheme.viewForeground = parseKDEColor(value);
        }
        else if (currentGroup == "Colors:Selection") {
            if (key == "BackgroundNormal") scheme.selectionBackground = parseKDEColor(value);
            else if (key == "ForegroundNormal") scheme.selectionForeground = parseKDEColor(value);
        }
        else if (currentGroup == "Colors:Tooltip") {
            if (key == "BackgroundNormal") scheme.tooltipBackground = parseKDEColor(value);
            else if (key == "ForegroundNormal") scheme.tooltipForeground = parseKDEColor(value);
        }
        else if (currentGroup == "Colors:Complementary" || currentGroup == "General") {
            if (key == "AccentColor") scheme.accentColor = parseKDEColor(value);
            else if (key == "ColorScheme") scheme.isDarkTheme = value.toLower().contains("dark");
        }
        else if (currentGroup == "General") {
            if (key == "ColorScheme") scheme.isDarkTheme = value.toLower().contains("dark");
        }
    }

    // Set default accent color if not specified
    if (!scheme.accentColor.isValid()) {
        scheme.accentColor = QColor(61, 174, 233);  // Default Breeze blue
    }

    // Set link colors
    scheme.linkColor = scheme.accentColor.darker(110);
    scheme.visitedLinkColor = QColor(155, 89, 182);

    // Set decoration colors
    scheme.focusColor = scheme.accentColor;
    scheme.hoverColor = scheme.accentColor.lighter(120);
    scheme.activeColor = scheme.accentColor;
    scheme.inactiveColor = scheme.windowForeground;
    scheme.inactiveColor.setAlpha(128);
    scheme.disabledColor = scheme.windowForeground;
    scheme.disabledColor.setAlpha(96);

    // Set border colors
    scheme.borderColor = scheme.windowForeground;
    scheme.borderColor.setAlpha(64);
    scheme.separatorColor = scheme.windowForeground;
    scheme.separatorColor.setAlpha(32);

    // Detect dark theme based on background luminance
    if (scheme.windowBackground.isValid()) {
        scheme.isDarkTheme = scheme.windowBackground.lightness() < 128;
    }

    scheme.isValid = scheme.windowBackground.isValid() && scheme.windowForeground.isValid();

    return scheme;
}

QColor KDEStyleManager::parseKDEColor(const QString& colorStr) const
{
    // KDE colors can be in various formats:
    // - RGB values: "0,0,0" or "255,255,255"
    // - RGBA values: "0,0,0,255"
    // - Hex: "#000000" or "#00000000"

    if (colorStr.startsWith('#')) {
        return QColor(colorStr);
    }

    QStringList components = colorStr.split(',');
    if (components.size() >= 3) {
        int r = components[0].trimmed().toInt();
        int g = components[1].trimmed().toInt();
        int b = components[2].trimmed().toInt();
        int a = (components.size() >= 4) ? components[3].trimmed().toInt() : 255;
        return QColor(r, g, b, a);
    }

    return QColor();
}

// ============================================================================
// Path Utilities
// ============================================================================
QString KDEStyleManager::getKDEConfigPath() const
{
    return _kdeConfigDir;
}

QString KDEStyleManager::getKDEDataPath() const
{
    return _kdeDataDir;
}

QString KDEStyleManager::getColorSchemePath(const QString& schemeName) const
{
    QStringList searchPaths;
    searchPaths << _kdeDataDir + "/color-schemes"
                << "/usr/share/color-schemes"
                << "/usr/local/share/color-schemes";

    QString fileName = schemeName;
    if (!fileName.endsWith(".colors")) {
        fileName += ".colors";
    }

    for (const QString& path : searchPaths) {
        QString fullPath = path + "/" + fileName;
        if (QFile::exists(fullPath)) {
            return fullPath;
        }
    }

    return QString();
}

QString KDEStyleManager::getIconThemePath(const QString& themeName) const
{
    QStringList searchPaths = KDEIcons::getIconSearchPaths();

    for (const QString& path : searchPaths) {
        QString themePath = path + "/" + themeName;
        if (QDir(themePath).exists()) {
            return themePath;
        }
    }

    return QString();
}

// ============================================================================
// File System Watching
// ============================================================================
void KDEStyleManager::setupConfigWatcher()
{
    if (!_configWatcher) {
        return;
    }

    QStringList filesToWatch;
    filesToWatch << _kdeConfigDir + "/kdeglobals"
                 << _kdeConfigDir + "/kcmfonts"
                 << _kdeConfigDir + "/kcmicons"
                 << _kdeConfigDir + "/kwinrc";

    for (const QString& file : filesToWatch) {
        if (QFile::exists(file)) {
            _configWatcher->addPath(file);
        }
    }
}

// ============================================================================
// Apply Methods
// ============================================================================
void KDEStyleManager::applyPalette()
{
    QPalette palette = getKDEPalette();
    QApplication::setPalette(palette);

    // Apply to all widgets
    for (QWidget* widget : QApplication::allWidgets()) {
        widget->setPalette(palette);
    }
}

void KDEStyleManager::applyStyleSheet()
{
    if (!_colorScheme.isValid) {
        return;
    }

    QString styleSheet = QString(
        "QToolTip {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  padding: 4px;"
        "}"
        "QFrame[frameShape=\"4\"] {"  // HLine
        "  color: %4;"
        "}"
        "QFrame[frameShape=\"5\"] {"  // VLine
        "  color: %4;"
        "}"
        "QMenu::separator {"
        "  background: %4;"
        "  height: 1px;"
        "  margin: 4px 8px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: %5;"
        "  color: %6;"
        "}"
    ).arg(_colorScheme.tooltipBackground.name())
     .arg(_colorScheme.tooltipForeground.name())
     .arg(_colorScheme.borderColor.name())
     .arg(_colorScheme.separatorColor.name(0))  // With alpha
     .arg(_colorScheme.selectionBackground.name())
     .arg(_colorScheme.selectionForeground.name());

    qApp->setStyleSheet(styleSheet);
}

// ============================================================================
// KDEDetection Namespace Implementation
// ============================================================================
namespace KDEDetection {

bool isRunningUnderKDE()
{
    return KDEStyleManager::isKDEPlasma();
}

bool isPlasmaDesktop()
{
    QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
    return desktop.contains("KDE", Qt::CaseInsensitive);
}

QString getPlasmaVersion()
{
    return KDEStyleManager::getKDEVersion();
}

QString getKConfigValue(const QString& file, const QString& group,
                       const QString& key, const QString& defaultValue)
{
    KDEStyleManager* mgr = KDEStyleManager::instance();
    return mgr->readKConfigValue(file, group, key, defaultValue).toString();
}

} // namespace KDEDetection

// ============================================================================
// KDEIcons Namespace Implementation
// ============================================================================
namespace KDEIcons {

QIcon loadIconFromTheme(const QString& iconName, const QString& themeName)
{
    if (iconName.isEmpty()) {
        return QIcon();
    }

    // If theme name is specified, temporarily set it
    QString originalTheme = QIcon::themeName();
    if (!themeName.isEmpty()) {
        QIcon::setThemeName(themeName);
    }

    QIcon icon = QIcon::fromTheme(iconName);

    // Restore original theme
    if (!themeName.isEmpty()) {
        QIcon::setThemeName(originalTheme);
    }

    return icon;
}

QIcon loadIconWithFallbacks(const QString& iconName, const QStringList& fallbacks)
{
    QIcon icon = QIcon::fromTheme(iconName);
    if (!icon.isNull()) {
        return icon;
    }

    for (const QString& fallback : fallbacks) {
        icon = QIcon::fromTheme(fallback);
        if (!icon.isNull()) {
            return icon;
        }
    }

    return QIcon();
}

QString findIconInTheme(const QString& iconName, const QString& themePath, int size)
{
    if (themePath.isEmpty() || iconName.isEmpty()) {
        return QString();
    }

    // Common icon directories
    QStringList sizeDirs;
    sizeDirs << QString("%1x%2").arg(size).arg(size)
             << QString::number(size)
             << "scalable";

    QStringList categories;
    categories << "apps" << "actions" << "devices" << "places" << "mimetypes"
               << "status" << "categories" << "emblems" << "animations";

    QStringList extensions;
    extensions << ".png" << ".svg" << ".svgz" << ".xpm";

    for (const QString& sizeDir : sizeDirs) {
        for (const QString& category : categories) {
            for (const QString& ext : extensions) {
                QString path = QString("%1/%2/%3/%4%5")
                    .arg(themePath, sizeDir, category, iconName, ext);
                if (QFile::exists(path)) {
                    return path;
                }
            }
        }
    }

    return QString();
}

QStringList getIconSearchPaths()
{
    QStringList paths;

    QString home = QDir::homePath();

    // User directories
    paths << home + "/.icons"
          << home + "/.local/share/icons"
          << qEnvironmentVariable("XDG_DATA_HOME", home + "/.local/share") + "/icons";

    // System directories
    QString xdgDataDirs = qEnvironmentVariable("XDG_DATA_DIRS", "/usr/local/share:/usr/share");
    for (const QString& dir : xdgDataDirs.split(':')) {
        paths << dir + "/icons";
    }

    paths << "/usr/share/pixmaps";

    return paths;
}

} // namespace KDEIcons

} // namespace KDE
} // namespace QtControls
