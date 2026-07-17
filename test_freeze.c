#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>

int main() {
    Display* d = XOpenDisplay(NULL);
    if (!d) return 1;
    Window root = DefaultRootWindow(d);
    
    KeyCode kc_a = XKeysymToKeycode(d, XK_a);
    XGrabKey(d, kc_a, 0, root, True, GrabModeAsync, GrabModeSync);
    
    printf("Grabbed 'a'. Press 'a'.\n");
    fflush(stdout);
    
    XEvent ev;
    while (1) {
        XNextEvent(d, &ev);
        if (ev.type == KeyPress) {
            printf("KeyPress!\n");
            fflush(stdout);
            XAllowEvents(d, ReplayKeyboard, ev.xkey.time);
            XFlush(d);
        } else if (ev.type == KeyRelease) {
            printf("KeyRelease!\n");
            fflush(stdout);
            XAllowEvents(d, ReplayKeyboard, ev.xkey.time);
            XFlush(d);
        }
    }
    return 0;
}
