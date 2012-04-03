#ifndef __MT_dac_MS_20120309_
#define __MT_dac_MS_20120309_

#include "spi.h"
#include "flash.h"

/* ports */
#define DAC_DDR DDRB
#define DAC_PORT PORTB
#define DAC_CS 3

//!!! SHARED with audio interface, use only 4 low bits
//static uint8_t volume_flags;
#define volume_flags GPIOR2

#define VOLUME_MUTE 0
#define VOLUME_CHANGED 1
#define HOLD_VOLUME 3 /* using rotary will not modify volume */

/* initializes dac during bootup */
void dac_init(void);

/* before anything can be done, dac has to be selected using dac_begin */
void inline dac_begin(void)
{
    flash_pause();
    DAC_PORT &= ~_BV(DAC_CS);
}


void inline dac_end(void)
{
    DAC_PORT |= _BV(DAC_CS);
    flash_unpause();
}


/* set audio volume 0 - 255 */
void dac_volume(uint8_t vol);

extern volatile uint8_t volume_level;

/* add substract one from actual volume till it reaches MAX or 0 */ 
void inline dac_louder(void)
{
    if(volume_level < 245) {
        volume_level += 10;
        volume_flags |= _BV(VOLUME_CHANGED);
    }
}

void inline dac_quieter(void)
{
    if(volume_level > 10) {
        volume_level -= 10;
        volume_flags |= _BV(VOLUME_CHANGED);
    }
}


/* add substract one from actual volume till it reaches volume or 0 */ 
void dac_fadein(void);
void dac_fadeout(void);

/* set dac output to value*Vcc/256 */
void dac_output(uint8_t value);

/* mute/unmute but remember volume */
void dac_mute(void);
void dac_unmute(void);

#endif /* __MT_dac_MS_20120309_ */
