#include "TestUtils.h"
#include "mesh_transport_network_protocol.h"
#include <iostream>

using namespace std;

TEST_F(TNPTest, ReceiveBroadcast) {
    TNPTest::initializeProtocolWithDefaultParameters();
    int length = 5;
    uint8 data[5] = {0x89, 0x27, 0x90, 0x12, 0xBA};
    broadcastMessage(data, length);
    
    processIncomingMessage(advertisingData[0], 7 + length);
    
    // Check message callback count and its data, and that the broadcast is 
    // forwarded
    ASSERT_EQ(1, TNPTest::messageCallbacks);
    ASSERT_EQ(2, TNPTest::advertisingCalls);
    
    TNPTest::validateData(data, TNPTest::messageData[0], length);
    TNPTest::validateData(advertisingData[0], advertisingData[1], length + 7);
    
    // Make sure the broadcast isn't forwarded if recieved a 2nd time
    processIncomingMessage(advertisingData[0], 7 + length);
    ASSERT_EQ(1, TNPTest::messageCallbacks);
}

TEST_F(TNPTest, ReceiveGroupBroadcast) {
    TNPTest::initializeProtocolWithDefaultParameters();
    int length = 5;
    uint8 data[5] = {0x99, 0xF1, 0xAB, 0x3B, 0xB1};
    uint16 groupID = 27189;
    broadcastGroupMessage(groupID, data, length);
    
    joinGroup(groupID);
    
    // Test processing a message addressed to this node
    processIncomingMessage(advertisingData[0], 9 + length);
    
    // Check message callback count and its data. Make sure the message is
    // forwarded to the rest of the network, as well as the app
    ASSERT_EQ(1, TNPTest::messageCallbacks);
    TNPTest::validateData(data, TNPTest::messageData[0], length);
    ASSERT_EQ(2, TNPTest::advertisingCalls);
    
    // Test processing a message that isn't addressed to the groupID
    broadcastGroupMessage(874, data, length);
    processIncomingMessage(advertisingData[1], 9 + length);
    // Make sure it's forwarded to the rest of the network, but not the app
    ASSERT_EQ(1, TNPTest::messageCallbacks);
    ASSERT_EQ(3, TNPTest::advertisingCalls);
}

TEST_F(TNPTest, ReceiveStatelessAddressedMessage) {
    TNPTest::initializeProtocolWithDefaultParameters();
    int length = 5;
    uint8 data[5] = {0x99, 0xF1, 0xAB, 0x3B, 0xB1};
    sendStatelessMessage(TNPTest::nodeId, data, length);
    
    // Test processing a message addressed to this node
    processIncomingMessage(advertisingData[0], 9 + length);
    
    // Check message callback count and its data. Make sure the message isn't
    // forwarded, since it's addressed to this node
    ASSERT_EQ(1, TNPTest::messageCallbacks);
    TNPTest::validateData(data, TNPTest::messageData[0], length);
    ASSERT_EQ(1, TNPTest::advertisingCalls);
    
    // Test processing the message again, make sure it isn't processed.
    processIncomingMessage(advertisingData[0], 9 + length);
    ASSERT_EQ(1, TNPTest::messageCallbacks);
    ASSERT_EQ(1, TNPTest::advertisingCalls);
    
    // Test processing a message that isn't addressed to this node
    sendStatelessMessage(27218, data, length);
    processIncomingMessage(advertisingData[1], 9 + length);
    // Make sure it's forwarded to the rest of the network, but not the app
    ASSERT_EQ(1, TNPTest::messageCallbacks);
    ASSERT_EQ(2, TNPTest::advertisingCalls);
}

TEST_F(TNPTest, ReceiveStatefulAddressedMessage) {
    TNPTest::initializeProtocolWithDefaultParameters();
    uint8 length = 5;
    uint8 data[5] = {0x99, 0xF1, 0xAB, 0x3B, 0xB1};
    uint16 sender = TNPTest::nodeId;
    uint16 receiver = 0x7281;
    
    // Send message using default node ID, i.e. from the sender
    sendStatefulMessage(receiver, data, length);
    
    // Switch to the reciever and process message addressed to it
    initializeMeshConnectionProtocol(TNPTest::networkID, receiver, 
            &TNPTest::advertiseCallback, &TNPTest::messageCallback,
            &TNPTest::getTimestamp);
    processIncomingMessage(advertisingData[0], 9 + length);
    
    // Check message callback count and its data. Make sure the message isn't
    // forwarded, since it's addressed to this node, BUT that an ACK is sent.
    ASSERT_EQ(1, TNPTest::messageCallbacks);
    TNPTest::validateData(data, TNPTest::messageData[0], length);
    ASSERT_EQ(2, TNPTest::advertisingCalls);
    // Validate that the ACK is actually an ACK, and that it's sent to the 
    // node from which the stateful message orginated, i.e. the sender
    TNPTest::validateHeaderData(TNPTest::networkID, receiver, sender,
            STATEFUL_MESSAGE_ACK, 1, advertisingData[1]);
    
    // Test processing a message that isn't addressed to this node
    sendStatefulMessage(27218, data, length);
    processIncomingMessage(advertisingData[1], 9 + length);
    // Make sure it's forwarded to the rest of the network, but not the app
    ASSERT_EQ(1, TNPTest::messageCallbacks);
    ASSERT_EQ(3, TNPTest::advertisingCalls);
}

TEST_F(TNPTest, ReceiveMessageFromAnotherNetwork) {
    TNPTest::initializeProtocolWithDefaultParameters();
    int length = 5;
    uint8 data[5] = {0x99, 0xF1, 0xAB, 0x3B, 0xB1};
    uint24 networkID = 0x7281;
    
    // Send message using default network ID
    sendStatefulMessage(TNPTest::nodeId, data, length);
    
    // Switch to local network ID and process message
    initializeMeshConnectionProtocol(networkID, TNPTest::nodeId, 
            &TNPTest::advertiseCallback, &TNPTest::messageCallback,
            &TNPTest::getTimestamp);
    processIncomingMessage(advertisingData[0], 9 + length);
    // Make sure no message is forwarded neither to the app or the network
    ASSERT_EQ(0, TNPTest::messageCallbacks);
    ASSERT_EQ(1, TNPTest::advertisingCalls);
}

TEST_F(TNPTest, ReceiveInvalidMessage) {
    TNPTest::initializeProtocolWithDefaultParameters();
    int length = 0;
    
    // Some invalid messages
    uint8 data1[9] = {0x46, 0x75, 0x34, 0x23, 0x24, 0x12, 0x23, 0x12, 0x21};
    uint8 data2[15] = {0};
    uint8 data3[12] = {1};
    
    processIncomingMessage(data1, 9);
    processIncomingMessage(data2, 15);
    processIncomingMessage(data3, 12);
    ASSERT_EQ(0, TNPTest::messageCallbacks);
    ASSERT_EQ(0, TNPTest::advertisingCalls);
}

TEST_F(TNPTest, ResendStatefulMessage) {
    TNPTest::initializeProtocolWithDefaultParameters();
    int length = 0;
    
    // Some invalid messages
    uint8 data1[9] = {0x46, 0x75, 0x34, 0x23, 0x24, 0x12, 0x23, 0x12, 0x21};
    uint8 data2[15] = {0};
    uint8 data3[12] = {1};
    
    processIncomingMessage(data1, 9);
    processIncomingMessage(data2, 15);
    processIncomingMessage(data3, 12);
    ASSERT_EQ(0, TNPTest::messageCallbacks);
    ASSERT_EQ(0, TNPTest::advertisingCalls);
}

// TEST: resend stateful messages

// TEST: resend message with same source and sequence ID 