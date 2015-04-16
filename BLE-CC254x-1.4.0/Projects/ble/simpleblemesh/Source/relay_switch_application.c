#include "relay_switch_application.h"
#include "OnBoard.h"
#include "print_uart.h"

#define RELAY_SWITCH_PIN 0x02
uint8 status = 0;
applicationClientResponseFunction clientCallback;
applicationSendMessageFunction sendMessageCallback;

void initializeRelaySwitchApp(applicationClientResponseFunction ccb,
                              applicationSendMessageFunction smcb) 
{
  clientCallback = ccb;
  sendMessageCallback = smcb;
  
  // Set up the pin
  P0SEL &= ~RELAY_SWITCH_PIN; // Configure PIN P0_1 as GPIO
  P0DIR |= RELAY_SWITCH_PIN; // Configure PIN P0_1 as output
  P0_1 = status;
}

void processIcomingMessageRelaySwitch(uint16 source, uint8* data, uint8 length)
{
  switch(data[0]) {
    case RELAY_SWITCH_STATUS_CHANGE:
      status =  data[1] > 0? 1 : 0;
      P0_1 = status;
      break;
    case RELAY_SWITCH_STATUS_GET_REQUEST:
      uint8 responsedata[3] = {RELAY_SWITCH_APPLICATION_CODE, 
                                RELAY_SWITCH_STATUS_GET_RESPONSE, status};
      sendMessageCallback(source, responsedata, 3);
      break;
    case RELAY_SWITCH_STATUS_GET_RESPONSE:
      uint8 forwardData[5] = {0,0, RELAY_SWITCH_APPLICATION_CODE, RELAY_SWITCH_STATUS_GET_RESPONSE, data[1]};
      *((uint16*)forwardData) = source; 
      clientCallback(forwardData, length+3);
    break;
  }
}

