/* dmx_artnet.h
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

  Artnet library, needs a internet connection before useful

 */

#ifndef DMX_ARTNET_H
#define DMX_ARTNET_H

#include <inttypes.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/udp.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip6_addr.h"

#define ART_NET_PORT 0x1936 // or 6454

#define ART_POLL 0x2000
#define ART_DMX 0x5000
#define ART_DMX_OFFSET 18
#define ART_PROTO_VER 14

#define MAX_ARTNET_BUFFER 530

typedef struct ArtnetPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _sequence;
  uint8_t _physical;
  uint8_t _universe;
  uint8_t _universe_subnet;
  uint8_t _slot_length_hi; //2 - 512 has to be even
  uint8_t _slot_length_lo; //2 - 512 has to be even
  uint8_t *_dmx_data; // no start bit
} ArtnetPacket;

typedef struct ArtnetNode {
  ArtnetPacket *_packet;
  struct udp_pcb *_udp;
  ip_addr_t *_own_ip;
  ip_addr_t *_dest_ip;
  uint16_t _port;

} ArtnetNode;

/*External*/
void startDMXArtnet(void);

/*Internal*/
void createPacketArtnet(void);
void sendDMXArtnet(void);
void parsePacketArtnet(struct pbuf *p);
void receiveDMXArtnet(void *arg,
                  struct udp_pcb *pcb,
                  struct pbuf *p,
                  const ip_addr_t *addr,
                  u16_t port);
void udp_artnet_init(void);




extern ArtnetNode ARTNET;

#endif // ifndef  DMX_ARTNET_H
