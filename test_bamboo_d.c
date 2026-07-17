#include "libbamboo.h"
#include <stdio.h>
#include <string.h>

int main() {
    Bamboo_Init();
    
    Bamboo_ProcessKey('d');
    printf("After d: [%s]\n", Bamboo_GetState());
    
    Bamboo_ProcessKey('d');
    printf("After dd: [%s]\n", Bamboo_GetState());
    
    Bamboo_ProcessKey('d');
    printf("After ddd: [%s]\n", Bamboo_GetState());
    
    Bamboo_ProcessKey('d');
    printf("After dddd: [%s]\n", Bamboo_GetState());
    
    return 0;
}
