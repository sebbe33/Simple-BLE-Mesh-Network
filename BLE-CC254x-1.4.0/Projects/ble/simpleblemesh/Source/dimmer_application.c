#include "dimmer_application.h"
#include "OnBoard.h"

#define RELAY_SWITCH_PIN 0x02
static uint8 dimValue = 0;
static applicationClientResponseFunction clientCallback;
static applicationSendMessageFunction sendMessageCallback;
static UARTWriteFunction uartWriteFunction;

void initializeDimmerApp(applicationClientResponseFunction ccb,
                              applicationSendMessageFunction smcb,
                              UARTWriteFunction uart)
{
  clientCallback = ccb;
  sendMessageCallback = smcb;
  uartWriteFunction = uart;
  // Write 0 to uart
  uart(&dimValue, 1);
}

void processIncomingMessageDimmer(uint16 source, uint8* data, uint8 length)
{
  switch(data[0]) {
    case DIMMER_SET_DIM_VALUE:
      dimValue = data[1];
      uartWriteFunction(&dimValue, 1);
      break;
    case DIMMER_GET_DIM_VALUE_REQUEST:
      uint8 responsedata[3] = {DIMMER_APPLICATION_CODE, 
                                DIMMER_GET_DIM_VALUE_RESPONSE, dimValue};
      sendMessageCallback(source, responsedata, 3);
      break;
    case DIMMER_GET_DIM_VALUE_RESPONSE:
      uint8 forwardData[5] = {0,0, DIMMER_APPLICATION_CODE, DIMMER_GET_DIM_VALUE_RESPONSE, data[1]};
      *((uint16*)forwardData) = source; 
      clientCallback(forwardData, length+3);
    break;
  }
}

