#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    Display* d = XOpenDisplay(NULL);
    if (!d) return 1;
    Window root = DefaultRootWindow(d);
    
    XGrabKeyboard(d, root, True, GrabModeSync, GrabModeAsync, CurrentTime);
    printf("Grabbed. Press 'a' to swallow and inject 'b'.\n");
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
                
                // Inject 'b'
                KeyCode b_code = XKeysymToKeycode(d, XK_b);
                XTestFakeKeyEvent(d, b_code, True, CurrentTime);
                XTestFakeKeyEvent(d, b_code, False, CurrentTime);
                XFlush(d);
            } else if (ev.type == KeyPress && ks == XK_Escape) {
                printf("Escaped!\n");
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
