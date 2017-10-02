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
#include "driver/spi_master.h"
#include "lib/blizzard_blynk.h"
#include "lib/blizzard_nvs.h"
#include "lib/blizzard_uart.h"
#include "lib/blizzard_structs.h"
#include "lib/blizzard_wdmx.h"
#include "lib/blizzard_rdm.h"
#include "lib/blizzard_connection_manager.h"
#include "Arduino.h"

#define MISO 32;
#define MOSI 16;
#define SCLK 4;
#define CS1 5;
#define CS2 2;
#define MASTER_SWITCH 39;

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

  /* spi interface
  int ret;
  gpio_config_t io_conf;
  spi_device_handle_t spi;
  spi_bus_config_t buscfg = {
    .miso_io_num = 32,
    .mosi_io_num = 16,
    .sclk_io_num = 4,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1
  };
  spi_device_interface_config_t devcfg = {
    .clock_speed_hz = 10000000,
    .mode = 0,
    .spics_io_num = 5,
    .queue_size = 3
  };
  spi_device_interface_config_t devcfg1 = {
    .clock_speed_hz = 10000000,
    .mode = 0,
    .spics_io_num = 2,
    .queue_size = 3
  };
  */


  //initallize
  init_blizzard_nvs();

  init_nvs_key_pair_default(NVS_MAC_INDEX);

  populate_all_dmx_nvs_values();

/*
  for(i = 0; i < 9; i++)
    print_nvs_values(i);*/

  //setSSID("blizznet", 8);
  //setPASS("destroyer", 9);

  //TODO Make Ethernet workflow
  start_blizzard_ethernet();

  //start_connection_manager();

  //xTaskCreatePinnedToCore(&start_blynk, "BLYNK", 2048 * 6, NULL, tskIDLE_PRIORITY + 6, NULL, 0); //pinned to core 0

  //vTaskDelay(15000);

/*
  temp_ip = get_wifi_ip();
  setOwnIPAddress((uint8_t *) &temp_ip); //set DHCP address
*/

  clearDMX();
  startDMXUart(SEND);

  printConnectionInfo();

  startWDMX();
  ESP_LOGI(TAG, "3");
  startDMXArtnet(RECEIVE);
  ESP_LOGI(TAG, "2");
  turn_on_wdmx();
  /* // SPI interface
  ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
  assert(ret == ESP_OK);

  ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
  assert(ret == ESP_OK);

  ret = spi_bus_add_device(HSPI_HOST, &devcfg1, &spi);
  assert(ret == ESP_OK);

  //LED gpio setup
  if(!GPIO_IS_VALID_GPIO(39))
      ESP_LOGI(TAG, "Invalid GPIO");

  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = GPIO_SEL_39;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;
  gpio_config(&io_conf);
  */

  /* Listen for Config Changes */
  ESP_LOGI(TAG, "1");
  while(1)
  {
  /*  if(input_mode_changed)
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
    }*/

    //ESP_LOGI(TAG, "TICK: %d", i++);
    //printDMX();

    vTaskDelay(100);
  }

}
