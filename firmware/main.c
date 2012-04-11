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
    MODE_SINGLE,
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
    teaching_mode = MODE_SINGLE;
}

#define BUFFER_LEN 100
static uint8_t buffer[BUFFER_LEN+1];


uint8_t menu_item(const uint8_t *entry)
{
    play_characters(entry, getchar_eep, COMPOSED);
    
    led_on(LED_RED);
    interface_buttons &= ~_BV(BUTTON);
    audio_wait_init(6);
    audio_play();
    timeout(1500, _BV(BUTTON));
    led_off(LED_RED);
    audio_stop();
    if (interface_buttons & _BV(BUTTON)) {
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
    dac_volume(128);

    _delay_ms(2000);
    play_characters(s_welcome, getchar_eep, COMPOSED);

    // init morse
    audio_morse_init(500, 20, 20);
    play_morse(s_welcome_morse, getchar_eep);

    _delay_ms(1500);
    
    while(1) {
        /* menu code */
        interface_begin(LATCHING_MODE, 0);

        while(1) {
            if (menu_item(s_single)) {
                teaching_mode = MODE_SINGLE;
                break;
            }

            if (menu_item(s_groups)) {
                teaching_mode = MODE_GROUPS;
                break;
            }
            
            if (menu_item(s_keying)) {
                teaching_mode = MODE_KEYING;
                break;
            }

            if (menu_item(s_next)) {
                lesson_change(1);
                continue;
            }

            if (menu_item(s_previous)) {
                lesson_change(-1);
                continue;
            }

        }

        interface_begin(LATCHING_MODE, 0);

        /* teaching code */
        while(!(interface_buttons & _BV(BUTTON))) {
            uint8_t lesson = lesson_id();

            play_characters(s_lesson, getchar_eep, COMPOSED);

            buffer[0] = tensinascii(lesson+1);
            buffer[1] = onesinascii(lesson+1);
            buffer[2] = 0;
            play_characters(buffer, getchar_str, FULL);

            uint8_t speed = 6, effective_speed = 6;
            uint8_t correct = 0;
            uint8_t lesson_len = lesson_new(lesson, BUFFER_LEN, &speed, &effective_speed, buffer);

            audio_wait_init(2);
            audio_play();
            set_sleep_mode(SLEEP_MODE_IDLE);
            interface_begin(LATCHING_MODE, 0);
            while(
                  (!(interface_buttons & _BV(KEY_A))) &&
                  (!(interface_buttons & _BV(BUTTON)))) sleep_mode();
            audio_stop();

            if (interface_buttons & _BV(BUTTON)) break;

            _delay_ms(1000);

            if (teaching_mode == MODE_SINGLE) {
                /* single char teaching mode */
                uint8_t *ch = buffer;

                while (*ch) {
                    uint8_t xid;
                    uint8_t tmp[] = {*ch, 0};
                
                    // we do not need to disable morse ints, because
                    // sinewave is not playing yet
                    morse_find(*ch, &xid);
                    if ((*ch) == xid) {
                        play_characters(tmp, getchar_str, FULL);
                        _delay_ms(500);
                    
                        /* it is just a single char, no need to use
                           effective speed */
                        audio_morse_init(500, speed, speed);
                        play_morse(tmp, getchar_str);
                    
			interface_begin(LATCHING_MODE, 0);
                        led_on(LED_RED);
                        audio_wait_init(6);
                        audio_play();
                        timeout(1500, _BV(KEY_A));
                        led_off(LED_RED);
                        audio_stop();
                        _delay_ms(500);
                    
                        if (interface_buttons & _BV(KEY_A)) ++correct;
                    }
                    else lesson_len--;
                
                    ++ch;
                }
            }
            /* end single char teaching mode */        


            /* long test */
            else if (teaching_mode == MODE_GROUPS) {
                audio_morse_init(500, speed, effective_speed);
                play_morse(buffer, getchar_str);
                _delay_ms(1500);

                /* convert spaces to audible spaces */
                uint8_t *tchr = buffer;
                while(*tchr) {
                    if(*tchr == SPACE) *tchr = AUDIBLE_SPACE;
                    ++tchr;
                }

                led_on(LED_RED);
                interface_begin(LATCHING_MODE, _BV(KEY_A));
                _delay_ms(500);
                play_characters(buffer, getchar_str, FULL);
                _delay_ms(1000);
                led_off(LED_RED);
                correct = interface_presses;
            }
            /* end long test */

            /* keying test */
            else if (teaching_mode == MODE_KEYING) {
                interface_iambic_key();

                /* keying uses effective_speed
                   setting effective argument to 0 enables keyer */
                audio_morse_init(500, effective_speed, 0);

                interface_begin(NONLATCHING_MODE, _BV(KEY_A));
                audio_play();
                while(!(interface_buttons & _BV(BUTTON))) sleep_mode();
                audio_stop();
                interface_standard_key();
            }
            /* end keying test */

            /* play resulting score */
            play_characters(s_correct, getchar_eep, COMPOSED);

            buffer[0] = tensinascii(correct);
            buffer[1] = onesinascii(correct);
            buffer[2] = 0;
            play_characters(buffer, getchar_str, FULL);

            play_characters(s_outof, getchar_eep, COMPOSED);

            buffer[0] = tensinascii(lesson_len);
            buffer[1] = onesinascii(lesson_len);
            buffer[2] = 0;
            play_characters(buffer, getchar_str, 0);

            /* if the success rate is higher than 95%, move to the next lesson */
            if ((lesson_len > 0) && correct >= (lesson_len - lesson_len/20)) {
                lesson_change(1);
                play_characters(s_congrats, getchar_eep, COMPOSED);
            }

            _delay_ms(3000);
        }
    }

    return 0;
}
