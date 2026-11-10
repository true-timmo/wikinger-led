#include <Arduino.h>
#include <esp_bt.h>
#include <EEPROM.h>
#include <map>

// Pins vor den eigenen Headern definieren (die haben eigene Defaults)
#if CONFIG_IDF_TARGET_ESP32C3
// ESP32-C3 Super Mini
// Nicht verwenden: GPIO 2, 8, 9 (Strapping, 8 = Onboard-LED, 9 = BOOT-Taste),
// GPIO 18/19 (USB), GPIO 11-17 (Flash). ADC nur auf GPIO 0-4 (ADC1).
#define CLK_PIN       GPIO_NUM_7
#define SI_PIN        GPIO_NUM_6
#define SENSOR_ANALOG GPIO_NUM_3
#define POWER_KEY_PIN GPIO_NUM_10
#define BATTERY_PIN   GPIO_NUM_4   // Spannungsteiler 1:1 (2x 1 MOhm + 100 nF)
#else
// ESP32 DevKit V4
#define CLK_PIN       GPIO_NUM_14
#define SI_PIN        GPIO_NUM_12
#define SENSOR_ANALOG GPIO_NUM_33
#define POWER_KEY_PIN GPIO_NUM_27
#define BATTERY_PIN   -1           // keine Akkumessung
#endif

#include "soc/rtc.h"
#include "SunSensor.h"
#include "LimitedDarknessHandler.h"
#include "RGBRedWave.h"
#include "TimedWifiAccessPoint.h"
#include "ArduinoOTAHandler.h"
#include "PowerManager.h"

#ifndef APSSID
#define APSSID "Wikinger"
#define APPSK  "sbu421974"
#endif


#define NUM_LEDS    43
#define BRIGHTNESS  80

//TimedWifiAccessPoint ap(APSSID, APPSK, 120000UL); // 2 Minuten

Threshold threshold("threshold", 200, 255);
SunSensor sensor(SENSOR_ANALOG, "sensor", &threshold, 7);
LimitedDarknessHandler darknessHandler(1024 * 60 * 60 * 6);
RGBRedWave<SI_PIN, CLK_PIN> ledStrip("ledStrip", NUM_LEDS);
PowerManager power(POWER_KEY_PIN, BATTERY_PIN);
//ArduinoOTAHandler ota("otaHandler", &ledStrip);

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(32);
  btStop();

  power.begin();
  power.releasePinOnSleep(SI_PIN);
  power.releasePinOnSleep(CLK_PIN);

  delay(100);

  const PowerManager::BatteryState battery = power.checkBattery();
  if (battery != PowerManager::BatteryState::Ok) {
    Serial.println("Akku schwach, LEDs bleiben aus...");
    power.sleep(battery);
  }

  if (sensor.read() == false) {
    Serial.println("Noch hell, gehe schlafen...");
    power.setNightOver(false);
    power.sleep();
  }

  if (power.isNightOver()) {
    Serial.println("Maximale Leuchtdauer erreicht, schlafe bis morgen...");
    power.sleep();
  }

  Serial.println("Dunkel, schalte LED-Versorgung ein");
  power.pressPowerKey();

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
    if (power.isBatteryCheckDue()) {
      const PowerManager::BatteryState battery = power.checkBattery();
      if (battery != PowerManager::BatteryState::Ok) {
        Serial.println("Akku schwach, LEDs aus");
        ledStrip.switchOff();
        power.sleep(battery);
      }
    }

    const bool isDark = sensor.read();
    const bool lightsOn = darknessHandler.handleDarkness(isDark);

    if (isDark == false) {
      Serial.println("Hell geworden, gehe schlafen...");
      power.setNightOver(false);
      power.sleep();
    }

    if (lightsOn == false) {
      Serial.println("Maximale Leuchtdauer erreicht, schlafe bis morgen...");
      power.setNightOver(true);
      power.sleep();
    }
  //}

  delay(100);
}
