#include <ibus.h>
#include <string>
#include <algorithm>
#include <fstream>
#include <vector>
#include <gio/gio.h>
#include <map>

#include "libbamboo.h"

static bool g_macroEnabled = false;
static std::map<std::string, std::string> g_macros;

static void reload_macros() {
    g_macros.clear();
    g_macroEnabled = false;
    const char* home = getenv("HOME");
    if (!home) return;
    std::string path = std::string(home) + "/UnikeyWayland/global.json";
    std::ifstream f(path);
    if (!f.is_open()) return;
    std::string json((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    
    if (json.find("\"macroEnabled\": true") != std::string::npos || json.find("\"macroEnabled\":true") != std::string::npos) {
        g_macroEnabled = true;
    }
    
    size_t macro_start = json.find("\"macros\":");
    if (macro_start != std::string::npos) {
        size_t obj_start = json.find("{", macro_start);
        size_t obj_end = json.find("}", macro_start);
        if (obj_start != std::string::npos && obj_end != std::string::npos) {
            std::string macros_str = json.substr(obj_start + 1, obj_end - obj_start - 1);
            size_t pos = 0;
            while ((pos = macros_str.find("\"", pos)) != std::string::npos) {
                size_t key_end = macros_str.find("\"", pos + 1);
                if (key_end == std::string::npos) break;
                std::string key = macros_str.substr(pos + 1, key_end - pos - 1);
                
                size_t colon = macros_str.find(":", key_end + 1);
                if (colon == std::string::npos) break;
                
                size_t val_start = macros_str.find("\"", colon + 1);
                if (val_start == std::string::npos) break;
                
                size_t val_end = macros_str.find("\"", val_start + 1);
                if (val_end == std::string::npos) break;
                
                std::string val = macros_str.substr(val_start + 1, val_end - val_start - 1);
                g_macros[key] = val;
                pos = val_end + 1;
            }
        }
    }
}

#define IBUS_TYPE_UNIKEY_ENGINE (ibus_unikey_engine_get_type ())
#define IBUS_UNIKEY_ENGINE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_UNIKEY_ENGINE, IBusUnikeyEngine))
#define IBUS_UNIKEY_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_UNIKEY_ENGINE, IBusUnikeyEngineClass))
#define IBUS_IS_UNIKEY_ENGINE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_UNIKEY_ENGINE))
#define IBUS_IS_UNIKEY_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_UNIKEY_ENGINE))

typedef struct _IBusUnikeyEngine IBusUnikeyEngine;
typedef struct _IBusUnikeyEngineClass IBusUnikeyEngineClass;

struct _IBusUnikeyEngine {
    IBusEngine parent;
    GString *preedit_string;
    std::string *composed_word;
    bool viet_mode;
    bool use_preedit;
    bool app_wants_preedit;
    bool has_surrounding_text;
    guint surrounding_cursor;
    guint surrounding_anchor;
};

struct _IBusUnikeyEngineClass {
    IBusEngineClass parent;
};

GType ibus_unikey_engine_get_type (void);

static void ibus_unikey_engine_class_init (IBusUnikeyEngineClass *klass);
static void ibus_unikey_engine_init (IBusUnikeyEngine *engine);
static void ibus_unikey_engine_destroy (IBusObject *object);
static gboolean ibus_unikey_engine_process_key_event (IBusEngine *engine, guint keyval, guint keycode, guint modifiers);
static void ibus_unikey_engine_focus_in (IBusEngine *engine);
static void ibus_unikey_engine_focus_out (IBusEngine *engine);
static void ibus_unikey_engine_reset (IBusEngine *engine);
static void ibus_unikey_engine_enable (IBusEngine *engine);
static void ibus_unikey_engine_disable (IBusEngine *engine);
static void ibus_unikey_engine_set_content_type (IBusEngine *engine, guint purpose, guint hints);
static void ibus_unikey_engine_set_surrounding_text (IBusEngine *engine, IBusText *text, guint cursor_pos, guint anchor_pos);
static void ibus_unikey_engine_property_activate (IBusEngine *engine, const gchar *prop_name, guint prop_state);

G_DEFINE_TYPE (IBusUnikeyEngine, ibus_unikey_engine, IBUS_TYPE_ENGINE)

static std::vector<std::string> preedit_apps;

static std::string get_process_name(guint pid) {
    if (pid == 0) return "";
    char path[128];
    snprintf(path, sizeof(path), "/proc/%u/comm", pid);
    FILE *f = fopen(path, "r");
    if (!f) return "";
    char name[256];
    if (fgets(name, sizeof(name), f)) {
        size_t len = strlen(name);
        if (len > 0 && name[len-1] == '\n') {
            name[len-1] = '\0';
        }
        fclose(f);
        return std::string(name);
    }
    fclose(f);
    return "";
}

static std::string get_process_exe(guint pid) {
    if (pid == 0) return "";
    char path[128];
    snprintf(path, sizeof(path), "/proc/%u/exe", pid);
    char exe[512];
    ssize_t len = readlink(path, exe, sizeof(exe) - 1);
    if (len != -1) {
        exe[len] = '\0';
        char *base = strrchr(exe, '/');
        return base ? (base + 1) : exe;
    }
    return "";
}

static std::vector<std::string> load_preedit_apps() {
    std::vector<std::string> apps;
    apps.push_back("kitty");
    apps.push_back("alacritty");
    apps.push_back("konsole");
    apps.push_back("gnome-terminal");
    apps.push_back("xfce4-terminal");
    apps.push_back("lxterminal");
    apps.push_back("studio");
    apps.push_back("java");
    apps.push_back("google-chrome");
    apps.push_back("chrome");
    apps.push_back("chromium");
    apps.push_back("firefox");
    apps.push_back("code");
    apps.push_back("electron");
    apps.push_back("obsidian");

    const char* home = getenv("HOME");
    if (home) {
        std::string config_path = std::string(home) + "/UnikeyWayland/preedit_apps.txt";
        std::ifstream f(config_path);
        if (f.is_open()) {
            std::string line;
            while (std::getline(f, line)) {
                if (!line.empty() && line[0] != '#') {
                    apps.push_back(line);
                }
            }
        }
    }
    return apps;
}

static guint get_pid_of_dbus_name(const gchar *name) {
    if (!name || name[0] == '\0') return 0;
    GError *error = NULL;
    GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (!conn) {
        g_warning("Failed to get session bus: %s", error->message);
        g_clear_error(&error);
        return 0;
    }
    
    GVariant *result = g_dbus_connection_call_sync(
        conn,
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        "GetConnectionUnixProcessID",
        g_variant_new("(s)", name),
        G_VARIANT_TYPE("(u)"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );
    
    g_object_unref(conn);
    
    if (!result) {
        g_clear_error(&error);
        return 0;
    }
    
    guint pid = 0;
    g_variant_get(result, "(u)", &pid);
    g_variant_unref(result);
    return pid;
}

static void ibus_unikey_engine_focus_in_id (IBusEngine *engine_base, const gchar *client_id, const gchar *detail) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (engine_base);
    ibus_unikey_engine_reset(engine_base);
    
    if (!client_id || client_id[0] == '\0') {
        engine->app_wants_preedit = false;
        engine->use_preedit = false;
        return;
    }

    guint pid = get_pid_of_dbus_name(client_id);
    std::string comm = get_process_name(pid);
    std::string exe = get_process_exe(pid);

    engine->app_wants_preedit = false;

    if (preedit_apps.empty()) {
        preedit_apps = load_preedit_apps();
    }

    for (const auto& app : preedit_apps) {
        if ((!comm.empty() && comm.find(app) != std::string::npos) ||
            (!exe.empty() && exe.find(app) != std::string::npos)) {
            engine->app_wants_preedit = true;
            break;
        }
    }
    
    // Set use_preedit here as well
    engine->use_preedit = engine->app_wants_preedit;
}

static void ibus_unikey_engine_class_init (IBusUnikeyEngineClass *klass) {
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

    ibus_object_class->destroy = ibus_unikey_engine_destroy;

    engine_class->process_key_event = ibus_unikey_engine_process_key_event;
    engine_class->focus_in = ibus_unikey_engine_focus_in;
    engine_class->focus_out = ibus_unikey_engine_focus_out;
    engine_class->reset = ibus_unikey_engine_reset;
    engine_class->enable = ibus_unikey_engine_enable;
    engine_class->disable = ibus_unikey_engine_disable;
    engine_class->set_content_type = ibus_unikey_engine_set_content_type;
    engine_class->set_surrounding_text = ibus_unikey_engine_set_surrounding_text;
    engine_class->property_activate = ibus_unikey_engine_property_activate;
    
    // Đăng ký callback nhận diện ứng dụng
    // @ts-ignore
    engine_class->focus_in_id = ibus_unikey_engine_focus_in_id;
}

static void ibus_unikey_engine_init (IBusUnikeyEngine *engine) {
    Bamboo_Init();
    engine->viet_mode = true;
    engine->preedit_string = g_string_new("");
    engine->composed_word = new std::string();
    engine->use_preedit = false; 
    engine->app_wants_preedit = false;
    engine->has_surrounding_text = false;
    engine->surrounding_cursor = 0;
    engine->surrounding_anchor = 0;
}

static void ibus_unikey_engine_destroy (IBusObject *object) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (object);
    if (engine->preedit_string) {
        g_string_free(engine->preedit_string, TRUE);
        engine->preedit_string = nullptr;
    }
    if (engine->composed_word) {
        delete engine->composed_word;
        engine->composed_word = nullptr;
    }
    IBUS_OBJECT_CLASS (ibus_unikey_engine_parent_class)->destroy (object);
}

static gboolean ibus_unikey_engine_process_key_event (IBusEngine *engine_base, guint keyval, guint keycode, guint modifiers) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (engine_base);
    g_printerr("KEYVAL: %d, char: %c, use_preedit: %d, has_sur: %d, char_backs: %d\n", keyval, keyval < 128 ? keyval : '?', engine->use_preedit, engine->has_surrounding_text, 0);
    
    // Bỏ qua khi nhả phím (key release)
    if (modifiers & IBUS_RELEASE_MASK) {
        return FALSE;
    }
    
    // Bỏ qua các modifier key (Ctrl, Alt, Super, nhưng cho qua Shift)
    if (modifiers & (IBUS_CONTROL_MASK | IBUS_MOD1_MASK | IBUS_SUPER_MASK)) {
        // Reset composed state khi bấm Ctrl/Alt/Super
        *(engine->composed_word) = "";
        Bamboo_Reset();
        if (engine->use_preedit && engine->preedit_string->len > 0) {
            g_string_truncate(engine->preedit_string, 0);
            ibus_engine_hide_preedit_text(engine_base);
        }
        return FALSE;
    }

    // Toggle Vietnamese/English mode với phím mà bạn muốn (ví dụ Super+Space)
    // Hiện tại không handle toggle ở đây vì đã có qua property/menu

    if (!engine->viet_mode) {
        *(engine->composed_word) = "";
        Bamboo_Reset();
        return FALSE;
    }

    // Chỉ xử lý ASCII printable, BackSpace
    char c = 0;
    if (keyval == IBUS_KEY_BackSpace) {
        c = '\b';
    } else if (keyval == IBUS_KEY_Return || keyval == IBUS_KEY_KP_Enter) {
        c = '\n';
    } else if (keyval >= IBUS_KEY_space && keyval <= IBUS_KEY_asciitilde) {
        c = (char)keyval;
    } else {
        // Phím chức năng (Arrow, Esc, F1..., ...): commit preedit nếu có rồi trả phím
        if (engine->use_preedit && engine->preedit_string->len > 0) {
            IBusText *text = ibus_text_new_from_string(engine->preedit_string->str);
            ibus_engine_hide_preedit_text(engine_base);
            g_string_truncate(engine->preedit_string, 0);
            ibus_engine_commit_text(engine_base, text);
            *(engine->composed_word) = "";
            Bamboo_Reset();
        } else {
            std::string current_word = *(engine->composed_word);
            if (!current_word.empty()) {
                if (g_macroEnabled && g_macros.find(current_word) != g_macros.end()) {
                    std::string macro_val = g_macros[current_word];
                    int char_backs = g_utf8_strlen(current_word.c_str(), -1);
                    if (char_backs > 0) {
                        ibus_engine_delete_surrounding_text(engine_base, -char_backs, char_backs);
                    }
                    IBusText *text = ibus_text_new_from_string(macro_val.c_str());
                    ibus_engine_commit_text(engine_base, text);
                }
            }
            *(engine->composed_word) = "";
            Bamboo_Reset();
        }
        return FALSE;
    }

    // c != 0 từ đây trở xuống
    std::string old_composed = *(engine->composed_word);

    if (engine->use_preedit) {
        // ====== PREEDIT MODE (terminal, Android Studio, v.v.) ======
        if (c == '\b') {
            // Kiểm tra preedit có rỗng không
            char *cur_preedit = Bamboo_GetPreeditString();
            bool was_empty = (!cur_preedit || cur_preedit[0] == '\0');
            if (cur_preedit) free(cur_preedit);
            
            if (was_empty) {
                return FALSE; // Cho phép backspace thật đi qua
            }
            Bamboo_RemoveLastChar();
        } else if (!Bamboo_CanProcessKey(c)) {
            // Không phải phím Bamboo xử lý được: commit preedit rồi trả phím
            char *commit_str = Bamboo_GetCommitString();
            if (commit_str && commit_str[0] != '\0') {
                IBusText *text = ibus_text_new_from_string(commit_str);
                ibus_engine_commit_text(engine_base, text);
            }
            if (commit_str) free(commit_str);
            ibus_engine_hide_preedit_text(engine_base);
            g_string_truncate(engine->preedit_string, 0);
            *(engine->composed_word) = "";
            Bamboo_Reset();
            return FALSE; // Cho phím gốc đi qua
        } else {
            Bamboo_ProcessKey(c);
        }

        char *preedit_str = Bamboo_GetPreeditString();
        uint32_t byte_len = preedit_str ? strlen(preedit_str) : 0;
        
        if (byte_len == 0) {
            // Bamboo đã commit (từ hoàn chỉnh)
            ibus_engine_hide_preedit_text(engine_base);
            g_string_truncate(engine->preedit_string, 0);
            char *commit = Bamboo_GetCommitString();
            if (commit && commit[0] != '\0') {
                IBusText *text = ibus_text_new_from_string(commit);
                ibus_engine_commit_text(engine_base, text);
            }
            if (commit) free(commit);
            *(engine->composed_word) = "";
        } else {
            g_string_assign(engine->preedit_string, preedit_str);
            *(engine->composed_word) = preedit_str;
            IBusText *text = ibus_text_new_from_string(preedit_str);
            // Thêm underline styling
            ibus_text_append_attribute(text, IBUS_ATTR_TYPE_UNDERLINE, IBUS_ATTR_UNDERLINE_SINGLE, 0, -1);
            
            // LỖI "NHẢY" CURSOR LÀ Ở ĐÂY: IBus cần số lượng ký tự (character count), không phải byte count!
            uint32_t char_len = g_utf8_strlen(preedit_str, -1);
            ibus_engine_update_preedit_text(engine_base, text, char_len, TRUE);
        }
        if (preedit_str) free(preedit_str);
        return TRUE;

    } else {
        // ====== NORMAL MODE (Chrome, GTK, Qt apps) - Diffing ======
        if (c == '\b') {
            if (engine->composed_word->empty()) {
                return FALSE; // Backspace thật khi không có gì
            }
            Bamboo_RemoveLastChar();
        } else if (!Bamboo_CanProcessKey(c)) {
            std::string current_word = *(engine->composed_word);
            *(engine->composed_word) = "";
            Bamboo_Reset();

            if (g_macroEnabled && g_macros.find(current_word) != g_macros.end()) {
                std::string macro_val = g_macros[current_word];
                int char_backs = g_utf8_strlen(current_word.c_str(), -1);
                if (char_backs > 0) {
                    ibus_engine_delete_surrounding_text(engine_base, -char_backs, char_backs);
                }
                IBusText *text = ibus_text_new_from_string(macro_val.c_str());
                ibus_engine_commit_text(engine_base, text);
            }
            return FALSE;
        } else {
            Bamboo_ProcessKey(c);
        }

        // Lấy trạng thái preedit hiện tại của Bamboo
        char *preedit_ptr = Bamboo_GetPreeditString();
        std::string new_composed = preedit_ptr ? preedit_ptr : "";
        if (preedit_ptr) free(preedit_ptr);

        // Diffing đơn giản và rollback về đầu ký tự UTF-8
        size_t common_bytes = 0;
        while (common_bytes < old_composed.length() && common_bytes < new_composed.length() &&
               old_composed[common_bytes] == new_composed[common_bytes]) {
            common_bytes++;
        }
        
        while (common_bytes > 0 && (old_composed[common_bytes] & 0xC0) == 0x80) {
            common_bytes--;
        }
        
        int char_backs = 0;
        if (common_bytes < old_composed.length()) {
            char_backs = g_utf8_strlen(old_composed.c_str() + common_bytes, -1);
        }
        
        if (char_backs > 0) {
            // Trên X11, delete_surrounding_text hoạt động tốt trên Chrome.
            // Bug Omnibox bôi đen chỉ xảy ra trên Native Wayland, X11 dùng Backspace sẽ gây race condition!
            ibus_engine_delete_surrounding_text(engine_base, -char_backs, char_backs);
        }

        // Commit phần suffix mới
        std::string suffix = new_composed.substr(common_bytes);
        if (!suffix.empty()) {
            IBusText *text = ibus_text_new_from_string(suffix.c_str());
            ibus_engine_commit_text(engine_base, text);
        }

        *(engine->composed_word) = new_composed;
        return TRUE;
    }
}

static void ibus_unikey_engine_focus_in (IBusEngine *engine) {
    ibus_unikey_engine_reset(engine);
    reload_macros();
    
    IBusPropList *prop_list = ibus_prop_list_new ();
    IBusText *label_setup = ibus_text_new_from_string ("Cấu hình Unikey...");
    IBusProperty *prop_setup = ibus_property_new ("Setup",
                                                  PROP_TYPE_NORMAL,
                                                  label_setup,
                                                  "preferences-system", // icon
                                                  NULL, // tooltip
                                                  TRUE, // sensitive
                                                  TRUE, // visible
                                                  PROP_STATE_UNCHECKED,
                                                  NULL);
    ibus_prop_list_append (prop_list, prop_setup);
    ibus_engine_register_properties (engine, prop_list);
}

static void ibus_unikey_engine_property_activate (IBusEngine *engine, const gchar *prop_name, guint prop_state) {
    if (g_strcmp0 (prop_name, "Setup") == 0) {
        system("/usr/local/bin/unikey-wayland --setup &");
    }
}

static void ibus_unikey_engine_focus_out (IBusEngine *engine) {
    ibus_unikey_engine_reset(engine);
}

static void ibus_unikey_engine_reset (IBusEngine *engine_base) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (engine_base);
    engine->composed_word->clear();
    Bamboo_Reset();
    if (engine->preedit_string->len > 0) {
        g_string_truncate(engine->preedit_string, 0);
        ibus_engine_hide_preedit_text(engine_base);
    }
}

static void ibus_unikey_engine_enable (IBusEngine *engine) {}
static void ibus_unikey_engine_disable (IBusEngine *engine) {}

static void ibus_unikey_engine_set_content_type (IBusEngine *engine_base, guint purpose, guint hints) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (engine_base);
    engine->use_preedit = engine->app_wants_preedit || (purpose == IBUS_INPUT_PURPOSE_TERMINAL);
}

static void ibus_unikey_engine_set_surrounding_text (IBusEngine *engine_base, IBusText *text, guint cursor_pos, guint anchor_pos) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (engine_base);
    engine->has_surrounding_text = (text != NULL && text->text != NULL && text->text[0] != '\0');
    engine->surrounding_cursor = cursor_pos;
    engine->surrounding_anchor = anchor_pos;
}

static void ibus_disconnected_cb (IBusBus *bus, gpointer user_data) {
    ibus_quit ();
}

int main (int argc, char **argv) {
    ibus_init ();

    IBusBus *bus = ibus_bus_new ();
    g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);

    IBusFactory *factory = ibus_factory_new (ibus_bus_get_connection (bus));
    ibus_factory_add_engine (factory, "unikey-wayland", IBUS_TYPE_UNIKEY_ENGINE);

    if (ibus_bus_request_name (bus, "org.freedesktop.IBus.UnikeyWayland", 0) == 0) {
        g_printerr ("Failed to request D-Bus name.\n");
        return 1;
    }

    ibus_main ();
    return 0;
}
