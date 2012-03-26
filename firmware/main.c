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

#define SINGLE_MODE 1

uint8_t teaching_lesson EEMEM = 0;
uint8_t randomizer EEMEM = 0;

/* use watchdog interrupt for software debounce */
ISR(WDT_vect)
{
    /* disable WD */
    WDTCR |= _BV(WDCE) | _BV(WDE);
    WDTCR &= ~(_BV(WDE));

    /* enable input interrupts */
    GIMSK |= _BV(PCIE1);
}

void debounce(void)
{
    /* disable input interrupts */
    GIMSK &= ~_BV(PCIE1);

    /* reset WDT timer to 64ms */
    WDTCR = _BV(WDCE) | _BV(WDE);
    WDTCR = _BV(WDP1) | _BV(WDE) | _BV(WDIE); /* interrupt has to be enabled, we need it and it prevents reboot */
}

uint8_t interface_buttons;
uint8_t interface_mask;
uint8_t interface_presses;

#define KEY_A 0
#define KEY_B 1
#define BUTTON 2

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
    srandom(1+srand);
    eeprom_write_byte(&randomizer, srand+1);

    /* initialize interface struct */
    interface_presses = 0;
    interface_buttons = 0;
    interface_mask = 0;

    /* initialize learning mode */
    teaching_mode = _BV(MODE_SINGLE);
}


/* pin change int for key */
ISR(PCINT_vect){
    if (!(PINA & _BV(PA4))) {
            debounce();
            interface_buttons |= _BV(KEY_A);
            if(interface_mask & _BV(KEY_A)) ++interface_presses;
    }

    if (!(PINA & _BV(PA5))) {
            debounce();
            interface_buttons |= _BV(KEY_B);
            if(interface_mask & _BV(KEY_B)) ++interface_presses;
    }
}

void interface_start(void)
{
    interface_buttons = 0;
    interface_mask = _BV(KEY_A);
    interface_presses = 0;
    GIMSK |= _BV(PCIE1);
    PCMSK0 |= _BV(PCINT4);
}

void interface_end(void)
{
    PCMSK0 &= ~ _BV(PCINT4);
    interface_mask = 0;
}

static uint8_t buffer[51];

uint16_t timeout(uint16_t ms)
{
    while ((ms>100) && (!(interface_buttons & _BV(KEY_A)))) {
        _delay_ms(100);
        ms -= 100;
    }

    return ms;
}

uint8_t menu_item(const uint8_t *entry)
{
    play_characters(entry, getchar_eep);
    
    led_on(LED_RED);
    interface_buttons &= ~_BV(KEY_A);
    audio_wait_init(6);
    audio_play();
    timeout(1500);
    led_off(LED_RED);
    audio_stop();
    if (interface_buttons & _BV(KEY_A)) {
        return 1;
    }
    _delay_ms(500);
    return 0;
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
        interface_start();
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
                uint8_t lesson = eeprom_read_byte(&teaching_lesson);
                ++lesson;
                eeprom_write_byte(&teaching_lesson, lesson);
                break;
            }

            if (menu_item(s_previous)) {
                uint8_t lesson = eeprom_read_byte(&teaching_lesson);
                if (lesson) {
                    --lesson;
                    eeprom_write_byte(&teaching_lesson, lesson);                
                }
                break;
            }

        }
        interface_end();

        /* teaching code */
        while(!(interface_buttons & _BV(BUTTON))) {
            uint8_t lesson = eeprom_read_byte(&teaching_lesson);

            play_characters(s_lesson, getchar_eep);

            buffer[0] = ((lesson+1) / 10) + '0';
            buffer[1] = ((lesson+1) % 10) + '0';
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
                
                    morse_find(*ch, &xid);
                    if (*ch == xid) {
                        play_characters(tmp, getchar_str);
                        _delay_ms(500);
                    
                        audio_morse_init(500, speed, speed);
                        play_morse(tmp, getchar_str);
                    
                        interface_start();
                        led_on(LED_RED);
                        audio_wait_init(6);
                        audio_play();
                        timeout(1500);
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
                interface_start();
                _delay_ms(500);
                play_characters(buffer, getchar_str);
                _delay_ms(1000);
                led_off(LED_RED);
                interface_end();
                correct = interface_presses;
            }
            /* end long test */

            /* play resultng score */
            play_characters(s_correct, getchar_eep);

            buffer[0] = '0' + (correct / 10);
            buffer[1] = '0' + (correct % 10);
            buffer[2] = 0;
            play_characters(buffer, getchar_str);

            play_characters(s_outof, getchar_eep);

            buffer[0] = '0' + (lesson_len / 10);
            buffer[1] = '0' + (lesson_len % 10);
            buffer[2] = 0;
            play_characters(buffer, getchar_str);

            /* if the success rate is higher than 95%, move to the next lesson */
            if (correct >= (lesson_len - lesson_len/20)) {
                ++lesson;
                eeprom_write_byte(&teaching_lesson, lesson);
                play_characters(s_congrats, getchar_eep);
            }

            _delay_ms(3000);
        }
    }

    return 0;
}
