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
	fmt.Println("After mej:", engine.GetProcessedString(bamboo.VietnameseMode))
	
	// Remove last char (simulate backspace)
	engine.Reset()
	for _, k := range keys[:len(keys)-1] {
		engine.ProcessKey(k, bamboo.VietnameseMode)
	}
	fmt.Println("After mej - backspace:", engine.GetProcessedString(bamboo.VietnameseMode))
}
