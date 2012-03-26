#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "sine.h"
#include "dac.h"

uint8_t wavetable_clock;  /* configuration for wavetable clock prescaler */
volatile uint8_t sine_id;

/* one quarter (0 - 90deg) of sine function */
uint8_t sine_table[] EEMEM = {
    0,
    11,
    22,
    33,
    46,
    56,
    65,
    76,
    85,
    93,
    100,
    108,
    113,
    118,
    122,
    125,
    126
};

#define sine_table_len 16
#define sine_len (sine_table_len * 4)
#define SINE_STEP 2

uint8_t sine_table_get(uint8_t id)
{
    return eeprom_read_byte(sine_table + id)/2;
}

/* get sine value transposed on 0 - 255 (center at 128),
   angle values are represented by a number between 0 - 63 (step 5.625 deg) */
uint8_t sine(uint8_t id)
{
    uint8_t q = (id >> 4) & 0b11; /* divide by number of table entries */
    id = id & 0x0f; /* remove quadrant id from table id */

    if(q == 0) return 128 + sine_table_get(id);
    else if (q == 1) return 129 + sine_table_get(sine_table_len - 1 - id);
    else if (q == 2) return 128 - sine_table_get(id);
    else if (q == 3) return 127 - sine_table_get(sine_table_len - 1 - id);

    return 0;
}

/* interrupt which outputs next wavetable value and resets the timer */
ISR(TIMER0_COMPA_vect)
{
    TCNT0H = 0;
    TCNT0L = 0;

    uint8_t v = sine(sine_id);

    dac_begin();
    dac_output(v);
    dac_end();

    sine_id = (sine_id + SINE_STEP) % sine_len;
}

void sine_init(uint16_t pitch)
{
    /* 
       wavetable timer
       timer0 16bit mode
    */
    TCCR0A = _BV(TCW0);

    wavetable_clock = 0b001; /* prescaler 1 */

    OCR0B = (F_CPU / (pitch * (sine_len / SINE_STEP))) >> 8;
    OCR0A = (F_CPU / (pitch * (sine_len / SINE_STEP))) & 0xff; /* TOP value for the wavetable timer */
    
    cli();
    TCNT0H = 0;
    TCNT0L = 0; /* actual counter value */
    sei();

    /* enable interrupt */
    TIMSK |= _BV(OCIE0A);

    /* keep the clock stopped for now */
    TCCR0B = 0;
}

void sine_deinit()
{
    wavetable_clock = 0;
    TCCR0B = 0;

    /* disable interrupt */
    TIMSK &= ~_BV(OCIE0A);
}

void sine_start()
{
    sine_id = 0;
    uint8_t sine0 = sine(sine_id++);

    dac_begin();
    dac_fadein();
    dac_output(sine0);
    dac_end();

    /* reset wavetable */
    cli();
    TCNT0H = 0;
    TCNT0L = 0;
    sei();

    /* start the clock */
    TCCR0B = wavetable_clock;
}

void sine_stop()
{
    /* stop the clock */
    TCCR0B = 0;

    dac_begin();
    dac_output(sine(0));
    dac_end();
}
