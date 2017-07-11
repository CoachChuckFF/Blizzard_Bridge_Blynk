#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lib/dmx.h"
#include "lib/blizzard_uart.h"
#include "lib/blizzard_structs.h"

static const char *TAG = "BLIZZARD UART";

ESP32BLIZZARD BLIZZARD;

volatile uint8_t tx_done;

#define UART_ENTER_CRITICAL(mux)    portENTER_CRITICAL(mux)
#define UART_EXIT_CRITICAL(mux)     portEXIT_CRITICAL(mux)
#define UART_ENTER_CRITICAL_ISR(mux)    portENTER_CRITICAL_ISR(mux)
#define UART_EXIT_CRITICAL_ISR(mux)     portEXIT_CRITICAL_ISR(mux)

static DRAM_ATTR uart_dev_t* const UART[UART_NUM_MAX] = {&UART0, &UART1, &UART2};
static portMUX_TYPE uart_spinlock[UART_NUM_MAX] = {portMUX_INITIALIZER_UNLOCKED, portMUX_INITIALIZER_UNLOCKED, portMUX_INITIALIZER_UNLOCKED};

void startBlizzardUart()
{
  if(BLIZZARD._enabled == ENABLE)
    stopBlizzardUart();

  BLIZZARD._enabled = ENABLE;
  BLIZZARD._dmx_state = BLIZZARD_STATE_LISTEN;

  uart_blizzard_init();
}

void stopBlizzardUart()
{
  return;
}

void uart_blizzard_init()
{
  uart_config_t uart_config = {
    .baud_rate = BLIZZARD_BAUD,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122
  };

  uart_param_config(BLIZZARD_UART, &uart_config);

  uart_set_pin(BLIZZARD_UART, BLIZZARD_TX_PIN, BLIZZARD_RX_PIN,
    UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  uart_driver_install(BLIZZARD_UART, sizeof(BlizzardPacket) * 3, sizeof(BlizzardPacket) * 2, 0, NULL, 0);

  xTaskCreate(&blizzard_listen, "Blizzard Listen", 2048, NULL, 15, NULL);
}

void blizzard_listen()
{
  int ret_val;
  ESP_LOGI(TAG, "Listening");
  while(BLIZZARD._enabled)
  {
ERROR_BLIZZARD:
    uart_flush(BLIZZARD_UART);

    ret_val = uart_read_bytes(BLIZZARD_UART,
                              ((uint8_t *)&(BLIZZARD._rx)),
                              sizeof(BlizzardPacket) - (DMX_MAX_SLOTS * sizeof(uint8_t)),
                              BLIZZARD_RX_TO);
    if(ret_val == -1)
    {
      ESP_LOGI(TAG, "read byte fail");
      goto ERROR_BLIZZARD;
    }
    if(ret_val != (sizeof(BlizzardPacket) - (DMX_MAX_SLOTS * sizeof(uint8_t))))
    {
      ESP_LOGI(TAG, "partial packet found");
      goto ERROR_BLIZZARD;
    }
    //ESP_LOGI(TAG, "ret_val %d", ret_val);
    if(BLIZZARD._rx._header[0] == 'C' &&
       BLIZZARD._rx._header[1] == 'A' &&
       BLIZZARD._rx._header[2] == 'K')
    {

      ret_val = uart_read_bytes(BLIZZARD_UART,
                                BLIZZARD._rx._data,
                                BLIZZARD._rx._data_count * sizeof(uint8_t),
                                BLIZZARD_RX_TO);
      if(ret_val == -1)
      {
        ESP_LOGI(TAG, "read data fail");
        ESP_LOGI(TAG, "failed data size: %d", BLIZZARD._rx._data_count);
        goto ERROR_BLIZZARD;
      }
      handle_blizzard_packet(BLIZZARD._rx._opcode);
    }


  }
}

void handle_blizzard_packet(uint8_t opcode)
{
  ESP_LOGI(TAG, "handle packet");
  return;
}
