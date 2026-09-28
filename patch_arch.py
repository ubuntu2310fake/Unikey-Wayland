import re
with open("package_arch.sh", "r") as f:
    code = f.read()

# Add directories
code = code.replace("mkdir -p arch_pkg/usr/libexec", "mkdir -p arch_pkg/usr/libexec\nmkdir -p arch_pkg/usr/src/ukw-driver-1.0\nmkdir -p arch_pkg/usr/lib/systemd/system\nmkdir -p arch_pkg/usr/share/X11/xkb/symbols")

# Add copy commands
copy_cmd = """# Copy Ring-0 components
cp wayland-client/build/ukw_daemon arch_pkg/usr/bin/
cp ring0-engine/dkms.conf arch_pkg/usr/src/ukw-driver-1.0/
cp ring0-engine/Makefile arch_pkg/usr/src/ukw-driver-1.0/
cp ring0-engine/ukw_driver.c arch_pkg/usr/src/ukw-driver-1.0/
cp ring0-engine/ukw.service arch_pkg/usr/lib/systemd/system/
cp ring0-engine/xkb/ukw arch_pkg/usr/share/X11/xkb/symbols/
"""
code = code.replace("chmod 755 arch_pkg/usr/bin/unikey-wayland", copy_cmd + "\nchmod 755 arch_pkg/usr/bin/unikey-wayland\nchmod 755 arch_pkg/usr/bin/ukw_daemon")

# Add .INSTALL script
install_script = """cat <<'INSTALL_EOF' > arch_pkg/.INSTALL
post_install() {
    if command -v dkms >/dev/null 2>&1; then
        dkms add -m ukw-driver -v 1.0 || true
        dkms build -m ukw-driver -v 1.0 || true
        dkms install -m ukw-driver -v 1.0 || true
    fi
    systemctl daemon-reload || true
    systemctl enable ukw.service || true
    systemctl restart ukw.service || true
}

pre_remove() {
    systemctl stop ukw.service || true
    systemctl disable ukw.service || true
    if command -v dkms >/dev/null 2>&1; then
        dkms remove -m ukw-driver -v 1.0 --all || true
    fi
}
INSTALL_EOF
"""
code = code.replace("touch arch_pkg/.INSTALL", install_script)

# Add dkms dependency
code = code.replace("depend = wayland", "depend = wayland\ndepend = dkms")

with open("package_arch.sh", "w") as f:
    f.write(code)
