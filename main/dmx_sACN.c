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
sACNDataPacket SACNPACKET;
char SACNNAME[64];
ip_addr_t ip;

const uint8_t ACN_ID[12] = {'A','S','C','-','E','1','.','3','1', 0, 0, 0};
const uint8_t CID[16] = {'B','e','e','p','B','e','e','p','B','l','i','z','z','a','r','d'};

void startDMXsACN()
{
  createDataPacketsACN();
  udp_sacn_init();
}

void sendDMXDatasACN(uint16_t universe){
  int i, j;
  struct pbuf *p;

  //establish pbuf
  p = pbuf_alloc(PBUF_TRANSPORT, MAX_SACN_BUFFER ,PBUF_RAM);

  SACNPACKET._universe_h =  universe >> 8;
  SACNPACKET._universe_l =  universe & 0xff;

  //update name?
  //
  //


  //transfer payload

  for(i = 0, j = 0; i < MAX_SACN_BUFFER; i++)
  {
    if(i < SACN_DMX_START_INDEX)
      ((uint8_t*)(p->payload))[i] = ((uint8_t*)(SACN._packet))[i]; //skip start bit
    else
      ((uint8_t*)(p->payload))[i] = (SACN._packet->_dmx_data)[j++]; //sACN uses a start bit
  }

  //increment sequence
  SACNPACKET._sequence = (!SACNPACKET._sequence) ? 1 : SACNPACKET._sequence + 1;

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
void createDataPacketsACN()
{
  uint16_t i, flag_length;
  char *name = getName();

/********* Root Layer *******************/

  SACNPACKET._preamble_size = SACN_PREAMBLE_SIZE;
  SACNPACKET._postamble_size = SACN_POSTABMLE_SIZE;

  for(i = 0; i < (sizeof(ACN_ID)/sizeof(uint8_t)); i++)
    SACNPACKET._acn_id[i] = ACN_ID[i];

  //low 12 bits PDU length high 4 bits 0x7
  flag_length = getSlots() + 110 + 0x7000;
  SACNPACKET._flags_length_h = flag_length >> 8;
  SACNPACKET._flags_length_l = flag_length & 0xFF;

  SACNPACKET._root_vector = VECTOR_ROOT_E131_DATA;

  for(i = 0; i < (sizeof(CID)/sizeof(uint8_t)); i++)
    SACNPACKET._cid[i] = CID[i];

/********* Frame Layer *******************/

  flag_length = getSlots() + 88 + 0x7000;
  SACNPACKET._frame_flength_h = flag_length >> 8;
  SACNPACKET._frame_flength_l = flag_length & 0xFF;

  SACNPACKET._frame_vector = VECTOR_E131_DATA_PACKET;

  for(i = 0; i < NAME_MAX_LENGTH; i++)
    SACNPACKET._source_name[i] = name[i];

  SACNPACKET._priority = SACN_PRIORITY_DEFAULT;

  SACNPACKET._sync_address = 0x00;

  SACNPACKET._sequence = 0x01; //this increments as time goes on in sendDMXsACN (no seq 0!)

  SACNPACKET._options = 0x00;

  SACNPACKET._universe_h = 0x00; //this will be set in sendDMXsACN
  SACNPACKET._universe_l = 0x00;

/********* DMP Layer *******************/

  flag_length = getSlots() + 11 + 0x7000;
  SACNPACKET._dmp_flength_h = flag_length >> 8;
  SACNPACKET._dmp_flength_l = flag_length & 0xFF;

  SACNPACKET._dmp_vector = VECTOR_DMP_SET_PROPERTY;

  SACNPACKET._type = SACN_ADDRESS_DATA_TYPE;

  SACNPACKET._first_address = SACN_FIRST_ADDRESS_DEFAULT;

  SACNPACKET._address_increment = SACN_FIRST_ADDRESS_INCREMENT_DEFAULT;

  flag_length = getSlots() + 1;
  SACNPACKET._dmx_slots_h = flag_length >> 8;
  SACNPACKET._dmx_slots_l = flag_length && 0xFF;

  SACNPACKET._dmx_data = getDMXBuffer(); //just a pointer, will copy data in sendDMXsACN

}
