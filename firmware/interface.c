#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include "interface.h"
#include "dac.h"

uint8_t interface_mask;
volatile uint8_t interface_presses;

#define ROTARY_LOOKUP_PREV 0b0100000110000010
#define ROTARY_LOOKUP_NEXT 0b0010100000010100

static void debounce(void);

void interface_init(void)
{
    /* setup KEY ports */
    DDRA |= _BV(PA5); // middle of the jack connector
    DDRA &= ~_BV(PA4); // tip of the jack connector
    PORTA |= _BV(PA4) | _BV(PA5);

    /* setup ADC pin PA2 */

    /* setup rotary button + A and B sensors */
    DDRB &= 0b00001111;
    PORTB |= 0b01110000;

    /* initialize interface struct */
    interface_presses = 0;
    interface_buttons = 0;
    interface_mask = 0;
}

/* pin change int for key */
ISR(PCINT_vect){
    if (!(PINA & _BV(PA4))) {
            interface_buttons |= _BV(KEY_A);
            if(interface_mask & _BV(KEY_A)) ++interface_presses;
            debounce();
    }

    if (!(PINA & _BV(PA5))) {
            interface_buttons |= _BV(KEY_B);
            if(interface_mask & _BV(KEY_B)) ++interface_presses;
            debounce();
    }

    /* get rotary vector[oldB oldA B A]*/
    uint8_t r = ((ROTARY_PIN >> ROTARY_SHIFT) & 0b11) | (interface_buttons & 0b1100);
    
    if (ROTARY_LOOKUP_NEXT & _BV(r)) {
        if (volume_flags & _BV(HOLD_VOLUME)) interface_buttons |= _BV(ROTARY_NEXT);
        else dac_louder();
        debounce();
    }

    else if (ROTARY_LOOKUP_PREV & _BV(r)) {
        if (volume_flags & _BV(HOLD_VOLUME)) interface_buttons |= _BV(ROTARY_PREV);
        else dac_quieter();
        debounce();
    }

    /* save old rotary */
    interface_buttons &= ~0b1100;
    interface_buttons |= (r << 2) & 0b1100;
}

void interface_begin(void)
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


/* use watchdog interrupt for software debounce */
ISR(WDT_vect)
{
    /* disable WD */
    WDTCR |= _BV(WDCE) | _BV(WDE);
    WDTCR &= ~(_BV(WDE));

    /* enable input interrupts */
    GIMSK |= _BV(PCIE1);
}

static void debounce(void)
{
    /* disable input interrupts */
    GIMSK &= ~_BV(PCIE1);

    /* reset WDT timer to 64ms */
    WDTCR = _BV(WDCE) | _BV(WDE);
    WDTCR = _BV(WDP1) | _BV(WDE) | _BV(WDIE); /* interrupt has to be enabled, we need it and it prevents reboot */
}

uint16_t timeout(uint16_t ms)
{
    while ((ms>100) && (!(interface_buttons & _BV(KEY_A)))) {
        _delay_ms(100);
        ms -= 100;
    }

    return ms;
}
