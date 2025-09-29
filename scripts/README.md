# Building

Here you will find the build instructions and scripts for arch, ubuntu and fedora, as well as general instructions for other distributions.

## Fedora

The fedora build is done in a container.

The container pulls in the latest fedora image, updates the packages, installs build dependencies and copies over the `build.sh` script.

To build the container, run:

```bash
docker build --build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g) -t ntfydesktopbuilder:fedora ./fedora
```

To build ntfyDesktop, run:

```bash
mkdir -p ./artifacts && docker run --rm -v "$(pwd)/artifacts:/home/user/artifacts" ntfydesktopbuilder:fedora
```

The `ND_THREAD_COUNT` environmental variable can be used to specify the number of threads used when building.

The artifacts directory will be created/used in the current working directory, and inside it the container will leave the build artifacts.

## Ubuntu

The ubuntu build is done in a container.

The container pulls in the latest ubuntu image, updates and upgrades the repositories and packages, installs build dependencies and copies over the `build.sh` script.

To build the container, run:

```bash
docker build --build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g) -t ntfydesktopbuilder:ubuntu ./ubuntu
```

To build ntfyDesktop, run:

```bash
mkdir -p ./artifacts && docker run --rm -v "$(pwd)/artifacts:/home/user/artifacts" ntfydesktopbuilder:ubuntu
```

The `ND_THREAD_COUNT` environmental variable can be used to specify the number of threads used when building.

The artifacts directory will be created/used in the current working directory, and inside it the container will leave the build artifacts.

## Arch

There are no build artifacts for arch as each user builds the app themselves, during installation, from the latest release.

The build/installation is done from the Arch User Repository, the package name is `ntfydesktop`.

The current `PKGBUILD` is also kept here, in the `arch` directory.

Example build/installation using `yay`:

```bash
yay ntfydesktop
```

## Manual build or other distribution

### Manual build

To build manually on one of the officially supported distributions, just follow the steps from the Dockerfile and build.sh files on your host.

### Other distribution

If you want to build ntfyDesktop for another distribution you will need the following:

- Basic development tools (git, make, etc.)
- A C++ compiler (e.g. g++)
- Ninja and CMake (with [ECM](https://api.kde.org/frameworks/extra-cmake-modules/html/index.html))
- The [GNU Perfect Hash Function Generator (gperf)](https://www.gnu.org/software/gperf/manual/gperf.html)
- libcurl development libraries
- Base Qt6 development libraries (with SQLite support)
- KDE Frameworks' KCoreAddons, Ki18n, KNotifications and KXmlGui development libraries

#### Clone the repository

```bash
git clone https://github.com/emmaexe/ntfyDesktop.git && cd ntfyDesktop
```

#### or download the latest release

```bash
curl -s https://api.github.com/repos/emmaexe/ntfyDesktop/releases/latest | grep "tarball_url" | cut -d '"' -f 4 | xargs curl -L -o ntfyDesktop.tar.gz && mkdir ntfyDesktop && tar -xzf ntfyDesktop.tar.gz -C ntfyDesktop --strip-components=1 && rm ntfyDesktop.tar.gz && cd ntfyDesktop
```

#### Build the project

Replace `YOUR_THREADS_HERE` with the number of threads you want to use while building:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build -G Ninja
cmake --build build --parallel YOUR_THREADS_HERE
```

#### Create packages for installation

```bash
cd build && cpack
```
