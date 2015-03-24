/* 
 * File:   TestUtils.h
 * Author: sebastianblomberg
 *
 * Created on den 23 mars 2015, 20:18
 */

#ifndef TESTUTILS_H
#define	TESTUTILS_H
#include "mesh_transport_network_protocol.h"
#include <gtest/gtest.h>

class TNPTest : public testing::Test {
public:  
    static int advertisingCalls, messageCallbacks;
    static uint32 timestamp;
    static uint8 advertisingData[20][32], messageData[20][23];
    static const uint24 networkID = 0xFACB;
    static const uint16 nodeId = 0xC89A;
    
    virtual void SetUp() {
        destructMeshConnectionProtocol();
        advertisingCalls = 0;
        messageCallbacks = 0;
    }

    static uint32 getTimestamp() {
        return timestamp;
    }
    
    static void initializeProtocolWithDefaultParameters() {
        initializeMeshConnectionProtocol(TNPTest::networkID, TNPTest::nodeId, 
            &TNPTest::advertiseCallback, 
            &TNPTest::messageCallback,
            &TNPTest::getTimestamp);
    }
    
    static void advertiseCallback(uint8* data, uint8 length) {
        for(int i = 0; i < length; i++) {
            advertisingData[advertisingCalls][i] = data[i];
        }
        advertisingCalls++;
    }
    
    static void messageCallback(uint8* message, uint8 length) {
        for(int i = 0; i < length; i++) {
            messageData[messageCallbacks][i] = message[i];
        }
        messageCallbacks++;
    }
    
    static void validateHeaderData(uint24 networkID, uint16 source, 
            uint16 destination, MessageType type, uint8 length, uint8* rawData) {
        MessageHeader* header = (MessageHeader*) rawData;
        ASSERT_EQ(source, header->source);
        ASSERT_EQ(networkID, header->networkIdentifier);
        ASSERT_EQ(type, header->type);
        ASSERT_EQ(length, header->length);
        ASSERT_EQ(destination, (rawData[8] << 8) | rawData[7]);
    }
    
    static void validateData(uint8* expected, uint8* result, uint8 length) {
        // Check correct data
        for(int i = 0; i < length; i++) {
            ASSERT_EQ(expected[i], result[i]);
        }
    }
};

#endif	/* TESTUTILS_H */

