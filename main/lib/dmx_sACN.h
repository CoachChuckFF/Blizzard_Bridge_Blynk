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

   The esp32_dmx library supports output and input of DMX using the UART
   serial output of an ESP32 microcontroller.  esp32_dmx uses
   UART2 for output and input.  This means that hardware Serial
   can still be used for USB communication.

   (do not use UART1 because it is connected to flash)

 */

#ifndef DMX_SACN_H
#define DMX_SACN_H

#include <inttypes.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/udp.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip6_addr.h"

#define SACN_PORT 0x15C0 // or 5568

#define MAX_SACN_BUFFER 638

#define SACN_DMX_OFFSET 125
#define SACN_CID_LENGTH 16

#define SACN_SEQ_INDEX 111
#define SACN_UNI_H_INDEX 113
#define SACN_UNI_L_INDEX 114
#define SACN_DMX_START_INDEX 125
#define SACN_DMX_DATA_INDEX 126

typedef struct sACNPacket {
  uint8_t _packet_buffer[MAX_SACN_BUFFER];
  uint8_t *_dmx_data;
  uint8_t _sequence;
  uint16_t _universe;
} sACNPacket;

typedef struct sACNNode {
  sACNPacket *_packet;
  struct udp_pcb *_udp;
  ip_addr_t *_own_ip;
  ip_addr_t *_dest_ip;
  uint16_t _port;

} sACNNode;

/*External*/
void startDMXsACN(void);

/*Internal*/
void createPacketsACN(void);
void sendDMXsACN(uint16_t universe);
void parsePacketsACN(struct pbuf *p);
void receiveDMXsACN(void *arg,
                  struct udp_pcb *pcb,
                  struct pbuf *p,
                  const ip_addr_t *addr,
                  u16_t port);
void udp_sacn_init(void);

extern sACNNode SACN;
extern sACNPacket SACNPACKET;

#endif // ifndef DMX_SACN
