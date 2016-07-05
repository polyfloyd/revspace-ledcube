#include "animations.h"
#include <math.h>
#include <Arduino.h>
#include "frame.h"

void animSmoothFade(uint8_t *frame) {
    setAll(frame, sin(millis() / 300.0) * 127 + 127);
}

void animSteps(uint8_t *frame) {
    map2D(frame, [](uint8_t x, uint8_t y) {
        return uint8_t((millis() / 100) % 8);
    });
}

void animWave(uint8_t *frame) {
    map2D(frame, [](uint8_t x, uint8_t y) {
        float time = millis() * 0.003;
        float sx = x * 0.2  + time;
        float sy = y * 0.15 + time;
        return uint8_t(sin(sx) * sin(sy) * 6.5);
    });
}
