// https://dhole.github.io/post/gameboy_cartridge_emu_1/

#include <Arduino.h>
#include "rom.h"

#define GB_CLK 2UL
#define GB_WR 3UL
#define GB_RD 4UL
#define GB_CS 5UL
#define GB_A_MIN 6UL
#define GB_A_MAX 21UL
#define GB_D_MIN 22UL
#define GB_D_MAX 29UL

#define P(x) (1UL << x)
#define GB_A (P(GB_A_MIN + 0) | P(GB_A_MIN + 1) | P(GB_A_MIN + 2) | P(GB_A_MIN + 3) | P(GB_A_MIN + 4) | P(GB_A_MIN + 5) | P(GB_A_MIN + 6) | P(GB_A_MIN + 7) | P(GB_A_MIN + 8) | P(GB_A_MIN + 9) | P(GB_A_MIN + 10) | P(GB_A_MIN + 11) | P(GB_A_MIN + 12) | P(GB_A_MIN + 13) | P(GB_A_MIN + 14) | P(GB_A_MIN + 15))
#define GB_D (P(GB_D_MIN + 0) | P(GB_D_MIN + 1) | P(GB_D_MIN + 2) | P(GB_D_MIN + 3) | P(GB_D_MIN + 4) | P(GB_D_MIN + 5) | P(GB_D_MIN + 6) | P(GB_D_MIN + 7))

uint8_t uart_rx_buf[0x100];
uint8_t uart_tx_buf[0x100];
uint8_t uart_n = 0;

uint8_t ram[0x4000]; // 16KB; could have gotten away with just having the above variable in this one... but it's whatever really

void setup() {
    // we don't care about the time taken to set everything up

    Serial.begin(38400); // default baudrate for meshtastic serial

    pinMode(GB_CLK, INPUT_PULLUP);
    pinMode(GB_WR, INPUT_PULLUP);
    pinMode(GB_RD, INPUT_PULLUP);
    pinMode(GB_CS, INPUT_PULLUP);
    for(int p = GB_A_MIN; p <= GB_A_MAX; p++)
        pinMode(p, INPUT_PULLUP);
    for(int p = GB_D_MIN; p <= GB_D_MAX; p++) {
        pinMode(p, INPUT_PULLUP);
        digitalWrite(p, LOW);
    }
}
uint16_t addr;
uint8_t data;
void loop() {
    // we do care about the loop time tho

    while(!(sio_hw->gpio_in & P(GB_CLK)));
    // a single NOP instruction takes ~8ns
    // also yes, i could not come up with a better way -- macros seem overkill here
    __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();

    // oh god i'm annotating this a lot
    // tbf i am writing something i barely understand here lol

    if(sio_hw->gpio_in & P(GB_RD)) {
        // write operation (they're inverted lolz)
        sio_hw->gpio_oe_clr |= GB_D;

        while(!(sio_hw->gpio_in & P(GB_WR)));
        addr = (sio_hw->gpio_in & (P(GB_A_MAX + 1) - 1)) >> GB_A_MIN; // a very convoluted way to read just the address pins
        while(sio_hw->gpio_in & P(GB_CLK)); // waiting for clock fall to read the data
        data = (sio_hw->gpio_in & (P(GB_D_MAX + 1) - 1)) >> GB_D_MIN;
        if(addr >= 0xa200 && addr < 0xa300) uart_tx_buf[addr - 0xa200] = data; // set data to write to esp32
        else if(addr == 0xa300) uart_n = Serial.readBytes(uart_rx_buf, data); // read data from esp32
        else if(addr == 0xa301) Serial.write(uart_tx_buf, data); // send data to esp32
        else if(addr >= 0xa000 && addr < 0xe000) ram[addr - 0xa000] = data; // writing to ram
    } else {
        // read operation
        sio_hw->gpio_oe_set |= GB_D;

        addr = (sio_hw->gpio_in & (P(GB_A_MAX + 1) - 1)) >> GB_A_MIN;
        if(addr >= 0xa100 && addr < 0xa200) data = uart_rx_buf[addr - 0xa100]; // reading from read data buffer
        else if(addr == 0xa302) data = uart_n; // read data byte count
        else if(addr >= 0x0000 && addr < 0x4000) data = rom[addr]; // reading from rom
        else if(addr >= 0xa000 && addr < 0xe000) data = ram[addr - 0xa000]; // reading from ram
        else data = 0; // fallback
        // (sio_hw->gpio_in & (P(GB_D_MAX + 1) - 1)) >> GB_D_MIN;
        sio_hw->gpio_out &= (~(P(GB_D_MAX + 1) - 1)) ^ (~(P(GB_D_MIN + 1) - 1)); // an even more convoluted way of clearing the state of all the unneeded pins
        sio_hw->gpio_out |= data << GB_D_MIN;
        while(sio_hw->gpio_in & P(GB_CLK)); // waiting for clock fall for the next iteration
    }
}