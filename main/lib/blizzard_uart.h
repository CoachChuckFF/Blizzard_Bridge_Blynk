#ifndef BLIZZARD_UART_H
#define BLIZZARD_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "blizzard_structs.h"

#define BLIZZARD_UART UART_NUM_1
#define BLIZZARD_TX_PIN 16
#define BLIZZARD_RX_PIN 34
#define BLIZZARD_BAUD		921600

#define BLIZZARD_RX_TO 1000

enum BLIZZARD_STATE {
  BLIZZARD_STATE_LISTEN,
  BLIZZARD_STATE_CHECK_HEADER,
  BLIZZARD_STATE_READ_OPCODE,
  BLIZZARD_STATE_READ_DATA_LENGTH,
  BLIZZARD_STATE_READ_DATA,
  BLIZZARD_STATE_SEND_DMX_RESPONSE,
  BLIZZARD_STATE_SEND_IP_RESPONSE,
  BLIZZARD_STATE_SEND_MODE_RESPONSE,
  BLIZZARD_STATE_SEDN_SLOT_RESPONSE,
  BLIZZARD_STATE_SEND_OK_RESPONSE,
  BLIZZARD_STATE_SEND_ERROR_RESPONSE
} BLIZZARD_STATE;

typedef struct ESP32BLIZZARD {
  uint8_t _dmx_state;
  uint8_t _enabled;
  BlizzardPacket _rx;
  BlizzardPacket _tx;
  //put in blizzard packet rec and trans
} ESP32BLIZZARD;

void startBlizzardUart(void);
void stopBlizzardUart(void);

void uart_blizzard_init(void);
void blizzard_listen(void);
void handle_blizzard_packet(uint8_t opcode);

#ifdef __cplusplus
}
#endif

#endif // ifndef BLIZZARD_UART_H
