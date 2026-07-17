package main

import (
	"fmt"
	"github.com/BambooEngine/bamboo-core"
)

func main() {
	defs := bamboo.GetInputMethodDefinitions()
	im := bamboo.ParseInputMethod(defs, "Telex")
	engine := bamboo.NewEngine(im, uint(bamboo.VietnameseMode))
	
	fmt.Println("Can process space?", engine.CanProcessKey(' '))
}
