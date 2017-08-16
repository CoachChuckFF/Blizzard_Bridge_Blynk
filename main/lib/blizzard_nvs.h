#ifndef BLIZZARD_NVS_H
#define BLIZZARD_NVS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <inttypes.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "dmx.h"
#include "rdm.h"

//NVS Stuff
#define NVS_NAMESPACE "Blizzard"
#define NVS_DEVICE_NAME_KEY "DEVICE_NAME"
#define NVS_SSID_KEY "SSID"
#define NVS_PASS_KEY "PASS"
#define NVS_INPUT_MODE_KEY "INPUT_MODE"
#define NVS_OUTPUT_MODE_KEY "OUTPUT_MODE"
#define NVS_OWN_IP_ADDRESS_KEY "OWN_IP_ADDRESS"
#define NVS_OWN_NETMASK_KEY "OWN_NETMASK"
#define NVS_OWN_ADDRESS_KEY "OWN_ADDRESS"
#define NVS_OWN_UNIVERSE_KEY "OWN_UNIVERSE"
#define NVS_SLOTS_KEY "SLOTS"
#define NVS_OWN_UUID_KEY "RDM_UUID"
#define NVS_OWN_ID_KEY "OWN_ID"
#define NVS_DHCP_ENABLE_KEY "DHCP_ENABLE"
#define NVS_NEED_WIFI_MANAGER_KEY "NEED_WIFI"

#define NVS_DEVICE_NAME_INDEX 0
#define NVS_SSID_INDEX 1
#define NVS_PASS_INDEX 2
#define NVS_INPUT_MODE_INDEX 3
#define NVS_OUTPUT_MODE_INDEX 4
#define NVS_OWN_IP_ADDRESS_INDEX 5
#define NVS_OWN_NETMASK_INDEX 11
#define NVS_OWN_ADDRESS_INDEX 6
#define NVS_OWN_UNIVERSE_INDEX 7
#define NVS_SLOTS_INDEX 8
#define NVS_OWN_UUID_INDEX 9
#define NVS_OWN_ID_INDEX 10
#define NVS_DHCP_ENABLE_INDEX 12
#define NVS_NEED_WIFI_MANAGER_INDEX 13

extern nvs_handle config_nvs_handle;

//Default Values
#define DEFAULT_DEVICE_NAME "Mr Blink Blink"
#define DEFAULT_SSID "ssid"
#define DEFAULT_PASS "password"
#define DEFAULT_OUTPUT_MODE DMX_MODE
#define DEFAULT_INPUT_MODE ARTNET_MODE
#define DEFAULT_OWN_IP "192.168.1.1" //not used anywhere - for reference only
#define DEFAULT_OWN_NETMASK "255.255.255.0" //not used anywhere - for refrence only
#define DEFAULT_OWN_ADDRESS 0
#define DEFAULT_OWN_UNIVERSE 0
#define DEFAULT_SLOTS DMX_MAX_SLOTS
#define DEFAULT_OWN_ID 1
#define DEFAULT_DHCP_ENABLE_KEY ENABLE
#define DEFAULT_NEED_WIFI_MANAGER ENABLE

void init_blizzard_nvs(void); //you must call this before other nvs
void init_nvs_key_pair_default(uint8_t index);
void populate_all_dmx_nvs_values(void);
void update_u8_nvs_val(const char* key, uint8_t value);
void update_u16_nvs_val(const char* key, uint8_t value);
void update_blob_nvs_val(const char* key, uint8_t* value, uint8_t length);
void update_str_nvs_val(const char* key, char* value);
void print_nvs_values(uint8_t index);



#ifdef __cplusplus
}
#endif

#endif
