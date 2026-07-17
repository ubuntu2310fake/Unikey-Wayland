#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>

int error_handler(Display *d, XErrorEvent *e) {
    // Ignore BadAccess
    return 0;
}

int main() {
    Display* d = XOpenDisplay(NULL);
    if (!d) return 1;
    Window root = DefaultRootWindow(d);
    
    XSetErrorHandler(error_handler);
    
    KeySym keys[] = { XK_a, XK_b, XK_c, XK_Escape };
    unsigned int mods[] = { 0, ShiftMask };
    
    for (int i = 0; i < 4; i++) {
        KeyCode kc = XKeysymToKeycode(d, keys[i]);
        if (kc == 0) continue;
        for (int j = 0; j < 2; j++) {
            XGrabKey(d, kc, mods[j], root, True, GrabModeSync, GrabModeAsync);
        }
    }
    
    XSync(d, False);
    printf("Grabbed passively. Type 'a' to swallow, Esc to exit.\n");
    fflush(stdout);
    
    XEvent ev;
    while (1) {
        XNextEvent(d, &ev);
        if (ev.type == KeyPress || ev.type == KeyRelease) {
            KeySym ks = XLookupKeysym(&ev.xkey, 0);
            if (ev.type == KeyPress && ks == XK_a) {
                printf("Swallowed 'a'!\n");
                fflush(stdout);
                XAllowEvents(d, AsyncKeyboard, ev.xkey.time);
            } else if (ev.type == KeyPress && ks == XK_Escape) {
                printf("Escaped!\n");
                fflush(stdout);
                XAllowEvents(d, ReplayKeyboard, ev.xkey.time);
                break;
            } else {
                XAllowEvents(d, ReplayKeyboard, ev.xkey.time);
            }
            XFlush(d);
        }
    }
    
    XCloseDisplay(d);
    return 0;
}
