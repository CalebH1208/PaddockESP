#include "ESPHal.h"
#include "driver/twai.h"

static void configure_CAN();
static int send_CAN(uint32_t ID,uint8_t len,uint8_t * data);
static twai_message_t recieve_CAN();
void setupCANLED(uint32_t pin);
void blinkCANLED(uint32_t pin);
