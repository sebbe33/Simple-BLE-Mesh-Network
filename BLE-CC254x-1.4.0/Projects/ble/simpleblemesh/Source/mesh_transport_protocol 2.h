#ifndef MESH_TRANSPORT_PROTOCOL_H
#define MESH_TRANSPORT_PROTOCOL_H

#ifndef TEST

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef long uint32;
typedef uint32 uint24;
#define TRUE 1
#define FALSE 0
#else
#include "comdef.h"
#endif

typedef enum
{
  BROADCAST = 0,
  GROUP_BROADCAST,
  STATELESS_MESSAGE,
  STATEFUL_MESSAGE,
  STATEFUL_MESSAGE_ACK

} MessageType;

typedef void (*advertiseDataFunction)(uint8* data, uint8 length);
typedef void (*onMessageRecieved)(uint8* message, uint8 length);

typedef struct  
{
   uint8 networkIdentifierPrefix;
   uint16 networkIdentifier;
   uint8 length : 5;
   MessageType type : 3;
   uint16 sequenceID;
   uint16 source;
   uint16 destination;
} MessageHeader;

typedef struct 
{
    uint16 source;
    uint8 sequenceID;
    uint16 time;
} ProccessedMessageInformation;

void initializeMeshConnectionProtocol(uint24 networkIdentifier, 
	uint16 deviceIdentifier, 
	advertiseDataFunction dataFunction, 
	onMessageRecieved messageCallback);

void processIncomingMessage(uint8* data, uint8 length);

void broadcastMessage(uint8* message, uint8 length);

void broadcastGroupMessage(uint16 groupDestination, uint8* message, uint8 length);

void sendStatefulMessage(uint16 destination, uint8* message, uint8 length);

void sendStatelessMessage(uint16 destination, uint8* message, uint8 length);

void destructMeshConnectionProtocol();

void clearSentMessages(uint32 timeStamp);
#endif