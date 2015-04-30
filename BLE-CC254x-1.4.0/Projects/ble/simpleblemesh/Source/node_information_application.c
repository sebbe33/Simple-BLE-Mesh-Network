#include "node_information_application.h"
#include "osal.h"
#define NODE_NAME_LENGTH_MAX 16
#define NODE_NAME_ADR 29

#include "print_uart.h"

static applicationClientResponseFunction clientCallback;
static applicationSendMessageFunction sendMessageCallback;
static mainApplicationStatusCallback getMainApplicationStatus;
static persistNameCallback persistNameFunction;
static readNameCallback readNameFunction;
static uint8 mainApplicationId = 0;
static uint8 nodeName[NODE_NAME_LENGTH_MAX] = {0};
static uint8 nodeNameLength = 0;

static void readName();
static void persistName();

void initializeNodeInformationApplication(applicationClientResponseFunction ccb,
                              applicationSendMessageFunction smcb,
                              uint8 mainAppId,
                              mainApplicationStatusCallback masc,
                              persistNameCallback peristNameFunc,
                              readNameCallback readNameFunc) {
  clientCallback = ccb;
  sendMessageCallback = smcb;
  mainApplicationId = mainAppId;
  getMainApplicationStatus = masc;
  persistNameFunction = peristNameFunc;
  readNameFunction = readNameFunc;
  
  // Read persisted name
  readName();
}


void processIcomingMessageNodeInformation(uint16 source, uint8* data, uint8 length) {
  uint8 message[22];

  switch(data[0]) {
  case NODE_INFORMATION_GENERAL_INFO_REQUEST:
#ifndef IS_SERVER
    uint8 messageLength = 4;
    message[0] = NODE_INFORMATION_APPLICATION_CODE;
    message[1] = NODE_INFORMATION_GENERAL_INFO_RESPONSE;
    message[2] = mainApplicationId;
    message[3] = getMainApplicationStatus();
    
    if(nodeNameLength < NODE_NAME_LENGTH_MAX) {
      // Include the length as byte nr. 3
      message[4] = nodeNameLength;
      osal_memcpy(&message[5], nodeName, nodeNameLength);
      messageLength += nodeNameLength + 1;
    } else {
      // copy it directly
      osal_memcpy(&message[4], nodeName, nodeNameLength);
      messageLength += nodeNameLength;
    }
    
    sendMessageCallback(source, message, messageLength);
#endif
    break;
  case NODE_INFORMATION_GENERAL_INFO_RESPONSE:
    message[2] =  NODE_INFORMATION_APPLICATION_CODE;
    // Set source 
    *((uint16*)message) = source; 
    // copy response data
    osal_memcpy(&message[3], data, length);
    clientCallback(message, length+3);
    break;
  case NODE_INFORMATION_GET_NAME_REQUEST:
#ifndef IS_SERVER
     message[0] = NODE_INFORMATION_APPLICATION_CODE;
     message[1] = NODE_INFORMATION_GET_NAME_RESPONSE; 
     message[2] = nodeNameLength;
    // Copy node name
    osal_memcpy(&message[3], nodeName, nodeNameLength);
    sendMessageCallback(source, message, nodeNameLength + 3);
#endif
    break;
  case NODE_INFORMATION_GET_NAME_RESPONSE:
    message[2] =  NODE_INFORMATION_APPLICATION_CODE;
    // Set source 
    *((uint16*)message) = source; 
    // copy response data
    osal_memcpy(&message[3], data, length);
    
    clientCallback(message, length + 3);
    break;
  case NODE_INFORMATION_SET_NAME:
    nodeNameLength = data[1];
    osal_memcpy(nodeName, &data[2], nodeNameLength);
    persistName();
    break;
  }
}

static void readName() 
{
    readNameFunction(NODE_NAME_ADR, &nodeNameLength, 1);
    readNameFunction(NODE_NAME_ADR + 1, nodeName, nodeNameLength);
}
static void persistName() 
{
  persistNameFunction(NODE_NAME_ADR, &nodeNameLength, 1);
  persistNameFunction(NODE_NAME_ADR + 1, nodeName, nodeNameLength);
}