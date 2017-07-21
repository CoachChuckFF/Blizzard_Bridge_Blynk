/**************************************************************************/
/*!
    @file     esp32_dmx.c
    @author   Christian Krueger
    @license  BSD (see esp32_dmx.h)
    @copyright 2017 by Christian Krueger
    DMX UART Driver for esp32_dmx
		Uses UART2 for tx and rx
    @section  HISTORY
    v1.0 - First release
*/
/**************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lib/dmx.h"
#include "lib/dmx_uart.h"
#include "soc/uart_struct.h"

static const char *TAG = "DMX UART";

/*------------------------------------------------- starDMX -----
 |  Function startDMX
 |
 |  Purpose:  To establish DMX functionality on the DMX_UART.
 |	DMX_UART = UART2, beacuse UART0 and 1 are already in use.
 |	To read dmx data just access DMX._dmx_data[] from the caller
 |	Modify it at will.
 |
 |	To switch direction mid program, just recall this function
 |	with a diffrent direction.
 |
 |  Parameters:
 |      dir
 |				-SEND, sends the contents of DMX._dmx_data. forever.
 |				-RECEIVE, reads DMX from DMX_UART into DMX._dmx_data
 |			slots
 |				-Can be between 512 and 24 - length of DMX to send/recieve
 |
 |  Returns:  N/A
 *-------------------------------------------------------------------*/
void startDMXUart (uint8_t dir)
{
		gpio_config_t io_conf;

		//GPIO setup
    //This sets the direction for the MAX 485 chip
		//DIRECTION_PIN should be wired directly to
		//RE and DE on the MAX485 chip
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	  io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1 << DIRECTION_PIN );
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;

    //configure GPIO with the given settings
    gpio_config(&io_conf);
		gpio_set_level(DIRECTION_PIN, dir);

		uart_dmx_init(DMX_DATA_BAUD, dir);

}


/*------------------------------------------------- uart_dmx_init -----
 |  Function uart_dmx_init
 |
 |  Purpose:  Start the UART periferal to DMX standards
 |		DMX is 8N2 at 250k Baud. There is a hardware bug where
 |		2 stop bits don't work correctly so we set it to 1 stop bits
 |		and enable rs485's dL1 mode to compensate
 |
 |  Parameters:
 |      Baudrate
 |				-DMX_DATA_BAUD 250K for DMX data
 |				-DMX_BREAK_BAUD 88K to send a break
 |
 |  Returns:  N/A
 *-------------------------------------------------------------------*/
void uart_dmx_init(int baudrate, uint8_t dir)
{
	uart_config_t uart_config = {
	.baud_rate = DMX_DATA_BAUD,
	.data_bits = UART_DATA_8_BITS,
	.parity = UART_PARITY_DISABLE,
	.stop_bits = UART_STOP_BITS_2,
	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	.rx_flow_ctrl_thresh = 122
};

uart_param_config(DMX_UART, &uart_config);

uart_set_pin(DMX_UART, DMX_TX_PIN, DMX_RX_PIN,
	UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);



uart_driver_install(DMX_UART, DMX_MAX_SLOTS * 2, 0, 0, NULL, ESP_INTR_FLAG_LOWMED);

changeDirectionDMXUart(dir);

}

void changeDirectionDMXUart(uint8_t dir)
{

	gpio_set_level(DIRECTION_PIN, dir);

	if(dir == SEND)
	{
		 uart_enable_tx_intr(DMX_UART, 1, 1); //threshold = 1 enable = 1;
		 //xTaskCreate(&DMXTx, "DMX UART TX", 1024 * 2, NULL, 15, NULL);
	}
	else
	{
		uart_set_baudrate(DMX_UART, DMX_DATA_BAUD);
		uart_set_stop_bits(DMX_UART, UART_STOP_BITS_2);
		uart_disable_tx_intr(DMX_UART);
	}

if(dir == SEND)
{
	ESP_LOGI(TAG, "SEND");
}
else
{
	ESP_LOGI(TAG, "RECEIVE");
}

}

/*------------------------------------------------- stopDMX -----
 |  Function stopDMX
 |
 |  Purpose:  To stop DMX functionality and free up DMX_UART
 |
 |  Parameters: N/A
 |
 |  Returns:  N/A
 *-------------------------------------------------------------------*/
void stopDMXUart()
{
	ESP_LOGI(TAG, "stop uart dmx");
	uart_disable_intr_mask(DMX_UART, UART_INTR_MASK); //disable all inturrupts
	uart_driver_delete(DMX_UART);

	return;
}
