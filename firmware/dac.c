#include <avr/io.h>
#include "dac.h"
#include "spi.h"

/* protocol:

   [A3, A2, A1, A0, C1, C0, D9, D8, ... , D0]

   [A3:A0] register address
   [C1:C0] 11 = read, 00 = write, 01 = increment (8bit), 10 = decrement (8bit)

   data in D9 are ignored, since internal registers are only 9bit wide
   8 bit commands use only D9, D8

   device response contains CMDERR bit in D8 (1 = OK, 0 = error)
*/

/* MCP4XXX register addresses */
#define DAC_AUDIO 0x01
#define DAC_VOLUME 0x00
#define DAC_TCON 0x04
#define DAC_STATUS 0x05

/* MCP4XXX commands */
#define DAC_WRITE 0b00
#define DAC_READ 0b11
#define DAC_INC 0b01
#define DAC_DEC 0b10

/* volume status */
uint8_t volume_level;


void dac_fadein()
{
}

void dac_fadeout()
{
}

void dac_volume(uint8_t vol)
{
    volume_level = vol;
    volume_flags |= _BV(VOLUME_CHANGED);
}

void dac_output(uint8_t value)
{
    /* output the command byte for audio value */
    spi_transfer((DAC_AUDIO << 4) | (DAC_WRITE << 2));

    /* output the data byte for audio value */
    spi_transfer(value);

    if (volume_flags & _BV(VOLUME_CHANGED)) {
        /* output the command byte for volume value */
        spi_transfer((DAC_VOLUME << 4) | (DAC_WRITE << 2));

        /* output the data byte for volume value */
        spi_transfer((volume_flags & VOLUME_MUTE) ? 0 : volume_level);

        volume_flags &= ~_BV(VOLUME_CHANGED);
    }
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
    volume_flags |= _BV(VOLUME_MUTE) | _BV(VOLUME_CHANGED);
}

void dac_unmute()
{
    volume_flags &= ~_BV(VOLUME_MUTE);
    volume_flags |= _BV(VOLUME_CHANGED);
}
