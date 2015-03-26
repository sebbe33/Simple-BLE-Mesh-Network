#include "mesh_transport_network_protocol.h"
#define PROCESSED_MESSAGE_LENGTH 100
#define REMOVE_PROCESSED_MESSAGE_AFTER 3000
#define PENDING_ACK_MAX 20
#define PENDING_ACK_RESEND_TIMEOUT 1000
#define GROUP_MEMBERSHIP_MAX 40

#include <stdio.h>

/* Private varialbles */
static uint24 networkIdentifier; 
static uint16 id;
static advertiseDataFunction advertise;
static onMessageRecieved forwardMessageToApp;
static getSystemTimestampFunction getSystemTimestamp;

static uint8 proccessedMessageStartIndex = 0, processedMessageEndIndex = 0;
static ProccessedMessageInformation proccessedMessages[PROCESSED_MESSAGE_LENGTH];
static uint8 currentSequenceId = 0;

static PendingACK pendingACKS[PENDING_ACK_MAX];
static uint8 lastPendingACKIndex = 0;
static uint8 pendingACKMessages[PENDING_ACK_MAX][23];

static uint16 groupMemberships[GROUP_MEMBERSHIP_MAX];
static uint8 groupMemberIndex = 0;


/* Private functions */
static void constructDataMessage(uint8* data, MessageType type, uint16 destination, uint8* message, uint8 length);
static uint8 isMemberOfGroup(uint16 group);
static uint8 hasProccesedMessage(MessageHeader* messageHeader);
static void insertProccesedMessage(MessageHeader* messageHeader);
static void insertPendingACK(uint8* message);
static void removePendingACK(uint8* message);
static uint8 isMemberOfGroup(uint16 group);
static void clearProcessedMessages();
static void resendNonACKedMessages();
static void sendStatefulMessageHelper(uint16 destination, uint8* data, 
        uint8* message, uint8 length);

void initializeMeshConnectionProtocol(uint24 networkId, 
	uint16 deviceIdentifier, 
	advertiseDataFunction dataFunction, 
	onMessageRecieved messageCallback,
    getSystemTimestampFunction timestampFunction) 
{

    networkIdentifier = networkId; 
	id = deviceIdentifier;
	advertise = dataFunction;
	forwardMessageToApp = messageCallback;
    getSystemTimestamp = timestampFunction;
    lastPendingACKIndex = 0;
    groupMemberIndex = 0;
    proccessedMessageStartIndex = 0;
    processedMessageEndIndex = 0;
    for(uint8 i = 0; i < PENDING_ACK_MAX; i++) 
    {
        pendingACKS[i].destination = 0;
    }
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
    
    uint16 destination = (message[8] << 8) | message[7];
	if(header->type == BROADCAST) 
	{
        // Forward message to the rest of the network
        advertise(message, length);
        // Forward to application
		forwardMessageToApp(&message[7], length - 7);
	} 
	else if (header->type == GROUP_BROADCAST && isMemberOfGroup(destination)) 
	{
        advertise(message, length);
		forwardMessageToApp(&message[9], length - 9);
	} 
	else if(destination == id) 
	{
		switch (header->type) 
		{
			case STATELESS_MESSAGE:
				forwardMessageToApp(&message[9], length - 9);
				break;
	  		case STATEFUL_MESSAGE:
                forwardMessageToApp(&message[9], length - 9);
                // Send ACK
                constructDataMessage(message, STATEFUL_MESSAGE_ACK, header->source, &header->sequenceID, 1);
                advertise(message, 9);
	  			break;
	  		case STATEFUL_MESSAGE_ACK:
	  			removePendingACK(message);
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
    sendStatefulMessageHelper(destination, data, message, length);
    // Beware destination will be not correct when casting from raw data to 
    // header, but id doesn't matter in this case
    insertPendingACK(data);
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

uint8 joinGroup(uint16 groupId) 
{
    if(groupMemberIndex < GROUP_MEMBERSHIP_MAX) 
    {
        groupMemberships[groupMemberIndex++] = groupId;
        return TRUE;
    }
    
    return FALSE;
}

void periodicTask() 
{
    // Clear processed messages which are older than 
    // now + REMOVE_PROCESSED_MESSAGE_AFTER
    clearProcessedMessages();
    
    // Go through and resend stateful messages which haven't been ACK'ed
    resendNonACKedMessages();
    
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
    proccessedMessages[processedMessageEndIndex].time = getSystemTimestamp();
}

void constructDataMessage(uint8* data, MessageType type, uint16 destination, uint8* message, uint8 length) 
{
    MessageHeader* header = (MessageHeader*) data;
    header->networkIdentifier = networkIdentifier;
    header->type = type;
    header->length = length;
    header->sequenceID = currentSequenceId++;
    header->source = id;
    header->destination = destination;
    
    // Compensate for struct padding, shift destination bytes 1 byte to the left
    data[7] = data[8];
    data[8] = data[9];
    
    for(char i = 9; i < 9 + length; i++) {
        data[i] = message[i-9];
    }
}

static void resendNonACKedMessages()
{
    uint32 timestamp = getSystemTimestamp() - PENDING_ACK_RESEND_TIMEOUT;
    uint8 data[32];
    for(uint8 i = 0; i < PENDING_ACK_MAX; i++)
    {
        if(pendingACKS[i].destination != 0 && pendingACKS[i].time < timestamp) 
        {
            // Resend unACK'ed messages
            sendStatefulMessageHelper(pendingACKS[i].destination, 
                    data, pendingACKS[i].message, pendingACKS[i].length);
            pendingACKS[i].time = getSystemTimestamp();
            // Set the sequence id to that of the new message
            pendingACKS[i].sequenceId = currentSequenceId - 1;
        }
    }
}

static void sendStatefulMessageHelper(uint16 destination, uint8* data, uint8* message, uint8 length) 
{
    constructDataMessage(data, STATEFUL_MESSAGE, destination, message, length);
    advertise(data, length + 9);
}

void clearProcessedMessages()
{
    uint32 timestamp = getSystemTimestamp();
    uint8 searchTo = processedMessageEndIndex + 1;
    if(proccessedMessageStartIndex > processedMessageEndIndex) 
    {
        searchTo = PROCESSED_MESSAGE_LENGTH;
    }
    timestamp -= REMOVE_PROCESSED_MESSAGE_AFTER;
    
    for(uint8 i = proccessedMessageStartIndex; i < searchTo; i++) 
    {
        if(proccessedMessages[i].time < timestamp) {
           proccessedMessageStartIndex++;
        }
    }
    
    if(searchTo == PROCESSED_MESSAGE_LENGTH) 
    {
        for(uint8 i = 0; i <= processedMessageEndIndex; i++) 
        {
            if(proccessedMessages[i].time < timestamp) {
                proccessedMessageStartIndex++;
             }
        }
    }
}

static void insertPendingACK(uint8* message) 
{
    MessageHeader* messageHeader = (MessageHeader*) message;
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
    } else {
        lastPendingACKIndex = i;
    }
    uint16 destination = (message[8] << 8) | message[7];
    pendingACKS[i].sequenceId = messageHeader->sequenceID;
    pendingACKS[i].destination = destination;
    pendingACKS[i].time = getSystemTimestamp();
    pendingACKS[i].length = messageHeader->length;
    pendingACKS[i].message = pendingACKMessages[i];
    // TODO : Replace by memcpy
    for(uint8 u = 0; u < messageHeader->length; u++) 
    {
        pendingACKMessages[i][u] = message[u+9];
    }
}

static void removePendingACK(uint8* message) 
{
    MessageHeader* messageHeader = (MessageHeader*) message;
    for(uint8 i = 0; i < PENDING_ACK_MAX; i++) 
    {
        if(messageHeader->source == pendingACKS[i].destination
            && message[9] == pendingACKS[i].sequenceId) {
            pendingACKS[i].destination = 0;
        }
    }
}

static uint8 isMemberOfGroup(uint16 group) 
{
    for(uint8 i = 0; i < groupMemberIndex; i = 0)
    {
        if(groupMemberships[i] == group)
            return TRUE;
    }
    return FALSE;
}

