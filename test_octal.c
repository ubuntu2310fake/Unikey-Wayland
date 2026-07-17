#include <stdio.h>
int main() {
    const char* str = "\0\0" "123";
    printf("Char 0: %d\n", str[0]);
    printf("Char 1: %d\n", str[1]);
    printf("Char 2: %d\n", str[2]);
    printf("Char 3: %d\n", str[3]);
    printf("Char 4: %d\n", str[4]);
    return 0;
}
