// Run Menu Implementation - To be merged into Notepad_plus_Window.cpp
// This file contains the Run menu slot implementations

void MainWindow::onRunRun()
{
    if (_pNotepad_plus) {
        _pNotepad_plus->showRunDlg();
    }
}

void MainWindow::onRunLaunchInBrowser()
{
    if (!_pNotepad_plus) return;

    Buffer* buffer = _pNotepad_plus->getCurrentBuffer();
    if (!buffer) return;

    std::wstring filePath = buffer->getFullPathName();
    if (filePath.empty()) return;

    // Launch in default browser using QDesktopServices
    QString url = QString::fromStdWString(filePath);
    // Check if it's a local file
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = QUrl::fromLocalFile(url).toString();
    }
    QDesktopServices::openUrl(QUrl(url));
}
