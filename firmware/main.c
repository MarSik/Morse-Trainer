#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <stdlib.h>

#include "dac.h"
#include "flash.h"
#include "audio.h"

void setup(void)
{
    /* setup SPI ports */
    DDRB |= _BV(PB0) | _BV(PB2);
    PORTB = 0;

    dac_init();
    flash_init();
}

int main(void)
{
    setup();

    audio_morse_init(600, 10);

    while(!audio_buffer_full())
        audio_morse_data(4, 0b00001000, 7);

    dac_volume(255);
    audio_play();

    while(1) {
        while(audio_buffer_full());
        audio_morse_data(4, 0b00001000, 7);
    }

    return 0;
}
