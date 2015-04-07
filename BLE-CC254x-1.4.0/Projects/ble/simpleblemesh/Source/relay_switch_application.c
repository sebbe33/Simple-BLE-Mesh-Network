#include "relay_switch_application.h"
#include "OnBoard.h"
#include "print_uart.h"
void initializeRelaySwitchApp() 
{
  P0SEL &= ~RELAY_SWITCH_PIN; // Configure PIN P0_1 as GPIO
  P0DIR |= RELAY_SWITCH_PIN; // Configure PIN P0_1 as output
  P0_1 = 0;
}

void processIcomingMessageRelaySwitch(uint8* data, uint8 length) 
{
    P0_1 = data[0] > 0? 1 : 0;
}

