package main

/*
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
*/
import "C"
import (
	"github.com/BambooEngine/bamboo-core"
)

var preeditor bamboo.IEngine
var currentMethod string = "Telex"
var currentFlags uint = bamboo.EfreeToneMarking
var spellCheckEnabled bool
var autoRestoreEnabled bool
var rawKeys []rune

func initEngine(methodName string) {
	currentMethod = methodName
	defs := bamboo.GetInputMethodDefinitions()
	im := bamboo.ParseInputMethod(defs, currentMethod)
	preeditor = bamboo.NewEngine(im, currentFlags)
	rawKeys = rawKeys[:0]
}

//export Bamboo_Init
func Bamboo_Init() {
	initEngine("Telex")
}

//export Bamboo_SetInputMethod
func Bamboo_SetInputMethod(method C.int) {
	methodName := "Telex"
	switch int(method) {
	case 1:
		methodName = "Telex"
	case 2:
		methodName = "VNI"
	case 3:
		methodName = "VIQR"
	case 4:
		methodName = "Telex 2"
	default:
		methodName = "Telex"
	}
	initEngine(methodName)
}

//export Bamboo_SetOptions
func Bamboo_SetOptions(freeMarking C.bool, modernStyle C.bool, spellCheck C.bool, autoRestore C.bool) {
	var flags uint = 0
	if bool(freeMarking) {
		flags |= bamboo.EfreeToneMarking
	}
	// Bamboo Core semantics:
	// EstdToneStyle (true) puts tone on first vowel -> òa, úy (truyền thống)
	// EstdToneStyle (false) puts tone on second vowel -> oà, uý (kiểu mới)
	// Checkbox "Đặt dấu oà, uý (thay vì òa, úy)":
	// Unticked (false) -> user wants òa, úy -> set EstdToneStyle
	// Ticked (true) -> user wants oà, uý -> clear EstdToneStyle
	if !bool(modernStyle) {
		flags |= bamboo.EstdToneStyle
	}
	spellCheckEnabled = bool(spellCheck)
	autoRestoreEnabled = bool(autoRestore)
	currentFlags = flags
	if preeditor != nil {
		preeditor.SetFlag(currentFlags)
	}
}

//export Bamboo_CanProcessKey
func Bamboo_CanProcessKey(key C.uint32_t) C.bool {
	if preeditor == nil {
		return C.bool(false)
	}
	return C.bool(preeditor.CanProcessKey(rune(key)))
}

//export Bamboo_ProcessKey
func Bamboo_ProcessKey(key C.uint32_t) {
	processKey(rune(key))
}

//export Bamboo_RemoveLastChar
func Bamboo_RemoveLastChar() {
	removeLastKey()
}

func processKey(key rune) {
	if preeditor != nil {
		rawKeys = append(rawKeys, key)
		preeditor.ProcessKey(key, bamboo.VietnameseMode)
	}
}

// Replay raw keys because a transformation may consume more than one key.
func removeLastKey() {
	if preeditor == nil || len(rawKeys) == 0 {
		return
	}
	rawKeys = rawKeys[:len(rawKeys)-1]
	preeditor.Reset()
	for _, key := range rawKeys {
		preeditor.ProcessKey(key, bamboo.VietnameseMode)
	}
}

// Use the stricter validation only when the word is committed.
func processedString(final bool) string {
	if preeditor == nil {
		return ""
	}

	vietnamese := preeditor.GetProcessedString(bamboo.VietnameseMode)
	if !bamboo.HasAnyVietnameseRune(vietnamese) {
		return vietnamese
	}

	invalidPartial := spellCheckEnabled && !preeditor.IsValid(false)
	invalidFinal := final && autoRestoreEnabled && !preeditor.IsValid(true)
	if invalidPartial || invalidFinal {
		return preeditor.GetProcessedString(bamboo.EnglishMode)
	}
	return vietnamese
}

//export Bamboo_GetPreeditString
func Bamboo_GetPreeditString() *C.char {
	if preeditor == nil {
		return C.CString("")
	}
	return C.CString(processedString(false))
}

//export Bamboo_GetCommitString
func Bamboo_GetCommitString() *C.char {
	return C.CString(processedString(true))
}

//export Bamboo_Reset
func Bamboo_Reset() {
	if preeditor != nil {
		rawKeys = rawKeys[:0]
		preeditor.Reset()
	}
}

func main() {}
