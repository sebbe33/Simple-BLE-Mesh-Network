#include "relay_switch_application.h"
#include "OnBoard.h"
#include "print_uart.h"

#define RELAY_SWITCH_PIN 0x02
uint8 status = 0;
void initializeRelaySwitchApp() 
{
  P0SEL &= ~RELAY_SWITCH_PIN; // Configure PIN P0_1 as GPIO
  P0DIR |= RELAY_SWITCH_PIN; // Configure PIN P0_1 as output
  P0_1 = status;
}

void processIcomingMessageRelaySwitch(uint8* data, uint8 length) 
{
  switch(data[0]) {
    case RELAY_SWITCH_STATUS_CHANGE:
      status =  data[1] > 0? 1 : 0;
      P0_1 = status;
      break;
    case RELAY_SWITCH_STATUS_GET_REQUEST:
      break;
    case RELAY_SWITCH_STATUS_GET_RESPONSE:
      
    break;
  }
}

