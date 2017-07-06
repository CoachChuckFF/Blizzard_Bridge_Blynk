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
#include "Arduino.h"

static const char *TAG = "DMX UART EXAMPLE";

#define UART_DMX UART_NUM_2
#define DMX_TX_PIN 12
#define DMX_RX_PIN 14
#define DMX_DATA_BAUD		250000
#define DMX_BREAK_BAUD 	 88000

#define UART_BLIZZARD UART_NUM_1
#define BLIZZARD_TX_PIN 16
#define BLIZZARD_RX_PIN 34
#define BLIZZARD_BAUD		921600

#define DMX_STATE_START 0
#define DMX_STATE_DATA 1
#define DMX_STATE_BREAK 2
#define DMX_STATE_MAB 3
#define DMX_STATE_IDLE 4

#define EXAMPLE_WIFI_SSID "blizznet"
#define EXAMPLE_WIFI_PASS "destroyer"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

nvs_handle config_nvs_handle;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

int state;

uint8_t dmx[513];

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void test()
{
  uint16_t i = 0;
  uint8_t j = 3;
  uint8_t k = 140;
  uint8_t l = 210;
  uint8_t direction = RECEIVE;
  //xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                      //false, true, portMAX_DELAY);
  ESP_LOGI(TAG, "Connected to AP");
  setName((char*)"magic box", 9);
  setOwnUniverse(1);
  startDMXArtnet(direction);
  while(1)
  {
    switch(direction)
    {
      case SEND:
        /*for(i = 1; i < DMX_MAX_SLOTS; i++)
          setDMXData(i, i * j);
        j++;*/
        sendDMXDataArtnet(1);
        vTaskDelay(1000 / portTICK_RATE_MS);
      break;
      case RECEIVE:
        printf("\n------------ START -------------\n");

        for(i = 0; i < 513; i++)
        {
          if(!(i%32))
            printf("\n");
          if(getDMXData(i))
            printf(" %d-%d ", i, getDMXData(i));
        }
        printf("\n------------ END -------------\n");
        vTaskDelay(5000 / portTICK_RATE_MS);
      break;
    }

  }
}

void configure_uart()
{
  uart_config_t uart_config = {
    .baud_rate = DMX_DATA_BAUD,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122
  };

  uart_config_t uart_config_1 = {
    .baud_rate = BLIZZARD_BAUD,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122
  };

  ESP_LOGI(TAG, "UART config");

  uart_param_config(UART_DMX, &uart_config);
  uart_param_config(UART_BLIZZARD, &uart_config_1);

  uart_set_pin(UART_DMX, DMX_TX_PIN, DMX_RX_PIN,
    UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  uart_set_pin(UART_BLIZZARD, BLIZZARD_TX_PIN, BLIZZARD_RX_PIN,
    UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  uart_driver_install(UART_DMX, 1024 * 2, 0, 0, NULL, 0);
  uart_driver_install(UART_BLIZZARD, 1024 * 2, 0, 0, NULL, 0);
}

//initalize and look for flags
void app_main()
{
  uint8_t* data;
  int ret_val;

  configure_uart();
  //nvs_flash_init();
  //initialise_wifi();
  //xTaskCreate(&ArduinoLoop, "Arduino Core", 2048, NULL, 10, NULL);
  //vTaskDelay(10000 / portTICK_RATE_MS);

  ret_val = nvs_flash_init();
  if(ret_val != ESP_OK)
    ESP_LOGI(TAG, "NVS INIT FAILED %d", ret_val);

  ret_val = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &config_nvs_handle);
  if(ret_val != ESP_OK)
    ESP_LOGI(TAG, "NVS OPEN FAILED %d", ret_val);



  clearDMX();
  //setOwnUniverse(1);
  //startDMXArtnet(DMX_RECEIVE);
  //xTaskCreate(&test, "test", 2048, NULL, 5, NULL);

  ESP_LOGI(TAG, "MAIN STATE");
  state = DMX_STATE_START;

  for(;;)
  {
    uart_write_bytes(UART_BLIZZARD, (const char *) getDMXBuffer(), DMX_MAX_SLOTS);
    switch( state )
    {
        case DMX_STATE_START:
          uart_set_baudrate(UART_DMX, DMX_DATA_BAUD);
          uart_set_stop_bits(UART_DMX, UART_STOP_BITS_2);
          state = DMX_STATE_DATA;
        break;
        case DMX_STATE_DATA:
          uart_write_bytes(UART_DMX, (const char *) getDMXBuffer(), DMX_MAX_SLOTS);
          state = DMX_STATE_BREAK;
        break;
        case DMX_STATE_BREAK:
          uart_wait_tx_done(UART_DMX, 10);
          //vTaskDelay(3);
          uart_set_baudrate(UART_DMX, DMX_BREAK_BAUD);
          uart_set_stop_bits(UART_DMX, UART_STOP_BITS_1);
          uart_write_bytes(UART_DMX, (const char *) getDMXBuffer(), 1);
          state = DMX_STATE_MAB;
        break;
        case DMX_STATE_MAB:
          uart_wait_tx_done(UART_DMX, 10);
          //vTaskDelay(2);
          state = DMX_STATE_START;
        break;
    }

  }


}
