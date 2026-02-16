#!/usr/bin/env bash
set -e

sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build qt6-base-dev qt6-base-dev-tools qt6-tools-dev \
                        libqt6networkauth6-dev libgl1-mesa-dev libxkbcommon-dev libvulkan-dev \
                        libxcb-cursor0 rpm patchelf file libfuse2 debhelper

# Build binary
echo "Building app..."
rm -rf build AppDir
cmake -B build -S EmsNotify -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)

# Prepare AppDir
mkdir -p AppDir/usr/bin AppDir/usr/share/applications AppDir/usr/share/icons/hicolor/256x256/apps
cp build/EmsNotify AppDir/usr/bin/
cat > AppDir/usr/share/applications/emsnotify.desktop <<EOF
[Desktop Entry]
Type=Application
Name=EMS Notify
Exec=EmsNotify
Icon=emsnotify
Categories=Utility;
Terminal=false
EOF
cp EmsNotify/resources/icons/clock.png AppDir/usr/share/icons/hicolor/256x256/apps/emsnotify.png