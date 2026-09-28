import re
with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Fix bs_count
old_macro = """                    usleep(5000); send_ev(g_fd, EV_KEY, code, 0); syn(g_fd); // release trigger
                    usleep(5000);
                    release_alpha(g_fd, held);
                    int bs_count = word.size() + 1;
                    if (cur_tone != 0) bs_count++;"""
new_macro = """                    // Do not release trigger to OS because it was never sent as down!
                    release_alpha(g_fd, held);
                    usleep(5000);
                    int bs_count = word.size();
                    if (cur_tone != 0) bs_count++;"""
code = code.replace(old_macro, new_macro)

# Fix tap_shift sleep
code = code.replace("void tap_shift(int fd, int code) {\n    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 1); syn(fd); usleep(1500);\n    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(1500);\n    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(1500);\n    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 0); syn(fd); usleep(1500);\n}", "void tap_shift(int fd, int code) {\n    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 1); syn(fd); usleep(5000);\n    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(5000);\n    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(1500);\n    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 0); syn(fd); usleep(1500);\n}")

with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
