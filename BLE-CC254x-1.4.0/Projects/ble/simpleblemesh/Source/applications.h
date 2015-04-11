#ifndef APPLICATIONS_H
#define APPLICATIONS_H

// to be called by applications when they need to deliver results to a client,
// i.e. someone connected to the GATT Server
typedef void (*applicationClientResponseFunction)(uint8* data, uint8 length);

typedef void (*applicationSendMessageFunction)(uint16 destination, uint8* message, uint8 length);


#endif