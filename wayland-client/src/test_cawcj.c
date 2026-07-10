#include <stdio.h>
#include <stdlib.h>
#include "libbamboo.h"

int main() {
    Bamboo_Init();
    
    char* keys = "cawcj";
    for (int i = 0; i < 5; i++) {
        Bamboo_ProcessKey(keys[i]);
        char* str = Bamboo_GetPreeditString();
        printf("Key '%c' -> Preedit: %s\n", keys[i], str);
        free(str);
    }
    
    char* commit = Bamboo_GetCommitString();
    printf("Commit: %s\n", commit);
    free(commit);
    
    return 0;
}
