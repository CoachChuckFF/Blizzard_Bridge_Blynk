/* Uart Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Arduino.h"
#if defined(ESP8266)
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#else
#include <WiFi.h>          //https://github.com/esp8266/Arduino
#endif

//needed for library
#include <DNSServer.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "lib/blizzard_blynk.h"
#include "lib/blizzard_nvs.h"
#include "lib/blizzard_wdmx.h"
#include "lib/dmx.h"
#include "lib/blizzard_wifi.h"
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).

static const char *TAG = "BLYNK";

char auth[] = "48ad103165ae48418a7bda0c65837c25";
uint8_t gen_buf[64];

//Optimizations
#define BLYNK_NO_BUILTIN
#define BLYNK_NO_FLOAT

/* Widgets */
WidgetTerminal terminal(V12);
BlynkTimer timer;

uint8_t row_index;

uint8_t octet = 0;
uint8_t ip[4];


void blizzard_timer_event()
{
  update_blynk_wdmx_led();
}

void update_blynk_wdmx_led()
{
  Blynk.virtualWrite(V16, 255);
  switch(get_wdmx_color())
  {
    case WDMX_COLOR_RED:
      Blynk.setProperty(V16, "color", "#FF0000");
    break;
    case WDMX_COLOR_GREEN:
      Blynk.setProperty(V16, "color", "#00FF00");
    break;
    case WDMX_COLOR_BLUE:
      Blynk.setProperty(V16, "color", "#0000FF");
    break;
    case WDMX_COLOR_YELLOW:
      Blynk.setProperty(V16, "color", "#FFFF00");
    break;
    case WDMX_COLOR_CYAN:
      Blynk.setProperty(V16, "color", "#FF00FF");
    break;
    case WDMX_COLOR_MAGENTA:
      Blynk.setProperty(V16, "color", "#00FFFF");
    break;
    case WDMX_COLOR_WHITE:
      Blynk.setProperty(V16, "color", "#FFFFFF");
    break;
    case WDMX_COLOR_OFF:
      Blynk.setProperty(V16, "color", "#000000");
    break;
  }
}

void start_blynk()
{
  uint32_t tick_count = 0;
  initArduino();

  memset(ip, 0, 4);

  delay(3000);

  //Blynk.begin(auth, ssid, pass);
  Blynk.config(auth);
  Blynk.connect();

  /* Talk to Terminal */
  terminal.print(String(getName()));
  terminal.println(" has connected!");
  terminal.flush();

  /* Init Table */
  Blynk.virtualWrite(V11, "clr");
  Blynk.virtualWrite(V11, "add", 0, "Device Name", String(getName()));

  /* Init Timer */
  timer.setInterval(BLYNK_TIMER_INTERVAL, blizzard_timer_event);

  while(1)
  {
    Blynk.run();
    //timer.run();

    //update led if wdmx is on
    /*if(tick_count++ > 5)
    {
      tick_count = 0;
      update_blynk_wdmx_led();
    }*/


    delay(50); //freertos context switch

  }
}

BLYNK_CONNECTED()
{
  //Blynk.syncAll();
  //Blynk.syncVirtual(V9, V10, V0, V1, V2, V3, V4, V5, V6, V7, V8);
  //Blynk.virtualWrite(V16, 0); //turn off LED
}

BLYNK_WRITE(V17)
{
  int val = param.asInt();

  switch (val)
  {
    case HIGH:
      setNeedWifiManager(ENABLE);
    break;
  }

}


//Input
BLYNK_WRITE(V9)
{
  switch(param.asInt())
  {
    case 1:
      setInputMode(DMX_MODE);
    break;
    case 2:
      //set input mode as Artnet
      setInputMode(ARTNET_MODE);
    break;
    case 3:
      //set input mode as SACN
      setInputMode(SACN_MODE);
    break;
    case 4:
      //set input mode as SACN
      setInputMode(WDMX_MODE);
    break;
  }
}

//Output
BLYNK_WRITE(V10)
{
  switch(param.asInt())
  {
    case 1:
      setOutputMode(DMX_MODE);
    break;
    case 2:
      //set input mode as Artnet
      setOutputMode(ARTNET_MODE);
    break;
    case 3:
      //set input mode as SACN
      setOutputMode(SACN_MODE);
    break;
    case 4:
      //set input mode as SACN
      setOutputMode(WDMX_MODE);
    break;
  }
}

BLYNK_WRITE(V14)
{
  int val = param.asInt();
  if(isWDMXBusy())
  {
    ESP_LOGI(TAG, "BUSY");
    return;
  }
  ESP_LOGI(TAG, "TOGGLE POWER %d %d", val, isWDMXBusy());

  switch (val)
  {
    case HIGH:
      turn_on_wdmx();
      Blynk.virtualWrite(V16, 255);
      ESP_LOGI(TAG, "ON");

    break;
    case LOW:
      turn_off_wdmx();
      //Blynk.virtualWrite(V16, 0);
      ESP_LOGI(TAG, "OFF");
    break;
  }
}

BLYNK_WRITE(V13)
{
  int ret_val;
  if(isWDMXBusy())
  {
    ESP_LOGI(TAG, "BUSY");
    return;
  }

  if(param.asInt())
  {

    ret_val = easy_wdmx_connect();
    Blynk.notify((ret_val) ? "Easy Connect(tm) Error. Wah Wah" : "Hooray! You set the thing to the thing!");
    if(ret_val)
    {
      switch_wdmx_button_off();
      Blynk.virtualWrite(V14, 0);
    }
    else
    {
      Blynk.virtualWrite(V14, 1);
    }

  }

}

BLYNK_WRITE(V15)
{
  int val = param.asInt();
  if(isWDMXBusy())
  {
    ESP_LOGI(TAG, "BUSY");
    return;
  }
  ESP_LOGI(TAG, "BUTTON %d", val);
  switch (val)
  {
    case HIGH:
      switch_wdmx_button_on();
    break;
    case LOW:
      switch_wdmx_button_off();
    break;
  }
}

BLYNK_WRITE(V12)
{
  uint8_t i = 0,j = 0, delim_count = 0, delim_indexes[3], ip_index[3];
  String val = param.asStr();

  for(i = 0; i < val.length(); i++)
  {
    if(val[i] == ' ')
    {
      gen_buf[i] = '\0'; //null terminate
      delim_indexes[delim_count++] = i + 1;
    }
    else
    {
      gen_buf[i] = val[i];
    }


  }
  //zero out the rest of the buffer
  for(; i < 64; i++)
    gen_buf[i] = 0;

  if(String("help") == val)
  {
    //terminal.println("There is no help");
    terminal.println("setname <name>");
    terminal.println("setip <ip>");
    terminal.println("setdhcp <en/dis>");
    terminal.println("clear");
  }
  else if(String("setname") == String((char*)(gen_buf)))
  {
    if(delim_count == 0)
    {
      terminal.println("usage: setname <name>");
    }
    else
    {
      terminal.print("name changed to ");
      terminal.println(String((char*)(&gen_buf[delim_indexes[0]])));
      update_str_nvs_val(NVS_DEVICE_NAME_KEY,((char*)(&gen_buf[delim_indexes[0]])));
    }
  }
  else if(String("setip") == String((char*)(gen_buf)))
  {
    if(delim_count == 0)
    {
      terminal.println("usage: setip <255.255.255.255>");
    }
    else
    {

      for(i = 0; i < 15; i++)
      {
        if(gen_buf[delim_indexes[0] + i] == '.')
        {
          gen_buf[delim_indexes[0] + i] = '\0'; //null terminate
          ip_index[j++] = i + 1;
        }
        else if(gen_buf[delim_indexes[0] + i] == '\0')
        {
          break;
        }
      }

      if(j != 3){
        terminal.println("incorrect ip");
        terminal.println("usage: setip <255.255.255.255>");
        return;
      }

      ip[3] = (uint8_t) String((char*)(&gen_buf[delim_indexes[0]])).toInt();
      ip[2] = (uint8_t) String((char*)(&gen_buf[delim_indexes[0] + ip_index[0]])).toInt();
      ip[1] = (uint8_t) String((char*)(&gen_buf[delim_indexes[0] + ip_index[1]])).toInt();
      ip[0] = (uint8_t) String((char*)(&gen_buf[delim_indexes[0] + ip_index[2]])).toInt();
      //terminal.println(String(ip[3] + "." ip[2] + "." + ip[1] + "." ip[0]));


      terminal.print("ip set to ");
      terminal.print(ip[3], DEC);
      terminal.print(".");
      terminal.print(ip[2], DEC);
      terminal.print(".");
      terminal.print(ip[1], DEC);
      terminal.print(".");
      terminal.println(ip[0], DEC);

      changeIP(ip);
      setDHCPEnable(DISABLE);
    }
  }
  else if(String("setdhcp") == String((char*)(gen_buf)))
  {
    if(delim_count == 0)
    {
      terminal.println("usage:setdhcp <en/dis>");
    }
    else
    {
      if(String("en") == String((char*)(&gen_buf[delim_indexes[0]])))
      {
        setDHCPEnable(ENABLE);
        terminal.println("dhcp enabled");
      }
      else if(String("dis") == String((char*)(&gen_buf[delim_indexes[0]])))
      {
        setDHCPEnable(DISABLE);
        terminal.println("dhcp disabled");
      }
      else
      {
        terminal.println("usage:setdhcp <en/dis>");
      }
    }
  }
  else if(String("clear") == String((char*)(gen_buf)))
  {
    for(i = 0; i < 25; i++) //25 lines is all blynk stores
      terminal.println();
  }
  else
  {
    terminal.println("Not a valid command, try 'help'");
  }
  terminal.flush();
}


BLYNK_WRITE(V0)
{
  uint8_t val = (uint8_t) param.asInt();
  setDMXData(1 + (8 * octet), val);

}

BLYNK_WRITE(V1)
{
  uint8_t val = (uint8_t) param.asInt();
  setDMXData(2 + (8 * octet), val);

}

BLYNK_WRITE(V2)
{
  uint8_t val = (uint8_t) param.asInt();
  setDMXData(3 + (8 * octet), val);

}

BLYNK_WRITE(V3)
{
  uint8_t val = (uint8_t) param.asInt();
  setDMXData(4 + (8 * octet), val);

}

BLYNK_WRITE(V4)
{
  uint8_t val = (uint8_t) param.asInt();
  setDMXData(5 + (8 * octet), val);

}

BLYNK_WRITE(V5)
{
  uint8_t val = (uint8_t) param.asInt();
  setDMXData(6 + (8 * octet), val);

}

BLYNK_WRITE(V6)
{
  uint8_t val = (uint8_t) param.asInt();
  setDMXData(7 + (8 * octet), val);

}

BLYNK_WRITE(V7)
{
  uint8_t val = (uint8_t) param.asInt();
  setDMXData(8 + (8 * octet), val);

}

BLYNK_WRITE(V8)
{
  octet = (uint8_t) param.asInt();
}
