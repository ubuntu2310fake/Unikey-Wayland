package main

import (
	"testing"

	"github.com/BambooEngine/bamboo-core"
)

func typeKeys(keys string) {
	for _, key := range keys {
		processKey(key)
	}
}

func resetTestEngine(method string, spellCheck, autoRestore bool) {
	currentFlags = bamboo.EfreeToneMarking
	spellCheckEnabled = spellCheck
	autoRestoreEnabled = autoRestore
	initEngine(method)
}

func TestTelexComposition(t *testing.T) {
	resetTestEngine("Telex", true, true)
	typeKeys("theer")
	if got := processedString(true); got != "thể" {
		t.Fatalf("theer: got %q, want %q", got, "thể")
	}
}

func TestInputMethodsSelectedBySettings(t *testing.T) {
	tests := []struct {
		method string
		keys   string
	}{
		{method: "VNI", keys: "the63"},
		{method: "VIQR", keys: "the^?"},
		{method: "Telex 2", keys: "theer"},
	}

	for _, tc := range tests {
		t.Run(tc.method, func(t *testing.T) {
			resetTestEngine(tc.method, true, true)
			typeKeys(tc.keys)
			if got := processedString(true); got != "thể" {
				t.Fatalf("%s: got %q, want %q", tc.keys, got, "thể")
			}
		})
	}
}

func TestInvalidVietnameseSequenceRestoresRawKeys(t *testing.T) {
	resetTestEngine("Telex", true, true)
	typeKeys("afc") // àc is invalid: c only accepts acute or dot tones.
	if got := processedString(true); got != "afc" {
		t.Fatalf("got %q, want raw keys %q", got, "afc")
	}
}

func TestAutoRestoreCanBeDisabled(t *testing.T) {
	resetTestEngine("Telex", false, false)
	typeKeys("afc")
	if got := processedString(true); got != "àc" {
		t.Fatalf("got %q, want un-restored composition %q", got, "àc")
	}
}

func TestBackspaceReplaysBambooStateCorrectly(t *testing.T) {
	resetTestEngine("Telex", true, true)
	typeKeys("theer")
	removeLastKey()
	if got := processedString(false); got != "thê" {
		t.Fatalf("got %q, want %q", got, "thê")
	}
}
