/* esp32_dmx.h
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

  Generic DMX object class

 */

#ifndef DMX_H
#define DMX_H

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

//globals
#define SEND 1
#define RECEIVE 0

#define ENABLE 1
#define DISABLE 0

//Modes
#define ETHERNET_MODE 0
#define WIFI_MODE 1
#define DMX_MODE 2
#define WDMX 3

//Sub Modes
#define ARTNET_MODE 0
#define SACN_MODE 1
#define BLYNK_MODE 2

//Max and Mins
#define DMX_MAX_SLOTS 513 // 512 + start byte
#define DMX_MIN_SLOTS 24

#define NAME_MAX_LENGTH 64
#define SSID_MAX_LENGTH 32
#define PASS_MAX_LENGTH 32

//flags
extern uint8_t device_name_changed;
extern uint8_t com_made_changed;
extern uint8_t com_sub_mode_changed;
extern uint8_t own_ip_changed;
extern uint8_t own_address_changed;
extern uint8_t own_universe_changed;
extern uint8_t slots_changed;

//NVS Handle
#define NVS_NAMESPACE "Blizzard"
#define NVS_DEVICE_NAME_KEY "DEVICE_NAME"
#define NVS_SSID_KEY "SSID"
#define NVS_PASS_KEY "PASS"
#define NVS_COM_MODE_KEY "COM_MODE"
#define NVS_COM_SUB_MODE_KEY "COM_SUB_MODE"
#define NVS_OWN_IP_ADDRESS_KEY "OWN_IP_ADDRESS"
#define NVS_OWN_ADDRESS_KEY "OWN_ADDRESS"
#define NVS_OWN_UNIVERSE_KEY "OWN_UNIVERSE"
#define NVS_SLOTS_KEY "SLOTS"
extern nvs_handle config_nvs_handle;

/* If need be add in option to add in a buffer*/

char* getName(void);

void setName(char *name, uint8_t length);

char* getSSID(void);

void setSSID(char *ssid, uint8_t length); //has to be null terminated

char* getPASS(void);

void setPASS(char *pass, uint8_t length); //has to be null terminated

uint8_t getComMode(void);

void setComMode(uint8_t com_mode);

uint8_t getSubComMode(void);

void setSubComMode(uint8_t com_sub_mode);

uint8_t* getOwnIPAddress();

void setOwnIPAddress(uint8_t* address);

uint16_t getOwnAddress(void);

void setOwnAddress(uint16_t address);

uint16_t getOwnUniverse(void);

void setOwnUniverse(uint16_t universe);

uint16_t getSlots(void);

void setSlots(uint16_t count);

uint8_t getDMXData(uint16_t slot);

void setDMXData(uint16_t slot, uint8_t value);

uint8_t* getDMXBuffer(void);

void clearDMX(void);

void maxDMX(void);

void copyToDMX(uint8_t *buf, uint16_t start_index, uint16_t length);

void copyFromDMX(uint8_t *buf, uint16_t start_index, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif // ifndef DMX_H
