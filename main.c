
#include <stdio.h>

#include <gb/gb.h>
#include <gb/font.h>
#include <gb/console.h>
#include <gb/drawing.h>
#include <time.h>

#define BACKGROUND_ADDR 0x9800u

unsigned char fontRemapping[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,96,97,
0,0,0,0,0,0,0,0,0,0,0,0,98,99,100,101,
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95
};

UBYTE joypadData;


UBYTE strlen(const UBYTE *str) {
    UBYTE len = 0;
    while (*str++) len++;
    return len; 
}

void toDisplay(UBYTE* address, const UBYTE* data, UWORD size)
{
    const UBYTE *d = data; 
    for(UBYTE i = 0 ; i < size; ++i)
    {
        while(STAT_REG & 0b10);
        *address++ = fontRemapping[*d++];
    }
}

void BackgroundPrintf(UBYTE x, UBYTE y, const unsigned char * text) {
    UBYTE * addr = (UBYTE *)(BACKGROUND_ADDR + ((UWORD)y << 5) + x); 
    toDisplay(addr, text, strlen(text));
}


UINT8 serial_send_recv(UINT8 b) {
    SB_REG = b;                 // Set byte to transfer
    SC_REG = 0x81;              // Transfer requested, use internal clock
    while(SC_REG & 0x80);       // While transfer in progress, wait...
    return SB_REG;              // Received byte
}

void printHex(UBYTE x, UBYTE y, UINT8 val) {
    UINT8 hiNibble = ((val & 0xf0) >> 4);
    unsigned char hi[2] = {
        hiNibble < 0x0a ? hiNibble + 0x30 : hiNibble + 0x37,
        0
    };
    UINT8 loNibble = (val & 0x0f);
    unsigned char lo[2] = {
        loNibble < 0x0a ? loNibble + 0x30 : loNibble + 0x37,
        0
    };
    BackgroundPrintf( x, y, hi );
    BackgroundPrintf( x+1, y, lo );
}


void main()
{
    font_t font;
    font_init();

    color(WHITE, BLACK, SOLID);

    font = font_load(font_ibm);
    font_set(font);

    mode(get_mode() | M_NO_SCROLL);

    
    BackgroundPrintf( 4, 0, "GBLink Inputs" );
    BackgroundPrintf( 2, 4, "INPUTS: " );

    while(TRUE)
    {
        //wait_vbl_done();

        joypadData = joypad();

        // Send joypad data constantly over serial link (which is conveniently a byte long)
        serial_send_recv(joypadData);

        printHex(10, 4, joypadData);
    }
}