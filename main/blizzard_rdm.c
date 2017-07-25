#include <stdio.h>
#include "lib/blizzard_rdm.h"
#include "lib/blizzard_nvs.h"
#include "lib/dmx.h"

static const char *TAG = "RDM";

static rdm_dev dev;
uint8_t* own_uuid;
uint8_t HANDLER_RUNNING;

void rdmHandler(void)
{
  //waits for rdm requests
  /*if(dev == NULL)
  {
    ESP_LOGI(TAG, "HANDLER: NULL DEV");
    return;
  }*/
  getXRDMGroup();
  if(dev.xRDMGroup == NULL)
  {
    ESP_LOGI(TAG, "HANDLER GROUP NULL");
    return;
  }

  ESP_LOGI(TAG, "RDM LOOP");

  while(1)
  {
    dev.xRDMBits = xEventGroupWaitBits(dev.xRDMGroup, RDM_BITS,
                        true, false, portMAX_DELAY);

    if(dev.xRDMBits & RDM_BITS)
    {
      parse_rdm_packet();
      printRDMPacket(RECEIVE);
      if(check_rdm_checksum())
      {
        switch(dev._rx._mdb._cc)
        {
          case 0:

          break;
          case 1:

          break;
        }
      }
    }
    else
    {
      //timout
    }
  }

  vTaskDelete(NULL);

}

void parse_rdm_packet(void)
{
  uint8_t pdl, i;
  pdl = dev._rx._mdb._pdl;
  dev._rx._checksum[0] = dev._rx._mdb._pd[pdl];
  dev._rx._checksum[1] = dev._rx._mdb._pd[pdl + 1];

}

uint8_t check_rdm_checksum(void)
{
  uint16_t checksum = 0, check;
  uint8_t i;

  for(i = 0; i < dev._rx._message_len - 2; i++) //minus 2 for checksum
  {
    checksum += ((uint8_t *)&dev._rx)[i];
  }

  check = (((uint16_t)dev._rx._checksum[0]) << 8) | ((uint16_t)dev._rx._checksum[1] && 0xFF);

  if(check != checksum)
  {
    ESP_LOGI(TAG, "CHECK ERROR- %d != %d", checksum, check);
    return 0;
  }

  ESP_LOGI(TAG, "CHECKSUM MATCH");
  return 1;

}

void set_rdm_unique_packet(void)
{
  uint8_t i;
  uint16_t checksum = 0;
  /*
  if(dev == NULL)
  {
    ESP_LOGI(TAG, "NULL DEV Setting Unique Packet");
    return;
  }*/

  //preamble setup
  memset(dev._tx_disco._preamble, 0xFE, 7);
  dev._tx_disco._preamble_seperator = 0xAA;

  //uuid setup
  dev._tx_disco._man_id_msb[0] = own_uuid[0] | 0xAA;
  dev._tx_disco._man_id_msb[1] = own_uuid[0] | 0x55;

  dev._tx_disco._man_id_lsb[0] = own_uuid[1] | 0xAA;
  dev._tx_disco._man_id_lsb[1] = own_uuid[1] | 0x55;

  dev._tx_disco._dev_id3[0] = own_uuid[2] | 0xAA;
  dev._tx_disco._dev_id3[1] = own_uuid[2] | 0x55;

  dev._tx_disco._dev_id2[0] = own_uuid[3] | 0xAA;
  dev._tx_disco._dev_id2[1] = own_uuid[3] | 0x55;

  dev._tx_disco._dev_id1[0] = own_uuid[4] | 0xAA;
  dev._tx_disco._dev_id1[1] = own_uuid[4] | 0x55;

  dev._tx_disco._dev_id0[0] = own_uuid[5] | 0xAA;
  dev._tx_disco._dev_id0[1] = own_uuid[5] | 0x55;

  for(i = 0; i < 12; i++)
    checksum += ((uint8_t *) &dev._tx_disco)[i];

  dev._tx_disco._checksum_msb[0] = (uint8_t) ((checksum >> 8) | 0xAA) & 0xFF;
  dev._tx_disco._checksum_msb[1] = (uint8_t) ((checksum >> 8) | 0x55) & 0xFF;

  dev._tx_disco._checksum_msb[0] = (uint8_t) ((checksum & 0xFF) | 0xAA) & 0xFF;
  dev._tx_disco._checksum_msb[1] = (uint8_t) ((checksum & 0xFF) | 0x55) & 0xFF;
}

//should only be called after uuid is established
void startRDM(void)
{
  uint8_t i;

/*
  if(dev != NULL)
    stopRDM();*/

/*
  dev = (rdm_dev *)malloc(sizeof(rdm_dev));
  if(dev == NULL)
  {
    ESP_LOGI(TAG, "Malloc Returned NULL");
    return;
  }*/

  memset(&dev, 0, sizeof(rdm_dev));

  dev.xRDMGroup = xEventGroupCreate();

  if(dev.xRDMGroup == NULL)
  {
    ESP_LOGI(TAG, "xEventGroupCreate Retured NULL");
    stopRDM();
    return;
  }

  own_uuid = getOwnUUID();

  dev._enabled = 1;
  dev._muted = 0;

  set_rdm_unique_packet();

  xTaskCreatePinnedToCore(&rdmHandler, "RDM HANDLER", 2048, NULL, tskIDLE_PRIORITY, &dev.xRDMHandle, 0);
}


//free rdm structure
void stopRDM()
{
  /*
  if(dev == NULL)
  {
    ESP_LOGI(TAG, "Nothing to Stop");
    return;
  }*/

  if(dev.xRDMHandle != NULL)
    vTaskDelete( dev.xRDMHandle );

  if(dev.xRDMGroup != NULL)
    vEventGroupDelete(dev.xRDMGroup);

  //free(dev);
}

rdm_dev* getRDMDev(void)
{
  /*
    if(dev == NULL)
      ESP_LOGI(TAG, "GET DEV - NULL DEV");
*/
    return &dev;
}

EventGroupHandle_t* getXRDMGroup(void)
{
  /*
  if(dev == NULL)
  {
    ESP_LOGI(TAG, "GET GROUP - NULL DEV");
    return NULL;
  }
  */
  if(dev.xRDMGroup == NULL)
  {
    ESP_LOGI(TAG, "GET GROUP - NULL GROUP");
    return NULL;
  }

  return &dev.xRDMGroup;
}

BaseType_t* getXRDMTaskWoken(void)
{
  /*
  if(dev == NULL)
  {
    ESP_LOGI(TAG, "GET TASKWOKEN - NULL DEV");
    return NULL;
  }
*/
  return &dev.xRDMTaskWoken;
}

rdm_packet* getTxRDMPacket()
{/*
  if(dev == NULL)
  {
    ESP_LOGI(TAG, "Can't get TX RDM (NULL)");
    return NULL;
  }*/
  return &dev._tx;
}

rdm_packet* getRxRDMPacket()
{/*
  if(dev == NULL)
  {
    ESP_LOGI(TAG, "Can't get RX RDM (NULL)");
    return NULL;
  }*/
  return &dev._rx;
}

rdm_unique_packet* getTxDiscoRDMPacket()
{/*
  if(dev == NULL)
  {
    ESP_LOGI(TAG, "Can't get TXDISCO RDM (NULL)");
    return NULL;
  }*/
  return &dev._tx_disco;
}

uint8_t isRDMEnabled(void)
{/*
  if(dev == NULL)
    return 0;*/
  return dev._enabled;
}

uint8_t isRDMMuted(void)
{/*
  if(dev == NULL)
  {
    ESP_LOGI(TAG, "Not Enabled");
    return 0;
  }*/
  return dev._muted;
}

void printRDMPacket(uint8_t dir)
{
  uint8_t i;
  rdm_packet* pack = (dir == RECEIVE) ? &dev._rx : &dev._tx;

  printf("-----------------RDM DATA---------------------\n");
  for(i = 0; i < pack->_message_len; i++)
  {

    printf(" %d ", ((uint8_t *)pack)[i]);
    if(!(i % 8))
      printf("\n");

  }
  printf("----------------RDM DATA-----------------\n");

}
