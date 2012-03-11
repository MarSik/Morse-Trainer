#include <avr/io.h>
#include "dac.h"

/* protocol:

   [A3, A2, A1, A0, C1, C0, D9, D8, ... , D0]

   [A3:A0] register address
   [C1:C0] 11 = read, 00 = write, 01 = increment (8bit), 10 = decrement (8bit)

   data in D9 are ignored, since internal registers are only 9bit wide
   8 bit commands use only D9, D8

   device response contains CMDERR bit in D8 (1 = OK, 0 = error)
*/

/* MCP4XXX register addresses */
#define DAC_AUDIO 0x00
#define DAC_VOLUME 0x01
#define DAC_TCON 0x04
#define DAC_STATUS 0x05

/* MCP4XXX commands */
#define DAC_WRITE 0b00
#define DAC_READ 0b11
#define DAC_INC 0b01
#define DAC_DEC 0b10

/* volume status */
struct{
    uint8_t level:8;
    uint8_t changed:1;
    uint8_t mute:1;
} volume;


void dac_begin()
{
    // SPI mode data clocked in on positive edge
    //          data read on following negative edge
    //          USI uses PORTB
    //          USI uses software clock USICLK/USITC
    USICR = _BV(USIWM0);
    USIPP &= ~_BV(USIPOS);

    // CS active low
    DAC_PORT &= ~_BV(DAC_CS);
}


void dac_end()
{
    DAC_PORT |= _BV(DAC_CS);
}

void dac_volume(uint8_t vol)
{
    volume.level = vol;
    volume.changed = 1;
}

void dac_output(uint8_t value)
{
    uint8_t i;

    /* output the command byte for audio value */
    USIDR = (DAC_AUDIO << 4) | (DAC_WRITE << 2);
    for(i = 0; i < 8; i++) {
        USICR |= _BV(USITC);
        USICR |= _BV(USITC) | _BV(USICLK);
    }

    /* output the data byte for audio value */
    USIDR = value;
    for(i = 0; i < 8; i++) {
        USICR |= _BV(USITC);
        USICR |= _BV(USITC) | _BV(USICLK);
    }


    if (!volume.changed) return;

    /* output the command byte for volume value */
    USIDR = (DAC_VOLUME << 4) | (DAC_WRITE << 2);
    for(i = 0; i < 8; i++) {
        USICR |= _BV(USITC);
        USICR |= _BV(USITC) | _BV(USICLK);
    }


    /* output the data byte for volume value */
    USIDR = volume.mute ? 0 : volume.level;
    for(i = 0; i < 8; i++) {
        USICR |= _BV(USITC);
        USICR |= _BV(USITC) | _BV(USICLK);
    }

    volume.changed = 0;
}

void dac_init()
{
    DAC_DDR |= _BV(DAC_CS);
    DAC_PORT |= _BV(DAC_CS);

    /* full output, middle level */
    dac_volume(255);
    dac_begin();
    dac_output(128);
    dac_end();
}

void dac_mute()
{
    volume.mute = 1;
    volume.changed = 1;
}

void dac_unmute()
{
    volume.mute = 0;
    volume.changed = 1;
}
