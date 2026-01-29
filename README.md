What is Notepad++ ?
===================

This is a fork of Notepad++ where I am experimenting with converting a large windows based application into a Linux native application.

[![GitHub release](https://img.shields.io/github/release/notepad-plus-plus/notepad-plus-plus.svg)](../../releases/latest)&nbsp;&nbsp;&nbsp;&nbsp;[![Build Status](https://img.shields.io/github/actions/workflow/status/notepad-plus-plus/notepad-plus-plus/CI_build.yml)](https://github.com/notepad-plus-plus/notepad-plus-plus/actions/workflows/CI_build.yml)
&nbsp;&nbsp;&nbsp;&nbsp;[![Join the discussions at https://community.notepad-plus-plus.org/](https://notepad-plus-plus.org/assets/images/NppCommunityBadge.svg)](https://community.notepad-plus-plus.org/)

Notepad++ is a free (free as in both "free speech" and "free beer") source code
editor and Notepad replacement that supports several programming languages and
natural languages. Running in the MS Windows environment, its use is governed by
[GPL License](LICENSE).

See the [Notepad++ official site](https://notepad-plus-plus.org/) for more information.

Notepad++ for Linux
-------------------

Notepad++ is now available on Linux as a native application built with Qt6.
The Linux port maintains feature parity with the Windows version through a
comprehensive Platform Abstraction Layer.

### Linux Features

- **Native Qt6 Application**: No Wine or emulation required
- **XDG Compliance**: Follows Linux desktop standards
- **KDE Plasma Integration**: Enhanced support for KDE desktop
- **Plugin Support**: Growing compatibility with Notepad++ plugins
- **Multiple Installation Methods**: Package manager, Flatpak, AppImage, or build from source

### Quick Start for Linux

```bash
# Build from source
mkdir build && cd build
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

See the [Linux documentation](docs/) for detailed information:
- [Build Instructions](docs/BUILD.md)
- [Installation Guide](docs/INSTALL.md)
- [User Manual](docs/USER_MANUAL.md)
- [Architecture Documentation](docs/ARCHITECTURE.md)
- [FAQ](docs/FAQ.md)


Supported OS
------------

### Windows

All the Windows systems still supported by Microsoft are supported by Notepad++. However, not all Notepad++ users can or want to use the newest system. Here is the [Supported systems information](SUPPORTED_SYSTEM.md) you may need in case you are one of them.

### Linux

Notepad++ for Linux is tested on the following distributions:

- Ubuntu 22.04 LTS and later
- Fedora 39 and later
- Arch Linux / Manjaro
- openSUSE Tumbleweed and Leap 15.5+
- Debian 12+

See [INSTALL.md](docs/INSTALL.md) for installation options and [BUILD.md](docs/BUILD.md) for build instructions.




Build Notepad++
---------------

### Windows

Please follow [build guide](BUILD.md) to build Notepad++ from source on Windows.

### Linux

Please follow [Linux build guide](docs/BUILD.md) to build Notepad++ from source on Linux.

Quick start:
```bash
mkdir build && cd build
cmake ../PowerEditor/src -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```


Documentation
-------------

- [Windows Build Guide](BUILD.md)
- [Linux Build Guide](docs/BUILD.md)
- [Linux Installation Guide](docs/INSTALL.md)
- [Linux User Manual](docs/USER_MANUAL.md)
- [Architecture Documentation](docs/ARCHITECTURE.md)
- [Porting Guide](docs/PORTING.md)
- [FAQ](docs/FAQ.md)
- [Changelog](docs/CHANGELOG.md)

Contribution
------------

Contributions are welcome. Be mindful of our [Contribution Rules](CONTRIBUTING.md) to increase the likelihood of your contribution getting accepted.

For Linux-specific contributions, see [Linux Contributing Guide](docs/CONTRIBUTING.md).

[Notepad++ Contributors](https://github.com/notepad-plus-plus/notepad-plus-plus/graphs/contributors)


