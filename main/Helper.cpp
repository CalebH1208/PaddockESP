#include "Helper.h"


dataContainer CreateDataContainer(int length) {
    dataContainer ret;
    ret.length = length;
    ret.dataArray = (int*) malloc(sizeof(int) * length);
    for (int i = 0; i < length; i++) {
        ret.dataArray[i] = 37;  // Initialize with default value
    }
    return ret;
}

void freeDataContainer(dataContainer container) {
    free(container.dataArray);
}

void updateData(RFM96 radio, multiReciever* TotalData) {
    
    uint8_t values[LoRa_PACKET_LENGTH] = {0};  // Buffer to hold incoming packet
    radio.readData(values, LoRa_PACKET_LENGTH);

    int sourceDet = (values[0] * 256 + values[1]);  // Source detection (IC/EV/Lap Timer)
    testvalue = TotalData->ICData->length;
    switch (sourceDet) {
    case 0:  // IC Data
        for (int i = 0; i < TotalData->ICData->length; i++) {
            TotalData->ICData->dataArray[i] = (values[2 * i] * 256 + values[2 * i + 1]);
        }
        break;

    case 1:  // EV Data
        for (int i = 0; i < TotalData->EVData->length; i++) {
            TotalData->EVData->dataArray[i] = (values[2 * i] * 256 + values[2 * i + 1]);
        }
        break;

    case 2:  // Lap Timer Data
        TotalData->lapData->lap = (values[2] * 256 + values[3]);
        TotalData->lapData->minutes = (values[4] * 256 + values[5]);
        TotalData->lapData->seconds = values[7];
        TotalData->lapData->microSeconds = 0;  // Initialize to 0 before bit-shifting
        for (int i = 0; i < 8; i++) {
            TotalData->lapData->microSeconds |= ((uint64_t)values[9 + i]) << (i * 8);
        }
        break;
}
}
