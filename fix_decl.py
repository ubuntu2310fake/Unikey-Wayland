import re
with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Add forward declaration
code = code.replace("std::vector<int> utf8_to_keycodes", "int char_to_keycode(char c);\nstd::vector<int> utf8_to_keycodes")
with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
