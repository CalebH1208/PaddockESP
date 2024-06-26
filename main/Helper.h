#include <stdlib.h>
#include "can.h"
#include "ESPHal.h"

#define LoRa_PACKET_LENGTH 52

typedef struct dataC_t dataContainer;

dataContainer CreateDataContainer(int length);
void freeDataContainer(dataContainer);
int updateArray(twai_message_t message,dataContainer* DataArray);
int LoRaSend(dataContainer DataArray,RFM96);
void updateData(RFM96 radio,dataContainer* Data);
