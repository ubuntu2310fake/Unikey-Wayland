#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <unistd.h>
#include "../../wayland-client/src/libbamboo.h"

Display* g_display = nullptr;
bool g_viet_mode = true;
std::string g_composed_word = "";

int error_handler(Display* d, XErrorEvent* e) {
    // Bỏ qua lỗi BadAccess nếu có ứng dụng khác đã grab phím này
    return 0;
}

void init_x11() {
    g_display = XOpenDisplay(NULL);
    if (!g_display) {
        std::cerr << "Failed to open X display" << std::endl;
        exit(1);
    }
    XSetErrorHandler(error_handler);
}

void send_backspaces(int count) {
    if (!g_display) return;
    KeyCode bk = XKeysymToKeycode(g_display, XK_BackSpace);
    
    usleep(5000); // Wait 5ms for Chrome to process FocusOut/FocusIn storm from XUngrabKeyboard
    
    // Fake release first in case the physical key is still held down
    XTestFakeKeyEvent(g_display, bk, False, CurrentTime);
    
    for (int i = 0; i < count; i++) {
        XTestFakeKeyEvent(g_display, bk, True, CurrentTime);
        XTestFakeKeyEvent(g_display, bk, False, CurrentTime);
    }
    XSync(g_display, False);
}

void send_unicode(uint32_t unicode_char) {
    if (!g_display) return;
    
    KeySym ks;
    if (unicode_char < 0x0100) ks = unicode_char;
    else ks = unicode_char | 0x01000000;
    
    KeyCode existing_kc = XKeysymToKeycode(g_display, ks);
    if (existing_kc != 0) {
        usleep(5000); // Wait 5ms for Chrome to process FocusOut/FocusIn storm
        
        // Fake release first in case the physical key is still held down
        XTestFakeKeyEvent(g_display, existing_kc, False, CurrentTime);
        
        XTestFakeKeyEvent(g_display, existing_kc, True, CurrentTime);
        XTestFakeKeyEvent(g_display, existing_kc, False, CurrentTime);
        XSync(g_display, False);
        return;
    }
    
    int min_kc, max_kc;
    XDisplayKeycodes(g_display, &min_kc, &max_kc);
    KeyCode kc = max_kc; 
    
    KeySym new_map[2] = {ks, ks};
    XChangeKeyboardMapping(g_display, kc, 2, new_map, 1);
    XSync(g_display, False);
    
    usleep(5000);
    
    XTestFakeKeyEvent(g_display, kc, True, CurrentTime);
    XTestFakeKeyEvent(g_display, kc, False, CurrentTime);
    XSync(g_display, False);
}

void send_unicode_string(const std::string& utf8_str) {
    int i = 0;
    while (i < utf8_str.length()) {
        uint32_t codepoint = 0;
        unsigned char c = utf8_str[i];
        if (c < 0x80) {
            codepoint = c;
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            codepoint = ((c & 0x1F) << 6) | (utf8_str[i+1] & 0x3F);
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            codepoint = ((c & 0x0F) << 12) | ((utf8_str[i+1] & 0x3F) << 6) | (utf8_str[i+2] & 0x3F);
            i += 3;
        } else {
            i++; // Fallback to prevent infinite loop
        }
        if (codepoint != 0) {
            send_unicode(codepoint);
        }
    }
}

void grab_keys() {
    Window root = DefaultRootWindow(g_display);
    
    // Chỉ bắt các phím gõ chữ, bỏ qua phím tắt hệ thống (Alt, Super, Ctrl)
    KeySym keys_to_grab[] = {
        XK_a, XK_b, XK_c, XK_d, XK_e, XK_f, XK_g, XK_h, XK_i, XK_j, XK_k, XK_l, XK_m,
        XK_n, XK_o, XK_p, XK_q, XK_r, XK_s, XK_t, XK_u, XK_v, XK_w, XK_x, XK_y, XK_z,
        XK_A, XK_B, XK_C, XK_D, XK_E, XK_F, XK_G, XK_H, XK_I, XK_J, XK_K, XK_L, XK_M,
        XK_N, XK_O, XK_P, XK_Q, XK_R, XK_S, XK_T, XK_U, XK_V, XK_W, XK_X, XK_Y, XK_Z,
        XK_0, XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7, XK_8, XK_9,
        XK_minus, XK_equal, XK_bracketleft, XK_bracketright, XK_backslash,
        XK_semicolon, XK_apostrophe, XK_comma, XK_period, XK_slash, XK_grave,
        XK_space, XK_BackSpace, XK_Return,
        XK_Control_L, XK_Control_R, XK_Shift_L, XK_Shift_R // Để xử lý g_ctrl_down và g_shift_down
    };
    
    unsigned int modifiers[] = {
        0,
        ShiftMask,
        LockMask,
        ShiftMask | LockMask
    };
    
    for (size_t i = 0; i < sizeof(keys_to_grab)/sizeof(KeySym); i++) {
        KeyCode kc = XKeysymToKeycode(g_display, keys_to_grab[i]);
        if (kc == 0) continue;
        for (size_t j = 0; j < 4; j++) {
            XGrabKey(g_display, kc, modifiers[j], root, True, GrabModeAsync, GrabModeSync);
        }
    }
    XSync(g_display, False);
}

int main(int argc, char** argv) {
    Bamboo_Init();
    init_x11();
    grab_keys();
    
    std::cout << "Grabbed X11 Keyboard (Passive Mode). Unikey X11 is running!" << std::endl;
    
    std::set<KeyCode> eaten_keys;
    bool g_ctrl_down = false;
    
    XEvent ev;
    while (1) {
        XNextEvent(g_display, &ev);
        
        if (ev.type == KeyPress || ev.type == KeyRelease) {
            KeyCode kc = ev.xkey.keycode;
            bool is_press = (ev.type == KeyPress);
            
            if (!is_press && eaten_keys.count(kc)) {
                eaten_keys.erase(kc);
                // Swallow this release event
                XAllowEvents(g_display, AsyncKeyboard, ev.xkey.time);
                XFlush(g_display);
                continue;
            }
            
            KeySym ks = XLookupKeysym(&ev.xkey, 0);
            
            if (ks == XK_Control_L || ks == XK_Control_R) {
                g_ctrl_down = is_press;
            }
            
            // Replay Ctrl/Shift events immediately so other apps receive them normally
            if (ks == XK_Control_L || ks == XK_Control_R || ks == XK_Shift_L || ks == XK_Shift_R) {
                XAllowEvents(g_display, ReplayKeyboard, ev.xkey.time);
                XFlush(g_display);
                continue;
            }
            
            if (!g_viet_mode || g_ctrl_down) {
                // Pass through
                XAllowEvents(g_display, ReplayKeyboard, ev.xkey.time);
                XFlush(g_display);
                continue;
            }
            
            // Dịch phím thành ASCII
            char buf[16] = {0};
            KeySym translated_ks;
            int len = XLookupString(&ev.xkey, buf, sizeof(buf)-1, &translated_ks, NULL);
            uint32_t c = 0;
            if (len == 1 && buf[0] >= 32 && buf[0] < 127) {
                c = buf[0];
            } else if (ks == XK_BackSpace) {
                c = '\b';
            } else if (ks == XK_space) {
                c = ' ';
            } else if (ks == XK_Return) {
                c = '\n';
            }
            
            bool eaten = false;
            if (is_press && c != 0) {
                std::string old_composed = g_composed_word;
                
                if (c == '\b') {
                    if (!g_composed_word.empty()) {
                        Bamboo_RemoveLastChar();
                        eaten = true;
                    }
                } else if (!Bamboo_CanProcessKey(c)) {
                    g_composed_word = "";
                    Bamboo_Reset();
                } else {
                    Bamboo_ProcessKey(c);
                    eaten = true;
                }
                
                if (eaten) {
                    eaten_keys.insert(kc);
                    
                    char* preedit_ptr = Bamboo_GetPreeditString ? Bamboo_GetPreeditString() : nullptr;
                    std::string new_composed = preedit_ptr ? preedit_ptr : "";
                    
                    int common_bytes = 0;
                    size_t i = 0, j = 0;
                    while (i < old_composed.length() && j < new_composed.length()) {
                        int len1 = 1, len2 = 1;
                        while (i + len1 < old_composed.length() && (old_composed[i + len1] & 0xC0) == 0x80) len1++;
                        while (j + len2 < new_composed.length() && (new_composed[j + len2] & 0xC0) == 0x80) len2++;
                        if (len1 == len2 && old_composed.substr(i, len1) == new_composed.substr(j, len2)) {
                            common_bytes += len1;
                            i += len1;
                            j += len2;
                        } else {
                            break;
                        }
                    }
                    
                    int byte_backs = old_composed.length() - common_bytes;
                    int chars_to_delete = 0;
                    if (byte_backs > 0) {
                        size_t idx = common_bytes;
                        while (idx < old_composed.length()) {
                            chars_to_delete++;
                            idx++;
                            while (idx < old_composed.length() && (old_composed[idx] & 0xC0) == 0x80) idx++;
                        }
                    }
                    
                    std::string suffix = new_composed.substr(common_bytes);
                    
                    // SWALLOW the physical key event FIRST
                    XAllowEvents(g_display, AsyncKeyboard, ev.xkey.time);
                    XUngrabKeyboard(g_display, CurrentTime); // RELEASE active grab
                    XFlush(g_display);
                    
                    // Then inject the modifications
                    if (chars_to_delete > 0) send_backspaces(chars_to_delete);
                    if (!suffix.empty()) send_unicode_string(suffix);
                    
                    g_composed_word = new_composed;
                    continue; 
                }
            }
            
            // Nếu không bị nuốt, pass qua cho X server xử lý
            XAllowEvents(g_display, ReplayKeyboard, ev.xkey.time);
            XFlush(g_display);
            
        } else if (ev.type == MappingNotify) {
            XRefreshKeyboardMapping(&ev.xmapping);
            // Regrab keys since mapping changed
            XUngrabKey(g_display, AnyKey, AnyModifier, DefaultRootWindow(g_display));
            grab_keys();
        }
    }
    
    return 0;
}
