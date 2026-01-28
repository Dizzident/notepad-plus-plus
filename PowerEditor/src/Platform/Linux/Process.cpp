// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../Process.h"
#include "../FileSystem.h"
#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QStringList>
#include <QtCore/QObject>
#include <QtGui/QDesktopServices>
#include <QtCore/QDebug>
#include <QtCore/QMimeType>
#include <QtCore/QMimeDatabase>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <cstring>

namespace PlatformLayer {

// Helper functions
namespace {
    QString wstringToQString(const std::wstring& wstr) {
        return QString::fromWCharArray(wstr.c_str());
    }

    std::wstring qstringToWstring(const QString& qstr) {
        return qstr.toStdWString();
    }

    QStringList parseArguments(const std::wstring& args) {
        QStringList result;
        QString current;
        bool inQuotes = false;
        bool escapeNext = false;

        for (wchar_t wc : args) {
            QChar c = QChar::fromLatin1(static_cast<char>(wc));

            if (escapeNext) {
                current.append(c);
                escapeNext = false;
                continue;
            }

            if (c == '\\') {
                escapeNext = true;
                current.append(c);
            }
            else if (c == '"') {
                inQuotes = !inQuotes;
                // Don't include the quote character
            }
            else if (c.isSpace() && !inQuotes) {
                if (!current.isEmpty()) {
                    result.append(current);
                    current.clear();
                }
            }
            else {
                current.append(c);
            }
        }

        if (!current.isEmpty()) {
            result.append(current);
        }

        return result;
    }
}

// ============================================================================
// Linux Implementation of IProcess
// ============================================================================
class ProcessLinux : public IProcess {
public:
    ProcessLinux() = default;
    ~ProcessLinux() override = default;

    ProcessInfo run(const std::wstring& command,
                   const std::wstring& args,
                   const ProcessOptions& options) override {
        ProcessInfo info;

        QString program = wstringToQString(command);
        QStringList arguments = parseArguments(args);

        // Handle elevated execution using pkexec
        if (options.elevated) {
            arguments.prepend(program);
            program = "pkexec";
        }

        QProcess* process = new QProcess();

        // Set working directory
        if (!options.workingDir.empty()) {
            process->setWorkingDirectory(wstringToQString(options.workingDir));
        }

        // Set environment variables
        if (!options.envVars.empty()) {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            for (const auto& var : options.envVars) {
                QString varStr = wstringToQString(var);
                int equalsPos = varStr.indexOf('=');
                if (equalsPos > 0) {
                    env.insert(varStr.left(equalsPos), varStr.mid(equalsPos + 1));
                }
            }
            process->setProcessEnvironment(env);
        }

        // Start the process
        if (options.hidden) {
            process->setStandardOutputFile(QProcess::nullDevice());
            process->setStandardErrorFile(QProcess::nullDevice());
        }

        process->start(program, arguments);

        if (!process->waitForStarted(5000)) {
            delete process;
            info.success = false;
            return info;
        }

        // Store process info
        info.pid = static_cast<uint32_t>(process->processId());
        info.command = command;
        info.success = true;

        // Store the QProcess pointer as native handle
        // Note: Caller must manage this memory or call terminateProcess
        process->setParent(nullptr);  // Detach from Qt parent mechanism

        // If not waiting, we need to keep the process alive
        if (!options.waitForExit) {
            // Connect finished signal to cleanup
            QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [process](int, QProcess::ExitStatus) {
                    process->deleteLater();
                });
            // We can't store the raw pointer since QProcess will delete itself
            // Instead, we'll store 0 and rely on PID for later operations
            info.nativeHandle = nullptr;
        } else {
            process->waitForFinished(-1);
            delete process;
        }

        return info;
    }

    ProcessResult runSync(const std::wstring& command,
                         const std::wstring& args,
                         const ProcessOptions& options) override {
        ProcessResult result;

        QString program = wstringToQString(command);
        QStringList arguments = parseArguments(args);

        // Handle elevated execution
        if (options.elevated) {
            arguments.prepend(program);
            program = "pkexec";
        }

        QProcess process;

        // Set working directory
        if (!options.workingDir.empty()) {
            process.setWorkingDirectory(wstringToQString(options.workingDir));
        }

        // Set environment variables
        if (!options.envVars.empty()) {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            for (const auto& var : options.envVars) {
                QString varStr = wstringToQString(var);
                int equalsPos = varStr.indexOf('=');
                if (equalsPos > 0) {
                    env.insert(varStr.left(equalsPos), varStr.mid(equalsPos + 1));
                }
            }
            process.setProcessEnvironment(env);
        }

        // Set up output capture
        if (options.captureOutput) {
            process.setProcessChannelMode(options.mergeStderr
                ? QProcess::MergedChannels
                : QProcess::SeparateChannels);
        } else {
            if (options.hidden) {
                process.setStandardOutputFile(QProcess::nullDevice());
                process.setStandardErrorFile(QProcess::nullDevice());
            }
        }

        process.start(program, arguments);

        if (!process.waitForStarted(30000)) {
            result.success = false;
            result.errorMessage = L"Failed to start process: " +
                qstringToWstring(process.errorString());
            return result;
        }

        // Capture output if requested
        if (options.captureOutput) {
            // Read all output
            if (!process.waitForFinished(-1)) {
                result.success = false;
                result.errorMessage = L"Process failed: " +
                    qstringToWstring(process.errorString());
                return result;
            }
            result.output = process.readAllStandardOutput().toStdString();
            result.errorOutput = process.readAllStandardError().toStdString();
        } else {
            // Just wait for completion
            if (!process.waitForFinished(-1)) {
                result.success = false;
                result.errorMessage = L"Process failed: " +
                    qstringToWstring(process.errorString());
                return result;
            }
        }

        result.exitCode = static_cast<uint32_t>(process.exitCode());
        result.success = (process.exitStatus() == QProcess::NormalExit);

        return result;
    }

    bool shellOpen(const std::wstring& fileOrUrl,
                  const std::wstring& args,
                  bool elevated) override {
        QString target = wstringToQString(fileOrUrl);

        if (elevated) {
            // Use pkexec to open with elevated privileges
            QString program;
            QStringList arguments;

            // Try to determine the right application
            QMimeType mimeType = QMimeDatabase().mimeTypeForFile(target);
            QString defaultApp = mimeType.defaultApplication();

            if (!defaultApp.isEmpty()) {
                program = "pkexec";
                arguments << defaultApp << target;
            } else {
                // Fall back to xdg-open with pkexec
                program = "pkexec";
                arguments << "xdg-open" << target;
            }

            QProcess process;
            process.start(program, arguments);
            return process.waitForStarted(5000);
        }

        // Use QDesktopServices for normal file/URL opening
        QUrl url = QUrl::fromUserInput(target);
        return QDesktopServices::openUrl(url);
    }

    bool shellOpenWith(const std::wstring& application,
                      const std::wstring& file,
                      const std::wstring& args,
                      bool elevated) override {
        QString program = wstringToQString(application);
        QStringList arguments;
        arguments << wstringToQString(file);

        if (!args.empty()) {
            arguments << parseArguments(args);
        }

        if (elevated) {
            arguments.prepend(program);
            program = "pkexec";
        }

        QProcess process;
        process.start(program, arguments);
        return process.waitForStarted(5000);
    }

    bool waitForProcess(const ProcessInfo& info, uint32_t timeoutMs) override {
        if (info.pid == 0) return false;

        // Use waitpid to wait for the specific process
        pid_t pid = static_cast<pid_t>(info.pid);
        int status;

        if (timeoutMs == 0xFFFFFFFF) {
            // Infinite wait
            pid_t result = waitpid(pid, &status, 0);
            return result == pid;
        } else {
            // Timed wait - need to use a loop with polling
            uint32_t waited = 0;
            const uint32_t pollInterval = 100; // 100ms

            while (waited < timeoutMs) {
                pid_t result = waitpid(pid, &status, WNOHANG);
                if (result == pid) {
                    return true;
                } else if (result == -1) {
                    return false;
                }

                // Sleep for poll interval
                usleep(pollInterval * 1000);
                waited += pollInterval;
            }

            return false; // Timeout
        }
    }

    bool terminateProcess(const ProcessInfo& info, bool force) override {
        if (info.pid == 0) return false;

        pid_t pid = static_cast<pid_t>(info.pid);
        int signal = force ? SIGKILL : SIGTERM;

        int result = kill(pid, signal);
        return result == 0;
    }

    bool isProcessRunning(const ProcessInfo& info) override {
        return isProcessRunning(info.pid);
    }

    bool getExitCode(const ProcessInfo& info, uint32_t& exitCode) override {
        if (info.pid == 0) return false;

        // For processes we've already waited on, we can't get the exit code
        // again via waitpid. In Linux, we would need to track this separately.
        // For now, check if the process is still running
        if (isProcessRunning(info.pid)) {
            return false; // Still running, no exit code yet
        }

        // Process has exited, but we don't have a way to get the exit code
        // after the fact without using non-portable methods
        exitCode = 0;
        return true;
    }

    bool isProcessRunning(uint32_t pid) override {
        if (pid == 0) return false;

        // Check if process exists by sending signal 0
        int result = kill(static_cast<pid_t>(pid), 0);

        if (result == 0) {
            return true;
        } else {
            // ESRCH means process doesn't exist
            return errno != ESRCH;
        }
    }

    uint32_t getCurrentProcessId() override {
        return static_cast<uint32_t>(getpid());
    }

    std::wstring getCurrentProcessPath() override {
        // Read /proc/self/exe symlink
        char buffer[4096];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1) {
            buffer[len] = '\0';
            return qstringToWstring(QString::fromUtf8(buffer));
        }
        return std::wstring();
    }

    bool isCurrentProcessElevated() override {
        // Check if we're running as root
        return geteuid() == 0;
    }

    bool restartElevated(const std::wstring& args) override {
        QString program = wstringToQString(getCurrentProcessPath());
        QStringList arguments = parseArguments(args);

        // Prepend pkexec
        arguments.prepend(program);
        program = "pkexec";

        // Detach and run
        QProcess::startDetached(program, arguments);

        return true;
    }
};

// ============================================================================
// Singleton Accessor
// ============================================================================
IProcess& IProcess::getInstance() {
    static ProcessLinux instance;
    return instance;
}

// ============================================================================
// Static Helper Implementations
// ============================================================================
std::wstring IProcess::quoteArgument(const std::wstring& arg) {
    // Check if quoting is needed
    if (arg.find_first_of(L" \"\t\n\v'") == std::wstring::npos) {
        return arg;
    }

    // Use single quotes for shell-style escaping on Linux
    // Replace any single quotes in the argument
    std::wstring result = L"'";
    for (wchar_t c : arg) {
        if (c == L'\'') {
            result += L"'\"'\"'";  // Close quote, add escaped quote, reopen
        } else {
            result += c;
        }
    }
    result += L'\'';
    return result;
}

std::wstring IProcess::buildCommandLine(const std::wstring& program,
                                       const std::vector<std::wstring>& args) {
    std::wstring cmdLine = quoteArgument(program);

    for (const auto& arg : args) {
        cmdLine += L" " + quoteArgument(arg);
    }

    return cmdLine;
}

std::wstring IProcess::findExecutable(const std::wstring& name) {
    QString program = wstringToQString(name);

    // Use QStandardPaths to find the executable
    QString result = QStandardPaths::findExecutable(program);
    if (!result.isEmpty()) {
        return qstringToWstring(result);
    }

    // Try common executable extensions
    QStringList extensions = { "", ".sh", ".bin" };
    for (const QString& ext : extensions) {
        result = QStandardPaths::findExecutable(program + ext);
        if (!result.isEmpty()) {
            return qstringToWstring(result);
        }
    }

    return std::wstring();
}

// ============================================================================
// Utility Functions
// ============================================================================
namespace ProcessUtils {

ProcessResult execute(const std::wstring& command,
                     const std::vector<std::wstring>& args,
                     const std::wstring& workingDir) {
    QString program = wstringToQString(command);
    QStringList arguments;

    for (const auto& arg : args) {
        arguments << wstringToQString(arg);
    }

    ProcessOptions options;
    options.workingDir = workingDir;
    options.captureOutput = true;
    options.waitForExit = true;

    std::wstring cmdLine;
    for (const auto& arg : args) {
        cmdLine += L" " + IProcess::quoteArgument(arg);
    }

    return IProcess::getInstance().runSync(command, cmdLine.substr(1), options);
}

bool openDocument(const std::wstring& path) {
    return IProcess::getInstance().shellOpen(path, L"", false);
}

bool openUrl(const std::wstring& url) {
    return IProcess::getInstance().shellOpen(url, L"", false);
}

bool runDetached(const std::wstring& command,
                const std::vector<std::wstring>& args,
                const std::wstring& workingDir) {
    std::wstring cmdLine;
    for (const auto& arg : args) {
        cmdLine += L" " + IProcess::quoteArgument(arg);
    }

    ProcessOptions options;
    options.workingDir = workingDir;

    ProcessInfo info = IProcess::getInstance().run(command, cmdLine.substr(1), options);
    return info.success;
}

} // namespace ProcessUtils

} // namespace PlatformLayer
