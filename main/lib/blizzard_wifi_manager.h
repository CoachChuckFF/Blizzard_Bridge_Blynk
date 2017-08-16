#ifndef BLIZZARD_WIFI_MANAGER_H
#define BLIZZARD_WIFI_MANAGER_H


#ifdef __cplusplus
extern "C" {
#endif

#include "driver/uart.h"
#include "driver/gpio.h"
#include "dmx.h"


#define RESET_WIFI_PIN GPIO_NUM_39
#define RESET_WIFI_PIN_SEL GPIO_SEL_39

void start_wifi_manager(void);
void clear_wifi_settings(void);
void reset_button_setup(void);
uint8_t read_reset_button(void);

#ifdef __cplusplus
}
#endif

#endif
