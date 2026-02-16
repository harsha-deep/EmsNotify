#!/usr/bin/env bash
set -e

APP=EmsNotify

# Build project
rm -rf build
mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6

cmake --build . --config Release
cd ..

# Prepare AppDir
rm -rf AppDir
mkdir -p AppDir/usr/bin
mkdir -p AppDir/usr/share/applications
mkdir -p AppDir/usr/share/icons/hicolor/256x256/apps

# Copy compiled binary
cp build/$APP AppDir/usr/bin/

# Copy desktop file
cp packaging/$APP.desktop AppDir/usr/share/applications/

# Copy icon
cp resources/icons/$APP.png AppDir/usr/share/icons/hicolor/256x256/apps/

# Download linuxdeploy tools
wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy*.AppImage

# Build AppImage
./linuxdeploy-x86_64.AppImage \
  --appdir AppDir \
  --executable AppDir/usr/bin/$APP \
  --desktop-file AppDir/usr/share/applications/$APP.desktop \
  --icon-file AppDir/usr/share/icons/hicolor/256x256/apps/$APP.png \
  --plugin qt \
  --output appimage

echo "AppImage created!"