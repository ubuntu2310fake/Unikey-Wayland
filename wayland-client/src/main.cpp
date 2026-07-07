#include <wayland-client.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <map>
#include <set>

#include <QApplication>
#include <QSocketNotifier>
#include "mainwindow.h"
#include "trayicon.h"

#include "input-method-unstable-v1-client-protocol.h"
#include "../include/ukengine_wrapper.h"

struct WaylandState {
    wl_display* display;
    wl_registry* registry;
    wl_seat* seat;
    zwp_input_method_v1* input_method;
    zwp_input_method_context_v1* context;
    wl_keyboard* keyboard;
    
    UkEngineWrapper ukengine;
    bool active;
    uint32_t latest_serial;
    std::string composed_word = "";
};

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

static bool g_ctrl_pressed = false;
static bool g_shift_pressed = false;
static bool g_alt_pressed = false;
static bool g_other_pressed = false;

static MainWindow* g_mainWindow = nullptr;

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
            if (g_ctrl_pressed && g_shift_pressed && !g_other_pressed && state->ukengine.getSwitchKey() == 0) {
                if (g_mainWindow) {
                    g_mainWindow->setVietMode(!state->ukengine.getVietMode());
                } else {
                    state->ukengine.setVietMode(!state->ukengine.getVietMode());
                }
            }
            g_ctrl_pressed = false;
            g_other_pressed = false;
        } else if (key == 42 || key == 54) {
            if (g_ctrl_pressed && g_shift_pressed && !g_other_pressed && state->ukengine.getSwitchKey() == 0) {
                if (g_mainWindow) {
                    g_mainWindow->setVietMode(!state->ukengine.getVietMode());
                } else {
                    state->ukengine.setVietMode(!state->ukengine.getVietMode());
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
    if (key == 44 && g_alt_pressed && state_key == 1 && state->ukengine.getSwitchKey() == 1) {
        if (g_mainWindow) {
            g_mainWindow->setVietMode(!state->ukengine.getVietMode());
        } else {
            state->ukengine.setVietMode(!state->ukengine.getVietMode());
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
    
    std::cout << "DEBUG: Key received. code=" << key << ", state=" << state_key 
              << ", ascii=" << (c ? c : '?') << std::endl;
    
    if (c != 0) {
        int backs = 0;
        std::string processed = state->ukengine.processKey(c, backs);

        std::cout << "DEBUG: Processed result. backs=" << backs 
                  << ", processed='" << processed << "'" << std::endl;

        if (backs > 0 || !processed.empty()) {
            if (backs > 0) {
                int byte_backs = backs;
                if (backs <= (int)state->composed_word.length()) {
                    int i = state->composed_word.length();
                    int chars_to_delete = backs;
                    while (chars_to_delete > 0 && i > 0) {
                        i--;
                        while (i > 0 && (state->composed_word[i] & 0xC0) == 0x80) {
                            i--;
                        }
                        chars_to_delete--;
                    }
                    byte_backs = state->composed_word.length() - i;
                    state->composed_word.erase(i, byte_backs);
                } else {
                    state->composed_word.clear();
                }
                zwp_input_method_context_v1_delete_surrounding_text(state->context, -byte_backs, byte_backs);
            }
            if (!processed.empty()) {
                state->composed_word += processed;
                zwp_input_method_context_v1_commit_string(state->context, state->latest_serial, processed.c_str());
            } else if (backs > 0) {
                zwp_input_method_context_v1_commit_string(state->context, state->latest_serial, "");
            }
            eaten_keys.insert(key);
            return;
        } else if (c != '\b' && c != '\n') {
            // Printable character, not handled by UniKey.
            // We commit it directly to avoid race conditions with physical key forwarding
            std::string str(1, c);
            state->composed_word.clear();
            zwp_input_method_context_v1_commit_string(state->context, state->latest_serial, str.c_str());
            eaten_keys.insert(key);
            return;
        } else {
            // c is \b or \n. We must reset because we are forwarding them to the client.
            state->ukengine.reset();
            state->composed_word.clear();
        }
    } else {
        std::cout << "DEBUG: Key not handled, ascii=0. Forwarding..." << std::endl;
        state->ukengine.reset();
        state->composed_word.clear();
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


static void input_method_context_surrounding_text(void* data, struct zwp_input_method_context_v1* context, const char* text, uint32_t cursor, uint32_t anchor) {}
static void input_method_context_reset(void* data, struct zwp_input_method_context_v1* context) {}
static void input_method_context_content_type(void* data, struct zwp_input_method_context_v1* context, uint32_t hint, uint32_t purpose) {}
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
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    WaylandState state = {};
    state.ukengine.init();

    state.display = wl_display_connect(NULL);
    if (!state.display) {
        std::cerr << "Failed to connect to Wayland display." << std::endl;
        return 1;
    }

    state.registry = wl_display_get_registry(state.display);
    wl_registry_add_listener(state.registry, &registry_listener, &state);

    wl_display_roundtrip(state.display);

    if (!state.input_method) {
        std::cerr << "Compositor does not support zwp_input_method_v1." << std::endl;
        return 1;
    }

    zwp_input_method_v1_add_listener(state.input_method, &input_method_listener, &state);

    MainWindow mainWindow(&state.ukengine);
    g_mainWindow = &mainWindow;
    TrayIcon trayIcon(&state.ukengine, &mainWindow);

    std::cout << "Wayland IM v1 Client started with Qt GUI. Waiting for events..." << std::endl;

    int fd = wl_display_get_fd(state.display);
    QSocketNotifier notifier(fd, QSocketNotifier::Read);
    QObject::connect(&notifier, &QSocketNotifier::activated, [&state, &app]() {
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

    int ret = app.exec();

    if (state.keyboard) {
        wl_proxy_destroy((struct wl_proxy*)state.keyboard);
    }
    if (state.context) {
        zwp_input_method_context_v1_destroy(state.context);
    }
    wl_display_disconnect(state.display);

    return ret;
}
