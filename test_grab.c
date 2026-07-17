#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>

int main() {
    Display* d = XOpenDisplay(NULL);
    if (!d) return 1;
    Window root = DefaultRootWindow(d);
    
    // Grab keyboard sync
    XGrabKeyboard(d, root, True, GrabModeSync, GrabModeAsync, CurrentTime);
    printf("Grabbed keyboard. Type 'a' to swallow, other keys pass. Type Esc to exit.\n");
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
        }
    }
    
    XUngrabKeyboard(d, CurrentTime);
    XCloseDisplay(d);
    return 0;
}
