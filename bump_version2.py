import re
import datetime

VERSION = "3.0.1"
DATE = datetime.datetime.now().strftime("%Y-%m-%d")

with open("package_arch.sh", "r") as f: code = f.read()
code = re.sub(r'PKGVER="[^"]+"', f'PKGVER="{VERSION}"', code)
with open("package_arch.sh", "w") as f: f.write(code)

with open("unikey-wayland.spec", "r") as f: code = f.read()
code = re.sub(r'Version:\s+[0-9\.]+', f'Version:        {VERSION}', code)
with open("unikey-wayland.spec", "w") as f: f.write(code)

with open("upload_ppa.sh", "r") as f: code = f.read()
code = re.sub(r'VERSION="[^"]+"', f'VERSION="{VERSION}"', code)
with open("upload_ppa.sh", "w") as f: f.write(code)

with open("debian/changelog", "r") as f: code = f.read()
lines = code.split("\\n")
lines[0] = f"unikey-wayland ({VERSION}~ppa1~noble) noble; urgency=medium"
with open("debian/changelog", "w") as f: f.write("\\n".join(lines))

try:
    with open("CHANGELOG.md", "r") as f: code = f.read()
except FileNotFoundError:
    code = ""
changelog_entry = f"## [{VERSION}] - {DATE}\\n- Fix Arch Linux packaging conflict with xkeyboard-config\\n- Fix backspace logic deleting consonants and failing to retain capitalized state\\n- Monitor mouse events to reset macro engine properly on focus change\\n- Show first-run configuration guide on launch\\n\\n"
code = changelog_entry + code
with open("CHANGELOG.md", "w") as f: f.write(code)
