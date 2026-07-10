package main

/*
#cgo LDFLAGS: -L${SRCDIR} -lbamboo
#include "libbamboo.h"
*/
import "C"
import "fmt"

func main() {
    C.Bamboo_Init()
    C.Bamboo_ProcessKey(C.uint32_t('c'))
    C.Bamboo_ProcessKey(C.uint32_t('o'))
    C.Bamboo_ProcessKey(C.uint32_t('s'))
    
    str := C.GoString(C.Bamboo_GetPreeditString())
    fmt.Println("Result of 'cos':", str)
}
