#!/bin/bash
set -e

ND_THREAD_COUNT="${ND_THREAD_COUNT:-4}"

echo "Cloning..."
git clone https://github.com/emmaexe/ntfyDesktop.git .

echo "Setting up build..."
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release -B build -G Ninja

echo "Building..."
cmake --build build --parallel $ND_THREAD_COUNT

echo "Packaging..."
cd build
cpack -G DEB
cp ntfyDesktop-*.deb /home/user/artifacts/
cp ntfyDesktop-*.deb.sha256 /home/user/artifacts/

echo "Done."
