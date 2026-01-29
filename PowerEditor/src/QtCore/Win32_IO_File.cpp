// This file is part of Notepad++ project
// Copyright (C)2021 Pavel Nedev (pg.nedev@gmail.com)

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
// Win32_IO_File.cpp - Linux/Qt implementation
// ============================================================================
// This file provides Linux-compatible implementations of Win32_IO_File
// for the Qt6 port. These are stub implementations that use standard C++
// file I/O.
// ============================================================================

#include "MISC/Common/Common.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <locale>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "MISC/Common/FileInterface.h"

using namespace std;

Win32_IO_File::Win32_IO_File(const wchar_t *fname)
{
    if (fname)
    {
        // Convert wchar_t path to UTF-8 for Linux
        _path = wstring2string(std::wstring(fname), CP_UTF8);

        // Check if file exists
        struct stat fileStat;
        bool fileExists = (stat(_path.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode));

        // Open file with appropriate flags
        int flags = O_WRONLY | O_CREAT;
        if (fileExists)
        {
            // Truncate existing file
            flags |= O_TRUNC;
        }
        else
        {
            // Create new file
            flags |= O_TRUNC;
        }

        int fd = open(_path.c_str(), flags, 0644);
        if (fd >= 0)
        {
            _hFile = reinterpret_cast<HANDLE>(static_cast<intptr_t>(fd));
        }
        else
        {
            _dwErrorCode = errno;
        }
    }
}

void Win32_IO_File::close()
{
    if (isOpened())
    {
        int fd = static_cast<int>(reinterpret_cast<intptr_t>(_hFile));

        if (_written)
        {
            // Sync to disk
            fsync(fd);
        }

        ::close(fd);
        _hFile = INVALID_HANDLE_VALUE;
    }
}

bool Win32_IO_File::write(const void *wbuf, size_t buf_size)
{
    if (!isOpened() || (wbuf == nullptr))
        return false;

    int fd = static_cast<int>(reinterpret_cast<intptr_t>(_hFile));

    ssize_t total_written = 0;
    const char* buf = static_cast<const char*>(wbuf);

    while (static_cast<size_t>(total_written) < buf_size)
    {
        ssize_t written = ::write(fd, buf + total_written, buf_size - total_written);
        if (written < 0)
        {
            if (errno == EINTR)
                continue;
            _dwErrorCode = errno;
            return false;
        }
        total_written += written;
    }

    _written = true;
    return true;
}
