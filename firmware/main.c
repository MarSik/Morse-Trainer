#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>

#include "dac.h"
#include "flash.h"
#include "audio.h"
#include "morse.h"
#include "sine.h"

void setup(void)
{
    /* setup USI - SPI, PORTB */
    USICR = _BV(USIWM0);
    USIPP &= ~_BV(USIPOS);

    /* setup MOSI/SCK ports */
    DDRB |= _BV(DDB1) | _BV(DDB2);
    PORTB &= ~_BV(PB1) & ~_BV(PB2);

    /* initialize subsystem interfaces */
    dac_init();
    flash_init();
}

int main(void)
{
    setup();
    
    uint8_t c;
    uint8_t a2,a1,a0;
    uint16_t l;

    dac_volume(100);
    c = 0;

    while(1) {
        uint8_t v_bitmask = MORSE_MASK(c);
        uint8_t v_length = MORSE_LEN(c);
        uint8_t v_id = MORSE_ID(c);
        c = (c + 1) % MORSE_TABLE_LEN;

        // init morse
        audio_morse_init(600, 6);

        // add morse data to buffer and play it 
        audio_morse_data(v_length, v_bitmask, 7);
        audio_play();
        while(!audio_buffer_empty());
        audio_stop();

        _delay_ms(500);

        // init audio
        audio_wav_init(8000);

        // get sound data for morse char
        l = flash_info(v_id, &a2, &a1, &a0);
        flash_read_init(a2, a1, a0);

        // prefill buffer
        while (!audio_buffer_full(1) && l>0) {
            audio_wav_data(flash_read());
            l--;
        }

        // start playing
        audio_play();

        // refill buffer when needed
        while (l>0) {
            while (audio_buffer_full(1));
            audio_wav_data(flash_read());
            l--;
        }

        // wait till everything is played
        while(!audio_buffer_empty());
        audio_stop();

        _delay_ms(500);
    }

    return 0;
}
