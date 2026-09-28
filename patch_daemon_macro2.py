import re

with open("ring0-engine/ukw_daemon.cpp", "r") as f:
    code = f.read()

# Replace utf8_to_telex with utf8_to_keycodes
utf8_old = r"std::string utf8_to_telex\(const std::string& utf8\) \{[\s\S]*?return res;\n\}"
utf8_new = """
std::vector<int> utf8_to_keycodes(const std::string& utf8) {
    std::vector<int> res;
    for (size_t i = 0; i < utf8.length(); ) {
        unsigned char c = utf8[i];
        if (c < 0x80) {
            if (c >= 'A' && c <= 'Z') { res.push_back(-char_to_keycode(c + 32)); } // negative means shift
            else { res.push_back(char_to_keycode(c)); }
            i++; continue; 
        }
        
        if (i + 1 >= utf8.length()) break;
        unsigned char c2 = utf8[i+1];
        
        if (c == 0xC3) {
            if (c2 == 0xA1) { res.push_back(KEY_A); res.push_back(KEY_VN_SAC); i+=2; } // á
            else if (c2 == 0xA0) { res.push_back(KEY_A); res.push_back(KEY_VN_HUY); i+=2; } // à
            else if (c2 == 0xA3) { res.push_back(KEY_A); res.push_back(KEY_VN_NGA); i+=2; } // ã
            else if (c2 == 0xA2) { res.push_back(KEY_VN_AA); i+=2; } // â
            else if (c2 == 0xAA) { res.push_back(KEY_VN_EE); i+=2; } // ê
            else if (c2 == 0xA9) { res.push_back(KEY_E); res.push_back(KEY_VN_SAC); i+=2; } // é
            else if (c2 == 0xA8) { res.push_back(KEY_E); res.push_back(KEY_VN_HUY); i+=2; } // è
            else if (c2 == 0xAC) { res.push_back(KEY_E); res.push_back(KEY_VN_NGA); i+=2; } // ẽ (Wait, ẽ is 0xC3 0xAB)
            else if (c2 == 0xAB) { res.push_back(KEY_E); res.push_back(KEY_VN_NGA); i+=2; } // ẽ
            else if (c2 == 0xAD) { res.push_back(KEY_I); res.push_back(KEY_VN_SAC); i+=2; } // í
            else if (c2 == 0xAC) { res.push_back(KEY_I); res.push_back(KEY_VN_HUY); i+=2; } // ì
            else if (c2 == 0xB3) { res.push_back(KEY_O); res.push_back(KEY_VN_SAC); i+=2; } // ó
            else if (c2 == 0xB2) { res.push_back(KEY_O); res.push_back(KEY_VN_HUY); i+=2; } // ò
            else if (c2 == 0xB5) { res.push_back(KEY_O); res.push_back(KEY_VN_NGA); i+=2; } // õ
            else if (c2 == 0xB4) { res.push_back(KEY_VN_OO); i+=2; } // ô
            else if (c2 == 0xBA) { res.push_back(KEY_U); res.push_back(KEY_VN_SAC); i+=2; } // ú
            else if (c2 == 0xB9) { res.push_back(KEY_U); res.push_back(KEY_VN_HUY); i+=2; } // ù
            else if (c2 == 0xBD) { res.push_back(KEY_Y); res.push_back(KEY_VN_SAC); i+=2; } // ý
            else if (c2 == 0xBC) { res.push_back(KEY_Y); res.push_back(KEY_VN_HUY); i+=2; } // ỳ
            // Uppercase
            else if (c2 == 0x81) { res.push_back(-KEY_A); res.push_back(KEY_VN_SAC); i+=2; } // Á
            else if (c2 == 0x80) { res.push_back(-KEY_A); res.push_back(KEY_VN_HUY); i+=2; } // À
            else { i+=2; }
        } else if (c == 0xC4) {
            if (c2 == 0x91) { res.push_back(KEY_VN_DD); i+=2; } // đ
            else if (c2 == 0x90) { res.push_back(-KEY_VN_DD); i+=2; } // Đ
            else if (c2 == 0x83) { res.push_back(KEY_VN_AW); i+=2; } // ă
            else if (c2 == 0x82) { res.push_back(-KEY_VN_AW); i+=2; } // Ă
            else if (c2 == 0xA9) { res.push_back(KEY_I); res.push_back(KEY_VN_NGA); i+=2; } // ĩ
            else { i+=2; }
        } else if (c == 0xC5) {
            if (c2 == 0xA9) { res.push_back(KEY_U); res.push_back(KEY_VN_NGA); i+=2; } // ũ
            else { i+=2; }
        } else if (c == 0xC6) {
            if (c2 == 0xA1) { res.push_back(KEY_VN_OW); i+=2; } // ơ
            else if (c2 == 0xA0) { res.push_back(-KEY_VN_OW); i+=2; } // Ơ
            else if (c2 == 0xB0) { res.push_back(KEY_VN_UW); i+=2; } // ư
            else if (c2 == 0xAF) { res.push_back(-KEY_VN_UW); i+=2; } // Ư
            else { i+=2; }
        } else if (c == 0xE1) {
            if (i + 2 >= utf8.length()) break;
            unsigned char c3 = utf8[i+2];
            if (c2 == 0xBA) {
                if (c3 == 0xA1) { res.push_back(KEY_A); res.push_back(KEY_VN_NAN); i+=3; } // ạ
                else if (c3 == 0xA3) { res.push_back(KEY_A); res.push_back(KEY_VN_HOI); i+=3; } // ả
                else if (c3 == 0xA5) { res.push_back(KEY_VN_AA); res.push_back(KEY_VN_SAC); i+=3; } // ấ
                else if (c3 == 0xA7) { res.push_back(KEY_VN_AA); res.push_back(KEY_VN_HUY); i+=3; } // ầ
                else if (c3 == 0xA9) { res.push_back(KEY_VN_AA); res.push_back(KEY_VN_HOI); i+=3; } // ẩ
                else if (c3 == 0xAB) { res.push_back(KEY_VN_AA); res.push_back(KEY_VN_NGA); i+=3; } // ẫ
                else if (c3 == 0xAD) { res.push_back(KEY_VN_AA); res.push_back(KEY_VN_NAN); i+=3; } // ậ
                else if (c3 == 0xAF) { res.push_back(KEY_VN_AW); res.push_back(KEY_VN_SAC); i+=3; } // ắ
                else if (c3 == 0xB1) { res.push_back(KEY_VN_AW); res.push_back(KEY_VN_HUY); i+=3; } // ằ
                else if (c3 == 0xB3) { res.push_back(KEY_VN_AW); res.push_back(KEY_VN_HOI); i+=3; } // ẳ
                else if (c3 == 0xB5) { res.push_back(KEY_VN_AW); res.push_back(KEY_VN_NGA); i+=3; } // ẵ
                else if (c3 == 0xB7) { res.push_back(KEY_VN_AW); res.push_back(KEY_VN_NAN); i+=3; } // ặ
                else if (c3 == 0xB9) { res.push_back(KEY_E); res.push_back(KEY_VN_NAN); i+=3; } // ẹ
                else if (c3 == 0xBB) { res.push_back(KEY_E); res.push_back(KEY_VN_HOI); i+=3; } // ẻ
                else if (c3 == 0xBF) { res.push_back(KEY_VN_EE); res.push_back(KEY_VN_SAC); i+=3; } // ế
                else { i+=3; }
            }
            else if (c2 == 0xBB) {
                if (c3 == 0x81) { res.push_back(KEY_VN_EE); res.push_back(KEY_VN_HUY); i+=3; } // ề
                else if (c3 == 0x83) { res.push_back(KEY_VN_EE); res.push_back(KEY_VN_HOI); i+=3; } // ể
                else if (c3 == 0x85) { res.push_back(KEY_VN_EE); res.push_back(KEY_VN_NGA); i+=3; } // ễ
                else if (c3 == 0x87) { res.push_back(KEY_VN_EE); res.push_back(KEY_VN_NAN); i+=3; } // ệ
                else if (c3 == 0x89) { res.push_back(KEY_I); res.push_back(KEY_VN_HOI); i+=3; } // ỉ
                else if (c3 == 0x8B) { res.push_back(KEY_I); res.push_back(KEY_VN_NAN); i+=3; } // ị
                else if (c3 == 0x8D) { res.push_back(KEY_O); res.push_back(KEY_VN_NAN); i+=3; } // ọ
                else if (c3 == 0x8F) { res.push_back(KEY_O); res.push_back(KEY_VN_HOI); i+=3; } // ỏ
                else if (c3 == 0x91) { res.push_back(KEY_VN_OO); res.push_back(KEY_VN_SAC); i+=3; } // ố
                else if (c3 == 0x93) { res.push_back(KEY_VN_OO); res.push_back(KEY_VN_HUY); i+=3; } // ồ
                else if (c3 == 0x95) { res.push_back(KEY_VN_OO); res.push_back(KEY_VN_HOI); i+=3; } // ổ
                else if (c3 == 0x97) { res.push_back(KEY_VN_OO); res.push_back(KEY_VN_NGA); i+=3; } // ỗ
                else if (c3 == 0x99) { res.push_back(KEY_VN_OO); res.push_back(KEY_VN_NAN); i+=3; } // ộ
                else if (c3 == 0x9B) { res.push_back(KEY_VN_OW); res.push_back(KEY_VN_SAC); i+=3; } // ớ
                else if (c3 == 0x9D) { res.push_back(KEY_VN_OW); res.push_back(KEY_VN_HUY); i+=3; } // ờ
                else if (c3 == 0x9F) { res.push_back(KEY_VN_OW); res.push_back(KEY_VN_HOI); i+=3; } // ở
                else if (c3 == 0xA1) { res.push_back(KEY_VN_OW); res.push_back(KEY_VN_NGA); i+=3; } // ỡ
                else if (c3 == 0xA3) { res.push_back(KEY_VN_OW); res.push_back(KEY_VN_NAN); i+=3; } // ợ
                else if (c3 == 0xA5) { res.push_back(KEY_U); res.push_back(KEY_VN_NAN); i+=3; } // ụ
                else if (c3 == 0xA7) { res.push_back(KEY_U); res.push_back(KEY_VN_HOI); i+=3; } // ủ
                else if (c3 == 0xA9) { res.push_back(KEY_VN_UW); res.push_back(KEY_VN_SAC); i+=3; } // ứ
                else if (c3 == 0xAB) { res.push_back(KEY_VN_UW); res.push_back(KEY_VN_HUY); i+=3; } // ừ
                else if (c3 == 0xAD) { res.push_back(KEY_VN_UW); res.push_back(KEY_VN_HOI); i+=3; } // ử
                else if (c3 == 0xAF) { res.push_back(KEY_VN_UW); res.push_back(KEY_VN_NGA); i+=3; } // ữ
                else if (c3 == 0xB1) { res.push_back(KEY_VN_UW); res.push_back(KEY_VN_NAN); i+=3; } // ự
                else if (c3 == 0xB7) { res.push_back(KEY_Y); res.push_back(KEY_VN_HOI); i+=3; } // ỷ
                else if (c3 == 0xB9) { res.push_back(KEY_Y); res.push_back(KEY_VN_NGA); i+=3; } // ỹ
                else { i+=3; }
            } else { i+=3; }
        } else {
            i++;
        }
    }
    return res;
}
"""
code = re.sub(utf8_old, utf8_new, code)

# Update tap loop for macro
macro_tap_old = """                    std::string v = utf8_to_telex(m_map[k]);
                    for (char ch : v) {
                        if (ch >= 'A' && ch <= 'Z') tap_shift(g_fd, char_to_keycode(ch + 32));
                        else tap(g_fd, char_to_keycode(ch));
                    }"""
macro_tap_new = """                    std::vector<int> v = utf8_to_keycodes(m_map[k]);
                    for (int kc : v) {
                        if (kc < 0) tap_shift(g_fd, -kc);
                        else tap(g_fd, kc);
                    }"""
code = code.replace(macro_tap_old, macro_tap_new)

with open("ring0-engine/ukw_daemon.cpp", "w") as f:
    f.write(code)
