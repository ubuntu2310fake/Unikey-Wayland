#include <ibus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gio/gio.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>
#include "../../wayland-client/include/ukengine_wrapper.h"
#include "../../keyhook/keyhook.h"

#define IBUS_TYPE_UNIKEY_ENGINE (ibus_unikey_engine_get_type ())

typedef struct _IBusUnikeyEngine IBusUnikeyEngine;
typedef struct _IBusUnikeyEngineClass IBusUnikeyEngineClass;

#define IBUS_UNIKEY_ENGINE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), IBUS_TYPE_UNIKEY_ENGINE, IBusUnikeyEngine))
#define IBUS_UNIKEY_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), IBUS_TYPE_UNIKEY_ENGINE, IBusUnikeyEngineClass))

struct _IBusUnikeyEngine {
    IBusEngine parent;
    UkEngineWrapper* ukengine;
    GString* preedit_string;
    bool use_preedit;
    bool app_wants_preedit;
};

struct _IBusUnikeyEngineClass {
    IBusEngineClass parent;
};

GType ibus_unikey_engine_get_type (void);

G_DEFINE_TYPE (IBusUnikeyEngine, ibus_unikey_engine, IBUS_TYPE_ENGINE)

static void ibus_unikey_engine_init (IBusUnikeyEngine *engine) {
    engine->ukengine = new UkEngineWrapper();
    engine->ukengine->init();
    // Bật tiếng Việt mặc định
    engine->ukengine->setVietMode(true);
    engine->preedit_string = g_string_new("");
    engine->use_preedit = false; // Default: false (Fake backspace for most apps)
    engine->app_wants_preedit = false;

}

static void ibus_unikey_engine_destroy (IBusObject *object) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (object);
    if (engine->ukengine) {
        delete engine->ukengine;
        engine->ukengine = nullptr;
    }
    if (engine->preedit_string) {
        g_string_free(engine->preedit_string, TRUE);
        engine->preedit_string = nullptr;
    }
    IBUS_OBJECT_CLASS (ibus_unikey_engine_parent_class)->destroy (object);
}

static gboolean ibus_unikey_engine_process_key_event (IBusEngine *engine_base, guint keyval, guint keycode, guint modifiers) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (engine_base);

    // Bỏ qua key release
    if (modifiers & IBUS_RELEASE_MASK) {
        return FALSE;
    }
    
    if (!engine->ukengine->getVietMode()) {
        engine->ukengine->reset();
        return FALSE; 
    }
    
    // Bỏ qua nếu có modifier đặc biệt (Ctrl, Alt, Super)
    if (modifiers & (IBUS_CONTROL_MASK | IBUS_MOD1_MASK | IBUS_SUPER_MASK)) {
        engine->ukengine->reset();
        return FALSE;
    }

    unsigned char c = 0;
    if (keyval >= 0x20 && keyval <= 0x7e) {
        c = (unsigned char)keyval;
    } else if (keyval == IBUS_KEY_BackSpace) {
        c = '\b';
    } else if (keyval == IBUS_KEY_Return || keyval == IBUS_KEY_KP_Enter) {
        c = '\r';
    } else if (keyval == IBUS_KEY_space || keyval == IBUS_KEY_KP_Space) {
        c = ' ';
    } else {
        engine->ukengine->reset();
        if (engine->preedit_string->len > 0) {
            IBusText *text = ibus_text_new_from_string(engine->preedit_string->str);
            ibus_engine_commit_text(engine_base, text);
            g_string_truncate(engine->preedit_string, 0);
            ibus_engine_hide_preedit_text(engine_base);
        }
        return FALSE; // Không xử lý phím lạ
    }

    int backs = 0;
    std::string processed = engine->ukengine->processKey(c, backs);

    bool use_preedit = engine->use_preedit;

    if (use_preedit) {
        if (c == '\b' && backs == 1 && processed.empty()) {
            glong len = g_utf8_strlen(engine->preedit_string->str, -1);
            if (len > 0) {
                gchar *new_str = g_utf8_substring(engine->preedit_string->str, 0, len - 1);
                g_string_assign(engine->preedit_string, new_str);
                g_free(new_str);
            } else {
                return FALSE;
            }
        } else if (c != '\n') {
            if (backs > 0) {
                glong len = g_utf8_strlen(engine->preedit_string->str, -1);
                if (len >= backs) {
                    gchar *new_str = g_utf8_substring(engine->preedit_string->str, 0, len - backs);
                    g_string_assign(engine->preedit_string, new_str);
                    g_free(new_str);
                } else {
                    g_string_truncate(engine->preedit_string, 0);
                }
            }
            if (!processed.empty()) {
                g_string_append(engine->preedit_string, processed.c_str());
            }
        }

        if (!engine->ukengine->isComposing() || c == ' ' || c == '\n') {
            if (engine->preedit_string->len > 0) {
                IBusText *text = ibus_text_new_from_string(engine->preedit_string->str);
                ibus_engine_commit_text(engine_base, text);
                g_string_truncate(engine->preedit_string, 0);
                ibus_engine_hide_preedit_text(engine_base);
            } else if (backs == 0 && processed.empty()) {
                return FALSE;
            }
            if (c == '\n') {
                return FALSE;
            }
            return TRUE;
        } else {
            IBusText *text = ibus_text_new_from_string(engine->preedit_string->str);
            ibus_text_append_attribute(text, IBUS_ATTR_TYPE_UNDERLINE, IBUS_ATTR_UNDERLINE_SINGLE, 0, g_utf8_strlen(engine->preedit_string->str, -1));
            ibus_engine_update_preedit_text(engine_base, text, g_utf8_strlen(engine->preedit_string->str, -1), TRUE);
            return TRUE;
        }
    } else {
        // Fake backspace
        if (backs == 0 && processed.empty()) {
            return FALSE;
        }
        if (backs > 0) {
            ibus_engine_delete_surrounding_text(engine_base, -backs, backs);
        }
        if (!processed.empty()) {
            IBusText *text = ibus_text_new_from_string(processed.c_str());
            ibus_engine_commit_text(engine_base, text);
        }
        return TRUE;
    }
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
        g_warning("Failed to get process ID for %s: %s", name, error->message);
        g_clear_error(&error);
        return 0;
    }
    
    guint pid = 0;
    g_variant_get(result, "(u)", &pid);
    g_variant_unref(result);
    return pid;
}

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
    // Default apps
    apps.push_back("kitty");
    apps.push_back("alacritty");
    apps.push_back("konsole");
    apps.push_back("gnome-terminal");
    apps.push_back("xfce4-terminal");
    apps.push_back("lxterminal");
    apps.push_back("studio");
    apps.push_back("java");
    
    const char* home = getenv("HOME");
    if (home) {
        std::string config_path = std::string(home) + "/UnikeyWayland/preedit_apps.txt";
        std::ifstream f(config_path);
        if (f.is_open()) {
            apps.clear();
            std::string line;
            while (std::getline(f, line)) {
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                if (!line.empty() && line[0] != '#') {
                    std::transform(line.begin(), line.end(), line.begin(), ::tolower);
                    apps.push_back(line);
                }
            }
        }
    }
    return apps;
}

static void ibus_unikey_engine_focus_in (IBusEngine *engine) {
    IBusPropList *prop_list = ibus_prop_list_new ();

    // Fallback: check active window via xprop
    FILE *fp = popen("xprop -root _NET_ACTIVE_WINDOW 2>/dev/null", "r");
    if (fp) {
        char buf[256];
        if (fgets(buf, sizeof(buf), fp)) {
            unsigned long wid = 0;
            if (sscanf(buf, "_NET_ACTIVE_WINDOW(WINDOW): window id # 0x%lx", &wid) == 1 && wid > 0) {
                char cmd[256];
                snprintf(cmd, sizeof(cmd), "xprop -id 0x%lx WM_CLASS 2>/dev/null", wid);
                FILE *fp2 = popen(cmd, "r");
                if (fp2) {
                    char buf2[512];
                    if (fgets(buf2, sizeof(buf2), fp2)) {
                        char class_name[256] = {0};
                        if (sscanf(buf2, "WM_CLASS(STRING) = \"%*[^\"]\", \"%[^\"]\"", class_name) == 1 ||
                            sscanf(buf2, "WM_CLASS(STRING) = \"%[^\"]\"", class_name) == 1) {
                            std::string wmClass = class_name;
                            std::transform(wmClass.begin(), wmClass.end(), wmClass.begin(), ::tolower);
                            
                            IBusUnikeyEngine* e = (IBusUnikeyEngine*)engine;
                            std::vector<std::string> preedit_apps = load_preedit_apps();
                            e->app_wants_preedit = false;
                            for (const auto& app : preedit_apps) {
                                if (wmClass == app) {
                                    e->app_wants_preedit = true;
                                    break;
                                }
                            }
                        }
                    }
                    pclose(fp2);
                }
            }
        }
        pclose(fp);
    }

    const char* session_type = getenv("XDG_SESSION_TYPE");
    const char* version_str = "Unikey (GNOME Edition)";
    if (session_type && g_ascii_strcasecmp(session_type, "x11") == 0) {
        version_str = "Unikey (X11 Edition)";
    } else if (session_type && g_ascii_strcasecmp(session_type, "wayland") == 0) {
        version_str = "Unikey (Wayland Edition)";
    }

    IBusText *label_version = ibus_text_new_from_string (version_str);
    IBusProperty *prop_version = ibus_property_new ("Version",
                                                  PROP_TYPE_NORMAL,
                                                  label_version,
                                                  NULL, // icon
                                                  NULL, // tooltip
                                                  FALSE, // sensitive (disabled so it acts as a label)
                                                  TRUE, // visible
                                                  PROP_STATE_UNCHECKED,
                                                  NULL);
    ibus_prop_list_append (prop_list, prop_version);

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

    IBusText *label_exclude = ibus_text_new_from_string ("Danh sách loại trừ...");
    IBusProperty *prop_exclude = ibus_property_new ("Exclude",
                                                  PROP_TYPE_NORMAL,
                                                  label_exclude,
                                                  "list-remove", // icon
                                                  NULL, // tooltip
                                                  TRUE, // sensitive
                                                  TRUE, // visible
                                                  PROP_STATE_UNCHECKED,
                                                  NULL);
    ibus_prop_list_append (prop_list, prop_exclude);

    ibus_engine_register_properties (engine, prop_list);
}

static void ibus_unikey_engine_property_activate (IBusEngine *engine, const gchar *prop_name, guint prop_state) {
    if (g_strcmp0 (prop_name, "Setup") == 0) {
        system("/usr/local/bin/unikey-wayland --setup &");
    } else if (g_strcmp0 (prop_name, "Exclude") == 0) {
        system("/usr/local/bin/unikey-wayland --exclude &");
    }
}

static void ibus_unikey_engine_focus_in_id (IBusEngine *engine_base, const gchar *object_path, const gchar *client) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (engine_base);

    if (IBUS_ENGINE_CLASS (ibus_unikey_engine_parent_class)->focus_in_id) {
        IBUS_ENGINE_CLASS (ibus_unikey_engine_parent_class)->focus_in_id (engine_base, object_path, client);
    }

    guint pid = get_pid_of_dbus_name(client);
    std::string comm = get_process_name(pid);
    std::string exe = get_process_exe(pid);

    FILE* dbgf = fopen("/tmp/uk-debug.log", "a");
    if (dbgf) {
        fprintf(dbgf, "focus_in_id: object_path=%s, client=%s, pid=%u, comm=%s, exe=%s\n", object_path ? object_path : "(null)", client ? client : "(null)", pid, comm.c_str(), exe.c_str());
        fclose(dbgf);
    }

    std::vector<std::string> preedit_apps = load_preedit_apps();
    engine->app_wants_preedit = false;

    
    // Check by DBus process name first
    for (const auto& app : preedit_apps) {
        if (comm == app || exe == app) {
            engine->app_wants_preedit = true;
            break;
        }
    }
    
    // Fallback: If behind a bridge, check actual active window using xprop
    if (!engine->app_wants_preedit && (comm == "ibus-x11" || comm == "ibus-daemon" || comm.find("ibus-") == 0 || comm.empty())) {
        FILE *f1 = popen("xprop -root _NET_ACTIVE_WINDOW 2>/dev/null", "r");
        if (f1) {
            char line1[256];
            std::string window_id;
            if (fgets(line1, sizeof(line1), f1)) {
                char *hash = strchr(line1, '#');
                if (hash) {
                    window_id = std::string(hash + 2);
                    window_id.erase(window_id.find_last_not_of(" \n\r\t") + 1);
                }
            }
            pclose(f1);
            
            if (!window_id.empty()) {
                std::string cmd = "xprop -id " + window_id + " WM_CLASS 2>/dev/null";
                FILE *f2 = popen(cmd.c_str(), "r");
                if (f2) {
                    char line2[512];
                    if (fgets(line2, sizeof(line2), f2)) {
                        std::string wm_class(line2);
                        std::transform(wm_class.begin(), wm_class.end(), wm_class.begin(), ::tolower);
                        for (const auto& app : preedit_apps) {
                            std::string app_lower = app;
                            std::transform(app_lower.begin(), app_lower.end(), app_lower.begin(), ::tolower);
                            if (wm_class.find(app_lower) != std::string::npos) {
                                engine->app_wants_preedit = true;
                                break;
                            }
                        }
                    }
                    pclose(f2);
                }
            }
        }
    }
    
    // Update preedit setting immediately
    engine->use_preedit = engine->app_wants_preedit;
}

static void ibus_unikey_engine_set_content_type (IBusEngine *engine_base, guint purpose, guint hints) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (engine_base);
    engine->use_preedit = engine->app_wants_preedit || (purpose == IBUS_INPUT_PURPOSE_TERMINAL);
}

static void ibus_unikey_engine_reset (IBusEngine *engine_base) {
    IBusUnikeyEngine *engine = IBUS_UNIKEY_ENGINE (engine_base);
    UkEngineWrapper* e = (UkEngineWrapper*) engine->ukengine;
    e->reset();
    if (engine->preedit_string->len > 0) {
        g_string_truncate(engine->preedit_string, 0);
        ibus_engine_hide_preedit_text(engine_base);
    }
}

static void ibus_unikey_engine_class_init (IBusUnikeyEngineClass *klass) {
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (klass);
    object_class->destroy = ibus_unikey_engine_destroy;

    IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);
    engine_class->process_key_event = ibus_unikey_engine_process_key_event;
    engine_class->focus_in = ibus_unikey_engine_focus_in;
    engine_class->property_activate = ibus_unikey_engine_property_activate;
    engine_class->set_content_type = ibus_unikey_engine_set_content_type;
    engine_class->reset = ibus_unikey_engine_reset;
    engine_class->focus_in_id = ibus_unikey_engine_focus_in_id;
}

int main (int argc, char **argv) {
    ibus_init ();

    IBusBus *bus = ibus_bus_new ();
    g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_quit), NULL);

    IBusFactory *factory = ibus_factory_new (ibus_bus_get_connection (bus));
    ibus_factory_add_engine (factory, "unikey-wayland", IBUS_TYPE_UNIKEY_ENGINE);

    if (argc > 1 && strcmp(argv[1], "--ibus") == 0) {
        ibus_bus_request_name (bus, "org.freedesktop.IBus.UnikeyWayland", 0);
    }

    ibus_main ();
    return 0;
}
