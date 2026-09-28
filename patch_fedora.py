import re

with open("unikey-wayland.spec", "r") as f:
    spec = f.read()

# Replace sources
old_sources = """Source0:        unikey-wayland
Source1:        io.github.ubuntu2310fake.UnikeyWayland.desktop
Source2:        io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
Source3:        io.github.ubuntu2310fake.UnikeyWayland.svg
Source4:        ibus-engine-unikey-wayland
Source5:        unikey-wayland.xml
Source6:        ibus-setup-unikey-wayland.desktop"""

new_sources = """Source0:        unikey-wayland
Source1:        io.github.ubuntu2310fake.UnikeyWayland.desktop
Source2:        io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
Source3:        io.github.ubuntu2310fake.UnikeyWayland.svg
Source4:        ukw_daemon
Source5:        dkms.conf
Source6:        Makefile
Source7:        ukw_driver.c
Source8:        ukw.service
Source9:        ukw_xkb"""

spec = spec.replace(old_sources, new_sources)

# Add DKMS requirement
spec = spec.replace("Requires:       qt6-qtwayland", "Requires:       qt6-qtwayland\nRequires:       dkms")

# Replace %post
old_post = """%post
# Comment out fcitx and ibus in system-wide profiles
sed -i 's/^export.*fcitx/#&/g' /etc/profile.d/*.sh 2>/dev/null || true
sed -i 's/^export.*ibus/#&/g' /etc/profile.d/*.sh 2>/dev/null || true

# Disable fcitx and ibus in user directories and hide autostart
for d in /home/*; do
    if [ -d "$d" ]; then
        sed -i 's/^export.*fcitx/#&/g' "$d/.bashrc" "$d/.profile" "$d/.xprofile" 2>/dev/null || true
        sed -i 's/^export.*ibus/#&/g' "$d/.bashrc" "$d/.profile" "$d/.xprofile" 2>/dev/null || true
        
        mkdir -p "$d/.config/autostart"
        echo -e "[Desktop Entry]\\nHidden=true" > "$d/.config/autostart/org.fcitx.Fcitx5.desktop"
        echo -e "[Desktop Entry]\\nHidden=true" > "$d/.config/autostart/imsettings-start.desktop"
        
        # Try to fix permissions
        chown -R $(stat -c "%U:%G" "$d") "$d/.config/autostart" 2>/dev/null || true
    fi
done"""

new_post = """%post
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
"""
spec = spec.replace(old_post, new_post)

# Replace %install
old_install = """%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/libexec
mkdir -p %{buildroot}/usr/share/ibus/component
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/metainfo
mkdir -p %{buildroot}/usr/share/icons/hicolor/scalable/apps

# Copy our pre-compiled files from the SOURCES directory
cp %{SOURCE0} %{buildroot}/usr/bin/unikey-wayland
cp %{SOURCE1} %{buildroot}/usr/share/applications/io.github.ubuntu2310fake.UnikeyWayland.desktop
cp %{SOURCE2} %{buildroot}/usr/share/metainfo/io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
cp %{SOURCE3} %{buildroot}/usr/share/icons/hicolor/scalable/apps/io.github.ubuntu2310fake.UnikeyWayland.svg
cp %{SOURCE4} %{buildroot}/usr/libexec/ibus-engine-unikey-wayland
cp %{SOURCE5} %{buildroot}/usr/share/ibus/component/unikey-wayland.xml
cp %{SOURCE6} %{buildroot}/usr/share/applications/ibus-setup-unikey-wayland.desktop

# Ensure correct permissions
chmod 755 %{buildroot}/usr/bin/unikey-wayland
chmod 755 %{buildroot}/usr/libexec/ibus-engine-unikey-wayland
chmod 644 %{buildroot}/usr/share/applications/io.github.ubuntu2310fake.UnikeyWayland.desktop
chmod 644 %{buildroot}/usr/share/metainfo/io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
chmod 644 %{buildroot}/usr/share/icons/hicolor/scalable/apps/io.github.ubuntu2310fake.UnikeyWayland.svg
chmod 644 %{buildroot}/usr/share/ibus/component/unikey-wayland.xml
chmod 644 %{buildroot}/usr/share/applications/ibus-setup-unikey-wayland.desktop"""

new_install = """%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/src/ukw-driver-1.0
mkdir -p %{buildroot}/usr/lib/systemd/system
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

# Ensure correct permissions
chmod 755 %{buildroot}/usr/bin/unikey-wayland
chmod 755 %{buildroot}/usr/bin/ukw_daemon
chmod 644 %{buildroot}/usr/share/applications/io.github.ubuntu2310fake.UnikeyWayland.desktop
chmod 644 %{buildroot}/usr/share/metainfo/io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
chmod 644 %{buildroot}/usr/share/icons/hicolor/scalable/apps/io.github.ubuntu2310fake.UnikeyWayland.svg
"""
spec = spec.replace(old_install, new_install)

# Replace %files
old_files = """%files
/usr/bin/unikey-wayland
/usr/share/applications/io.github.ubuntu2310fake.UnikeyWayland.desktop
/usr/share/metainfo/io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
/usr/share/icons/hicolor/scalable/apps/io.github.ubuntu2310fake.UnikeyWayland.svg
%{_libexecdir}/ibus-engine-unikey-wayland
%{_datadir}/ibus/component/unikey-wayland.xml
%{_datadir}/applications/ibus-setup-unikey-wayland.desktop"""

new_files = """%files
/usr/bin/unikey-wayland
/usr/bin/ukw_daemon
/usr/share/applications/io.github.ubuntu2310fake.UnikeyWayland.desktop
/usr/share/metainfo/io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml
/usr/share/icons/hicolor/scalable/apps/io.github.ubuntu2310fake.UnikeyWayland.svg
/usr/src/ukw-driver-1.0/dkms.conf
/usr/src/ukw-driver-1.0/Makefile
/usr/src/ukw-driver-1.0/ukw_driver.c
/usr/lib/systemd/system/ukw.service
/usr/share/X11/xkb/symbols/ukw"""
spec = spec.replace(old_files, new_files)

with open("unikey-wayland.spec", "w") as f:
    f.write(spec)

with open(".github/workflows/ci-cd.yml", "r") as f:
    yml = f.read()

# Replace file copies in ci-cd.yml
old_copies = """          cp wayland-client/build/ibus-engine-unikey-wayland ~/rpmbuild/SOURCES/
          cp ibus-engine/unikey-wayland.xml ~/rpmbuild/SOURCES/
          cp ibus-engine/ibus-setup-unikey-wayland.desktop ~/rpmbuild/SOURCES/"""
new_copies = """          cp wayland-client/build/ukw_daemon ~/rpmbuild/SOURCES/
          cp ring0-engine/dkms.conf ~/rpmbuild/SOURCES/
          cp ring0-engine/Makefile ~/rpmbuild/SOURCES/
          cp ring0-engine/ukw_driver.c ~/rpmbuild/SOURCES/
          cp ring0-engine/ukw.service ~/rpmbuild/SOURCES/
          cp ring0-engine/xkb/ukw ~/rpmbuild/SOURCES/ukw_xkb"""
yml = yml.replace(old_copies, new_copies)

with open(".github/workflows/ci-cd.yml", "w") as f:
    f.write(yml)
