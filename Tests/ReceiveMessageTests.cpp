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
    uint16 sender = TNPTest::nodeId;
    uint16 receiver1 = 0x18FB, receiver2 = 0x83A1;
    uint8 message1[9] = {0x46, 0x75, 0x34, 0x23, 0x24, 0x12, 0x23, 0x12, 0x21};
    uint8 message2[3] = {1};
    uint8 message3[5] = {0x81, 0x32, 0x3, 0x34, 0x09};
    
    // Send 3 messages from the sender
    TNPTest::initializeProtocolWithDefaultParameters();
    TNPTest::timestamp = 4810;
    sendStatefulMessage(receiver1, message1, 9);
    // Send next message 750 ms after the first one
    TNPTest::timestamp += 750;
    sendStatefulMessage(receiver2, message2, 3);
    // and the next one after additional 200 ms 
    TNPTest::timestamp += 200;
    sendStatefulMessage(receiver1, message3, 5);
    
    int advertisingCount = TNPTest::advertisingCalls;
    // Execute periodic task and make sure no messages are resent
    periodicTask();
    ASSERT_EQ(advertisingCount, TNPTest::advertisingCalls);
    
    // Go ahead 101 ms in time, run periodic task which should resend first msg
    TNPTest::timestamp += 101;
    periodicTask();
    ASSERT_EQ(advertisingCount + 1, TNPTest::advertisingCalls);
    // Validate that the right data is sent out
    TNPTest::validateHeaderData(TNPTest::networkID, sender, receiver1, 
            STATEFUL_MESSAGE, 9, TNPTest::advertisingData[TNPTest::advertisingCalls-1]);
    TNPTest::validateData(&TNPTest::advertisingData[0][9], &TNPTest::advertisingData[TNPTest::advertisingCalls-1][9], 9);
    
    // ACK the first message
    uint8 ackData[10] = {0};
    MessageHeader* ackHeader = (MessageHeader*) ackData;
    ackHeader->networkIdentifier = TNPTest::networkID;
    ackHeader->destination = sender;
    ackHeader->source = receiver1;
    ackHeader->sequenceID = 99;
    ackHeader->length = 1;
    ackHeader->type = STATEFUL_MESSAGE_ACK;
    ackData[7] = ackData[8];
    ackData[8] = ackData[9];

    // Assign the sequenceID of the resent message as the content
    ackData[9] = ((MessageHeader*)TNPTest::advertisingData[TNPTest::advertisingCalls-1])->sequenceID; 
    // Process the ACK for the first message
    processIncomingMessage(ackData, 10);

    // Go ahead 1001 ms and run periodic task, which should resend only the two
    // last sent messages, since the first one has been acked
    advertisingCount = TNPTest::advertisingCalls;
    TNPTest::timestamp += 1001;
    periodicTask();
    ASSERT_EQ(advertisingCount + 2, TNPTest::advertisingCalls);
    // Validate that the right data is sent out in message 2
    TNPTest::validateHeaderData(TNPTest::networkID, sender, receiver2, 
            STATEFUL_MESSAGE, 3, TNPTest::advertisingData[TNPTest::advertisingCalls-2]);
    TNPTest::validateData(&TNPTest::advertisingData[1][9], &TNPTest::advertisingData[TNPTest::advertisingCalls-2][9], 3);
    // in message 3
    TNPTest::validateHeaderData(TNPTest::networkID, sender, receiver1, 
            STATEFUL_MESSAGE, 5, TNPTest::advertisingData[TNPTest::advertisingCalls-1]);
    TNPTest::validateData(&TNPTest::advertisingData[2][9], &TNPTest::advertisingData[TNPTest::advertisingCalls-1][9], 5);
    
    // Ack the 2nd message
    ackHeader->source = receiver2;
    ackData[9] = ((MessageHeader*)TNPTest::advertisingData[TNPTest::advertisingCalls-2])->sequenceID; 
    ackHeader->sequenceID += 1;
    processIncomingMessage(ackData, 10);
    
    // Go ahead 100ms and check that the no message isn't resent again, since
    // the timeout hasn't expired yet (901ms left until the resend of 2 last msg)
    advertisingCount = TNPTest::advertisingCalls;
    TNPTest::timestamp += 100;
    periodicTask();
    ASSERT_EQ(advertisingCount, TNPTest::advertisingCalls);
    
    // Send an ack for the 3rd message that uses an old sequence number (the
    // first one send with message 3)
    ackHeader->source = receiver1;
    ackData[9] = ((MessageHeader*)TNPTest::advertisingData[2])->sequenceID;  
    ackHeader->sequenceID += 1;
    processIncomingMessage(ackData, 10);
    
    // Go ahead 901 ms and check that only the 3rd message is resent,
    // since the 2nd has been ACK'ed
    advertisingCount = TNPTest::advertisingCalls;
    TNPTest::timestamp += 901;
    periodicTask();
    ASSERT_EQ(advertisingCount + 1, TNPTest::advertisingCalls);
    TNPTest::validateHeaderData(TNPTest::networkID, sender, receiver1, 
            STATEFUL_MESSAGE, 5, TNPTest::advertisingData[TNPTest::advertisingCalls-1]);
    
    // ACK the 3rd message
    ackHeader->source = receiver1;
    ackData[9] = ((MessageHeader*)TNPTest::advertisingData[TNPTest::advertisingCalls-1])->sequenceID;  
    ackHeader->sequenceID += 1;
    processIncomingMessage(ackData, 10);
    // Make sure that after a timeout period, all messages have been ACK'ed
    advertisingCount = TNPTest::advertisingCalls;
    TNPTest::timestamp += 1001;
    periodicTask();
    ASSERT_EQ(advertisingCount, TNPTest::advertisingCalls);
}