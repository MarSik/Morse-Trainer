#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <stdlib.h>

#include "lang.h"
#include "dac.h"
#include "flash.h"
#include "audio.h"
#include "morse.h"
#include "leds.h"
#include "lesson.h"

uint8_t teaching_lesson EEMEM = 0;
uint8_t randomizer EEMEM = 0;

/* use watchdog interrupt for software debounce */
ISR(WDT_vect)
{
    /* disable WD */
    WDTCR |= _BV(WDCE);
    WDTCR &= ~_BV(WDE);

    /* enable input interrupts */
}

void debounce(void)
{
    /* disable input interrupts */

    /* reset WDT timer */
    WDTCR |= _BV(WDIE); /* interrupt has to be enabled, we need it and it prevents reboot */
    WDTCR |= _BV(WDE);
}

void setup(void)
{
    /* setup USI - SPI, PORTB */
    USICR = _BV(USIWM0);
    USIPP &= ~_BV(USIPOS);

    /* setup MOSI/SCK ports */
    DDRB |= _BV(DDB1) | _BV(DDB2);
    PORTB &= ~_BV(PB1) & ~_BV(PB2);

    /* setup KEY ports */
    DDRA |= _BV(PA5); // middle of the jack connector
    DDRA &= ~_BV(PA4); // tip of the jack connector
    PORTA |= _BV(PA4) | _BV(PA5);

    /* setup ADC pin PA2 */

    /* setup rotary */
    DDRB &= 0b00001111;
    PORTB |= 0b01110000;

    /* initialize subsystem interfaces */
    dac_init();
    flash_init();
    leds_init();

    /* initialize random and change seed for next boot */
    uint8_t srand = eeprom_read_byte(&randomizer);
    srandom(srand);
    eeprom_write_byte(&randomizer, srand+1);
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

static uint8_t buffer[31];

int main(void)
{
    setup();
    
    uint8_t c;

    dac_begin();
    dac_volume(128);
    dac_end();

    c = 0;

    _delay_ms(2000);

    play_characters(s_welcome, getchar_eep);

    // init morse
    audio_morse_init(500, 20, 20);
    play_morse(s_welcome_morse, getchar_eep);

    _delay_ms(1500);

    while(1) {
        uint8_t lesson = eeprom_read_byte(&teaching_lesson);

        play_characters(s_lesson, getchar_eep);

        buffer[0] = ((lesson+1) / 10) + '0';
        buffer[1] = ((lesson+1) % 10) + '0';
        buffer[2] = 0;
        play_characters(buffer, getchar_str);

        uint8_t speed, effective_speed;
        lesson_new(lesson, 30, &speed, &effective_speed, buffer);

        audio_wait_init();
        audio_play();
        while(PINA & _BV(PA4));
        audio_stop();

        _delay_ms(1000);

        audio_morse_init(500, speed, effective_speed);
        play_morse(buffer, getchar_str);
        _delay_ms(2000);

        play_characters(buffer, getchar_str);

        _delay_ms(5000);
    }

    return 0;
}
