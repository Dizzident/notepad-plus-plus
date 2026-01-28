// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../Dialogs.h"

// Forward declarations for KDE Frameworks classes
// These are used to avoid including KDE headers directly in the header file
class QWidget;
class QString;
class QStringList;

namespace Platform {
namespace KDE {

// ============================================================================
// KDE Dialogs Implementation
// ============================================================================
// This class provides native KDE Plasma file dialogs and message boxes.
// It uses KDE Frameworks (KFileDialog, KMessageBox, etc.) when available,
// and falls back to Qt dialogs when KDE is not available.
//
// Features:
// - Native KDE File Dialogs with Plasma look and feel
// - KDE Message Boxes with native styling
// - KDE Input Dialogs
// - Native Plasma notification support
// - KIO integration for remote files (sftp, smb, etc.)
// - KDE Wallet integration for secure storage
// ============================================================================

class KDEDialogs : public IDialogs {
public:
    KDEDialogs();
    ~KDEDialogs() override;

    // ------------------------------------------------------------------------
    // Factory and Availability Check
    // ------------------------------------------------------------------------

    /// Check if KDE Frameworks is available at runtime
    static bool isKDEAvailable();

    /// Create a new KDEDialogs instance
    static IDialogs* create();

    /// Set the parent widget for all dialogs
    void setParent(QWidget* parent);

    // ------------------------------------------------------------------------
    // Message Boxes
    // ------------------------------------------------------------------------

    DialogResult messageBox(const std::wstring& message,
                           const std::wstring& title,
                           MessageBoxType type,
                           MessageBoxIcon icon,
                           DialogResult defaultButton) override;

    void showInfo(const std::wstring& message, const std::wstring& title) override;
    void showWarning(const std::wstring& message, const std::wstring& title) override;
    void showError(const std::wstring& message, const std::wstring& title) override;
    bool askYesNo(const std::wstring& message, const std::wstring& title) override;
    DialogResult askYesNoCancel(const std::wstring& message, const std::wstring& title) override;
    bool askRetryCancel(const std::wstring& message, const std::wstring& title) override;

    // ------------------------------------------------------------------------
    // File Dialogs
    // ------------------------------------------------------------------------

    std::wstring showOpenFileDialog(const std::wstring& title,
                                   const std::vector<FileFilter>& filters,
                                   const FileDialogOptions& options) override;

    std::vector<std::wstring> showOpenFilesDialog(const std::wstring& title,
                                                 const std::vector<FileFilter>& filters,
                                                 const FileDialogOptions& options) override;

    std::wstring showSaveFileDialog(const std::wstring& title,
                                   const std::vector<FileFilter>& filters,
                                   const std::wstring& defaultFileName,
                                   const FileDialogOptions& options) override;

    // ------------------------------------------------------------------------
    // Folder Dialogs
    // ------------------------------------------------------------------------

    std::wstring showFolderDialog(const std::wstring& title,
                                 const FolderDialogOptions& options) override;

    // ------------------------------------------------------------------------
    // Input Dialogs
    // ------------------------------------------------------------------------

    bool showInputDialog(const std::wstring& title,
                        const std::wstring& prompt,
                        std::wstring& value,
                        bool isPassword) override;

    bool showInputDialogEx(const InputDialogOptions& options,
                          std::wstring& value) override;

    bool showMultiLineInputDialog(const std::wstring& title,
                                 const std::wstring& prompt,
                                 std::wstring& value) override;

    int showListDialog(const std::wstring& title,
                      const std::wstring& prompt,
                      const std::vector<std::wstring>& items,
                      int defaultIndex) override;

    // ------------------------------------------------------------------------
    // Custom Dialogs
    // ------------------------------------------------------------------------

    DialogResult showCustomDialog(void* dialogData) override;

    // ------------------------------------------------------------------------
    // Dialog Utilities
    // ------------------------------------------------------------------------

    void centerDialog(void* dialogHandle) override;
    void setDialogPosition(void* dialogHandle, int x, int y) override;
    void getDialogPosition(void* dialogHandle, int& x, int& y) override;
    void setDialogSize(void* dialogHandle, int width, int height) override;
    void getDialogSize(void* dialogHandle, int& width, int& height) override;
    void setDialogTitle(void* dialogHandle, const std::wstring& title) override;
    void enableDialog(void* dialogHandle, bool enable) override;
    bool isDialogEnabled(void* dialogHandle) override;
    void bringToFront(void* dialogHandle) override;
    void setModal(void* dialogHandle, bool modal) override;

    // ------------------------------------------------------------------------
    // KDE-Specific Features
    // ------------------------------------------------------------------------

    /// Show a native Plasma notification
    void showNotification(const std::wstring& title, const std::wstring& message);

    /// Show a KDE file dialog with KIO integration for remote files
    std::wstring showKIOFileDialog(const std::wstring& title,
                                   const std::vector<FileFilter>& filters,
                                   const std::wstring& startDir,
                                   bool openMode);

    /// Store a password securely using KDE Wallet
    bool storePassword(const std::wstring& wallet, const std::wstring& key, const std::wstring& password);

    /// Retrieve a password from KDE Wallet
    bool retrievePassword(const std::wstring& wallet, const std::wstring& key, std::wstring& password);

private:
    class Impl;
    std::unique_ptr<Impl> _impl;

    // Helper methods
    QString convertToQString(const std::wstring& str) const;
    std::wstring convertToWString(const QString& str) const;
    QString buildKDEFilterString(const std::vector<FileFilter>& filters) const;
};

// ============================================================================
// Convenience Functions for KDE-Specific Features
// ============================================================================
namespace KDEDialogsUtils {

/// Show a Plasma notification
void showPlasmaNotification(const std::wstring& title, const std::wstring& message);

/// Check if running in a KDE Plasma session
bool isPlasmaSession();

/// Get the KDE version string
std::wstring getKDEVersion();

/// Open a file with the default KDE application
bool openFileWithKDE(const std::wstring& filePath);

} // namespace KDEDialogsUtils

} // namespace KDE
} // namespace Platform
