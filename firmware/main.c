#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>

#include "dac.h"
#include "flash.h"
#include "audio.h"
#include "morse.h"

void setup(void)
{
    /* setup MOSI/SCK ports */
    DDRB |= _BV(DDB1) | _BV(DDB2);
    PORTB &= ~_BV(PB1) & ~_BV(PB2);
    USIPP = 0;

    /* initialize subsystem interfaces */
    dac_init();
    flash_init();
}

int main(void)
{
    setup();
    
    audio_morse_init(600, 10);

    while(!audio_buffer_full())
        audio_morse_data(morse_table[25*3], morse_table[25*3+2], 7);

    dac_volume(255);
    audio_play();

    while(1) {
        while(audio_buffer_full());
        audio_morse_data(morse_table[25*3], morse_table[25*3+2], 7);
    }
    
    return 0;
}
