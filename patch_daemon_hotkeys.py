import re

with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Add variables for Ctrl+Shift detection
init_vars = """    std::vector<int> word;
    int cur_tone = 0, tone_pos = -1;
    bool held[KEY_MAX] = {};
    bool other_pressed = false;
"""
code = re.sub(r'    std::vector<int> word;\n    int cur_tone = 0, tone_pos = -1;\n    bool held\[KEY_MAX\] = \{\};', init_vars, code)

# Update hotkey logic
hotkey_logic = """
        if (value == 0) {
            held[code] = false;
            if (code == KEY_LEFTCTRL || code == KEY_RIGHTCTRL) {
                if (held[KEY_LEFTSHIFT] || held[KEY_RIGHTSHIFT]) {
                    if (!other_pressed) viet_mode = !viet_mode;
                }
                other_pressed = false;
            } else if (code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT) {
                if (held[KEY_LEFTCTRL] || held[KEY_RIGHTCTRL]) {
                    if (!other_pressed) viet_mode = !viet_mode;
                }
                other_pressed = false;
            }
        }
        else if (value == 1) {
            held[code] = true;
            if (code != KEY_LEFTCTRL && code != KEY_RIGHTCTRL && code != KEY_LEFTSHIFT && code != KEY_RIGHTSHIFT) {
                other_pressed = true;
            }
            if (code == KEY_Z && (held[KEY_LEFTALT] || held[KEY_RIGHTALT])) {
                viet_mode = !viet_mode;
                continue; // Swallow Z
            }
            if (code == KEY_RIGHTALT) {
                viet_mode = !viet_mode;
                continue;
            }
        }
"""
code = re.sub(r'        if \(value == 0\) held\[code\] = false;\n        else if \(value == 1\) held\[code\] = true;\n', hotkey_logic, code)

with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
