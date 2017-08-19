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
#include "lib/blizzard_rdm.h"
#include "lib/blizzard_wifi_manager.h"
#include "Arduino.h"

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
  uint32_t temp_ip;
  uint8_t i = 0;
  size_t length;


  //initallize
  init_blizzard_nvs();

  populate_all_dmx_nvs_values();

  for(i = 0; i < 9; i++)
    print_nvs_values(i);

  //setSSID("blizznet", 8);
  //setPASS("destroyer", 9);

  //TODO Make Ethernet workflow
  //initialise_blizzard_ethernet();

  //setNeedWifiManager(ENABLE);

  if(getNeedWifiManager() == ENABLE)
  {
    start_wifi_manager();
  }
  else
  {
    initialise_blizzard_wifi(getSSID(), getPASS());
  }

  xTaskCreatePinnedToCore(&start_blynk, "BLYNK", 2048 * 6, NULL, tskIDLE_PRIORITY + 6, NULL, 0); //pinned to core 0

  //vTaskDelay(15000);

/*
  temp_ip = get_wifi_ip();
  setOwnIPAddress((uint8_t *) &temp_ip); //set DHCP address
*/

  setDHCPEnable(ENABLE);

  if(getDHCPEnable() == DISABLE)
  {
    changeIP(getOwnIPAddress()); //set to last static ip address
  }
  else
  {
    temp_ip = get_wifi_ip();
    setOwnIPAddress((uint8_t *) &temp_ip); //set DHCP address
  }

  get_wifi_ip();

  clearDMX();
  startDMXUart(RECEIVE);

  startWDMX();
  startDMXArtnet(RECEIVE);

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
          changeDirectionArtnet(RECEIVE);
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

          changeDirectionArtnet(SEND);
        break;
        case SACN_MODE:
          //startDMXsACN();//TODO add in recive mode
        break;
      }
      output_mode_changed = 0;
    }

    //ESP_LOGI(TAG, "TICK: %d", i++);
    //get_wifi_ip();
    vTaskDelay(1000);
  }

}
