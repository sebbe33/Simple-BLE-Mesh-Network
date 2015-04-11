#ifndef APPLICATIONS_H
#define APPLICATIONS_H

// to be called by applications when they need to deliver results to a client,
// i.e. someone connected to the GATT Server
typedef void (*applicationClientResponseFunction)(uint8* data, uint8 length);

// Called by applications when they need to send messages to other nodes
typedef void (*applicationSendMessageFunction)(uint16 destination, uint8* message, uint8 length);

// Called when an incoming message for an application is to be processed
typedef void (*applicationProcessMessageFunction)(uint16 destination, uint8* data, uint8 length);

typedef struct {
    uint8 code;
    applicationProcessMessageFunction fun;
} Application;

#endif