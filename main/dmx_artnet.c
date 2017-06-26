#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/udp.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip6_addr.h"

#include "lib/dmx_artnet.h"
#include "lib/dmx.h"

static const char *TAG = "ARTNET";

ArtnetNode ARTNET;
ArtnetPacket ARTNETPACKET;
ip_addr_t ip;


const uint8_t ARTNET_ID[8] = {'A','r','t','-','N','e','t',0};

//direction so you can ignor incoming DMX data packets
void startDMXArtnet(uint8_t direction)
{

  if(ARTNET._enabled == DMX_ENABLE)
    stopDMXArtnet();

  ARTNET._direction = direction;
  ARTNET._enabled = DMX_ENABLE;
  createPacketArtnet();
  udp_artnet_init();

}

void stopDMXArtnet()
{
  ARTNET._enabled = DMX_DISABLE;
}

void udp_artnet_init()
{
  ARTNET._udp = udp_new();
  ARTNET._own_ip = IP_ADDR_ANY;
  IP_ADDR4(&ip, 192,168,1,186);
  ARTNET._dest_ip = &ip;
  ARTNET._port = ART_NET_PORT;
  ESP_LOGI(TAG, "Bind %d",udp_bind(ARTNET._udp, ARTNET._own_ip, ARTNET._port));
  //move to send?
  //ESP_LOGI(TAG, "Connect %d", udp_connect(ARTNET._udp, ARTNET._dest_ip, ARTNET._port));

  udp_recv(ARTNET._udp, recieveDMXArtnet ,NULL);

}
// add in varible slots
void createPacketArtnet()
{
  uint8_t i;

  ARTNET._packet = &ARTNETPACKET;

  //copy artnet id
  for(i = 0; i < 8; i++)
    ARTNETPACKET._id[i] = ARTNET_ID[i];

  ARTNETPACKET._opcode = ART_OP_DMX;

  ARTNETPACKET._protocol_version = ART_PROTO_VER;

  ARTNETPACKET._sequence = 1; //will increment in sendDMXArtnet

  ARTNETPACKET._physical = 1; //"for information only"- ARTNET 4

  ARTNETPACKET._universe_subnet = 0; //will be set in sendDMXArtnet

  ARTNETPACKET._universe = 0; //will be set in sendDMXArtnet

  ARTNETPACKET._slot_length_hi = ((DMX_MAX_SLOTS - 1) >> 8) & 0xFF; //2 - 512 has to be even
  ARTNETPACKET._slot_length_lo = (DMX_MAX_SLOTS - 1) & 0xFF; //2 - 512 has to be even

  ARTNETPACKET._dmx_data = getDMXBuffer();

}

void sendDMXDataArtnet(uint16_t universe){
  int i, j;
  struct pbuf *p;



  if(ARTNET._enabled != DMX_ENABLE)
  {
    ESP_LOGI(TAG, "You silly, call startDMXArtnet(DMX_SEND/DMX_RECEIVE) first!")
    return;
  }

  ESP_LOGI(TAG, "Connect %d", udp_connect(ARTNET._udp, ARTNET._dest_ip, ARTNET._port));

  //establish pbuf
  p = pbuf_alloc(PBUF_TRANSPORT, MAX_ARTNET_BUFFER ,PBUF_RAM);

  ARTNETPACKET._universe = (uint8_t)(universe >> 8);
  ARTNETPACKET._universe_subnet = (uint8_t)(universe & 0xFF);

  //transfer payload
  for(i = 0, j = 1; i < MAX_ARTNET_BUFFER; i++)
  {
    if(i < ART_DMX_DATA_INDEX)
      ((uint8_t*)(p->payload))[i] = ((uint8_t*)(ARTNET._packet))[i]; //transfer header info
    else
      ((uint8_t*)(p->payload))[i] = ARTNET._packet->_dmx_data[j++]; //transfer DMX data j = 1 -> skip start bit
  }

  ARTNETPACKET._sequence = (!ARTNETPACKET._sequence) ? 1 : ARTNETPACKET._sequence + 1;
  ESP_LOGI(TAG, "Send %d", udp_send(ARTNET._udp, p));

  pbuf_free(p);
}

void sendPollArtnet()
{
  if(ARTNET._enabled != DMX_ENABLE)
  {
    ESP_LOGI(TAG, "You silly, call startDMXArtnet(DMX_SEND/DMX_RECEIVE) first!")
    return;
  }
}

void sendPollReplyArtnet(struct pbuf *p, const ip_addr_t *addr)
{



}



void recieveDMXArtnet(void *arg,
                  struct udp_pcb *pcb,
                  struct pbuf *p,
                  const ip_addr_t *addr,
                  u16_t port)
{
  uint32_t i;
  uint16_t opcode;

  for(i = 0; i < 8; i++)
  {
    if(((uint8_t*)p->payload)[i] != ARTNET_ID[i])
    {
      ESP_LOGI(TAG, "Invalid ArtnetPacket");
      goto FREE_P;
    }
  }

  opcode = ((uint16_t*)p->payload)[ART_OPCODE_U16_INDEX];

  switch(opcode)
  {
    case ART_OP_DMX:
      if(ARTNET._direction != DMX_RECEIVE)
      {
        ESP_LOGI(TAG, "Not in Receive Mode %d", ARTNET._direction);
        goto FREE_P;
      }
      parseDMXDataPacketArtnet(p);
      //ESP_LOGI(TAG, "Artnet DMX Data");
    break;
    case ART_OP_POLL:
      sendPollReplyArtnet(p, addr);
      //ESP_LOGI(TAG, "Artnet Poll");
    break;
    case ART_OP_POLL_REPLY:
      parsePollReplyArtnet(p);
      //ESP_LOGI(TAG, "Artnet Poll Reply");
    break;
  }

  //ESP_LOGI(TAG, ":)");

FREE_P:

  pbuf_free(p);
}


void parseDMXDataPacketArtnet(struct pbuf *p)
{
  uint16_t slots, universe, i;
  uint8_t sequence, *dmx_data; //may not do anything with this yet

  universe = ((uint16_t)((uint8_t*)p->payload)[ART_DATA_UNIVERSE_H_INDEX]) << 8;
  universe |= ((uint16_t)((uint8_t*)p->payload)[ART_DATA_UNIVERSE_L_INDEX]) && 0xFF;

  //if not meant for this universe send it away!
  if(universe != getOwnUniverse())
  {
    ESP_LOGI(TAG, "Not in my house! --Universe %d--", universe);
    return;
  }

  //copy over dmx data into global DMX array
  slots = ((uint16_t)((uint8_t*)p->payload)[ART_DATA_SLOTS_H_INDEX]) << 8;
  slots |= ((uint16_t)((uint8_t*)p->payload)[ART_DATA_SLOTS_L_INDEX]) && 0xFF;
  //ESP_LOGI(TAG, "DMX slots recieved %d", slots);

  dmx_data = getDMXBuffer();

  for(i = 0; i < slots; i++)
    dmx_data[i + 1] = ((uint8_t*)p->payload)[ART_DMX_DATA_INDEX + i];

}

void parsePollReplyArtnet(struct pbuf *p)
{

}
