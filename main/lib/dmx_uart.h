/* dmx_uart.h
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
   The esp32_dmx library supports output and input of DMX using the UART
   serial output of an ESP32 microcontroller.  esp32_dmx uses
   UART2 for output and input.  This means that hardware Serial
   can still be used for USB communication.
   (do not use UART1 because it is connected to flash)
 */

#ifndef DMX_UART_H
#define DMX_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "driver/uart.h"
#include "driver/gpio.h"

#define DIRECTION_PIN 23

#define DMX_UART UART_NUM_2
#define DMX_TX_PIN 12
#define DMX_RX_PIN 14
#define DMX_DATA_BAUD		250000
#define DMX_BREAK_BAUD 	 88000

#define DMX_STATE_START 0
#define DMX_STATE_DATA 1
#define DMX_STATE_BREAK 2
#define DMX_STATE_MAB 3
#define DMX_STATE_IDLE 4

/* ESP32 DMX struct*/
typedef struct ESP32DMX {
  uint8_t  _dmx_state;
  uint16_t  _idle_count;
  uint16_t  _current_slot;
  uint16_t  _slots;
  uint8_t _direction; //0-> send DMX 1-> recieve DMX
  uint8_t _enabled;
  uint8_t  *_dmx_data;
} ESP32DMX;

/* UART Handler (Internal) Functions */
//void uart_enable_interrupt(void);
void uart_dmx_init(int baudrate);

/* DMX Handler (User) Functions */
void startDMXUart(uint8_t dir);
void stopDMXUart( void );

/* Interrupt (Internal) handlers */
static void txEmptyInterruptHandlerDMX(void);
static void receiveInterruptHandlerDMX(void);
static void handleMAB(void);
static void DMXTx(uint8_t value);

extern ESP32DMX DMX;

#ifdef __cplusplus
}
#endif

#endif // ifndef DMX_UART_H
