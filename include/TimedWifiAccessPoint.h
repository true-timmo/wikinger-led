#ifndef TIMED_WIFI_ACCESS_POINT_H
#define TIMED_WIFI_ACCESS_POINT_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <functional>

class TimedWifiAccessPoint {
public:
    TimedWifiAccessPoint(const String& ssid,
                        const String& password = String(),
                        unsigned long timeoutMs = 120000UL,
                        uint8_t channel = 1,
                        bool hidden = false,
                        uint8_t max_connections = 4)
      : _ssid(ssid),
        _password(password),
        _timeoutMs(timeoutMs),
        _channel(channel),
        _hidden(hidden),
        _maxConnections(max_connections),
        _active(false),
        _lastSeenClientMs(0),
        _checkIntervalMs(1000UL),
        _lastCheckMs(0)
    {}

    bool begin() {
        WiFi.mode(WIFI_AP);
        // Start AP: falls password leer -> offener AP
        bool ok;
        if (_password.length() == 0) {
            ok = WiFi.softAP(_ssid.c_str(), nullptr, _channel, _hidden, _maxConnections);
        } else {
            ok = WiFi.softAP(_ssid.c_str(), _password.c_str(), _channel, _hidden, _maxConnections);
        }

        if (!ok) {
            _active = false;
            return false;
        }

        // Initialisieren Zeiten: Startzeit als "letzter Client gesehen" --> zählt als Start der Wartezeit
        _lastSeenClientMs = millis();
        _lastCheckMs = 0;
        _active = true;
        return true;
    }

    // Sofort stoppen und WiFi ausschalten
    void stop() {
        if (!_active) return;

        esp_err_t err = esp_wifi_stop();
        (void)err;

        err = esp_wifi_deinit();
        (void)err;

        WiFi.mode(WIFI_OFF);

        _active = false;
        if (_onStopped) _onStopped();
    }

    // return true wenn AP noch aktiv, false wenn AP inaktiv
    bool update() {
        if (!_active) return false;

        unsigned long now = millis();
        if (now - _lastCheckMs < _checkIntervalMs) return true;
        _lastCheckMs = now;

        int clients = WiFi.softAPgetStationNum();

        if (clients > 0) {
            _lastSeenClientMs = now;
            return true;
        }

        if (now - _lastSeenClientMs >= _timeoutMs) {
            stop();
            return false;
        }

        return true;
    }

    bool restart() {
        if (_active) return true;
        return begin();
    }

    bool isActive() const { return _active; }

    int getClientCount() const {
        if (!_active) return 0;
        return WiFi.softAPgetStationNum();
    }

    void setCheckInterval(unsigned long ms) { _checkIntervalMs = ms; }
    void setTimeout(unsigned long ms) { _timeoutMs = ms; }

private:
    String _ssid;
    String _password;
    unsigned long _timeoutMs;
    uint8_t _channel;
    bool _hidden;
    uint8_t _maxConnections;

    bool _active;
    unsigned long _lastSeenClientMs;
    unsigned long _checkIntervalMs;
    unsigned long _lastCheckMs;

    std::function<void()> _onStopped;
};

#endif // TIMED_WIFI_ACCESS_POINT_H