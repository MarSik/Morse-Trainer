#include <avr/io.h>
#include <avr/interrupt.h>

#include "audio.h"
#include "dac.h"
#include "sine.h"
#include "morse.h"
#include "leds.h"

typedef void (*int_routine)(void);

volatile int_routine next_sample; /* pointer to proper next sample method */

uint8_t sample_clock;     /* configuration for sample clock prescaler */
uint16_t dit_length;      /* number of sample clock cycles for one dit */
uint16_t dit_length_farnsworth; /* number of sample clock cycles for one dit
                                   when outputting last space in farnsworth mode */

/* interrupt which sets output to morse space */
void inline output_space(void);

ISR(TIMER1_COMPA_vect)
{
    output_space();
}

/* interrupt which loads new sample */
ISR(TIMER1_OVF_vect)
{
    next_sample();
}

/* audio buffer and it's pointers */
/* number of buffer bytes, must be even number */
#define AUDIO_BUFFER_SIZE 16
static volatile uint8_t buffer_data[AUDIO_BUFFER_SIZE];
static volatile struct _buffer_pointers {
    volatile uint8_t first:4; /* points at the first valid item */
    volatile uint8_t empty:4; /* points at the first available space */
    volatile uint8_t finished:1; /* there was nothing to play when next_sample was started */
    volatile uint8_t chardits:6; /* dits needed for this character */
} buffer_ctrl;

void inline enable_morse_int(void)
{
    TIMSK |= _BV(OCIE1A) | _BV(TOIE1);
}

void inline disable_morse_int(void)
{
    TIMSK &= ~_BV(OCIE1A) & ~_BV(TOIE1);
}

void sample_wait(void)
{
    led_toggle(PA5);
}

/* outputs next wav sample and moves buffer pointer */
void sample_wav(void)
{
    if (buffer_ctrl.first == buffer_ctrl.empty) {
        buffer_ctrl.finished = 1;
        return;
    }
    dac_begin();
    dac_output(buffer_data[buffer_ctrl.first]);
    dac_end();
    buffer_ctrl.first = (buffer_ctrl.first + 1) % AUDIO_BUFFER_SIZE;
}

/* configure next dit/dah and space to sampling timer
   and when at the end of character move buffer pointer to next char */
void sample_morse(void)
{
    if (buffer_ctrl.first == buffer_ctrl.empty) {
        buffer_ctrl.finished = 1;
        led_on(LED_RED);
        return;
    }

    uint8_t bitmask_id = (buffer_ctrl.first + 1) % AUDIO_BUFFER_SIZE;

    struct { /* structure to save lengths of symbol and following space */
        uint8_t symbol:4;
        uint8_t space:4;
        uint8_t last:1;
    } l;

    if (buffer_data[bitmask_id] & 0x1) l.symbol = DAH_LEN; /* DAH */
    else l.symbol = DIT_LEN; /* DIT */

    /* shift bitmask and decrement counter */
    buffer_data[bitmask_id] >>= 1;
    buffer_data[buffer_ctrl.first]--;
    
    /* last didah */
    l.last = ((buffer_data[buffer_ctrl.first] & 0x0f) == 0);

    if (l.last) {
        /* last didah, send defined space */
        l.space = buffer_data[buffer_ctrl.first] >> 4;

        /* shift buffer pointer to the next char */
        buffer_ctrl.first = (buffer_ctrl.first + 2) % AUDIO_BUFFER_SIZE;
    }
    else {
        l.space = SPACE_LEN; // one space after didah
    }

    /* setup sampling timer to output symbol and space */
    uint16_t len;
    len = l.symbol * dit_length; /* length of symbol */

    cli();
    TC1H = len >> 8;
    OCR1A = len & 0xff;
    sei();

    
    /* count number of dits and compute space to have proper
       effective farnsworth speed:
       
       character goes fast and space fills the time up
       
       ! supported only for 20 wpm letters (or slower)   !
       ! and 12 wpm effective (or faster)                !
       ! where the ending space fits into 10 bit number. !

       worst case, eight dashes (8*3 + 7) and long space (7) = 38 dits
       12 wpm = 48 ticks per dit, 20 wpm = 29 ticks per dit
       
       38 * 48 = 1824 (12 wpm length)
       28 * 29 = 812  (already sent using 20 wpm, seven dashes and spaces)
       the last part has to be 1012 ticks long (87 ticks for dash and 925 for space)
    */
    buffer_ctrl.chardits += l.space + l.symbol; 

    if (l.last && dit_length_farnsworth!=dit_length) {
        uint16_t efflen = buffer_ctrl.chardits * dit_length_farnsworth;

        /* add the prolonged space at the end of current symbol */
        len = len + efflen - (buffer_ctrl.chardits - l.space) * dit_length;
        buffer_ctrl.chardits = 0;
    }
    else len = (l.symbol+l.space) * dit_length; /* length of symbol + space */

    cli();
    TC1H = len >> 8;
    OCR1C = len & 0xff;
    sei();

    /* start outputing sound */
    sine_start();
    led_on(LED_GRN);
}

void inline output_space(void)
{
    /* stop sound, output morse space */
    sine_stop();
    led_off(LED_GRN);
}

/* Initialize sampling timer for audio */
void audio_wav_init(uint16_t samplerate)
{
    audio_buffer_clear();
    next_sample = &sample_wav;

    /*
       sampling timer
       timer1 in 10bit mode

       normal counter mode
       pins disconnected
    */
    TCCR1A = 0;
    TCCR1C = 0;
    TCCR1D = 0;

    PLLCSR = 0; /* disable PLL */

    /*
      compute settings for sample timer
      
      first, we need number of cpu cycles per sample
      then we need to set prescaler to reduce the number under 1024 (10bits)
    */
    uint16_t cyclespersample = F_CPU / samplerate;
    sample_clock = 1; /* clock select */

    while (cyclespersample > 1023) {
        cyclespersample >>= 1;
        sample_clock++; //increase prescaler divider (+1 multiplies prescaler by 2)
    }

    cli();
    TC1H = cyclespersample >> 8;
    OCR1C = cyclespersample & 0xff; /* cycles per one sample */
    sei();

    /* enable only overflow interrupt */
    TIMSK &= ~_BV(OCIE1A);
    TIMSK |= _BV(TOIE1);


    /*
      sine output is disabled
    */
    sine_deinit();
}

/* Initialize sampling/wavetable timer for morse output

  Morse speed definitions

  PARIS = 50 dot time segments
  12wpm = 600 dots per minute = 10 dots per second
  20wpm = 1000 dots per minute = 16.66 dots per second
*/

void audio_morse_init(uint16_t pitch, uint8_t wpm, uint8_t effective_wpm)
{
    audio_buffer_clear();
    next_sample = &sample_morse;

    /*
       sampling timer
       timer1 in 10bit mode

       normal counter mode
       pins disconnected
    */
    TCCR1A = 0;
    TCCR1C = 0;
    TCCR1D = 0;

    PLLCSR = 0; /* disable PLL */

    /* we need to set the timer so have ten possible lengths, where each is
       as long as needed by the specified WPM.

       1 WPM = 50 dits per minute
       So compute number of cycles per dit (has to be 3 or more)
       
       max speed will be 60 wpm = 1 dit per  20ms = 160000 cycles
       min speed will be  5 wpm = 1 dit per 240ms = 1920000 cycles

       with prescaler 16384 it is:
          60 wpm = 10   cycles per 1 dit
          30 wpm = 20     -"-
          20 wpm = 29     -"-
          10 wpm = 59     -"-
           6 wpm = 98     -"-
           3 wpm = 195    -"-
           1 wpm = 586    -"-

       timer has to be able to count to ten dits (dash + long pause)
       as it has 10bit precision, the maximum dit can be only 102 cycles long

       minimum supported speed is then 6 wpm
    */
    sample_clock = 0b1111; /* clock select (prescaler 16384) */
    dit_length = 586 / wpm; /* number of timer cycles for one dit at given speed */ 
    dit_length_farnsworth = 586 / effective_wpm; /* number of timer cycles for one dit at given speed */ 

    cli();
    TC1H = 0;
    OCR1A = 0; /* symbol length */

    TC1H = 0;
    OCR1C = 0; /* symbol + space */

    TC1H = 0;
    TCNT1 = 0; /* actual counter value */ 
    sei();

    /* enable compareA and overflow interrupts */
    TIMSK |= _BV(OCIE1A) | _BV(TOIE1);

    /* initialize sinewave generator */
    sine_init(pitch);
}

void audio_wait_init()
{
    next_sample = &sample_wait;
    sample_clock = 0b1111;

    cli();
    TC1H = 0;
    OCR1C = 122; /* cycles per one sample */
    sei();

    /* enable only overflow interrupt */
    TIMSK &= ~_BV(OCIE1A);
    TIMSK |= _BV(TOIE1);
}

/* unmute, load first sample and start needed timers */
void audio_start()
{
    next_sample();

    buffer_ctrl.finished = 0;
    TCCR1B = sample_clock;
}

/* Stop both timers and reset AD to middle position */
void audio_stop()
{
    TCCR1B = 0;
    TCCR0B = 0;

    sine_stop();
    led_off(LED_GRN);
    led_off(LED_RED);
}

/* Is the buffer full? */
uint8_t audio_buffer_full(uint8_t needed)
{
    return (((buffer_ctrl.empty + needed) % AUDIO_BUFFER_SIZE) == buffer_ctrl.first);
}

uint8_t audio_buffer_empty(void)
{
    return (buffer_ctrl.first == buffer_ctrl.empty);
}

uint8_t audio_buffer_finished(void)
{
    return buffer_ctrl.finished;
}

/* Empty the buffer */
void audio_buffer_clear()
{
    buffer_ctrl.first = 0;
    buffer_ctrl.empty = 0;
    buffer_ctrl.chardits = 0;
}

/* Feed the buffer with wav data */
uint8_t audio_wav_data(uint8_t sample)
{
    if (audio_buffer_full(1)) return 0;

    buffer_ctrl.finished = 0;

    buffer_data[buffer_ctrl.empty] = sample;

    buffer_ctrl.empty = (buffer_ctrl.empty + 1) % AUDIO_BUFFER_SIZE;

    return 1;
}

/* Add morse symbol and following space to buffer */
uint8_t audio_morse_data(uint8_t len, uint8_t bitmask, uint8_t space)
{
    if (audio_buffer_full(2)) return 0;

    buffer_ctrl.finished = 0;

    uint8_t maskidx = (buffer_ctrl.empty + 1) % AUDIO_BUFFER_SIZE;

    buffer_data[buffer_ctrl.empty] = (space << 4) | (len & 0xf);
    buffer_data[maskidx] = bitmask;

    /* guard the crit section */
    disable_morse_int();
    buffer_ctrl.empty = (buffer_ctrl.empty + 2) % AUDIO_BUFFER_SIZE;
    enable_morse_int();

    return 2;
}
