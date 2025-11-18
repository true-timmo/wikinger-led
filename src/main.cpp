#include <Arduino.h>
#include <esp_bt.h>
#include <EEPROM.h>
#include <map>

#include "soc/rtc.h"
#include "SunSensor.h"
#include "LimitedDarknessHandler.h"
#include "RGBRedWave.h"
#include "TimedWifiAccessPoint.h"
#include "ArduinoOTAHandler.h"

#ifndef APSSID
#define APSSID "Wikinger"
#define APPSK  "sbu421974"
#endif

#define CLK_PIN     GPIO_NUM_14
#define SI_PIN      GPIO_NUM_12

#define NUM_LEDS    43
#define BRIGHTNESS  80
#define SLEEP_TIMEOUT (1000ULL * 1000ULL * 20) // 20 Sekunden

#define ENCODER_DT GPIO_NUM_35
#define ENCODER_CLK GPIO_NUM_34
#define ENCODER_SWITCH GPIO_NUM_32
#define SENSOR_ANALOG GPIO_NUM_33

//TimedWifiAccessPoint ap(APSSID, APPSK, 120000UL); // 2 Minuten

Threshold threshold("threshold", 200, 255);
SunSensor sensor(SENSOR_ANALOG, "sensor", &threshold, 7);
LimitedDarknessHandler darknessHandler(1024 * 60 * 60 * 6);
RGBRedWave<SI_PIN, CLK_PIN> ledStrip("ledStrip", NUM_LEDS);
//ArduinoOTAHandler ota("otaHandler", &ledStrip);

esp_sleep_wakeup_cause_t wakeup_reason;

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(32);
  btStop();

  pinMode(GPIO_NUM_27, OUTPUT_OPEN_DRAIN);
  digitalWrite(GPIO_NUM_27, HIGH);

  delay(100);

  wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER && sensor.read() == false) {
    Serial.println("Immer noch hell, gehe wieder schlafen...");

    ledStrip.switchOff();
    digitalWrite(GPIO_NUM_27, LOW);
    delay(200);
    digitalWrite(GPIO_NUM_27, HIGH);
    esp_sleep_enable_timer_wakeup(SLEEP_TIMEOUT);
    esp_deep_sleep_start();
  } else {
    Serial.println("Normaler Reset oder Power-On");
    //ap.begin();
    //ota.setup();
  }

  ledStrip.dim(BRIGHTNESS);
  darknessHandler.addLed(&ledStrip);
}

void loop()
{
  //const bool isApActive = ap.update();

  //if (isApActive == true && ap.getClientCount() > 0) {
  //  ota.setLevel(1);
  //  ota.handle();
  //} else {
    bool isDark = sensor.read();
    darknessHandler.handleDarkness(isDark);

    if (isDark == false) {
      Serial.println("Noch hell, gehe schlafen...");
      esp_sleep_enable_timer_wakeup(SLEEP_TIMEOUT);
      esp_deep_sleep_start();
    }
  //}

  delay(100);
}