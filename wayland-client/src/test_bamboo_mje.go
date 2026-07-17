package main

import (
	"fmt"
	"github.com/BambooEngine/bamboo-core"
)

func main() {
	defs := bamboo.GetInputMethodDefinitions()
	im := bamboo.ParseInputMethod(defs, "Telex")
	engine := bamboo.NewEngine(im, uint(bamboo.VietnameseMode))
	
	for _, k := range []rune{'m', 'j'} {
		engine.ProcessKey(k, bamboo.VietnameseMode)
	}
	fmt.Println("mj:", engine.GetProcessedString(bamboo.VietnameseMode))
}
