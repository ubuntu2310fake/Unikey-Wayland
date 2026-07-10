#include <wayland-client.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>

static void log_to_file(const std::string& msg) {
    std::ofstream f("/tmp/uk_debug.log", std::ios::app);
    if (f.is_open()) {
        f << msg << std::endl;
    }
}

#include <QApplication>
#include <QSocketNotifier>
#include "mainwindow.h"
#include "trayicon.h"

#include "input-method-unstable-v1-client-protocol.h"
// No ukengine_wrapper needed
#include "windowtracker.h"
#include "libbamboo.h"

struct WaylandState {
    wl_display* display;
    wl_registry* registry;
    wl_seat* seat;
    zwp_input_method_v1* input_method;
    zwp_input_method_context_v1* context;
    wl_keyboard* keyboard;
    
    bool viet_mode = true;
    bool active;
    uint32_t latest_serial;
    std::string composed_word = "";
    std::vector<char> raw_keys_normal;
    uint32_t content_purpose = 0;
    
    std::string surrounding_text = "";
    uint32_t surrounding_cursor = 0;
    uint32_t surrounding_anchor = 0;
    bool has_surrounding_text = false;
};

static void pop_utf8_char(std::string& str) {
    if (str.empty()) return;
    while (!str.empty()) {
        unsigned char c = str.back();
        str.pop_back();
        if ((c & 0xC0) != 0x80) break;
    }
}

static int utf8_length(const std::string& str) {
    int len = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        if ((str[i] & 0xC0) != 0x80) len++;
    }
    return len;
}

static int utf8_common_prefix(const std::string& s1, const std::string& s2) {
    int common = 0;
    size_t i = 0, j = 0;
    while (i < s1.length() && j < s2.length()) {
        int len1 = 1, len2 = 1;
        while (i + len1 < s1.length() && (s1[i + len1] & 0xC0) == 0x80) len1++;
        while (j + len2 < s2.length() && (s2[j + len2] & 0xC0) == 0x80) len2++;
        
        if (len1 == len2 && s1.substr(i, len1) == s2.substr(j, len2)) {
            common++;
            i += len1;
            j += len2;
        } else {
            break;
        }
    }
    return common;
}

static std::string utf8_substring(const std::string& str, int start_char) {
    size_t i = 0;
    int chars = 0;
    while (i < str.length() && chars < start_char) {
        int len = 1;
        while (i + len < str.length() && (str[i + len] & 0xC0) == 0x80) len++;
        i += len;
        chars++;
    }
    return str.substr(i);
}

// Evdev keycodes map
static char get_ascii_from_keycode(uint32_t key, uint32_t mods) {
    bool shift = (mods & 1); // Shift check
    bool capslock = (mods & 2); // CapsLock check
    bool uppercase = shift ^ capslock; // XOR shift and capslock for letter case

    if (key >= 2 && key <= 10) {
        const char* symbols = "!@#$%^&*(";
        return shift ? symbols[key-2] : '1' + (key-2);
    }
    if (key == 11) return shift ? ')' : '0';
    if (key == 12) return shift ? '_' : '-';
    if (key == 13) return shift ? '+' : '=';
    if (key == 41) return shift ? '~' : '`';
    
    if (key >= 16 && key <= 25) {
        const char* row1 = "qwertyuiop";
        return uppercase ? (row1[key-16] - 32) : row1[key-16];
    }
    if (key >= 30 && key <= 38) {
        const char* row2 = "asdfghjkl";
        return uppercase ? (row2[key-30] - 32) : row2[key-30];
    }
    if (key >= 44 && key <= 50) {
        const char* row3 = "zxcvbnm";
        return uppercase ? (row3[key-44] - 32) : row3[key-44];
    }
    
    if (key == 57) return ' '; // Space
    if (key == 14) return '\b'; // Backspace
    if (key == 28) return '\n'; // Enter
    if (key == 26) return shift ? '{' : '[';
    if (key == 27) return shift ? '}' : ']';
    if (key == 43) return shift ? '|' : '\\';
    if (key == 39) return shift ? ':' : ';';
    if (key == 40) return shift ? '"' : '\'';
    if (key == 51) return shift ? '<' : ',';
    if (key == 52) return shift ? '>' : '.';
    if (key == 53) return shift ? '?' : '/';
    
    return 0; // Unhandled
}

static uint32_t g_modifiers = 0;
static std::set<uint32_t> eaten_keys;
bool g_terminal_mode = false;

static bool g_ctrl_pressed = false;
static bool g_shift_pressed = false;
static bool g_alt_pressed = false;
static bool g_other_pressed = false;

static MainWindow* g_mainWindow = nullptr;
static WindowTracker* g_windowTracker = nullptr;
static bool g_app_excluded = false;

void show_main_window() {
    if (g_mainWindow) {
        QMetaObject::invokeMethod(g_mainWindow, []() {
            g_mainWindow->show();
            g_mainWindow->raise();
            g_mainWindow->activateWindow();
        });
    }
}

static void keyboard_keymap(void* data, struct wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size) {
    close(fd);
}

static void keyboard_enter(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys) {}
static void keyboard_leave(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface) {}

static void keyboard_key(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state_key) {
    WaylandState* state = static_cast<WaylandState*>(data);
    
    if (!state->active || !state->context) {
        return;
    }

    // Track modifier key states
    if (state_key == 1) { // Pressed
        if (key == 29 || key == 97) { // Left/Right Ctrl
            g_ctrl_pressed = true;
        } else if (key == 42 || key == 54) { // Left/Right Shift
            g_shift_pressed = true;
        } else if (key == 56 || key == 100) { // Left/Right Alt
            g_alt_pressed = true;
        } else {
            g_other_pressed = true;
        }
    } else if (state_key == 0) { // Released
        if (key == 29 || key == 97) {
            if (g_ctrl_pressed && g_shift_pressed && !g_other_pressed) {
                if (g_mainWindow) {
                    g_mainWindow->setVietMode(!state->viet_mode);
                } else {
                    state->viet_mode = !state->viet_mode;
                }
            }
            g_ctrl_pressed = false;
            g_other_pressed = false;
        } else if (key == 42 || key == 54) {
            if (g_ctrl_pressed && g_shift_pressed && !g_other_pressed) {
                if (g_mainWindow) {
                    g_mainWindow->setVietMode(!state->viet_mode);
                } else {
                    state->viet_mode = !state->viet_mode;
                }
            }
            g_shift_pressed = false;
            g_other_pressed = false;
        } else if (key == 56 || key == 100) {
            g_alt_pressed = false;
            g_other_pressed = false;
        }
    }



    // Alt + Z hotkey
    if (key == 44 && g_alt_pressed && state_key == 1) {
        if (g_mainWindow) {
            g_mainWindow->setVietMode(!state->viet_mode);
        } else {
            state->viet_mode = !state->viet_mode;
        }
        eaten_keys.insert(key);
        return;
    }

    // Ctrl + Shift + F5 (CS+F5) hotkey to show settings
    if (key == 63 && g_ctrl_pressed && g_shift_pressed && state_key == 1) {
        show_main_window();
        eaten_keys.insert(key);
        return;
    }

    if (state_key == 0) {
        if (eaten_keys.count(key)) {
            eaten_keys.erase(key);
            return; // Key release for an eaten key: drop it
        }
        // Key release: forward it so the app doesn't get stuck
        zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
        return;
    }
    
    bool has_modifiers = (g_modifiers & (4 | 8 | 64)) != 0;
    char c = has_modifiers ? 0 : get_ascii_from_keycode(key, g_modifiers);
    
    std::stringstream ss_key;
    ss_key << "DEBUG: Key received. code=" << key << ", state=" << state_key 
           << ", ascii=" << (c ? c : '?');
    log_to_file(ss_key.str());
    
    if (key == 88 && g_ctrl_pressed && g_shift_pressed && state_key == 1) { // F12 = 88
        g_terminal_mode = !g_terminal_mode;
        std::stringstream ss_term;
        ss_term << "Terminal Mode Toggled: " << g_terminal_mode;
        log_to_file(ss_term.str());
        eaten_keys.insert(key);
        return;
    }

    std::stringstream ss_viet;
    ss_viet << "DEBUG: viet_mode=" << state->viet_mode;
    log_to_file(ss_viet.str());

    if (!state->viet_mode) {
        log_to_file("DEBUG: Forwarding in E mode");
        Bamboo_Reset();
        state->composed_word.clear();
        zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
        return;
    }

    if (c != 0) {
        if (state->content_purpose == 12 || g_terminal_mode || g_app_excluded) {
            // Preedit mode (Konsole, Kitty, Alacritty, or user-excluded apps)
            // Sử dụng Bamboo CGO
            if (c == '\b') {
                char* old_preedit = Bamboo_GetPreeditString();
                bool was_empty = (!old_preedit || strlen(old_preedit) == 0);
                if (old_preedit) free(old_preedit);
                
                if (was_empty) {
                    zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
                    return;
                }
                Bamboo_RemoveLastChar();
                
                char* new_preedit = Bamboo_GetPreeditString();
                uint32_t byte_len = new_preedit ? strlen(new_preedit) : 0;
                
                if (byte_len == 0) {
                    zwp_input_method_context_v1_preedit_string(state->context, state->latest_serial, "", "");
                } else {
                    zwp_input_method_context_v1_preedit_cursor(state->context, byte_len);
                    zwp_input_method_context_v1_preedit_styling(state->context, 0, byte_len, 5);
                    zwp_input_method_context_v1_preedit_string(state->context, state->latest_serial, new_preedit, new_preedit);
                }
                if (new_preedit) free(new_preedit);
                eaten_keys.insert(key);
                return;
            }
            
            if (!Bamboo_CanProcessKey(c)) {
                char* commit_str = Bamboo_GetCommitString();
                zwp_input_method_context_v1_preedit_string(state->context, state->latest_serial, "", "");
                if (commit_str && strlen(commit_str) > 0) {
                    zwp_input_method_context_v1_commit_string(state->context, state->latest_serial, commit_str);
                }
                if (commit_str) free(commit_str);
                Bamboo_Reset();
                
                zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
                return;
            } else {
                Bamboo_ProcessKey(c);
                char* preedit_str = Bamboo_GetPreeditString();
                uint32_t byte_len = strlen(preedit_str);
                
                std::stringstream ss;
                ss << "DEBUG: PREEDIT SENDING TO KONSOLE: '" << preedit_str << "' len=" << byte_len;
                log_to_file(ss.str());
                
                zwp_input_method_context_v1_preedit_cursor(state->context, byte_len);
                zwp_input_method_context_v1_preedit_styling(state->context, 0, byte_len, 5);
                zwp_input_method_context_v1_preedit_string(state->context, state->latest_serial, preedit_str, preedit_str);
                free(preedit_str);
                eaten_keys.insert(key);
                return;
            }
        } else {
            // Normal Mode (Chrome, Gtk, Qt apps) - Use Bamboo Diffing
            if (c == '\b') {
                if (state->composed_word.empty()) {
                    zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
                    return;
                }
                Bamboo_RemoveLastChar();
            } else if (!Bamboo_CanProcessKey(c)) {
                Bamboo_Reset();
                state->composed_word.clear();
                zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
                return;
            } else {
                Bamboo_ProcessKey(c);
            }
            
            char* preedit_str = Bamboo_GetPreeditString();
            std::string new_composed = preedit_str ? preedit_str : "";
            if (preedit_str) free(preedit_str);
            
            int common_bytes = 0;
            size_t i = 0, j = 0;
            while (i < state->composed_word.length() && j < new_composed.length()) {
                int len1 = 1, len2 = 1;
                while (i + len1 < state->composed_word.length() && (state->composed_word[i + len1] & 0xC0) == 0x80) len1++;
                while (j + len2 < new_composed.length() && (new_composed[j + len2] & 0xC0) == 0x80) len2++;
                
                if (len1 == len2 && state->composed_word.substr(i, len1) == new_composed.substr(j, len2)) {
                    common_bytes += len1;
                    i += len1;
                    j += len2;
                } else {
                    break;
                }
            }

            int byte_backs = state->composed_word.length() - common_bytes;
            if (byte_backs > 0) {
                // Giải pháp Hybrid:
                // Trình duyệt Chrome có một lỗi toán học khiến delete_surrounding_text bị hỏng nếu có vùng bôi đen ngược.
                // Do đó, nếu phát hiện có vùng bôi đen (Chrome Omnibox), ta dùng phím Backspace mô phỏng chỉ riêng cho lúc này.
                // Các trường hợp gõ bình thường khác, ta vẫn dùng delete_surrounding_text để đảm bảo mượt mà 100%.
                if (state->has_surrounding_text && state->surrounding_cursor != state->surrounding_anchor) {
                    int chars_to_delete = 0;
                    size_t idx = common_bytes;
                    while (idx < state->composed_word.length()) {
                        chars_to_delete++;
                        idx++;
                        while (idx < state->composed_word.length() && (state->composed_word[idx] & 0xC0) == 0x80) idx++;
                    }
                    // +1 Backspace để phá vỡ vùng bôi đen
                    chars_to_delete++;
                    for (int k = 0; k < chars_to_delete; k++) {
                        zwp_input_method_context_v1_key(state->context, state->latest_serial, time, 14, 1);
                        zwp_input_method_context_v1_key(state->context, state->latest_serial, time, 14, 0);
                    }
                    // Xóa bôi đen nội bộ để không bị lặp lại nếu Wayland chậm
                    state->surrounding_cursor = state->surrounding_anchor;
                } else {
                    zwp_input_method_context_v1_delete_surrounding_text(state->context, -byte_backs, byte_backs);
                }
            }

            std::string suffix = new_composed.substr(common_bytes);
            if (!suffix.empty()) {
                zwp_input_method_context_v1_commit_string(state->context, state->latest_serial, suffix.c_str());
            }

            // Đã commit_string ở trên rồi nên không cần làm gì thêm ở đây nữa
            state->composed_word = new_composed;
            eaten_keys.insert(key);
            return;
        }
    } else {
        // c == 0 (Phím chức năng, phím tắt Ctrl, Alt, Arrow, Esc...)
        if (state->content_purpose == 12 || g_terminal_mode || g_app_excluded) {
            char* preedit = Bamboo_GetPreeditString();
            if (preedit && strlen(preedit) > 0) {
                zwp_input_method_context_v1_commit_string(state->context, state->latest_serial, preedit);
                zwp_input_method_context_v1_preedit_string(state->context, state->latest_serial, "", "");
                Bamboo_Reset();
            }
            if (preedit) free(preedit);
        } else {
            Bamboo_Reset();
            state->composed_word.clear();
            state->raw_keys_normal.clear();
        }
    }
    
    // If we didn't handle it (or if it was a backspace/unhandled), forward it to the client
    zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
}

static void keyboard_modifiers(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    WaylandState* state = static_cast<WaylandState*>(data);
    g_modifiers = mods_depressed | mods_latched | mods_locked;
    
    if (state->context) {
        zwp_input_method_context_v1_modifiers(state->context, serial, mods_depressed, mods_latched, mods_locked, group);
    }
}

static void keyboard_repeat_info(void* data, struct wl_keyboard* keyboard, int32_t rate, int32_t delay) {}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_keymap,
    .enter = keyboard_enter,
    .leave = keyboard_leave,
    .key = keyboard_key,
    .modifiers = keyboard_modifiers,
    .repeat_info = keyboard_repeat_info,
};


static void input_method_context_surrounding_text(void* data, struct zwp_input_method_context_v1* context, const char* text, uint32_t cursor, uint32_t anchor) {
    WaylandState* state = (WaylandState*)data;
    if (text) state->surrounding_text = text;
    else state->surrounding_text = "";
    state->surrounding_cursor = cursor;
    state->surrounding_anchor = anchor;
    state->has_surrounding_text = true;
}
static void input_method_context_reset(void* data, struct zwp_input_method_context_v1* context) {
    WaylandState* state = static_cast<WaylandState*>(data);
    if (state) {
        Bamboo_Reset();
        state->composed_word.clear();
    }
}
static void input_method_context_content_type(void* data, struct zwp_input_method_context_v1* context, uint32_t hint, uint32_t purpose) {
    WaylandState* state = static_cast<WaylandState*>(data);
    if (state) {
        state->content_purpose = purpose;
        std::stringstream ss_ct;
        ss_ct << "DEBUG: content_type hint=" << hint << ", purpose=" << purpose;
        log_to_file(ss_ct.str());
    }
}
static void input_method_context_invoke_action(void* data, struct zwp_input_method_context_v1* context, uint32_t button, uint32_t index) {}

static void input_method_context_commit_state(void* data, struct zwp_input_method_context_v1* context, uint32_t serial) {
    WaylandState* state = static_cast<WaylandState*>(data);
    state->latest_serial = serial;
}

static void input_method_context_preferred_language(void* data, struct zwp_input_method_context_v1* context, const char* language) {}

static const struct zwp_input_method_context_v1_listener input_method_context_listener = {
    .surrounding_text = input_method_context_surrounding_text,
    .reset = input_method_context_reset,
    .content_type = input_method_context_content_type,
    .invoke_action = input_method_context_invoke_action,
    .commit_state = input_method_context_commit_state,
    .preferred_language = input_method_context_preferred_language,
};


static void input_method_activate(void* data, struct zwp_input_method_v1* input_method, struct zwp_input_method_context_v1* context) {
    WaylandState* state = static_cast<WaylandState*>(data);
    state->active = true;

    if (state->keyboard) {
        wl_proxy_destroy((struct wl_proxy*)state->keyboard);
        state->keyboard = nullptr;
    }

    if (state->context) {
        zwp_input_method_context_v1_destroy(state->context);
    }
    
    state->context = context;
    zwp_input_method_context_v1_add_listener(state->context, &input_method_context_listener, state);
    
    state->keyboard = zwp_input_method_context_v1_grab_keyboard(state->context);
    if (state->keyboard) {
        wl_keyboard_add_listener(state->keyboard, &keyboard_listener, state);
    }
}

static void input_method_deactivate(void* data, struct zwp_input_method_v1* input_method, struct zwp_input_method_context_v1* context) {
    WaylandState* state = static_cast<WaylandState*>(data);
    state->active = false;
    
    if (state->keyboard) {
        wl_proxy_destroy((struct wl_proxy*)state->keyboard);
        state->keyboard = nullptr;
    }
    
    if (state->context == context) {
        zwp_input_method_context_v1_destroy(state->context);
        state->context = nullptr;
    } else {
        zwp_input_method_context_v1_destroy(context);
    }
}

static const struct zwp_input_method_v1_listener input_method_listener = {
    .activate = input_method_activate,
    .deactivate = input_method_deactivate,
};

static void registry_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    WaylandState* state = static_cast<WaylandState*>(data);
    
    if (strcmp(interface, wl_seat_interface.name) == 0) {
        state->seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, 7));
    } else if (strcmp(interface, zwp_input_method_v1_interface.name) == 0) {
        state->input_method = static_cast<zwp_input_method_v1*>(wl_registry_bind(registry, name, &zwp_input_method_v1_interface, 1));
    }
}

static void registry_global_remove(void* data, struct wl_registry* registry, uint32_t name) {}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

int main(int argc, char **argv) {
    Bamboo_Init(); // KÍCH HOẠT NÃO CGO BAMBOO
    
    setenv("QT_QPA_PLATFORM", "wayland;xcb", 0); // Prefer Wayland, fallback to xcb. 0 means don't overwrite if user explicitly set it.
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    WaylandState state = {};

    state.display = wl_display_connect(NULL);
    if (!state.display) {
        std::cerr << "Failed to connect to Wayland display. Running in GUI-only mode." << std::endl;
    } else {
        state.registry = wl_display_get_registry(state.display);
        wl_registry_add_listener(state.registry, &registry_listener, &state);
        wl_display_roundtrip(state.display);
    }

    bool has_wayland_im = (state.input_method != nullptr);
    if (state.display && !has_wayland_im) {
        std::cerr << "Compositor does not support zwp_input_method_v1. Running in GUI-only mode." << std::endl;
    } else if (state.display && has_wayland_im) {
        zwp_input_method_v1_add_listener(state.input_method, &input_method_listener, &state);
    }

    bool is_gnome_edition = !has_wayland_im;
    app.setQuitOnLastWindowClosed(is_gnome_edition);

    MainWindow mainWindow(&state.viet_mode, is_gnome_edition);
    g_mainWindow = &mainWindow;

    WindowTracker windowTracker;
    g_windowTracker = &windowTracker;
    QObject::connect(&windowTracker, &WindowTracker::activeWindowChangedSignal, [&](const QString& windowClass) {
        g_app_excluded = windowTracker.isAppExcluded(windowClass.toStdString());
        if (g_app_excluded) {
            std::stringstream ss;
            ss << "DEBUG: Application excluded: " << windowClass.toStdString();
            log_to_file(ss.str());
            state.composed_word.clear();
            Bamboo_Reset();
        }
    });

    windowTracker.injectKWinScript();

    TrayIcon* trayIcon = nullptr;
    if (!is_gnome_edition) {
        trayIcon = new TrayIcon(&state.viet_mode, &mainWindow, is_gnome_edition);
    }

    bool showExclude = false;
    if (argc > 1) {
        if (strcmp(argv[1], "--setup") == 0) {
            mainWindow.show();
        } else if (strcmp(argv[1], "--exclude") == 0) {
            mainWindow.show();
            showExclude = true;
        }
    }
    if (showExclude) {
        mainWindow.selectTab("Danh sách loại trừ");
    }

    log_to_file("Wayland IM v1 Client started with Qt GUI. Waiting for events...");

    QSocketNotifier* notifier = nullptr;
    if (state.display) {
        int fd = wl_display_get_fd(state.display);
        notifier = new QSocketNotifier(fd, QSocketNotifier::Read, &app);
        QObject::connect(notifier, &QSocketNotifier::activated, [&state, &app]() {
            if (wl_display_dispatch(state.display) == -1) {
                std::cerr << "Wayland display disconnected or error." << std::endl;
                app.quit();
                return;
            }
            while (wl_display_dispatch_pending(state.display) > 0) {
                // Keep dispatching pending events
            }
            wl_display_flush(state.display);
        });

        // We must dispatch any pending events before entering the event loop
        while (wl_display_dispatch_pending(state.display) > 0) {}
        wl_display_flush(state.display);
    }

    int ret = app.exec();

    if (state.keyboard) {
        wl_proxy_destroy((struct wl_proxy*)state.keyboard);
    }
    if (state.context) {
        zwp_input_method_context_v1_destroy(state.context);
    }
    if (state.display) {
        wl_display_disconnect(state.display);
    }
    if (trayIcon) {
        delete trayIcon;
    }

    return ret;
}
