#ifndef _BLIZZARD_WDMX_H
#define _BLIZZARD_WDMX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "dmx.h"

#define WDMX_RED_PIN 35
#define WDMX_GREEN_PIN 32
#define WDMX_BLUE_PIN 36
#define WDMX_POWER_PIN 15
#define WDMX_BUTTON_PIN 33

#define WDMX_RED_PIN_SEL GPIO_SEL_35
#define WDMX_GREEN_PIN_SEL GPIO_SEL_32
#define WDMX_BLUE_PIN_SEL GPIO_SEL_36
#define WDMX_POWER_PIN_SEL GPIO_SEL_15
#define WDMX_BUTTON_PIN_SEL GPIO_SEL_33

#define WDMX_RED 0
#define WDMX_GREEN 1
#define WDMX_BLUE 2

#define WDMX_COLOR_OFF 0
#define WDMX_COLOR_RED 1
#define WDMX_COLOR_GREEN 2
#define WDMX_COLOR_BLUE 3
#define WDMX_COLOR_YELLOW 4
#define WDMX_COLOR_CYAN 5
#define WDMX_COLOR_MAGENTA 6
#define WDMX_COLOR_WHITE 7

#define WDMX_PRESS_SEC 500
#define WDMX_1_SEC 2000
#define WDMX_3_SEC 6000
#define WDMX_SWITCH_DELAY 33

#define WDMX_MODE_WIDMX WDMX_COLOR_RED
#define WDMX_MODE_RECEIVE WDMX_COLOR_GREEN
#define WDMX_MODE_TRANSMIT WDMX_COLOR_BLUE
#define WDMX_MODE_GZ WDMX_COLOR_YELLOW

#define CONNECT_TIMEOUT 50
#define CONNECT_TIMOUT_DELAY 50

#define CONNECT_ERROR 3
#define CONNECT_SUCCESS 0

#define WDMX_MAX_MODES 9 //checks modes this many times

//initailizes gpios
void startWDMX(void);
void stopWDMX(void);
uint8_t isWDMXOn(void);
uint8_t isWDMXBusy(void);
uint8_t isWDMXEnabled(void);
void turn_on_wdmx(void);
void turn_off_wdmx(void);
void press_wdmx_button(uint32_t hold_time);
void switch_wdmx_button_on(void);
void switch_wdmx_button_off(void);
uint8_t read_wdmx_led(uint8_t index);
uint8_t check_wdmx_color(uint8_t index);
uint8_t get_wdmx_color();
uint8_t enter_wdmx_config_mode(void);
uint8_t select_wdmx_mode(uint8_t mode);
uint8_t easy_wdmx_connect(void);


#ifdef __cplusplus
}
#endif

#endif
