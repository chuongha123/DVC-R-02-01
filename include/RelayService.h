#pragma once

#include <Arduino.h>

// Initialize relay pins and default states
void RelayService_Begin();

// Set relay (1 or 2) state
void RelayService_Set(int relayIndex, bool on);

// Get relay state
bool RelayService_Get(int relayIndex);

// Optional helper: set by pin number
void RelayService_SetByPin(int pin, bool on);
