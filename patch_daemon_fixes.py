import re

with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Fix 1: Add Shift state to word buffer
old_vector = "std::vector<int> word;"
new_vector = "std::vector<int> word; // negative means shifted"
code = code.replace(old_vector, new_vector)

old_push = "else word.push_back(code);"
new_push = """else {
                bool is_upper = held[KEY_LEFTSHIFT] || held[KEY_RIGHTSHIFT];
                word.push_back(is_upper ? -code : code);
            }"""
code = code.replace(old_push, new_push)

old_tap_start = """for (int i = start_pos; i < word.size(); i++) {
                tap(g_fd, word[i]);
                if (cur_tone != 0 && i == tone_pos) tap(g_fd, cur_tone);
            }"""
new_tap_start = """for (int i = start_pos; i < word.size(); i++) {
                int c = word[i];
                if (c < 0) tap_shift(g_fd, -c);
                else tap(g_fd, c);
                if (cur_tone != 0 && i == tone_pos) tap(g_fd, cur_tone);
            }"""
code = code.replace(old_tap_start, new_tap_start)

# In looks_vietnamese and find_vowel_pos, use abs()
code = re.sub(r'for \(int k : word\) if \(is_vn_special\(k\)\)', 'for (int k : word) if (is_vn_special(abs(k)))', code)
code = re.sub(r'for \(int k : word\) \{\s*bool v = is_vowel\(k\);', 'for (int k : word) {\\n        bool v = is_vowel(abs(k));', code)
code = re.sub(r'int c = buf\[i\];', 'int c = abs(buf[i]);', code)
code = re.sub(r'if \(is_vowel\(buf\[i\]\)\)', 'if (is_vowel(abs(buf[i])))', code)
code = re.sub(r'is_vowel\(buf\[first_vowel_idx - 1\]\)', 'is_vowel(abs(buf[first_vowel_idx - 1]))', code)
code = re.sub(r'buf\[first_vowel_idx-1\]', 'abs(buf[first_vowel_idx-1])', code)
code = re.sub(r'buf\[first_vowel_idx\]', 'abs(buf[first_vowel_idx])', code)
code = re.sub(r'buf\[first_vowel_idx\+1\]', 'abs(buf[first_vowel_idx+1])', code)

# In telex logic, use abs()
code = re.sub(r'word\[i\] == KEY_', 'abs(word[i]) == KEY_', code)

# Fix target replacement to preserve shift
old_replace = """std::vector<int> new_word = word;
            new_word[target_pos] = target;
            if (target_pos2 != -1) new_word[target_pos2] = target2;
            if (cancel_dau) new_word.push_back(code);"""
new_replace = """std::vector<int> new_word = word;
            new_word[target_pos] = (word[target_pos] < 0) ? -target : target;
            if (target_pos2 != -1) new_word[target_pos2] = (word[target_pos2] < 0) ? -target2 : target2;
            if (cancel_dau) new_word.push_back(held[KEY_LEFTSHIFT] || held[KEY_RIGHTSHIFT] ? -code : code);"""
code = code.replace(old_replace, new_replace)

old_replace2 = """if (tone == cur_tone) {
                    new_cur_tone = 0; new_tone_pos = -1;
                    new_word.push_back(code);
                }"""
new_replace2 = """if (tone == cur_tone) {
                    new_cur_tone = 0; new_tone_pos = -1;
                    new_word.push_back(held[KEY_LEFTSHIFT] || held[KEY_RIGHTSHIFT] ? -code : code);
                }"""
code = code.replace(old_replace2, new_replace2)

# Fix 2: Remove +1 bs_count for cur_tone
old_bs = """int bs_count = word.size() - start_pos + 1; // +1 pass through
            if (cur_tone != 0 && tone_pos != -1 && tone_pos >= start_pos) bs_count += 1;"""
new_bs = "int bs_count = word.size() - start_pos + 1;"
code = code.replace(old_bs, new_bs)

old_bs2 = """int bs_count = word.size() - start_pos + 1;
                if (cur_tone != 0 && tone_pos != -1 && tone_pos >= start_pos) bs_count += 1;"""
new_bs2 = "int bs_count = word.size() - start_pos + 1;"
code = code.replace(old_bs2, new_bs2)

# Fix 3: Read from /dev/input/mice
mice_logic = """
    int mice_fd = open("/dev/input/mice", O_RDONLY | O_NONBLOCK);
    
    ukw_event ev;
    while (1) {
        if (mice_fd >= 0) {
            unsigned char mbuf[3];
            if (read(mice_fd, mbuf, 3) == 3) {
                if (mbuf[0] & 1) { // Left click
                    pthread_mutex_lock(&lock);
                    word.clear(); cur_tone = 0; tone_pos = -1;
                    pthread_mutex_unlock(&lock);
                }
            }
        }
        
        if (read(g_fd, &ev, sizeof(ev)) != sizeof(ev)) {
            usleep(1000);
            continue;
        }
"""
code = re.sub(r'ukw_event ev;\n\s*while \(read\(g_fd, &ev, sizeof\(ev\)\) == sizeof\(ev\)\) \{', mice_logic, code)

with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
