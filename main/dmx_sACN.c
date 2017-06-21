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

#include "lib/dmx_sACN.h"
#include "lib/dmx.h"

static const char *TAG = "ARTNET";

sACNNode SACN;
sACNPacket SACNPACKET;
ip_addr_t ip;


const uint8_t sCAN_ID[9] = {'A','S','C','-','E','1','.','1','7'};

void startDMXsACN()
{
  SACNPACKET._dmx_data = getDMXBuffer();
  createPacketsACN();
  udp_sacn_init();

}

void sendDMXsACN(uint16_t universe){
  int i, j;
  struct pbuf *p;

  //establish pbuf
  p = pbuf_alloc(PBUF_TRANSPORT, MAX_SACN_BUFFER ,PBUF_RAM);

  SACNPACKET._packet_buffer[SACN_SEQ_INDEX] = (!SACNPACKET._sequence) ? 1 : SACNPACKET._sequence++;

  SACNPACKET._packet_buffer[SACN_UNI_H_INDEX] =  universe >> 8;
  SACNPACKET._packet_buffer[SACN_UNI_L_INDEX] =  universe & 0xff;

  //transfer payload

  for(i = 0, j = 0; i < MAX_SACN_BUFFER; i++)
  {
    if(i < SACN_DMX_START_INDEX)
      ((uint8_t*)(p->payload))[i] = ((uint8_t*)(SACN._packet))[i]; //skip start bit
    else
      ((uint8_t*)(p->payload))[i] = ((uint8_t*)(SACN._packet->_dmx_data))[j++]; //sACN uses a start bit
  }

  ESP_LOGI(TAG, "Send %d", udp_send(SACN._udp, p));

  pbuf_free(p);
}

void recieveDMXsACN(void *arg,
                  struct udp_pcb *pcb,
                  struct pbuf *p,
                  const ip_addr_t *addr,
                  u16_t port)
{
  int i;
  for(i = 0; i < 8; i++)
  {
    ESP_LOGI(TAG, "FAIL");
    goto FREE_P;
    /*if(((uint8_t*)p->payload)[i] != ARTNET_ID[i])
    {
      ESP_LOGI(TAG, "FAIL");
      goto FREE_P;
    }*/
  }
  ESP_LOGI(TAG, ":)");
FREE_P:
  pbuf_free(p);
}

void udp_sacn_init()
{
  SACN._packet = &SACNPACKET;
  SACN._udp = udp_new();
  SACN._own_ip = IP_ADDR_ANY;
  IP_ADDR4(&ip, 192,168,1,186);
  SACN._dest_ip = &ip;
  SACN._port = SACN_PORT;
  ESP_LOGI(TAG, "Bind %d",udp_bind(SACN._udp, SACN._own_ip, SACN._port));
  ESP_LOGI(TAG, "Connect %d", udp_connect(SACN._udp, SACN._dest_ip, SACN._port));

  udp_recv(SACN._udp, recieveDMXsACN ,NULL);

}
void parsePacketsACN(struct pbuf *p)
{

}

// add in varible slots
void createPacketsACN()
{
  uint16_t i;

  for(i = 0; i < MAX_SACN_BUFFER; i++)
    SACNPACKET._packet_buffer[i] = 0;        // zero outside layers & start code

  SACNPACKET._packet_buffer[0] = 0x00;
  SACNPACKET._packet_buffer[1] = 0x10;

  for(i = 0; i < (sizeof(sCAN_ID)/sizeof(uint8_t)); i++)
    SACNPACKET._packet_buffer[i + 4] = sCAN_ID[i]; //starts at packet index 4

   uint16_t fplusl = getSlots() + 110 + 0x7000;
   SACNPACKET._packet_buffer[16] = fplusl >> 8;
   SACNPACKET._packet_buffer[17] = fplusl & 0xff;
   SACNPACKET._packet_buffer[21] = 0x04;
   fplusl = getSlots() + 88 + 0x7000;
   SACNPACKET._packet_buffer[38] = fplusl >> 8;
   SACNPACKET._packet_buffer[39] = fplusl & 0xff;
   SACNPACKET._packet_buffer[43] = 0x02;
   SACNPACKET._packet_buffer[44] = 'A';
   SACNPACKET._packet_buffer[45] = 'r';
   SACNPACKET._packet_buffer[46] = 'd';
   SACNPACKET._packet_buffer[47] = 'u';
   SACNPACKET._packet_buffer[48] = 'i';
   SACNPACKET._packet_buffer[49] = 'n';
   SACNPACKET._packet_buffer[50] = 'o';
   SACNPACKET._packet_buffer[108] = 100;    //priority

   SACNPACKET._sequence = 1;

   SACNPACKET._packet_buffer[111] =  SACNPACKET._sequence;
   SACNPACKET._packet_buffer[113] =  SACNPACKET._universe >> 8;
   SACNPACKET._packet_buffer[114] =  SACNPACKET._universe & 0xff;
   fplusl = getSlots() + 11 + 0x7000;
   SACNPACKET._packet_buffer[115] = fplusl >> 8;
   SACNPACKET._packet_buffer[116] = fplusl & 0xff;
   SACNPACKET._packet_buffer[117] = 0x02;
   SACNPACKET._packet_buffer[118] = 0xa1;
   SACNPACKET._packet_buffer[122] = 0x01;
   fplusl = getSlots() + 1;
   SACNPACKET._packet_buffer[123] = fplusl >> 8;
   SACNPACKET._packet_buffer[124] = fplusl & 0xFF;

  clearDMX();
}
