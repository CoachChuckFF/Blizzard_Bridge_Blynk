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

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/udp.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip6_addr.h"

#include "blizzard_connection_manager.h"

#define ART_NET_PORT 0x1936 // or 6454
#define ART_NET_PORT_REV 0x3619

#define ART_OP_DMX 0x5000
#define ART_OP_POLL 0x2000
#define ART_OP_POLL_REPLY 0x2100
#define ART_OP_PROG 0xF800
#define ART_OP_PROG_REPLY 0xF900
#define ART_OP_ART_ADDRESS 0x6000

#define ART_PROTO_VER 14

#define ART_OPCODE_U16_INDEX 4
#define ART_DATA_UNIVERSE_H_INDEX 15
#define ART_DATA_UNIVERSE_L_INDEX 14
#define ART_DATA_SLOTS_H_INDEX 16
#define ART_DATA_SLOTS_L_INDEX 17
#define ART_DMX_DATA_INDEX 18

#define MAX_ARTNET_BUFFER 530

#define ARTNET_DMX_SEND_DELAY 1000

typedef struct ArtnetPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _sequence;
  uint8_t _physical;
  uint8_t _universe_subnet; //low byte
  uint8_t _universe; //high 7 bits
  uint8_t _slot_length_hi; //2 - 512 has to be even
  uint8_t _slot_length_lo; //2 - 512 has to be even
  uint8_t *_dmx_data; // no start bit
}__attribute__((packed)) ArtnetPacket;

typedef struct ArtnetPollReplyPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint8_t _own_ip[4];
  uint16_t _port; //always 0x1936
  uint8_t _version_info_hi; //high byte of firmware version
  uint8_t _version_info_lo; //low byte of firmware version
  uint8_t _net_switch;
  uint8_t _sub_switch;
  uint8_t _oem_hi;
  uint8_t _oem_lo;
  uint8_t _ubea_version; //set to 0
  union {
    struct {
      uint8_t _ubea_present: 1;
      uint8_t _rdm_status: 1;
      uint8_t _boot_status: 1;
      uint8_t _reserverd: 1;
      uint8_t _port_prog_auth: 2;
      uint8_t _indicator_state: 2;
    };
    uint8_t val;
  } _status_1;
  uint8_t _esta_man_lo;
  uint8_t _esta_man_hi;
  uint8_t _short_name[18];
  uint8_t _long_name[64];
  uint8_t _node_report[64];
  uint8_t _num_ports_hi;
  uint8_t _num_ports_lo;
  uint8_t _port_types[4];
  uint8_t _good_input[4];
  uint8_t _good_output[4];
  uint8_t _sw_in[4];
  uint8_t _sw_out[4];
  uint8_t _sw_video;
  uint8_t _sw_macro;
  uint8_t _sw_remote;
  uint8_t _spare[3]; //spare bytes set to 0
  uint8_t _style;
  uint8_t _mac[6]; //set high bytes to 0 if no info
  uint8_t _bind_ip[4];
  uint8_t _bind_index;
  union {
    struct {
      uint8_t _web_config_support: 1;
      uint8_t _dhcp_capable: 1;
      uint8_t _port_addressing: 1;
      uint8_t _sacn_switchable: 1;
      uint8_t _squawking: 1;
      uint8_t _reserverd: 3;
    };
    uint8_t val;
  } _status_2;
  uint8_t _filler[26]; //zero for now
}__attribute__((packed)) ArtnetPollReplyPacket;

typedef struct ArtnetProgPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint16_t _filler_1; //0's
  union {
    struct {
      uint8_t _prog_port: 1; //not used
      uint8_t _prog_subnet_mask: 1;
      uint8_t _prog_ip: 1;
      uint8_t _set_default: 1;
      uint8_t _filler: 2;
      uint8_t _dhcp_en: 1;
      uint8_t _prog_en: 1;
    };
    uint8_t val;
  } _command;
  uint8_t _filler_2;
  uint8_t _prog_ip[4];
  uint8_t _prog_subnet_mask[4];
  uint16_t _prog_port; //not useed
  uint8_t _spare[8]; //not used

}__attribute__((packed)) ArtnetProgPacket;

typedef struct ArtnetProgReplyPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _filler[4];
  uint8_t _prog_ip[4];
  uint8_t _prog_subnet_mask[4];
  uint16_t _prog_port; //not used
  union {
    struct {
      uint8_t _reserved_1: 3;
      uint8_t _dhcp_en: 1;
      uint8_t _reserved_2: 4;
    };
    uint8_t val;
  } _status;
  uint8_t _spare[7];

}__attribute__((packed)) ArtnetProgReplyPacket;

typedef struct ArtnetArtAddressPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _net_switch;
  uint8_t _bind_index;
  uint8_t _short_name[18];
  uint8_t _long_name[64];
  uint8_t _sw_in[4];
  uint8_t _sw_out[4];
  uint8_t _sub_switch;
  uint8_t _sw_video;
  uint8_t _command;
}__attribute__((packed)) ArtnetArtAddressPacket;

typedef struct ArtnetNode {
  ArtnetPacket *_packet;
  ArtnetPollReplyPacket *_poll_reply_packet;
  struct udp_pcb *_udp;
  ip_addr_t *_own_ip;
  ip_addr_t *_dest_ip;
  uint16_t _port;
  uint8_t _direction;
  uint8_t _enabled;

} ArtnetNode;

/*External*/
void startDMXArtnet(uint8_t direction);
void stopDMXArtnet(void);
void changeDirectionArtnet(uint8_t direction);
void sendDMXDataArtnet(uint16_t universe);
/*Internal*/
void sendArtnetLoop(void);
void parseArtAddressArtnet(struct pbuf *p, const ip_addr_t *addr);
void sendPollReplyArtnet(struct pbuf *p, const ip_addr_t *addr);
void parseProgArtnet(struct pbuf *p, const ip_addr_t *addr);
void sendProgArtnet(void); //TODO
void sendProgReplyArtnet(struct pbuf *p, const ip_addr_t *addr);
void sendPollArtnet(void); // run periodically in own thread
void parsePollReplyArtnet(struct pbuf *p);
void parseDMXDataPacketArtnet(struct pbuf *p);
void recieveDMXArtnet(void *arg,
                  struct udp_pcb *pcb,
                  struct pbuf *p,
                  const ip_addr_t *addr,
                  u16_t port);
void udp_artnet_init(void);
void createPacketArtnet(void);
void createPacketArtnetPollReply(void);



extern ArtnetNode ARTNET;

#ifdef __cplusplus
}
#endif

#endif // ifndef  DMX_ARTNET_H
