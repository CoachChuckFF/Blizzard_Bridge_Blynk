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

#define HIGH 1
#define LOW 0

//MEDIUMS
#define ETHERNET_MODE 0
#define WIFI_MODE 1
#define NETWORKLESS_MODE 2

//MODES
#define DMX_MODE 2
#define WDMX_MODE 3
#define ARTNET_MODE 0
#define SACN_MODE 1

//Max and Mins
#define DMX_MAX_SLOTS 513 // 512 + start byte
#define DMX_MIN_SLOTS 24

#define NAME_MAX_LENGTH 64
#define SSID_MAX_LENGTH 32
#define PASS_MAX_LENGTH 32

//flags
extern uint8_t device_name_changed;
extern uint8_t input_mode_changed;
extern uint8_t output_mode_changed;
extern uint8_t own_ip_changed;
extern uint8_t own_address_changed;
extern uint8_t own_universe_changed;
extern uint8_t slots_changed;


/* If need be add in option to add in a buffer*/

char* getName(void);

void setName(char *name, uint8_t length);

uint8_t* getOwnUUID(void);

void setOwnUUID(uint8_t* uuid);

char* getSSID(void);

void setSSID(char *ssid, uint8_t length); //has to be null terminated

char* getPASS(void);

void setPASS(char *pass, uint8_t length); //has to be null terminated

uint8_t getMedium(void);

void setMedium(uint8_t medium);

uint8_t getInputMode(void);

void setInputMode(uint8_t input_mode);

uint8_t getOutputMode(void);

void setOutputMode(uint8_t output_mode);

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

void printDMX(void);

#ifdef __cplusplus
}
#endif

#endif // ifndef DMX_H
