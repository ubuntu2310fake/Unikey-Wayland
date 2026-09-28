import re

with open("wayland-client/src/mainwindow.h", "r") as f:
    h_code = f.read()
h_code = re.sub(r'\s*// Preedit exclude/include apps list\s*class\s*public:', '\npublic:', h_code)
with open("wayland-client/src/mainwindow.h", "w") as f:
    f.write(h_code)

with open("wayland-client/src/mainwindow.cpp", "r") as f:
    cpp_code = f.read()
cpp_code = re.sub(r'\s*connect\(m_preeditAppsTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::applySettings\);', '', cpp_code)
with open("wayland-client/src/mainwindow.cpp", "w") as f:
    f.write(cpp_code)
