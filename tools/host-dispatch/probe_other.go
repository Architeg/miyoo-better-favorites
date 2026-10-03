//go:build !windows

package main

import "fmt"

func detectWindows() (host, error) { return host{}, fmt.Errorf("Windows entry point requires Windows") }
