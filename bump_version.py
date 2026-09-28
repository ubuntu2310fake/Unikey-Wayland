import re
import datetime

VERSION = "3.0.0"
DATE = datetime.datetime.now().strftime("%Y-%m-%d")

# 1. package_arch.sh
with open("package_arch.sh", "r") as f: code = f.read()
code = re.sub(r'PKGVER="[^"]+"', f'PKGVER="{VERSION}"', code)
with open("package_arch.sh", "w") as f: f.write(code)

# 2. unikey-wayland.spec
with open("unikey-wayland.spec", "r") as f: code = f.read()
code = re.sub(r'Version:\s+[0-9\.]+', f'Version:        {VERSION}', code)
with open("unikey-wayland.spec", "w") as f: f.write(code)

# 3. upload_ppa.sh
with open("upload_ppa.sh", "r") as f: code = f.read()
code = re.sub(r'VERSION="[^"]+"', f'VERSION="{VERSION}"', code)
with open("upload_ppa.sh", "w") as f: f.write(code)

# 4. debian/changelog
with open("debian/changelog", "r") as f: code = f.read()
lines = code.split("\n")
lines[0] = f"unikey-wayland ({VERSION}~ppa1~noble) noble; urgency=medium"
with open("debian/changelog", "w") as f: f.write("\n".join(lines))

# 5. CHANGELOG.md
try:
    with open("CHANGELOG.md", "r") as f: code = f.read()
except FileNotFoundError:
    code = ""
changelog_entry = f"## [{VERSION}] - {DATE}\n- Rewrite core engine as Ring-0 C++ Daemon\n- Fix Wayland Dropped Keys bug (0-delay typing)\n- Natively support Unicode UTF-8 Macros in XKB\n- Eliminate IBus Bamboo dependencies\n\n"
code = changelog_entry + code
with open("CHANGELOG.md", "w") as f: f.write(code)
