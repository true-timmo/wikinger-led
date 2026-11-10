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
#define STATUS_LED_PIN GPIO_NUM_8  // blaue Onboard-LED (low-aktiv)
#else
// ESP32 DevKit V4
#define CLK_PIN       GPIO_NUM_14
#define SI_PIN        GPIO_NUM_12
#define SENSOR_ANALOG GPIO_NUM_33
#define POWER_KEY_PIN GPIO_NUM_27
#define BATTERY_PIN   -1           // keine Akkumessung
#define STATUS_LED_PIN -1
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

// Debug-Ausgabe: Nach dem Deep Sleep braucht der PC etwas, bis die
// USB-Konsole wieder verbunden ist. Kostet bei jedem Aufwachen diese Zeit
// an Strom, also für den Dauerbetrieb auf 0 setzen.
#define DEBUG_USB_WAIT_MS   1500
#define SENSOR_LOG_MS       2000

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
#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0); // ohne angeschlossenen PC nicht blockieren
#endif
  delay(DEBUG_USB_WAIT_MS);
  EEPROM.begin(32);
  btStop();

  power.setStatusLed(STATUS_LED_PIN);
  power.begin();
  power.pullDownOnSleep(SI_PIN);
  power.pullDownOnSleep(CLK_PIN);

  delay(100);

  const PowerManager::BatteryState battery = power.checkBattery();
  if (battery != PowerManager::BatteryState::Ok) {
    Serial.println("Akku schwach, LEDs bleiben aus...");
    power.sleep(battery);
  }

  const bool isDark = sensor.read();
  sensor.printStatus();

  if (isDark == false) {
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
        power.notifyLedsOff();
        power.sleep(battery);
      }
    }

    const bool isDark = sensor.read();
    const bool lightsOn = darknessHandler.handleDarkness(isDark);

    static unsigned long lastSensorLog = 0;
    if (millis() - lastSensorLog >= SENSOR_LOG_MS) {
      lastSensorLog = millis();
      sensor.printStatus();
    }

    if (isDark == false) {
      sensor.printStatus();
      Serial.println("Hell geworden, gehe schlafen...");
      power.notifyLedsOff();
      power.setNightOver(false);
      power.sleep();
    }

    if (lightsOn == false) {
      Serial.println("Maximale Leuchtdauer erreicht, schlafe bis morgen...");
      power.notifyLedsOff();
      power.setNightOver(true);
      power.sleep();
    }
  //}

  delay(100);
}
