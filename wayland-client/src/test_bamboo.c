#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "libbamboo.h"

int main() {
    Bamboo_Init();
    printf("Init done\n");
    
    Bamboo_ProcessKey('c');
    Bamboo_ProcessKey('o');
    Bamboo_ProcessKey('s');
    
    char* str = Bamboo_GetPreeditString();
    printf("Result of 'cos': %s\n", str);
    return 0;
}
