#ifndef NODE_INFORMATION_APPLICATION_H 
#define NODE_INFORMATION_APPLICATION_H 
#include "comdef.h"
#include "applications.h"

#define NODE_INFORMATION_APPLICATION_CODE 3

// OP-Codes
#define NODE_INFORMATION_GENERAL_INFO_REQUEST 0x01
#define NODE_INFORMATION_GENERAL_INFO_RESPONSE 0x02
#define NODE_INFORMATION_GET_NAME_REQUEST 0x03
#define NODE_INFORMATION_GET_NAME_RESPONSE 0x04
#define NODE_INFORMATION_SET_NAME 0x05
#define NODE_INFORMATION_JOIN_GROUP 0x06
#define NODE_INFORMATION_LEAVE_GROUP 0x07

typedef uint8 (*mainApplicationStatusCallback) ();
typedef void (*persistNameCallback)(uint16 address, uint8* data, uint8 length);
typedef void (*readNameCallback)(uint16 address, uint8* data, uint8 length);

void initializeNodeInformationApplication(applicationClientResponseFunction ccb,
                              applicationSendMessageFunction smcb,
                              uint8 mainAppId,
                              mainApplicationStatusCallback mascs,
                              persistNameCallback peristNameFunction,
                              readNameCallback readNameFunction);
void processIcomingMessageNodeInformation(uint16 source, uint8* data, uint8 length);

uint8* getNetworkName(uint8* lengthPointer);
uint16 getNetworkID();
uint16 getNodeID();

#endif
