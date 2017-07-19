#include "lib/blizzard_wdmx.h"

static const char *TAG = "WDMX";

uint8_t WDMX_ENABLE = DISABLE;
uint8_t WDMX_POWER_ON = DISABLE;
uint8_t WDMX_BUSY = DISABLE;

void startWDMX(void)
{
  gpio_config_t io_conf;

  if(WDMX_ENABLE == ENABLE)
    stopWDMX();

  //PWR and BUTTON setup
  if(!GPIO_IS_VALID_OUTPUT_GPIO(WDMX_POWER_PIN) ||
      !GPIO_IS_VALID_OUTPUT_GPIO(WDMX_BUTTON_PIN))
      ESP_LOGI(TAG, "Invalid GPIO Output");

  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = WDMX_POWER_PIN_SEL | WDMX_BUTTON_PIN_SEL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;
  gpio_config(&io_conf);

  //LED gpio setup
  if(!GPIO_IS_VALID_GPIO(WDMX_RED_PIN) ||
      !GPIO_IS_VALID_GPIO(WDMX_GREEN_PIN) ||
      !GPIO_IS_VALID_GPIO(WDMX_BLUE_PIN))
      ESP_LOGI(TAG, "Invalid GPIO");

  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = WDMX_RED_PIN_SEL | WDMX_GREEN_PIN_SEL | WDMX_BLUE_PIN_SEL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;
  gpio_config(&io_conf);

  gpio_set_level(WDMX_POWER_PIN, LOW);
  gpio_set_level(WDMX_BUTTON_PIN, LOW);

  WDMX_ENABLE = ENABLE;
}
void stopWDMX(void)
{
  //nothing
  turn_off_wdmx();
  WDMX_ENABLE = DISABLE;
  return;
}

uint8_t isWDMXOn(void)
{
  return WDMX_POWER_ON;
}

uint8_t isWDMXBusy(void)
{
  return WDMX_BUSY;
}

uint8_t isWDMXEnabled(void)
{
  return WDMX_ENABLE;
}

void turn_on_wdmx(void)
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  gpio_set_level(WDMX_POWER_PIN, HIGH);
  WDMX_POWER_ON = ENABLE;
}

void turn_off_wdmx(void)
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  gpio_set_level(WDMX_POWER_PIN, LOW);
  WDMX_POWER_ON = DISABLE;
}

uint8_t read_wdmx_led(uint8_t index)
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  switch (index) {
    case WDMX_RED:
      return (uint8_t) gpio_get_level(WDMX_RED_PIN);
    break;
    case WDMX_GREEN:
      return (uint8_t) gpio_get_level(WDMX_GREEN_PIN);
    break;
    case WDMX_BLUE:
      return (uint8_t) gpio_get_level(WDMX_BLUE_PIN);
    break;
  }
  return 0; //default
}

uint8_t check_wdmx_color(uint8_t index)
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  switch(index)
  {
    case WDMX_COLOR_RED:
      return (!read_wdmx_led(WDMX_RED) && read_wdmx_led(WDMX_GREEN) && read_wdmx_led(WDMX_BLUE));
    break;
    case WDMX_COLOR_GREEN:
      return (read_wdmx_led(WDMX_RED) && !read_wdmx_led(WDMX_GREEN) && read_wdmx_led(WDMX_BLUE));
    break;
    case WDMX_COLOR_BLUE:
      return (read_wdmx_led(WDMX_RED) && read_wdmx_led(WDMX_GREEN) && !read_wdmx_led(WDMX_BLUE));
    break;
    case WDMX_COLOR_YELLOW:
      return (!read_wdmx_led(WDMX_RED) && !read_wdmx_led(WDMX_GREEN) && read_wdmx_led(WDMX_BLUE));
    break;
    case WDMX_COLOR_WHITE:
      return (!read_wdmx_led(WDMX_RED) && !read_wdmx_led(WDMX_GREEN) && !read_wdmx_led(WDMX_BLUE));
    break;
    case WDMX_COLOR_CYAN:
      return (read_wdmx_led(WDMX_RED) && !read_wdmx_led(WDMX_GREEN) && !read_wdmx_led(WDMX_BLUE));
    break;
    case WDMX_COLOR_MAGENTA:
      return (!read_wdmx_led(WDMX_RED) && read_wdmx_led(WDMX_GREEN) && !read_wdmx_led(WDMX_BLUE));
    break;
    case WDMX_COLOR_OFF:
      return (read_wdmx_led(WDMX_RED) && read_wdmx_led(WDMX_GREEN) && read_wdmx_led(WDMX_BLUE));
    break;

  }
  ESP_LOGI(TAG, "CHECK COLOR ERROR");
  return 0;
}

uint8_t get_wdmx_color()
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  uint8_t count = 0;

  count += (!read_wdmx_led(WDMX_RED)) ? 1 : 0;
  count += (!read_wdmx_led(WDMX_BLUE)) ? 2 : 0;
  count += (!read_wdmx_led(WDMX_GREEN)) ? 4 : 0;

  switch(count)
  {
    case 0:
      return WDMX_COLOR_OFF;
    break;
    case 1:
      return WDMX_COLOR_RED;
    break;
    case 2:
      return WDMX_COLOR_BLUE;
    break;
    case 4:
      return WDMX_COLOR_GREEN;
    break;
    case 3:
      return WDMX_COLOR_MAGENTA;
    break;
    case 5:
      return WDMX_COLOR_YELLOW;
    break;
    case 6:
      return WDMX_COLOR_CYAN;
    break;
    case 7:
      return WDMX_COLOR_WHITE;
    break;

  }
  ESP_LOGI(TAG, "GET COLOR ERROR");
  return 0;
}

void press_wdmx_button(uint32_t hold_time)
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  gpio_set_level(WDMX_BUTTON_PIN, HIGH);
  vTaskDelay(hold_time);
  gpio_set_level(WDMX_BUTTON_PIN, LOW);
}

void switch_wdmx_button_on(void)
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  gpio_set_level(WDMX_BUTTON_PIN, HIGH);
}

void switch_wdmx_button_off(void)
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  gpio_set_level(WDMX_BUTTON_PIN, LOW);
}

uint8_t enter_wdmx_config_mode(void)
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  //go into config mode
  uint8_t timeout = 0;
  turn_off_wdmx();
  ESP_LOGI(TAG, "WAITING FOR OFF");
  while(!check_wdmx_color(WDMX_COLOR_OFF)){if(timeout++ > CONNECT_TIMEOUT){return CONNECT_ERROR;} vTaskDelay(CONNECT_TIMOUT_DELAY);}
  timeout = 0;
  switch_wdmx_button_on();
  ESP_LOGI(TAG, "WAITING FOR WHITE");
  while(!check_wdmx_color(WDMX_COLOR_WHITE)){if(timeout++ > CONNECT_TIMEOUT){return CONNECT_ERROR;} vTaskDelay(CONNECT_TIMOUT_DELAY);}
  timeout = 0;
  turn_on_wdmx();
  vTaskDelay(WDMX_SWITCH_DELAY);
  switch_wdmx_button_off();
  ESP_LOGI(TAG, "WAITING FOR NOT WHITE");
  while(check_wdmx_color(WDMX_COLOR_WHITE)){if(timeout++ > CONNECT_TIMEOUT){return CONNECT_ERROR;} vTaskDelay(CONNECT_TIMOUT_DELAY);}
  return CONNECT_SUCCESS;
}

//MUST call enter_wdmx_config_mode
uint8_t select_wdmx_mode(uint8_t mode)
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  uint8_t i = 0;
  ESP_LOGI(TAG, "selecting mode %d", get_wdmx_color());
  //find mode
  while(1)
  {
    if(check_wdmx_color(mode))
      break;
    ESP_LOGI(TAG, "Current Color: %d, Mode Requested: %d", get_wdmx_color(), mode);
    press_wdmx_button(WDMX_PRESS_SEC);
    vTaskDelay(WDMX_1_SEC);
    if(i++ > WDMX_MAX_MODES)
    {
      ESP_LOGI(TAG, "CANNOT FIND MODE");
      return CONNECT_ERROR;
    }
  }

  //select mode
  switch_wdmx_button_on();
  while(!check_wdmx_color(WDMX_COLOR_WHITE)){;;}
  switch_wdmx_button_off();
  ESP_LOGI(TAG, "mode found!");
  return CONNECT_SUCCESS;
}

uint8_t easy_wdmx_connect(void)
{
  if(!WDMX_ENABLE)
  {
    ESP_LOGI(TAG, "WDMX DISABLED")
    return;
  }
  WDMX_BUSY = ENABLE;
  ESP_LOGI(TAG, "EASY CONNECT START");
  if(enter_wdmx_config_mode())
    goto CON_ERROR;
  if(select_wdmx_mode(WDMX_MODE_TRANSMIT))
    goto CON_ERROR;
  WDMX_BUSY = DISABLE;
  return CONNECT_SUCCESS;

CON_ERROR:
  WDMX_BUSY = DISABLE;
  return CONNECT_ERROR;
}
