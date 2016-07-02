#include "stdint.h"

// Calculates the absolute index in a frame from an XYZ-coordinate.
#define FR_IDX(x, y, z) (z * 64 + y * 8 + x)

// Applies the specified function to each LED in the frame. The function should
// return the respective grayscale value.
void map3D(uint8_t *frame, uint8_t (*func)(uint8_t, uint8_t, uint8_t));

// Applies the specified function to each column. The functon should return the
// coordinate of the LED along the Z-axis to set to full power.
void map2D(uint8_t *frame, uint8_t (*func)(uint8_t, uint8_t));

// Sets all LEDs in the frame to the same value.
void setAll(uint8_t *frame, uint8_t value);
