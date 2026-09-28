import re

# Fix daemon else if
with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()
code = code.replace("else if (cmd.find(\"MACRO:\") != std::string::npos) {", "if (cmd.find(\"MACRO:\") != std::string::npos) {")
with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)

# Fix saveConfig
with open("wayland-client/src/mainwindow.cpp", "r") as f:
    code = f.read()
sync_add = r"""    for (const auto& pair : m_macros) {
        macro_cmd += pair.first + "=" + pair.second + ";";
    }
    macro_cmd += "SWITCH:" + std::to_string(getSwitchKey()) + ";";
    macro_cmd += (p_viet_mode && *p_viet_mode) ? "MODE:VI;" : "MODE:EN;";"""
code = re.sub(r'    for \(const auto& pair : m_macros\) \{\n        macro_cmd \+= pair\.first \+ "=" \+ pair\.second \+ ";";\n    \}', sync_add, code)
with open("wayland-client/src/mainwindow.cpp", "w") as f:
    f.write(code)

