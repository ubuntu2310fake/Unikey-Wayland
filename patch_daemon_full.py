import re

with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Remove usleep(15000) from syn
code = code.replace("void syn(int fd) { send_ev(fd, EV_SYN, SYN_REPORT, 0); usleep(15000); }", "void syn(int fd) { send_ev(fd, EV_SYN, SYN_REPORT, 0); }")

# Update tap with usleep(1500)
code = re.sub(r'void tap\(int fd, int code\) \{\n    send_ev\(fd, EV_KEY, code, 1\); syn\(fd\);[ \t]*\n    send_ev\(fd, EV_KEY, code, 0\); syn\(fd\);', 'void tap(int fd, int code) {\n    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(1500);\n    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(1500);', code)

# Update tap_shift with usleep(1500)
code = re.sub(r'void tap_shift\(int fd, int code\) \{\n    send_ev\(fd, EV_KEY, KEY_LEFTSHIFT, 1\); syn\(fd\);[ \t]*\n    send_ev\(fd, EV_KEY, code, 1\); syn\(fd\);[ \t]*\n    send_ev\(fd, EV_KEY, code, 0\); syn\(fd\);[ \t]*\n    send_ev\(fd, EV_KEY, KEY_LEFTSHIFT, 0\); syn\(fd\);', 'void tap_shift(int fd, int code) {\n    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 1); syn(fd); usleep(1500);\n    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(1500);\n    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(1500);\n    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 0); syn(fd); usleep(1500);', code)

# Add update_status() and switch_cfg
init_vars = """    std::vector<int> word;
    int cur_tone = 0, tone_pos = -1;
    bool held[KEY_MAX] = {};
    bool other_pressed = false;
    int switch_cfg = 0;
"""
code = code.replace("    std::vector<int> word;\n    int cur_tone = 0, tone_pos = -1;\n    bool held[KEY_MAX] = {};\n    bool other_pressed = false;\n", init_vars)

# Add update_status() globally
status_func = """
static void update_status() {
    int fd = open("/tmp/ukw_status", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd >= 0) {
        write(fd, viet_mode ? "VI" : "EN", 2);
        close(fd);
    }
}
"""
code = code.replace("bool is_alpha(int c) {", status_func + "\nbool is_alpha(int c) {")

# Update cmd_listener
cmd_listener_old = """            if (cmd.find("MODE:VI") != std::string::npos) {
                viet_mode = true;
            } else if (cmd.find("MODE:EN") != std::string::npos) {
                viet_mode = false;
            }"""
cmd_listener_new = """            if (cmd.find("MODE:VI") != std::string::npos) {
                viet_mode = true; update_status();
            } else if (cmd.find("MODE:EN") != std::string::npos) {
                viet_mode = false; update_status();
            }
            if (cmd.find("SWITCH:0") != std::string::npos) switch_cfg = 0;
            else if (cmd.find("SWITCH:1") != std::string::npos) switch_cfg = 1;
            else if (cmd.find("SWITCH:2") != std::string::npos) switch_cfg = 2;
"""
code = code.replace(cmd_listener_old, cmd_listener_new)

# Update hotkey_logic
hotkey_logic_old = """        if (value == 0) {
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
        }"""
hotkey_logic_new = """        if (value == 0) {
            held[code] = false;
            if (switch_cfg == 0) {
                if (code == KEY_LEFTCTRL || code == KEY_RIGHTCTRL) {
                    if (held[KEY_LEFTSHIFT] || held[KEY_RIGHTSHIFT]) {
                        if (!other_pressed) { viet_mode = !viet_mode; update_status(); }
                    }
                    other_pressed = false;
                } else if (code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT) {
                    if (held[KEY_LEFTCTRL] || held[KEY_RIGHTCTRL]) {
                        if (!other_pressed) { viet_mode = !viet_mode; update_status(); }
                    }
                    other_pressed = false;
                }
            }
        }
        else if (value == 1) {
            held[code] = true;
            if (code != KEY_LEFTCTRL && code != KEY_RIGHTCTRL && code != KEY_LEFTSHIFT && code != KEY_RIGHTSHIFT) {
                other_pressed = true;
            }
            if (switch_cfg == 1 && code == KEY_Z && (held[KEY_LEFTALT] || held[KEY_RIGHTALT])) {
                viet_mode = !viet_mode; update_status();
                continue; // Swallow Z
            }
            if (switch_cfg == 2 && code == KEY_RIGHTALT) {
                viet_mode = !viet_mode; update_status();
                continue;
            }
        }"""
code = code.replace(hotkey_logic_old, hotkey_logic_new)

# Initial update_status
code = code.replace("    std::vector<int> word;\n    int cur_tone = 0, tone_pos = -1;", "    update_status();\n    std::vector<int> word;\n    int cur_tone = 0, tone_pos = -1;")

with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
