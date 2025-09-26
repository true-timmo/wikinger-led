#ifndef TIMED_WIFI_ACCESS_POINT_H
#define TIMED_WIFI_ACCESS_POINT_H

#include <Arduino.h>
#include <WiFi.h>
#include <functional>

class TimedWifiAccessPoint {
public:
    // Konstruktor
    // ssid: SSID des AP
    // password: Passwort (leer = offener AP)
    // timeoutMs: Timeout in Millisekunden bis Abschalten, wenn kein Client verbunden war (default 120000 ms = 2 min)
    // channel, hidden, max_connections: optionale AP-Parameter
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

    // Startet den AccessPoint (synchron). Nach dem Start läuft der Timeout;
    // wenn innerhalb timeoutMs kein Client verbunden wird, wird der AP deaktiviert.
    bool begin() {
        // Stelle sicher, dass WiFi im AP-Modus ist
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
        // Stoppe AP
        WiFi.softAPdisconnect(true);
        // Schalte WiFi aus (optional)
        WiFi.mode(WIFI_OFF);
        _active = false;
        if (_onStopped) _onStopped();
    }

    // Aufruf in loop(): prüft in Abständen ob timeout erreicht ist und schaltet ab.
    // return true wenn AP noch aktiv, false wenn AP inaktiv
    bool update() {
        if (!_active) return false;

        unsigned long now = millis();
        // Nur alle _checkIntervalMs prüfen (z.B. 1s)
        if (now - _lastCheckMs < _checkIntervalMs) return true;
        _lastCheckMs = now;

        int clients = WiFi.softAPgetStationNum();

        if (clients > 0) {
            // Es ist mindestens 1 Client verbunden -> Timer zurücksetzen
            _lastSeenClientMs = now;
            return true;
        }

        // Kein Client aktuell - prüfe, ob timeout überschritten
        if (now - _lastSeenClientMs >= _timeoutMs) {
            // Timeout! Stoppe AP
            stop();
            return false;
        }

        return true;
    }

    // Falls du das AP später wieder aktivieren willst:
    bool restart() {
        if (_active) return true;
        return begin();
    }

    bool isActive() const { return _active; }

    // Anzahl der aktuell verbundenen Stationen (Clients)
    int getClientCount() const {
        if (!_active) return 0;
        return WiFi.softAPgetStationNum();
    }

    // Setze Checkintervall (ms) -- wie oft geprüft wird (default 1000 ms)
    void setCheckInterval(unsigned long ms) { _checkIntervalMs = ms; }

    // Setze Timeout (ms)
    void setTimeout(unsigned long ms) { _timeoutMs = ms; }

    // Optionaler Callback, wird aufgerufen, wenn AP gestoppt wird (wegen timeout oder stop())
    void setOnStoppedCallback(std::function<void()> cb) { _onStopped = cb; }

    // Hilfsfunktionen für Debug / Info
    String getSsid() const { return _ssid; }
    String getPassword() const { return _password; }
    unsigned long getTimeout() const { return _timeoutMs; }

private:
    String _ssid;
    String _password;
    unsigned long _timeoutMs;
    uint8_t _channel;
    bool _hidden;
    uint8_t _maxConnections;

    bool _active;
    unsigned long _lastSeenClientMs;  // millis() der letzten Sichtung eines Clients (oder AP-Start)
    unsigned long _checkIntervalMs;   // wie oft geprüft wird (ms)
    unsigned long _lastCheckMs;

    std::function<void()> _onStopped;
};

#endif // TIMED_WIFI_ACCESS_POINT_H