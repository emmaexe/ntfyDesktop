# <img alt="logo" height="48" src="https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/app/icons/ntfyDesktop.svg"> Ntfy Desktop

![GitHub License](https://img.shields.io/github/license/emmaexe/ntfyDesktop)
![GitHub Tag](https://img.shields.io/github/v/tag/emmaexe/ntfyDesktop?label=Latest%20stable%20version)
![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/emmaexe/ntfyDesktop/total)

A desktop client for [ntfy](https://github.com/binwiederhier/ntfy). Allows you to subscribe to topics from any ntfy server and recieve notifications natively on the desktop.

## Screenshots

WIP

## Want to contribute? Found a bug? Have a question?

[<img alt="github-plural" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/social/github-plural_vector.svg">](https://github.com/emmaexe/ntfyDesktop/issues)
[<img alt="discord-plural" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/social/discord-plural_vector.svg">](https://ln.emmaexe.moe/discord-server)
[<img alt="matrix-plural" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/social/matrix-plural_vector.svg">](https://ln.emmaexe.moe/matrix-server)

## Installation

| Format | Distro | Link |
| :-: | :-: | :-: |
| Flatpak | Any | WIP |
| RPM | Fedora | WIP |
| DEB | Ubuntu | WIP |
| TAR.GZ | Any | WIP |

## Building

### Download the necessary dependencies

#### Fedora

```bash
dnf groupinstall "Development Tools"
dnf install gcc-c++ cmake extra-cmake-modules boost-devel libcurl-devel qt6-qtbase-devel kf6-kcoreaddons-devel kf6-ki18n-devel kf6-knotifications-devel rpm-build
```

#### Ubuntu

⚠️ Until Ubuntu 24.10 releases, KDE Frameworks 6 will not be available on it, and you won't be able to build the project.

```bash
apt install git g++ cmake extra-cmake-modules libcurl4-openssl-dev qt6-base-dev libkf6coreaddons-dev libkf6i18n-dev libkf6notifications-dev
```

#### Others

You will need the following:

- Basic development tools (git, make, etc.)
- A C++ compiler (e.g. g++)
- CMake (with [ECM](https://api.kde.org/frameworks/extra-cmake-modules/html/index.html))
- libcurl development libraries
- Base Qt6 development libraries
- KDE Frameworks' KCoreAddons Ki18n and KNotifications development libraries

### Clone the repository

```bash
git clone https://github.com/emmaexe/ntfyDesktop.git && cd ntfyDesktop
```

#### or download the latest release

```bash
curl -s https://api.github.com/repos/emmaexe/ntfyDesktop/releases/latest | grep "tarball_url" | cut -d '"' -f 4 | xargs curl -L -o ntfyDesktop.tar.gz && mkdir ntfyDesktop && tar -xzf ntfyDesktop.tar.gz -C ntfyDesktop --strip-components=1 && rm ntfyDesktop.tar.gz && cd ntfyDesktop
```

### Build the project

```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

### Create packages for installation

```bash
cd build
cpack
```
