import re

with open("wayland-client/src/mainwindow.h", "r") as f:
    h_code = f.read()
h_code = re.sub(r'class QPlainTextEdit;\n', '', h_code)
h_code = re.sub(r'\s*QPlainTextEdit\* m_preeditAppsTextEdit;', '', h_code)
with open("wayland-client/src/mainwindow.h", "w") as f:
    f.write(h_code)

with open("wayland-client/src/mainwindow.cpp", "r") as f:
    cpp_code = f.read()

# Remove UI setup
cpp_code = re.sub(r'\s*// --- Tab Danh sách loại trừ ---\s*QWidget\* tabExclude = new QWidget\(\);\s*QVBoxLayout\* excludeLayout = new QVBoxLayout\(tabExclude\);\s*QLabel\* excludeLabel = new QLabel\("Các ứng dụng tự động dùng gạch chân \(Preedit\):\\n\(Một dòng cho mỗi ứng dụng, ví dụ: kitty, studio, java\)"\);\s*m_preeditAppsTextEdit = new QPlainTextEdit\(this\);\s*excludeLayout->addWidget\(excludeLabel\);\s*excludeLayout->addWidget\(m_preeditAppsTextEdit\);\s*m_tabWidget->addTab\(tabExclude, "Danh sách loại trừ"\);', '', cpp_code)

# Remove loadConfig usage
cpp_code = re.sub(r'\s*if \(root\.contains\("preedit_apps"\)\) \{\s*QJsonArray arr = root\["preedit_apps"\].toArray\(\);\s*QStringList apps;\s*for \(const QJsonValue& val : arr\) \{\s*apps \+= val\.toString\(\);\s*\}\s*m_preeditAppsTextEdit->setPlainText\(apps\.join\("\\n"\)\);\s*\}', '', cpp_code)

# Remove saveConfig usage
cpp_code = re.sub(r'\s*QStringList apps = m_preeditAppsTextEdit->toPlainText\(\)\.split\("\\n", Qt::SkipEmptyParts\);\s*QJsonArray arr;\s*for \(const QString& app : apps\) \{\s*arr\.append\(app\.trimmed\(\)\);\s*\}\s*root\["preedit_apps"\] = arr;', '', cpp_code)

with open("wayland-client/src/mainwindow.cpp", "w") as f:
    f.write(cpp_code)
