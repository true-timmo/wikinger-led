#include <Arduino.h>
#include <ESP32Encoder.h>
#include <esp_bt.h>
#include <EEPROM.h>
#include <map>

#include "SunSensor.h"
#include "LimitedDarknessHandler.h"
#include "RGBFireWave.h"

#define CLK_PIN     GPIO_NUM_14
#define SI_PIN      GPIO_NUM_12

#define NUM_LEDS    4 //43
#define BRIGHTNESS  127

#define LED_STATUS 26
#define ENCODER_DT GPIO_NUM_35
#define ENCODER_CLK GPIO_NUM_34
#define ENCODER_SWITCH GPIO_NUM_32
#define SENSOR_ANALOG GPIO_NUM_33

Led statusLed(LED_STATUS);

Threshold threshold("threshold", 200, 255);
SunSensor sensor(SENSOR_ANALOG, "sensor", &threshold, 7);
LimitedDarknessHandler darknessHandler(1024 * 60 * 60 * 6);
RGBFireWave<SI_PIN, CLK_PIN> ledStrip("ledStrip", NUM_LEDS);

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(32);
  delay(100);

  ledStrip.dim(BRIGHTNESS);
  darknessHandler.addLed(&ledStrip);

  btStop();
}

void loop()
{
  darknessHandler.handleDarkness(sensor.read());

  delay(50);
}