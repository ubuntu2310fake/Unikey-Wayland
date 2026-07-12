Name:           unikey-wayland
Version:        1.0.4
Release:        1%{?dist}
Summary:        Unikey Wayland Input Method for Vietnamese
Packager:       Trương Hiếu
Vendor:         Trương Hiếu

Source0:        unikey-wayland
Source1:        unikey-wayland.desktop
Source2:        unikey-wayland.metainfo.xml

License:        GPL-2.0-or-later
URL:            https://github.com/ubuntu2310fake/Unikey-Wayland

# Disable debuginfo package generation
%define debug_package %{nil}

%description
Unikey-Wayland is a lightweight Vietnamese input method for Wayland environments, powered by the UniKey engine and Qt 6 GUI.

%post
# Comment out fcitx and ibus in system-wide profiles
sed -i 's/^export.*fcitx/#&/g' /etc/profile.d/*.sh 2>/dev/null || true
sed -i 's/^export.*ibus/#&/g' /etc/profile.d/*.sh 2>/dev/null || true

# Disable fcitx and ibus in user directories and hide autostart
for d in /home/*; do
    if [ -d "$d" ]; then
        sed -i 's/^export.*fcitx/#&/g' "$d/.bashrc" "$d/.profile" "$d/.xprofile" 2>/dev/null || true
        sed -i 's/^export.*ibus/#&/g' "$d/.bashrc" "$d/.profile" "$d/.xprofile" 2>/dev/null || true
        
        mkdir -p "$d/.config/autostart"
        echo -e "[Desktop Entry]\nHidden=true" > "$d/.config/autostart/org.fcitx.Fcitx5.desktop"
        echo -e "[Desktop Entry]\nHidden=true" > "$d/.config/autostart/imsettings-start.desktop"
        
        # Try to fix permissions
        chown -R $(stat -c "%U:%G" "$d") "$d/.config/autostart" 2>/dev/null || true
    fi
done

%prep
# Nothing to prepare since we are packaging precompiled binaries

%build
# Nothing to build since we are packaging precompiled binaries

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/metainfo

# Copy our pre-compiled files from the SOURCES directory
cp %{SOURCE0} %{buildroot}/usr/bin/unikey-wayland
cp %{SOURCE1} %{buildroot}/usr/share/applications/unikey-wayland.desktop
cp %{SOURCE2} %{buildroot}/usr/share/metainfo/unikey-wayland.metainfo.xml

# Ensure correct permissions
chmod 755 %{buildroot}/usr/bin/unikey-wayland
chmod 644 %{buildroot}/usr/share/applications/unikey-wayland.desktop
chmod 644 %{buildroot}/usr/share/metainfo/unikey-wayland.metainfo.xml

%files
/usr/bin/unikey-wayland
/usr/share/applications/unikey-wayland.desktop
/usr/share/metainfo/unikey-wayland.metainfo.xml

%changelog
* Fri Jul 10 2026 Trương Hiếu - 1.0.3-1
- Add fallback IBus engine for GNOME Wayland desktop environment
- Fix preedit text cursor jumping/duplication issues on Terminal emulators
- Fix normal mode character duplication in Google Chrome and Electron apps on Wayland using sleep-delayed Backspaces

* Wed Jul 08 2026 Trương Hiếu - 1.0.2-1
- Fix duplicate characters in Wayland Terminal Emulators
- Add Terminal Mode toggle in Tray Menu

* Tue Jul 07 2026 Trương Hiếu - 1.0.0-1
- Initial release under the name unikey-wayland
