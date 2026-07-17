#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>

int main() {
    Display* d = XOpenDisplay(NULL);
    KeyCode kc = XKeysymToKeycode(d, 'a');
    printf("KeyCode for 'a': %d\n", kc);
    
    KeyCode kc_d = XKeysymToKeycode(d, 0x0111);
    printf("KeyCode for 'đ': %d\n", kc_d);
    return 0;
}
