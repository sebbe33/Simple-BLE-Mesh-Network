#ifndef RELAY_SWITCH_APPLICATION
#define RELAY_SWITCH_APPLICATION

#include "comdef.h"

#define RELAY_SWITCH_CODE 1
#define RELAY_SWITCH_PIN 0x02

void initializeRelaySwitchApp();
void processIcomingMessageRelaySwitch(uint8* data, uint8 length);

#endif