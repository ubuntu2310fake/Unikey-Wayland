import re

with open("wayland-client/src/mainwindow.h", "r") as f:
    h_code = f.read()

h_code = re.sub(r'explicit MainWindow\(bool\* p_viet_mode, bool is_gnome = false, QWidget \*parent = nullptr\);',
                'explicit MainWindow(bool* p_viet_mode, bool is_gnome = false, QWidget *parent = nullptr, bool show_guide = false);', h_code)

with open("wayland-client/src/mainwindow.h", "w") as f:
    f.write(h_code)
