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
#endif


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



#define NUM_LEDS    43
#define BRIGHTNESS  80
#define SLEEP_TIMEOUT       (1000ULL * 1000ULL * 20)      // 20 Sekunden
#define SLEEP_TIMEOUT_EMPTY (1000ULL * 1000ULL * 60 * 30) // 30 Minuten bei leerem Akku

// Akkugrenzen in mV (Spannung am Akku, nicht am Pin)
#define BATTERY_DIVIDER       2.0f  // bei Bedarf gegen Multimeter kalibrieren
#define BATTERY_CRITICAL_MV   3100  // darunter nur noch lange schlafen
#define BATTERY_LOW_MV        3300  // darunter LEDs aus
#define BATTERY_RECOVERED_MV  3600  // LEDs erst wieder ab hier (Hysterese)
#define BATTERY_CHECK_MS      10000

//TimedWifiAccessPoint ap(APSSID, APPSK, 120000UL); // 2 Minuten

Threshold threshold("threshold", 200, 255);
SunSensor sensor(SENSOR_ANALOG, "sensor", &threshold, 7);
LimitedDarknessHandler darknessHandler(1024 * 60 * 60 * 6);
RGBRedWave<SI_PIN, CLK_PIN> ledStrip("ledStrip", NUM_LEDS);
//ArduinoOTAHandler ota("otaHandler", &ledStrip);

// Überlebt den Deep Sleep: Akku war leer und muss erst wieder geladen werden
RTC_DATA_ATTR bool batteryLow = false;
unsigned long lastBatteryCheck = 0;

uint32_t readBatteryMillivolts()
{
#ifdef BATTERY_PIN
  uint32_t sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogReadMilliVolts(BATTERY_PIN);
  }
  return (uint32_t)((sum / 16) * BATTERY_DIVIDER);
#else
  return 4200; // keine Messung auf der alten Hardware
#endif
}

// Prüft den Akku. Liefert false, wenn die LEDs nicht leuchten dürfen.
bool checkBattery(uint32_t mv)
{
  if (mv < BATTERY_LOW_MV) {
    batteryLow = true;
  } else if (mv >= BATTERY_RECOVERED_MV) {
    batteryLow = false;
  }
  return !batteryLow;
}

// Kurzer Tastendruck am CD42: schaltet den 5V-Ausgang ein.
// Achtung: zwei Drücke innerhalb ~1 s schalten ihn wieder aus.
void pressPowerKey()
{
  digitalWrite(POWER_KEY_PIN, LOW);
  delay(200);
  digitalWrite(POWER_KEY_PIN, HIGH);
  delay(100); // Step-up hochfahren lassen
}

void goToSleep(uint64_t timeout)
{
  // Leitungen zur (gleich stromlosen) LED-Kette nicht mehr treiben
  pinMode(SI_PIN, INPUT);
  pinMode(CLK_PIN, INPUT);

  Serial.flush();
  esp_sleep_enable_timer_wakeup(timeout);
  esp_deep_sleep_start();
}

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(32);
  btStop();

  pinMode(POWER_KEY_PIN, OUTPUT_OPEN_DRAIN);
  digitalWrite(POWER_KEY_PIN, HIGH);

  delay(100);

  const uint32_t batteryMv = readBatteryMillivolts();
  Serial.printf("Akku: %u mV\n", batteryMv);

  if (batteryMv < BATTERY_CRITICAL_MV) {
    Serial.println("Akku fast leer, schlafe lange...");
    batteryLow = true;
    goToSleep(SLEEP_TIMEOUT_EMPTY);
  }

  if (!checkBattery(batteryMv)) {
    Serial.println("Akku schwach, LEDs bleiben aus...");
    goToSleep(SLEEP_TIMEOUT);
  }

  if (sensor.read() == false) {
    Serial.println("Noch hell, gehe schlafen...");
    goToSleep(SLEEP_TIMEOUT);
  }

  Serial.println("Dunkel, schalte LED-Versorgung ein");
  pressPowerKey();

  ledStrip.dim(BRIGHTNESS);
  darknessHandler.addLed(&ledStrip);
  lastBatteryCheck = millis();
}

void loop()
{
  //const bool isApActive = ap.update();

  //if (isApActive == true && ap.getClientCount() > 0) {
  //  ota.setLevel(1);
  //  ota.handle();
  //} else {
    if (millis() - lastBatteryCheck >= BATTERY_CHECK_MS) {
      lastBatteryCheck = millis();
      const uint32_t batteryMv = readBatteryMillivolts();

      if (!checkBattery(batteryMv)) {
        Serial.printf("Akku schwach (%u mV), LEDs aus\n", batteryMv);
        ledStrip.switchOff();
        goToSleep(SLEEP_TIMEOUT);
      }
    }

    bool isDark = sensor.read();
    darknessHandler.handleDarkness(isDark);

    if (isDark == false) {
      Serial.println("Hell geworden, gehe schlafen...");
      goToSleep(SLEEP_TIMEOUT);
    }
  //}

  delay(100);
}
