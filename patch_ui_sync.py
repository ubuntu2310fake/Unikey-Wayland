import re

with open("wayland-client/src/trayicon.h", "r") as f:
    h_code = f.read()
if "QTimer" not in h_code:
    h_code = h_code.replace("#include <QMenu>", "#include <QMenu>\n#include <QTimer>")
    h_code = h_code.replace("void updateIcon();", "void updateIcon();\nprivate slots:\n    void checkStatusFile();")
    h_code = h_code.replace("QIcon m_iconE;", "QIcon m_iconE;\n    QTimer* m_timer;")
    with open("wayland-client/src/trayicon.h", "w") as f:
        f.write(h_code)

with open("wayland-client/src/trayicon.cpp", "r") as f:
    cpp_code = f.read()
if "m_timer = new QTimer" not in cpp_code:
    init_timer = """
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TrayIcon::checkStatusFile);
    m_timer->start(200);
"""
    cpp_code = cpp_code.replace("updateIcon();", "updateIcon();\n" + init_timer)
    
    timer_slot = """
void TrayIcon::checkStatusFile() {
    std::ifstream f("/tmp/ukw_status");
    if (f.is_open()) {
        std::string s;
        f >> s;
        bool is_vi = (s == "VI");
        if (*p_viet_mode != is_vi) {
            *p_viet_mode = is_vi;
            updateIcon();
        }
    }
}
"""
    cpp_code += timer_slot
    with open("wayland-client/src/trayicon.cpp", "w") as f:
        f.write(cpp_code)
