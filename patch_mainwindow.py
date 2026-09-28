import re

with open("wayland-client/src/mainwindow.cpp", "r") as f:
    code = f.read()

send_cmd = r"""        std::string cmd = viet ? "MODE:VI;" : "MODE:EN;";
        cmd += "SWITCH:" + std::to_string(getSwitchKey()) + ";";
        write(fd, cmd.c_str(), cmd.length());"""

code = re.sub(r'        std::string cmd = viet \? "MODE:VI;" : "MODE:EN;";\n        write\(fd, cmd\.c_str\(\), cmd\.length\(\)\);', send_cmd, code)

with open("wayland-client/src/mainwindow.cpp", "w") as f:
    f.write(code)
