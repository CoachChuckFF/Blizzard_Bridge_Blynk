#include "lib/blizzard_connection_manager.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static const char *TAG = "CONNECTION MANAGER";

static EventGroupHandle_t wifi_event_group;

const int CONNECTED_BIT = BIT0;

uint8_t INIT_DONE = 0;

uint8_t CONNECTED;
uint8_t WIFI_CONNECTED;
uint8_t ETH_CONNECTED;

static char SSID[SSID_MAX_LENGTH];
static char PASS[PASS_MAX_LENGTH];

static uint8_t IP_ADDRESS[4];
static uint8_t NETMASK[4];
static uint8_t GATEWAY[4];
static uint8_t MAC[6];
static uint8_t DHCP_ENABLE;
static uint8_t WIFI_MANAGER_ENABLE;


void start_connection_manager(void)
{

  tcpip_adapter_init();

  if(getWifiManagerEnable() == ENABLE)
  {
    start_wifi_manager();
  }
  else
  {
    ESP_ERROR_CHECK( esp_event_loop_init(blizzard_event_handler, NULL) );
    start_blizzard_wifi(getSSID(), getPass());

    if(getDHCPEnable() == DISABLE)
    {
      setIP(getIP());
      setNetmask(getNetmask());
      setGateway(getGateway());
    }
    //
  }
  /*
  if(getDHCPEnable() == DISABLE)
  {
    changeIP(getIP()); //set to last static ip address
  }
  else
  {
    temp_ip = get_wifi_ip();
    setOwnIPAddress((uint8_t *) &temp_ip); //set DHCP address
  }*/

  INIT_DONE = 1;

  printConnectionInfo();

}

void stop_connection_manager(void)
{
  stop_blizzard_wifi();
}

static esp_err_t blizzard_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG, "start");
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        printConnectionInfo();
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
              setWifiManagerEnable(ENABLE);
              esp_restart();
            break;
            case AUTH_FAIL:
              //password is bad - wifi manager
              ESP_LOGI(TAG, "Auth Failed");
            break;
            case NO_AP_FOUND:
              //ssid wrong - wifi manager
              ESP_LOGI(TAG, "AP not Found");
              setWifiManagerEnable(ENABLE);
              esp_restart();
            break;
            default:
              ESP_LOGI(TAG, "Discon %d", event->event_info.disconnected.reason);
              esp_wifi_connect();
              xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        }
        break;
    case SYSTEM_EVENT_WIFI_READY:
        ESP_LOGI(TAG,"WIFI READY");
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        ESP_LOGI(TAG,"SCAN DONE");
        break;
    case SYSTEM_EVENT_STA_STOP:
        ESP_LOGI(TAG,"STA STOP");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG,"STA CONNECTED");
        break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        ESP_LOGI(TAG,"STA AUTH CHANGE");
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        ESP_LOGI(TAG,"LOST IP");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        ESP_LOGI(TAG,"STA WPS ER SUCCESS");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        ESP_LOGI(TAG,"STA WPS ER FAILED");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        ESP_LOGI(TAG,"STA WPS ER TIMEOUT");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
        ESP_LOGI(TAG,"STA WPS ER PIN");
        break;
    case SYSTEM_EVENT_AP_START:
        ESP_LOGI(TAG,"AP START");
        break;
    case SYSTEM_EVENT_AP_STOP:
        ESP_LOGI(TAG,"AP STOP");
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG,"AP STACONNECTED");
        break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
        ESP_LOGI(TAG,"AP PROBEREQRECVED");
        break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
        ESP_LOGI(TAG,"AP STA GOT IP6");
        break;
    case SYSTEM_EVENT_ETH_START:
        ESP_LOGI(TAG,"ETH START");
        break;
    case SYSTEM_EVENT_ETH_STOP:
        ESP_LOGI(TAG,"ETH STOP");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        ESP_LOGI(TAG,"ETH CONNECTED");
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        ESP_LOGI(TAG,"ETH DISCONNECTED");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        ESP_LOGI(TAG,"GOT IP");
        break;
    case SYSTEM_EVENT_MAX:
        ESP_LOGI(TAG,"EVENT MAX");
        break;

    default:
        ESP_LOGI(TAG, "EVENT %d", event->event_id);
        break;
    }
    return ESP_OK;
}

// ----------------------------------- WIFI ------------------------------------

void start_blizzard_wifi(char* ssid, char* pass)
{
    tcpip_adapter_ip_info_t ipInfo;
    wifi_config_t wifi_config;
    uint8_t i = 0;

    wifi_event_group = xEventGroupCreate();
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

void stop_blizzard_wifi()
{
  esp_wifi_disconnect();
  esp_wifi_stop();
  esp_wifi_deinit();
}

// -------------------------------- ETHERNET -----------------------------------

void start_blizzard_ethernet()
{
    esp_err_t ret = ESP_OK;
    esp_event_loop_init(NULL, NULL);

    eth_config_t config = DEFAULT_ETHERNET_PHY_CONFIG;
    //Set the PHY address in the example configuration
    config.phy_addr = PHY1;
    config.gpio_config = eth_gpio_config_rmii;
    config.tcpip_input = tcpip_adapter_eth_input;

#ifdef CONFIG_PHY_USE_POWER_PIN
    //Replace the default 'power enable' function with an example-specific
       //one that toggles a power GPIO.
    config.phy_power_enable = phy_device_power_enable_via_gpio;
#endif

    ret = esp_eth_init(&config);

    if(ret == ESP_OK) {
        esp_eth_enable();
        //xTaskCreate(eth_task, "eth_task", 2048, NULL, (tskIDLE_PRIORITY + 2), NULL);
    }
    //vTaskDelay(20000 / portTICK_PERIOD_MS);


}

#ifdef CONFIG_PHY_USE_POWER_PIN

static void phy_device_power_enable_via_gpio(bool enable)
{
    assert(DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable);

    if (!enable) {
        /* Do the PHY-specific power_enable(false) function before powering down */
        DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable(false);
    }

    gpio_pad_select_gpio(PIN_PHY_POWER);
    gpio_set_direction(PIN_PHY_POWER,GPIO_MODE_OUTPUT);
    if(enable == true) {
        gpio_set_level(PIN_PHY_POWER, 1);
        ESP_LOGD(TAG, "phy_device_power_enable(TRUE)");
    } else {
        gpio_set_level(PIN_PHY_POWER, 0);
        ESP_LOGD(TAG, "power_enable(FALSE)");
    }

    // Allow the power up/down to take effect, min 300us
    vTaskDelay(1);

    if (enable) {
        /* Run the PHY-specific power on operations now the PHY has power */
        DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable(true);
    }
}
#endif

static void eth_gpio_config_rmii(void)
{
    // RMII data pins are fixed:
    // TXD0 = GPIO19
    // TXD1 = GPIO22
    // TX_EN = GPIO21
    // RXD0 = GPIO25
    // RXD1 = GPIO26
    // CLK == GPIO0
    phy_rmii_configure_data_interface_pins();
    // MDC is GPIO 23, MDIO is GPIO 18
    phy_rmii_smi_configure_pins(PIN_SMI_MDC, PIN_SMI_MDIO);
}

void eth_task(void *pvParameter)
{
    tcpip_adapter_ip_info_t ip;
    memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (1) {

        vTaskDelay(2000 / portTICK_PERIOD_MS);

        if (tcpip_adapter_get_ip_info(ESP_IF_ETH, &ip) == 0) {
            ESP_LOGI(TAG, "~~~~~~~~~~~");
            ESP_LOGI(TAG, "ETHIP:"IPSTR, IP2STR(&ip.ip));
            ESP_LOGI(TAG, "ETHPMASK:"IPSTR, IP2STR(&ip.netmask));
            ESP_LOGI(TAG, "ETHPGW:"IPSTR, IP2STR(&ip.gw));
            ESP_LOGI(TAG, "~~~~~~~~~~~");
        }
    }
}

/* -------------------------------- GETTERS/SETTERS --------------------------*/

char* getSSID(void)
{
  return SSID;
}

void setSSID(char *ssid, uint8_t length)
{
  uint8_t i;
  if(ssid == NULL)
  {
    ESP_LOGI(TAG, "NULL SSID");
    return;
  }
  if(length - 2 > SSID_MAX_LENGTH)
  {
    ESP_LOGI(TAG, "SSID length too long");
    return;
  }
  if(!length)
  {
    length = SSID_MAX_LENGTH - 1;
  }

  for(i = 0; i < length; i++)
  {
    SSID[i] = ssid[i];
    if(ssid[i] == '\0')
      break;
  }
  SSID[i] = '\0'; //null termination
  update_str_nvs_val(NVS_SSID_KEY, (char *) SSID);
}

char* getPass(void)
{
  return PASS;
}

void setPass(char *pass, uint8_t length)
{
  uint8_t i;
  if(pass == NULL)
  {
    ESP_LOGI(TAG, "NULL PASS");
    return;
  }
  if(length - 2 > PASS_MAX_LENGTH)
  {
    ESP_LOGI(TAG, "PASS length too long");
    return;
  }
  if(!length)
  {
    length = PASS_MAX_LENGTH - 1;
  }

  for(i = 0; i < length; i++)
  {
    PASS[i] = pass[i];
    if(pass[i] == '\0')
      break;
  }
  PASS[i] = '\0'; //null termination

  update_str_nvs_val(NVS_PASS_KEY, (char *) PASS);
}

uint8_t* getIP()
{
  tcpip_adapter_ip_info_t ip_info;

  if(!DHCP_ENABLE)
    return IP_ADDRESS;

  tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip_info);

  IP_ADDRESS[3] = (ip_info.ip.addr >> 24) & 0x000000FF;
  IP_ADDRESS[2] = (ip_info.ip.addr >> 16) & 0x000000FF;
  IP_ADDRESS[1] = (ip_info.ip.addr >> 8) & 0x000000FF;
  IP_ADDRESS[0] = (ip_info.ip.addr) & 0x000000FF;

  return IP_ADDRESS;
}

void setIP(uint8_t* ip)
{
  tcpip_adapter_ip_info_t ip_info;
  if(!INIT_DONE)
  {
    IP_ADDRESS[3] = ip[3];
    IP_ADDRESS[2] = ip[2];
    IP_ADDRESS[1] = ip[1];
    IP_ADDRESS[0] = ip[0];
    return;
  }


  if(ip == NULL)
  {
    ESP_LOGI(TAG, "NULL IP Address");
    return;
  }

  if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip_info) == 0) {
    tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
    IP4_ADDR(&ip_info.ip, ip[3],ip[2],ip[1],ip[0]);
    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);

    update_blob_nvs_val(NVS_OWN_IP_ADDRESS_KEY, ip, 4);

    IP_ADDRESS[3] = ip[3];
    IP_ADDRESS[2] = ip[2];
    IP_ADDRESS[1] = ip[1];
    IP_ADDRESS[0] = ip[0];
  }

}

uint8_t* getNetmask()
{
  tcpip_adapter_ip_info_t ip_info;

  if(!DHCP_ENABLE)
    return NETMASK;

  tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip_info);

  NETMASK[3] = (ip_info.netmask.addr >> 24) & 0x000000FF;
  NETMASK[2] = (ip_info.netmask.addr >> 16) & 0x000000FF;
  NETMASK[1] = (ip_info.netmask.addr >> 8) & 0x000000FF;
  NETMASK[0] = (ip_info.netmask.addr) & 0x000000FF;

  return NETMASK;
}

void setNetmask(uint8_t* netmask)
{
  tcpip_adapter_ip_info_t ip_info;

  if(!INIT_DONE)
  {
    NETMASK[3] = netmask[3];
    NETMASK[2] = netmask[2];
    NETMASK[1] = netmask[1];
    NETMASK[0] = netmask[0];
    return;
  }


  if(netmask == NULL)
  {
    ESP_LOGI(TAG, "NULL Netmask");
    return;
  }

  if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip_info) == 0) {
    tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
    IP4_ADDR(&ip_info.netmask, netmask[3], netmask[2], netmask[1], netmask[0]);
    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);

    update_blob_nvs_val(NVS_OWN_NETMASK_KEY, netmask, 4);

    NETMASK[3] = netmask[3];
    NETMASK[2] = netmask[2];
    NETMASK[1] = netmask[1];
    NETMASK[0] = netmask[0];
  }

}

uint8_t* getGateway()
{
  tcpip_adapter_ip_info_t ip_info;

  if(!DHCP_ENABLE)
    return GATEWAY;

  tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip_info);

  GATEWAY[3] = (ip_info.gw.addr >> 24) & 0x000000FF;
  GATEWAY[2] = (ip_info.gw.addr >> 16) & 0x000000FF;
  GATEWAY[1] = (ip_info.gw.addr >> 8) & 0x000000FF;
  GATEWAY[0] = (ip_info.gw.addr) & 0x000000FF;

  return GATEWAY;
}

void setGateway(uint8_t* gateway)
{
  tcpip_adapter_ip_info_t ip_info;

  if(!INIT_DONE)
  {
    GATEWAY[3] = gateway[3];
    GATEWAY[2] = gateway[2];
    GATEWAY[1] = gateway[1];
    GATEWAY[0] = gateway[0];
    return;
  }

  if(gateway == NULL)
  {
    ESP_LOGI(TAG, "NULL Gateway");
    return;
  }

  if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip_info) == 0) {
    tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
    IP4_ADDR(&ip_info.gw, gateway[3], gateway[2], gateway[1], gateway[0]);
    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);

    update_blob_nvs_val(NVS_GATEWAY_KEY, gateway, 4);

    GATEWAY[3] = gateway[3];
    GATEWAY[2] = gateway[2];
    GATEWAY[1] = gateway[1];
    GATEWAY[0] = gateway[0];
  }
}

uint8_t* getMac(void)
{
  return MAC;
}

void setMac(uint8_t* mac)
{
  if(mac == NULL)
  {
    ESP_LOGI(TAG, "NULL MAC");
    return;
  }

  update_blob_nvs_val(NVS_MAC_KEY, mac, 6);

  MAC[5] = mac[5];
  MAC[4] = mac[4];
  MAC[3] = mac[3];
  MAC[2] = mac[2];
  MAC[1] = mac[1];
  MAC[0] = mac[0];

}

uint8_t getDHCPEnable(void)
{
  return DHCP_ENABLE;
}

void setDHCPEnable(uint8_t enable)
{
  DHCP_ENABLE = enable;
  update_u8_nvs_val(NVS_DHCP_ENABLE_KEY, DHCP_ENABLE);
}

uint8_t getWifiManagerEnable(void)
{
  return WIFI_MANAGER_ENABLE;
}

void setWifiManagerEnable(uint8_t enable)
{
  WIFI_MANAGER_ENABLE = enable;
  update_u8_nvs_val(NVS_WIFI_MANAGER_ENABLE_KEY, WIFI_MANAGER_ENABLE);
}

void printConnectionInfo(void)
{
  uint32_t temp = 0;
  tcpip_adapter_ip_info_t ip;
  memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
  if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip) == 0) {
      ESP_LOGI(TAG, "~~~~~~~~~~~");
      ESP_LOGI(TAG, "IP:"IPSTR, IP2STR(&ip.ip));
      ESP_LOGI(TAG, "Netmask:"IPSTR, IP2STR(&ip.netmask));
      ESP_LOGI(TAG, "Gateway:"IPSTR, IP2STR(&ip.gw));
      ESP_LOGI(TAG, "MAC %02X-%02X-%02X-%02X-%02X-%02X",
                                                          MAC[5],
                                                          MAC[4],
                                                          MAC[3],
                                                          MAC[2],
                                                          MAC[1],
                                                          MAC[0]);
      ESP_LOGI(TAG, "DHCP: %s", (getDHCPEnable()) ? "Enabled" : "Disabled");
      ESP_LOGI(TAG, "Connected: %s", (CONNECTED) ? "Yes" : "No");
      if(CONNECTED){
        ESP_LOGI(TAG, "Connection Type: %s", (WIFI_CONNECTED) ? "Wifi" : "Ethernet");
      }
      ESP_LOGI(TAG, "~~~~~~~~~~~");
  }
}
