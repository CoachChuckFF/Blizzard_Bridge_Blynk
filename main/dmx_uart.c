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

static const char *TAG = "DMX UART";

ESP32DMX DMX;

volatile uint8_t tx_done;

#define UART_ENTER_CRITICAL(mux)    portENTER_CRITICAL(mux)
#define UART_EXIT_CRITICAL(mux)     portEXIT_CRITICAL(mux)
#define UART_ENTER_CRITICAL_ISR(mux)    portENTER_CRITICAL_ISR(mux)
#define UART_EXIT_CRITICAL_ISR(mux)     portEXIT_CRITICAL_ISR(mux)

static DRAM_ATTR uart_dev_t* const UART[UART_NUM_MAX] = {&UART0, &UART1, &UART2};
static portMUX_TYPE uart_spinlock[UART_NUM_MAX] = {portMUX_INITIALIZER_UNLOCKED, portMUX_INITIALIZER_UNLOCKED, portMUX_INITIALIZER_UNLOCKED};

/*------------------------------------------------- starDMX -----
 |  Function startDMX
 |
 |  Purpose:  To establish DMX functionality on the UART_DMX.
 |	UART_DMX = UART2, beacuse UART0 and 1 are already in use.
 |	To read dmx data just access DMX._dmx_data[] from the caller
 |	Modify it at will.
 |
 |	To switch direction mid program, just recall this function
 |	with a diffrent direction.
 |
 |  Parameters:
 |      dir
 |				-DMX_SEND, sends the contents of DMX._dmx_data. forever.
 |				-DMX_RECEIVE, reads DMX from DMX_UART into DMX._dmx_data
 |			slots
 |				-Can be between 512 and 24 - length of DMX to send/recieve
 |
 |  Returns:  N/A
 *-------------------------------------------------------------------*/
void startDMXUart (uint8_t dir)
{
		gpio_config_t io_conf;

		if(DMX._enabled == DMX_ENABLE)
			stopDMXUart();

		DMX._enabled = DMX_ENABLE;
		DMX._dmx_state = DMX_STATE_START;
		DMX._slots = getSlots();
		DMX._idle_count = 0;
		DMX._current_slot = 0;
		DMX._direction = dir;
		DMX._dmx_data = getDMXBuffer();

		//clear DMX data
		clearDMX();

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

		uart_init(DMX_DATA_BAUD);

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
	uart_disable_intr_mask(DMX_UART, UART_INTR_MASK); //disable all inturrupts
	uart_isr_free(DMX_UART);
	uart_flush(DMX_UART);
	DMX._enabled = DMX_DISABLE;
}

/*------------------------------------------------- uart_init -----
 |  Function uart_init
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
void uart_init(int baudrate)
{

uart_config_t uart_config = {
 .baud_rate = baudrate,
 .data_bits = UART_DATA_8_BITS,
 .parity = UART_PARITY_DISABLE,
 .stop_bits = UART_STOP_BITS_2,
 .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
 .rx_flow_ctrl_thresh = 1,
};

//Set UART parameters
uart_param_config(DMX_UART, &uart_config);

uart_set_pin(DMX_UART, DMX_TX_PIN, DMX_RX_PIN,
	UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

if(DMX._direction == DMX_SEND)
{
	//enable tx fifo full interrupt
	uart_isr_register(DMX_UART, (void *) txEmptyInterruptHandler, NULL, ESP_INTR_FLAG_LOWMED, NULL);
	UART_ENTER_CRITICAL(&uart_spinlock[DMX_UART]);
	UART[DMX_UART]->int_ena.txfifo_empty = 1;
	UART[DMX_UART]->int_ena.tx_done = 1;
	UART[DMX_UART]->conf1.txfifo_empty_thrhd = 1;
	UART_EXIT_CRITICAL(&uart_spinlock[DMX_UART]);
	xTaskCreate(&handleMAB, "MAB", 2048, NULL, 5, NULL);
}
else //DMX_RECEIVE
{
	uart_isr_register(DMX_UART, (void *) receiveInterruptHandler, NULL, ESP_INTR_FLAG_LOWMED, NULL);
	UART_ENTER_CRITICAL(&uart_spinlock[DMX_UART]);
	//set interrupts
	UART[DMX_UART]->int_ena.rxfifo_full = 1;
	UART[DMX_UART]->int_ena.brk_det = 1;
	UART[DMX_UART]->conf1.rxfifo_full_thrhd = 1;
	UART_EXIT_CRITICAL(&uart_spinlock[DMX_UART]);
}

}
/*---------------------------------------- handleMAB -----
 |  Function handleMAB
 |
 |  Purpose:  This is a seperate thread that deals with the
 |	DMX Tx state machine, it works as txEmptyInterruptHandler
 |	describes
 |
 |
 |  Parameters: N/A
 |
 |  Returns:  N/A
 *-------------------------------------------------------------------*/
static void handleMAB()
{
	uint32_t i;
	ESP_LOGI(TAG, "LOOP");
	while(DMX._enabled)
	{
		switch(DMX._dmx_state)
		{
			case DMX_STATE_START:
				uart_set_baudrate(DMX_UART, DMX_DATA_BAUD);
				uart_set_stop_bits(DMX_UART, UART_STOP_BITS_2);
				UART[DMX_UART]->int_ena.txfifo_empty = 1;
				DMX._current_slot = 0;
				DMX._dmx_state = DMX_STATE_DATA;
			break;
			case DMX_STATE_DATA:
				;;//handled in isr
			break;
			case DMX_STATE_BREAK:
				UART[DMX_UART]->int_ena.tx_done = 1;
				while(!tx_done){;;} //wait for last byte of data

				uart_set_baudrate(DMX_UART, DMX_BREAK_BAUD);
				uart_set_stop_bits(DMX_UART, UART_STOP_BITS_1);
				DMXTx(0x00); //send break
				DMX._dmx_state = DMX_STATE_MAB;
			break;
			case DMX_STATE_MAB:
				while(!tx_done){;;} //wait for break byte

				DMX._dmx_state = DMX_STATE_START;
			break;
		}
	}
}

/*------------------------------------------------- DMXTx -----
 |  Function DMXTx
 |
 |  Purpose:  Safe thread sends a byte to fifo
 |
 |  Parameters: N/A
 |
 |  Returns:  N/A
 *-------------------------------------------------------------------*/
static void DMXTx(uint8_t value)
{
	WRITE_PERI_REG(UART_FIFO_AHB_REG(DMX_UART), value);
	if(DMX._dmx_state != DMX_STATE_DATA || DMX._current_slot == DMX._slots + 1)
	{
		UART_ENTER_CRITICAL(&uart_spinlock[DMX_UART]);
		UART[DMX_UART]->int_ena.tx_done = 1;
		tx_done = 0;
		UART_EXIT_CRITICAL(&uart_spinlock[DMX_UART]);
	}

}
/*---------------------------------------- txEmptyInterruptHandler -----
 |  Function txEmptyInterruptHandler
 |
 |  Purpose:  The is where the DMX magic happens, it places
 |	one byte of DMX._dmx_data data directly into the UART fifo to be
 |	transmitted. Then once the fifo is empty, it does it again.
 |	It also tracks the states, so when its time to send a break
 |	it drops the baud to DMX_BREAK_BAUD and sends a 0 (looks like
 |	a break) then it spin waits of the MAB (unused UART is pulled high)
 |	Then it will send a start code.
 |
 |
 |  Parameters: N/A
 |
 |  Returns:  N/A
 *-------------------------------------------------------------------*/
static void txEmptyInterruptHandler(void)
{
	uint8_t done = UART[DMX_UART]->int_st.tx_done;

	UART[DMX_UART]->int_clr.txfifo_empty = 1;
	UART[DMX_UART]->int_clr.tx_done = 1;

	UART[DMX_UART]->int_ena.val = 0;

	UART_ENTER_CRITICAL_ISR(&uart_spinlock[DMX_UART]);

	if( done )
	{
		tx_done = 1;
		goto EXIT_ISR;
	}

	if(DMX._dmx_state == DMX_STATE_DATA)
	{
			WRITE_PERI_REG(UART_FIFO_AHB_REG(DMX_UART),(0xFF & DMX._dmx_data[DMX._current_slot++]));

			if(DMX._current_slot > DMX._slots)
			{
				DMX._dmx_state = DMX_STATE_BREAK;
				tx_done = 0;
				//UART[DMX_UART]->int_ena.tx_done = 1;
			}
			else
				UART[DMX_UART]->int_ena.txfifo_empty = 1;
	}

EXIT_ISR:

	UART_EXIT_CRITICAL_ISR(&uart_spinlock[DMX_UART]);
}

/*---------------------------------------- receiveInterruptHandler -----
 |  Function receiveInterruptHandler
 |
 |  Purpose: Receives DMX data and puts it into DMX._dmx_data
 |	Pretty simple: If break detected, the next byte should be a
 |	start byte. It'll collect data until the next break.
 |
 |	Needs to be a spinlock so the RTOS does not interrupt it
 |
 |  Parameters: N/A
 |
 |  Returns:  N/A
 *-------------------------------------------------------------------*/
static void receiveInterruptHandler() {
	uint8_t incoming_byte;

	UART_ENTER_CRITICAL_ISR(&uart_spinlock[DMX_UART]);

	if ( UART[DMX_UART]->int_st.brk_det ) {				//break detected
		DMX._dmx_state = DMX_STATE_BREAK;
		DMX._current_slot = 0;
		UART[DMX_UART]->int_clr.brk_det = 1;
		goto EXIT_ISR;
	}
	incoming_byte = UART[DMX_UART]->fifo.rw_byte;
	switch ( DMX._dmx_state ) {

	case DMX_STATE_DATA:

		DMX._dmx_data[DMX._current_slot++] = incoming_byte;
		if ( DMX._current_slot > DMX_MAX_SLOTS ) {
			DMX._dmx_state = DMX_STATE_IDLE;			// go to idle, wait for next break
		}

		break;

		case DMX_STATE_BREAK:
			if ( incoming_byte == 0 ) {						//start code == zero (DMX)
				DMX._dmx_state = DMX_STATE_DATA;
				DMX._current_slot = 0;
			} else {
				DMX._dmx_state = DMX_STATE_IDLE;
			}
			break;

	}

	UART[DMX_UART]->int_clr.rxfifo_full = 1;

EXIT_ISR:

	UART_EXIT_CRITICAL_ISR(&uart_spinlock[DMX_UART]);

}
