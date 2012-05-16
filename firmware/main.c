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
#include "play.h"
#include "interface.h"

static uint8_t randomizer EEMEM = 0;

static enum{
    MODE_GROUPS,
    MODE_DIGRAMS,
    MODE_MORSE_TX,
    MODE_MORSE_RX,
    MODE_KEYING
} teaching_mode;

void setup(void)
{
    /* disable AC */
    ACSRA |= _BV(ACD);

    /* setup USI - SPI, PORTB */
    USICR = _BV(USIWM0);
    USIPP &= ~_BV(USIPOS);

    /* setup MOSI/SCK ports */
    DDRB |= _BV(DDB1) | _BV(DDB2);
    PORTB &= ~_BV(PB1) & ~_BV(PB2);

    /* initialize subsystem interfaces */
    dac_init();
    flash_init();
    leds_init();
    interface_init();

    /* initialize random and change seed for next boot */
    uint8_t srand = eeprom_read_byte(&randomizer);
    srandom(1+srand);
    eeprom_write_byte(&randomizer, srand+1);

    /* initialize learning mode */
    teaching_mode = MODE_MORSE_TX;
}

#define BUFFER_LEN 10
static uint8_t buffer[BUFFER_LEN+1];

#define MENU_SELECT 1
#define MENU_PREV 2
#define MENU_NEXT 3
#define MENU_TIMEOUT 0

uint8_t menu_item(const uint8_t *entry)
{
    play_characters(entry, getchar_eep, COMPOSED);
    
    led_on(LED_RED);
    interface_buttons &= ~(_BV(BUTTON) | _BV(ROTARY_NEXT) | _BV(ROTARY_PREV));
    audio_wait_init(6);
    audio_play();
    timeout(1500, _BV(BUTTON));
    led_off(LED_RED);
    audio_stop();

    if (interface_buttons & _BV(BUTTON)) return MENU_SELECT;
    else if (interface_buttons & _BV(ROTARY_NEXT)) return MENU_NEXT;
    else if (interface_buttons & _BV(ROTARY_PREV)) return MENU_PREV;

    return MENU_TIMEOUT;
}

/* for some reason, this code was more space efficient on gcc
   than div or itoa from avr-libc with -Os
*/
static uint8_t tensinascii(uint8_t n)
{
    return '0' + n / (uint8_t)10;
}

static uint8_t onesinascii(uint8_t n)
{
    return '0' + n % (uint8_t)10;
}


int main(void)
{
    setup();
    dac_volume(128);

    uint8_t lesson_flags = 0;    

    while(1) {
        /* menu code */
        interface_begin(LATCHING_MODE, 0);
        volume_flags |= _BV(HOLD_VOLUME);

        while(1) {
            if (menu_item(s_resume) == MENU_SELECT) {
                break;
            }

            if (menu_item(s_status) == MENU_SELECT) {
                delay(500);
                play_characters(s_score, getchar_eep);

                buffer[0] = tensinascii(correct);
                buffer[1] = onesinascii(correct);
                buffer[2] = '\0';
                play_characters(buffer, getchar_str);
                continue;
            }

            if (menu_item(s_morse_tx) == MENU_SELECT) {
                teaching_mode = MODE_MORSE_TX;
                continue;
            }

            if (menu_item(s_morse_rx) == MENU_SELECT) {
                teaching_mode = MODE_MORSE_RX;
                continue;
            }

            if (menu_item(s_digrams) == MENU_SELECT) {
                teaching_mode = MODE_DIGRAMS;
                continue;
            }

            if (menu_item(s_groups) == MENU_SELECT) {
                teaching_mode = MODE_GROUPS;
                continue;
            }
            
            if (menu_item(s_keying) == MENU_SELECT) {
                teaching_mode = MODE_KEYING;
                continue;
            }

            if ((lesson_flags & LESSON_ALL) && menu_item(s_lesson_repeat) == MENU_SELECT) {
                lesson_flags &= ~LESSON_ALL;
                continue;
            }

            if (!(lesson_flags & LESSON_ALL) && menu_item(s_lesson_all) == MENU_SELECT) {
                lesson_flags |= LESSON_ALL;
                continue;
            }

            if (menu_item(s_next) == MENU_SELECT) {
                lesson_change(1);
                continue;
            }

            if (menu_item(s_previous) == MENU_SELECT) {
                lesson_change(-1);
                continue;
            }

            if (menu_item(s_add) == MENU_SELECT) {
                lesson_chars_change(1);
                continue;
            }

            if (menu_item(s_remove) == MENU_SELECT) {
                lesson_chars_change(-1);
                continue;
            }

        }

        interface_begin(LATCHING_MODE, 0);
        volume_flags &= ~_BV(HOLD_VOLUME);

        /* teaching code */
        while(!(interface_buttons & _BV(BUTTON))) {
            uint8_t lesson = lesson_id();

            uint8_t speed = 6, effective_speed = 6;
            uint8_t correct = 0;
            uint8_t lesson_len = 0;

            /* Prepare one block of lesson data */
            if (teaching_mode == MODE_MORSE_TX || teaching_mode == MODE_MORSE_RX) lesson_len = lesson_new(lesson, 1, &speed, &effective_speed, buffer, lesson_flags);
            else if (teaching_mode == MODE_DIGRAMS) lesson_len = lesson_new(lesson, 2, &speed, &effective_speed, buffer, lesson_flags);
            else if (teaching_mode == MODE_GROUPS) lesson_len = lesson_new(lesson, BUFFER_LEN, &speed, &effective_speed, buffer, lesson_flags | LESSON_GROUPS);

            /* Lower score */
            if (correct > lesson_len) correct -= lesson_len;
            else correct = 0;

            /* Play "question part" */
            if (teaching_mode == MODE_MORSE_TX ||
                teaching_mode == MODE_KEYING) {
                play_characters(buffer, getchar_str, FULL);
            }
            else {
                audio_morse_init(500, speed, effective_speed);
                play_morse(buffer, getchar_str);
            }

            /* Prepare interface for counting answers */
            interface_buttons &= ~_BV(KEY_A);
            led_on(LED_RED);

            /* Play "answer" and count */
            if (teaching_mode == MODE_MORSE_TX ||
                teaching_mode == MODE_KEYING) {
                audio_morse_init(500, speed, effective_speed);
                play_morse(buffer, getchar_str);
            }
            else {
                play_characters(buffer, getchar_str, FULL);
            }

            /* Wait for click */
            timeout(1500, _BV(KEY_A));

            /* Increase score */
            if (interface_buttons & _BV(KEY_A)) correct += lesson_len;

            /* If the score is higher than 100, add character */
            if (correct >= 99) {
                lesson_chars_change(1);
                correct = 0; // reset counter
            }

            _delay_ms(2000);
        }
    }

    return 0;
}
