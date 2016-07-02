#define NUM_TLCS 4
#include "Tlc5940.h"

#define PORT_HI(port, bit) (port |= bit)
#define PORT_LO(port, bit) (port &= ~bit)

#include "frame.h"

volatile void refreshCallback();

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

volatile static uint8_t frame[512];
volatile static uint8_t *shownFrame = frame;

void setup() {
    Tlc.init();
    tlc_onUpdateFinished = refreshCallback;
    for (int z = 0; z < 8; z++) {
        *zPins[z].ctl |= zPins[z].bit;
    }

    for (uint16_t i = 0; i < 256; i++) {
        grayscaleTable[255 - i] = i * 16;
    }
}

void loop() {
    map3D((uint8_t*)frame, [](uint8_t x, uint8_t y, uint8_t z) {
        return uint8_t((millis() / 10) % 256);
    });
}

volatile void refreshCallback() {
    volatile static uint8_t shownZ = 0;

    PORT_LO(*zPins[shownZ].port, zPins[shownZ].bit);
    shownZ = (shownZ + 1) % 8;
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Tlc.set(y * 8 + x, grayscaleTable[shownFrame[FR_IDX(x, y, shownZ)]]);
        }
    }
    Tlc.update();
    PORT_HI(*zPins[shownZ].port, zPins[shownZ].bit);
}
