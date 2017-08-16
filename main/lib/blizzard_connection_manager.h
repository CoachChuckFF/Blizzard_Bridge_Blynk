#ifndef BLIZZARD_CONNECTION_MANAGER_H
#define BLIZZARD_CONNECTION_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_wifi_types.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "dmx_artnet.h"
#include "dmx_sACN.h"
#include "esp_log.h"
#include "soc/uart_struct.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define AUTH_EXPIRE 2
#define AUTH_FAIL 202
#define NO_AP_FOUND 201

static esp_err_t blizzard_wifi_event_handler(void *ctx, system_event_t *event);
void initialise_blizzard_wifi(char * ssid, char * pass);
void deinitalise_blizzard_wifi(void);
uint32_t get_wifi_ip(void);
void changeIP(uint8_t* ip);

#ifdef __cplusplus
}
#endif

#endif
