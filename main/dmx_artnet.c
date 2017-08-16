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
ArtnetPollReplyPacket POLLREPLYPACKET;
uint8_t status_1, status_2;
uint8_t send_active;

const uint8_t ARTNET_ID[8] = {'A','r','t','-','N','e','t',0};

//direction so you can ignore incoming DMX data packets
void startDMXArtnet(uint8_t direction)
{

  if(ARTNET._enabled == ENABLE)
    stopDMXArtnet();

  ARTNET._direction = direction;
  ARTNET._enabled = ENABLE;
  createPacketArtnet();
  createPacketArtnetPollReply();
  udp_artnet_init();
  ESP_LOGI(TAG, "Artnet Start");
}

void stopDMXArtnet()
{
  ARTNET._enabled = DISABLE;
}

void changeDirectionArtnet(uint8_t direction)
{
  if(direction == ARTNET._direction)
    return;

  if(direction == SEND)
  {
    ARTNET._direction = SEND;
    xTaskCreatePinnedToCore(&sendArtnetLoop, "SEND ARTNET", 2048, NULL, tskIDLE_PRIORITY, NULL, 0);
  }
  else
  {
    ARTNET._direction = RECEIVE;
    while(send_active){ vTaskDelay(10); }
  }

}

void udp_artnet_init()
{
  ip_addr_t ip;
  ARTNET._udp = udp_new();
  ARTNET._own_ip = IP_ADDR_ANY;
  IP_ADDR4(&ip, 255,255,255,255); //broadcast ip
  ARTNET._dest_ip = &ip;
  ARTNET._port = ART_NET_PORT;
  ESP_LOGI(TAG, "Bind %d",udp_bind(ARTNET._udp, ARTNET._own_ip, ARTNET._port));

  udp_recv(ARTNET._udp, recieveDMXArtnet ,NULL);

  if(ARTNET._direction == SEND)
    xTaskCreatePinnedToCore(&sendArtnetLoop, "SEND ARTNET", 2048, NULL, tskIDLE_PRIORITY, NULL, 0);

}

void sendArtnetLoop(void)
{
  send_active = 1;
  //create sending to multiple universes
  while(ARTNET._direction == SEND)
  {
    sendDMXDataArtnet(getOwnUniverse());
    vTaskDelay(ARTNET_DMX_SEND_DELAY);

    //TODO send ArtPoll
  }
  send_active = 0;
  vTaskDelete(NULL);
}

void recieveDMXArtnet(void *arg,
                  struct udp_pcb *pcb,
                  struct pbuf *p,
                  const ip_addr_t *addr,
                  u16_t port)
{
  uint32_t i;
  uint16_t opcode;

  //ESP_LOGI(TAG, "PACKET REC");

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
      if(ARTNET._direction != RECEIVE)
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
// add in varible slots

//send in broadcast
//TODO send to diffrent universes
void sendDMXDataArtnet(uint16_t universe){
  int i, j, ret_val;
  struct pbuf *p;



  if(ARTNET._enabled != ENABLE)
  {
    ESP_LOGI(TAG, "You silly, call startDMXArtnet(SEND/RECEIVE) first!")
    return;
  }

  //ESP_LOGI(TAG, "Connect %d", udp_connect(ARTNET._udp, ARTNET._dest_ip, ARTNET._port));

  //set end IP
  udp_connect(ARTNET._udp, ARTNET._dest_ip, ARTNET._port);
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

  ret_val = udp_send(ARTNET._udp, p);
  if(ret_val)
    ESP_LOGI(TAG, "Send Error %d", ret_val);

  udp_disconnect(ARTNET._udp);

  pbuf_free(p);
}


void sendPollReplyArtnet(struct pbuf *p, const ip_addr_t *addr)
{
  int ret_val, i;
  struct pbuf *p_send;

  if(ARTNET._enabled != ENABLE)
  {
    ESP_LOGI(TAG, "You silly, call startDMXArtnet(SEND/RECEIVE) first!")
    return;
  }

  //take out for performance
  createPacketArtnetPollReply();

  p_send = pbuf_alloc(PBUF_TRANSPORT, sizeof(ArtnetPollReplyPacket) ,PBUF_RAM);

  udp_connect(ARTNET._udp, addr, ARTNET._port);

  for(i = 0; i < sizeof(ArtnetPollReplyPacket); i++)
    ((uint8_t*)(p_send->payload))[i] = ((uint8_t*)(ARTNET._poll_reply_packet))[i];

  ret_val = udp_send(ARTNET._udp, p_send);

  udp_disconnect(ARTNET._udp);

  if(ret_val)
    ESP_LOGI(TAG, "Send Poll Reply Error %d", ret_val);
}

void sendPollArtnet()
{
  if(ARTNET._enabled != ENABLE)
  {
    ESP_LOGI(TAG, "You silly, call startDMXArtnet(SEND/RECEIVE) first!")
    return;
  }

  //TODO this later
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
  //collect ip addresses of connected devices?
  return;
}

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

void createPacketArtnetPollReply()
{
  uint8_t i;
  char temp;

  ARTNET._poll_reply_packet = &POLLREPLYPACKET;

  status_1 = 0; //TODO getStatus1
  status_2 = 0; //TODO getStatus2

  for(i = 0; i < 8; i++)
    POLLREPLYPACKET._id[i] = ARTNET_ID[i];

  POLLREPLYPACKET._opcode = ART_OP_POLL_REPLY;

  for(i = 0; i < 4; i++)
    POLLREPLYPACKET._own_ip[3 - i] = getOwnIPAddress()[i];

  POLLREPLYPACKET._port = ART_NET_PORT;

  POLLREPLYPACKET._version_info_hi = 0;
  POLLREPLYPACKET._version_info_lo = 3;

  POLLREPLYPACKET._net_switch = (uint8_t)(getOwnUniverse() >> 8);
  POLLREPLYPACKET._sub_switch = (uint8_t)((getOwnUniverse() & 0xFF) >> 4);

  POLLREPLYPACKET._oem_hi = 0x12; //just cause
  POLLREPLYPACKET._oem_lo = 0x51;

  POLLREPLYPACKET._ubea_version = 0;

  POLLREPLYPACKET._status_1 = status_1;

  POLLREPLYPACKET._esta_man_lo = 0x04; //blizzard esta id
  POLLREPLYPACKET._esta_man_hi = 0x01;

  for(i = 0; i < 26; i++)
  {
    temp = getName()[i];
    if(temp == '\0')
      break; //end of the name
    POLLREPLYPACKET._short_name[i] = temp;
  }

  for(i = 0; i < 64; i++)
  {
    temp = getName()[i];
    if(temp == '\0')
      break; //end of the name
    POLLREPLYPACKET._long_name[i] = temp;
  }

  POLLREPLYPACKET._node_report[0] = 0x69; //TODO Embed Diognostics here

  POLLREPLYPACKET._num_ports_hi = 0; //TODO add getNumPorts
  POLLREPLYPACKET._num_ports_lo = 1;

  POLLREPLYPACKET._port_types[0] = 0xC0;
  POLLREPLYPACKET._port_types[1] = 0; //TODO (getNumPorts > 1) ? 0xC0 : 0;
  POLLREPLYPACKET._port_types[2] = 0; //TODO (getNumPorts > 2) ? 0xC0 : 0;
  POLLREPLYPACKET._port_types[3] = 0; //TODO (getNumPorts > 3) ? 0xC0 : 0;

  POLLREPLYPACKET._good_input[0] = 0; //unused
  POLLREPLYPACKET._good_input[1] = 0;
  POLLREPLYPACKET._good_input[2] = 0;
  POLLREPLYPACKET._good_input[3] = 0;

  POLLREPLYPACKET._good_output[0] = 0; //unused
  POLLREPLYPACKET._good_output[1] = 0;
  POLLREPLYPACKET._good_output[2] = 0;
  POLLREPLYPACKET._good_output[3] = 0;


  //input port = dmx in artnet out
  //set an output universe?
  POLLREPLYPACKET._sw_in[0] = (uint8_t)(getOwnUniverse() & 0x0F); //TODO (uint8_t) getDestUniverse & 0x0F;
  POLLREPLYPACKET._sw_in[1] = 0; //TODO (getNumPorts > 1) ? 0xC0 : 0;
  POLLREPLYPACKET._sw_in[2] = 0; //TODO (getNumPorts > 2) ? 0xC0 : 0;
  POLLREPLYPACKET._sw_in[3] = 0; //TODO (getNumPorts > 3) ? 0xC0 : 0;

  POLLREPLYPACKET._sw_out[0] = (uint8_t)(getOwnUniverse() & 0x0F);
  POLLREPLYPACKET._sw_out[1] = 0; //TODO (getNumPorts > 1) ? 0xC0 : 0;
  POLLREPLYPACKET._sw_out[2] = 0; //TODO (getNumPorts > 2) ? 0xC0 : 0;
  POLLREPLYPACKET._sw_out[3] = 0; //TODO (getNumPorts > 3) ? 0xC0 : 0;

  POLLREPLYPACKET._sw_video = 0; //unused
  POLLREPLYPACKET._sw_macro = 0;
  POLLREPLYPACKET._sw_remote = 0;

  POLLREPLYPACKET._spare[0] = 0;
  POLLREPLYPACKET._spare[1] = 0;
  POLLREPLYPACKET._spare[2] = 0;

  POLLREPLYPACKET._style = 0x00; //DMX to/from Artnet - Node

  POLLREPLYPACKET._mac[0] = 0x00; //TODO getOwnMac()

  POLLREPLYPACKET._bind_ip[0] = 0; //not used
  POLLREPLYPACKET._bind_ip[1] = 0;
  POLLREPLYPACKET._bind_ip[2] = 0;
  POLLREPLYPACKET._bind_ip[3] = 0;

  POLLREPLYPACKET._bind_index = 0; //not used

  POLLREPLYPACKET._status_2 = status_2;

  for(i = 0; i < 26; i++)
    POLLREPLYPACKET._filler[i] = 0; //not used

}
