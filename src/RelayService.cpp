#include "RelayService.h"
#include "ConfigStore.h"

// If your relay module is active LOW, set RELAY_ACTIVE_LOW to 1 (default).
#ifndef RELAY_ACTIVE_LOW
#define RELAY_ACTIVE_LOW 0
#endif

#if RELAY_ACTIVE_LOW
#define RELAY_ON LOW
#define RELAY_OFF HIGH
#else
#define RELAY_ON HIGH
#define RELAY_OFF LOW
#endif

static bool relayState[NUM_RELAYS] = {false};

void RelayService_Begin()
{
    for (int i = 0; i < NUM_RELAYS; ++i)
    {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], RELAY_OFF); // default off
        relayState[i] = false;
    }
}

void RelayService_SetByPin(int pin, bool on)
{
    for (int i = 0; i < NUM_RELAYS; ++i)
    {
        if (RELAY_PINS[i] == pin)
        {
            digitalWrite(RELAY_PINS[i], on ? RELAY_ON : RELAY_OFF);
            relayState[i] = on;
            return;
        }
    }
}

void RelayService_Set(int relayIndex, bool on)
{
    if (relayIndex < 1 || relayIndex > NUM_RELAYS) return;
    int idx = relayIndex - 1;
    digitalWrite(RELAY_PINS[idx], on ? RELAY_ON : RELAY_OFF);
    relayState[idx] = on;
}

bool RelayService_Get(int relayIndex)
{
    if (relayIndex < 1 || relayIndex > NUM_RELAYS) return false;
    return relayState[relayIndex - 1];
}
