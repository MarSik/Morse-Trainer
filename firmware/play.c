#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include "audio.h"
#include "flash.h"
#include "morse.h"
#include "play.h"
#include "sine.h"
#include "interface.h"
#include "lang.h"

uint8_t getchar_str(const uint8_t *s)
{
    return *s;
}

uint8_t getchar_eep(const uint8_t *s)
{
    return eeprom_read_byte(s);
}

uint8_t getchar_pgm(const uint8_t *s)
{
    return pgm_read_byte(s);
}

void play_characters(const uint8_t *chs, getchar_f get)
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
        while (!audio_buffer_full() && l>0) {
            audio_wav_data(flash_read());
            l--;
        }

        if (l==0) { // whole character was read already, set up next character
            flash_end();
            ++chs;
        }
        else break; // buffer is full, but the current character has data left, let it play
    }
    
    // start playing
    audio_play();
    
    // refill buffer when needed (read has already been prepared)
    // if there are data for current char, read them
    // if no, move to the next char
    set_sleep_mode(SLEEP_MODE_IDLE);
    while(get(chs)) {
        while (l>0) {
            while (audio_buffer_full()) sleep_mode();
            audio_wav_data(flash_read());
            l--;
        }
        flash_end();

        // move to the next character
        ++chs;
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
    while(!audio_buffer_finished()) sleep_mode();
    audio_stop();
}

void play_character(uint8_t id)
{
    uint8_t s[] = {id, 0x0};
    play_characters(s, getchar_str);
}

uint8_t play_morse(const uint8_t *chs, getchar_f get)
{
    uint8_t v_id = 0x0; // last played char
    uint8_t next = 0x0;

    // prefill buffer
    v_id = get(chs);
    while (v_id && !audio_buffer_full()) {
        disable_sine_int();
        uint8_t v_idx = morse_find(v_id, &v_id);
        enable_sine_int();
        ++chs;
        next = get(chs);

        // add morse data to buffer and play it
        // also check the letter ahead to determine space length
        if(v_id) {
            disable_sine_int();
            uint8_t v_bitmask = MORSE_MASK(v_idx);
            uint8_t v_length = MORSE_LEN(v_idx);
            enable_sine_int();
            audio_morse_data(v_length, v_bitmask,
                             (next == SPACE) ?
                             WORD_SPACE_LEN : LETTER_SPACE_LEN);
        }

        v_id = next;
    }

    audio_play();

    // keep buffer filled
    set_sleep_mode(SLEEP_MODE_IDLE);
    while (v_id) {
        disable_sine_int();
        uint8_t v_idx = morse_find(v_id, &v_id);
        enable_sine_int();

        ++chs;
        next = get(chs);

        // add morse data to buffer and play it
        if(v_id) {
            disable_sine_int();
            uint8_t v_bitmask = MORSE_MASK(v_idx);
            uint8_t v_length = MORSE_LEN(v_idx);
            enable_sine_int();
            while(audio_buffer_full()) sleep_mode(); // wait till there is some space in the buffer

            audio_morse_data(v_length, v_bitmask,
                             (next == ' ') ?
                             WORD_SPACE_LEN : LETTER_SPACE_LEN);
        }

        v_id = next;
    }

    while(!audio_buffer_finished()) sleep_mode();
    audio_stop();

    return v_id;
}
