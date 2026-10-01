import re

with open("wayland-client/src/mainwindow.cpp", "r") as f:
    code = f.read()

# Add QMessageBox include
if "#include <QMessageBox>" not in code:
    code = "#include <QMessageBox>\n" + code

# Fix constructor signature
old_sig = "MainWindow::MainWindow(bool* p_viet_mode, bool is_gnome, QWidget *parent)"
new_sig = "MainWindow::MainWindow(bool* p_viet_mode, bool is_gnome, QWidget *parent, bool show_guide)"
code = code.replace(old_sig, new_sig)

with open("wayland-client/src/mainwindow.cpp", "w") as f:
    f.write(code)

with open("wayland-client/src/mainwindow.h", "r") as f:
    h_code = f.read()

# Fix header signature if it was wrong
old_h_sig = "MainWindow(bool* p_viet_mode, bool is_gnome, QWidget *parent = nullptr);"
new_h_sig = "MainWindow(bool* p_viet_mode, bool is_gnome, QWidget *parent = nullptr, bool show_guide = false);"
h_code = h_code.replace(old_h_sig, new_h_sig)

with open("wayland-client/src/mainwindow.h", "w") as f:
    f.write(h_code)

with open("wayland-client/src/main.cpp", "r") as f:
    main_code = f.read()
    
# Fix instantiation in main.cpp if needed
old_inst = "MainWindow w(&disable_tray, is_gnome);"
new_inst = "MainWindow w(&disable_tray, is_gnome, nullptr, show_guide);"
main_code = main_code.replace(old_inst, new_inst)

with open("wayland-client/src/main.cpp", "w") as f:
    f.write(main_code)
