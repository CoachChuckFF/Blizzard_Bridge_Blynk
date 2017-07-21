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
#include "lib/blizzard_blynk.h"
#include "lib/blizzard_nvs.h"
#include "lib/blizzard_uart.h"
#include "lib/blizzard_structs.h"
#include "lib/blizzard_wifi.h"
#include "lib/blizzard_eth.h"
#include "lib/blizzard_wdmx.h"
//#include "Arduino.h"

static const char *TAG = "MAIN";

extern uint8_t device_name_changed;
extern uint8_t input_mode_changed;
extern uint8_t output_mode_changed;
extern uint8_t own_ip_changed;
extern uint8_t own_address_changed;
extern uint8_t own_universe_changed;
extern uint8_t slots_changed;

//initalize and look for flags
void app_main()
{
  uint8_t* data;
  esp_err_t ret_val;
  uint8_t i = 0;
  size_t length;

  //initallize

  init_blizzard_nvs();

  populate_all_dmx_nvs_values();

  //start with trying to connect to wifi
  //try to connect with nvs values
/*
  for(i = 0; i < 9; i++)
    print_nvs_values(i);*/


  initialise_blizzard_wifi(getSSID(), getPASS());
  setOwnIPAddress(get_wifi_ip());
  print_nvs_values(NVS_OWN_IP_ADDRESS_INDEX); //does not work yet
  initialise_blizzard_ethernet();
  /*for(i = 0; i < 9; i++)
    print_nvs_values(i);*/


  //nvs_flash_init();
  //initialise_blizzard_wifi();
  //xTaskCreate(&ArduinoLoop, "Arduino Core", 2048, NULL, 10, NULL);
  //vTaskDelay(10000 / portTICK_RATE_MS);

  //startBlizzardUart();
  //startDMXUart(SEND);

  //initialise_blizzard_ethernet();
  //vTaskDelay(5000);

  //startDMXArtnet(SEND);
  xTaskCreatePinnedToCore(&start_blynk, "BLYNK", 2048 * 6, NULL, tskIDLE_PRIORITY + 6, NULL, 0); //pinned to core 0
  //vTaskStartScheduler();

  clearDMX();
  //startDMXUart(RECEIVE);
  startDMXUart(SEND);
  //startDMXUart((getInputMode() == DMX_MODE) ? RECEIVE : SEND);
  startWDMX();
  //setOwnUniverse(1);


  //xTaskCreate(&test, "test", 2048, NULL, 5, NULL);

  //main loop - listens to flag changes and responds to flag changes
  /*startWDMX();
  switch_wdmx_button_on();*/

  //easy_wdmx_connect();
  //startDMXArtnet(RECEIVE);
  /* Listen for Config Changes */
  while(1)
  {
    if(input_mode_changed)
    {
      ESP_LOGI(TAG, "input mode changed to: %d", getInputMode());
      switch(getInputMode())
      {
        case DMX_MODE:
          changeDirectionDMXUart(RECEIVE);
          //stopDMXTx();
        break;
        case WDMX_MODE:
          //TODO be able to toggle wdmx
        break;
        case ARTNET_MODE:
          //startDMXArtnet(RECEIVE);
        break;
        case SACN_MODE:
          //startDMXsACN();//TODO add in recive mode
        break;
      }
      input_mode_changed = 0;
    }

    if(output_mode_changed)
    {
      ESP_LOGI(TAG, "output mode changed to: %d", getOutputMode());
      switch(getOutputMode())
      {
        case DMX_MODE:
          changeDirectionDMXUart(SEND);
          //stopDMXRx();
        break;
        case WDMX_MODE:
          //TODO be able to toggle wdmx
        break;
        case ARTNET_MODE:

          //startDMXArtnet(SEND);
        break;
        case SACN_MODE:
          //startDMXsACN();//TODO add in recive mode
        break;
      }
      output_mode_changed = 0;
    }



    //ESP_LOGI(TAG, "MAIN LOOP");
    //sendDMXDataArtnet(0);
    //delay for context switch
    //ESP_LOGI(TAG, "COLOR %d", get_wdmx_color());
    //ESP_LOGI(TAG, "DMX0 %d", getDMXData(0));
    printDMX();
    ESP_LOGI(TAG, "TICK: %d", i++);
    vTaskDelay(1000);
  }

}
