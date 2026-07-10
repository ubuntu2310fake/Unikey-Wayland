package main

/*
#cgo LDFLAGS: -L${SRCDIR} -lbamboo
#include "libbamboo.h"
*/
import "C"
import "fmt"

func main() {
    C.Bamboo_Init()
    
    canC := C.Bamboo_CanProcessKey(C.uint32_t('c'))
    canS := C.Bamboo_CanProcessKey(C.uint32_t('s'))
    
    fmt.Printf("Can process 'c'? %v\n", bool(canC))
    fmt.Printf("Can process 's'? %v\n", bool(canS))
}
