// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include "../../Parameters.h"
#include <QDialog>
#include <QColor>

// Forward declarations
class QComboBox;
class QListWidget;
class QCheckBox;
class QPushButton;
class QLabel;
class QGroupBox;
class QFontComboBox;
class QLineEdit;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;

enum fontStyleType {BOLD_STATUS, ITALIC_STATUS, UNDERLINE_STATUS};

namespace QtControls {

// Forward declaration
class ScintillaPreview;

// ============================================================================
// WordStyleDlg - Qt implementation of Style Configurator
// ============================================================================
class WordStyleDlg : public StaticDialog {
    Q_OBJECT

public:
    explicit WordStyleDlg(QWidget* parent = nullptr);
    ~WordStyleDlg() override;

    // Initialize the dialog
    void init();

    // Show the dialog
    void doDialog();

    // Prepare to cancel/close without saving
    void prepare2Cancel();

    // Select a theme by name
    bool selectThemeByName(const QString& themeName);

    // Navigate to a specific section (format: "Language:Style")
    bool goToSection(const QString& section);

    // Check if data has been modified
    bool isDirty() const { return _isDirty; }

    // Restore global override values
    void restoreGlobalOverrideValues();

protected:
    void setupUI();
    void connectSignals();

private slots:
    // Language and style selection
    void onLanguageChanged(int index);
    void onStyleChanged(int index);

    // Font settings
    void onFontChanged(const QString& font);
    void onFontSizeChanged(const QString& size);

    // Font style toggles
    void onBoldToggled(bool checked);
    void onItalicToggled(bool checked);
    void onUnderlineToggled(bool checked);

    // Color pickers
    void onFgColorClicked();
    void onBgColorClicked();

    // Theme management
    void onThemeChanged(int index);
    void onSaveThemeClicked();
    void onSaveAsThemeClicked();

    // Global override
    void onGlobalFgToggled(bool checked);
    void onGlobalBgToggled(bool checked);
    void onGlobalFontToggled(bool checked);
    void onGlobalFontSizeToggled(bool checked);
    void onGlobalBoldToggled(bool checked);
    void onGlobalItalicToggled(bool checked);
    void onGlobalUnderlineToggled(bool checked);

    // Navigation
    void onGoToSettingsClicked();

    // Dialog actions
    void onOkClicked();
    void onCancelClicked();

    // User keywords and extensions
    void onUserKeywordsChanged();
    void onUserExtChanged();

private:
    // UI Setup helpers
    void createLanguageSection(QGridLayout* layout, int& row);
    void createStyleSection(QGridLayout* layout, int& row);
    void createFontSection(QGridLayout* layout, int& row);
    void createColorSection(QGridLayout* layout, int& row);
    void createGlobalOverrideSection(QGridLayout* layout, int& row);
    void createPreviewSection(QGridLayout* layout, int& row);
    void createButtonSection(QGridLayout* layout, int& row);

    // Data loading
    void loadLanguages();
    void loadThemes();
    void loadFontSizes();
    void setStyleListFromLexer(int lexerIndex);
    void setVisualFromStyleList();

    // Style application
    void applyStyleChanges();
    void updatePreview();

    // Style getters/setters
    Style& getCurrentStyler();
    const Style& getCurrentStyler() const;
    bool getCurrentStyleName(QString& styleName) const;

    // Update methods
    void updateFontName();
    void updateFontSize();
    void updateFontStyleStatus(fontStyleType styleType);
    void updateColour(bool isBackground);
    void updateUserKeywords();
    void updateExtension();
    void updateGlobalOverrideCtrls();

    // Theme management
    void switchToTheme(int themeIndex);
    void updateThemeName(const QString& themeName);
    void applyCurrentSelectedThemeAndUpdateUI();

    // Utility
    int getApplicationInfo() const;
    int whichTabColourIndex() const;
    int whichIndividualTabColourId() const;
    int whichFindDlgStatusMsgColourIndex() const;
    bool isDocumentMapStyle() const;
    void notifyDataModified();
    void enableFontControls(bool enable);
    void enableColorControls(bool enable);
    QColor colorFromColorRef(COLORREF color) const;
    COLORREF colorRefFromColor(const QColor& color) const;

    // UI Components - Language selection
    QComboBox* _languageCombo = nullptr;
    QComboBox* _themeCombo = nullptr;

    // UI Components - Style list
    QListWidget* _styleList = nullptr;
    QLabel* _styleDescLabel = nullptr;

    // UI Components - Font settings
    QFontComboBox* _fontCombo = nullptr;
    QComboBox* _fontSizeCombo = nullptr;
    QCheckBox* _boldCheck = nullptr;
    QCheckBox* _italicCheck = nullptr;
    QCheckBox* _underlineCheck = nullptr;

    // UI Components - Color pickers
    QPushButton* _fgColorBtn = nullptr;
    QPushButton* _bgColorBtn = nullptr;
    QLabel* _fgColorLabel = nullptr;
    QLabel* _bgColorLabel = nullptr;

    // UI Components - Global override
    QGroupBox* _globalOverrideGroup = nullptr;
    QCheckBox* _globalFgCheck = nullptr;
    QCheckBox* _globalBgCheck = nullptr;
    QCheckBox* _globalFontCheck = nullptr;
    QCheckBox* _globalFontSizeCheck = nullptr;
    QCheckBox* _globalBoldCheck = nullptr;
    QCheckBox* _globalItalicCheck = nullptr;
    QCheckBox* _globalUnderlineCheck = nullptr;
    QPushButton* _goToSettingsBtn = nullptr;

    // UI Components - User keywords and extensions
    QLineEdit* _userKeywordsEdit = nullptr;
    QLineEdit* _userExtEdit = nullptr;

    // UI Components - Preview
    ScintillaPreview* _preview = nullptr;

    // UI Components - Buttons
    QPushButton* _saveThemeBtn = nullptr;
    QPushButton* _saveAsThemeBtn = nullptr;
    QPushButton* _okBtn = nullptr;
    QPushButton* _cancelBtn = nullptr;

    // Data
    LexerStylerArray _lsArray;
    StyleArray _globalStyles;
    QString _themeName;

    // Backup for cancel
    LexerStylerArray _styles2restored;
    StyleArray _gstyles2restored;
    GlobalOverride _gOverride2restored;

    // State
    int _currentLexerIndex = 0;
    int _currentThemeIndex = 0;
    bool _isDirty = false;
    bool _isThemeDirty = false;
    bool _isThemeChanged = false;
    bool _restoreInvalid = false;

    // Constants
    static constexpr bool C_FOREGROUND = false;
    static constexpr bool C_BACKGROUND = true;
};

// ============================================================================
// ScintillaPreview - Preview widget for style changes
// ============================================================================
class ScintillaPreview : public QWidget {
    Q_OBJECT

public:
    explicit ScintillaPreview(QWidget* parent = nullptr);
    ~ScintillaPreview() override;

    void setSampleText(const QString& text);
    void setLexer(int lexerId);
    void applyStyle(const Style& style);
    void clearStyles();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString _sampleText;
    int _lexerId = 0;
    QVector<Style> _styles;
};

} // namespace QtControls
