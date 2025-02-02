# <img alt="logo" height="48" src="https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/assets/ntfyDesktop.svg"> Ntfy Desktop

![GitHub License](https://img.shields.io/github/license/emmaexe/ntfyDesktop)
![GitHub Tag](https://img.shields.io/github/v/tag/emmaexe/ntfyDesktop?label=Latest%20stable%20version)
![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/emmaexe/ntfyDesktop/total)

A desktop client for [ntfy](https://github.com/binwiederhier/ntfy). Allows you to subscribe to topics from any ntfy server and recieve notifications natively on the desktop.

## Installation

### Flatpak

Ntfy Desktop is available on flathub:

[<img alt="flathub" height="56" src="https://flathub.org/api/badge?svg&locale=en">](https://flathub.org/apps/moe.emmaexe.ntfyDesktop)

### Fedora

To install Ntfy Desktop on Fedora, first add the repository:

```bash
sudo dnf config-manager addrepo --from-repofile=https://pkg.emmaexe.moe/rpm/emmas-pkgs.repo
```

Then install the app:

```bash
sudo dnf install ntfydesktop
```

### Ubuntu

To install Ntfy Desktop on Ubuntu, first import the gpg key:

```bash
curl -fsSL https://pkg.emmaexe.moe/emmas-packages.pub | sudo gpg --dearmor -o /usr/share/keyrings/emmas-packages.gpg
```

Then add the repository:

```bash
echo "deb [signed-by=/usr/share/keyrings/emmas-packages.gpg] https://pkg.emmaexe.moe/apt stable main" | sudo tee /etc/apt/sources.list.d/emmas-packages.list
```

And finally, install the app:

```bash
sudo apt update
sudo apt install ntfydesktop
```

### Manual installation

[<img alt="fedora" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/available/fedora_vector.svg">](https://github.com/emmaexe/ntfyDesktop/releases/download/v1.4.0/ntfyDesktop-1.4.0.rpm)
[<img alt="ubuntu" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/available/ubuntu_vector.svg">](https://github.com/emmaexe/ntfyDesktop/releases/download/v1.4.0/ntfyDesktop-1.4.0.deb)

You can also download the [latest release](https://github.com/emmaexe/ntfyDesktop/releases/latest) for manual installation.

## Screenshots

![First screenshot](https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/assets/screenshot1.png)

![Second screenshot](https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/assets/screenshot2.png)

![Third screenshot](https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/assets/screenshot3.png)

![Fourth screenshot](https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/assets/screenshot4.png)

## Want to contribute? Found a bug? Have a question?

[<img alt="github-plural" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/social/github-plural_vector.svg">](https://github.com/emmaexe/ntfyDesktop/issues)
[<img alt="discord-plural" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/social/discord-plural_vector.svg">](https://ln.emmaexe.moe/discord-server)
[<img alt="matrix-plural" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/social/matrix-plural_vector.svg">](https://ln.emmaexe.moe/matrix-server)

## Building

### Download the necessary dependencies

#### Fedora

```bash
dnf groupinstall "Development Tools"
dnf install gcc-c++ cmake extra-cmake-modules libcurl-devel qt6-qtbase-devel kf6-kcoreaddons-devel kf6-ki18n-devel kf6-knotifications-devel kf6-kxmlgui-devel rpm-build
```

#### Ubuntu

```bash
apt install git g++ cmake extra-cmake-modules libcurl4-openssl-dev qt6-base-dev libkf6coreaddons-dev libkf6i18n-dev libkf6notifications-dev libkf6xmlgui-dev
```

#### Others

You will need the following:

- Basic development tools (git, make, etc.)
- A C++ compiler (e.g. g++)
- CMake (with [ECM](https://api.kde.org/frameworks/extra-cmake-modules/html/index.html))
- libcurl development libraries
- Base Qt6 development libraries (with SQLite support)
- KDE Frameworks' KCoreAddons, Ki18n, KNotifications and KXmlGui development libraries

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
cd build && cpack
```
