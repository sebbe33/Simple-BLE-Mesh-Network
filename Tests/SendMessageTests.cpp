#include "TestUtils.h"
#include <gtest/gtest.h>
#include "mesh_transport_network_protocol.h"
#include <iostream>
#define HEADER_SIZE sizeof(MessageHeader)
using namespace std;

int TNPTest::advertisingCalls = 0, 
        TNPTest::messageCallbacks = 0;
uint32 TNPTest::timestamp = 0;
uint8 TNPTest::advertisingData[20][32] = {0}, TNPTest::messageData[20][23] = {0};

TEST_F(TNPTest, BroadcastTest) {
    uint16 networkID = 0xFACB;
    uint16 nodeId = 0xC89A;
    initializeMeshConnectionProtocol(networkID, nodeId, 
            &TNPTest::advertiseCallback, &TNPTest::messageCallback,
            &TNPTest::getTimestamp);
    int length = 5;
    uint8 data[5] = {0xB, 0xA, 0xB, 0xA, 0};
    broadcastMessage(data, length);
    
    // Check right amount of callbacks
    ASSERT_EQ(1, TNPTest::advertisingCalls);
    ASSERT_EQ(1, TNPTest::messageCallbacks);

    // Check header data
    MessageHeader* header = (MessageHeader*) advertisingData[0];
    ASSERT_EQ(nodeId, header->source);
    ASSERT_EQ(networkID, header->networkIdentifier);
    ASSERT_EQ(BROADCAST, header->type);
    ASSERT_EQ(length, header->length);
    
    // Check content
    TNPTest::validateData(data, &advertisingData[0][6], length);
}

TEST_F(TNPTest, GroupBroadcastTest) {
    uint16 networkID = 0xEB81;
    uint16 nodeId = 0x82AB, destination = 0x2EB1;
    initializeMeshConnectionProtocol(networkID, nodeId, 
            &TNPTest::advertiseCallback, &TNPTest::messageCallback,
            &TNPTest::getTimestamp);
    int length = 5;
    uint8 data[5] = {0xB, 0xA, 0xB, 0xA, 0};
    
    broadcastGroupMessage(destination, data, length);
    
    // Check right amount of callbacks
    ASSERT_EQ(1, TNPTest::advertisingCalls);
    ASSERT_EQ(0, TNPTest::messageCallbacks);
    
    // Check header data
    TNPTest::validateHeaderData(networkID, nodeId, destination, GROUP_BROADCAST, 
        length, advertisingData[0]);
    
    // Check content
    TNPTest::validateData(data, &advertisingData[0][HEADER_SIZE], length);
    
    // Become member of group and make sure the broadcast is forwarded
    joinGroup(destination);
    broadcastGroupMessage(destination, data, length);
    ASSERT_EQ(2, TNPTest::advertisingCalls);
    ASSERT_EQ(1, TNPTest::messageCallbacks);
}

TEST_F(TNPTest, StatelessMessageTest) {
    uint16 networkID = 0x1BFA;
    uint16 nodeId = 0xB910, destination = 0x8271;
    initializeMeshConnectionProtocol(networkID, nodeId, 
            &TNPTest::advertiseCallback, &TNPTest::messageCallback,
            &TNPTest::getTimestamp);
    int length = 5;
    uint8 data[5] = {0x1B, 0xAF, 0x89, 0x30, 0x59};
    
    sendStatelessMessage(destination, data, length);
    
    // Check right amount of callbacks
    ASSERT_EQ(1, TNPTest::advertisingCalls);
    ASSERT_EQ(0, TNPTest::messageCallbacks);
    
    // Check header data
    TNPTest::validateHeaderData(networkID, nodeId, destination, STATELESS_MESSAGE, 
        length, advertisingData[0]);
    
    // Check content
    TNPTest::validateData(data, &advertisingData[0][HEADER_SIZE], length);
    
    // Check forward if the message is to self
    sendStatelessMessage(nodeId, data, length);
    ASSERT_EQ(1, TNPTest::advertisingCalls);
    ASSERT_EQ(1, TNPTest::messageCallbacks);
}

TEST_F(TNPTest, StatefulMessageTest) {
    uint16 networkID = 0x8FA9;
    uint16 nodeId = 0x0918, destination = 0x8211;
    initializeMeshConnectionProtocol(networkID, nodeId, 
            &TNPTest::advertiseCallback, &TNPTest::messageCallback,
            &TNPTest::getTimestamp);
    int length = 5;
    uint8 data[5] = {0x0B, 0x81, 0x89, 0x77, 0x88};
    
    sendStatefulMessage(destination, data, length);
    
    // Check right amount of callbacks
    ASSERT_EQ(1, TNPTest::advertisingCalls);
    ASSERT_EQ(0, TNPTest::messageCallbacks);
    
    // Check header data
    TNPTest::validateHeaderData(networkID, nodeId, destination, STATEFUL_MESSAGE, 
        length, advertisingData[0]);
    
    // Check content
    TNPTest::validateData(data, &advertisingData[0][HEADER_SIZE], length);
    
    // Check forward if the message is to self
    sendStatefulMessage(nodeId, data, length);
    ASSERT_EQ(1, TNPTest::advertisingCalls);
    ASSERT_EQ(1, TNPTest::messageCallbacks);
}

