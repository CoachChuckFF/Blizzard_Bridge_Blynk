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
#include "lib/dmx.h"

static const char *TAG = "DMX";

static uint8_t DMX[DMX_MAX_SLOTS];

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
