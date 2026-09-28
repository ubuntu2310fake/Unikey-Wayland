with open("debian/unikey-wayland.postinst", "r") as f:
    code = f.read()

dkms_code = """
    # Install DKMS module
    if command -v dkms >/dev/null 2>&1; then
        dkms add -m ukw-driver -v 1.0 || true
        dkms build -m ukw-driver -v 1.0 || true
        dkms install -m ukw-driver -v 1.0 || true
    fi
    
    # Reload systemd
    systemctl daemon-reload || true
    systemctl enable ukw.service || true
    systemctl restart ukw.service || true
"""

code = code.replace('if [ "$1" = "configure" ]; then', 'if [ "$1" = "configure" ]; then' + dkms_code)

with open("debian/unikey-wayland.postinst", "w") as f:
    f.write(code)
