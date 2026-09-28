#include <ibus.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>

static void send_to_daemon(const std::string& cmd) {
    int fd = open("/tmp/ukw_cmd", O_WRONLY | O_NONBLOCK);
    if (fd >= 0) {
        write(fd, cmd.c_str(), cmd.length());
        close(fd);
    }
}

typedef struct _IBusUnikeyEngine IBusUnikeyEngine;
typedef struct _IBusUnikeyEngineClass IBusUnikeyEngineClass;

struct _IBusUnikeyEngine { IBusEngine parent; };
struct _IBusUnikeyEngineClass { IBusEngineClass parent; };

#define IBUS_TYPE_UNIKEY_ENGINE (ibus_unikey_engine_get_type())
GType ibus_unikey_engine_get_type(void);

G_DEFINE_TYPE(IBusUnikeyEngine, ibus_unikey_engine, IBUS_TYPE_ENGINE)

static void ibus_unikey_engine_init(IBusUnikeyEngine *engine) {}
static void ibus_unikey_engine_focus_in(IBusEngine *engine) {
    send_to_daemon("MODE:VI;");
}
static void ibus_unikey_engine_focus_out(IBusEngine *engine) {
    send_to_daemon("MODE:EN;");
}
static gboolean ibus_unikey_engine_process_key_event(IBusEngine *engine, guint keyval, guint keycode, guint modifiers) {
    return FALSE; // Always pass through to OS/Ring0
}

static void ibus_unikey_engine_class_init(IBusUnikeyEngineClass *klass) {
    IBusEngineClass *engine_class = IBUS_ENGINE_CLASS(klass);
    engine_class->process_key_event = ibus_unikey_engine_process_key_event;
    engine_class->focus_in = ibus_unikey_engine_focus_in;
    engine_class->focus_out = ibus_unikey_engine_focus_out;
}

static void ibus_disconnected_cb(IBusBus *bus, gpointer user_data) {
    ibus_quit();
}

int main(int argc, char **argv) {
    ibus_init();
    IBusBus *bus = ibus_bus_new();
    g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnected_cb), NULL);
    
    IBusFactory *factory = ibus_factory_new(ibus_bus_get_connection(bus));
    ibus_factory_add_engine(factory, "unikey-wayland", IBUS_TYPE_UNIKEY_ENGINE);
    
    if (argc > 1 && g_strcmp0(argv[1], "--ibus") == 0) {
        ibus_bus_request_name(bus, "org.freedesktop.IBus.UnikeyWayland", 0);
    } else {
        IBusComponent *component = ibus_component_new("org.freedesktop.IBus.UnikeyWayland", "Unikey Wayland", "1.0", "GPL", "Trương Hiếu", "https://github.com/ubuntu2310fake/Unikey-Wayland", "", "ibus-unikey");
        IBusEngineDesc *engine_desc = ibus_engine_desc_new("unikey-wayland", "Unikey (Wayland Engine)", "Unikey Input Method Engine", "vi", "GPL", "Trương Hiếu", "ibus-unikey", "ukw");
        ibus_component_add_engine(component, engine_desc);
        ibus_bus_register_component(bus, component);
    }
    
    ibus_main();
    return 0;
}
