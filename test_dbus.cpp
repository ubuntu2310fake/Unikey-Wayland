#include <gio/gio.h>
#include <iostream>

static guint get_pid_of_dbus_name(const gchar *name) {
    if (!name || name[0] == '\0') return 0;
    GError *error = NULL;
    GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (!conn) return 0;
    
    GVariant *result = g_dbus_connection_call_sync(
        conn, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
        "GetConnectionUnixProcessID", g_variant_new("(s)", name), G_VARIANT_TYPE("(u)"),
        G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error
    );
    g_object_unref(conn);
    if (!result) return 0;
    
    guint pid = 0;
    g_variant_get(result, "(u)", &pid);
    g_variant_unref(result);
    return pid;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        std::cout << get_pid_of_dbus_name(argv[1]) << std::endl;
    }
    return 0;
}
