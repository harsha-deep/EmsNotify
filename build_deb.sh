#!/usr/bin/env bash
set -e

mkdir -p deb-package/DEBIAN deb-package/usr/bin deb-package/usr/share/applications deb-package/usr/share/icons/hicolor/256x256/apps

cp build/EmsNotify deb-package/usr/bin/
cp AppDir/usr/share/applications/emsnotify.desktop deb-package/usr/share/applications/
cp EmsNotify/resources/icons/clock.png deb-package/usr/share/icons/hicolor/256x256/apps/emsnotify.png

cat > deb-package/DEBIAN/control <<EOF
Package: emsnotify
Version: 1.0
Section: utils
Priority: optional
Architecture: amd64
Maintainer: Centre for Smart Governance
Description: EMS Notify - Employee Management System Notifier
EOF

dpkg-deb --build deb-package
echo "DEB built: $(pwd)/deb-package.deb"