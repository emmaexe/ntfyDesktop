# <img alt="logo" height="48" src="https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/assets/ntfyDesktop.svg"> Ntfy Desktop

![GitHub License](https://img.shields.io/github/license/emmaexe/ntfyDesktop)
![GitHub Tag](https://img.shields.io/github/v/tag/emmaexe/ntfyDesktop?label=Latest%20stable%20version)
![Flathub Downloads](https://img.shields.io/flathub/downloads/moe.emmaexe.ntfyDesktop?label=downloads%20-%20flathub)
![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/emmaexe/ntfyDesktop/total?label=downloads%20-%20github)

[<img alt="flathub" height="56" src="https://raw.githubusercontent.com/PenPow/Badges/refs/heads/main/src/assets/available/flathub/cozy.svg">](#flatpak)
[<img alt="fedora" height="56" src="https://raw.githubusercontent.com/PenPow/Badges/refs/heads/main/src/assets/supported/fedora/cozy.svg">](#fedora)
[<img alt="ubuntu" height="56" src="https://raw.githubusercontent.com/PenPow/Badges/refs/heads/main/src/assets/supported/ubuntu/cozy.svg">](#ubuntu)
[<img alt="arch" height="56" src="https://raw.githubusercontent.com/PenPow/Badges/refs/heads/main/src/assets/supported/arch/cozy.svg">](#arch)
[<img alt="github" height="56" src="https://raw.githubusercontent.com/PenPow/Badges/refs/heads/main/src/assets/available/github/cozy.svg">](#manual-installation)

A fully featured desktop client for [ntfy](https://github.com/binwiederhier/ntfy) for Linux, built with Qt and C++. Allows you to subscribe to topics from any ntfy server and receive notifications natively on the desktop.

**Features include:**

- Native desktop notifications with:
  - Attachment and icon support, for displaying images and linking to files directly on the notification
  - Notification click event support, just like on Android
  - Notification button support, just like on Android
- Notification history
- Full authentication support and ability to use all protocols that ntfy supports
- Ability to import from an [ntfy android](https://github.com/binwiederhier/ntfy-android) backup
- Customizable behavior when (re)connecting to servers and keeping history
- Builds for `x86_64` and `aarch64` architectures

## Screenshots

![First screenshot](https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/assets/screenshot1.png)

![Second screenshot](https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/assets/screenshot2.png)

![Third screenshot](https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/assets/screenshot3.png)

![Fourth screenshot](https://raw.githubusercontent.com/emmaexe/ntfyDesktop/main/assets/screenshot4.png)

## Installation

### Flatpak

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

### Arch

On Arch Linux, Ntfy Desktop is available in the [Arch User Repository](https://aur.archlinux.org/packages/ntfydesktop), under the name `ntfydesktop`.

Example installation method, using the `yay` AUR helper:

```bash
yay ntfydesktop
```

### Manual installation

You can download the latest build artifacts in the [latest release](https://github.com/emmaexe/ntfyDesktop/releases/latest) for manual installation.

## Building

To build the project, please refer to the [Building Guide](./scripts/README.md)

## Want to contribute? Found a bug? Have a question?

[<img alt="github-plural" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/social/github-plural_vector.svg">](https://github.com/emmaexe/ntfyDesktop/issues)
[<img alt="discord-plural" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/social/discord-plural_vector.svg">](https://ln.emmaexe.moe/discord-server)
[<img alt="matrix-plural" height="56" src="https://raw.githubusercontent.com/emmaexe/devins-badges/v3/assets/cozy/social/matrix-plural_vector.svg">](https://ln.emmaexe.moe/matrix-server)
