#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "libbamboo.h"

int main() {
    Bamboo_Init();
    
    bool canC = Bamboo_CanProcessKey('c');
    bool canS = Bamboo_CanProcessKey('s');
    
    printf("Can process 'c'? %d\n", canC);
    printf("Can process 's'? %d\n", canS);
    return 0;
}
