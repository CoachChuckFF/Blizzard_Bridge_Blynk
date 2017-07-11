#include "lib/blizzard_wifi.h"

#define EXAMPLE_WIFI_SSID "blizznet"
#define EXAMPLE_WIFI_PASS "destroyer"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static const char *TAG = "WIFI";

static EventGroupHandle_t wifi_event_group;

const int CONNECTED_BIT = BIT0;

static esp_err_t blizzard_wifi_event_handler(void *ctx, system_event_t *event)
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

void initialise_blizzard_wifi()
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(blizzard_wifi_event_handler, NULL) );
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
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");
}
/*
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
        for(i = 1; i < DMX_MAX_SLOTS; i++)
          setDMXData(i, i * j);
        j++;
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
}*/
