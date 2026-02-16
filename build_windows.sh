#!/usr/bin/env bash
set -e

# Requirements:
#  - Qt6 installed
#  - CMake
#  - Visual Studio 2022
#  - Inno Setup

BUILD_DIR=out/build/release
INSTALLER=EMSNotifySetup.exe

echo "Configuring..."
cmake -B "$BUILD_DIR" -S EmsNotify -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64

echo "Building..."
cmake --build "$BUILD_DIR" --config Release

echo "Deploying Qt libs..."
mkdir -p deploy
cp "$BUILD_DIR/Release/EmsNotify.exe" deploy/
windeployqt deploy/EmsNotify.exe

echo "Building installer..."
iscc EmsNotify/emsnotify.iss

echo "Done!"
echo " -> EXE: deploy/EmsNotify.exe"
echo " -> MSI/Installer: $INSTALLER"