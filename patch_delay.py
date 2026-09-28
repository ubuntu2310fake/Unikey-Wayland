import re

with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Update tap with usleep(15000)
old_tap = """void tap(int fd, int code) {
    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(1500);
    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(1500);
}"""
new_tap = """void tap(int fd, int code) {
    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(15000);
}"""
code = code.replace(old_tap, new_tap)

# Update tap_shift with usleep(15000)
old_shift = """void tap_shift(int fd, int code) {
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 1); syn(fd); usleep(5000);
    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(5000);
    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(1500);
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 0); syn(fd); usleep(1500);
}"""
new_shift = """void tap_shift(int fd, int code) {
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 1); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 0); syn(fd); usleep(15000);
}"""
code = code.replace(old_shift, new_shift)

with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
