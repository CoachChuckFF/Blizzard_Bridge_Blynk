/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include "lib/dmx_uart.h"
#include "lib/dmx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "DMX UART EXAMPLE";

extern ESP32DMX DMX;

static DRAM_ATTR uart_dev_t* const UART[UART_NUM_MAX] = {&UART0, &UART1, &UART2};
void hello_task(void *pvParameter)
{
    int i;
    int j;
    startDMX(DMX_SEND, 512);

    while(1) {

        
        if(DMX._direction == DMX_SEND)
        {

          for(i = 1; i <513; i++)
          {
            DMX._dmx_data[i] = 0xFF;
          }

        }else
        {
          printf("\n------------ START -------------\n");

          for(i = 0; i < 513; i++)
          {
            if(!(i%32))
              printf("\n");
            if(DMX._dmx_data[i])
              printf(" %d-%d ", i, DMX._dmx_data[i]);
          }

          printf("\n------------ END -------------\n");
        }
        vTaskDelay(50 / portTICK_RATE_MS);

    }

    fflush(stdout);
    system_restart();
}

void app_main()
{
    nvs_flash_init();
    system_init();
    xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
}
