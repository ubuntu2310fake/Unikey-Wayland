#include <iostream>
#include <linux/input.h>

char evdev_to_ascii(int code, bool shift) {
    const char* lower = 
        "\0\0" "1234567890-=\b\t" 
        "qwertyuiop[]\n\0as"
        "dfghjkl;'`\0\\zxcv"
        "bnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    const char* upper = 
        "\0\0" "!@#$%^&*()_+\b\t" 
        "QWERTYUIOP{}\n\0AS"
        "DFGHJKL:\"~\0|ZXCV"
        "BNM<>?\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    if (code >= 0 && code < 80) {
        return shift ? upper[code] : lower[code];
    }
    return 0;
}

int main() {
    std::cout << evdev_to_ascii(KEY_A, false) << std::endl;
    std::cout << evdev_to_ascii(KEY_A, true) << std::endl;
    std::cout << evdev_to_ascii(KEY_SPACE, false) << std::endl;
    return 0;
}
