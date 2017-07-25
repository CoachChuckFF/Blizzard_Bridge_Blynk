#ifndef _BLIZZARD_RDM_H
#define _BLIZZARD_RDM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rdm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <stdint.h>

#define RDM_BITS ( 1 << 3 )

//should be always ready to send
typedef struct rdm_unique_packet {
  uint8_t _preamble[7]; //all 0xFE
  uint8_t _preamble_seperator; //0xAA
  uint8_t _man_id_msb[2]; //or with AA then 55
  uint8_t _man_id_lsb[2]; //or with AA then 55
  uint8_t _dev_id3[2]; //ditto
  uint8_t _dev_id2[2]; //ditto
  uint8_t _dev_id1[2]; //ditto
  uint8_t _dev_id0[2]; //ditto
  uint8_t _checksum_msb[2]; //ditto
  uint8_t _checksum_lsb[2]; //ditto

} rdm_unique_packet;

//message data block
typedef struct rdm_mdb {
  uint8_t _cc; //command class
  uint8_t _pid; //parameter id
  uint8_t _pdl; //parameter data length
  uint8_t _pd[233]; //parameter data
} rdm_mdb;

//standard packet structure
typedef struct rdm_packet {
  uint8_t _sc;
  uint8_t _sub_sc;
  uint8_t _message_len;
  uint8_t _dest_uid[6];
  uint8_t _src_uid[6];
  uint8_t _tn; //transaction number
  uint8_t _port_id; //port id/response typedef
  uint8_t _message_count;
  uint8_t _sub_device[2];
  rdm_mdb _mdb;
  uint8_t _checksum[2];
} rdm_packet;

typedef struct rdm_dev {
  uint8_t _muted;
  uint8_t _enabled;
  rdm_packet _tx;
  rdm_packet _rx;
  rdm_unique_packet _tx_disco;
  EventGroupHandle_t xRDMGroup;
  TaskHandle_t xRDMHandle;
  BaseType_t xRDMTaskWoken;
  EventBits_t xRDMBits;
} rdm_dev;

void rdmHandler(void);

void startRDM(void);
void stopRDM(void);

void set_rdm_unique_packet(void);

void parse_rdm_packet(void);
uint8_t check_rdm_checksum(void);

rdm_dev* getRDMDev(void);
EventGroupHandle_t* getXRDMGroup(void);
BaseType_t* getXRDMTaskWoken(void);
rdm_packet* getTxRDMPacket(void);
rdm_packet* getRxRDMPacket(void);
rdm_unique_packet* getTxDiscoRDMPacket(void);

uint8_t isRDMEnabled(void);
uint8_t isRDMMuted(void);

void printRDMPacket(uint8_t dir);

#ifdef __cplusplus
}
#endif

#endif
