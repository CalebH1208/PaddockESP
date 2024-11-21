#include <stdio.h>
#include <string.h>
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define SSID      "CANs is Bussin"
#define PASS      "squigglen"
#define CHAN  1
#define MAXC       4

void wifi_init_softap(void);
static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data);
