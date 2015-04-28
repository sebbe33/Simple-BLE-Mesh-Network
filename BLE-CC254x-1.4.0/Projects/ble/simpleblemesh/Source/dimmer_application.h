#ifndef DIMMER_APPLICATION
#define DIMMER_APPLICATION

#include "comdef.h"
#include "applications.h"

#define DIMMER_APPLICATION_CODE 2
#define DIMMER_SET_DIM_VALUE 0x01
#define DIMMER_GET_DIM_VALUE_REQUEST 0x02
#define DIMMER_GET_DIM_VALUE_RESPONSE 0x03

typedef void (*UARTWriteFunction)(uint8* data, uint8 length);

void initializeDimmerApp(applicationClientResponseFunction ccb,
                              applicationSendMessageFunction smcb,
                              UARTWriteFunction uart);
void processIncomingMessageDimmer(uint16 destination, uint8* data, uint8 length);
uint8 getDimValue();
#endif