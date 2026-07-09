import sys

with open("unikey-wayland.spec", "r") as f:
    lines = f.readlines()

new_lines = []
inserted = False
for line in lines:
    new_lines.append(line)
    if line.startswith("Summary:") and not inserted:
        new_lines.append("Packager:       Trương Hiếu\n")
        new_lines.append("Vendor:         Trương Hiếu\n")
        inserted = True

with open("unikey-wayland.spec", "w") as f:
    f.writelines(new_lines)
