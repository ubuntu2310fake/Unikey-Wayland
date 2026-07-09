#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <poll.h>
#include <cstring>
#include <linux/input.h>
#include <linux/uinput.h>

// Utility to get current monotonic time in nanoseconds
uint64_t get_nano_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Convert ASCII char to Linux input keycode (QWERTY layout mapping)
int char_to_keycode(char c) {
    switch (c) {
        case 'a': case 'A': return KEY_A;
        case 'b': case 'B': return KEY_B;
        case 'c': case 'C': return KEY_C;
        case 'd': case 'D': return KEY_D;
        case 'e': case 'E': return KEY_E;
        case 'f': case 'F': return KEY_F;
        case 'g': case 'G': return KEY_G;
        case 'h': case 'H': return KEY_H;
        case 'i': case 'I': return KEY_I;
        case 'j': case 'J': return KEY_J;
        case 'k': case 'K': return KEY_K;
        case 'l': case 'L': return KEY_L;
        case 'm': case 'M': return KEY_M;
        case 'n': case 'N': return KEY_N;
        case 'o': case 'O': return KEY_O;
        case 'p': case 'P': return KEY_P;
        case 'q': case 'Q': return KEY_Q;
        case 'r': case 'R': return KEY_R;
        case 's': case 'S': return KEY_S;
        case 't': case 'T': return KEY_T;
        case 'u': case 'U': return KEY_U;
        case 'v': case 'V': return KEY_V;
        case 'w': case 'W': return KEY_W;
        case 'x': case 'X': return KEY_X;
        case 'y': case 'Y': return KEY_Y;
        case 'z': case 'Z': return KEY_Z;
        
        case '1': return KEY_1;
        case '2': return KEY_2;
        case '3': return KEY_3;
        case '4': return KEY_4;
        case '5': return KEY_5;
        case '6': return KEY_6;
        case '7': return KEY_7;
        case '8': return KEY_8;
        case '9': return KEY_9;
        case '0': return KEY_0;
        
        case ' ': return KEY_SPACE;
        case '\n': return KEY_ENTER;
        case '-': return KEY_MINUS;
        case '=': return KEY_EQUAL;
        case '[': return KEY_LEFTBRACE;
        case ']': return KEY_RIGHTBRACE;
        case ';': return KEY_SEMICOLON;
        case '\'': return KEY_APOSTROPHE;
        case ',': return KEY_COMMA;
        case '.': return KEY_DOT;
        case '/': return KEY_SLASH;
        case '\\': return KEY_BACKSLASH;
        case '\b': return KEY_BACKSPACE;
    }
    return 0;
}

// Sniffer implementation
void run_sniffer() {
    std::vector<int> fds;
    DIR* dir = opendir("/dev/input");
    if (!dir) {
        std::cerr << "ERROR: Cannot open /dev/input" << std::endl;
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            std::string path = "/dev/input/" + std::string(entry->d_name);
            int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
            if (fd >= 0) {
                // Verify if it's a keyboard
                unsigned long key_b[KEY_CNT / 64 + 1];
                memset(key_b, 0, sizeof(key_b));
                if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_b)), key_b) >= 0) {
                    // Check if it has basic keys (A and ENTER) to filter out power buttons, etc.
                    auto test_bit = [&](int bit) {
                        return (key_b[bit / 64] & (1ULL << (bit % 64))) != 0;
                    };
                    if (test_bit(KEY_A) && test_bit(KEY_ENTER)) {
                        fds.push_back(fd);
                    } else {
                        close(fd);
                    }
                } else {
                    close(fd);
                }
            }
        }
    }
    closedir(dir);

    if (fds.empty()) {
        std::cerr << "ERROR: No physical keyboards found in /dev/input" << std::endl;
        return;
    }

    std::cout << "SNIFFER_READY: Listening on " << fds.size() << " keyboard devices." << std::endl;

    std::vector<struct pollfd> pfd(fds.size());
    for (size_t i = 0; i < fds.size(); ++i) {
        pfd[i].fd = fds[i];
        pfd[i].events = POLLIN;
    }

    while (true) {
        int ret = poll(pfd.data(), pfd.size(), -1);
        if (ret < 0) break;

        for (size_t i = 0; i < pfd.size(); ++i) {
            if (pfd[i].revents & POLLIN) {
                struct input_event ev;
                while (read(pfd[i].fd, &ev, sizeof(ev)) > 0) {
                    if (ev.type == EV_KEY) {
                        uint64_t ns = (uint64_t)ev.time.tv_sec * 1000000000ULL + (uint64_t)ev.time.tv_usec * 1000ULL;
                        // Print exact keycode, value (1 = press, 0 = release, 2 = repeat) and timestamp
                        std::cout << "HW_KEY " << ev.code << " " << ev.value << " " << ns << std::endl;
                    }
                }
            }
        }
    }

    for (int fd : fds) close(fd);
}

// Spammer implementation
void run_spammer(const std::string& sequence, int delay_ms) {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "ERROR: Cannot open /dev/uinput. Do you have root permissions?" << std::endl;
        return;
    }

    // Enable key events
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);

    // Register all standard keys
    for (int i = 1; i < 256; ++i) {
        ioctl(fd, UI_SET_KEYBIT, i);
    }

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "ImTester Virtual Keyboard");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    // Wait for the virtual device to be registered by the system
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "SPAMMER_READY" << std::endl;

    auto send_key = [&](int code, int value) {
        struct input_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.type = EV_KEY;
        ev.code = code;
        ev.value = value;
        write(fd, &ev, sizeof(ev));

        // Sync
        ev.type = EV_SYN;
        ev.code = SYN_REPORT;
        ev.value = 0;
        write(fd, &ev, sizeof(ev));
    };

    for (char c : sequence) {
        int keycode = char_to_keycode(c);
        if (keycode == 0) continue;

        uint64_t ns_press = get_nano_time();
        std::cout << "SPAM_KEY " << keycode << " 1 " << ns_press << std::endl;
        send_key(keycode, 1); // Press
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms / 2));

        uint64_t ns_release = get_nano_time();
        std::cout << "SPAM_KEY " << keycode << " 0 " << ns_release << std::endl;
        send_key(keycode, 0); // Release
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms / 2));
    }

    // Clean up
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " --sniff | --spam <sequence> <delay_ms>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "--sniff") {
        run_sniffer();
    } else if (mode == "--spam" && argc >= 4) {
        std::string seq = argv[2];
        int delay = std::stoi(argv[3]);
        run_spammer(seq, delay);
    } else {
        std::cerr << "Invalid arguments" << std::endl;
        return 1;
    }

    return 0;
}
