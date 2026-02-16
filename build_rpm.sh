#!/usr/bin/env bash
set -e

export VERSION=1.0

mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

cat > ~/rpmbuild/SPECS/emsnotify.spec <<EOF
Name: emsnotify
Version: $VERSION
Release: 1
Summary: EMS Notify - Employee Management System Notifier
License: Proprietary

%description
A system tray application for EMS notifications

%install
mkdir -p %{buildroot}/usr/bin %{buildroot}/usr/share/applications %{buildroot}/usr/share/icons/hicolor/256x256/apps
cp $(pwd)/build/EmsNotify %{buildroot}/usr/bin/
cp $(pwd)/AppDir/usr/share/applications/emsnotify.desktop %{buildroot}/usr/share/applications/
cp $(pwd)/EmsNotify/resources/icons/clock.png %{buildroot}/usr/share/icons/hicolor/256x256/apps/

%files
/usr/bin/EmsNotify
/usr/share/applications/emsnotify.desktop
/usr/share/icons/hicolor/256x256/apps/emsnotify.png
EOF

rpmbuild -bb ~/rpmbuild/SPECS/emsnotify.spec
cp ~/rpmbuild/RPMS/x86_64/*.rpm .
echo "RPM built!"