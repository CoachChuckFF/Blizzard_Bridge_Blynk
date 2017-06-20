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

#include "lib/esp32_artnet.h"
#include "lib/dmx.h"

static const char *TAG = "ARTNET";

ArtnetNode ARTNET;
ArtnetPacket PACKET;
ip_addr_t ip;


const uint8_t ARTNET_ID[8] = {'A','r','t','-','N','e','t',0};

void startArtnet()
{
  PACKET._dmx_data = getDMXBuffer();
  createPacket();
  udp_art_init();

}

void sendDMX(void){
  int i;
  struct pbuf *p;

  //establish pbuf
  p = pbuf_alloc(PBUF_TRANSPORT, MAX_ARTNET_BUFFER ,PBUF_RAM);

  //transfer payload

  for(i = 0; i < MAX_ARTNET_BUFFER; i++)
    ((uint8_t*)(p->payload))[i] = ((uint8_t*)(ARTNET._packet))[i];

  ESP_LOGI(TAG, "Send %d", udp_send(ARTNET._udp, p));

  pbuf_free(p);
}

void recieveDMX(void *arg,
                  struct udp_pcb *pcb,
                  struct pbuf *p,
                  const ip_addr_t *addr,
                  u16_t port)
{
  int i;
  for(i = 0; i < 8; i++)
  {
    if(((uint8_t*)p->payload)[i] != ARTNET_ID[i])
    {
      ESP_LOGI(TAG, "FAIL");
      goto FREE_P;
    }
  }
  ESP_LOGI(TAG, ":)");
FREE_P:
  pbuf_free(p);
}

void udp_art_init()
{
  ARTNET._udp = udp_new();
  ARTNET._own_ip = IP_ADDR_ANY;
  IP_ADDR4(&ip, 192,168,1,186);
  ARTNET._dest_ip = &ip;
  ARTNET._port = ART_NET_PORT;
  ESP_LOGI(TAG, "Bind %d",udp_bind(ARTNET._udp, ARTNET._own_ip, ARTNET._port));
  ESP_LOGI(TAG, "Connect %d", udp_connect(ARTNET._udp, ARTNET._dest_ip, ARTNET._port));

  udp_recv(ARTNET._udp, recieveDMX ,NULL);

}
void parsePacket(struct pbuf *p)
{

}

// add in varible slots
void createPacket()
{
  uint8_t i;

  ARTNET._packet = &PACKET;

  //copy artnet id
  for(i = 0; i < 8; i++)
    PACKET._id[i] = ARTNET_ID[i];

  PACKET._opcode = ART_DMX;

  PACKET._protocol_version = ART_PROTO_VER;

  PACKET._sequence = 0;

  PACKET._physical = 1;

  PACKET._universe = 1;

  PACKET._universe_subnet = 0;

  PACKET._slot_length_hi = (DMX_MAX_SLOTS >> 8) & 0xFF; //2 - 512 has to be even
  PACKET._slot_length_lo = DMX_MAX_SLOTS & 0xFF; //2 - 512 has to be even

  clearDMX();
}
