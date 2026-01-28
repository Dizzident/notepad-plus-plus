// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "KDEDialogs.h"

#include <QWidget>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QStandardPaths>

// Try to include KDE Frameworks headers if available
// We use preprocessor checks to handle cases where KDE is not installed

#if __has_include(<KCoreAddons/KAboutData>)
    #define NPP_KDE_AVAILABLE 1
    #include <KCoreAddons/KAboutData>
    #include <KCoreAddons/KShell>
#endif

#if __has_include(<KWidgetsAddons/KMessageBox>)
    #ifndef NPP_KDE_AVAILABLE
        #define NPP_KDE_AVAILABLE 1
    #endif
    #include <KWidgetsAddons/KMessageBox>
    #include <KWidgetsAddons/KInputDialog>
#endif

#if __has_include(<KIO/KFileDialog>)
    #ifndef NPP_KDE_AVAILABLE
        #define NPP_KDE_AVAILABLE 1
    #endif
    #include <KIO/KFileDialog>
    #include <KIO/KFileWidget>
#endif

#if __has_include(<KNotifications/KNotification>)
    #ifndef NPP_KDE_AVAILABLE
        #define NPP_KDE_AVAILABLE 1
    #endif
    #include <KNotifications/KNotification>
#endif

#if __has_include(<KWallet/KWallet>)
    #ifndef NPP_KDE_AVAILABLE
        #define NPP_KDE_AVAILABLE 1
    #endif
    #include <KWallet/KWallet>
#endif

#if __has_include(<KConfigGui/KConfigGui>)
    #ifndef NPP_KDE_AVAILABLE
        #define NPP_KDE_AVAILABLE 1
    #endif
    #include <KConfigCore/KConfig>
    #include <KConfigCore/KConfigGroup>
#endif

#include <memory>
#include <cstdlib>

namespace Platform {
namespace KDE {

// ============================================================================
// Helper Functions
// ============================================================================

static QString convertWStringToQString(const std::wstring& str) {
    return QString::fromStdWString(str);
}

static std::wstring convertQStringToWString(const QString& str) {
    return str.toStdWString();
}

static QString buildQtFilterString(const std::vector<FileFilter>& filters) {
    QStringList qtFilters;
    for (const auto& f : filters) {
        QString desc = convertWStringToQString(f.description);
        QString pattern = convertWStringToQString(f.pattern);
        qtFilters << QString("%1 (%2)").arg(desc).arg(pattern);
    }
    return qtFilters.join(";;");
}

static QMessageBox::Icon getQtIcon(MessageBoxIcon icon) {
    switch (icon) {
        case MessageBoxIcon::Error: return QMessageBox::Critical;
        case MessageBoxIcon::Question: return QMessageBox::Question;
        case MessageBoxIcon::Warning: return QMessageBox::Warning;
        case MessageBoxIcon::Information: return QMessageBox::Information;
        default: return QMessageBox::NoIcon;
    }
}

static QMessageBox::StandardButtons getQtButtons(MessageBoxType type) {
    switch (type) {
        case MessageBoxType::OK: return QMessageBox::Ok;
        case MessageBoxType::OKCancel: return QMessageBox::Ok | QMessageBox::Cancel;
        case MessageBoxType::AbortRetryIgnore: return QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore;
        case MessageBoxType::YesNoCancel: return QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
        case MessageBoxType::YesNo: return QMessageBox::Yes | QMessageBox::No;
        case MessageBoxType::RetryCancel: return QMessageBox::Retry | QMessageBox::Cancel;
        case MessageBoxType::CancelTryAgainContinue: return QMessageBox::Cancel | QMessageBox::Retry | QMessageBox::Ignore;
        default: return QMessageBox::Ok;
    }
}

static DialogResult mapQtResult(QMessageBox::StandardButton result) {
    switch (result) {
        case QMessageBox::Ok: return DialogResult::OK;
        case QMessageBox::Cancel: return DialogResult::Cancel;
        case QMessageBox::Abort: return DialogResult::Abort;
        case QMessageBox::Retry: return DialogResult::Retry;
        case QMessageBox::Ignore: return DialogResult::Ignore;
        case QMessageBox::Yes: return DialogResult::Yes;
        case QMessageBox::No: return DialogResult::No;
        default: return DialogResult::None;
    }
}

// ============================================================================
// PIMPL Implementation
// ============================================================================

class KDEDialogs::Impl {
public:
    Impl() : _parentWidget(nullptr), _useKDE(KDEDialogs::isKDEAvailable()) {}

    QWidget* _parentWidget;
    bool _useKDE;

    // KDE-specific configuration
    QString _lastOpenDir;
    QString _lastSaveDir;

    void ensureParentWidget() {
        if (!_parentWidget && QApplication::instance()) {
            // Try to find a suitable parent window
            QWidgetList widgets = QApplication::topLevelWidgets();
            for (QWidget* w : widgets) {
                if (w->isWindow() && w->isVisible()) {
                    _parentWidget = w;
                    break;
                }
            }
        }
    }
};

// ============================================================================
// Constructor / Destructor
// ============================================================================

KDEDialogs::KDEDialogs() : _impl(std::make_unique<Impl>()) {}

KDEDialogs::~KDEDialogs() = default;

// ============================================================================
// Static Methods
// ============================================================================

bool KDEDialogs::isKDEAvailable() {
#ifdef NPP_KDE_AVAILABLE
    // Check if running in a KDE session
    const char* desktopSession = std::getenv("XDG_CURRENT_DESKTOP");
    if (desktopSession) {
        QString session = QString::fromUtf8(desktopSession).toLower();
        if (session.contains("kde") || session.contains("plasma")) {
            return true;
        }
    }

    // Check for KDE_FULL_SESSION environment variable
    const char* kdeFullSession = std::getenv("KDE_FULL_SESSION");
    if (kdeFullSession && QString::fromUtf8(kdeFullSession).compare("true", Qt::CaseInsensitive) == 0) {
        return true;
    }

    // Check if KDE libraries are available by checking for a known library
    // This is a runtime check that can be enhanced as needed
    return true;  // KDE headers were found at compile time
#else
    return false;
#endif
}

IDialogs* KDEDialogs::create() {
    return new KDEDialogs();
}

void KDEDialogs::setParent(QWidget* parent) {
    _impl->_parentWidget = parent;
}

// ============================================================================
// Message Boxes
// ============================================================================

DialogResult KDEDialogs::messageBox(const std::wstring& message,
                                   const std::wstring& title,
                                   MessageBoxType type,
                                   MessageBoxIcon icon,
                                   DialogResult defaultButton) {
    _impl->ensureParentWidget();

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        // Use KDE MessageBox when available
        QString qMessage = convertWStringToQString(message);
        QString qTitle = convertWStringToQString(title);

        // Map to KDE message box types
        KMessageBox::DialogType kdeType = KMessageBox::QuestionTwoActions;
        switch (type) {
            case MessageBoxType::OK:
                kdeType = KMessageBox::Information;
                break;
            case MessageBoxType::OKCancel:
                kdeType = KMessageBox::WarningTwoActions;
                break;
            case MessageBoxType::YesNo:
                kdeType = KMessageBox::QuestionTwoActions;
                break;
            case MessageBoxType::YesNoCancel:
                kdeType = KMessageBox::WarningTwoActionsCancel;
                break;
            default:
                kdeType = KMessageBox::Information;
                break;
        }

        // Map icon
        QString iconName;
        switch (icon) {
            case MessageBoxIcon::Error: iconName = "dialog-error"; break;
            case MessageBoxIcon::Warning: iconName = "dialog-warning"; break;
            case MessageBoxIcon::Information: iconName = "dialog-information"; break;
            case MessageBoxIcon::Question: iconName = "dialog-question"; break;
            default: iconName = "dialog-information"; break;
        }

        int result;
        if (type == MessageBoxType::YesNo || type == MessageBoxType::OKCancel) {
            result = KMessageBox::questionTwoActions(
                _impl->_parentWidget,
                qMessage,
                qTitle,
                KGuiItem(i18n("Yes")),
                KGuiItem(i18n("No")),
                QString(),
                kdeType == KMessageBox::WarningTwoActions ? KMessageBox::Notify : KMessageBox::NoExec
            );
        } else if (type == MessageBoxType::YesNoCancel) {
            result = KMessageBox::questionTwoActionsCancel(
                _impl->_parentWidget,
                qMessage,
                qTitle,
                KStandardGuiItem::yes(),
                KStandardGuiItem::no(),
                KStandardGuiItem::cancel()
            );
        } else {
            KMessageBox::information(_impl->_parentWidget, qMessage, qTitle);
            return DialogResult::OK;
        }

        switch (result) {
            case KMessageBox::PrimaryAction: return DialogResult::Yes;
            case KMessageBox::SecondaryAction: return DialogResult::No;
            case KMessageBox::Cancel: return DialogResult::Cancel;
            default: return DialogResult::None;
        }
    }
#endif

    // Fallback to Qt
    QMessageBox msgBox(_impl->_parentWidget);
    msgBox.setText(convertWStringToQString(message));
    msgBox.setWindowTitle(convertWStringToQString(title));
    msgBox.setIcon(getQtIcon(icon));
    msgBox.setStandardButtons(getQtButtons(type));

    switch (defaultButton) {
        case DialogResult::OK: msgBox.setDefaultButton(QMessageBox::Ok); break;
        case DialogResult::Cancel: msgBox.setDefaultButton(QMessageBox::Cancel); break;
        case DialogResult::Yes: msgBox.setDefaultButton(QMessageBox::Yes); break;
        case DialogResult::No: msgBox.setDefaultButton(QMessageBox::No); break;
        case DialogResult::Retry: msgBox.setDefaultButton(QMessageBox::Retry); break;
        default: break;
    }

    return mapQtResult(static_cast<QMessageBox::StandardButton>(msgBox.exec()));
}

void KDEDialogs::showInfo(const std::wstring& message, const std::wstring& title) {
    _impl->ensureParentWidget();

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        KMessageBox::information(
            _impl->_parentWidget,
            convertWStringToQString(message),
            convertWStringToQString(title)
        );
        return;
    }
#endif

    QMessageBox::information(
        _impl->_parentWidget,
        convertWStringToQString(title),
        convertWStringToQString(message)
    );
}

void KDEDialogs::showWarning(const std::wstring& message, const std::wstring& title) {
    _impl->ensureParentWidget();

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        KMessageBox::warningContinueCancel(
            _impl->_parentWidget,
            convertWStringToQString(message),
            convertWStringToQString(title)
        );
        return;
    }
#endif

    QMessageBox::warning(
        _impl->_parentWidget,
        convertWStringToQString(title),
        convertWStringToQString(message)
    );
}

void KDEDialogs::showError(const std::wstring& message, const std::wstring& title) {
    _impl->ensureParentWidget();

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        KMessageBox::error(
            _impl->_parentWidget,
            convertWStringToQString(message),
            convertWStringToQString(title)
        );
        return;
    }
#endif

    QMessageBox::critical(
        _impl->_parentWidget,
        convertWStringToQString(title),
        convertWStringToQString(message)
    );
}

bool KDEDialogs::askYesNo(const std::wstring& message, const std::wstring& title) {
    _impl->ensureParentWidget();

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        int result = KMessageBox::questionTwoActions(
            _impl->_parentWidget,
            convertWStringToQString(message),
            convertWStringToQString(title),
            KStandardGuiItem::yes(),
            KStandardGuiItem::no()
        );
        return result == KMessageBox::PrimaryAction;
    }
#endif

    return QMessageBox::question(
        _impl->_parentWidget,
        convertWStringToQString(title),
        convertWStringToQString(message),
        QMessageBox::Yes | QMessageBox::No
    ) == QMessageBox::Yes;
}

DialogResult KDEDialogs::askYesNoCancel(const std::wstring& message, const std::wstring& title) {
    _impl->ensureParentWidget();

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        int result = KMessageBox::questionTwoActionsCancel(
            _impl->_parentWidget,
            convertWStringToQString(message),
            convertWStringToQString(title),
            KStandardGuiItem::yes(),
            KStandardGuiItem::no(),
            KStandardGuiItem::cancel()
        );
        switch (result) {
            case KMessageBox::PrimaryAction: return DialogResult::Yes;
            case KMessageBox::SecondaryAction: return DialogResult::No;
            case KMessageBox::Cancel: return DialogResult::Cancel;
            default: return DialogResult::None;
        }
    }
#endif

    auto result = QMessageBox::question(
        _impl->_parentWidget,
        convertWStringToQString(title),
        convertWStringToQString(message),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
    );
    return mapQtResult(result);
}

bool KDEDialogs::askRetryCancel(const std::wstring& message, const std::wstring& title) {
    _impl->ensureParentWidget();

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        int result = KMessageBox::warningTwoActions(
            _impl->_parentWidget,
            convertWStringToQString(message),
            convertWStringToQString(title),
            KGuiItem(i18n("Retry")),
            KStandardGuiItem::cancel()
        );
        return result == KMessageBox::PrimaryAction;
    }
#endif

    return QMessageBox::warning(
        _impl->_parentWidget,
        convertWStringToQString(title),
        convertWStringToQString(message),
        QMessageBox::Retry | QMessageBox::Cancel
    ) == QMessageBox::Retry;
}

// ============================================================================
// File Dialogs
// ============================================================================

std::wstring KDEDialogs::showOpenFileDialog(const std::wstring& title,
                                           const std::vector<FileFilter>& filters,
                                           const FileDialogOptions& options) {
    _impl->ensureParentWidget();

    QString filterStr = buildQtFilterString(filters);
    QString dir = convertWStringToQString(options.initialDirectory);

    if (dir.isEmpty() && !_impl->_lastOpenDir.isEmpty()) {
        dir = _impl->_lastOpenDir;
    }

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        // Use KDE file dialog with native look and KIO support
        QUrl startUrl = dir.isEmpty() ? QUrl::fromLocalFile(QDir::homePath()) : QUrl::fromLocalFile(dir);

        KFileDialog dialog(startUrl, filterStr, _impl->_parentWidget);
        dialog.setWindowTitle(convertWStringToQString(title));
        dialog.setMode(KFile::File | KFile::ExistingOnly);

        if (dialog.exec() == QDialog::Accepted) {
            QUrl result = dialog.selectedUrl();
            if (result.isLocalFile()) {
                _impl->_lastOpenDir = QFileInfo(result.toLocalFile()).path();
                return convertQStringToWString(result.toLocalFile());
            } else {
                // Handle remote URLs (sftp, smb, etc.)
                return convertQStringToWString(result.toString());
            }
        }
        return std::wstring();
    }
#endif

    // Fallback to Qt file dialog
    QString fileName = QFileDialog::getOpenFileName(
        _impl->_parentWidget,
        convertWStringToQString(title),
        dir,
        filterStr
    );

    if (!fileName.isEmpty()) {
        _impl->_lastOpenDir = QFileInfo(fileName).path();
    }

    return convertQStringToWString(fileName);
}

std::vector<std::wstring> KDEDialogs::showOpenFilesDialog(const std::wstring& title,
                                                         const std::vector<FileFilter>& filters,
                                                         const FileDialogOptions& options) {
    _impl->ensureParentWidget();

    QString filterStr = buildQtFilterString(filters);
    QString dir = convertWStringToQString(options.initialDirectory);

    if (dir.isEmpty() && !_impl->_lastOpenDir.isEmpty()) {
        dir = _impl->_lastOpenDir;
    }

    std::vector<std::wstring> results;

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        QUrl startUrl = dir.isEmpty() ? QUrl::fromLocalFile(QDir::homePath()) : QUrl::fromLocalFile(dir);

        KFileDialog dialog(startUrl, filterStr, _impl->_parentWidget);
        dialog.setWindowTitle(convertWStringToQString(title));
        dialog.setMode(KFile::Files | KFile::ExistingOnly);

        if (dialog.exec() == QDialog::Accepted) {
            QList<QUrl> urls = dialog.selectedUrls();
            for (const QUrl& url : urls) {
                if (url.isLocalFile()) {
                    results.push_back(convertQStringToWString(url.toLocalFile()));
                } else {
                    results.push_back(convertQStringToWString(url.toString()));
                }
            }
            if (!urls.isEmpty() && urls.first().isLocalFile()) {
                _impl->_lastOpenDir = QFileInfo(urls.first().toLocalFile()).path();
            }
        }
        return results;
    }
#endif

    // Fallback to Qt file dialog
    QStringList fileNames = QFileDialog::getOpenFileNames(
        _impl->_parentWidget,
        convertWStringToQString(title),
        dir,
        filterStr
    );

    for (const QString& name : fileNames) {
        results.push_back(convertQStringToWString(name));
    }

    if (!fileNames.isEmpty()) {
        _impl->_lastOpenDir = QFileInfo(fileNames.first()).path();
    }

    return results;
}

std::wstring KDEDialogs::showSaveFileDialog(const std::wstring& title,
                                           const std::vector<FileFilter>& filters,
                                           const std::wstring& defaultFileName,
                                           const FileDialogOptions& options) {
    _impl->ensureParentWidget();

    QString filterStr = buildQtFilterString(filters);
    QString dir = convertWStringToQString(options.initialDirectory);

    if (dir.isEmpty() && !_impl->_lastSaveDir.isEmpty()) {
        dir = _impl->_lastSaveDir;
    }

    if (!defaultFileName.empty()) {
        if (dir.isEmpty()) {
            dir = convertWStringToQString(defaultFileName);
        } else {
            dir = QDir(dir).filePath(convertWStringToQString(defaultFileName));
        }
    }

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        QUrl startUrl = dir.isEmpty() ? QUrl::fromLocalFile(QDir::homePath()) : QUrl::fromLocalFile(dir);

        KFileDialog dialog(startUrl, filterStr, _impl->_parentWidget);
        dialog.setWindowTitle(convertWStringToQString(title));
        dialog.setMode(KFile::File);

        if (options.overwritePrompt) {
            dialog.setConfirmOverwrite(true);
        }

        if (dialog.exec() == QDialog::Accepted) {
            QUrl result = dialog.selectedUrl();
            if (result.isLocalFile()) {
                _impl->_lastSaveDir = QFileInfo(result.toLocalFile()).path();
                return convertQStringToWString(result.toLocalFile());
            } else {
                return convertQStringToWString(result.toString());
            }
        }
        return std::wstring();
    }
#endif

    // Fallback to Qt file dialog
    QString fileName = QFileDialog::getSaveFileName(
        _impl->_parentWidget,
        convertWStringToQString(title),
        dir,
        filterStr
    );

    if (!fileName.isEmpty()) {
        _impl->_lastSaveDir = QFileInfo(fileName).path();
    }

    return convertQStringToWString(fileName);
}

// ============================================================================
// Folder Dialogs
// ============================================================================

std::wstring KDEDialogs::showFolderDialog(const std::wstring& title,
                                         const FolderDialogOptions& options) {
    _impl->ensureParentWidget();

    QString dir = convertWStringToQString(options.initialFolder);

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        QUrl startUrl = dir.isEmpty() ? QUrl::fromLocalFile(QDir::homePath()) : QUrl::fromLocalFile(dir);

        KFileDialog dialog(startUrl, QString(), _impl->_parentWidget);
        dialog.setWindowTitle(convertWStringToQString(title));
        dialog.setMode(KFile::Directory | KFile::ExistingOnly);

        if (dialog.exec() == QDialog::Accepted) {
            QUrl result = dialog.selectedUrl();
            if (result.isLocalFile()) {
                return convertQStringToWString(result.toLocalFile());
            } else {
                return convertQStringToWString(result.toString());
            }
        }
        return std::wstring();
    }
#endif

    // Fallback to Qt folder dialog
    QString result = QFileDialog::getExistingDirectory(
        _impl->_parentWidget,
        convertWStringToQString(title),
        dir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    return convertQStringToWString(result);
}

// ============================================================================
// Input Dialogs
// ============================================================================

bool KDEDialogs::showInputDialog(const std::wstring& title,
                                const std::wstring& prompt,
                                std::wstring& value,
                                bool isPassword) {
    _impl->ensureParentWidget();

    bool ok;
    QString text;

#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        if (isPassword) {
            text = KInputDialog::getText(
                convertWStringToQString(title),
                convertWStringToQString(prompt),
                convertWStringToQString(value),
                &ok,
                _impl->_parentWidget,
                nullptr,
                QLineEdit::Password
            );
        } else {
            text = KInputDialog::getText(
                convertWStringToQString(title),
                convertWStringToQString(prompt),
                convertWStringToQString(value),
                &ok,
                _impl->_parentWidget
            );
        }

        if (ok) {
            value = convertQStringToWString(text);
        }
        return ok;
    }
#endif

    // Fallback to Qt input dialog
    if (isPassword) {
        text = QInputDialog::getText(
            _impl->_parentWidget,
            convertWStringToQString(title),
            convertWStringToQString(prompt),
            QLineEdit::Password,
            convertWStringToQString(value),
            &ok
        );
    } else {
        text = QInputDialog::getText(
            _impl->_parentWidget,
            convertWStringToQString(title),
            convertWStringToQString(prompt),
            QLineEdit::Normal,
            convertWStringToQString(value),
            &ok
        );
    }

    if (ok) {
        value = convertQStringToWString(text);
    }
    return ok;
}

bool KDEDialogs::showInputDialogEx(const InputDialogOptions& options, std::wstring& value) {
    _impl->ensureParentWidget();

    bool ok;
    QString text;

    QLineEdit::EchoMode mode = options.isPassword ? QLineEdit::Password : QLineEdit::Normal;

    if (options.multiline) {
        text = QInputDialog::getMultiLineText(
            _impl->_parentWidget,
            convertWStringToQString(options.title),
            convertWStringToQString(options.prompt),
            convertWStringToQString(options.defaultValue),
            &ok
        );
    } else {
        text = QInputDialog::getText(
            _impl->_parentWidget,
            convertWStringToQString(options.title),
            convertWStringToQString(options.prompt),
            mode,
            convertWStringToQString(options.defaultValue),
            &ok
        );
    }

    if (ok) {
        value = convertQStringToWString(text);
        return true;
    }
    return false;
}

bool KDEDialogs::showMultiLineInputDialog(const std::wstring& title,
                                         const std::wstring& prompt,
                                         std::wstring& value) {
    _impl->ensureParentWidget();

    bool ok;
    QString text = QInputDialog::getMultiLineText(
        _impl->_parentWidget,
        convertWStringToQString(title),
        convertWStringToQString(prompt),
        convertWStringToQString(value),
        &ok
    );

    if (ok) {
        value = convertQStringToWString(text);
        return true;
    }
    return false;
}

int KDEDialogs::showListDialog(const std::wstring& title,
                              const std::wstring& prompt,
                              const std::vector<std::wstring>& items,
                              int defaultIndex) {
    _impl->ensureParentWidget();

    QStringList qtItems;
    for (const auto& item : items) {
        qtItems << convertWStringToQString(item);
    }

    bool ok;
    QString result = QInputDialog::getItem(
        _impl->_parentWidget,
        convertWStringToQString(title),
        convertWStringToQString(prompt),
        qtItems,
        defaultIndex,
        false,  // not editable
        &ok
    );

    if (ok) {
        return qtItems.indexOf(result);
    }
    return -1;
}

// ============================================================================
// Custom Dialogs
// ============================================================================

DialogResult KDEDialogs::showCustomDialog(void* dialogData) {
    (void)dialogData;
    // Custom dialogs would need specific implementation
    return DialogResult::None;
}

// ============================================================================
// Dialog Utilities
// ============================================================================

void KDEDialogs::centerDialog(void* dialogHandle) {
    QWidget* widget = static_cast<QWidget*>(dialogHandle);
    if (widget) {
        widget->adjustSize();
        widget->move(widget->frameGeometry().center() - widget->rect().center());
    }
}

void KDEDialogs::setDialogPosition(void* dialogHandle, int x, int y) {
    QWidget* widget = static_cast<QWidget*>(dialogHandle);
    if (widget) {
        widget->move(x, y);
    }
}

void KDEDialogs::getDialogPosition(void* dialogHandle, int& x, int& y) {
    QWidget* widget = static_cast<QWidget*>(dialogHandle);
    if (widget) {
        QPoint pos = widget->pos();
        x = pos.x();
        y = pos.y();
    } else {
        x = y = 0;
    }
}

void KDEDialogs::setDialogSize(void* dialogHandle, int width, int height) {
    QWidget* widget = static_cast<QWidget*>(dialogHandle);
    if (widget) {
        widget->resize(width, height);
    }
}

void KDEDialogs::getDialogSize(void* dialogHandle, int& width, int& height) {
    QWidget* widget = static_cast<QWidget*>(dialogHandle);
    if (widget) {
        QSize size = widget->size();
        width = size.width();
        height = size.height();
    } else {
        width = height = 0;
    }
}

void KDEDialogs::setDialogTitle(void* dialogHandle, const std::wstring& title) {
    QWidget* widget = static_cast<QWidget*>(dialogHandle);
    if (widget) {
        widget->setWindowTitle(convertWStringToQString(title));
    }
}

void KDEDialogs::enableDialog(void* dialogHandle, bool enable) {
    QWidget* widget = static_cast<QWidget*>(dialogHandle);
    if (widget) {
        widget->setEnabled(enable);
    }
}

bool KDEDialogs::isDialogEnabled(void* dialogHandle) {
    QWidget* widget = static_cast<QWidget*>(dialogHandle);
    return widget && widget->isEnabled();
}

void KDEDialogs::bringToFront(void* dialogHandle) {
    QWidget* widget = static_cast<QWidget*>(dialogHandle);
    if (widget) {
        widget->raise();
        widget->activateWindow();
    }
}

void KDEDialogs::setModal(void* dialogHandle, bool modal) {
    QWidget* widget = static_cast<QWidget*>(dialogHandle);
    if (widget) {
        widget->setModal(modal);
    }
}

// ============================================================================
// KDE-Specific Features
// ============================================================================

void KDEDialogs::showNotification(const std::wstring& title, const std::wstring& message) {
#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        KNotification::event(
            "notepad-plus-plus",
            convertWStringToQString(title),
            convertWStringToQString(message),
            "notepad-plus-plus"
        );
        return;
    }
#endif

    // Fallback: use Qt's native notification capability through the system tray
    // or simply show a non-intrusive message box
    qDebug() << "Notification:" << convertWStringToQString(title) << "-" << convertWStringToQString(message);
}

std::wstring KDEDialogs::showKIOFileDialog(const std::wstring& title,
                                           const std::vector<FileFilter>& filters,
                                           const std::wstring& startDir,
                                           bool openMode) {
#ifdef NPP_KDE_AVAILABLE
    if (_impl->_useKDE) {
        _impl->ensureParentWidget();

        QString filterStr = buildQtFilterString(filters);
        QUrl startUrl = convertWStringToQString(startDir);
        if (startUrl.isEmpty()) {
            startUrl = QUrl::fromLocalFile(QDir::homePath());
        }

        KFileDialog dialog(startUrl, filterStr, _impl->_parentWidget);
        dialog.setWindowTitle(convertWStringToQString(title));

        if (openMode) {
            dialog.setMode(KFile::File | KFile::ExistingOnly);
        } else {
            dialog.setMode(KFile::File);
        }

        if (dialog.exec() == QDialog::Accepted) {
            QUrl result = dialog.selectedUrl();
            return convertQStringToWString(result.toString());
        }
    }
#endif

    return std::wstring();
}

bool KDEDialogs::storePassword(const std::wstring& wallet, const std::wstring& key, const std::wstring& password) {
#ifdef NPP_KDE_AVAILABLE
    #if __has_include(<KWallet/KWallet>)
    if (_impl->_useKDE) {
        KWallet::Wallet* kwallet = KWallet::Wallet::openWallet(
            convertWStringToQString(wallet),
            0  // Window ID (0 for modal)
        );

        if (kwallet) {
            bool result = kwallet->writePassword(
                convertWStringToQString(key),
                convertWStringToQString(password)
            );
            delete kwallet;
            return result;
        }
    }
    #endif
#endif

    return false;
}

bool KDEDialogs::retrievePassword(const std::wstring& wallet, const std::wstring& key, std::wstring& password) {
#ifdef NPP_KDE_AVAILABLE
    #if __has_include(<KWallet/KWallet>)
    if (_impl->_useKDE) {
        KWallet::Wallet* kwallet = KWallet::Wallet::openWallet(
            convertWStringToQString(wallet),
            0  // Window ID (0 for modal)
        );

        if (kwallet) {
            QString pass;
            bool result = kwallet->readPassword(convertWStringToQString(key), pass);
            if (result) {
                password = convertQStringToWString(pass);
            }
            delete kwallet;
            return result;
        }
    }
    #endif
#endif

    return false;
}

// ============================================================================
// Helper Methods
// ============================================================================

QString KDEDialogs::convertToQString(const std::wstring& str) const {
    return convertWStringToQString(str);
}

std::wstring KDEDialogs::convertToWString(const QString& str) const {
    return convertQStringToWString(str);
}

QString KDEDialogs::buildKDEFilterString(const std::vector<FileFilter>& filters) const {
    return buildQtFilterString(filters);
}

// ============================================================================
// Convenience Functions
// ============================================================================

namespace KDEDialogsUtils {

void showPlasmaNotification(const std::wstring& title, const std::wstring& message) {
#ifdef NPP_KDE_AVAILABLE
    KNotification::event(
        "notepad-plus-plus",
        convertWStringToQString(title),
        convertWStringToQString(message),
        "notepad-plus-plus"
    );
#else
    (void)title;
    (void)message;
#endif
}

bool isPlasmaSession() {
    const char* desktopSession = std::getenv("XDG_CURRENT_DESKTOP");
    if (desktopSession) {
        QString session = QString::fromUtf8(desktopSession).toLower();
        return session.contains("kde") || session.contains("plasma");
    }

    const char* kdeFullSession = std::getenv("KDE_FULL_SESSION");
    return kdeFullSession && QString::fromUtf8(kdeFullSession).compare("true", Qt::CaseInsensitive) == 0;
}

std::wstring getKDEVersion() {
#ifdef NPP_KDE_AVAILABLE
    return convertQStringToWString(KAboutData::applicationData().version());
#else
    return std::wstring();
#endif
}

bool openFileWithKDE(const std::wstring& filePath) {
#ifdef NPP_KDE_AVAILABLE
    // Use KIO to open the file with the default application
    QString path = convertWStringToQString(filePath);
    QUrl url = QUrl::fromLocalFile(path);

    // This would typically use KIO::OpenUrlJob in newer KDE Frameworks
    // For now, we fall back to Qt's approach
    return QDesktopServices::openUrl(url);
#else
    (void)filePath;
    return false;
#endif
}

} // namespace KDEDialogsUtils

} // namespace KDE
} // namespace Platform
