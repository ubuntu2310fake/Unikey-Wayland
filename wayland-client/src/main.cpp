#include <wayland-client.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <set>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <deque>

static void log_to_file(const std::string& msg) {
    static const bool enabled = [] {
        const char* value = getenv("UNIKEY_WAYLAND_DEBUG");
        return value && strcmp(value, "0") != 0;
    }();
    if (!enabled) return;

    std::ofstream f("/tmp/uk_debug.log", std::ios::app);
    if (f.is_open()) {
        f << msg << std::endl;
    }
}

#include <QApplication>
#include <QSocketNotifier>
#include <QTimer>
#include "mainwindow.h"
#include "trayicon.h"

#include "input-method-unstable-v1-client-protocol.h"
// No ukengine_wrapper needed
#include "windowtracker.h"
#include "libbamboo.h"
#include "text_transaction.h"

struct QueuedKeyEvent {
    uint32_t serial;
    uint32_t time;
    uint32_t key;
    uint32_t state;
    uint32_t modifiers;
};

struct PendingEdit {
    bool active = false;
    uint64_t generation = 0;
    uint64_t timeout_token = 0;
    uint64_t repair_token = 0;
    std::string expected_tail;
    std::string failure_tail;
    std::string desired_text;
    size_t failure_bytes = 0;
    size_t repair_bytes = 0;
    std::string repair_text;
    SurroundingSnapshot observed_failure;
    unsigned repair_attempts = 0;
};

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
    uint32_t content_purpose = 0;
    
    std::string surrounding_text = "";
    uint32_t surrounding_cursor = 0;
    uint32_t surrounding_anchor = 0;
    bool has_surrounding_text = false;
    PendingEdit pending_edit;
    std::deque<QueuedKeyEvent> queued_keys;
    std::deque<std::string> recent_tails;
    bool draining_keys = false;
    QTimer* drain_timer = nullptr;
    uint64_t next_edit_generation = 0;
    uint64_t next_timer_token = 0;
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
WindowTracker* g_windowTracker = nullptr;

static bool g_app_excluded = false;

static void reset_composition(WaylandState* state, bool clear_client_preedit = false) {
    Bamboo_Reset();
    if (state) {
        if (clear_client_preedit && state->context && !state->composed_word.empty()) {
            zwp_input_method_context_v1_preedit_string(
                state->context, state->latest_serial, "", "");
        }
        state->composed_word.clear();
    }
}

static bool has_fresh_surrounding(const WaylandState* state) {
    return state->has_surrounding_text && !state->pending_edit.active;
}

static void clear_pending_edit(WaylandState* state) {
    state->pending_edit = {};
}

static void remember_committed_tail(WaylandState* state,
                                    const std::string& tail) {
    if (tail.empty()) return;
    state->recent_tails.push_back(tail);
    if (state->recent_tails.size() > 32) state->recent_tails.pop_front();
}

static void drain_queued_keys(WaylandState* state);

static void arm_pending_timeout(WaylandState* state) {
    const uint64_t generation = state->pending_edit.generation;
    const uint64_t token = ++state->next_timer_token;
    state->pending_edit.timeout_token = token;
    QTimer::singleShot(750, QCoreApplication::instance(),
                       [state, generation, token]() {
        if (!state->pending_edit.active ||
            state->pending_edit.generation != generation ||
            state->pending_edit.timeout_token != token) {
            return;
        }
        // A client that never reports surrounding text must not stall input.
        clear_pending_edit(state);
        state->has_surrounding_text = false;
        reset_composition(state);
        drain_queued_keys(state);
    });
}

static void schedule_queued_keys(WaylandState* state) {
    if (!state->pending_edit.active && !state->queued_keys.empty() &&
        state->drain_timer && !state->drain_timer->isActive()) {
        state->drain_timer->start();
    }
}

static void replace_native_composition(WaylandState* state,
                                       const std::string& new_composition,
                                       const std::string& trailing_text = "") {
    const std::string old_composition = state->composed_word;
    const size_t common = utf8_common_prefix_bytes(old_composition, new_composition);
    const size_t old_tail_bytes = old_composition.size() - common;
    const std::string suffix = new_composition.substr(common) + trailing_text;

    if (old_tail_bytes == 0 && suffix.empty()) {
        state->composed_word = new_composition;
        return;
    }

    SurroundingReplacement replacement;
    if (state->has_surrounding_text) {
        replacement = make_surrounding_replacement(old_tail_bytes,
                                                   state->surrounding_text,
                                                   state->surrounding_cursor,
                                                   state->surrounding_anchor);
    } else {
        replacement.index = -static_cast<int32_t>(old_tail_bytes);
        replacement.length = static_cast<uint32_t>(old_tail_bytes);
    }

    const auto expected = apply_surrounding_replacement(
        state->surrounding_text, state->surrounding_cursor,
        state->surrounding_anchor, replacement, suffix);

    if (old_tail_bytes > 0) {
        zwp_input_method_context_v1_delete_surrounding_text(
            state->context, replacement.index, replacement.length);
    }

    // An empty commit still applies the pending deletion.
    zwp_input_method_context_v1_commit_string(
        state->context, state->latest_serial, suffix.c_str());
    remember_committed_tail(state, new_composition + trailing_text);

    if (expected.valid) {
        state->surrounding_text = expected.text;
        state->surrounding_cursor = expected.cursor;
        state->surrounding_anchor = expected.anchor;
    }

    if (old_tail_bytes > 0 && expected.valid) {
        state->pending_edit.active = true;
        state->pending_edit.generation = ++state->next_edit_generation;
        state->pending_edit.expected_tail = new_composition + trailing_text;
        state->pending_edit.desired_text = new_composition + trailing_text;
        state->pending_edit.failure_tail = old_composition + suffix;
        state->pending_edit.failure_bytes =
            old_composition.size() + suffix.size();
        state->pending_edit.repair_attempts = 0;
        arm_pending_timeout(state);
    }
    state->composed_word = new_composition;
}

static bool repair_failed_edit(WaylandState* state,
                               const std::string& text,
                               uint32_t cursor,
                               uint32_t anchor) {
    if (state->pending_edit.repair_attempts >= 3 ||
        cursor > text.size() || anchor > text.size()) {
        return false;
    }

    const auto replacement = make_surrounding_replacement(
        state->pending_edit.repair_bytes, text, cursor, anchor);
    if (!replacement.uses_surrounding) return false;

    zwp_input_method_context_v1_delete_surrounding_text(
        state->context, replacement.index, replacement.length);
    zwp_input_method_context_v1_commit_string(
        state->context, state->latest_serial,
        state->pending_edit.repair_text.c_str());
    remember_committed_tail(state, state->pending_edit.repair_text);

    const auto expected = apply_surrounding_replacement(
        text, cursor, anchor, replacement, state->pending_edit.repair_text);
    if (expected.valid) {
        state->surrounding_text = expected.text;
        state->surrounding_cursor = expected.cursor;
        state->surrounding_anchor = expected.anchor;
    }
    state->pending_edit.failure_tail += state->pending_edit.repair_text;
    state->pending_edit.failure_bytes += state->pending_edit.repair_text.size();
    ++state->pending_edit.repair_attempts;
    arm_pending_timeout(state);
    return true;
}

static void arm_failed_edit_repair(WaylandState* state) {
    const uint64_t generation = state->pending_edit.generation;
    const uint64_t token = ++state->next_timer_token;
    const SurroundingSnapshot failure = state->pending_edit.observed_failure;
    state->pending_edit.repair_token = token;
    QTimer::singleShot(12, QCoreApplication::instance(),
                       [state, generation, token, failure]() {
        if (!state->pending_edit.active ||
            state->pending_edit.generation != generation ||
            state->pending_edit.repair_token != token || !failure.valid) {
            return;
        }
        repair_failed_edit(state, failure.text,
                           failure.cursor, failure.anchor);
    });
}

static std::string bamboo_string(bool final) {
    char* value = final ? Bamboo_GetCommitString() : Bamboo_GetPreeditString();
    std::string result = value ? value : "";
    if (value) free(value);
    return result;
}

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

static void process_keyboard_key(WaylandState* state, uint32_t serial,
                                 uint32_t time, uint32_t key,
                                 uint32_t state_key, uint32_t modifiers) {
    static bool g_switched = false;

    // Track modifier key states
    if (state_key == 1) { // Pressed
        if (key == 29 || key == 97) { // Left/Right Ctrl
            g_ctrl_pressed = true;
            if (!g_shift_pressed && !g_alt_pressed) {
                g_other_pressed = false;
                g_switched = false;
            }
        } else if (key == 42 || key == 54) { // Left/Right Shift
            g_shift_pressed = true;
            if (!g_ctrl_pressed && !g_alt_pressed) {
                g_other_pressed = false;
                g_switched = false;
            }
        } else if (key == 56 || key == 100) { // Left/Right Alt
            g_alt_pressed = true;
        } else {
            g_other_pressed = true;
        }
    } else if (state_key == 0) { // Released
        int switchKeyConfig = g_mainWindow ? g_mainWindow->getSwitchKey() : 0;
        if (key == 29 || key == 97) {
            if (switchKeyConfig == 0 && g_ctrl_pressed && g_shift_pressed && !g_other_pressed && !g_switched) {
                reset_composition(state, true);
                if (g_mainWindow) {
                    g_mainWindow->setVietMode(!state->viet_mode);
                } else {
                    state->viet_mode = !state->viet_mode;
                }
                g_switched = true;
            }
            g_ctrl_pressed = false;
            if (!g_shift_pressed) {
                g_other_pressed = false;
                g_switched = false;
            }
        } else if (key == 42 || key == 54) {
            if (switchKeyConfig == 0 && g_ctrl_pressed && g_shift_pressed && !g_other_pressed && !g_switched) {
                reset_composition(state, true);
                if (g_mainWindow) {
                    g_mainWindow->setVietMode(!state->viet_mode);
                } else {
                    state->viet_mode = !state->viet_mode;
                }
                g_switched = true;
            }
            g_shift_pressed = false;
            if (!g_ctrl_pressed) {
                g_other_pressed = false;
                g_switched = false;
            }
        } else if (key == 56 || key == 100) {
            g_alt_pressed = false;
            g_other_pressed = false;
        }
    }



    // Alt + Z hotkey
    int switchKeyConfig = g_mainWindow ? g_mainWindow->getSwitchKey() : 0;
    bool real_alt_pressed = (modifiers & 8) != 0; // Better check to avoid sticky Alt bug
    if (switchKeyConfig == 1 && key == 44 && real_alt_pressed && state_key == 1) {
        reset_composition(state, true);
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
    
    bool has_modifiers = (modifiers & (4 | 8 | 64)) != 0;
    char c = has_modifiers ? 0 : get_ascii_from_keycode(key, modifiers);
    
    std::stringstream ss_key;
    ss_key << "DEBUG: Key received. code=" << key << ", state=" << state_key 
           << ", ascii=" << (c ? c : '?');
    log_to_file(ss_key.str());
    
    std::stringstream ss_viet;
    ss_viet << "DEBUG: viet_mode=" << state->viet_mode;
    log_to_file(ss_viet.str());

    if (!state->viet_mode) {
        log_to_file("DEBUG: Forwarding in E mode");
        reset_composition(state, true);
        zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
        return;
    }

    if (c != 0) {
        if (state->content_purpose == 12 || g_app_excluded) {
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
                state->composed_word = new_preedit ? new_preedit : "";
                if (new_preedit) free(new_preedit);
                eaten_keys.insert(key);
                return;
            }
            
            if (!Bamboo_CanProcessKey(c)) {
                std::string final_commit = bamboo_string(true);
                
                // Gõ tắt (Macro)
                if (g_mainWindow && g_mainWindow->isMacroEnabled()) {
                    const auto& macros = g_mainWindow->getMacros();
                    auto macro = macros.find(final_commit);
                    if (macro != macros.end()) {
                        final_commit = macro->second;
                    }
                }
                
                if (final_commit.length() > 0) {
                    zwp_input_method_context_v1_commit_string(state->context, state->latest_serial, final_commit.c_str());
                }
                reset_composition(state);
                
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
                state->composed_word = preedit_str;
                free(preedit_str);
                eaten_keys.insert(key);
                return;
            }
        } else {
            // Normal Mode (Chrome, Gtk, Qt apps) - Use Bamboo Diffing
            if (has_fresh_surrounding(state) &&
                !composition_matches_surrounding(state->surrounding_text,
                                                 state->surrounding_cursor,
                                                 state->surrounding_anchor,
                                                 state->composed_word)) {
                reset_composition(state);
            }

            if (c == '\b') {
                if (state->composed_word.empty()) {
                    zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
                    return;
                }
                Bamboo_RemoveLastChar();
            } else if (!Bamboo_CanProcessKey(c)) {
                std::string final_word = bamboo_string(true);
                const bool had_composition = !state->composed_word.empty();
                
                // Gõ tắt (Macro)
                if (g_mainWindow && g_mainWindow->isMacroEnabled()) {
                    const auto& macros = g_mainWindow->getMacros();
                    auto macro = macros.find(final_word);
                    if (macro != macros.end()) {
                        final_word = macro->second;
                    }
                }

                if (had_composition && c != '\n') {
                    replace_native_composition(state, final_word, std::string(1, c));
                    reset_composition(state);
                    eaten_keys.insert(key);
                    return;
                }

                replace_native_composition(state, final_word);
                reset_composition(state);
                zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
                return;
            } else {
                Bamboo_ProcessKey(c);
            }

            replace_native_composition(state, bamboo_string(false));
            eaten_keys.insert(key);
            return;
        }
    } else {
        // c == 0 (Phím chức năng, phím tắt Ctrl, Alt, Arrow, Esc...)
        if (state->content_purpose == 12 || g_app_excluded) {
            std::string final_commit = bamboo_string(true);
            if (!final_commit.empty()) {
                zwp_input_method_context_v1_commit_string(
                    state->context, state->latest_serial, final_commit.c_str());
            }
            reset_composition(state);
        } else {
            reset_composition(state);
            clear_pending_edit(state);
        }
    }
    
    // If we didn't handle it (or if it was a backspace/unhandled), forward it to the client
    zwp_input_method_context_v1_key(state->context, serial, time, key, state_key);
}

static void route_keyboard_key(WaylandState* state, uint32_t serial,
                               uint32_t time, uint32_t key,
                               uint32_t state_key, uint32_t modifiers) {
    if (!state->active || !state->context) return;

    if (state->pending_edit.active || !state->queued_keys.empty()) {
        if (state->queued_keys.size() < 512) {
            state->queued_keys.push_back({serial, time, key, state_key, modifiers});
        }
        schedule_queued_keys(state);
        return;
    }

    process_keyboard_key(state, serial, time, key, state_key, modifiers);
}

static void drain_queued_keys(WaylandState* state) {
    if (state->draining_keys || state->pending_edit.active ||
        state->queued_keys.empty()) return;
    state->draining_keys = true;
    const auto event = state->queued_keys.front();
    state->queued_keys.pop_front();
    process_keyboard_key(state, event.serial, event.time, event.key,
                         event.state, event.modifiers);
    state->draining_keys = false;
    schedule_queued_keys(state);
}

static void keyboard_key(void* data, struct wl_keyboard* keyboard,
                         uint32_t serial, uint32_t time, uint32_t key,
                         uint32_t state_key) {
    route_keyboard_key(static_cast<WaylandState*>(data), serial, time, key,
                       state_key, g_modifiers);
}

static void keyboard_modifiers(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    WaylandState* state = static_cast<WaylandState*>(data);
    g_modifiers = mods_depressed | mods_latched | mods_locked;
    
    if (mods_depressed == 0) {
        g_ctrl_pressed = false;
        g_shift_pressed = false;
        g_alt_pressed = false;
        g_other_pressed = false;
    }

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
    const std::string reported_text = text ? text : "";
    const bool was_pending = state->pending_edit.active;

    if (was_pending) {
        if (!state->pending_edit.failure_tail.empty() &&
            surrounding_prefix_ends_with(
                reported_text, cursor, anchor,
                state->pending_edit.failure_tail)) {
            state->pending_edit.repair_bytes =
                state->pending_edit.failure_bytes;
            state->pending_edit.repair_text =
                state->pending_edit.desired_text;
            state->pending_edit.observed_failure = {
                reported_text, cursor, anchor, true};
            arm_failed_edit_repair(state);
            return;
        }
        if (!surrounding_prefix_ends_with(
                reported_text, cursor, anchor,
                state->pending_edit.expected_tail)) {
            // KWin exposes the delete and commit as separate text-input-v3
            // transactions. Ignore the intermediate or an older callback.
            return;
        }
    } else if (!state->recent_tails.empty()) {
        for (size_t i = state->recent_tails.size(); i-- > 0;) {
            if (!surrounding_prefix_ends_with(
                    reported_text, cursor, anchor, state->recent_tails[i])) {
                continue;
            }
            if (i + 1 != state->recent_tails.size()) {
                // A commit callback can arrive after a newer key has already
                // been processed. Keep the newer optimistic caret state.
                return;
            }
            break;
        }
    }

    state->surrounding_text = reported_text;
    state->surrounding_cursor = cursor;
    state->surrounding_anchor = anchor;
    state->has_surrounding_text = true;
    clear_pending_edit(state);
    drain_queued_keys(state);
}
static void input_method_context_reset(void* data, struct zwp_input_method_context_v1* context) {
    WaylandState* state = static_cast<WaylandState*>(data);
    if (state) {
        reset_composition(state);
        state->has_surrounding_text = false;
        clear_pending_edit(state);
        state->queued_keys.clear();
        state->recent_tails.clear();
        if (state->drain_timer) state->drain_timer->stop();
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
    log_to_file("DEBUG: input_method_activate triggered by KWin!");
    WaylandState* state = static_cast<WaylandState*>(data);
    state->active = true;

    g_ctrl_pressed = false;
    g_shift_pressed = false;
    g_alt_pressed = false;
    g_other_pressed = false;
    eaten_keys.clear();
    state->queued_keys.clear();
    state->recent_tails.clear();
    if (state->drain_timer) state->drain_timer->stop();
    reset_composition(state);
    state->content_purpose = 0;
    state->has_surrounding_text = false;
    clear_pending_edit(state);

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
        log_to_file("DEBUG: grab_keyboard succeeded! Adding keyboard listener.");
        wl_keyboard_add_listener(state->keyboard, &keyboard_listener, state);
    } else {
        log_to_file("ERROR: grab_keyboard returned NULL!");
    }
}

static void input_method_deactivate(void* data, struct zwp_input_method_v1* input_method, struct zwp_input_method_context_v1* context) {
    WaylandState* state = static_cast<WaylandState*>(data);
    state->active = false;

    g_ctrl_pressed = false;
    g_shift_pressed = false;
    g_alt_pressed = false;
    g_other_pressed = false;
    eaten_keys.clear();
    state->queued_keys.clear();
    state->recent_tails.clear();
    if (state->drain_timer) state->drain_timer->stop();
    reset_composition(state);
    state->has_surrounding_text = false;
    clear_pending_edit(state);
    
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
    Bamboo_Init();
    int im_socket_fd = -1;
    char* wayland_socket_env = getenv("WAYLAND_SOCKET");
    if (wayland_socket_env) {
        int orig_fd = atoi(wayland_socket_env);
        im_socket_fd = dup(orig_fd);
        unsetenv("WAYLAND_SOCKET");
    }

    setenv("QT_QPA_PLATFORM", "wayland;xcb", 0); // Prefer Wayland, fallback to xcb.
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    WaylandState state = {};
    QTimer drainTimer;
    drainTimer.setSingleShot(true);
    drainTimer.setInterval(1);
    state.drain_timer = &drainTimer;
    QObject::connect(&drainTimer, &QTimer::timeout,
                     [&state]() { drain_queued_keys(&state); });

    if (im_socket_fd >= 0) {
        state.display = wl_display_connect_to_fd(im_socket_fd);
    } else {
        state.display = wl_display_connect(NULL);
    }

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

    bool is_gnome_edition = false;

    MainWindow mainWindow(&state.viet_mode, is_gnome_edition);
    g_mainWindow = &mainWindow;

    WindowTracker windowTracker;
    g_windowTracker = &windowTracker;
    QObject::connect(&windowTracker, &WindowTracker::activeWindowChangedSignal, [&](const QString& windowClass) {
        const bool was_excluded = g_app_excluded;
        g_app_excluded = windowTracker.isAppExcluded(windowClass.toStdString());
        if (was_excluded != g_app_excluded) {
            state.queued_keys.clear();
            state.recent_tails.clear();
            if (state.drain_timer) state.drain_timer->stop();
            reset_composition(&state, true);
            clear_pending_edit(&state);
            state.has_surrounding_text = false;
        }
        if (g_app_excluded) {
            std::stringstream ss;
            ss << "DEBUG: Application excluded: " << windowClass.toStdString();
            log_to_file(ss.str());
        }
    });

    windowTracker.injectKWinScript();

    TrayIcon* trayIcon = new TrayIcon(&state.viet_mode, &mainWindow, false);

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

    QSocketNotifier* waylandNotifier = nullptr;
    if (state.display) {
        waylandNotifier = new QSocketNotifier(
            wl_display_get_fd(state.display), QSocketNotifier::Read, &app);
        QObject::connect(waylandNotifier, &QSocketNotifier::activated,
                         [&state, &app, waylandNotifier]() {
            if (wl_display_dispatch(state.display) == -1) {
                waylandNotifier->setEnabled(false);
                std::cerr << "Wayland display disconnected or error." << std::endl;
                app.quit();
                return;
            }
            while (wl_display_dispatch_pending(state.display) > 0) {}
            wl_display_flush(state.display);
        });
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
