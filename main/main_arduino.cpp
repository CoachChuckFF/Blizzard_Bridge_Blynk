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
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "lib/main_arduino.h"
#include "lib/dmx.h"
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "48ad103165ae48418a7bda0c65837c25";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "blizznet";
char pass[] = "destroyer";

uint8_t octet = 0;

void ArduinoLoop()
{
  //initArduino();
  Blynk.begin(auth, ssid, pass);
  while(1)
  {
    Blynk.run();
    delay(50); //freertos context switch
  }
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
