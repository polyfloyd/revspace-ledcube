#define NUM_BUFFERS 2
#define DRAW_INTERVAL 20

#define MAX(a, b) (a > b ? a : b)

#include "animations.h"
#include "frame.h"
// You should include https://github.com/PaulStoffregen/Tlc5940 in your Arduino
// libraries and set NUM_TLCS to 4 in tlc_config.h.
#include "Tlc5940.h"

volatile void refreshCallback();
void draw(void (*func)(uint8_t*));
void drawFor(unsigned long duratonMillis, void (*func)(uint8_t*));

// This list contains the pin mapping for the Z-axis. Ports are rawly
// manipulated for more speed!
struct {
    uint8_t bit;
    volatile uint8_t *ctl;
    volatile uint8_t *port;
} zPins[] = {
    { 1<<2,  &DDRD, &PORTD }, // Pin 2
    { 1<<4,  &DDRD, &PORTD }, // Pin 4
    { 1<<5,  &DDRD, &PORTD }, // Pin 5
    { 1<<6,  &DDRD, &PORTD }, // Pin 6
    { 1<<7,  &DDRD, &PORTD }, // Pin 7
    { 1<<0,  &DDRB, &PORTB }, // Pin 8
    { 1<<2,  &DDRC, &PORTC }, // Pin A2
    { 1<<1,  &DDRC, &PORTC }, // Pin A1
};

// This table maps the frame's grayscale values (0 <= v < 256) to the value
// that the TLCs (0 <= v < 4096). An inversion is applied to match the pullup
// resistors and a logarithm is used to make the frame grayscale more linear in
// light output.
static uint16_t grayscaleTable[255];

volatile static uint8_t currentBufferIndex = 0;
volatile static uint8_t buffer[512 * NUM_BUFFERS] = { 0 };
volatile static uint8_t *newFrame = 0;

void setup() {
    Tlc.init();
    tlc_onUpdateFinished = refreshCallback;
    for (int z = 0; z < 8; z++) {
        *zPins[z].ctl |= zPins[z].bit;
    }

    for (uint16_t i = 0; i < 256; i++) {
        grayscaleTable[255 - i] = uint16_t(MAX((log(i / 255.0) + 4.0) * 1024.0 - 1, 0));
    }
}

void loop() {
    drawFor(2000, animSmoothFade);
    drawFor(2000, animSteps);
    drawFor(2000, animWave);
}

// This function can be used to draw using a double buffer and automaticaly
// swap to it. The specified function should asume that not all values in the
// argument buffer are set to 0.
void draw(void (*func)(uint8_t*)) {
    // Cycle to the next buffer and get a pointer to the beginning of it.
    currentBufferIndex = (currentBufferIndex + 1) % NUM_BUFFERS;
    volatile uint8_t *buf = &buffer[currentBufferIndex * 512];

    unsigned long begin = millis();
    // Apply the draw function.
    func((uint8_t*)buf);
    // Make the frame available to the refresh callback.
    newFrame = buf;
    // Give the display time to swap. The animation looks nicer that way.
    unsigned long end = millis();
    if (end - begin < DRAW_INTERVAL) {
        delay(DRAW_INTERVAL - (end - begin));
    }
}

// Repeatedly draws using the specified function for the specified duration.
void drawFor(unsigned long duratonMillis, void (*func)(uint8_t*)) {
    unsigned long until = millis() + duratonMillis;
    while (millis() <= until) {
        draw(func);
    }
}

// This function is run every time a TLC grayscale cycle has been completed.
volatile void refreshCallback() {
    volatile static uint8_t shownZ = 0;
    volatile static uint8_t *shownFrame = &buffer[currentBufferIndex * 512];

    // Turn off the previous level.
    *zPins[shownZ].port &= ~zPins[shownZ].bit;
    // Cycle to the next level.
    shownZ = (shownZ + 1) % 8;
    // If a frame has been completed and a new frame is available, set it as
    // the shown frame.
    if (!shownZ && newFrame) {
        shownFrame = newFrame;
        newFrame = 0;
    }
    // Write the current level data to the TLCs.
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Tlc.set(y * 8 + x, grayscaleTable[shownFrame[FR_IDX(x, y, shownZ)]]);
        }
    }
    Tlc.update();
    // Turn on the level to display the current level.
    *zPins[shownZ].port |= zPins[shownZ].bit;
}
