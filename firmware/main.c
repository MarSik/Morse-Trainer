#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>

#include "lang.h"
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

    /* setup KEY ports */
    DDRA &= ~_BV(PA5) & ~_BV(PA4);
    PORTA |= _BV(PA5) | _BV(PA4);

    /* setup leds */
    DDRA |= _BV(PA7) | _BV(PA6) | _BV(PA3);
    PORTA |= _BV(PA7) /*| _BV(PA6)*/ | _BV(PA3);

    /* setup ADC pin PA2 */

    /* setup rotary */
    DDRB &= 0b00001111;
    PORTB |= 0b01110000;

    /* initialize subsystem interfaces */
    dac_init();
    flash_init();
}

typedef uint8_t (*getchar_f)(uint8_t *);

uint8_t getchar_str(uint8_t *s)
{
    return *s;
}

uint8_t getchar_eep(uint8_t *s)
{
    return eeprom_read_byte(s);
}


void play_characters(uint8_t *chs, getchar_f get)
{
    uint8_t a2,a1,a0;
    uint16_t l = 0;

    // init audio, char 0 has info about bitrate
    flash_begin();
    l = flash_info(0, &a2, &a1, &a0);
    flash_end();
    
    audio_wav_init(l);
    
    while (get(chs)) {
        // get sound data for char
        flash_begin();
        l = flash_info(get(chs), &a2, &a1, &a0);
        flash_end();
    
        flash_begin();
        flash_read_init(a2, a1, a0);
    
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
    while(get(chs)) {
        while (l>0) {
            while (audio_buffer_full(1));
            audio_wav_data(flash_read());
            l--;
        }
        flash_end();

        // move to the next character
        chs++;
        if (get(chs)) {
            // get info about new char
            flash_begin();
            l = flash_info(get(chs), &a2, &a1, &a0);
            flash_end();
            
            // inititalize read
            flash_begin();
            flash_read_init(a2, a1, a0);
        }
    }
    
    // wait till everything is played
    while(!audio_buffer_finished());
    audio_stop();
}

void play_character(uint8_t id)
{
    uint8_t s[] = {id, 0x0};
    play_characters(s, getchar_str);
}

uint8_t play_morse(uint8_t *chs, getchar_f get)
{
    uint8_t v_id = 0x0; // last played char

    // init morse
    audio_morse_init(400, 12, 12);

    // prefill buffer
    while (get(chs) && !audio_buffer_full(2)) {
        uint8_t v_idx = morse_find(get(chs));
        v_id = MORSE_ID(v_idx);

        // add morse data to buffer and play it
        // also check the letter ahead to determine space length
        if(v_id) {
            uint8_t v_bitmask = MORSE_MASK(v_idx);
            uint8_t v_length = MORSE_LEN(v_idx);

            audio_morse_data(v_length, v_bitmask,
                             (get(chs+1) == ' ') ?
                             WORD_SPACE_LEN : LETTER_SPACE_LEN);
        }

        chs++;
    }

    audio_play();

    // keep buffer filled
    while (get(chs)) {
        uint8_t v_idx = morse_find(get(chs));
        v_id = MORSE_ID(v_idx);

        // add morse data to buffer and play it 
        if(v_id) {
            uint8_t v_bitmask = MORSE_MASK(v_idx);
            uint8_t v_length = MORSE_LEN(v_idx);

            while(audio_buffer_full(2)); // wait till there is some space in the buffer

            audio_morse_data(v_length, v_bitmask,
                             (get(chs+1) == ' ') ?
                             WORD_SPACE_LEN : LETTER_SPACE_LEN);
        }

        chs++;
    }

    while(!audio_buffer_finished());
    audio_stop();

    return v_id;
}

int main(void)
{
    setup();
    
    uint8_t c;

    dac_begin();
    dac_volume(255);
    dac_end();

    c = 0;

    _delay_ms(2000);

    uint8_t welcome[] = "vitejte v morse trenerovi.";
    play_characters(welcome, getchar_str);

    uint8_t welcome_morse[] = "VITEJTE V MORSE TRENEROVI.";
    play_morse(welcome_morse, getchar_str);

    _delay_ms(1500);

    while(1) {
        uint8_t s[] = {MORSE_ID(c), 0x0};
        uint8_t v_id = play_morse(s, getchar_str);

        c++;

        if (v_id) {
            play_character(v_id);
            _delay_ms(4000);
        }
        else c = 0;
    }

    return 0;
}
