#ifndef MESH_TRANSPORT_PROTOCOL_H
#define MESH_TRANSPORT_PROTOCOL_H

#ifndef TEST
typedef int32   int24;
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

typedef void (*advertiseDataFunction)(uint8* data);
typedef void (*onMessageRecieved)(uint8* message, uint8 length);

typedef struct  
{
   uint8 networkIdentifierPrefix;
   uint16 networkIdentifier;
   uint8 length : 5;
   MessageType type : 3;
   uint16 sequenceID;
   uint16 destination;
   uint16 source;
} MessageHeader;

void initializeMeshConnectionProtocol(uint24 networkIdentifier, 
	uint16 deviceIdentifier, 
	advertiseDataFunction dataFunction, 
	onMessageRecieved messageCallback);

void processIncomingMessage(uint8* data, unit8 length);

void broadcastMessage(uint8* message, uint8 length);

void broadcastGroupMessage(uint16 groupDestination, uint8* message, uint8 length);

void sendStatefulMessage(uint16 destination, uint8* message, uint8 length);

void sendStatelessMessage(uint16 destination, uint8* message, uint8 length);

void destructMeshConnectionProtocol();
#endif