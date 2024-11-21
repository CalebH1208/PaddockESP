#include <stdio.h>
#include <RadioLib.h>
#include "esp32_perfmon.h"
#include "esp_http_server.h"
#include "cJSON.h"


#include "Helper.cpp"
#include "WiFi.c"

// real pinouts!!!!
// #define SCK 18
// #define MISO 19
// #define MOSI 23
// EspHal* hal = new EspHal(SCK, MISO, MOSI);

// #define NSS 5
// #define DIO0 32
// #define NRST 27
// #define DIO1 34

//Fake pinouts!!!
#define SCK 18
#define MISO 19
#define MOSI 23
EspHal* hal = new EspHal(SCK, MISO, MOSI);

#define NSS 5
#define DIO0 2
#define NRST 14
#define DIO1 34

// Define firmware type: IC or EV
#define EV 1  // Comment out for IC firmware, leave for EV firmware

#ifdef EV
#define LENGTH 27
#else
#define LENGTH 26
#endif

RFM96 radio = new Module(hal, NSS, DIO0, NRST, DIO1);

// Predefined lengths for IC and EV data
#define ICLENGTH 27
#define EVLENGTH 27

// Create containers for each type of data
dataContainer ICData = CreateDataContainer(ICLENGTH);
dataContainer EVData = CreateDataContainer(EVLENGTH);
lapDataContainer lapData = {0};

// Structure to hold all types of data
multiReciever allData = {
    .ICData = &ICData,
    .EVData = &EVData,
    .lapData = &lapData
};

static const char *TAG = "main";

volatile bool receivedFlag = false;

/// HTTP response generation
cJSON* create_data_object(int index, double value) {
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "index", index);
    cJSON_AddNumberToObject(obj, "value", value);
    return obj;
}

static esp_err_t piggy(httpd_req_t *req) {
    // Create root cJSON object
    cJSON *root = cJSON_CreateObject();
    cJSON *data_array = cJSON_CreateArray();

    // Determine which data to send based on URI
    if (strcmp(req->uri, "/IC") == 0) {
        // Serving IC Data
        //ESP_LOGI(TAG, "Serving IC Data");
        cJSON_AddNumberToObject(root, "Length", allData.ICData->length);  // Add length to JSON
        for (int i = 0; i < allData.ICData->length; i++) {
            // Create data objects and add them to the array
            cJSON_AddItemToArray(data_array, create_data_object(i, allData.ICData->dataArray[i]));
        }
        // Add the array to the root object with the key "Data"
        cJSON_AddItemToObject(root, "Data", data_array);

    } else if (strcmp(req->uri, "/EV") == 0) {
        // Serving EV Data
        //ESP_LOGI(TAG, "Serving EV Data");
        cJSON_AddNumberToObject(root, "Length", allData.EVData->length);  // Add length to JSON
        for (int i = 0; i < allData.EVData->length; i++) {
            // Create data objects and add them to the array
            cJSON_AddItemToArray(data_array, create_data_object(i, allData.EVData->dataArray[i]));
        }
        // Add the array to the root object with the key "Data"
        cJSON_AddItemToObject(root, "Data", data_array);
        
    

    } else if (strcmp(req->uri, "/LAP") == 0) {
    // Lap Data
    ESP_LOGI(TAG, "Serving Lap Time Data");
        
        // Create a JSON object for the lap data
        cJSON *lap_data = cJSON_CreateObject();
        cJSON_AddNumberToObject(lap_data, "lap", allData.lapData->lap);
        cJSON_AddNumberToObject(lap_data, "minutes", allData.lapData->minutes);
        cJSON_AddNumberToObject(lap_data, "seconds", allData.lapData->seconds);
        cJSON_AddNumberToObject(lap_data, "microseconds", (allData.lapData->microSeconds)/1000);
        
        // Add the lap data object to the root object
        cJSON_AddItemToObject(root, "LapData", lap_data);
    } else {
        // Handle unknown URIs
        ESP_LOGI(TAG, "Unknown request URI: %s", req->uri);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Data not found");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // Convert cJSON object to string
    const char *response_str = cJSON_Print(root);
    if (response_str == NULL) {
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // Send the JSON response
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, response_str, HTTPD_RESP_USE_STRLEN);
    
    cJSON_Delete(root);
    free((void *)response_str);
    return ESP_OK;
}

/// LoRa message handling
void printLoRaMessage() {
    updateData(radio, &allData);
}

void packetReceived(void *nothing) {
    ESP_LOGI(TAG, "Initializing LoRa... ");
    int state = radio.begin(915.0F, 500.0F, 7U, 5U, 0x12U, 17, 8U, 0U);
    if (state != RADIOLIB_ERR_NONE) {
        ESP_LOGI(TAG, "failed, code %d\n", state);
        while (true) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    ESP_LOGI(TAG, "success!\n");
    state = radio.startReceive();
    radio.setPacketReceivedAction(printLoRaMessage);
    while (1) vTaskDelay(10000 / portTICK_PERIOD_MS);
}

extern "C" void app_main(void) {
    // Task to handle receiving packets
    xTaskCreatePinnedToCore(packetReceived, "packetReceived", 4096, NULL, 5, NULL, 1);

    // Initialize NVS flash for WiFi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi as access point
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    // HTTP server setup
    static const httpd_uri_t littlewolf = {
        .uri = "/IC",
        .method = HTTP_GET,
        .handler = piggy,
        .user_ctx = NULL
    };
    static const httpd_uri_t littlewolfs = {
        .uri = "/EV",
        .method = HTTP_GET,
        .handler = piggy,
        .user_ctx = NULL
    };
    static const httpd_uri_t littlewolves = {
        .uri = "/LAP",
        .method = HTTP_GET,
        .handler = piggy,
        .user_ctx = NULL
    };

    static httpd_handle_t server = NULL;
    static httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "HTTP Server Up");
        httpd_register_uri_handler(server, &littlewolf);
        httpd_register_uri_handler(server, &littlewolfs);
        httpd_register_uri_handler(server, &littlewolves);
    }

    // Start performance monitor
    //perfmon_start();

    while (1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);

    // Print values from lapData (assuming lapData is a single value or has a defined structure)
    //printf("IC Data: %d, recieved messages:%d\n", allData.ICData->dataArray[3],testvalue);

    }
}
