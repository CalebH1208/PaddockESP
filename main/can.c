#include "can.h"

static const char *CAN = "CAN";

static void configure_CAN(){
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21,GPIO_NUM_22,TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

     if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(CAN, "Driver initialized\n ");
    } else {
        ESP_LOGI(CAN, "Driver failed to initialize\n ");
        return;
    }
    if (twai_start() == ESP_OK) {
        ESP_LOGI(CAN, "Driver started\n ");

    } else {
        ESP_LOGI(CAN, "Failed to start driver\n ");

        return;
    }
}

static int send_CAN(uint32_t ID,uint8_t len,uint8_t * data){
    twai_message_t message;
    message.identifier = ID;
    message.extd = 0;
    message.data_length_code = len;
    for(int i =0;i<len;i++){
        message.data[i]=*(data+i);
    }
    if (twai_transmit(&message, pdMS_TO_TICKS(100)) == ESP_OK) {
        return 0; 
    } else {
        ESP_LOGI(CAN, "Can Send Failed");
        return 1;
    }
}
static twai_message_t recieve_CAN(){
    twai_message_t ret;
    ret.identifier = 0x00;
    if (twai_receive(&ret, pdMS_TO_TICKS(50)) == ESP_OK) {
    return ret;
} else {
    
    return ret;
}
}

void setupCANLED(uint32_t blink_gpio){
    esp_rom_gpio_pad_select_gpio(blink_gpio);
    gpio_set_direction((gpio_num_t)(blink_gpio), GPIO_MODE_OUTPUT);
    ESP_LOGI(CAN, "CANLED initialized");
}

void blinkCANLED(uint32_t blink_gpio){
    gpio_set_level((gpio_num_t)blink_gpio,1);
    vTaskDelay(200/portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t)blink_gpio,0);
}

