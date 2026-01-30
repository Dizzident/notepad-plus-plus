// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// ============================================================================
// main_linux.cpp - Linux Qt Entry Point for Notepad++
// ============================================================================
// This is the Linux equivalent of winmain.cpp
// It provides the Qt application entry point and command-line processing

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QSharedMemory>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QTranslator>
#include <QLocale>
#include <QTimer>
#include <QDebug>

#include <chrono>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <cwchar>

#include "Platform/FileSystem.h"
#include "Platform/Settings.h"
#include "Platform/Process.h"
#include "Parameters.h"
#include "Common.h"

// QtControls includes
#include "QtControls/Window.h"
#include "QtControls/MainWindow/Notepad_plus_Window.h"

// Notepad++ core includes
#include "Notepad_plus.h"
#include "NppDarkMode.h"
#include "localization.h"

// ============================================================================
// Command Line Parameter Types (mirroring Windows version)
// ============================================================================

namespace {

// Command line flags (matching Windows version)
const wchar_t FLAG_MULTI_INSTANCE[] = L"-multiInst";
const wchar_t FLAG_NO_PLUGIN[] = L"-noPlugin";
const wchar_t FLAG_READONLY[] = L"-ro";
const wchar_t FLAG_FULL_READONLY[] = L"-fullReadOnly";
const wchar_t FLAG_FULL_READONLY_SAVING_FORBIDDEN[] = L"-fullReadOnlySavingForbidden";
const wchar_t FLAG_NOSESSION[] = L"-nosession";
const wchar_t FLAG_NOTABBAR[] = L"-notabbar";
const wchar_t FLAG_SYSTRAY[] = L"-systemtray";
const wchar_t FLAG_LOADINGTIME[] = L"-loadingTime";
const wchar_t FLAG_HELP[] = L"--help";
const wchar_t FLAG_ALWAYS_ON_TOP[] = L"-alwaysOnTop";
const wchar_t FLAG_OPENSESSIONFILE[] = L"-openSession";
const wchar_t FLAG_RECURSIVE[] = L"-r";
const wchar_t FLAG_FUNCLSTEXPORT[] = L"-export=functionList";
const wchar_t FLAG_PRINTANDQUIT[] = L"-quickPrint";
const wchar_t FLAG_NOTEPAD_COMPATIBILITY[] = L"-notepadStyleCmdline";
const wchar_t FLAG_OPEN_FOLDERS_AS_WORKSPACE[] = L"-openFoldersAsWorkspace";
const wchar_t FLAG_SETTINGS_DIR[] = L"-settingsDir=";
const wchar_t FLAG_TITLEBAR_ADD[] = L"-titleAdd=";
const wchar_t FLAG_APPLY_UDL[] = L"-udl=";
const wchar_t FLAG_PLUGIN_MESSAGE[] = L"-pluginMessage=";
const wchar_t FLAG_MONITOR_FILES[] = L"-monitor";

// Global start time for performance measurement - defined in QtControls/Notepad_plus.cpp, declared in Notepad_plus.h
// extern std::chrono::steady_clock::time_point g_nppStartTimePoint;

// ============================================================================
// Helper Functions
// ============================================================================

// Convert QString to std::wstring
std::wstring qStringToWString(const QString& str)
{
    return str.toStdWString();
}

// Convert std::wstring to QString
QString wStringToQString(const std::wstring& str)
{
    return QString::fromStdWString(str);
}

// Check if a parameter is in the list and optionally remove it
bool isInList(const wchar_t* token2Find, std::vector<std::wstring>& params, bool eraseArg = true)
{
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        if (wcscmp(token2Find, it->c_str()) == 0)
        {
            if (eraseArg) params.erase(it);
            return true;
        }
    }
    return false;
}

// Get parameter value for single character flags (e.g., -n10)
bool getParamVal(wchar_t c, std::vector<std::wstring>& params, std::wstring& value)
{
    value = L"";
    size_t nbItems = params.size();

    for (size_t i = 0; i < nbItems; ++i)
    {
        const wchar_t* token = params.at(i).c_str();
        if (token[0] == '-' && wcslen(token) >= 2 && token[1] == c)
        {
            value = (token + 2);
            params.erase(params.begin() + i);
            return true;
        }
    }
    return false;
}

// Get parameter value for string-based flags (e.g., -settingsDir=path)
bool getParamValFromString(const wchar_t* str, std::vector<std::wstring>& params, std::wstring& value)
{
    value = L"";
    size_t nbItems = params.size();

    for (size_t i = 0; i < nbItems; ++i)
    {
        const wchar_t* token = params.at(i).c_str();
        std::wstring tokenStr = token;
        size_t pos = tokenStr.find(str);
        if (pos != std::wstring::npos && pos == 0)
        {
            value = (token + wcslen(str));
            params.erase(params.begin() + i);
            return true;
        }
    }
    return false;
}

// Get language type from parameter
LangType getLangTypeFromParam(std::vector<std::wstring>& params)
{
    std::wstring langStr;
    if (!getParamVal('l', params, langStr))
        return L_EXTERNAL;
    return NppParameters::getLangIDFromStr(langStr.c_str());
}

// Get localization path from parameter
std::wstring getLocalizationPathFromParam(std::vector<std::wstring>& params)
{
    std::wstring locStr;
    if (!getParamVal('L', params, locStr))
        return L"";
    locStr = stringToLower(stringReplace(locStr, L"_", L"-"));
    return NppParameters::getLocPathFromStr(locStr);
}

// Get number from parameter
intptr_t getNumberFromParam(char paramName, std::vector<std::wstring>& params, bool& isParamPresent)
{
    std::wstring numStr;
    if (!getParamVal(paramName, params, numStr))
    {
        isParamPresent = false;
        return -1;
    }
    isParamPresent = true;
    try
    {
        return static_cast<intptr_t>(std::stoll(numStr));
    }
    catch (...)
    {
        return -1;
    }
}

// Get easter egg name from parameter
std::wstring getEasterEggNameFromParam(std::vector<std::wstring>& params, unsigned char& type)
{
    std::wstring easterEggName;
    if (!getParamValFromString(L"-qn=", params, easterEggName))
    {
        if (!getParamValFromString(L"-qt=", params, easterEggName))
        {
            if (!getParamValFromString(L"-qf=", params, easterEggName))
                return L"";
            else
                type = 2;
        }
        else
            type = 1;
    }
    else
        type = 0;

    if (easterEggName.c_str()[0] == '"' && easterEggName.c_str()[easterEggName.length() - 1] == '"')
    {
        easterEggName = easterEggName.substr(1, easterEggName.length() - 2);
    }

    if (type == 2)
    {
        // Resolve relative path to full path
        std::wstring fullPath = PlatformLayer::IFileSystem::getInstance().getFullPathName(easterEggName);
        easterEggName = fullPath;
    }

    return easterEggName;
}

// Get ghost typing speed from parameter
int getGhostTypingSpeedFromParam(std::vector<std::wstring>& params)
{
    std::wstring speedStr;
    if (!getParamValFromString(L"-qSpeed", params, speedStr))
        return -1;

    try
    {
        int speed = std::stoi(speedStr, nullptr);
        if (speed <= 0 || speed > 3)
            return -1;
        return speed;
    }
    catch (...)
    {
        return -1;
    }
}

// Strip ignored params (for -z flag)
void stripIgnoredParams(std::vector<std::wstring>& params)
{
    for (auto it = params.begin(); it != params.end(); )
    {
        if (wcscmp(it->c_str(), L"-z") == 0)
        {
            auto nextIt = std::next(it);
            if (nextIt != params.end())
            {
                params.erase(nextIt);
            }
            it = params.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// Convert /p or /P to -quickPrint (Notepad compatibility)
void convertParamsToNotepadStyle(std::vector<std::wstring>& params)
{
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        if (wcscmp(it->c_str(), L"/p") == 0 || wcscmp(it->c_str(), L"/P") == 0)
        {
            *it = L"-quickPrint";
        }
    }
}

// Parse command line arguments
void parseCommandLine(const QStringList& arguments, std::vector<std::wstring>& paramVector)
{
    // Skip the first argument (program name)
    for (int i = 1; i < arguments.size(); ++i)
    {
        paramVector.push_back(qStringToWString(arguments[i]));
    }
}

// ============================================================================
// Single Instance Handler
// ============================================================================

class SingleInstanceHandler : public QObject
{
    Q_OBJECT

public:
    SingleInstanceHandler(const QString& appName, QObject* parent = nullptr)
        : QObject(parent),
          _sharedMemory(appName + "_SharedMemory"),
          _serverName(appName + "_IPC"),
          _localServer(nullptr),
          _isFirstInstance(false)
    {
    }

    ~SingleInstanceHandler()
    {
        cleanup();
    }

    bool initialize()
    {
        // Try to create shared memory segment
        _isFirstInstance = _sharedMemory.create(1);

        if (_isFirstInstance)
        {
            // First instance - start local server to listen for new instances
            startLocalServer();
            return true;
        }
        else
        {
            // Another instance is running
            _sharedMemory.attach();
            return false;
        }
    }

    bool isFirstInstance() const
    {
        return _isFirstInstance;
    }

    bool sendFilesToExistingInstance(const QStringList& files, const CmdLineParams& params)
    {
        QLocalSocket socket;
        socket.connectToServer(_serverName);

        if (!socket.waitForConnected(1000))
        {
            qWarning() << "Failed to connect to existing instance:" << socket.errorString();
            return false;
        }

        // Send command line parameters as JSON-like structure
        QByteArray data;
        data.append("CMDLINE_PARAMS\n");
        data.append((QString::number(params._line2go) + "\n").toUtf8());
        data.append((QString::number(params._column2go) + "\n").toUtf8());
        data.append((QString::number(params._pos2go) + "\n").toUtf8());
        data.append((QString(params._isReadOnly ? "1" : "0") + "\n").toUtf8());
        data.append((QString(params._isNoSession ? "1" : "0") + "\n").toUtf8());
        data.append((QString(params._isSessionFile ? "1" : "0") + "\n").toUtf8());
        data.append((QString(params._monitorFiles ? "1" : "0") + "\n").toUtf8());
        data.append("END_PARAMS\n");

        // Send files
        for (const QString& file : files)
        {
            data.append("FILE:" + file.toUtf8() + "\n");
        }
        data.append("END_FILES\n");

        socket.write(data);
        socket.flush();
        socket.waitForBytesWritten(2000);
        socket.disconnectFromServer();

        return true;
    }

    void setNewInstanceCallback(std::function<void(const QStringList&, const CmdLineParams&)> callback)
    {
        _callback = callback;
    }

private slots:
    void handleNewConnection()
    {
        QLocalSocket* socket = _localServer->nextPendingConnection();
        if (!socket)
            return;

        connect(socket, &QLocalSocket::readyRead, [this, socket]() {
            QByteArray data = socket->readAll();
            processIncomingData(data);
            socket->disconnectFromServer();
            socket->deleteLater();
        });

        connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
    }

private:
    void startLocalServer()
    {
        _localServer = new QLocalServer(this);
        connect(_localServer, &QLocalServer::newConnection, this, &SingleInstanceHandler::handleNewConnection);

        // Remove any existing server socket
        QLocalServer::removeServer(_serverName);

        if (!_localServer->listen(_serverName))
        {
            qWarning() << "Failed to start local server:" << _localServer->errorString();
        }
    }

    void processIncomingData(const QByteArray& data)
    {
        QStringList files;
        CmdLineParams params;

        QStringList lines = QString(data).split('\n');
        bool inParams = false;

        for (const QString& line : lines)
        {
            if (line == "CMDLINE_PARAMS")
            {
                inParams = true;
                continue;
            }
            else if (line == "END_PARAMS")
            {
                inParams = false;
                continue;
            }
            else if (line == "END_FILES")
            {
                continue;
            }

            if (inParams)
            {
                // Parse parameters in order
                static int paramIndex = 0;
                switch (paramIndex++)
                {
                    case 0: params._line2go = line.toLongLong(); break;
                    case 1: params._column2go = line.toLongLong(); break;
                    case 2: params._pos2go = line.toLongLong(); break;
                    case 3: params._isReadOnly = (line == "1"); break;
                    case 4: params._isNoSession = (line == "1"); break;
                    case 5: params._isSessionFile = (line == "1"); break;
                    case 6: params._monitorFiles = (line == "1"); break;
                }
            }
            else if (line.startsWith("FILE:"))
            {
                files.append(line.mid(5));
            }
        }

        if (_callback)
        {
            _callback(files, params);
        }
    }

    void cleanup()
    {
        if (_localServer)
        {
            _localServer->close();
            delete _localServer;
            _localServer = nullptr;
        }

        if (_isFirstInstance)
        {
            _sharedMemory.detach();
        }
    }

    QSharedMemory _sharedMemory;
    QString _serverName;
    QLocalServer* _localServer;
    bool _isFirstInstance;
    std::function<void(const QStringList&, const CmdLineParams&)> _callback;
};

// ============================================================================
// NotepadPlusPlusApp - Qt version of Notepad_plus_Window with core integration
// ============================================================================

class NotepadPlusPlusApp : public QtControls::MainWindow::MainWindow
{
    Q_OBJECT

public:
    NotepadPlusPlusApp() = default;
    ~NotepadPlusPlusApp() override = default;

    bool init(CmdLineParams* cmdLineParams)
    {
        _cmdLineParams = cmdLineParams;

        // Initialize NppParameters settings from command line
        NppParameters& nppParams = NppParameters::getInstance();
        NppGUI& nppGUI = nppParams.getNppGUI();

        // Apply command line settings to NppParameters
        // TODO: _pluginsManager is private - need to use public method or friend class
        // if (cmdLineParams->_isNoPlugin)
        //     _notepad_plus_plus_core._pluginsManager.disable();

        nppGUI._isCmdlineNosessionActivated = cmdLineParams->_isNoSession;
        nppGUI._isFullReadOnly = cmdLineParams->_isFullReadOnly;
        nppGUI._isFullReadOnlySavingForbidden = cmdLineParams->_isFullReadOnlySavingForbidden;

        // Initialize the Qt MainWindow with Notepad_plus core
        if (!QtControls::MainWindow::MainWindow::init(&_notepad_plus_plus_core))
        {
            qCritical() << "Failed to initialize MainWindow";
            return false;
        }

        // Set window properties
        setWindowTitle("Notepad++");
        resize(1024, 768);

        // Apply command line parameters
        if (cmdLineParams->_alwaysOnTop)
        {
            setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        }

        // Initialize the Notepad_plus core
        // Note: On Windows this uses HWND, on Linux we need a different approach
        // The core initialization happens through the public interface methods
        // TODO: These are private members - need friend class or public setter methods
        // _notepad_plus_plus_core._pPublicInterface = nullptr; // No HWND equivalent on Linux
        // _notepad_plus_plus_core._pMainWindow = nullptr;

        // Initialize the core components
        if (!initNotepadPlusCore())
        {
            qCritical() << "Failed to initialize Notepad++ core";
            return false;
        }

        // Show the window
        show();

        return true;
    }

    void showWindow()
    {
        show();
        raise();
        activateWindow();
    }

    void raiseAndActivateWindow()
    {
        raise();
        activateWindow();
    }

    void openFiles(const QStringList& files, const CmdLineParams& params)
    {
        // Convert QStringList to vector of wstrings for Notepad_plus
        std::vector<std::wstring> fileNames;
        for (const QString& file : files)
        {
            fileNames.push_back(qStringToWString(file));
        }

        // Load files through Notepad_plus core
        if (!fileNames.empty())
        {
            // Build command line string from file names
            std::wstring cmdLineString;
            for (const auto& fn : fileNames)
            {
                if (!cmdLineString.empty()) cmdLineString += L" ";
                cmdLineString += L"\"" + fn + L"\"";
            }

            // Load command line params
            // TODO: loadCommandlineParams is private - need to use public method or friend class
            // CmdLineParamsDTO dto = CmdLineParamsDTO::FromCmdLineParams(params);
            // _notepad_plus_plus_core.loadCommandlineParams(cmdLineString.c_str(), &dto);
            (void)cmdLineString;
            (void)params;
        }
    }

    Notepad_plus* getNotepadPlusCore()
    {
        return &_notepad_plus_plus_core;
    }

private:
    bool initNotepadPlusCore()
    {
        // TODO: This method needs access to private members of Notepad_plus
        // which requires either:
        // 1. Making NotepadPlusPlusApp a friend class of Notepad_plus
        // 2. Moving this initialization to a proper Qt version of Notepad_plus_Window
        // 3. Using only public methods of Notepad_plus

        // For now, we do basic initialization through public methods
        NppParameters& nppParams = NppParameters::getInstance();
        NppGUI& nppGUI = nppParams.getNppGUI();

        // Load last session if enabled
        // DISABLED: Session loading causes crash - needs proper implementation
        // if (nppGUI._rememberLastSession && !nppGUI._isCmdlineNosessionActivated)
        // {
        //     _notepad_plus_plus_core.loadLastSession();
        // }

        // Scan for localization files
        // TODO: Use public methods or move to friend class
        std::vector<std::wstring> fileNames;
        std::vector<std::wstring> patterns;
        patterns.push_back(L"*.xml");

        std::wstring nppDir = nppParams.getNppPath();

        LocalizationSwitcher& localizationSwitcher = nppParams.getLocalizationSwitcher();
        std::wstring localizationDir = nppDir;
        localizationDir = PlatformLayer::IFileSystem::pathAppend(localizationDir, L"localization\\");

        // TODO: getMatchedFileNames is private - need to use alternative approach
        // _notepad_plus_plus_core.getMatchedFileNames(localizationDir.c_str(), 0, patterns, fileNames, false, false);
        // for (size_t i = 0, len = fileNames.size(); i < len; ++i)
        //     localizationSwitcher.addLanguageFromXml(fileNames[i]);

        // Scan for themes
        fileNames.clear();
        ThemeSwitcher& themeSwitcher = nppParams.getThemeSwitcher();

        std::wstring appDataThemeDir = nppParams.isCloud() ? nppParams.getUserPath() : nppParams.getAppDataNppDir();
        if (!appDataThemeDir.empty())
        {
            appDataThemeDir = PlatformLayer::IFileSystem::pathAppend(appDataThemeDir, L"themes\\");
            // TODO: getMatchedFileNames is private
            // _notepad_plus_plus_core.getMatchedFileNames(appDataThemeDir.c_str(), 0, patterns, fileNames, false, false);
            // for (size_t i = 0, len = fileNames.size(); i < len; ++i)
            // {
            //     themeSwitcher.addThemeFromXml(fileNames[i]);
            // }
        }

        fileNames.clear();

        std::wstring nppThemeDir = nppDir;
        nppThemeDir = PlatformLayer::IFileSystem::pathAppend(nppThemeDir, L"themes\\");

        themeSwitcher.setThemeDirPath(nppThemeDir);

        // TODO: getMatchedFileNames is private
        // _notepad_plus_plus_core.getMatchedFileNames(nppThemeDir.c_str(), 0, patterns, fileNames, false, false);
        // for (size_t i = 0, len = fileNames.size(); i < len; ++i)
        // {
        //     std::wstring themeName(themeSwitcher.getThemeFromXmlFileName(fileNames[i].c_str()));
        //     if (!themeSwitcher.themeNameExists(themeName.c_str()))
        //     {
        //         themeSwitcher.addThemeFromXml(fileNames[i]);
        //
        //         if (!appDataThemeDir.empty())
        //         {
        //             std::wstring appDataThemePath = appDataThemeDir;
        //
        //             if (!doesDirectoryExist(appDataThemePath.c_str()))
        //             {
        //                 PlatformLayer::IFileSystem::getInstance().createDirectory(appDataThemePath.c_str());
        //             }
        //
        //             std::wstring fn = fileNames[i].substr(fileNames[i].find_last_of(L"\\/") + 1);
        //             appDataThemePath = PlatformLayer::IFileSystem::pathAppend(appDataThemePath, fn);
        //             themeSwitcher.addThemeStylerSavePath(fileNames[i], appDataThemePath);
        //         }
        //     }
        // }

        // Apply dark mode theme if enabled
        if (NppDarkMode::isWindowsModeEnabled())
        {
            std::wstring themePath;
            std::wstring xmlFileName = NppDarkMode::getThemeName();
            if (!xmlFileName.empty())
            {
                if (!nppParams.isLocal() || nppParams.isCloud())
                {
                    themePath = nppParams.getUserPath();
                    themePath = PlatformLayer::IFileSystem::pathAppend(themePath, L"themes\\");
                    themePath = PlatformLayer::IFileSystem::pathAppend(themePath, xmlFileName);
                }

                if (themePath.empty() || !doesFileExist(themePath.c_str()))
                {
                    themePath = themeSwitcher.getThemeDirPath();
                    themePath = PlatformLayer::IFileSystem::pathAppend(themePath, xmlFileName);
                }
            }
            else
            {
                const auto& themeInfo = themeSwitcher.getElementFromIndex(0);
                themePath = themeInfo.second;
            }

            if (doesFileExist(themePath.c_str()))
            {
                nppGUI._themeName.assign(themePath);
                nppParams.reloadStylers(themePath.c_str());
            }
        }

        // Create new document on startup if configured
        if (nppGUI._newDocDefaultSettings._addNewDocumentOnStartup && nppGUI._rememberLastSession)
        {
            _notepad_plus_plus_core.fileNew();
        }

        // Check for snapshot mode (backup)
        // TODO: checkModifiedDocument is private - need public method
        // bool isSnapshotMode = nppGUI.isSnapshotMode();
        // if (isSnapshotMode)
        // {
        //     _notepad_plus_plus_core.checkModifiedDocument(false);
        //     _notepad_plus_plus_core.launchDocumentBackupTask();
        // }

        return true;
    }

    Notepad_plus _notepad_plus_plus_core;
    CmdLineParams* _cmdLineParams = nullptr;
};

} // anonymous namespace

// ============================================================================
// Main Entry Point
// ============================================================================

int main(int argc, char* argv[])
{
    g_nppStartTimePoint = std::chrono::steady_clock::now();

    // Enable High DPI support
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    // Create Qt application
    QApplication app(argc, argv);

    // Set application metadata
    QApplication::setApplicationName("Notepad++");
    QApplication::setApplicationVersion("8.6.0");
    QApplication::setOrganizationName("Notepad++");

    // ============================================================================
    // Command Line Parsing
    // ============================================================================

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr(
        "Notepad++ - a free source code editor and Notepad replacement\n"
        "Copyright (C)2024 Notepad++ contributors\n"
        "This program comes with ABSOLUTELY NO WARRANTY.\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under certain conditions."
    ));
    parser.addHelpOption();
    parser.addVersionOption();

    // Define command line options
    QCommandLineOption multiInstanceOption(QStringList() << "m" << "multi-instance",
        QApplication::tr("Run multiple instances (ignore single instance setting)"));
    parser.addOption(multiInstanceOption);

    QCommandLineOption noSessionOption(QStringList() << "n" << "no-session",
        QApplication::tr("Don't load previous session on startup"));
    parser.addOption(noSessionOption);

    QCommandLineOption noPluginsOption(QStringList() << "p" << "no-plugins",
        QApplication::tr("Don't load any plugins"));
    parser.addOption(noPluginsOption);

    QCommandLineOption readOnlyOption(QStringList() << "r" << "read-only",
        QApplication::tr("Open files in read-only mode"));
    parser.addOption(readOnlyOption);

    QCommandLineOption lineNumberOption(QStringList() << "l" << "line",
        QApplication::tr("Start at line number"), "line");
    parser.addOption(lineNumberOption);

    QCommandLineOption columnOption(QStringList() << "c" << "column",
        QApplication::tr("Start at column"), "column");
    parser.addOption(columnOption);

    QCommandLineOption positionOption(QStringList() << "pos",
        QApplication::tr("Start at position"), "pos");
    parser.addOption(positionOption);

    QCommandLineOption languageOption(QStringList() << "lang",
        QApplication::tr("Force language for files"), "language");
    parser.addOption(languageOption);

    QCommandLineOption localizationOption(QStringList() << "L" << "localization",
        QApplication::tr("Use specified localization"), "lang");
    parser.addOption(localizationOption);

    QCommandLineOption settingsDirOption(QStringList() << "settingsDir",
        QApplication::tr("Use specified directory for settings"), "dir");
    parser.addOption(settingsDirOption);

    QCommandLineOption titleAddOption(QStringList() << "titleAdd",
        QApplication::tr("Add text to title bar"), "text");
    parser.addOption(titleAddOption);

    QCommandLineOption alwaysOnTopOption(QStringList() << "alwaysOnTop",
        QApplication::tr("Keep window always on top"));
    parser.addOption(alwaysOnTopOption);

    QCommandLineOption systemTrayOption(QStringList() << "systemtray",
        QApplication::tr("Minimize to system tray on startup"));
    parser.addOption(systemTrayOption);

    QCommandLineOption loadingTimeOption(QStringList() << "loadingTime",
        QApplication::tr("Show loading time in title bar"));
    parser.addOption(loadingTimeOption);

    QCommandLineOption monitorFilesOption(QStringList() << "monitor",
        QApplication::tr("Monitor files for changes"));
    parser.addOption(monitorFilesOption);

    QCommandLineOption notepadStyleOption(QStringList() << "notepadStyleCmdline",
        QApplication::tr("Enable Notepad-style command line compatibility"));
    parser.addOption(notepadStyleOption);

    QCommandLineOption openFoldersAsWorkspaceOption(QStringList() << "openFoldersAsWorkspace",
        QApplication::tr("Open folders as workspace"));
    parser.addOption(openFoldersAsWorkspaceOption);

    QCommandLineOption recursiveOption(QStringList() << "R" << "recursive",
        QApplication::tr("Recursive folder search"));
    parser.addOption(recursiveOption);

    QCommandLineOption sessionFileOption(QStringList() << "openSession",
        QApplication::tr("Open session file"), "file");
    parser.addOption(sessionFileOption);

    QCommandLineOption udlOption(QStringList() << "udl",
        QApplication::tr("Apply User Defined Language"), "udl");
    parser.addOption(udlOption);

    QCommandLineOption pluginMessageOption(QStringList() << "pluginMessage",
        QApplication::tr("Send message to plugin"), "message");
    parser.addOption(pluginMessageOption);

    // Files to open
    parser.addPositionalArgument("files", QApplication::tr("Files or folders to open"), "[files...]");

    // Process the command line
    parser.process(app);

    // ============================================================================
    // Initialize Platform Abstractions
    // ============================================================================

    // Initialize file system
    PlatformLayer::IFileSystem& fileSystem = PlatformLayer::IFileSystem::getInstance();

    // Initialize settings
    PlatformLayer::ISettings& settings = PlatformLayer::ISettings::getInstance();

    // Set custom settings directory if specified
    if (parser.isSet(settingsDirOption))
    {
        std::wstring settingsDir = qStringToWString(parser.value(settingsDirOption));
        // Remove quotes if present
        if (!settingsDir.empty() && settingsDir.front() == '"' && settingsDir.back() == '"')
        {
            settingsDir = settingsDir.substr(1, settingsDir.length() - 2);
        }
        // TODO: Set custom settings directory in settings
    }

    // Initialize settings
    if (!settings.init())
    {
        QMessageBox::critical(nullptr, QApplication::tr("Error"),
            QApplication::tr("Failed to initialize settings."));
        return 1;
    }

    // ============================================================================
    // Parse Parameters (matching Windows version)
    // ============================================================================

    QStringList args = parser.positionalArguments();
    std::vector<std::wstring> params;
    parseCommandLine(QApplication::arguments(), params);

    // Strip ignored params for -z flag
    stripIgnoredParams(params);

    // Convert to Notepad-style if requested
    if (parser.isSet(notepadStyleOption))
    {
        convertParamsToNotepadStyle(params);
    }

    // Build command line params structure
    CmdLineParams cmdLineParams;

    cmdLineParams._isNoPlugin = parser.isSet(noPluginsOption);
    cmdLineParams._isReadOnly = parser.isSet(readOnlyOption);
    cmdLineParams._isNoSession = parser.isSet(noSessionOption);
    cmdLineParams._alwaysOnTop = parser.isSet(alwaysOnTopOption);
    cmdLineParams._isPreLaunch = parser.isSet(systemTrayOption);
    cmdLineParams._showLoadingTime = parser.isSet(loadingTimeOption);
    cmdLineParams._monitorFiles = parser.isSet(monitorFilesOption);
    cmdLineParams._openFoldersAsWorkspace = parser.isSet(openFoldersAsWorkspaceOption);
    cmdLineParams._isRecursive = parser.isSet(recursiveOption);
    cmdLineParams._isSessionFile = parser.isSet(sessionFileOption);

    // Get line/column/position
    bool isParamPresent = false;
    if (parser.isSet(lineNumberOption))
    {
        cmdLineParams._line2go = parser.value(lineNumberOption).toLongLong();
    }
    if (parser.isSet(columnOption))
    {
        cmdLineParams._column2go = parser.value(columnOption).toLongLong();
    }
    if (parser.isSet(positionOption))
    {
        cmdLineParams._pos2go = parser.value(positionOption).toLongLong();
    }

    // Get language
    if (parser.isSet(languageOption))
    {
        std::wstring langStr = qStringToWString(parser.value(languageOption));
        cmdLineParams._langType = NppParameters::getLangIDFromStr(langStr.c_str());
    }

    // Get localization
    if (parser.isSet(localizationOption))
    {
        std::wstring locStr = qStringToWString(parser.value(localizationOption));
        locStr = stringToLower(stringReplace(locStr, L"_", L"-"));
        cmdLineParams._localizationPath = NppParameters::getLocPathFromStr(locStr);
    }

    // Get title bar addition
    std::wstring titleBarAdditional;
    if (parser.isSet(titleAddOption))
    {
        titleBarAdditional = qStringToWString(parser.value(titleAddOption));
    }

    // Get UDL name
    if (parser.isSet(udlOption))
    {
        cmdLineParams._udlName = qStringToWString(parser.value(udlOption));
    }

    // Get plugin message
    if (parser.isSet(pluginMessageOption))
    {
        cmdLineParams._pluginMessage = qStringToWString(parser.value(pluginMessageOption));
    }

    // Get easter egg / quote
    unsigned char quoteType = 0;
    cmdLineParams._easterEggName = getEasterEggNameFromParam(params, quoteType);
    cmdLineParams._quoteType = quoteType;

    // Get ghost typing speed
    cmdLineParams._ghostTypingSpeed = getGhostTypingSpeedFromParam(params);

    // ============================================================================
    // Load NppParameters (Core Settings)
    // ============================================================================

    NppParameters& nppParameters = NppParameters::getInstance();

    // Set command line string for reference
    std::wstring cmdLineString;
    for (const QString& arg : QApplication::arguments())
    {
        if (!cmdLineString.empty()) cmdLineString += L" ";
        cmdLineString += qStringToWString(arg);
    }
    nppParameters.setCmdLineString(cmdLineString);

    // Set localization if specified
    if (!cmdLineParams._localizationPath.empty())
    {
        nppParameters.setStartWithLocFileName(cmdLineParams._localizationPath);
    }

    // Set title bar addition if specified
    if (!titleBarAdditional.empty())
    {
        nppParameters.setTitleBarAdd(titleBarAdditional);
    }

    // Load parameters
    nppParameters.load();

    NppGUI& nppGui = nppParameters.getNppGUI();

    // ============================================================================
    // Single Instance Check
    // ============================================================================

    bool isMultiInst = parser.isSet(multiInstanceOption);

    // Check user preference for multi-instance
    if (nppGui._multiInstSetting == multiInst)
    {
        isMultiInst = true;
    }

    // Override settings for Notepad-style
    if (nppParameters.asNotepadStyle())
    {
        isMultiInst = true;
        cmdLineParams._isNoTab = true;
        cmdLineParams._isNoSession = true;
    }

    SingleInstanceHandler singleInstanceHandler("Notepad++");

    if (!isMultiInst)
    {
        bool isFirstInstance = singleInstanceHandler.initialize();

        if (!isFirstInstance)
        {
            // Another instance is running - send files to it
            if (!args.isEmpty() || !cmdLineParams._pluginMessage.empty())
            {
                if (singleInstanceHandler.sendFilesToExistingInstance(args, cmdLineParams))
                {
                    return 0; // Exit successfully, files sent to existing instance
                }
            }
            else
            {
                // Just activate the existing instance
                // TODO: Send activate message
                return 0;
            }
        }
    }

    // ============================================================================
    // Process Files to Open
    // ============================================================================

    QStringList filesToOpen;
    for (const QString& arg : args)
    {
        QFileInfo fileInfo(arg);
        if (fileInfo.exists())
        {
            filesToOpen.append(fileInfo.absoluteFilePath());
        }
        else
        {
            // File doesn't exist - might be a new file
            filesToOpen.append(QDir::current().absoluteFilePath(arg));
        }
    }

    // ============================================================================
    // Create and Initialize Main Window with Notepad++ Core
    // ============================================================================

    auto mainWindow = std::make_unique<NotepadPlusPlusApp>();

    if (!mainWindow->init(&cmdLineParams))
    {
        QMessageBox::critical(nullptr, QApplication::tr("Error"),
            QApplication::tr("Failed to initialize Notepad++."));
        return 1;
    }

    // Set up callback for new instance file opening
    singleInstanceHandler.setNewInstanceCallback(
        [&mainWindow](const QStringList& files, const CmdLineParams& params)
    {
        mainWindow->raiseAndActivateWindow();
        mainWindow->openFiles(files, params);
    });

    // Show the main window
    mainWindow->showWindow();

    // Open files from command line
    if (!filesToOpen.isEmpty())
    {
        mainWindow->openFiles(filesToOpen, cmdLineParams);
    }

    // ============================================================================
    // Run Event Loop
    // ============================================================================

    int result = app.exec();

    // ============================================================================
    // Cleanup
    // ============================================================================

    mainWindow->destroy();
    mainWindow.reset();

    // Save settings
    settings.saveConfig();

    return result;
}

// Include MOC for QObject-derived classes
#include "main_linux.moc"
