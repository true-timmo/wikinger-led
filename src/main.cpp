#include <Arduino.h>
#include <ESP32Encoder.h>
#include <esp_bt.h>
#include <EEPROM.h>
#include <map>

#include "FastLED.h"

#include "DimmableLed.h"
#include "Dimmable.h"
#include "SunSensor.h"
#include "LimitedDarknessHandler.h"
#include "TargetSwitcher.h"

#define CLK_PIN     GPIO_NUM_14
#define SI_PIN      GPIO_NUM_12
#define LED_TYPE    WS2801
#define COLOR_ORDER RBG
#define NUM_LEDS    4 //43
#define BRIGHTNESS  255

#define LED_STATUS 26
#define ENCODER_DT GPIO_NUM_35
#define ENCODER_CLK GPIO_NUM_34
#define ENCODER_SWITCH GPIO_NUM_32
#define SENSOR_ANALOG GPIO_NUM_33

CRGB leds[NUM_LEDS];
Led statusLed(LED_STATUS);

Threshold threshold("threshold", 70, 255);
SunSensor sensor(SENSOR_ANALOG, "sensor", &threshold, 7);
LimitedDarknessHandler darknessHandler(1024);

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(32);
  delay(100);

  FastLED.addLeds<LED_TYPE, SI_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS)
  .setCorrection(UncorrectedColor)
  .setDither(BRIGHTNESS < 255);

  FastLED.setBrightness(BRIGHTNESS);



  btStop();
}

void loop()
{
  static uint8_t waveOffset = 0;
  static uint8_t sparking = 150;
  waveOffset += 3;  // Adjust speed

  for (int i = 0; i < NUM_LEDS; i++) {
    int wave = sin8(i * 12 + waveOffset);  // Smooth sine wave motion

    // Generate "fire" colors in a non-linear wave
    uint8_t heatLevel = map(wave, 0, 255, 30, 150);  // Control brightness
    leds[i] = HeatColor(heatLevel);  // Convert to fire colors

    // Add random flicker
    if (random8() < sparking / 5) {
      leds[i] = CRGB::White;  // Occasional white sparks
    }
  }
  FastLED.show();
    
  Serial.println(darknessHandler.handleDarkness(sensor.read()));
  delay(50);
}