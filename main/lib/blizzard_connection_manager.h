/*
Blizzard connection manager - monitors active connection and will try to
reconnect on disconnect. Ethernet will always take precidance.

*/

#ifndef BLIZZARD_CONNECTION_MANAGER_H
#define BLIZZARD_CONNECTION_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <inttypes.h>
#include <stdio.h>

#include "rom/ets_sys.h"
#include "rom/gpio.h"

#include "soc/dport_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"

#include "eth_phy/phy_lan8720.h"
#include "esp_wifi.h"
#include "esp_eth.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_wifi_types.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "blizzard_nvs.h"
#include "blizzard_wifi_manager.h"

#define PIN_PHY_POWER 17
#define PIN_SMI_MDC   23
#define PIN_SMI_MDIO  18

#define DEFAULT_ETHERNET_PHY_CONFIG phy_lan8720_default_ethernet_config
#define CONFIG_PHY_USE_POWER_PIN

#define SSID_MAX_LENGTH 32
#define PASS_MAX_LENGTH 32

#define AUTH_EXPIRE 2
#define AUTH_FAIL 202
#define NO_AP_FOUND 201

extern uint8_t CONNECTED;
extern uint8_t WIFI_CONNECTED;
extern uint8_t ETH_CONNECTED;

//starts connection manager
void start_connection_manager(void);
void stop_connection_manager(void);
static esp_err_t blizzard_event_handler(void *ctx, system_event_t *event);

void start_blizzard_wifi(char* ssid, char* pass);
void stop_blizzard_wifi(void);

void start_blizzard_ethernet(void);
static void phy_device_power_enable_via_gpio(bool enable);
static void eth_gpio_config_rmii(void);

char* getSSID(void);
void setSSID(char* ssid, uint8_t length);
char* getPass(void);
void setPass(char* pass, uint8_t length);

uint8_t* getIP(void);
void setIP(uint8_t* ip);
uint8_t* getNetmask(void);
void setNetmask(uint8_t* nm);
uint8_t* getGateway(void);
void setGateway(uint8_t* gw);
uint8_t* getMac(void);
void setMac(uint8_t* mac);
uint8_t getDHCPEnable(void);
void setDHCPEnable(uint8_t);
uint8_t getWifiManagerEnable(void);
void setWifiManagerEnable(uint8_t enable);


void printConnectionInfo(void);

#ifdef __cplusplus
}
#endif

#endif
