#ifndef ADVERTISING_QUEUE_H
#define ADVERTISING_QUEUE_H
#include "comdef.h"
typedef struct  
{
    uint8 length;
    uint8* data;
    uint32 advertisingTimeStamp;
} AdvQueueItem;

uint8 getAdvertisementQueueSize();
uint8 enqueueAdvertisement(uint8 length, uint8* data, uint32 timeStamp);
AdvQueueItem* getFirstInAdvertisementQueue();
void removeFirstInAdvertisementQueue();
uint8 dequeueAdvertisement(uint16 source, uint8 sequenceID);


#endif