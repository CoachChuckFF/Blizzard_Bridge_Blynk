#include "lib/blizzard_wifi.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_wifi_types.h"
#include "lib/blizzard_wifi_manager.h"
#include <string.h>

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

#include "esp_log.h"
#include "soc/uart_struct.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static const char *TAG = "WIFI";

static EventGroupHandle_t wifi_event_group;

const int CONNECTED_BIT = BIT0;

static esp_err_t blizzard_wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG, "start");
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        get_wifi_ip();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      switch(event->event_info.disconnected.reason){
            case AUTH_EXPIRE:
              //password is bad - wifi manager
              ESP_LOGI(TAG, "Auth Expired");
              //ESP_ERROR_CHECK( esp_event_loop_set_cb(NULL, NULL) );
              //esp_wifi_deinit();
              //deinitalise_blizzard_wifi();
              //start_wifi_manager();
              setNeedWifiManager(ENABLE);
              esp_restart();
            break;
            case AUTH_FAIL:
              //password is bad - wifi manager
              ESP_LOGI(TAG, "Auth Failed");
            break;
            case NO_AP_FOUND:
              //ssid wrong - wifi manager
              ESP_LOGI(TAG, "AP not Found");
              setNeedWifiManager(ENABLE);
              esp_restart();
            break;
            default:
              ESP_LOGI(TAG, "Discon %d", event->event_info.disconnected.reason);
              esp_wifi_connect();
              xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        }
        break;
    default:
        ESP_LOGI(TAG, "WIFI EVENT %d", event->event_id);
        break;
    }
    return ESP_OK;
}

void initialise_blizzard_wifi(char* ssid, char* pass)
{


    tcpip_adapter_ip_info_t ipInfo;
    wifi_config_t wifi_config; /*= {
      .sta = {
        .ssid = "blizznet",
        .password = "destroyer"
      },
    };*/
    uint8_t i = 0;
    tcpip_adapter_init();
    /*if(DHCP_OPTION == DISABLED)
    {
      tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
      IP4_ADDR(&ipInfo.ip, 192,168,1,99);
      IP4_ADDR(&ipInfo.gw, 192,168,1,1);
      IP4_ADDR(&ipInfo.netmask, 255,255,255,0);
      tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
    }*/
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(blizzard_wifi_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    memset(&wifi_config, 0, sizeof(wifi_config));

    strcpy( (char *) wifi_config.sta.ssid, ssid );
    strcpy( (char *) wifi_config.sta.password, pass );

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_LOGI(TAG, "Setting WiFi configuration PASS %s...", wifi_config.sta.password);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");
}

void deinitalise_blizzard_wifi()
{
  esp_wifi_disconnect();
  esp_wifi_stop();
  esp_wifi_deinit();
}

uint32_t get_wifi_ip()
{
  uint32_t temp = 0;
  tcpip_adapter_ip_info_t ip;
  memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
  if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip) == 0) {
      ESP_LOGI(TAG, "~~~~~~~~~~~");
      ESP_LOGI(TAG, "WIFIIP:"IPSTR, IP2STR(&ip.ip));
      ESP_LOGI(TAG, "WIFIPMASK:"IPSTR, IP2STR(&ip.netmask));
      ESP_LOGI(TAG, "WIFIPGW:"IPSTR, IP2STR(&ip.gw));
      ESP_LOGI(TAG, "~~~~~~~~~~~");
  }

  //flip ip
  temp |= (ip.ip.addr >> 24) & 0x000000FF;
  temp |= (ip.ip.addr >> 8) & 0x0000FF00;
  temp |= (ip.ip.addr << 8) & 0x00FF0000;
  temp |= (ip.ip.addr << 24) & 0xFF000000;

  return temp;
}

void changeIP(uint8_t* ip)
{
  tcpip_adapter_ip_info_t ipInfo;
  tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
  IP4_ADDR(&ipInfo.ip, ip[3],ip[2],ip[1],ip[0]);
  IP4_ADDR(&ipInfo.gw, ip[3],ip[2],ip[1],1);
  IP4_ADDR(&ipInfo.netmask, getOwnNetmask()[0],getOwnNetmask()[1],getOwnNetmask()[2],getOwnNetmask()[3]);
  tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
  setOwnIPAddress(ip);
  get_wifi_ip();
}
