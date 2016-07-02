#define NUM_TLCS 4
#include "Tlc5940.h"

#define NUM_BUFFERS 2

#define MAX(a, b) (a > b ? a : b)
#define PORT_HI(port, bit) (port |= bit)
#define PORT_LO(port, bit) (port &= ~bit)

#include "frame.h"

volatile void refreshCallback();
void draw(void (*func)(uint8_t*));

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
    { 1<<4,  &DDRB, &PORTB }, // Pin 12
    { 1<<1,  &DDRC, &PORTC }, // Pin A1
};

static uint16_t grayscaleTable[255];

volatile static uint8_t currentBufferIndex = 0;
volatile static uint8_t buffer[512 * NUM_BUFFERS] = { 0 };
volatile static uint8_t *shownFrame = &buffer[currentBufferIndex * 512];
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
        map3D((uint8_t*)frame, [](uint8_t x, uint8_t y, uint8_t z) {
            return uint8_t((millis() / 10) % 256);
        });
    });
}

void draw(void (*func)(uint8_t*)) {
    currentBufferIndex = (currentBufferIndex + 1) % NUM_BUFFERS;
    volatile uint8_t *buf = &buffer[currentBufferIndex * 512];
    func((uint8_t*)buf);
    newFrame = buf;
}

volatile void refreshCallback() {
    volatile static uint8_t shownZ = 0;

    PORT_LO(*zPins[shownZ].port, zPins[shownZ].bit);
    shownZ = (shownZ + 1) % 8;
    if (!shownZ && newFrame) {
        shownFrame = newFrame;
        newFrame = 0;
    }
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Tlc.set(y * 8 + x, grayscaleTable[shownFrame[FR_IDX(x, y, shownZ)]]);
        }
    }
    Tlc.update();
    PORT_HI(*zPins[shownZ].port, zPins[shownZ].bit);
}
