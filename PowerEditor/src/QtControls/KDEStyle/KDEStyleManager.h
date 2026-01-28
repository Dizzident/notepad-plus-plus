// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.

#pragma once

#include <QObject>
#include <QPalette>
#include <QString>
#include <QColor>
#include <QFont>
#include <QIcon>
#include <QFileSystemWatcher>
#include <QSettings>
#include <memory>

namespace QtControls {
namespace KDE {

// ============================================================================
// KDEColorScheme - Represents a Plasma color scheme
// ============================================================================
struct KDEColorScheme {
    // Background colors
    QColor windowBackground;
    QColor buttonBackground;
    QColor viewBackground;
    QColor selectionBackground;
    QColor tooltipBackground;

    // Foreground colors
    QColor windowForeground;
    QColor buttonForeground;
    QColor viewForeground;
    QColor selectionForeground;
    QColor tooltipForeground;

    // Accent colors
    QColor accentColor;
    QColor accentHoverColor;
    QColor accentActiveColor;
    QColor linkColor;
    QColor visitedLinkColor;

    // Decoration colors
    QColor focusColor;
    QColor hoverColor;
    QColor activeColor;
    QColor inactiveColor;
    QColor disabledColor;

    // Border colors
    QColor borderColor;
    QColor separatorColor;

    // Dark mode detection
    bool isDarkTheme = false;

    // Validity
    bool isValid = false;
};

// ============================================================================
// KDEFontSettings - Represents KDE font configuration
// ============================================================================
struct KDEFontSettings {
    QFont generalFont;
    QFont fixedFont;
    QFont menuFont;
    QFont toolbarFont;
    QFont smallFont;
    int dpi = 96;
    bool isValid = false;
};

// ============================================================================
// KDEIconSettings - Represents KDE icon theme configuration
// ============================================================================
struct KDEIconSettings {
    QString themeName;
    QStringList fallbackThemes;
    int preferredSize = 22;
    bool isValid = false;
};

// ============================================================================
// KDEAnimationSettings - Represents KDE animation settings
// ============================================================================
struct KDEAnimationSettings {
    bool animationsEnabled = true;
    int animationSpeed = 100; // percentage
    bool isValid = false;
};

// ============================================================================
// KDEStyleManager - Manages KDE Plasma theming integration
// ============================================================================
class KDEStyleManager : public QObject {
    Q_OBJECT

public:
    // Singleton access
    static KDEStyleManager* instance();

    // Initialization
    void initialize();
    void applyKDEStyle();

    // Detection
    static bool isKDEPlasma();
    static bool isKDESession();
    static QString getKDEVersion();

    // Color scheme
    KDEColorScheme getColorScheme() const;
    QPalette getKDEPalette() const;
    bool isDarkMode() const;
    QColor getAccentColor() const;
    QColor getWindowBackground() const;
    QColor getViewBackground() const;
    QColor getSelectionBackground() const;

    // Icons
    KDEIconSettings getIconSettings() const;
    QString getIconTheme() const;
    QIcon getKDEIcon(const QString& iconName) const;
    QIcon getKDEIcon(const QString& iconName, int size) const;
    void applyKDEIcons();
    QString getIconPath(const QString& iconName, int size = 22) const;

    // Fonts
    KDEFontSettings getFontSettings() const;
    QFont getGeneralFont() const;
    QFont getFixedFont() const;
    QFont getMenuFont() const;
    void applyKDEFonts();

    // Animations
    KDEAnimationSettings getAnimationSettings() const;
    bool animationsEnabled() const;
    int getAnimationSpeed() const;

    // High DPI
    qreal getDevicePixelRatio() const;
    void applyHighDPISettings();

    // Watch for changes
    void startWatching();
    void stopWatching();
    bool isWatching() const;

    // Theme names
    QString getCurrentThemeName() const;
    QStringList getAvailableThemes() const;
    bool loadTheme(const QString& themeName);

signals:
    void colorSchemeChanged();
    void iconThemeChanged();
    void fontChanged();
    void styleChanged();
    void themeChanged(const QString& themeName);

private slots:
    void onKDEConfigChanged(const QString& path);
    void onIconThemeChanged();

private:
    explicit KDEStyleManager(QObject* parent = nullptr);
    ~KDEStyleManager() override;

    // Configuration reading
    void readKDEColorScheme();
    void readKDEIconTheme();
    void readKDEFonts();
    void readKDEAnimations();
    void readKDEHighDPI();

    // KConfig file parsing
    QVariant readKConfigValue(const QString& file, const QString& group,
                               const QString& key, const QVariant& defaultValue = QVariant()) const;
    QMap<QString, QString> readKConfigGroup(const QString& file, const QString& group) const;

    // Color scheme loading
    KDEColorScheme loadColorSchemeFromFile(const QString& filePath);
    QColor parseKDEColor(const QString& colorStr) const;

    // Icon theme loading
    void loadIconThemeIndex(const QString& themePath);

    // Path utilities
    QString getKDEConfigPath() const;
    QString getKDEDataPath() const;
    QString getColorSchemePath(const QString& schemeName) const;
    QString getIconThemePath(const QString& themeName) const;

    // File system watching
    void setupConfigWatcher();

    // Apply methods
    void applyPalette();
    void applyStyleSheet();

    // Member variables
    KDEColorScheme _colorScheme;
    KDEFontSettings _fontSettings;
    KDEIconSettings _iconSettings;
    KDEAnimationSettings _animationSettings;

    std::unique_ptr<QFileSystemWatcher> _configWatcher;
    QString _kdeConfigDir;
    QString _kdeDataDir;
    QString _currentThemeName;
    bool _watching = false;
    bool _initialized = false;

    // Singleton instance
    static KDEStyleManager* _instance;

    // Constants
    static constexpr const char* KWIN_COMPOSITOR = "KWIN_COMPOSE";
    static constexpr const char* KDE_SESSION = "KDE_SESSION_VERSION";
    static constexpr const char* XDG_CURRENT_DESKTOP = "XDG_CURRENT_DESKTOP";
    static constexpr const char* KDE_FULL_SESSION = "KDE_FULL_SESSION";
};

// ============================================================================
// Utility functions for KDE detection
// ============================================================================
namespace KDEDetection {
    bool isRunningUnderKDE();
    bool isPlasmaDesktop();
    QString getPlasmaVersion();
    QString getKConfigValue(const QString& file, const QString& group,
                           const QString& key, const QString& defaultValue = QString());
}

// ============================================================================
// Utility functions for icon handling
// ============================================================================
namespace KDEIcons {
    QIcon loadIconFromTheme(const QString& iconName, const QString& themeName = QString());
    QIcon loadIconWithFallbacks(const QString& iconName, const QStringList& fallbacks);
    QString findIconInTheme(const QString& iconName, const QString& themePath, int size);
    QStringList getIconSearchPaths();
}

} // namespace KDE
} // namespace QtControls
