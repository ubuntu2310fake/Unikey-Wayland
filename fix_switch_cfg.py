import re
with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()
code = code.replace("bool viet_mode = true;", "bool viet_mode = true;\nint switch_cfg = 0;")
code = code.replace("    int switch_cfg = 0;\n", "")
with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
