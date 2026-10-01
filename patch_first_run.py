with open("wayland-client/src/mainwindow.cpp", "r") as f:
    cpp_code = f.read()

# Replace the explicit show_guide with a persistent check
check_code = """
    QSettings settings("UnikeyWayland", "UnikeyWayland");
    bool guide_shown = settings.value("guide_v3_shown", false).toBool();
    if (!guide_shown || show_guide) {
        QMessageBox::information(this, "Cập nhật Kiến trúc Ring-0 (Bản 3.0+)",
            "Unikey Wayland đã được nâng cấp lên kiến trúc Kernel Ring-0 mạnh mẽ hơn!\\n\\n"
            "Để gõ tiếng Việt ở các Desktop Environment, bạn CẦN làm theo hướng dẫn sau:\\n\\n"
            "1. KDE Plasma: Vào Settings -> Keyboard -> Layouts -> Thêm bàn phím 'Vietnamese (Kernel Driver)' (Nằm trong nhóm English/US). Đặt nó làm mặc định.\\n"
            "2. GNOME: Vào Settings -> Keyboard -> Input Sources -> Thêm 'Vietnamese (Kernel Driver)'.\\n\\n"
            "Chú ý: Bạn KHÔNG CẦN dùng IBus hay Fcitx nữa. Engine Ring-0 hoạt động độc lập ở tầng Kernel!");
        settings.setValue("guide_v3_shown", true);
    }
"""

cpp_code = cpp_code.replace("""    if (show_guide) {
        QMessageBox::information(this, "Cập nhật Kiến trúc Ring-0 (Bản 3.0+)",
            "Unikey Wayland đã được nâng cấp lên kiến trúc Kernel Ring-0 mạnh mẽ hơn!\\n\\n"
            "Để gõ tiếng Việt ở các Desktop Environment, bạn CẦN làm theo hướng dẫn sau:\\n\\n"
            "1. KDE Plasma: Vào Settings -> Keyboard -> Layouts -> Thêm bàn phím 'Vietnamese (Kernel Driver)' (Nằm trong mục English/US). Đặt nó làm mặc định.\\n"
            "2. GNOME: Vào Settings -> Keyboard -> Input Sources -> Thêm 'Vietnamese (Kernel Driver)'.\\n\\n"
            "Chú ý: Bạn KHÔNG CẦN dùng IBus hay Fcitx nữa. Engine Ring-0 hoạt động độc lập ở tầng hạt nhân hệ điều hành!");
    }""", check_code)

cpp_code = "#include <QSettings>\n" + cpp_code

with open("wayland-client/src/mainwindow.cpp", "w") as f:
    f.write(cpp_code)
