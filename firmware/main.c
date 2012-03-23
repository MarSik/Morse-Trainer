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

uint8_t teaching_lesson EEMEM = 0;
uint8_t randomizer EEMEM = 0;

/* use watchdog interrupt for software debounce */
ISR(WDT_vect)
{
    /* disable WD */
    WDTCR |= _BV(WDCE) | _BV(WDE);
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
    srandom(1+srand);
    eeprom_write_byte(&randomizer, srand+1);
}


static uint8_t buffer[21];

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
        lesson_new(lesson, 20, &speed, &effective_speed, buffer);

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
