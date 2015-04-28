#ifndef RELAY_SWITCH_APPLICATION
#define RELAY_SWITCH_APPLICATION

#include "comdef.h"
#include "applications.h"

#define RELAY_SWITCH_APPLICATION_CODE 1
#define RELAY_SWITCH_STATUS_CHANGE 0x01
#define RELAY_SWITCH_STATUS_GET_REQUEST 0x02
#define RELAY_SWITCH_STATUS_GET_RESPONSE 0x03


void initializeRelaySwitchApp(applicationClientResponseFunction ccb,
                              applicationSendMessageFunction smcb);
void processIcomingMessageRelaySwitch(uint16 source, uint8* data, uint8 length);
uint8 getRelayStatus();

#endif