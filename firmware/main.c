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

uint8_t randomizer EEMEM = 0;
uint8_t teaching_mode;

#define MODE_SINGLE 0

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
    leds_init();
    interface_init();

    /* initialize random and change seed for next boot */
    uint8_t srand = eeprom_read_byte(&randomizer);
    srandom(1+srand);
    eeprom_write_byte(&randomizer, srand+1);

    /* initialize learning mode */
    teaching_mode = _BV(MODE_SINGLE);
}

static uint8_t buffer[51];


uint8_t menu_item(const uint8_t *entry)
{
    play_characters(entry, getchar_eep);
    
    led_on(LED_RED);
    interface_buttons &= ~_BV(KEY_A);
    audio_wait_init(6);
    audio_play();
    timeout(1500, KEY_A);
    led_off(LED_RED);
    audio_stop();
    if (interface_buttons & _BV(KEY_A)) {
        return 1;
    }
    _delay_ms(500);
    return 0;
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
        /* menu code */
        interface_begin(LATCHING_MODE, 0);
        while(1) {
            if (menu_item(s_single)) {
                teaching_mode = _BV(MODE_SINGLE);
                break;
            }

            if (menu_item(s_groups)) {
                teaching_mode = 0;
                break;
            }
            
            if (menu_item(s_next)) {
                lesson_change(1);
                break;
            }

            if (menu_item(s_previous)) {
                lesson_change(-1);
                break;
            }

        }
        interface_end();

        /* teaching code */
        while(!(interface_buttons & _BV(BUTTON))) {
            uint8_t lesson = lesson_id();

            play_characters(s_lesson, getchar_eep);

            buffer[0] = tensinascii(lesson+1);
            buffer[1] = onesinascii(lesson+1);
            buffer[2] = 0;
            play_characters(buffer, getchar_str);

            uint8_t lesson_len, speed = 6, effective_speed = 6;
            uint8_t correct = 0;
            lesson_len = lesson_new(lesson, 50, &speed, &effective_speed, buffer);

            audio_wait_init(2);
            audio_play();
            while(PINA & _BV(PA4));
            audio_stop();

            _delay_ms(1000);

            if (teaching_mode & _BV(MODE_SINGLE)) {
                /* single char teaching mode */
                uint8_t *ch = buffer;

                while (*ch) {
                    uint8_t xid;
                    uint8_t tmp[] = {*ch, 0};
                
                    // we do not need to disable morse ints, because
                    // sinewave is not playing yet
                    morse_find(*ch, &xid);
                    if (*ch == xid) {
                        play_characters(tmp, getchar_str);
                        _delay_ms(500);
                    
                        audio_morse_init(500, speed, speed);
                        play_morse(tmp, getchar_str);
                    
                        interface_begin(LATCHING_MODE, 0);
                        led_on(LED_RED);
                        audio_wait_init(6);
                        audio_play();
                        timeout(1500, KEY_A);
                        led_off(LED_RED);
                        audio_stop();
                        interface_end();
                    
                        if (interface_buttons & _BV(KEY_A)) ++correct;
                    }
                    else lesson_len--;
                
                    ++ch;
                }
            }
            /* end single char teaching mode */        


            /* long test */
            else {
                audio_morse_init(500, speed, effective_speed);
                play_morse(buffer, getchar_str);
                _delay_ms(1500);

                led_on(LED_RED);
                interface_begin(LATCHING_MODE, _BV(KEY_A));
                _delay_ms(500);
                play_characters(buffer, getchar_str);
                _delay_ms(1000);
                led_off(LED_RED);
                interface_end();
                correct = interface_presses;
            }
            /* end long test */

            /* play resulting score */
            play_characters(s_correct, getchar_eep);

            buffer[0] = tensinascii(correct);
            buffer[1] = onesinascii(correct);
            buffer[2] = 0;
            play_characters(buffer, getchar_str);

            play_characters(s_outof, getchar_eep);

            buffer[0] = tensinascii(lesson_len);
            buffer[1] = onesinascii(lesson_len);
            buffer[2] = 0;
            play_characters(buffer, getchar_str);

            /* if the success rate is higher than 95%, move to the next lesson */
            if (correct >= (lesson_len - lesson_len/20)) {
                lesson_change(1);
                play_characters(s_congrats, getchar_eep);
            }

            _delay_ms(3000);
        }
    }

    return 0;
}
