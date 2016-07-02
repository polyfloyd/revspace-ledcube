#include "stdint.h"

#define FR_IDX(x, y, z) (z * 64 + y * 8 + x)

void map3D(uint8_t *frame, uint8_t (*func)(uint8_t, uint8_t, uint8_t));
void map2D(uint8_t *frame, uint8_t (*func)(uint8_t, uint8_t));
void setAll(uint8_t *frame, uint8_t value);
