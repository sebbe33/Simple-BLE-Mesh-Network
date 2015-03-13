#include "mesh_transport_protocol.h"
#define PROCESSED_MESSAGE_LENGTH 100
#define REMOVE_PROCESSED_MESSAGE_AFTER 1000

/* Private varialbles */
static uint16 networkIdentifier; 
static uint8 networkIdentifierPrefix;
static uint16 id;
static advertiseDataFunction advertise;
static onMessageRecieved forwardMessageToApp;
static uint8 proccessedMessageStartIndex = 0, processedMessageEndIndex = 0;
static ProccessedMessageInformation proccessedMessages[PROCESSED_MESSAGE_LENGTH];
static uint8 currentSequenceId = 0;

/* Private functions */
static void constructDataMessage(uint8* data, MessageType type, uint16 destination, uint8* message, uint8 length);
static uint8 isMemberOfGroup(uint16 group);
static uint8 hasProccesedMessage(MessageHeader* messageHeader) 
{
    uint8 searchTo = processedMessageEndIndex + 1;
    if(proccessedMessageStartIndex > processedMessageEndIndex) 
    {
        searchTo = PROCESSED_MESSAGE_LENGTH;
    }
    
    for(uint8 i = proccessedMessageStartIndex; i < searchTo; i++) 
    {
        if(messageHeader->source == proccessedMessages[i].source
            && messageHeader->sequenceID == proccessedMessages[i].sequenceID) {
            return TRUE;
        }
    }
    
    if(searchTo == PROCESSED_MESSAGE_LENGTH) 
    {
        for(uint8 i = 0; i < processedMessageEndIndex; i++) 
        {
            if(messageHeader->source == proccessedMessages[i].source
                && messageHeader->sequenceID == proccessedMessages[i].sequenceID) {
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

static void insertProccesedMessage(MessageHeader* messageHeader) 
{
    processedMessageEndIndex++;
    if(processedMessageEndIndex == PROCESSED_MESSAGE_LENGTH) {
        processedMessageEndIndex = 0;
    }
    
    proccessedMessages[processedMessageEndIndex].sequenceID = messageHeader->sequenceID;
    proccessedMessages[processedMessageEndIndex].source = messageHeader->source;
#ifdef TEST
    proccessedMessages[processedMessageEndIndex].time = // TODO : time
#else
     //proccessedMessages[processedMessageEndIndex].time = osal_GetSystemClock();
#endif
}

uint8 isMemberOfGroup(uint16 group) {
	// TODO : Implement
	return 0;
}

void initializeMeshConnectionProtocol(uint24 networkId, 
	uint16 deviceIdentifier, 
	advertiseDataFunction dataFunction, 
	onMessageRecieved messageCallback) 
{

    networkIdentifier = (uint16) networkId ; 
    networkIdentifierPrefix = (uint8) networkId >> 16;
	id = deviceIdentifier;
	advertise = dataFunction;
	forwardMessageToApp = messageCallback;
}

void processIncomingMessage(uint8* message, uint8 length) 
{

	// If invalid message
	if(length < 7) return;

	MessageHeader* header = (MessageHeader*) message;

	// Check if the message is addressed to this network
	if(networkIdentifier != header->networkIdentifier 
            || header->networkIdentifierPrefix != networkIdentifierPrefix) 
        return;

    if(hasProccesedMessage(header)) 
        return;
    
    // TODO : Validate message type
    
	if(header->type == BROADCAST) 
	{
		forwardMessageToApp(&message[4], length - 4);
	} 
	else if (header->type == GROUP_BROADCAST && isMemberOfGroup(header->destination)) 
	{
		forwardMessageToApp(&message[6], length - 6);
	} 
	else if(header->destination == id) 
	{
		switch (header->type) 
		{
			case STATELESS_MESSAGE:
				forwardMessageToApp(&message[6], length - 6);
				break;
	  		case STATEFUL_MESSAGE:
	  			// TODO : Send ACK
	  			forwardMessageToApp(&message[6], length - 6);
	  			break;
	  		case STATEFUL_MESSAGE_ACK:
	  			// TODO : check off ack in list
	  			break;
            default:
                // Invalid message type
                return;
		}
	}
    
    // Forward message to the rest of the network
    advertise(message, length);
    // Save message as processed
    insertProccesedMessage(header);
    
}

void broadcastMessage(uint8* message, uint8 length)
{
    uint8 data[32];
    
    MessageHeader* header = (MessageHeader*) data;
    
    header->networkIdentifier = networkIdentifier;
    header->networkIdentifierPrefix =  networkIdentifierPrefix;
    header->type = BROADCAST;
    header->length = length;
    header->sequenceID = currentSequenceId++;
    header->source = id;
    
    for(char i = 8; i < 8 + length; i++) {
        data[i] = message[i-8];
    }
    
    advertise(data, length + 8);
    
}

void broadcastGroupMessage(uint16 groupDestination, uint8* message, uint8 length)
{
    uint8 data[32];
    constructDataMessage(data, GROUP_BROADCAST, groupDestination, message, length);
    advertise(data, length + 9);
}

void sendStatefulMessage(uint16 destination, uint8* message, uint8 length)
{
    uint8 data[32];
    constructDataMessage(data, STATEFUL_MESSAGE, destination, message, length);
    advertise(data, length + 9);
}

void sendStatelessMessage(uint16 destination, uint8* message, uint8 length)
{
    uint8 data[32];
    constructDataMessage(data, STATELESS_MESSAGE, destination, message, length);
    advertise(data, length + 9);
}

void destructMeshConnectionProtocol()
{
}

void constructDataMessage(uint8* data, MessageType type, uint16 destination, uint8* message, uint8 length) 
{
    MessageHeader* header = (MessageHeader*) data;
    
    header->networkIdentifier = networkIdentifier;
    header->networkIdentifierPrefix =  networkIdentifierPrefix;
    header->type = type;
    header->length = length;
    header->sequenceID = currentSequenceId++;
    header->source = id;
    header->destination = destination;
    
    for(char i = 9; i < 9 + length; i++) {
        data[i] = message[i-9];
    }
    
    advertise(data, length + 9);
}

void clearSentMessages(uint32 timeStamp)
{
    uint8 searchTo = processedMessageEndIndex + 1;
    if(proccessedMessageStartIndex > processedMessageEndIndex) 
    {
        searchTo = PROCESSED_MESSAGE_LENGTH;
    }
    timeStamp -= REMOVE_PROCESSED_MESSAGE_AFTER;
    
    for(uint8 i = proccessedMessageStartIndex; i < searchTo; i++) 
    {
        if(proccessedMessages[i].time < timeStamp) {
           proccessedMessageStartIndex++;
        }
    }
    
    if(searchTo == PROCESSED_MESSAGE_LENGTH) 
    {
        for(uint8 i = 0; i <= processedMessageEndIndex; i++) 
        {
            if(proccessedMessages[i].time < timeStamp) {
                proccessedMessageStartIndex++;
             }
        }
    }
}