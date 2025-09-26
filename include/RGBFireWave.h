#ifndef CLK_PIN
#define CLK_PIN     GPIO_NUM_14
#define SI_PIN      GPIO_NUM_12
#endif

#ifndef RGB_Fire_Wave_h_
#define RGB_Fire_Wave_h_

#include <Arduino.h>
#include <vector>
#include "FastLED.h"

template <uint8_t SI_PIN, uint8_t CLK_PIN>
class RGBFireWave: public Led, public Dimmable
{    
    private:
        static constexpr auto LED_TYPE    = WS2801;
        static constexpr auto COLOR_ORDER = RBG;
        static constexpr uint8_t SPARKING = 2;
        static constexpr uint8_t SPACING = 12;
        static constexpr uint8_t SPEED = 2;
        static constexpr uint8_t FRAME_MS = 20; // ms pro Frame

        uint32_t lastMillis = 0;
        uint8_t phase = 0;
        unsigned int brightness = 255;
        unsigned int num_leds = 4;
        std::vector<CRGB> leds;


    public:
        void dim(int value) override;
        unsigned int getLevel() override;
        void setLevel(unsigned int level);
        void switchOn() override;
        void switchOff() override;
        bool isOn() override;

        RGBFireWave(const char* name, int num_leds): Led(SI_PIN), Dimmable(name) {
            this->num_leds = num_leds;
            this->leds.resize(num_leds);

            FastLED.addLeds<LED_TYPE, SI_PIN, CLK_PIN, COLOR_ORDER>(this->leds.data(), num_leds)
                .setCorrection(UncorrectedColor)
                .setDither(this->brightness < 255);
            FastLED.setBrightness(this->brightness);
        };

        virtual ~RGBFireWave() {};
};


// ---- Methods ---- //
template <uint8_t SI_PIN, uint8_t CLK_PIN>
void RGBFireWave<SI_PIN, CLK_PIN>::setLevel(unsigned int level){
    this->brightness = level;
};

template <uint8_t SI_PIN, uint8_t CLK_PIN>
unsigned int RGBFireWave<SI_PIN, CLK_PIN>::getLevel(){
    return this->brightness;
};

template <uint8_t SI_PIN, uint8_t CLK_PIN>
void RGBFireWave<SI_PIN, CLK_PIN>::dim(int value){
    this->setLevel(value);

    FastLED.setDither(this->brightness < 255);
    FastLED.setBrightness(this->brightness);
};

template <uint8_t SI_PIN, uint8_t CLK_PIN>
void RGBFireWave<SI_PIN, CLK_PIN>::switchOn() {
    uint32_t now = millis();
    if (now - this->lastMillis < FRAME_MS) return;
    this->lastMillis = now;
    this->phase += SPEED;

    fadeToBlackBy(this->leds.data(), this->leds.size(), 16);

    for (int i = 0; i < (int)this->leds.size(); i++) {
        int wave = sin8(i * SPACING + this->phase);
        uint8_t heatLevel = map(wave, 0, 255, 30, 150);
        CRGB col = HeatColor(heatLevel);

        this->leds[i] = blend(this->leds[i], col, 192);

        if (random16() < SPARKING * 255) {
            this->leds[i] = CRGB::White;
        }
    }
    FastLED.show();
}

template <uint8_t SI_PIN, uint8_t CLK_PIN>
void RGBFireWave<SI_PIN, CLK_PIN>::switchOff() {
    for (auto &c : this->leds) c = CRGB::Black;
    if (!this->leds.empty()) this->leds[0] = CRGB::Black;

    FastLED.show();
    delay(1);
}

template <uint8_t SI_PIN, uint8_t CLK_PIN>
bool RGBFireWave<SI_PIN, CLK_PIN>::isOn() {
    for (int i = 0; i < this->leds.size(); i++) {
        if (this->leds[i] != CRGB::Black) {
            return true;
        }
    }
    return false;
}

#endif  // RGB_Fire_Wave_h_