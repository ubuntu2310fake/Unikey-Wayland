import re
with open("package_arch.sh", "r") as f:
    code = f.read()

# Change cp of ukw_xkb
code = code.replace("cp ring0-engine/xkb/ukw arch_pkg/usr/share/X11/xkb/symbols/", "mkdir -p arch_pkg/usr/share/unikey-wayland\ncp ring0-engine/xkb/ukw arch_pkg/usr/share/unikey-wayland/ukw_xkb")

# Fix INSTALL script
old_install = """cat <<'INSTALL_EOF' > arch_pkg/.INSTALL
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
INSTALL_EOF"""

new_install = """cat <<'INSTALL_EOF' > arch_pkg/.INSTALL
post_install() {
    cp /usr/share/unikey-wayland/ukw_xkb /usr/share/X11/xkb/symbols/ukw 2>/dev/null || true
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

post_remove() {
    rm -f /usr/share/X11/xkb/symbols/ukw
}
INSTALL_EOF"""

code = code.replace(old_install, new_install)

with open("package_arch.sh", "w") as f:
    f.write(code)
