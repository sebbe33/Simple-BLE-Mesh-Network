#ifndef NETWORK_INFORMATION_APPLICATION_H
#define NETWORK_INFORMATION_APPLICATION_H
#include "comdef.h"
#include "applications.h"

#define NETWORK_INFORMATION_APPLICATION_CODE 4
#define NETWORK_INFO_SET_NAME 0x01
#define NETWORK_INFO_GET_NAME_REQUEST 0x02
#define NETWORK_INFO_GET_NAME_RESPONSE 0x03
#define NETWORK_INFO_PAIRING_INFORMATION 0x04
#define NETWORK_INFO_PAIRING_RESPONSE 0x05
#define NETWORK_INFO_PAIRING_ADDITIONAL_INFO 0x06
#define NETWORK_INFO_NEW_DEVICE 0x07

typedef void (*changeTransportProtocolSettings)(uint16 networkID, uint16 nodeID);
typedef void (*taskBallbackFunction)();
typedef void (*schduleTaskFunction) (uint16 delay, taskBallbackFunction fun);
typedef void (*toggleFunction) (uint8 state);

void initializeNetworkInformationApp(applicationClientResponseFunction ccb,
                              applicationSendMessageFunction smcb,
                              applicationSendBroadcastFunction asbf,
                              persistDataCallback peristFunction,
                              readDataCallback readFunction,
                              schduleTaskFunction taskFun,
                              changeTransportProtocolSettings initTransportProtocol,
                              toggleFunction toggleBlueLedFunction,
                              toggleFunction toggleBlueLedBlinkingFunction,
                              toggleFunction togglePeriodicAdvertisementFunction);
void processIncomingMessageNetworkInformation(uint16 source, uint8* data, uint8 length);
void pairOtherNodeToThisNetwork();
void pairThisNodeToOtherNetwork();


#endif
