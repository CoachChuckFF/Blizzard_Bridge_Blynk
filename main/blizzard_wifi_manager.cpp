#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "Arduino.h"
#include <WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <WiFiClient.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "lib/blizzard_nvs.h"
#include "lib/blizzard_wdmx.h"
#include "lib/blizzard_wifi_manager.h"

static const char *TAG = "WIFI MANAGER";

void start_wifi_manager(void)
{
  WiFiManager wifiManager;
  uint8_t ran_once_flag = 0;
  char temp[64];
  initArduino();

  Serial.begin(115200);

  reset_button_setup();
  //ESP.restart();
  if (!wifiManager.startConfigPortal((const char *) getName())) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

Serial.println(wifiManager.getSSID());
Serial.println(wifiManager.getPassword());

memset(&temp, 0, 64);

wifiManager.getSSID().toCharArray(temp, wifiManager.getSSID().length()+1);
setSSID(temp, (uint8_t) wifiManager.getSSID().length()+1);

memset(&temp, 0, 64);

wifiManager.getPassword().toCharArray(temp, wifiManager.getPassword().length()+1);
setPass(temp, (uint8_t) wifiManager.getPassword().length()+1);

setWifiManagerEnable(DISABLE);

ESP.restart();

/*MANAGER:
  if(!wifiManager.autoConnect((const char *) getName())) {
    //delay(3000);
    ESP.restart();
    //delay(5000);
  }


  //only ever run once
  ESP_LOGI(TAG, "RESET LEVEL %d", read_reset_button());
  if(!ran_once_flag++ && read_reset_button())
  {
    WiFi.disconnect();
    goto MANAGER;
  }*/

}

void clear_wifi_settings(void)
{
  wifi_config_t conf;

  *conf.sta.ssid = 0;
  *conf.sta.password = 0;

  esp_wifi_set_config(WIFI_IF_STA, &conf);

}

void reset_button_setup()
{
  gpio_config_t io_conf;

  io_conf.intr_type = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = RESET_WIFI_PIN_SEL;
  io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
}

uint8_t read_reset_button()
{
  return (uint8_t) gpio_get_level(RESET_WIFI_PIN);
}
