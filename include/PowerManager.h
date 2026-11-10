#ifndef PowerManager_h_
#define PowerManager_h_

#include <Arduino.h>
#include <vector>
#include <esp_sleep.h>

// Überleben den Deep Sleep (RTC-Speicher)
static RTC_DATA_ATTR bool rtcBatteryLow = false; // Akku war leer, muss erst wieder laden
static RTC_DATA_ATTR bool rtcNightOver  = false; // Maximale Leuchtdauer dieser Nacht erreicht

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

    int keyPin;
    int batteryPin;
    float batteryDivider;
    unsigned long lastCheck = 0;
    std::vector<int> releasePins;

public:
    // batteryPin < 0: keine Akkumessung (Akku gilt immer als voll)
    // batteryDivider: Teilerverhältnis, bei Bedarf gegen Multimeter kalibrieren
    PowerManager(int keyPin, int batteryPin = -1, float batteryDivider = 2.0f)
    {
        this->keyPin = keyPin;
        this->batteryPin = batteryPin;
        this->batteryDivider = batteryDivider;
    }

    void begin()
    {
        pinMode(this->keyPin, OUTPUT_OPEN_DRAIN);
        digitalWrite(this->keyPin, HIGH);
    }

    // Pin wird vor dem Deep Sleep auf INPUT gesetzt, damit z. B. die
    // stromlose LED-Kette nicht über die Datenleitungen versorgt wird.
    void releasePinOnSleep(int pin)
    {
        this->releasePins.push_back(pin);
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
        digitalWrite(this->keyPin, LOW);
        delay(200);
        digitalWrite(this->keyPin, HIGH);
        delay(100); // Step-up hochfahren lassen
    }

    // Schläft passend zum Akkustand: kurz, oder lange wenn der Akku fast leer ist.
    void sleep(BatteryState state = BatteryState::Ok)
    {
        this->sleepFor(state == BatteryState::Critical ? SLEEP_EMPTY_US : SLEEP_US);
    }

    void sleepFor(uint64_t timeoutUs)
    {
        for (int pin : this->releasePins) {
            pinMode(pin, INPUT);
        }

        Serial.flush();
        esp_sleep_enable_timer_wakeup(timeoutUs);
        esp_deep_sleep_start();
    }
};

#endif  // PowerManager_h_
