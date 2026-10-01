import re

with open("wayland-client/src/main.cpp", "r") as f:
    code = f.read()

old_main = """    bool disable_tray = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--disable-tray") == 0) disable_tray = true;
    }"""
new_main = """    bool disable_tray = false;
    bool show_guide = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--disable-tray") == 0) disable_tray = true;
        if (strcmp(argv[i], "--show-guide") == 0) show_guide = true;
    }"""
code = code.replace(old_main, new_main)
code = code.replace("MainWindow w(&disable_tray, is_gnome);", "MainWindow w(&disable_tray, is_gnome, nullptr, show_guide);")

with open("wayland-client/src/main.cpp", "w") as f:
    f.write(code)

with open("wayland-client/src/mainwindow.h", "r") as f:
    h_code = f.read()

h_code = h_code.replace("MainWindow(bool* disable_tray, bool is_gnome, QWidget *parent = nullptr);", "MainWindow(bool* disable_tray, bool is_gnome, QWidget *parent = nullptr, bool show_guide = false);")
with open("wayland-client/src/mainwindow.h", "w") as f:
    f.write(h_code)

with open("wayland-client/src/mainwindow.cpp", "r") as f:
    cpp_code = f.read()

cpp_code = cpp_code.replace("MainWindow::MainWindow(bool* disable_tray, bool is_gnome, QWidget *parent)", "MainWindow::MainWindow(bool* disable_tray, bool is_gnome, QWidget *parent, bool show_guide)")

guide_msg = """
    if (show_guide) {
        QMessageBox::information(this, "Cập nhật Kiến trúc Ring-0 (Bản 3.0+)",
            "Unikey Wayland đã được nâng cấp lên kiến trúc Kernel Ring-0 mạnh mẽ hơn!\\n\\n"
            "Để gõ tiếng Việt ở các Desktop Environment, bạn CẦN làm theo hướng dẫn sau:\\n\\n"
            "1. KDE Plasma: Vào Settings -> Keyboard -> Layouts -> Thêm bàn phím 'Vietnamese (Kernel Driver)' (Nằm trong mục English/US). Đặt nó làm mặc định.\\n"
            "2. GNOME: Vào Settings -> Keyboard -> Input Sources -> Thêm 'Vietnamese (Kernel Driver)'.\\n\\n"
            "Chú ý: Bạn KHÔNG CẦN dùng IBus hay Fcitx nữa. Engine Ring-0 hoạt động độc lập ở tầng hạt nhân hệ điều hành!");
    }
"""
cpp_code = cpp_code.replace("loadConfig();", "loadConfig();" + guide_msg)
with open("wayland-client/src/mainwindow.cpp", "w") as f:
    f.write(cpp_code)
