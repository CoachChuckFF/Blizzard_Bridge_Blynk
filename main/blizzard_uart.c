#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lib/dmx.h"
#include "lib/blizzard_uart.h"

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


}

void stopBlizzardUart()
{
  return;
}
