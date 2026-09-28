import re
with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Remove the release trigger and fix bs_count
old_regex = r'send_ev\(g_fd, EV_KEY, code, 0\); syn\(g_fd\); // release trigger\s+release_alpha\(g_fd, held\);\s+int bs_count = word\.size\(\) \+ 1;\s+if \(cur_tone != 0\) bs_count\+\+;'

new_code = """// Do not release trigger to OS because it was never sent as down!
                    release_alpha(g_fd, held);
                    int bs_count = word.size();
                    if (cur_tone != 0) bs_count++;"""

code = re.sub(old_regex, new_code, code)

with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
