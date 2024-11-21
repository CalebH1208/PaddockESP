#include <stdlib.h>
#include <math.h>
#include "can.h"
#include "ESPHal.h"

extern int testvalue =0;

#define LoRa_PACKET_LENGTH 52  // Length of incoming LoRa packet

typedef struct dataC_t {
    int length;
    int *dataArray;
} dataContainer;

typedef struct {
    RFM96* radio;
    dataContainer* data;
} passVariables;

typedef struct {
    uint16_t minutes;
    uint8_t seconds;
    uint64_t microSeconds;
    uint16_t lap;
} lapDataContainer;

typedef struct {
    dataContainer* ICData;
    dataContainer* EVData;
    lapDataContainer* lapData;
} multiReciever;

dataContainer CreateDataContainer(int length);
void freeDataContainer(dataContainer);
void updateData(RFM96 radio, multiReciever* TotalData);
