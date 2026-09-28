import re

with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Make tap much safer (30ms)
old_tap = """void tap(int fd, int code) {
    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(15000);
}"""
new_tap = """void tap(int fd, int code) {
    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(30000);
    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(30000);
}"""
code = code.replace(old_tap, new_tap)

# Make tap_shift much safer (30ms)
old_shift = """void tap_shift(int fd, int code) {
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 1); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 0); syn(fd); usleep(15000);
}"""
new_shift = """void tap_shift(int fd, int code) {
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 1); syn(fd); usleep(30000);
    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(30000);
    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(30000);
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 0); syn(fd); usleep(30000);
}"""
code = code.replace(old_shift, new_shift)

# Add delay before typing macro
old_macro = """                    for (int i = 0; i < bs_count; i++) tap(g_fd, KEY_BACKSPACE);
                    
                    std::vector<int> v = utf8_to_keycodes(m_map[k]);"""
new_macro = """                    for (int i = 0; i < bs_count; i++) tap(g_fd, KEY_BACKSPACE);
                    usleep(50000); // Give Wayland compositor 50ms to process backspaces
                    
                    std::vector<int> v = utf8_to_keycodes(m_map[k]);"""
code = code.replace(old_macro, new_macro)

with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
