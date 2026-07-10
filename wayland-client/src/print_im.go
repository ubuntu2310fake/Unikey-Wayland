package main

import (
	"fmt"
	"github.com/BambooEngine/bamboo-core"
)

func main() {
    defs := bamboo.GetInputMethodDefinitions()
    fmt.Printf("Available input methods: ")
    for k := range defs {
        fmt.Printf("'%s' ", k)
    }
    fmt.Println()
}
