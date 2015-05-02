#include "network_information_application.h"
#include "osal.h"
#define NETWORK_ID_ADR		1
#define NETWORK_NAME_ADR	9
#define NETWORK_NAME_LENGTH_MAX 20
#define NODE_ID_ADR		5
#define UNIVERSAL_PAIRING_ADDRESS 0x0000
#define UNIVERSAL_NETWORK_ID 0xFFFF
#define PAIRING_TIMEOUT         30000
#define BURST_INTERVAL          5000

#include "print_uart.h"

static uint16 networkID;
static uint16 nodeID;
static uint16 nextNodeID = 9;
static uint8 networkName[NETWORK_NAME_LENGTH_MAX] = {0};
static uint8 networkNameLength;
static uint8 isPairing = FALSE;

/* Callbacks */
static applicationClientResponseFunction clientCallback;
static applicationSendMessageFunction sendMessage;
static persistDataCallback persistData;
static readDataCallback readData;
static changeTransportProtocolSettings changeTransportProtocol;
static applicationSendBroadcastFunction sendBroadcast;
static schduleTaskFunction sheduleTask;
static toggleFunction toggleBlueLed;
static toggleFunction toggleBlueLedBlinking;
static toggleFunction togglePeriodicAdvertisement;

/* Local functions */
static void readNetworkID();
static void persistNetworkID();
static void readNetworkName();
static void persistNetworkName();
static void readNodeID();
static void persistNodeID();
static void cancelPairing();
static void delayedBlueLedShutoff();
static void reccuringEvent();
void initializeNetworkInformationApp(applicationClientResponseFunction ccb,
                              applicationSendMessageFunction smcb,
                              applicationSendBroadcastFunction asbf,
                              persistDataCallback persistFunction,
                              readDataCallback readFunction,
                              schduleTaskFunction taskFun,
                              changeTransportProtocolSettings initTransportProtocol,
                              toggleFunction toggleBlueLedFunction,
                              toggleFunction toggleBlueLedBlinkingFunction,
                              toggleFunction togglePeriodicAdvertisementFunction) 
{
  clientCallback = ccb;
  sendMessage = smcb;
  sendBroadcast = asbf;
  persistData = persistFunction;
  readData = readFunction;
  changeTransportProtocol = initTransportProtocol;
  sheduleTask = taskFun;
  toggleBlueLed = toggleBlueLedFunction;
  toggleBlueLedBlinking = toggleBlueLedBlinkingFunction;
  togglePeriodicAdvertisement = togglePeriodicAdvertisementFunction;
  readNetworkID();
  readNodeID();
  readNetworkName();
}

uint8* getNetworkName(uint8* lengthPointer) 
{
    *lengthPointer = networkNameLength;
    return networkName;
}
uint16 getNetworkID() 
{
  return networkID;
}
uint16 getNodeID() 
{
  return nodeID;
}

void processIncomingMessageNetworkInformation(uint16 source, uint8* data, uint8 length)
{
  uint8 message[20];
  
  switch(data[0]) {
    case NETWORK_INFO_SET_NAME:
      networkNameLength = data[1];
      osal_memcpy(networkName, &data[2], networkNameLength);
      persistNetworkName();
      break;
    case NETWORK_INFO_GET_NAME_REQUEST:
      message[0] = NETWORK_INFORMATION_APPLICATION_CODE;
      message[1] = NETWORK_INFO_GET_NAME_RESPONSE; 
      message[2] = networkNameLength;
      // Copy node name
      osal_memcpy(&message[3], networkName, networkNameLength);
      sendMessage(source, message, networkNameLength + 3);
      break;
    case NETWORK_INFO_GET_NAME_RESPONSE:
      message[2] =  NETWORK_INFORMATION_APPLICATION_CODE;
      // Set source 
      *((uint16*)message) = source; 
      // copy response data
      osal_memcpy(&message[3], data, length);
      
      clientCallback(message, length + 3);
      break;
    case NETWORK_INFO_PAIRING_INFORMATION:
      if(isPairing == FALSE) {
        return;
      }
      
      // Set the network ID
      networkID = *((uint16*) &data[1]);
      // Set the node id
      nodeID = *((uint16*) &data[3]);

      // Send a response
      message[0] = NETWORK_INFORMATION_APPLICATION_CODE;
      message[1] = NETWORK_INFO_PAIRING_RESPONSE;
      *((uint16*)&message[2]) = nodeID;
      sendMessage(source, message, 4);
      debugPrintRawArray(message, 4);
      // Change the transport protocol settings to the received ones
      changeTransportProtocol(networkID, nodeID);
      break;
      
    case NETWORK_INFO_PAIRING_RESPONSE:
      if(isPairing == TRUE) {
        cancelPairing();
        // Turn on constant blue light to indicate success
        toggleBlueLed(TRUE);
        sheduleTask(5000, delayedBlueLedShutoff);
        debugPrintLine("Got it");
        // Send additional info
        uint16 destination = *((uint16*)&data[1]);
        message[0] = NETWORK_INFORMATION_APPLICATION_CODE;
        message[1] = NETWORK_INFO_PAIRING_ADDITIONAL_INFO;
        message[2] = networkNameLength;
        // Copy the network name to the message
        osal_memcpy(&message[3], networkName, networkNameLength);
        sendMessage(destination, message, networkNameLength + 3);
      }
      break;
  
    case NETWORK_INFO_PAIRING_ADDITIONAL_INFO:
      networkNameLength = data[1];
      // copy the network name to local variable
      osal_memcpy(networkName, &data[2], networkNameLength);
      isPairing = FALSE;
      // Turn on constant blue light to indicate success
      toggleBlueLedBlinking(FALSE);
      toggleBlueLed(TRUE);
      sheduleTask(5000, delayedBlueLedShutoff);
      
      // Persist new settings
      persistNetworkID();
      persistNetworkName();
      persistNodeID();
      // broadcast myself as being a part of the network.
      message[0] = NETWORK_INFORMATION_APPLICATION_CODE;
      message[1] = NETWORK_INFO_NEW_DEVICE;
      *((uint16*)&message[2]) = nodeID;
      sendBroadcast(message, 4);
      break;
      
    case NETWORK_INFO_NEW_DEVICE:
      // Set next node id to be the one receiver  + 1
      nextNodeID = *((uint16*)&data[1]) + 1;
      break;
  }
}

void pairOtherNodeToThisNetwork() 
{
  // set the tranport protocol settings to be universal (so that every device
  // can observe sent messages).
  changeTransportProtocol(UNIVERSAL_NETWORK_ID, nodeID);
  togglePeriodicAdvertisement(FALSE);
  
  // begin reoccuring event
  reccuringEvent();
  isPairing = TRUE;
  toggleBlueLedBlinking(TRUE);
}

void pairThisNodeToOtherNetwork() 
{
  changeTransportProtocol(UNIVERSAL_NETWORK_ID, UNIVERSAL_PAIRING_ADDRESS);
  togglePeriodicAdvertisement(FALSE);
  isPairing = TRUE;
  toggleBlueLedBlinking(TRUE);
  // set timeout function
  sheduleTask(PAIRING_TIMEOUT, cancelPairing);
}

static void readNetworkID() 
{
  readData(NETWORK_ID_ADR, (uint8*) &networkID, 2);
}

static void persistNetworkID()
{
  persistData(NETWORK_ID_ADR, (uint8*) &networkID, 2);
}

static void readNetworkName()
{
  readData(NETWORK_NAME_ADR, &networkNameLength, 1);
  readData(NETWORK_NAME_ADR + 1, networkName, networkNameLength);
}

static void persistNetworkName()
{
  persistData(NETWORK_NAME_ADR, &networkNameLength, 1);
  persistData(NETWORK_NAME_ADR + 1, networkName, networkNameLength);
}

static void readNodeID()
{
  readData(NODE_ID_ADR, (uint8*) &nodeID, 2);
}

static void persistNodeID()
{
  persistData(NODE_ID_ADR, (uint8*)&nodeID, 2);
}

static void cancelPairing()
{
  if(isPairing == FALSE) {
    return;
  }
  // Reset settings
  readNetworkName();
  readNodeID();
  readNetworkID();
  toggleBlueLedBlinking(FALSE);
  isPairing = FALSE;
  // Change back to the original transport protocol settings
  changeTransportProtocol(networkID, nodeID);
  togglePeriodicAdvertisement(TRUE);
}

static void delayedBlueLedShutoff() 
{
  toggleBlueLed(FALSE);
}

uint16 pairingTimeoutCounter = 0;
static void reccuringEvent() 
{
  if(pairingTimeoutCounter >= PAIRING_TIMEOUT) {
    cancelPairing();
    return;
  }
  
  uint8 message[6] = {NETWORK_INFORMATION_APPLICATION_CODE, NETWORK_INFO_PAIRING_INFORMATION};
  // Add the network ID
  *((uint16*) &message[2]) = networkID;
  // Add next available node id
  *((uint16*) &message[4]) = nextNodeID;
  sendMessage(UNIVERSAL_PAIRING_ADDRESS, message, 6);
  
  sheduleTask(BURST_INTERVAL, reccuringEvent);
  pairingTimeoutCounter += BURST_INTERVAL;
}

