#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <iostream>

void send_unicode(Display* d, KeySym ks) {
    int min_keycode, max_keycode;
    XDisplayKeycodes(d, &min_keycode, &max_keycode);
    
    // Use max_keycode to map our temporary keysym
    KeyCode kc = max_keycode; 
    
    KeySym old_ks;
    int keysyms_per_keycode_return;
    KeySym* map = XGetKeyboardMapping(d, kc, 1, &keysyms_per_keycode_return);
    if (map) { old_ks = map[0]; XFree(map); } else { old_ks = NoSymbol; }
    
    KeySym new_map[2] = {ks, ks};
    XChangeKeyboardMapping(d, kc, 2, new_map, 1);
    XSync(d, False);
    
    XTestFakeKeyEvent(d, kc, True, CurrentTime);
    XTestFakeKeyEvent(d, kc, False, CurrentTime);
    XSync(d, False);
    
    KeySym restore_map[2] = {old_ks, old_ks};
    XChangeKeyboardMapping(d, kc, 2, restore_map, 1);
    XSync(d, False);
}

int main() {
    Display *d = XOpenDisplay(NULL);
    if (!d) return 1;

    sleep(1); // wait to focus terminal
    
    // Unicode for á is U+00E1 -> keysym 0x010000e1? Or just 0x00e1 for latin1?
    // standard keysym for á is 0x00e1
    send_unicode(d, 0x00e1);
    
    XCloseDisplay(d);
    return 0;
}
