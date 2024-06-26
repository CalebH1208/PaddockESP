#include <stdio.h>
#include <RadioLib.h>
#include "esp32_perfmon.h"
#include "esp_http_server.h"
#include "cJSON.h"

//#include "can.h"
#include "Helper.h"
#include "WiFi.h"
//#include "can.c"
#include "Helper.cpp"
#include "WiFi.c"
#include "cJSON.c"

#define SCK 18
#define MISO 19
#define MOSI 23
EspHal* hal = new EspHal(SCK, MISO, MOSI);

#define NSS 5
#define DIO0 2
  #define NRST 14
#define DIO1 34

//comment out this line for IC firmware other wise leave for EV firmware
#define EV 1

#ifdef EV
#define LENGTH 27
#else
#define LENGTH 26
#endif

RFM96 radio = new Module(hal, NSS, DIO0, NRST, DIO1);
dataContainer data = CreateDataContainer(LENGTH);



static const char *TAG = "main";

volatile bool recievedFlag=false;
///http
cJSON* create_data_object(int ind,double val){
  cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "index", ind);
    cJSON_AddNumberToObject(obj, "value", val);
    return obj;
}


static esp_err_t piggy(httpd_req_t * req){
  cJSON *root = cJSON_CreateObject();
  cJSON_AddNumberToObject(root, "Length", LENGTH);
  cJSON *data_array = cJSON_CreateArray();
  for(int i=0;i<LENGTH;i++){
    cJSON_AddItemToArray(data_array, create_data_object(i,data.dataArray[i]));
  }
  cJSON_AddItemToObject(root, "Data",data_array);

  const char *str = cJSON_Print(root);
  httpd_resp_set_type(req,"application/json");
  httpd_resp_send(req,str,HTTPD_RESP_USE_STRLEN );
  free((void*)str);
  cJSON_Delete(root);
  return ESP_OK;
}

////


void printLoRaMessage(){
    updateData(radio,&data);
    return;
}
void packetRecieved(void * nothing){
// while(1){
//   updateData(radio,&data);
//   vTaskDelay(20/portTICK_PERIOD_MS);
// }
ESP_LOGI(TAG, "Initializing LoRa... ");
    int state = radio.begin(915.0F,500.0F,7U,5U,0x12U,17,8U,0U); 
    if (state != RADIOLIB_ERR_NONE) {
        ESP_LOGI(TAG, "failed, code %d\n", state);
        while(true) {
            hal->delay(1000);
        }
    }
    ESP_LOGI(TAG, "success!\n");
    //dataContainer data = CreateDataContainer(LENGTH);
    state = radio.startReceive();
radio.setPacketReceivedAction(printLoRaMessage);
while(1)vTaskDelay(10000/portTICK_PERIOD_MS);
}


extern "C" void app_main(void) {
  data.dataArray[0] = 0;
//  ESP_LOGI(TAG, "Initializing LoRa... ");
//     int state = radio.begin(915.0F,500.0F,7U,5U,0x12U,17,8U,0U); 
//     if (state != RADIOLIB_ERR_NONE) {
//         ESP_LOGI(TAG, "failed, code %d\n", state);
//         while(true) {
//             hal->delay(1000);
//         }
//     }
//     ESP_LOGI(TAG, "success!\n");
//     //dataContainer data = CreateDataContainer(LENGTH);
//     state = radio.startReceive();
    xTaskCreatePinnedToCore(packetRecieved,"packet Received",4096,NULL,5,NULL,1);

  // if (state == RADIOLIB_ERR_NONE) {
  //   ESP_LOGI(TAG, "success");
  // } else {
  //   ESP_LOGI(TAG, "failed, code %d\n", state);
  //   while (true);
  // }
  

esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
/////////////////////http stuff I guess???????//////////////
static const httpd_uri_t littlewolf = {
  .uri = "/",
  .method = HTTP_GET,
  .handler = piggy,
  .user_ctx = NULL
};
static httpd_handle_t server = NULL;
static httpd_config_t config = HTTPD_DEFAULT_CONFIG();
if(httpd_start(&server,&config)== ESP_OK){
  ESP_LOGI(TAG,"HTTP UP MFER");
  httpd_register_uri_handler(server,&littlewolf);
}





////////////////////////////////////////////////////
  perfmon_start();
    while(1){
        vTaskDelay(2000/portTICK_PERIOD_MS);
        // ESP_LOGI("DATA OUTPUT","\nPack Temp High: %d\nPack Temp Average: %d\nMotor Temp:%.1f\nInverter Temp: %.1f\nRPM: %d\nLow voltage: %.1f\nHigh Voltage: %.1f\n\n\n\n",
        // data.dataArray[22],data.dataArray[23],data.dataArray[17]/10.0,data.dataArray[19]/10.0,data.dataArray[21],data.dataArray[18]/10.0,data.dataArray[24]/10.0
        // );
        //motor:17,LV:18,Inverter:20,Rpm:21,High battery temp: 23,Average Temp:24,Total voltage:25,
        //low voltage battery and high, rpm
    }
    }