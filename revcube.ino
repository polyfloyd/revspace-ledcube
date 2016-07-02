#define NUM_TLCS 4
#include "Tlc5940.h"

#define NUM_BUFFERS 2

#define MAX(a, b) (a > b ? a : b)
#define PORT_HI(port, bit) (port |= bit)
#define PORT_LO(port, bit) (port &= ~bit)

#include "frame.h"

volatile void refreshCallback();
void draw(void (*func)(uint8_t*));

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

// This table maps the frame's grayscale values (0 <= v < 4096) to the value
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
    draw([](uint8_t *frame) {
        setAll(frame, sin(millis() / 300.0) * 127 + 127);
    });
}

// This function can be used to draw using a double buffer and automaticaly
// swap to it. The specified function should asume that not all values in the
// argument buffer are set to 0.
void draw(void (*func)(uint8_t*)) {
    // Cycle to the next buffer and get a pointer to the beginning of it.
    currentBufferIndex = (currentBufferIndex + 1) % NUM_BUFFERS;
    volatile uint8_t *buf = &buffer[currentBufferIndex * 512];
    // Apply the draw function.
    func((uint8_t*)buf);
    // Make the frame available to the refresh callback.
    newFrame = buf;
}

// This function is run every time a TLC grayscale cycle has been completed.
volatile void refreshCallback() {
    volatile static uint8_t shownZ = 0;
    volatile static uint8_t *shownFrame = &buffer[currentBufferIndex * 512];

    // Turn off the previous level.
    PORT_LO(*zPins[shownZ].port, zPins[shownZ].bit);
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
    PORT_HI(*zPins[shownZ].port, zPins[shownZ].bit);
}
