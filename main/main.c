/* Uart Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "lib/dmx_artnet.h"
#include "lib/dmx_sACN.h"
#include "esp_log.h"
#include "soc/uart_struct.h"
#include "lib/dmx.h"
#include <stdio.h>
#include "lib/dmx_uart.h"
#include "lib/dmx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "lib/main_arduino.h"
#include "lib/blizzard_nvs.h"
//#include "Arduino.h"

static const char *TAG = "MAIN";



extern nvs_handle config_nvs_handle;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */


//initalize and look for flags
char d[32];



void app_main()
{
  uint8_t* data;
  esp_err_t ret_val;
  uint8_t i;
  size_t length;

  init_blizzard_nvs();

  populate_all_dmx_nvs_values();


  for(i = 0; i < 9; i++)
    print_nvs_values(i);

  //nvs_flash_init();
  //initialise_wifi();
  //xTaskCreate(&ArduinoLoop, "Arduino Core", 2048, NULL, 10, NULL);
  //vTaskDelay(10000 / portTICK_RATE_MS);


  clearDMX();
  //setOwnUniverse(1);
  //startDMXArtnet(DMX_RECEIVE);
  //xTaskCreate(&test, "test", 2048, NULL, 5, NULL);


}
