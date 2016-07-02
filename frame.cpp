#include "frame.h"
#include <string.h>

void map3D(uint8_t *frame, uint8_t (*func)(uint8_t, uint8_t, uint8_t)) {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            for (int z = 0; z < 8; z++) {
                frame[FR_IDX(x, y, z)] = func(x, y, z);
            }
        }
    }
}

void map2D(uint8_t *frame, uint8_t (*func)(uint8_t, uint8_t)) {
    memset(frame, 0, 512);
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            frame[FR_IDX(x, y, func(x, y))] = 255;
        }
    }
}
