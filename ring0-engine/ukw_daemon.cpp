#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <fstream>
#include <sstream>

struct ukw_event {
    unsigned short type;
    unsigned short code;
    int value;
};

#define KEY_VN_AW   86
#define KEY_VN_AA   89
#define KEY_VN_EE   90
#define KEY_VN_OO   92
#define KEY_VN_DD   93
#define KEY_VN_UW   94
#define KEY_VN_OW   117
#define KEY_VN_SAC  122
#define KEY_VN_HUY  123
#define KEY_VN_HOI  124
#define KEY_VN_NGA  121
#define KEY_VN_NAN  91

static int g_fd = -1;
static bool viet_mode = true;
int switch_cfg = 0;
static bool macro_enabled = false;
static std::map<std::string, std::string> macros;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


static void update_status() {
    int fd = open("/tmp/ukw_status", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd >= 0) {
        write(fd, viet_mode ? "VI" : "EN", 2);
        close(fd);
    }
}

bool is_alpha(int c) {
    return (c >= KEY_Q && c <= KEY_P) ||
           (c >= KEY_A && c <= KEY_L) ||
           (c >= KEY_Z && c <= KEY_M);
}
bool is_vn_special(int k) {
    return k == KEY_VN_AW || k == KEY_VN_AA || k == KEY_VN_EE || k == KEY_VN_OO ||
           k == KEY_VN_OW || k == KEY_VN_UW || k == KEY_VN_DD;
}
bool is_vowel(int k) {
    return k == KEY_A || k == KEY_E || k == KEY_I || k == KEY_O || k == KEY_U || k == KEY_Y ||
           k == KEY_VN_AW || k == KEY_VN_AA || k == KEY_VN_EE || k == KEY_VN_OO ||
           k == KEY_VN_OW || k == KEY_VN_UW;
}

char keycode_to_char(int code) {
    if (code >= KEY_Q && code <= KEY_P) {
        const char map[] = "qwertyuiop";
        return map[code - KEY_Q];
    }
    if (code >= KEY_A && code <= KEY_L) {
        const char map[] = "asdfghjkl";
        return map[code - KEY_A];
    }
    if (code >= KEY_Z && code <= KEY_M) {
        const char map[] = "zxcvbnm";
        return map[code - KEY_Z];
    }
    return 0;
}



int char_to_keycode(char c);
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


int char_to_keycode(char c) {
    if (c == 'q') return KEY_Q; if (c == 'w') return KEY_W; if (c == 'e') return KEY_E;
    if (c == 'r') return KEY_R; if (c == 't') return KEY_T; if (c == 'y') return KEY_Y;
    if (c == 'u') return KEY_U; if (c == 'i') return KEY_I; if (c == 'o') return KEY_O;
    if (c == 'p') return KEY_P; if (c == 'a') return KEY_A; if (c == 's') return KEY_S;
    if (c == 'd') return KEY_D; if (c == 'f') return KEY_F; if (c == 'g') return KEY_G;
    if (c == 'h') return KEY_H; if (c == 'j') return KEY_J; if (c == 'k') return KEY_K;
    if (c == 'l') return KEY_L; if (c == 'z') return KEY_Z; if (c == 'x') return KEY_X;
    if (c == 'c') return KEY_C; if (c == 'v') return KEY_V; if (c == 'b') return KEY_B;
    if (c == 'n') return KEY_N; if (c == 'm') return KEY_M;
    if (c == ' ') return KEY_SPACE;
    if (c >= '0' && c <= '9') return KEY_1 + (c - '1'); // rough map
    return 0;
}

bool looks_vietnamese(const std::vector<int>& word) {
    for (int k : word) if (is_vn_special(k)) return true;
    bool in_vowel = false;
    int vowel_groups = 0;
    for (int k : word) {
        bool v = is_vowel(k);
        if (v && !in_vowel) { vowel_groups++; in_vowel = true; }
        else if (!v) in_vowel = false;
    }
    return vowel_groups <= 1;
}

int find_vowel_pos(const std::vector<int>& buf) {
    int len = buf.size();
    for (int i = len - 1; i >= 0; i--) {
        int c = buf[i];
        if (c == KEY_VN_OW || c == KEY_VN_EE || c == KEY_VN_OO ||
            c == KEY_VN_AW || c == KEY_VN_AA || c == KEY_VN_UW) return i;
    }
    int last_vowel_idx = -1;
    for (int i = len - 1; i >= 0; i--) {
        if (is_vowel(buf[i])) { last_vowel_idx = i; break; }
    }
    if (last_vowel_idx == -1) return -1;
    int first_vowel_idx = last_vowel_idx;
    while (first_vowel_idx > 0 && is_vowel(buf[first_vowel_idx - 1])) first_vowel_idx--;
    int count = last_vowel_idx - first_vowel_idx + 1;
    if (count == 1) return first_vowel_idx;
    if (count >= 2) {
        if (first_vowel_idx > 0 && buf[first_vowel_idx-1] == KEY_G && buf[first_vowel_idx] == KEY_I) return first_vowel_idx + 1;
        if (first_vowel_idx > 0 && buf[first_vowel_idx-1] == KEY_Q && buf[first_vowel_idx] == KEY_U) return first_vowel_idx + 1;
        if ((buf[first_vowel_idx] == KEY_O && (buf[first_vowel_idx+1] == KEY_A || buf[first_vowel_idx+1] == KEY_E)) ||
            (buf[first_vowel_idx] == KEY_U && buf[first_vowel_idx+1] == KEY_Y)) return first_vowel_idx + 1;
        if (len > last_vowel_idx + 1) return first_vowel_idx + 1;
        return first_vowel_idx;
    }
    return last_vowel_idx;
}

void send_ev(int fd, int type, int code, int value) {
    ukw_event e = {(unsigned short)type, (unsigned short)code, value};
    write(fd, &e, sizeof(e));
}
void syn(int fd) { send_ev(fd, EV_SYN, SYN_REPORT, 0); }
void tap(int fd, int code) {
    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(1500);
    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(1500); usleep(5000);
}
void tap_shift(int fd, int code) {
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 1); syn(fd); usleep(5000);
    send_ev(fd, EV_KEY, code, 1); syn(fd); usleep(5000);
    send_ev(fd, EV_KEY, code, 0); syn(fd); usleep(5000);
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 0); syn(fd); usleep(5000);
}
void release_alpha(int fd, bool held[]) {
    bool any = false;
    for (int k = 0; k < KEY_MAX; k++) {
        if (held[k] && is_alpha(k)) {
            send_ev(fd, EV_KEY, k, 0);
            held[k] = false; any = true;
        }
    }
    if (any) syn(fd);
}
void handle_exit(int sig) { if (g_fd >= 0) close(g_fd); exit(0); }

void* cmd_listener(void* arg) {
    const char* fifo_path = "/tmp/ukw_cmd";
    mkfifo(fifo_path, 0666);
    chmod(fifo_path, 0666); // Ensure anyone can write to it
    
    while (true) {
        int fd = open(fifo_path, O_RDONLY);
        if (fd < 0) { continue; }
        char buf[4096];
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = 0;
            std::string cmd(buf);
            pthread_mutex_lock(&lock);
            if (cmd.find("MODE:VI") != std::string::npos) {
                viet_mode = true; update_status();
            } else if (cmd.find("MODE:EN") != std::string::npos) {
                viet_mode = false; update_status();
            }
            if (cmd.find("SWITCH:0") != std::string::npos) switch_cfg = 0;
            else if (cmd.find("SWITCH:1") != std::string::npos) switch_cfg = 1;
            else if (cmd.find("SWITCH:2") != std::string::npos) switch_cfg = 2;
 if (cmd.find("MACRO:") != std::string::npos) {
                // Format: MACRO:enabled:key1=val1;key2=val2;
                macro_enabled = (cmd.substr(6, 1) == "1");
                macros.clear();
                size_t pos = 8;
                while (pos < cmd.length()) {
                    size_t eq = cmd.find('=', pos);
                    if (eq == std::string::npos) break;
                    size_t semi = cmd.find(';', eq);
                    if (semi == std::string::npos) break;
                    std::string k = cmd.substr(pos, eq - pos);
                    std::string v = cmd.substr(eq + 1, semi - eq - 1);
                    if (!k.empty() && !v.empty()) macros[k] = v;
                    pos = semi + 1;
                }
            }
            pthread_mutex_unlock(&lock);
        }
        close(fd);
    }
    return NULL;
}

int main() {
    signal(SIGINT, handle_exit); signal(SIGTERM, handle_exit);
    g_fd = open("/dev/ukw", O_RDWR);
    if (g_fd < 0) { std::cerr << "Cannot open /dev/ukw\n"; return 1; }
    
    pthread_t tid;
    pthread_create(&tid, NULL, cmd_listener, NULL);
    
    update_status();
    std::vector<int> word;
    int cur_tone = 0, tone_pos = -1;
    bool held[KEY_MAX] = {};
    bool other_pressed = false;


    ukw_event ev;
    while (read(g_fd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type != EV_KEY) { write(g_fd, &ev, sizeof(ev)); continue; }

        int code = ev.code, value = ev.value;

        if (value == 0) {
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
        }

        if (code == KEY_RIGHTALT) {
            if (value == 1) {
                pthread_mutex_lock(&lock);
                viet_mode = !viet_mode;
                pthread_mutex_unlock(&lock);
                word.clear(); cur_tone = 0; tone_pos = -1;
            }
            continue;
        }

        if (value == 2) { send_ev(g_fd, EV_KEY, code, 2); syn(g_fd); continue; }
        
        pthread_mutex_lock(&lock);
        bool mode = viet_mode;
        bool m_en = macro_enabled;
        auto m_map = macros;
        pthread_mutex_unlock(&lock);
        
        if (!mode) { send_ev(g_fd, EV_KEY, code, value); syn(g_fd); continue; }

        if (!is_alpha(code) && code != KEY_LEFTSHIFT && code != KEY_RIGHTSHIFT && code != KEY_CAPSLOCK) {
            // Trigger macro on space or punctuation
            if (value == 1 && m_en && !word.empty() && (code == KEY_SPACE || code == KEY_ENTER || code == KEY_COMMA || code == KEY_DOT || code == KEY_SLASH || code == KEY_SEMICOLON)) {
                std::string k = "";
                for (int c : word) {
                    char ch = keycode_to_char(c);
                    if (ch) k += ch;
                }
                if (m_map.find(k) != m_map.end()) {
                    // Do not release trigger to OS because it was never sent as down!
                    release_alpha(g_fd, held);
                    int bs_count = word.size();
                    if (cur_tone != 0) bs_count++;
                    for (int i = 0; i < bs_count; i++) tap(g_fd, KEY_BACKSPACE);
                    
                    std::vector<int> v = utf8_to_keycodes(m_map[k]);
                    for (int kc : v) {
                        if (kc < 0) tap_shift(g_fd, -kc);
                        else tap(g_fd, kc);
                    }
                    tap(g_fd, code); // retap trigger
                    word.clear(); cur_tone = 0; tone_pos = -1;
                    continue;
                }
            }
            
            if (value == 1) { word.clear(); cur_tone = 0; tone_pos = -1; }
            send_ev(g_fd, EV_KEY, code, value); syn(g_fd);
            continue;
        }

        if (value == 0) { send_ev(g_fd, EV_KEY, code, 0); syn(g_fd); continue; }
        send_ev(g_fd, EV_KEY, code, 1); syn(g_fd);

        // -- TELEX LOGIC --
        int target = 0; bool cancel_dau = false;
        int target_pos = -1, target2 = 0, target_pos2 = -1;
        
        if (looks_vietnamese(word)) {
            if (code == KEY_W) {
                for (int i = word.size() - 1; i >= 0; i--) {
                    if (word[i] == KEY_O) {
                        target = KEY_VN_OW; target_pos = i;
                        if (i > 0 && word[i-1] == KEY_U) { target2 = KEY_VN_UW; target_pos2 = i - 1; }
                        break;
                    } else if (word[i] == KEY_U) { target = KEY_VN_UW; target_pos = i; break;
                    } else if (word[i] == KEY_A) { target = KEY_VN_AW; target_pos = i; break;
                    } else if (word[i] == KEY_VN_OW) {
                        target = KEY_O; target_pos = i; cancel_dau = true;
                        if (i > 0 && word[i-1] == KEY_VN_UW) { target2 = KEY_U; target_pos2 = i - 1; }
                        break;
                    } else if (word[i] == KEY_VN_UW) { target = KEY_U; target_pos = i; cancel_dau = true; break;
                    } else if (word[i] == KEY_VN_AW) { target = KEY_A; target_pos = i; cancel_dau = true; break; }
                }
            } else if (code == KEY_A) {
                for (int i = word.size() - 1; i >= 0; i--) {
                    if (word[i] == KEY_A) { target = KEY_VN_AA; target_pos = i; break; }
                    else if (word[i] == KEY_VN_AA) { target = KEY_A; target_pos = i; cancel_dau = true; break; }
                }
            } else if (code == KEY_E) {
                for (int i = word.size() - 1; i >= 0; i--) {
                    if (word[i] == KEY_E) { target = KEY_VN_EE; target_pos = i; break; }
                    else if (word[i] == KEY_VN_EE) { target = KEY_E; target_pos = i; cancel_dau = true; break; }
                }
            } else if (code == KEY_O) {
                for (int i = word.size() - 1; i >= 0; i--) {
                    if (word[i] == KEY_O) { target = KEY_VN_OO; target_pos = i; break; }
                    else if (word[i] == KEY_VN_OO) { target = KEY_O; target_pos = i; cancel_dau = true; break; }
                }
            } else if (code == KEY_D) {
                for (int i = word.size() - 1; i >= 0; i--) {
                    if (word[i] == KEY_D) { target = KEY_VN_DD; target_pos = i; break; }
                    else if (word[i] == KEY_VN_DD) { target = KEY_D; target_pos = i; cancel_dau = true; break; }
                }
            }
        }

        if (target != 0) {
            send_ev(g_fd, EV_KEY, code, 0); syn(g_fd);
            release_alpha(g_fd, held);
            
            std::vector<int> new_word = word;
            new_word[target_pos] = target;
            if (target_pos2 != -1) new_word[target_pos2] = target2;
            if (cancel_dau) new_word.push_back(code);
            
            int new_tone_pos = (cur_tone != 0) ? find_vowel_pos(new_word) : -1;
            int start_pos = target_pos2 != -1 ? target_pos2 : target_pos;
            if (tone_pos != -1 && tone_pos < start_pos) start_pos = tone_pos;
            if (new_tone_pos != -1 && new_tone_pos < start_pos) start_pos = new_tone_pos;
            
            int bs_count = word.size() - start_pos + 1; // +1 pass through
            if (cur_tone != 0 && tone_pos != -1 && tone_pos >= start_pos) bs_count += 1;
            
            for (int i = 0; i < bs_count; i++) tap(g_fd, KEY_BACKSPACE);
            
            word = new_word;
            tone_pos = new_tone_pos;
            
            for (int i = start_pos; i < word.size(); i++) {
                tap(g_fd, word[i]);
                if (cur_tone != 0 && i == tone_pos) tap(g_fd, cur_tone);
            }
            continue;
        }

        int tone = 0;
        if      (code == KEY_S) tone = KEY_VN_SAC;
        else if (code == KEY_F) tone = KEY_VN_HUY;
        else if (code == KEY_R) tone = KEY_VN_HOI;
        else if (code == KEY_X) tone = KEY_VN_NGA;
        else if (code == KEY_J) tone = KEY_VN_NAN;

        if (tone != 0 && looks_vietnamese(word) && !word.empty()) {
            int vpos = find_vowel_pos(word);
            if (vpos >= 0) {
                send_ev(g_fd, EV_KEY, code, 0); syn(g_fd);
                release_alpha(g_fd, held);
                
                std::vector<int> new_word = word;
                int new_cur_tone = cur_tone;
                int new_tone_pos = tone_pos;
                
                if (tone == cur_tone) {
                    new_cur_tone = 0; new_tone_pos = -1;
                    new_word.push_back(code);
                } else {
                    new_cur_tone = tone; new_tone_pos = vpos;
                }
                
                int start_pos = word.size();
                if (tone_pos != -1 && tone_pos < start_pos) start_pos = tone_pos;
                if (new_tone_pos != -1 && new_tone_pos < start_pos) start_pos = new_tone_pos;
                
                int bs_count = word.size() - start_pos + 1;
                if (cur_tone != 0 && tone_pos != -1 && tone_pos >= start_pos) bs_count += 1;
                
                for (int i = 0; i < bs_count; i++) tap(g_fd, KEY_BACKSPACE);
                
                word = new_word; cur_tone = new_cur_tone; tone_pos = new_tone_pos;
                
                for (int i = start_pos; i < word.size(); i++) {
                    tap(g_fd, word[i]);
                    if (cur_tone != 0 && i == tone_pos) tap(g_fd, cur_tone);
                }
                continue;
            }
        }
        if (is_alpha(code)) word.push_back(code);
    }
    close(g_fd);
    return 0;
}
