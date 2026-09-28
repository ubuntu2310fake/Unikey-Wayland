import re

with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Change syn(fd) to include usleep(15000)
code = re.sub(r'void syn\(int fd\) \{ send_ev\(fd, EV_SYN, SYN_REPORT, 0\); \}', 'void syn(int fd) { send_ev(fd, EV_SYN, SYN_REPORT, 0); usleep(15000); }', code)

# Remove explicit usleep(5000) since syn() now sleeps 15ms
code = re.sub(r'usleep\(\d+\); ', '', code)
# Also remove any dangling usleeps that were on their own lines
code = re.sub(r'\n\s*usleep\(\d+\);', '', code)

with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
