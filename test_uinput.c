#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

struct ukw_event {
    unsigned short type;
    unsigned short code;
    int value;
};

void send_ev(int fd, int type, int code, int value) {
    struct ukw_event e = {(unsigned short)type, (unsigned short)code, value};
    write(fd, &e, sizeof(e));
}

void syn(int fd) { send_ev(fd, EV_SYN, SYN_REPORT, 0); }

int main() {
    int fd = open("/dev/ukw", O_WRONLY);
    sleep(1);
    
    // Type shift M
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 1); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, KEY_M, 1); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, KEY_M, 0); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, KEY_LEFTSHIFT, 0); syn(fd); usleep(15000);
    
    // Type A
    send_ev(fd, EV_KEY, KEY_A, 1); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, KEY_A, 0); syn(fd); usleep(15000);
    
    // Type Enter
    send_ev(fd, EV_KEY, KEY_ENTER, 1); syn(fd); usleep(15000);
    send_ev(fd, EV_KEY, KEY_ENTER, 0); syn(fd); usleep(15000);
    
    return 0;
}
