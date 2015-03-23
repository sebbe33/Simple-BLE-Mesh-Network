#include "mesh_transport_network_protocol.h"
#define PROCESSED_MESSAGE_LENGTH 100
#define REMOVE_PROCESSED_MESSAGE_AFTER 1000
#define PENDING_ACK_MAX 20

/* Private varialbles */
static uint24 networkIdentifier; 
static uint16 id;
static advertiseDataFunction advertise;
static onMessageRecieved forwardMessageToApp;
static uint8 proccessedMessageStartIndex = 0, processedMessageEndIndex = 0;
static ProccessedMessageInformation proccessedMessages[PROCESSED_MESSAGE_LENGTH];
static uint8 currentSequenceId = 0;
static PendingACK pendingACKS[PENDING_ACK_MAX];
static uint8 lastPendingACKIndex = 0;

/* Private functions */
static void constructDataMessage(uint8* data, MessageType type, uint16 destination, uint8* message, uint8 length);
static uint8 isMemberOfGroup(uint16 group);
static uint8 hasProccesedMessage(MessageHeader* messageHeader);
static void insertProccesedMessage(MessageHeader* messageHeader);
static void insertPendingACK(MessageHeader* messageHeader);
static void removePendingACK(MessageHeader* messageHeader);

static uint8 isMemberOfGroup(uint16 group) {
	// TODO : Implement
	return 0;
}

void initializeMeshConnectionProtocol(uint24 networkId, 
	uint16 deviceIdentifier, 
	advertiseDataFunction dataFunction, 
	onMessageRecieved messageCallback) 
{

    networkIdentifier = networkId; 
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
	if(networkIdentifier != header->networkIdentifier) 
        return;

    if(hasProccesedMessage(header)) 
        return;
    
    // TODO : Validate message type
    
	if(header->type == BROADCAST) 
	{
        // Forward message to the rest of the network
        advertise(message, length);
        // Forward to application
		forwardMessageToApp(&message[4], length - 4);
	} 
	else if (header->type == GROUP_BROADCAST && isMemberOfGroup(header->destination)) 
	{
        advertise(message, length);
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
                forwardMessageToApp(&message[6], length - 6);
                // Send ACK
                constructDataMessage(message, STATEFUL_MESSAGE_ACK, header->source, &header->sequenceID, 10);
                advertise(message, 10);
	  			break;
	  		case STATEFUL_MESSAGE_ACK:
	  			removePendingACK(header);
	  			break;
            default:
                // Invalid message type
                return;
		}
	}
    
    // Save message as processed
    insertProccesedMessage(header);
    
}

void broadcastMessage(uint8* message, uint8 length)
{
    uint8 data[32];
    MessageHeader* header = (MessageHeader*) data;
    
    header->networkIdentifier = networkIdentifier;
    header->type = BROADCAST;
    header->length = length;
    header->sequenceID = currentSequenceId++;
    header->source = id;
    
    for(char i = 7; i < 7 + length; i++) {
        data[i] = message[i-7];
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
    // Beware destination will be not correct when casting from raw data to 
    // header, but id doesn't matter in this case
    insertPendingACK((MessageHeader*) data);
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

uint8 hasProccesedMessage(MessageHeader* messageHeader) 
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

void insertProccesedMessage(MessageHeader* messageHeader) 
{
    processedMessageEndIndex++;
    if(processedMessageEndIndex == PROCESSED_MESSAGE_LENGTH) {
        processedMessageEndIndex = 0;
    }
    
    if(processedMessageEndIndex == proccessedMessageStartIndex) {
        proccessedMessageStartIndex++;
    }
    
    proccessedMessages[processedMessageEndIndex].sequenceID = messageHeader->sequenceID;
    proccessedMessages[processedMessageEndIndex].source = messageHeader->source;
#ifdef TEST
    proccessedMessages[processedMessageEndIndex].time = (uint16) time(NULL);
#else
     //proccessedMessages[processedMessageEndIndex].time = osal_GetSystemClock();
#endif
}

void constructDataMessage(uint8* data, MessageType type, uint16 destination, uint8* message, uint8 length) 
{
    MessageHeader* header = (MessageHeader*) data;
    header->networkIdentifier = networkIdentifier;
    header->type = type;
    header->length = length;
    header->sequenceID = 5;//currentSequenceId++;
    header->source = id;
    header->destination = destination;
    
    // Compensate for struct padding, shift destination bytes 1 byte to the left
    data[7] = data[8];
    data[8] = data[9];
    
    for(char i = 9; i < 9 + length; i++) {
        data[i] = message[i-9];
    }
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

static void insertPendingACK(MessageHeader* messageHeader) 
{
    uint8 i = 0;
    for(; i < PENDING_ACK_MAX; i++) 
    {
        if(pendingACKS[i].destination == 0) 
        {
            break;
        }
    }
    
    if(i == PENDING_ACK_MAX) 
    {
        // If the array is full, increment the last pending ACK index,
        // to overwrite the PendingAck which is the oldest
        i = lastPendingACKIndex = (lastPendingACKIndex + 1) % PENDING_ACK_MAX;
    }
    
    pendingACKS[i].sequenceId = messageHeader->sequenceID;
    pendingACKS[i].destination = messageHeader->source;
}

static void removePendingACK(MessageHeader* messageHeader) 
{
    for(uint8 i = 0; i < PENDING_ACK_MAX; i++) 
    {
        if(messageHeader->source == pendingACKS[i].destination
            && messageHeader->sequenceID == pendingACKS[i].sequenceId) {
            pendingACKS[i].destination = 0;
        }
    }
}