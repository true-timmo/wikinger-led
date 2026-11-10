#ifndef PowerManager_h_
#define PowerManager_h_

#include <Arduino.h>
#include <vector>
#include <esp_sleep.h>
#include <driver/gpio.h>

// Überleben den Deep Sleep (RTC-Speicher)
static RTC_DATA_ATTR bool rtcBatteryLow = false; // Akku war leer, muss erst wieder laden
static RTC_DATA_ATTR bool rtcNightOver  = false; // Maximale Leuchtdauer dieser Nacht erreicht
static RTC_DATA_ATTR uint32_t rtcWakeCount = 0;  // Anzahl Aufwachvorgänge seit Power-On

// Kümmert sich um Akkuüberwachung, den KEY-Taster des MH-CD42 und den Deep Sleep.
class PowerManager
{
public:
    enum class BatteryState { Ok, Low, Critical };

private:
    // Grenzen in mV (Spannung am Akku, nicht am Pin)
    static constexpr uint32_t CRITICAL_MV  = 3100; // darunter nur noch lange schlafen
    static constexpr uint32_t LOW_MV       = 3300; // darunter LEDs aus
    static constexpr uint32_t RECOVERED_MV = 3600; // LEDs erst wieder ab hier (Hysterese)
    static constexpr unsigned long CHECK_INTERVAL_MS = 10000;
    static constexpr uint64_t SLEEP_US       = 1000ULL * 1000ULL * 20;      // 20 Sekunden
    static constexpr uint64_t SLEEP_EMPTY_US = 1000ULL * 1000ULL * 60 * 30; // 30 Minuten
    static constexpr unsigned long MAINTENANCE_MS = 10000; // Wachzeit nach Reset für Uploads

    int keyPin;
    int batteryPin;
    float batteryDivider;
    unsigned long lastCheck = 0;
    std::vector<int> sleepPulldownPins;
    int statusLedPin = -1;

    // Onboard-LED des Super Mini ist low-aktiv
    void blinkStatus(int count, int onMs = 60, int offMs = 120)
    {
        if (this->statusLedPin < 0) {
            return;
        }

        pinMode(this->statusLedPin, OUTPUT);
        for (int i = 0; i < count; i++) {
            digitalWrite(this->statusLedPin, LOW);
            delay(onMs);
            digitalWrite(this->statusLedPin, HIGH);
            delay(offMs);
        }
    }

public:
    // batteryPin < 0: keine Akkumessung (Akku gilt immer als voll)
    // batteryDivider: Teilerverhältnis, bei Bedarf gegen Multimeter kalibrieren
    PowerManager(int keyPin, int batteryPin = -1, float batteryDivider = 2.0f)
    {
        this->keyPin = keyPin;
        this->batteryPin = batteryPin;
        this->batteryDivider = batteryDivider;
    }

    // Status-LED: blinkt 1x beim Aufwachen/Start, vor dem Schlafen
    // 2x (kurzer Schlaf) bzw. 3x lang (Akku fast leer, 30 min Schlaf).
    void setStatusLed(int pin)
    {
        this->statusLedPin = pin;
    }

    void begin()
    {
        this->blinkStatus(1, 150);

        // Evtl. noch aktives Pin-Halten einer früheren Firmware aufheben
        gpio_deep_sleep_hold_dis();
        gpio_hold_dis((gpio_num_t)this->keyPin);

        pinMode(this->keyPin, OUTPUT_OPEN_DRAIN);
        digitalWrite(this->keyPin, HIGH);

        const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
        if (cause == ESP_SLEEP_WAKEUP_TIMER) {
            rtcWakeCount++;
            Serial.printf("[Power] Aufgewacht (Timer), #%u\n", rtcWakeCount);
        } else {
            rtcWakeCount = 0;
            Serial.printf("[Power] Neustart (Grund %d)\n", (int)cause);
        }
    }

    // Pin wird im Deep Sleep als Eingang mit Pull-down geschaltet, damit die
    // stromlose LED-Kette nicht über die Daten-/Taktleitung mitversorgt wird.
    void pullDownOnSleep(int pin)
    {
        this->sleepPulldownPins.push_back(pin);
        gpio_hold_dis((gpio_num_t)pin);
    }

    uint32_t readBatteryMillivolts()
    {
        if (this->batteryPin < 0) {
            return 4200;
        }

        uint32_t sum = 0;
        for (int i = 0; i < 16; i++) {
            sum += analogReadMilliVolts(this->batteryPin);
        }
        return (uint32_t)((sum / 16) * this->batteryDivider);
    }

    BatteryState checkBattery()
    {
        this->lastCheck = millis();
        const uint32_t mv = this->readBatteryMillivolts();

        if (mv < CRITICAL_MV) {
            rtcBatteryLow = true;
            Serial.printf("Akku fast leer: %u mV\n", mv);
            return BatteryState::Critical;
        }

        if (mv < LOW_MV) {
            rtcBatteryLow = true;
        } else if (mv >= RECOVERED_MV) {
            rtcBatteryLow = false;
        }

        Serial.printf("Akku: %u mV%s\n", mv, rtcBatteryLow ? " (schwach)" : "");
        return rtcBatteryLow ? BatteryState::Low : BatteryState::Ok;
    }

    // true, wenn seit der letzten Messung das Prüfintervall abgelaufen ist
    bool isBatteryCheckDue()
    {
        return millis() - this->lastCheck >= CHECK_INTERVAL_MS;
    }

    bool isNightOver()
    {
        return rtcNightOver;
    }

    void setNightOver(bool nightOver)
    {
        rtcNightOver = nightOver;
    }

    // Kurzer Tastendruck am CD42: schaltet den 5V-Ausgang ein.
    // Achtung: zwei Drücke innerhalb ~1 s schalten ihn wieder aus.
    void pressPowerKey()
    {
        Serial.println("[CD42] KEY-Puls -> 5V-Ausgang ein");
        digitalWrite(this->keyPin, LOW);
        delay(200);
        digitalWrite(this->keyPin, HIGH);
        delay(100); // Step-up hochfahren lassen
    }

    // Nur zur Info: Der CD42 schaltet ohne Last selbst ab.
    void notifyLedsOff()
    {
        Serial.println("[CD42] LEDs aus -> 5V-Ausgang schaltet ohne Last nach ca. 30 s ab");
    }

    // Schläft passend zum Akkustand: kurz, oder lange wenn der Akku fast leer ist.
    void sleep(BatteryState state = BatteryState::Ok)
    {
        this->sleepFor(state == BatteryState::Critical ? SLEEP_EMPTY_US : SLEEP_US);
    }

    void sleepFor(uint64_t timeoutUs)
    {
        if (timeoutUs > SLEEP_US) {
            this->blinkStatus(3, 400, 200);
        } else {
            this->blinkStatus(2);
        }

        for (int pin : this->sleepPulldownPins) {
            pinMode(pin, INPUT_PULLDOWN);
        }

        // Nach einem Neustart/Reset einige Sekunden wach bleiben, damit ein
        // Upload möglich ist (im Deep Sleep ist USB weg).
        if (rtcWakeCount == 0 && millis() < MAINTENANCE_MS) {
            Serial.printf("[Power] Wartungsfenster: noch %lu ms wach\n", MAINTENANCE_MS - millis());
            delay(MAINTENANCE_MS - millis());
        }

        Serial.printf("[Power] Deep Sleep fuer %u s\n", (uint32_t)(timeoutUs / 1000000ULL));
#if ARDUINO_USB_CDC_ON_BOOT
        // Kein Serial.flush(): Über USB-CDC wartet es endlos, wenn kein PC
        // die Daten abholt (Akkubetrieb) -> ESP würde nie einschlafen.
        delay(20);
#else
        Serial.flush();
#endif
        esp_sleep_enable_timer_wakeup(timeoutUs);
        esp_deep_sleep_start();
    }
};

#endif  // PowerManager_h_
