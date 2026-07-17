#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    Display* d = XOpenDisplay(NULL);
    if (!d) return 1;
    Window root = DefaultRootWindow(d);
    
    KeyCode kc_a = XKeysymToKeycode(d, XK_a);
    KeyCode kc_b = XKeysymToKeycode(d, XK_b);
    
    // Grab both 'a' and 'b'
    XGrabKey(d, kc_a, 0, root, True, GrabModeAsync, GrabModeSync);
    XGrabKey(d, kc_b, 0, root, True, GrabModeAsync, GrabModeSync);
    
    printf("Grabbed 'a' and 'b'. Press 'a' to inject 'b'.\n");
    fflush(stdout);
    
    XEvent ev;
    while (1) {
        XNextEvent(d, &ev);
        if (ev.type == KeyPress) {
            KeySym ks = XLookupKeysym(&ev.xkey, 0);
            if (ks == XK_a) {
                printf("Pressed 'a'. Swallowing and injecting 'b'...\n");
                XAllowEvents(d, AsyncKeyboard, ev.xkey.time);
                XTestFakeKeyEvent(d, kc_b, True, CurrentTime);
                XTestFakeKeyEvent(d, kc_b, False, CurrentTime);
                XFlush(d);
            } else if (ks == XK_b) {
                printf("Pressed 'b'! (Did XTest trigger the grab?)\n");
                XAllowEvents(d, ReplayKeyboard, ev.xkey.time);
            } else {
                XAllowEvents(d, ReplayKeyboard, ev.xkey.time);
            }
            fflush(stdout);
        } else if (ev.type == KeyRelease) {
            XAllowEvents(d, ReplayKeyboard, ev.xkey.time);
        }
    }
    return 0;
}
