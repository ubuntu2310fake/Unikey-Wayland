package main

import (
	"fmt"
	"github.com/BambooEngine/bamboo-core"
)

func main() {
	defs := bamboo.GetInputMethodDefinitions()
	im := bamboo.ParseInputMethod(defs, "Telex")
	engine := bamboo.NewEngine(im, uint(bamboo.VietnameseMode))
	
	keys := []rune{'m', 'e', 'j'}
	for _, k := range keys {
		engine.ProcessKey(k, bamboo.VietnameseMode)
	}
	s := engine.GetProcessedString(bamboo.VietnameseMode)
	fmt.Printf("Bytes: ")
	for i := 0; i < len(s); i++ {
		fmt.Printf("%02x ", s[i])
	}
	fmt.Println()
}
