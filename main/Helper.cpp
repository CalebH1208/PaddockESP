#include "Helper.h"
#include <math.h>

typedef struct dataC_t{
    int length;
    int * dataArray;
}dataContainer;

typedef struct {
    RFM96* radio;
    dataContainer* data;
}passVariables;

dataContainer CreateDataContainer(int length){
    dataContainer ret;
    ret.length = length;
    ret.dataArray =(int*) malloc(sizeof(int)*length);
    for(int i =0;i<length;i++){
        ret.dataArray[i] =37;
    }
    return ret;
}
void freeDataArray(dataContainer Killme){
    free(Killme.dataArray);
}


void updateData(RFM96 radio,dataContainer* Data){
  uint8_t values[LoRa_PACKET_LENGTH] = {0};
  radio.readData(values,LoRa_PACKET_LENGTH);
  for(int i=0;i<Data->length;i++){
    Data->dataArray[i]=(values[2*i]*256+values[2*i+1]);
  }
  
}

      