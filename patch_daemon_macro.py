import re

with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Add utf8_to_telex
utf8_func = """
std::string utf8_to_telex(const std::string& utf8) {
    std::string res = "";
    for (size_t i = 0; i < utf8.length(); ) {
        unsigned char c = utf8[i];
        if (c < 0x80) { res += c; i++; continue; }
        
        if (i + 1 >= utf8.length()) break;
        unsigned char c2 = utf8[i+1];
        
        // Very basic mapping for common Vietnamese chars to Telex keystrokes
        if (c == 0xC3) {
            if (c2 == 0xA1) { res += "as"; i+=2; } // á
            else if (c2 == 0xA0) { res += "af"; i+=2; } // à
            else if (c2 == 0xA3) { res += "ax"; i+=2; } // ã
            else if (c2 == 0xA2) { res += "aa"; i+=2; } // â
            else if (c2 == 0xAA) { res += "ee"; i+=2; } // ê
            else if (c2 == 0xA9) { res += "es"; i+=2; } // é
            else if (c2 == 0xA8) { res += "ef"; i+=2; } // è
            else if (c2 == 0xAC) { res += "ex"; i+=2; } // ẽ
            else if (c2 == 0xAD) { res += "is"; i+=2; } // í
            else if (c2 == 0xAC) { res += "if"; i+=2; } // ì
            else if (c2 == 0xB3) { res += "os"; i+=2; } // ó
            else if (c2 == 0xB2) { res += "of"; i+=2; } // ò
            else if (c2 == 0xB5) { res += "ox"; i+=2; } // õ
            else if (c2 == 0xB4) { res += "oo"; i+=2; } // ô
            else if (c2 == 0xBA) { res += "us"; i+=2; } // ú
            else if (c2 == 0xB9) { res += "uf"; i+=2; } // ù
            else if (c2 == 0xBD) { res += "ys"; i+=2; } // ý
            else if (c2 == 0xBC) { res += "yf"; i+=2; } // ỳ
            // Uppercase
            else if (c2 == 0x81) { res += "As"; i+=2; } // Á
            else if (c2 == 0x80) { res += "Af"; i+=2; } // À
            else { i+=2; } // fallback
        } else if (c == 0xC4) {
            if (c2 == 0x91) { res += "dd"; i+=2; } // đ
            else if (c2 == 0x90) { res += "DD"; i+=2; } // Đ
            else if (c2 == 0x83) { res += "aw"; i+=2; } // ă
            else if (c2 == 0x82) { res += "Aw"; i+=2; } // Ă
            else if (c2 == 0xA9) { res += "ix"; i+=2; } // ĩ
            else { i+=2; }
        } else if (c == 0xC5) {
            if (c2 == 0xA9) { res += "ux"; i+=2; } // ũ
            else { i+=2; }
        } else if (c == 0xC6) {
            if (c2 == 0xA1) { res += "ow"; i+=2; } // ơ
            else if (c2 == 0xA0) { res += "Ow"; i+=2; } // Ơ
            else if (c2 == 0xB0) { res += "uw"; i+=2; } // ư
            else if (c2 == 0xAF) { res += "Uw"; i+=2; } // Ư
            else { i+=2; }
        } else if (c == 0xE1) {
            if (i + 2 >= utf8.length()) break;
            unsigned char c3 = utf8[i+2];
            // 3-byte chars (ạ, ả, ậ, ệ, ộ, ợ, ự...)
            if (c2 == 0xBA) {
                if (c3 == 0xA1) { res += "aj"; i+=3; } // ạ
                else if (c3 == 0xA3) { res += "ar"; i+=3; } // ả
                else if (c3 == 0xA5) { res += "aas"; i+=3; } // ấ
                else if (c3 == 0xA7) { res += "aaf"; i+=3; } // ầ
                else if (c3 == 0xA9) { res += "aar"; i+=3; } // ẩ
                else if (c3 == 0xAB) { res += "aax"; i+=3; } // ẫ
                else if (c3 == 0xAD) { res += "aaj"; i+=3; } // ậ
                else if (c3 == 0xAF) { res += "aws"; i+=3; } // ắ
                else if (c3 == 0xB1) { res += "awf"; i+=3; } // ằ
                else if (c3 == 0xB3) { res += "awr"; i+=3; } // ẳ
                else if (c3 == 0xB5) { res += "awx"; i+=3; } // ẵ
                else if (c3 == 0xB7) { res += "awj"; i+=3; } // ặ
                else if (c3 == 0xB9) { res += "ej"; i+=3; } // ẹ
                else if (c3 == 0xBB) { res += "er"; i+=3; } // ẻ
                else if (c3 == 0xBF) { res += "ees"; i+=3; } // ế
                else { i+=3; }
            }
            else if (c2 == 0xBB) {
                if (c3 == 0x81) { res += "eef"; i+=3; } // ề
                else if (c3 == 0x83) { res += "eer"; i+=3; } // ể
                else if (c3 == 0x85) { res += "eex"; i+=3; } // ễ
                else if (c3 == 0x87) { res += "eej"; i+=3; } // ệ
                else if (c3 == 0x89) { res += "ir"; i+=3; } // ỉ
                else if (c3 == 0x8B) { res += "ij"; i+=3; } // ị
                else if (c3 == 0x8D) { res += "oj"; i+=3; } // ọ
                else if (c3 == 0x8F) { res += "or"; i+=3; } // ỏ
                else if (c3 == 0x91) { res += "oos"; i+=3; } // ố
                else if (c3 == 0x93) { res += "oof"; i+=3; } // ồ
                else if (c3 == 0x95) { res += "oor"; i+=3; } // ổ
                else if (c3 == 0x97) { res += "oox"; i+=3; } // ỗ
                else if (c3 == 0x99) { res += "ooj"; i+=3; } // ộ
                else if (c3 == 0x9B) { res += "ows"; i+=3; } // ớ
                else if (c3 == 0x9D) { res += "owf"; i+=3; } // ờ
                else if (c3 == 0x9F) { res += "owr"; i+=3; } // ở
                else if (c3 == 0xA1) { res += "owx"; i+=3; } // ỡ
                else if (c3 == 0xA3) { res += "owj"; i+=3; } // ợ
                else if (c3 == 0xA5) { res += "uj"; i+=3; } // ụ
                else if (c3 == 0xA7) { res += "ur"; i+=3; } // ủ
                else if (c3 == 0xA9) { res += "uws"; i+=3; } // ứ
                else if (c3 == 0xAB) { res += "uwf"; i+=3; } // ừ
                else if (c3 == 0xAD) { res += "uwr"; i+=3; } // ử
                else if (c3 == 0xAF) { res += "uwx"; i+=3; } // ữ
                else if (c3 == 0xB1) { res += "uwj"; i+=3; } // ự
                else { i+=3; }
            } else { i+=3; }
        } else {
            // Ignore other unicode
            i++;
        }
    }
    return res;
}
"""
code = code.replace("int char_to_keycode(char c) {", utf8_func + "\nint char_to_keycode(char c) {")

# Update char_to_keycode to handle spaces and numbers
char_key_old = """    if (c == 'n') return KEY_N; if (c == 'm') return KEY_M;
    return 0;
}"""
char_key_new = """    if (c == 'n') return KEY_N; if (c == 'm') return KEY_M;
    if (c == ' ') return KEY_SPACE;
    if (c >= '0' && c <= '9') return KEY_1 + (c - '1'); // rough map
    return 0;
}"""
code = code.replace(char_key_old, char_key_new)

# Update the tap logic for macro
macro_tap_old = """                    std::string v = m_map[k];
                    for (char ch : v) {
                        if (ch >= 'A' && ch <= 'Z') tap_shift(g_fd, char_to_keycode(ch + 32));
                        else tap(g_fd, char_to_keycode(ch));
                    }"""
macro_tap_new = """                    std::string v = utf8_to_telex(m_map[k]);
                    for (char ch : v) {
                        if (ch >= 'A' && ch <= 'Z') tap_shift(g_fd, char_to_keycode(ch + 32));
                        else tap(g_fd, char_to_keycode(ch));
                    }"""
code = code.replace(macro_tap_old, macro_tap_new)

with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
