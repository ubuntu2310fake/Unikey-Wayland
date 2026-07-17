package main

import (
	"fmt"
	"github.com/BambooEngine/bamboo-core"
)

func main() {
	defs := bamboo.GetInputMethodDefinitions()
	im := bamboo.ParseInputMethod(defs, "Telex")
	engine := bamboo.NewEngine(im, uint(bamboo.VietnameseMode))
	
	// 'd', 'd', 'i', 'j', 't', ' ', 'm', 'e', 'j'
	keys := []rune{'d', 'd', 'i', 'j', 't', ' ', 'm', 'e', 'j'}
	
	for i, k := range keys {
		if engine.CanProcessKey(k) {
			engine.ProcessKey(k, bamboo.VietnameseMode)
		} else {
			engine.Reset()
		}
		fmt.Printf("After '%c': %s\n", k, engine.GetProcessedString(bamboo.VietnameseMode))
		if i == len(keys)-1 {
			// Simulate backspace
			engine.Reset()
			fmt.Printf("After backspace (internal state reset)\n")
			// The wrapper re-processes everything except the last key
			// But wait, the wrapper only reprocesses rawKeys!
			// If ' ' caused a Reset, rawKeys is ONLY 'm', 'e', 'j' !!!
			rawKeys := []rune{'m', 'e', 'j'}
			for _, rk := range rawKeys[:len(rawKeys)-1] {
				engine.ProcessKey(rk, bamboo.VietnameseMode)
			}
			fmt.Printf("After backspace: %s\n", engine.GetProcessedString(bamboo.VietnameseMode))
		}
	}
}
