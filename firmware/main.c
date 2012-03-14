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


void play_characters(uint8_t *chs)
{
    uint8_t a2,a1,a0;
    uint16_t l = 0;

    // init audio, char 0 has info about bitrate
    flash_begin();
    l = flash_info(0, &a2, &a1, &a0);
    flash_end();
    
    audio_wav_init(l);
    
    while (*chs) {
        // get sound data for char
        flash_begin();
        l = flash_info(*chs, &a2, &a1, &a0);
        flash_end();
    
        flash_begin();
        flash_read_init(a2, a1, a0);
    
        // buffer is full, read is prepared, let it play
        if (audio_buffer_full(1)) break;

        // prefill buffer
        while (!audio_buffer_full(1) && l>0) {
            audio_wav_data(flash_read());
            l--;
        }

        if (l==0) { // whole character was read already, set up next character
            flash_end();
            chs++;
        }
        else break; // buffer is full, but the current character has data left, let it play
    }
    
    // start playing
    audio_play();
    
    // refill buffer when needed (read has already been prepared)
    // if there are data for current char, read them
    // if no, move to the next char
    while(*chs) {
        while (l>0) {
            while (audio_buffer_full(1));
            audio_wav_data(flash_read());
            l--;
        }

        // move to the next character
        chs++;
        if (*chs) {
            flash_end(); // end reading current char

            // get info about new char
            flash_begin();
            l = flash_info(*chs, &a2, &a1, &a0);
            flash_end();
            
            // inititalize read
            flash_begin();
            flash_read_init(a2, a1, a0);
        }
        else flash_end(); // no additional character
    }
    
    // wait till everything is played
    while(!audio_buffer_empty() || !audio_buffer_finished());
    audio_stop();
}

void play_character(uint8_t id)
{
    uint8_t s[] = {id, 0x0};
    play_characters(s);
}

uint8_t play_morse(uint8_t *chs)
{
    uint8_t v_id = 0x0; // last played char

    // init morse
    audio_morse_init(600, 6);

    // prefill buffer
    while (*chs && !audio_buffer_full(2)) {
        uint8_t v_idx = morse_find(*chs);
        v_id = MORSE_ID(v_idx);
        uint8_t v_bitmask = MORSE_MASK(v_idx);
        uint8_t v_length = MORSE_LEN(v_idx);

        // add morse data to buffer and play it 
        if(v_id) audio_morse_data(v_length, v_bitmask, 7);

        chs++;
    }

    audio_play();

    // keep buffer filled
    while (*chs) {
        uint8_t v_idx = morse_find(*chs);
        v_id = MORSE_ID(v_idx);
        uint8_t v_bitmask = MORSE_MASK(v_idx);
        uint8_t v_length = MORSE_LEN(v_idx);

        while(audio_buffer_full(2)); // wait till there is some space in the buffer

        // add morse data to buffer and play it 
        audio_morse_data(v_length, v_bitmask, 7);

        chs++;
    }

    while(!audio_buffer_empty() || !audio_buffer_finished());
    audio_stop();

    return v_id;
}

int main(void)
{
    setup();
    
    uint8_t c;

    dac_volume(5);
    c = 0;

    _delay_ms(2000);

    uint8_t welcome[] = "vitejte v morse testeru.";
    play_characters(welcome);

    _delay_ms(1500);

    while(1) {
        if (MORSE_ID(c) == 0) c = 0;
        uint8_t s[] = {MORSE_ID(c), 0x0};
        uint8_t v_id = play_morse(s);

        c++;

        if (v_id) {
            play_character(v_id);
            _delay_ms(4000);
        }
    }

    return 0;
}
