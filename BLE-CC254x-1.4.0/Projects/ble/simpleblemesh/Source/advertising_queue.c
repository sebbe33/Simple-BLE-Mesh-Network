#include "advertising_queue.h"
#include "mesh_transport_network_protocol.h"

#define ADVERTISING_QUEUE_MAX_SIZE 5

static AdvQueueItem advertisingQueue[ADVERTISING_QUEUE_MAX_SIZE];
static uint8 size = 0;

uint8 getAdvertisementQueueSize() {
    return size;
}
uint8 enqueueAdvertisement(uint8 length, uint8* data, uint32 timeStamp) {
  if(size == ADVERTISING_QUEUE_MAX_SIZE) {
    return FALSE;
  }
  
  uint8 i = size - 1;
  for(; i >= 0; i--) {
    if(advertisingQueue[i].advertisingTimeStamp > timeStamp) {
        // move up in the queue
        advertisingQueue[i].length = advertisingQueue[i+1].length;
        advertisingQueue[i].data = advertisingQueue[i+1].data;
        advertisingQueue[i].advertisingTimeStamp = advertisingQueue[i+1].advertisingTimeStamp;
    } else {
      i++; // The current has a lower timestamp => insert one above
      break;
    }
  }
  
  advertisingQueue[i].length = length;
  advertisingQueue[i].data = data;
  advertisingQueue[i].advertisingTimeStamp = timeStamp;
  size++;
  
  return TRUE;
}

AdvQueueItem* getFirstInAdvertisementQueue() {
  return size > 0? &advertisingQueue[size] : NULL;
    
}

void removeFirstInAdvertisementQueue() {
  if(size == 0) {
    return;
  }
  
  size--;
  for(uint8 i = 0; i < size; i++) {
      advertisingQueue[size].length = advertisingQueue[size+1].length;
      advertisingQueue[size].data = advertisingQueue[size+1].data;
      advertisingQueue[size].advertisingTimeStamp = advertisingQueue[size+1].advertisingTimeStamp;
  }
  
}

uint8 dequeueAdvertisement(uint16 source, uint8 sequenceID) {
  uint8 moveDown = FALSE;
   for(uint8 i = 0; i < size; i++) {
     MessageHeader* header = (MessageHeader*) &advertisingQueue[i].data;
     if(header->source == source && header->sequenceID == sequenceID) {
      moveDown = TRUE;
      size--;
     } 
     
     if(moveDown == TRUE) {
      advertisingQueue[i].length = advertisingQueue[i+1].length;
      advertisingQueue[i].data = advertisingQueue[i+1].data;
      advertisingQueue[i].advertisingTimeStamp = advertisingQueue[i+1].advertisingTimeStamp;
     }
  } 
}