#ifndef CLK_PIN
#define CLK_PIN     GPIO_NUM_14
#define SI_PIN      GPIO_NUM_12
#endif

#ifndef RGB_Red_Wave_h_
#define RGB_Red_Wave_h_

#include <Arduino.h>
#include <vector>
#include "FastLED.h"

template <uint8_t SI_PIN, uint8_t CLK_PIN>
class RGBRedWave: public Led, public Dimmable
{    
    private:
        static constexpr auto LED_TYPE    = WS2801;
        static constexpr auto COLOR_ORDER = RBG;
        static constexpr uint8_t SPARKING = 2;
        static constexpr uint8_t SPACING = 12;
        static constexpr uint8_t SPEED = 8;
        static constexpr uint8_t FRAME_MS = 20; // ms pro Frame

        uint32_t lastMillis = 0;
        uint8_t phase = 0;
        unsigned int brightness = 60;
        unsigned int num_leds = 4;
        std::vector<CRGB> leds;


    public:
        void dim(int value) override;
        unsigned int getLevel() override;
        void setLevel(unsigned int level);
        void switchOn() override;
        void switchOff() override;
        bool isOn() override;

        RGBRedWave(const char* name, int num_leds): Led(SI_PIN), Dimmable(name) {
            this->num_leds = num_leds;
            this->leds.resize(num_leds);

            FastLED.addLeds<LED_TYPE, SI_PIN, CLK_PIN, COLOR_ORDER>(this->leds.data(), num_leds)
                .setCorrection(UncorrectedColor)
                .setDither(this->brightness < 255);
            FastLED.setBrightness(this->brightness);
        };

        virtual ~RGBRedWave() {};
};


// ---- Methods ---- //
template <uint8_t SI_PIN, uint8_t CLK_PIN>
void RGBRedWave<SI_PIN, CLK_PIN>::setLevel(unsigned int level){
    this->brightness = level;
};

template <uint8_t SI_PIN, uint8_t CLK_PIN>
unsigned int RGBRedWave<SI_PIN, CLK_PIN>::getLevel(){
    return this->brightness;
};

template <uint8_t SI_PIN, uint8_t CLK_PIN>
void RGBRedWave<SI_PIN, CLK_PIN>::dim(int value){
    this->setLevel(value);

    FastLED.setDither(this->brightness < 255);
    FastLED.setBrightness(this->brightness);
};

template <uint8_t SI_PIN, uint8_t CLK_PIN>
void RGBRedWave<SI_PIN, CLK_PIN>::switchOn() {
    uint32_t now = millis();
    if (now - this->lastMillis < FRAME_MS) return;
    this->lastMillis = now;
    this->phase += SPEED;

    fadeToBlackBy(this->leds.data(), this->leds.size(), constrain(this->num_leds, 4, 10));

    for (int i = 0; i < (int)this->leds.size(); i++) {
        uint8_t wave = sin8(i * SPACING + this->phase);
        uint8_t _brightness = map(wave, 0, 255, 0, this->brightness);
        CRGB col = CRGB(_brightness, 0, 0);
        this->leds[i] = blend(this->leds[i], col, 192);

        if (random16() < SPARKING * 255) {
            this->leds[i] = CRGB::Yellow;
        }
    }
    FastLED.show();
}

template <uint8_t SI_PIN, uint8_t CLK_PIN>
void RGBRedWave<SI_PIN, CLK_PIN>::switchOff() {
    for (auto &c : this->leds) c = CRGB::Black;
    if (!this->leds.empty()) this->leds[0] = CRGB::Black;

    FastLED.show();
    delay(1);
}

template <uint8_t SI_PIN, uint8_t CLK_PIN>
bool RGBRedWave<SI_PIN, CLK_PIN>::isOn() {
    for (int i = 0; i < this->leds.size(); i++) {
        if (this->leds[i] != CRGB::Black) {
            return true;
        }
    }
    return false;
}

#endif  // RGB_Red_Wave_h