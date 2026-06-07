#include <gb/gb.h>
#include <gb/drawing.h>
#include <stdint.h>

#pragma dataseg DATA_0

#define RX_BUF ((uint8_t*)0xa100)
#define TX_BUF ((uint8_t*)0xa200)
#define RX_INST ((uint8_t*)0xa300)
#define TX_INST ((uint8_t*)0xa301)
#define RX_NUM ((uint8_t*)0xa302)

uint8_t cur_mode = 0;

uint8_t v, l = 0;
#define PRESSED(x) ((v & x) && !(l & x))

char message[64];
uint8_t message_length = 0;
uint8_t char_idx;

void clear(void) {
    gotogxy(0, 0);
    for(uint8_t y = 0; y < GRAPHICS_HEIGHT / 8; y++)
        for(uint8_t x = 0; x < GRAPHICS_WIDTH / 8; x++)
            wrtchr(' ');
}

char get_ch(uint8_t x, uint8_t y) {
    if(x < 2 || x >= 18) return ' ';
    if(y < 1 || y >= 17) return ' ';
    uint8_t i = (y - 1) * 16 + x - 2;
    if(i >= 0x90 && i - 0x90 < message_length) return message[i - 0x90];
    if(i < 0x20 || i >= 0x7f) return ' ';
    return (char)i;
}
uint8_t get_x(void) {
    return (char_idx & 15) + 2;
}
uint8_t get_y(void) {
    return (char_idx >> 4) + 1;
}
void init_kb(void) {
    cur_mode = 1;
    char_idx = 0x20;
    message_length = 0;
    gotogxy(0, 0);
    for(uint8_t y = 0; y < GRAPHICS_HEIGHT / 8; y++)
        for(uint8_t x = 0; x < GRAPHICS_WIDTH / 8; x++) {
            if(x == get_x() && y == get_y()) color(WHITE, BLACK, SOLID);
            else color(BLACK, WHITE, SOLID);
            wrtchr(get_ch(x, y));
        }
}
void serial(void) {
    *RX_INST = 255;
    uint8_t n = *RX_NUM;
    for(uint16_t i = 0; i < n; i++)
        wrtchr(RX_BUF[i]);
    
    if(PRESSED(J_B))
        clear();
    else if(PRESSED(J_A))
        init_kb();
}
void keyboard(void) {
    uint8_t last = char_idx;
    uint8_t lx = get_x(), ly = get_y();
    if(PRESSED(J_LEFT)) char_idx--;
    if(PRESSED(J_RIGHT)) char_idx++;
    if(PRESSED(J_UP)) char_idx -= 16;
    if(PRESSED(J_DOWN)) char_idx += 16;
    if(char_idx < 0x20) char_idx += 0x5f;
    if(char_idx >= 0x7f) char_idx -= 0x5f;
    if(char_idx != last) {
        color(WHITE, BLACK, SOLID);
        gotogxy(get_x(), get_y());
        wrtchr(get_ch(get_x(), get_y()));
        color(BLACK, WHITE, SOLID);
        gotogxy(lx, ly);
        wrtchr(get_ch(lx, ly));
    }

    if(PRESSED(J_SELECT)) {
        clear();
        cur_mode = 0;
    } else if(PRESSED(J_START)) {
        clear();
        cur_mode = 0;

        for(uint8_t i = 0; i < message_length; i++)
            TX_BUF[i] = message[i];
        *TX_INST = message_length;
    } else if(PRESSED(J_A) && message_length < 64) {
        char c = get_ch(get_x(), get_y());

        gotogxy((message_length & 15) + 2, (message_length >> 4) + 10);
        wrtchr(c);

        message[message_length] = c;
        message_length++;
    } else if(PRESSED(J_B) && message_length > 0) {
        message_length--;
        gotogxy((message_length & 15) + 2, (message_length >> 4) + 10);
        wrtchr(' ');
    }
}

void main(void) {
    ENABLE_RAM;
    // SWITCH_RAM(0);

    // Loop forever
    while(1) {
        v = joypad();
        if(cur_mode) keyboard();
        else serial();
        vsync();
        l = v;
    }
}
