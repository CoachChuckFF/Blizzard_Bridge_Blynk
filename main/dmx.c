/* dmx.c
   Copyright 2017 by Christian Krueger

Copyright (c) 2017, Christian Krueger
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of esp32_dmx nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------------

  Generic DMX functions. Think of it as a C function without the CPP overhead.
  DMX is a uint8_t buffer that is directly accessed by the functions in this file
  The buffer should be accessed through the functions provided.

  !!! NOTE !!!
  DMX addressing starts at index 1 not 0, Index 0 is for the start byte!

  DMX start byte = DMX[0]
  DMX channel 1 = DMX[1]
  DMX channel 2 = DMX[2]

  Make thread safe?
 */
#include <stdio.h>
#include <inttypes.h>
#include "esp_log.h"
#include "lib/blizzard_nvs.h"
#include "lib/dmx.h"

static const char *TAG = "DMX";

//NVS Handle
extern nvs_handle config_nvs_handle;

static volatile uint8_t DMX[DMX_MAX_SLOTS];
static char DEVICE_NAME[NAME_MAX_LENGTH];
static char SSID[SSID_MAX_LENGTH];
static char PASS[PASS_MAX_LENGTH];
static uint8_t OWN_UUID[6];
static uint8_t INPUT_MODE; //ethernet, wifi, dmx
static uint8_t OUTPUT_MODE; //Arthnet, sACN, Blynk
static uint8_t MEDIUM;
static uint8_t OWN_IP_ADDRESS[4];
static uint8_t OWN_NETMASK[4];
static uint16_t OWN_ADDRESS;
static uint16_t OWN_UNIVERSE;
static uint16_t SLOTS;
static uint16_t OWN_ID;
static uint8_t DHCP_ENABLE;
static uint8_t NEED_WIFI_MANAGER;

//flags
uint8_t device_name_changed = 0;
uint8_t input_mode_changed = 0;
uint8_t output_mode_changed = 0;
uint8_t own_ip_changed = 0;
uint8_t own_address_changed = 0;
uint8_t own_universe_changed = 0;
uint8_t slots_changed = 0;

char* getName()
{
  return DEVICE_NAME;
}

//length is assuming the user did not include a null termination
void setName(char *name, uint8_t length)
{
  uint8_t i;
  if(name[0] == '/0')
  {
    ESP_LOGI(TAG, "Null string")
    return;
  }
  if(length >= NAME_MAX_LENGTH)
  {
    ESP_LOGI(TAG, "Name too long %d", length);
    return;
  }

  memset(DEVICE_NAME, 0, NAME_MAX_LENGTH);

  for(i = 0; i < length; i++)
  {
    if(name[i] == '\0')
      break;
    DEVICE_NAME[i] = name[i];
  }
  DEVICE_NAME[i] = '\0'; //null termination
  update_str_nvs_val(NVS_DEVICE_NAME_KEY, (char *) DEVICE_NAME);
}

void setOwnUUID(uint8_t * uuid)
{
  uint8_t i;

  for(i = 0; i < 6; i++)
    OWN_UUID[i] = uuid[i];

  //add in update_blob_nvs_val
}

uint8_t * getOwnUUID(void)
{
  return OWN_UUID;
}

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
  if(length - 1 > SSID_MAX_LENGTH)
  {
    ESP_LOGI(TAG, "SSID length too long");
    return;
  }

  for(i = 0; i < length; i++)
    SSID[i] = ssid[i];
  SSID[i] = '\0'; //null termination
  update_str_nvs_val(NVS_SSID_KEY, (char *) SSID);
}

char* getPASS(void)
{
  return PASS;
}

void setPASS(char *pass, uint8_t length)
{
  uint8_t i;
  if(pass == NULL)
  {
    ESP_LOGI(TAG, "NULL PASS");
    return;
  }
  if(length - 1 > PASS_MAX_LENGTH)
  {
    ESP_LOGI(TAG, "PASS length too long");
    return;
  }

  for(i = 0; i < length; i++)
    PASS[i] = pass[i];
  PASS[i] = '\0'; //null termination
  update_str_nvs_val(NVS_PASS_KEY, (char *) PASS);
}

uint8_t getMedium(void)
{
  return MEDIUM;
}

void setMedium(uint8_t medium)
{
  MEDIUM = medium;
}

uint8_t getInputMode(void)
{
  return INPUT_MODE;
}

void setInputMode(uint8_t input_mode)
{
  INPUT_MODE = input_mode;
  input_mode_changed = 1;
  update_u8_nvs_val(NVS_INPUT_MODE_KEY, INPUT_MODE);
}

uint8_t getOutputMode(void)
{
  return OUTPUT_MODE;
}

void setOutputMode(uint8_t output_mode)
{
  OUTPUT_MODE = output_mode;
  output_mode_changed = 1;
  update_u8_nvs_val(NVS_OUTPUT_MODE_KEY, OUTPUT_MODE);
}

uint8_t* getOwnIPAddress()
{
  return OWN_IP_ADDRESS;
}

void setOwnIPAddress(uint8_t* address)
{

  if(address == NULL)
  {
    ESP_LOGI(TAG, "NULL IP Address");
    return;
  }
  OWN_IP_ADDRESS[3] = address[3];
  OWN_IP_ADDRESS[2] = address[2];
  OWN_IP_ADDRESS[1] = address[1];
  OWN_IP_ADDRESS[0] = address[0];

  update_blob_nvs_val(NVS_OWN_IP_ADDRESS_KEY, address, sizeof(uint8_t) * 4);

  own_ip_changed = 1;
}

uint8_t* getOwnNetmask()
{
  return OWN_NETMASK;
}

void setOwnNetmask(uint8_t* netmask)
{
  OWN_NETMASK[3] = netmask[0];
  OWN_NETMASK[2] = netmask[1];
  OWN_NETMASK[1] = netmask[2];
  OWN_NETMASK[0] = netmask[3];
}

uint16_t getOwnAddress(void)
{
  return OWN_ADDRESS;
}

void setOwnAddress(uint16_t address)
{
  OWN_ADDRESS = address;
}

uint16_t getOwnUniverse()
{
  return OWN_UNIVERSE;
}

void setOwnUniverse(uint16_t universe)
{
  OWN_UNIVERSE = universe;
}

uint16_t getSlots()
{
  return SLOTS;
}

void setSlots(uint16_t count)
{
  if(count >= DMX_MAX_SLOTS)
  {
      ESP_LOGI(TAG, "Too many slots =!= %d", count);
      return;
  }
  if(count < DMX_MIN_SLOTS)
  {
      ESP_LOGI(TAG, "Too few slots =!= %d", count);
      return;
  }

  SLOTS = count;
}

uint16_t getOwnID(void)
{
  return OWN_ID;
}

void setOwnID(uint16_t id)
{
  OWN_ID = id;
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

uint8_t getNeedWifiManager(void)
{
  return NEED_WIFI_MANAGER;
}

void setNeedWifiManager(uint8_t need)
{
  NEED_WIFI_MANAGER = need;
  update_u8_nvs_val(NVS_NEED_WIFI_MANAGER_KEY, NEED_WIFI_MANAGER);
}

uint8_t getDMXData(uint16_t slot)
{
  if(slot >= DMX_MAX_SLOTS)
  {
      ESP_LOGI(TAG, "DMX (get) out of bounds DMX[%d]", slot);
      return 0;
  }

  return DMX[slot];
}

void setDMXData(uint16_t slot, uint8_t value)
{
  if(slot >= DMX_MAX_SLOTS)
  {
      ESP_LOGI(TAG, "DMX (set) out of bounds DMX[%d]", slot);
      return;
  }

  DMX[slot] = value;
}

uint8_t* getDMXBuffer()
{
  return DMX;
}


void clearDMX(void)
{
  int i;
  for(i = 0; i < DMX_MAX_SLOTS; i++)
    DMX[i] = 0;
}

/* Be carful this does not max out the start byte (index 0) */
void maxDMX(void)
{
  int i;
  for(i = 1; i < DMX_MAX_SLOTS; i++)
    DMX[i] = 0xFF;
}

void copyToDMX(uint8_t *buf, uint16_t start_index, uint16_t length)
{
  int i, j;
  if(buf == NULL)
  {
    ESP_LOGI(TAG, "Copy to DMX, NULL Buffer");
    return;
  }
  if(start_index >= DMX_MAX_SLOTS)
  {
    ESP_LOGI(TAG, "DMX (copy to) out of bounds DMX[%d]", start_index);
    return;
  }

  for(i = start_index, j = 0; i < DMX_MAX_SLOTS; i++, j ++, length--)
    if(!length)
      break;
    DMX[i] = buf[j];
}

void copyFromDMX(uint8_t *buf, uint16_t start_index, uint16_t length)
{
  int i, j;
  if(buf == NULL)
  {
    ESP_LOGI(TAG, "Copy from DMX, NULL Buffer");
    return;
  }
  if(start_index >= DMX_MAX_SLOTS)
  {
    ESP_LOGI(TAG, "DMX (copy from) out of bounds DMX[%d]", start_index);
    return;
  }

  for(i = start_index, j = 0; i < DMX_MAX_SLOTS; i++, j ++, length--)
    if(!length)
      break;
    buf[j] = DMX[i];
}

void printDMX()
{
  uint16_t i;
  printf("-----------------DMX DATA---------------------\n");
  for(i = 0; i < DMX_MAX_SLOTS; i++)
  {
    if(getDMXData(i))
      printf(" %d: %d ", i, getDMXData(i));
    if(!(i % 32))
      printf("\n");

  }
  printf("----------------DMX DATA-----------------\n");
}
