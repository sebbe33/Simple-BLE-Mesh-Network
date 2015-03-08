#include "mesh_transport_protocol.h"

/* Private varialbles */
static uint24 networkID; 
static uint16 id;
static advertiseDataFunction advertise;
static onMessageRecieved forwardMessageToApp;

/* Private functions */
uint8 isMemberOfGroup(uint16 group);

uint8 isMemberOfGroup(uint16 group) {
	// TODO : Implement
	return 0;
}

void initializeMeshConnectionProtocol(uint24 networkIdentifier, 
	uint16 deviceIdentifier, 
	advertiseDataFunction dataFunction, 
	onMessageRecieved messageCallback) 
{
	networkID = networkIdentifier;
	id = deviceIdentifier;
	advertise = dataFunction;
	forwardMessageToApp = messageCallback;
}

void processIncomingMessage(uint8* message, unit8 length) 
{

	// If invalid message
	if(length < 7) return;

	MessageHeader* header = (MessageHeader*) message;

	// Check if the message is addressed to this network
	if(networkID != ((header->networkIdentifierPrefix << 16) | header->networkIdentifier)) return;


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

		}
	}
}

void broadcastMessage(uint8* message, uint8 length)
{

}

void broadcastGroupMessage(uint16 groupDestination, uint8* message, uint8 length)
{

}

void sendStatefulMessage(uint16 destination, uint8* message, uint8 length)
{

}

void sendStatelessMessage(uint16 destination, uint8* message, uint8 length)
{

}

void destructMeshConnectionProtocol()
{

}