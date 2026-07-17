#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <iostream>

int main() {
    Display *d = XOpenDisplay(NULL);
    if (!d) return 1;

    XKeyEvent ev = {};
    ev.type = KeyPress;
    ev.display = d;
    ev.keycode = 30 + 8; // KEY_A (30) + 8 = 38
    ev.state = 0; // No modifiers

    char buf[32];
    KeySym ks;
    XLookupString(&ev, buf, sizeof(buf), &ks, NULL);
    std::cout << "KEY_A without shift: " << buf << std::endl;

    ev.state = ShiftMask;
    XLookupString(&ev, buf, sizeof(buf), &ks, NULL);
    std::cout << "KEY_A with shift: " << buf << std::endl;

    XCloseDisplay(d);
    return 0;
}
