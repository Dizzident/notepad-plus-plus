// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../Dialogs.h"

// Include KDE dialogs if available
#ifdef NPP_KDE_AVAILABLE
#include "../KDE/KDEDialogs.h"
#endif

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QWidget>
#include <QDialog>
#include <QApplication>

namespace PlatformLayer {

// Helper to convert FileFilter to Qt format
static QString buildQtFilterString(const std::vector<FileFilter>& filters) {
    QStringList qtFilters;
    for (const auto& f : filters) {
        QString desc = QString::fromStdWString(f.description);
        QString pattern = QString::fromStdWString(f.pattern);
        qtFilters << QString("%1 (%2)").arg(desc).arg(pattern);
    }
    return qtFilters.join(";;");
}

// Helper to map MessageBoxIcon to Qt
static QMessageBox::Icon getQtIcon(MessageBoxIcon icon) {
    switch (icon) {
        case MessageBoxIcon::Error: return QMessageBox::Critical;
        case MessageBoxIcon::Question: return QMessageBox::Question;
        case MessageBoxIcon::Warning: return QMessageBox::Warning;
        case MessageBoxIcon::Information: return QMessageBox::Information;
        default: return QMessageBox::NoIcon;
    }
}

// Helper to map MessageBoxType to Qt buttons
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

// Helper to convert Qt result to DialogResult
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
// Linux Implementation using Qt
// ============================================================================
class DialogsLinux : public IDialogs {
public:
    DialogsLinux() : _parentWidget(nullptr) {}

    void setParent(QWidget* parent) { _parentWidget = parent; }

    DialogResult messageBox(const std::wstring& message,
                           const std::wstring& title,
                           MessageBoxType type,
                           MessageBoxIcon icon,
                           DialogResult defaultButton) override {
        QMessageBox msgBox(_parentWidget);
        msgBox.setText(QString::fromStdWString(message));
        msgBox.setWindowTitle(QString::fromStdWString(title));
        msgBox.setIcon(getQtIcon(icon));
        msgBox.setStandardButtons(getQtButtons(type));

        // Set default button
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

    void showInfo(const std::wstring& message, const std::wstring& title) override {
        QMessageBox::information(_parentWidget,
                                 QString::fromStdWString(title),
                                 QString::fromStdWString(message));
    }

    void showWarning(const std::wstring& message, const std::wstring& title) override {
        QMessageBox::warning(_parentWidget,
                             QString::fromStdWString(title),
                             QString::fromStdWString(message));
    }

    void showError(const std::wstring& message, const std::wstring& title) override {
        QMessageBox::critical(_parentWidget,
                              QString::fromStdWString(title),
                              QString::fromStdWString(message));
    }

    bool askYesNo(const std::wstring& message, const std::wstring& title) override {
        return QMessageBox::question(_parentWidget,
                                     QString::fromStdWString(title),
                                     QString::fromStdWString(message),
                                     QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
    }

    DialogResult askYesNoCancel(const std::wstring& message, const std::wstring& title) override {
        auto result = QMessageBox::question(_parentWidget,
                                            QString::fromStdWString(title),
                                            QString::fromStdWString(message),
                                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        return mapQtResult(result);
    }

    bool askRetryCancel(const std::wstring& message, const std::wstring& title) override {
        return QMessageBox::warning(_parentWidget,
                                    QString::fromStdWString(title),
                                    QString::fromStdWString(message),
                                    QMessageBox::Retry | QMessageBox::Cancel) == QMessageBox::Retry;
    }

    std::wstring showOpenFileDialog(const std::wstring& title,
                                   const std::vector<FileFilter>& filters,
                                   const FileDialogOptions& options) override {
        QString filterStr = buildQtFilterString(filters);
        QString dir = QString::fromStdWString(options.initialDirectory);

        QString fileName = QFileDialog::getOpenFileName(_parentWidget,
                                                        QString::fromStdWString(title),
                                                        dir,
                                                        filterStr);

        return fileName.toStdWString();
    }

    std::vector<std::wstring> showOpenFilesDialog(const std::wstring& title,
                                                 const std::vector<FileFilter>& filters,
                                                 const FileDialogOptions& options) override {
        QString filterStr = buildQtFilterString(filters);
        QString dir = QString::fromStdWString(options.initialDirectory);

        QStringList fileNames = QFileDialog::getOpenFileNames(_parentWidget,
                                                              QString::fromStdWString(title),
                                                              dir,
                                                              filterStr);

        std::vector<std::wstring> results;
        for (const QString& name : fileNames) {
            results.push_back(name.toStdWString());
        }
        return results;
    }

    std::wstring showSaveFileDialog(const std::wstring& title,
                                   const std::vector<FileFilter>& filters,
                                   const std::wstring& defaultFileName,
                                   const FileDialogOptions& options) override {
        QString filterStr = buildQtFilterString(filters);
        QString dir = QString::fromStdWString(options.initialDirectory);

        if (!defaultFileName.empty()) {
            if (dir.isEmpty()) {
                dir = QString::fromStdWString(defaultFileName);
            } else {
                dir = QDir(dir).filePath(QString::fromStdWString(defaultFileName));
            }
        }

        QString fileName = QFileDialog::getSaveFileName(_parentWidget,
                                                        QString::fromStdWString(title),
                                                        dir,
                                                        filterStr);

        return fileName.toStdWString();
    }

    std::wstring showFolderDialog(const std::wstring& title,
                                 const FolderDialogOptions& options) override {
        QString dir = QString::fromStdWString(options.initialFolder);
        QString result = QFileDialog::getExistingDirectory(_parentWidget,
                                                           QString::fromStdWString(title),
                                                           dir,
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        return result.toStdWString();
    }

    bool showInputDialog(const std::wstring& title,
                        const std::wstring& prompt,
                        std::wstring& value,
                        bool isPassword) override {
        bool ok;
        QString text;

        if (isPassword) {
            text = QInputDialog::getText(_parentWidget,
                                         QString::fromStdWString(title),
                                         QString::fromStdWString(prompt),
                                         QLineEdit::Password,
                                         QString::fromStdWString(value),
                                         &ok);
        } else {
            text = QInputDialog::getText(_parentWidget,
                                         QString::fromStdWString(title),
                                         QString::fromStdWString(prompt),
                                         QLineEdit::Normal,
                                         QString::fromStdWString(value),
                                         &ok);
        }

        if (ok) {
            value = text.toStdWString();
        }
        return ok;
    }

    bool showInputDialogEx(const InputDialogOptions& options, std::wstring& value) override {
        bool ok;
        QString text;

        QLineEdit::EchoMode mode = options.isPassword ? QLineEdit::Password : QLineEdit::Normal;

        if (options.multiline) {
            text = QInputDialog::getMultiLineText(_parentWidget,
                                                  QString::fromStdWString(options.title),
                                                  QString::fromStdWString(options.prompt),
                                                  QString::fromStdWString(options.defaultValue),
                                                  &ok);
        } else {
            text = QInputDialog::getText(_parentWidget,
                                         QString::fromStdWString(options.title),
                                         QString::fromStdWString(options.prompt),
                                         mode,
                                         QString::fromStdWString(options.defaultValue),
                                         &ok);
        }

        if (ok) {
            value = text.toStdWString();
            return true;
        }
        return false;
    }

    bool showMultiLineInputDialog(const std::wstring& title,
                                 const std::wstring& prompt,
                                 std::wstring& value) override {
        bool ok;
        QString text = QInputDialog::getMultiLineText(_parentWidget,
                                                      QString::fromStdWString(title),
                                                      QString::fromStdWString(prompt),
                                                      QString::fromStdWString(value),
                                                      &ok);
        if (ok) {
            value = text.toStdWString();
            return true;
        }
        return false;
    }

    int showListDialog(const std::wstring& title,
                      const std::wstring& prompt,
                      const std::vector<std::wstring>& items,
                      int defaultIndex) override {
        QStringList qtItems;
        for (const auto& item : items) {
            qtItems << QString::fromStdWString(item);
        }

        bool ok;
        QString result = QInputDialog::getItem(_parentWidget,
                                               QString::fromStdWString(title),
                                               QString::fromStdWString(prompt),
                                               qtItems,
                                               defaultIndex,
                                               false,  // not editable
                                               &ok);
        if (ok) {
            return qtItems.indexOf(result);
        }
        return -1;
    }

    DialogResult showCustomDialog(void* dialogData) override {
        (void)dialogData;
        // Custom dialogs would need specific implementation
        return DialogResult::None;
    }

    void centerDialog(void* dialogHandle) override {
        QWidget* widget = static_cast<QWidget*>(dialogHandle);
        if (widget) {
            widget->adjustSize();
            widget->move(widget->frameGeometry().center() - widget->rect().center());
        }
    }

    void setDialogPosition(void* dialogHandle, int x, int y) override {
        QWidget* widget = static_cast<QWidget*>(dialogHandle);
        if (widget) {
            widget->move(x, y);
        }
    }

    void getDialogPosition(void* dialogHandle, int& x, int& y) override {
        QWidget* widget = static_cast<QWidget*>(dialogHandle);
        if (widget) {
            QPoint pos = widget->pos();
            x = pos.x();
            y = pos.y();
        } else {
            x = y = 0;
        }
    }

    void setDialogSize(void* dialogHandle, int width, int height) override {
        QWidget* widget = static_cast<QWidget*>(dialogHandle);
        if (widget) {
            widget->resize(width, height);
        }
    }

    void getDialogSize(void* dialogHandle, int& width, int& height) override {
        QWidget* widget = static_cast<QWidget*>(dialogHandle);
        if (widget) {
            QSize size = widget->size();
            width = size.width();
            height = size.height();
        } else {
            width = height = 0;
        }
    }

    void setDialogTitle(void* dialogHandle, const std::wstring& title) override {
        QWidget* widget = static_cast<QWidget*>(dialogHandle);
        if (widget) {
            widget->setWindowTitle(QString::fromStdWString(title));
        }
    }

    void enableDialog(void* dialogHandle, bool enable) override {
        QWidget* widget = static_cast<QWidget*>(dialogHandle);
        if (widget) {
            widget->setEnabled(enable);
        }
    }

    bool isDialogEnabled(void* dialogHandle) override {
        QWidget* widget = static_cast<QWidget*>(dialogHandle);
        return widget && widget->isEnabled();
    }

    void bringToFront(void* dialogHandle) override {
        QWidget* widget = static_cast<QWidget*>(dialogHandle);
        if (widget) {
            widget->raise();
            widget->activateWindow();
        }
    }

    void setModal(void* dialogHandle, bool modal) override {
        QWidget* widget = static_cast<QWidget*>(dialogHandle);
        if (widget) {
            QDialog* dialog = qobject_cast<QDialog*>(widget);
            if (dialog) {
                dialog->setModal(modal);
            }
        }
    }

private:
    QWidget* _parentWidget;
};

// ============================================================================
// Singleton Accessor
// ============================================================================
IDialogs& IDialogs::getInstance() {
#ifdef NPP_KDE_AVAILABLE
    // Check if KDE is available at runtime and use it if so
    if (KDE::KDEDialogs::isKDEAvailable()) {
        static KDE::KDEDialogs kdeInstance;
        return kdeInstance;
    }
#endif
    static DialogsLinux instance;
    return instance;
}

// ============================================================================
// File Filter Helpers
// ============================================================================
namespace DialogFilters {

std::vector<FileFilter> allFiles() {
    return { FileFilter(L"All Files", L"*") };
}

std::vector<FileFilter> textFiles() {
    return { FileFilter(L"Text Files", L"*.txt"),
             FileFilter(L"Log Files", L"*.log") };
}

std::vector<FileFilter> sourceCodeFiles() {
    return { FileFilter(L"C/C++ Files", L"*.c;*.cpp;*.h;*.hpp"),
             FileFilter(L"C# Files", L"*.cs"),
             FileFilter(L"Java Files", L"*.java"),
             FileFilter(L"Python Files", L"*.py") };
}

std::vector<FileFilter> xmlFiles() {
    return { FileFilter(L"XML Files", L"*.xml"),
             FileFilter(L"XSL Files", L"*.xsl;*.xslt") };
}

std::vector<FileFilter> htmlFiles() {
    return { FileFilter(L"HTML Files", L"*.html;*.htm"),
             FileFilter(L"CSS Files", L"*.css"),
             FileFilter(L"JavaScript Files", L"*.js") };
}

std::vector<FileFilter> imageFiles() {
    return { FileFilter(L"Image Files", L"*.bmp;*.gif;*.jpg;*.jpeg;*.png;*.tiff"),
             FileFilter(L"Bitmap Files", L"*.bmp"),
             FileFilter(L"PNG Files", L"*.png"),
             FileFilter(L"JPEG Files", L"*.jpg;*.jpeg") };
}

FileFilter filter(const std::wstring& description, const std::wstring& pattern) {
    return FileFilter(description, pattern);
}

std::vector<FileFilter> combine(const std::vector<FileFilter>& a,
                               const std::vector<FileFilter>& b) {
    std::vector<FileFilter> result = a;
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

} // namespace DialogFilters

// ============================================================================
// Progress Dialog Stub Implementation
// ============================================================================
class ProgressDialogLinux : public IProgressDialog {
public:
    void show(const std::wstring& title, const std::wstring& message) override {
        // Stub implementation
    }

    void hide() override {
        // Stub implementation
    }

    void setProgress(int percent) override {
        // Stub implementation
    }

    void setMessage(const std::wstring& message) override {
        // Stub implementation
    }

    void setStatus(const std::wstring& status) override {
        // Stub implementation
    }

    bool isCancelled() const override {
        return false;
    }

    void setCancelable(bool cancelable) override {
        // Stub implementation
    }

    void step(int increment = 1) override {
        // Stub implementation
    }

    void setRange(int min, int max) override {
        // Stub implementation
    }
};

std::unique_ptr<IProgressDialog> IProgressDialog::create() {
    return std::make_unique<ProgressDialogLinux>();
}

} // namespace PlatformLayer
