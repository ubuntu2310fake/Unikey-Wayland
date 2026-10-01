Name:           unikey-wayland
Version:        3.0.3
Release:        1%{?dist}
Summary:        Unikey Wayland Input Method for Vietnamese
Packager:       Trương Hiếu
Vendor:         Trương Hiếu

Source0:        unikey-wayland
Source1:        io.github.ubuntu2310fake.UnikeyWayland.desktop
Source2:        io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
Source3:        io.github.ubuntu2310fake.UnikeyWayland.svg
Source4:        ukw_daemon
Source5:        dkms.conf
Source6:        Makefile
Source7:        ukw_driver.c
Source8:        ukw.service
Source9:        ukw_xkb
Source10:       ukw.conf

License:        GPL-2.0-or-later
URL:            https://github.com/ubuntu2310fake/Unikey-Wayland

# Disable debuginfo package generation
%define debug_package %{nil}

%description
Unikey-Wayland is a lightweight Vietnamese input method for Wayland environments, powered by the UniKey engine and Qt 6 GUI.

%post
if command -v dkms >/dev/null 2>&1; then
    dkms add -m ukw-driver -v 1.0 || true
    dkms build -m ukw-driver -v 1.0 || true
    dkms install -m ukw-driver -v 1.0 || true
fi
systemctl daemon-reload || true
systemctl enable ukw.service || true
systemctl restart ukw.service || true

%preun
if [ "$1" = "0" ]; then
    systemctl stop ukw.service || true
    systemctl disable ukw.service || true
    if command -v dkms >/dev/null 2>&1; then
        dkms remove -m ukw-driver -v 1.0 --all || true
    fi
fi


%prep
# Nothing to prepare since we are packaging precompiled binaries

%build
# Nothing to build since we are packaging precompiled binaries

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/src/ukw-driver-1.0
mkdir -p %{buildroot}/usr/lib/systemd/system
mkdir -p %{buildroot}/usr/lib/modules-load.d
mkdir -p %{buildroot}/usr/share/X11/xkb/symbols
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/metainfo
mkdir -p %{buildroot}/usr/share/icons/hicolor/scalable/apps

# Copy our pre-compiled files from the SOURCES directory
cp %{SOURCE0} %{buildroot}/usr/bin/unikey-wayland
cp %{SOURCE1} %{buildroot}/usr/share/applications/io.github.ubuntu2310fake.UnikeyWayland.desktop
cp %{SOURCE2} %{buildroot}/usr/share/metainfo/io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
cp %{SOURCE3} %{buildroot}/usr/share/icons/hicolor/scalable/apps/io.github.ubuntu2310fake.UnikeyWayland.svg
cp %{SOURCE4} %{buildroot}/usr/bin/ukw_daemon
cp %{SOURCE5} %{buildroot}/usr/src/ukw-driver-1.0/dkms.conf
cp %{SOURCE6} %{buildroot}/usr/src/ukw-driver-1.0/Makefile
cp %{SOURCE7} %{buildroot}/usr/src/ukw-driver-1.0/ukw_driver.c
cp %{SOURCE8} %{buildroot}/usr/lib/systemd/system/ukw.service
cp %{SOURCE9} %{buildroot}/usr/share/X11/xkb/symbols/ukw
cp %{SOURCE10} %{buildroot}/usr/lib/modules-load.d/ukw.conf

# Ensure correct permissions
chmod 755 %{buildroot}/usr/bin/unikey-wayland
chmod 755 %{buildroot}/usr/bin/ukw_daemon
chmod 644 %{buildroot}/usr/share/applications/io.github.ubuntu2310fake.UnikeyWayland.desktop
chmod 644 %{buildroot}/usr/share/metainfo/io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
chmod 644 %{buildroot}/usr/share/icons/hicolor/scalable/apps/io.github.ubuntu2310fake.UnikeyWayland.svg
chmod 644 %{buildroot}/usr/lib/modules-load.d/ukw.conf


%files
/usr/bin/unikey-wayland
/usr/bin/ukw_daemon
/usr/share/applications/io.github.ubuntu2310fake.UnikeyWayland.desktop
/usr/share/metainfo/io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
/usr/share/icons/hicolor/scalable/apps/io.github.ubuntu2310fake.UnikeyWayland.svg
/usr/src/ukw-driver-1.0/dkms.conf
/usr/src/ukw-driver-1.0/Makefile
/usr/src/ukw-driver-1.0/ukw_driver.c
/usr/lib/systemd/system/ukw.service
/usr/lib/modules-load.d/ukw.conf
/usr/share/X11/xkb/symbols/ukw

%changelog
* Sun Aug 09 2026 Trương Hiếu - 2.0.10-1
- Windows Edition: Optimize KeyboardHookProc latency and batch SendInput calls to eliminate key stroke lag
* Sun Aug 09 2026 Trương Hiếu - 2.0.9-1
- Fix missing IBus engine binary and component XML in RPM package for GNOME support (#5)
- Auto-detect Google Workspace (Docs, Sheets, Slides, Forms) and Discord for Preedit mode
* Fri Jul 17 2026 Trương Hiếu - 2.0.8-1
- Fix missing KDE Wayland Virtual Keyboard metadata in desktop file
* Fri Jul 17 2026 Trương Hiếu - 2.0.6-1
- Sửa XML dể đưa lên kho ứng dụng
* Fri Jul 10 2026 Trương Hiếu - 1.0.3-1
- Add fallback IBus engine for GNOME Wayland desktop environment
- Fix preedit text cursor jumping/duplication issues on Terminal emulators
- Fix normal mode character duplication in Google Chrome and Electron apps on Wayland using sleep-delayed Backspaces

* Wed Jul 08 2026 Trương Hiếu - 1.0.2-1
- Fix duplicate characters in Wayland Terminal Emulators
- Add Terminal Mode toggle in Tray Menu

* Tue Jul 07 2026 Trương Hiếu - 1.0.0-1
- Initial release under the name unikey-wayland
