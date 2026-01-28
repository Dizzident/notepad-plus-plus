// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../Dialogs.h"
#include "../FileSystem.h"

#ifdef _WIN32

#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>
#include <cderr.h>

namespace PlatformLayer {

// Helper function to convert FileFilter to Windows format
static std::wstring buildFilterString(const std::vector<FileFilter>& filters) {
    if (filters.empty()) {
        return L"All Files (*.*)\0*.*\0";
    }

    std::wstring result;
    for (const auto& f : filters) {
        result += f.description + L" (" + f.pattern + L")\0" + f.pattern + L"\0";
    }
    result += L"\0"; // Double null terminator
    return result;
}

// Helper to map MessageBoxType to Win32
static UINT getMessageBoxType(MessageBoxType type) {
    switch (type) {
        case MessageBoxType::OK: return MB_OK;
        case MessageBoxType::OKCancel: return MB_OKCANCEL;
        case MessageBoxType::AbortRetryIgnore: return MB_ABORTRETRYIGNORE;
        case MessageBoxType::YesNoCancel: return MB_YESNOCANCEL;
        case MessageBoxType::YesNo: return MB_YESNO;
        case MessageBoxType::RetryCancel: return MB_RETRYCANCEL;
        case MessageBoxType::CancelTryAgainContinue: return MB_CANCELTRYCONTINUE;
        default: return MB_OK;
    }
}

// Helper to map MessageBoxIcon to Win32
static UINT getMessageBoxIcon(MessageBoxIcon icon) {
    switch (icon) {
        case MessageBoxIcon::Error: return MB_ICONERROR;
        case MessageBoxIcon::Question: return MB_ICONQUESTION;
        case MessageBoxIcon::Warning: return MB_ICONWARNING;
        case MessageBoxIcon::Information: return MB_ICONINFORMATION;
        default: return 0;
    }
}

// Helper to convert Win32 result to DialogResult
static DialogResult mapWin32Result(int result) {
    switch (result) {
        case IDOK: return DialogResult::OK;
        case IDCANCEL: return DialogResult::Cancel;
        case IDABORT: return DialogResult::Abort;
        case IDRETRY: return DialogResult::Retry;
        case IDIGNORE: return DialogResult::Ignore;
        case IDYES: return DialogResult::Yes;
        case IDNO: return DialogResult::No;
        case IDTRYAGAIN: return DialogResult::TryAgain;
        case IDCONTINUE: return DialogResult::Continue;
        default: return DialogResult::None;
    }
}

// ============================================================================
// Windows Implementation
// ============================================================================
class DialogsWin32 : public IDialogs {
public:
    DialogsWin32() : _hwndOwner(NULL) {}

    void setOwner(HWND hwnd) { _hwndOwner = hwnd; }

    DialogResult messageBox(const std::wstring& message,
                           const std::wstring& title,
                           MessageBoxType type,
                           MessageBoxIcon icon,
                           DialogResult defaultButton) override {
        UINT flags = getMessageBoxType(type) | getMessageBoxIcon(icon);

        // Set default button
        switch (defaultButton) {
            case DialogResult::OK: flags |= MB_DEFBUTTON1; break;
            case DialogResult::Cancel: flags |= MB_DEFBUTTON2; break;
            case DialogResult::Yes: flags |= (type == MessageBoxType::YesNoCancel) ? MB_DEFBUTTON1 : MB_DEFBUTTON1; break;
            case DialogResult::No: flags |= (type == MessageBoxType::YesNoCancel) ? MB_DEFBUTTON2 : MB_DEFBUTTON2; break;
            default: break;
        }

        int result = ::MessageBoxW(_hwndOwner, message.c_str(), title.c_str(), flags);
        return mapWin32Result(result);
    }

    void showInfo(const std::wstring& message, const std::wstring& title) override {
        ::MessageBoxW(_hwndOwner, message.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
    }

    void showWarning(const std::wstring& message, const std::wstring& title) override {
        ::MessageBoxW(_hwndOwner, message.c_str(), title.c_str(), MB_OK | MB_ICONWARNING);
    }

    void showError(const std::wstring& message, const std::wstring& title) override {
        ::MessageBoxW(_hwndOwner, message.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
    }

    bool askYesNo(const std::wstring& message, const std::wstring& title) override {
        int result = ::MessageBoxW(_hwndOwner, message.c_str(), title.c_str(), MB_YESNO | MB_ICONQUESTION);
        return result == IDYES;
    }

    DialogResult askYesNoCancel(const std::wstring& message, const std::wstring& title) override {
        int result = ::MessageBoxW(_hwndOwner, message.c_str(), title.c_str(), MB_YESNOCANCEL | MB_ICONQUESTION);
        return mapWin32Result(result);
    }

    bool askRetryCancel(const std::wstring& message, const std::wstring& title) override {
        int result = ::MessageBoxW(_hwndOwner, message.c_str(), title.c_str(), MB_RETRYCANCEL | MB_ICONWARNING);
        return result == IDRETRY;
    }

    std::wstring showOpenFileDialog(const std::wstring& title,
                                   const std::vector<FileFilter>& filters,
                                   const FileDialogOptions& options) override {
        OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) };
        wchar_t fileName[MAX_PATH] = { 0 };

        std::wstring filterStr = buildFilterString(filters);

        ofn.hwndOwner = _hwndOwner;
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = filterStr.empty() ? nullptr : filterStr.c_str();
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = title.empty() ? nullptr : title.c_str();
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (options.fileMustExist) ofn.Flags |= OFN_FILEMUSTEXIST;
        if (options.pathMustExist) ofn.Flags |= OFN_PATHMUSTEXIST;
        if (options.allowMultiSelect) ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;

        if (!options.initialDirectory.empty()) {
            ofn.lpstrInitialDir = options.initialDirectory.c_str();
        }

        if (!options.defaultExtension.empty()) {
            ofn.lpstrDefExt = options.defaultExtension.c_str();
        }

        if (GetOpenFileNameW(&ofn)) {
            return std::wstring(fileName);
        }

        return std::wstring();
    }

    std::vector<std::wstring> showOpenFilesDialog(const std::wstring& title,
                                                 const std::vector<FileFilter>& filters,
                                                 const FileDialogOptions& options) override {
        std::vector<std::wstring> results;
        OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) };

        // Use a larger buffer for multi-select
        const size_t bufferSize = 32768;
        std::vector<wchar_t> fileBuffer(bufferSize, 0);

        std::wstring filterStr = buildFilterString(filters);

        ofn.hwndOwner = _hwndOwner;
        ofn.lpstrFile = fileBuffer.data();
        ofn.nMaxFile = static_cast<DWORD>(bufferSize);
        ofn.lpstrFilter = filterStr.empty() ? nullptr : filterStr.c_str();
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = title.empty() ? nullptr : title.c_str();
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR;

        if (!options.initialDirectory.empty()) {
            ofn.lpstrInitialDir = options.initialDirectory.c_str();
        }

        if (GetOpenFileNameW(&ofn)) {
            // Parse the results
            wchar_t* ptr = fileBuffer.data();
            std::wstring directory = ptr;
            ptr += directory.length() + 1;

            if (*ptr == 0) {
                // Only one file selected
                results.push_back(directory);
            } else {
                // Multiple files selected
                while (*ptr != 0) {
                    results.push_back(directory + L"\\" + ptr);
                    ptr += wcslen(ptr) + 1;
                }
            }
        }

        return results;
    }

    std::wstring showSaveFileDialog(const std::wstring& title,
                                   const std::vector<FileFilter>& filters,
                                   const std::wstring& defaultFileName,
                                   const FileDialogOptions& options) override {
        OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) };
        wchar_t fileName[MAX_PATH] = { 0 };

        if (!defaultFileName.empty()) {
            wcsncpy_s(fileName, MAX_PATH, defaultFileName.c_str(), _TRUNCATE);
        }

        std::wstring filterStr = buildFilterString(filters);

        ofn.hwndOwner = _hwndOwner;
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = filterStr.empty() ? nullptr : filterStr.c_str();
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = title.empty() ? nullptr : title.c_str();
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        if (options.overwritePrompt) ofn.Flags |= OFN_OVERWRITEPROMPT;
        if (options.pathMustExist) ofn.Flags |= OFN_PATHMUSTEXIST;

        if (!options.initialDirectory.empty()) {
            ofn.lpstrInitialDir = options.initialDirectory.c_str();
        }

        if (!options.defaultExtension.empty()) {
            ofn.lpstrDefExt = options.defaultExtension.c_str();
        }

        if (GetSaveFileNameW(&ofn)) {
            return std::wstring(fileName);
        }

        return std::wstring();
    }

    std::wstring showFolderDialog(const std::wstring& title,
                                 const FolderDialogOptions& options) override {
        std::wstring result;

        IFileDialog* pfd = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                     IID_PPV_ARGS(&pfd));

        if (SUCCEEDED(hr)) {
            DWORD dwOptions;
            pfd->GetOptions(&dwOptions);
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);

            if (!title.empty()) {
                pfd->SetTitle(title.c_str());
            }

            if (!options.initialFolder.empty()) {
                IShellItem* psiFolder = nullptr;
                hr = SHCreateItemFromParsingName(options.initialFolder.c_str(), nullptr,
                                                IID_PPV_ARGS(&psiFolder));
                if (SUCCEEDED(hr)) {
                    pfd->SetFolder(psiFolder);
                    psiFolder->Release();
                }
            }

            hr = pfd->Show(_hwndOwner);
            if (SUCCEEDED(hr)) {
                IShellItem* psiResult = nullptr;
                hr = pfd->GetResult(&psiResult);
                if (SUCCEEDED(hr)) {
                    PWSTR pszPath = nullptr;
                    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                    if (SUCCEEDED(hr)) {
                        result = pszPath;
                        CoTaskMemFree(pszPath);
                    }
                    psiResult->Release();
                }
            }
            pfd->Release();
        }

        return result;
    }

    bool showInputDialog(const std::wstring& title,
                        const std::wstring& prompt,
                        std::wstring& value,
                        bool isPassword) override {
        // Windows doesn't have a built-in input dialog
        // This is a simplified version - in practice you might want a custom dialog
        InputDialogOptions options;
        options.title = title;
        options.prompt = prompt;
        options.defaultValue = value;
        options.isPassword = isPassword;
        return showInputDialogEx(options, value);
    }

    bool showInputDialogEx(const InputDialogOptions& options, std::wstring& value) override {
        // Simple implementation using dialog boxes
        // For a full implementation, you'd create a custom dialog resource
        wchar_t buffer[1024] = { 0 };
        if (!options.defaultValue.empty()) {
            wcsncpy_s(buffer, 1024, options.defaultValue.c_str(), _TRUNCATE);
        }

        // Use Windows InputBox (requires custom implementation or use of dialog templates)
        // For now, return false to indicate not fully implemented
        (void)options;
        (void)value;
        return false;
    }

    bool showMultiLineInputDialog(const std::wstring& title,
                                 const std::wstring& prompt,
                                 std::wstring& value) override {
        (void)title;
        (void)prompt;
        (void)value;
        // Requires custom dialog implementation
        return false;
    }

    int showListDialog(const std::wstring& title,
                      const std::wstring& prompt,
                      const std::vector<std::wstring>& items,
                      int defaultIndex) override {
        (void)title;
        (void)prompt;
        (void)items;
        (void)defaultIndex;
        // Requires custom dialog implementation
        return -1;
    }

    DialogResult showCustomDialog(void* dialogData) override {
        (void)dialogData;
        return DialogResult::None;
    }

    void centerDialog(void* dialogHandle) override {
        HWND hwnd = static_cast<HWND>(dialogHandle);
        if (!hwnd) return;

        RECT rcDlg, rcOwner;
        GetWindowRect(hwnd, &rcDlg);

        HWND hwndOwner = GetWindow(hwnd, GW_OWNER);
        if (hwndOwner) {
            GetWindowRect(hwndOwner, &rcOwner);
        } else {
            rcOwner.left = 0;
            rcOwner.top = 0;
            rcOwner.right = GetSystemMetrics(SM_CXSCREEN);
            rcOwner.bottom = GetSystemMetrics(SM_CYSCREEN);
        }

        int x = rcOwner.left + (rcOwner.right - rcOwner.left - (rcDlg.right - rcDlg.left)) / 2;
        int y = rcOwner.top + (rcOwner.bottom - rcOwner.top - (rcDlg.bottom - rcDlg.top)) / 2;

        SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void setDialogPosition(void* dialogHandle, int x, int y) override {
        HWND hwnd = static_cast<HWND>(dialogHandle);
        if (hwnd) {
            SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    }

    void getDialogPosition(void* dialogHandle, int& x, int& y) override {
        HWND hwnd = static_cast<HWND>(dialogHandle);
        RECT rc = { 0 };
        if (hwnd && GetWindowRect(hwnd, &rc)) {
            x = rc.left;
            y = rc.top;
        } else {
            x = y = 0;
        }
    }

    void setDialogSize(void* dialogHandle, int width, int height) override {
        HWND hwnd = static_cast<HWND>(dialogHandle);
        if (hwnd) {
            SetWindowPos(hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
        }
    }

    void getDialogSize(void* dialogHandle, int& width, int& height) override {
        HWND hwnd = static_cast<HWND>(dialogHandle);
        RECT rc = { 0 };
        if (hwnd && GetWindowRect(hwnd, &rc)) {
            width = rc.right - rc.left;
            height = rc.bottom - rc.top;
        } else {
            width = height = 0;
        }
    }

    void setDialogTitle(void* dialogHandle, const std::wstring& title) override {
        HWND hwnd = static_cast<HWND>(dialogHandle);
        if (hwnd) {
            SetWindowTextW(hwnd, title.c_str());
        }
    }

    void enableDialog(void* dialogHandle, bool enable) override {
        HWND hwnd = static_cast<HWND>(dialogHandle);
        if (hwnd) {
            EnableWindow(hwnd, enable ? TRUE : FALSE);
        }
    }

    bool isDialogEnabled(void* dialogHandle) override {
        HWND hwnd = static_cast<HWND>(dialogHandle);
        return hwnd && IsWindowEnabled(hwnd) != FALSE;
    }

    void bringToFront(void* dialogHandle) override {
        HWND hwnd = static_cast<HWND>(dialogHandle);
        if (hwnd) {
            SetForegroundWindow(hwnd);
            BringWindowToTop(hwnd);
        }
    }

    void setModal(void* dialogHandle, bool modal) override {
        HWND hwnd = static_cast<HWND>(dialogHandle);
        if (hwnd) {
            EnableWindow(_hwndOwner, modal ? FALSE : TRUE);
        }
    }

private:
    HWND _hwndOwner;
};

// ============================================================================
// Singleton Accessor
// ============================================================================
IDialogs& IDialogs::getInstance() {
    static DialogsWin32 instance;
    return instance;
}

// ============================================================================
// File Filter Helpers
// ============================================================================
namespace DialogFilters {

std::vector<FileFilter> allFiles() {
    return { FileFilter(L"All Files", L"*.*") };
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

} // namespace PlatformLayer

#endif // _WIN32
