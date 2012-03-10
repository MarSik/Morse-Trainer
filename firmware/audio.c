#include <avr/io.h>
#include <avr/interrupt.h>

#include "audio.h"
#include "dac.h"
#include "sine.h"

typedef void (*int_routine)(void);

volatile int_routine next_sample; /* pointer to proper next sample method */
volatile int_routine morse_space; /* pointer to method which sets output to space in morse mode */

volatile uint8_t sine_id;

uint8_t sample_clock;     /* configuration for sample clock prescaler */
uint8_t wavetable_clock;  /* configuration for wavetable clock prescaler */
uint16_t dit_length;      /* number of sample clock cycles for one dit */

/* interrupt which outputs next wavetable value and resets the timer */
ISR(TIMER0_COMPA_vect)
{
    TCNT0H = 0;
    TCNT0L = 0;

    uint8_t v = sine(sine_id);

    dac_begin();
    dac_output(v);
    dac_end();

    sine_id = sine_id % sine_len;
}

/* interrupt which sets output to morse space */
ISR(TIMER1_COMPA_vect)
{
    if (morse_space) morse_space();
}

/* interrupt which loads new sample */
ISR(TIMER1_OVF_vect)
{
    next_sample();
}

/* audio buffer and it's pointers */
volatile uint8_t buffer_data[AUDIO_BUFFER_SIZE];
volatile struct _buffer_pointers {
    uint8_t first:4; /* points at the first valid item */
    uint8_t empty:4; /* points at the first available space */
} buffer;


/* outputs next wav sample and moves buffer pointer */
void sample_wav(void)
{
    if (buffer.first == buffer.empty) return;
    dac_begin();
    dac_output(buffer_data[buffer.first]);
    dac_end();
    buffer.first = (buffer.first + 1) % AUDIO_BUFFER_SIZE;
}

/* configure next dit/dah and space to sampling timer
   and when at the end of character move buffer pointer to next char */
void sample_morse(void)
{
    if (buffer.first == buffer.empty) return;

    uint8_t bitmask_id = (buffer.first + 1) % AUDIO_BUFFER_SIZE;

    struct { /* structure to save lengths of symbol and following space */
        uint8_t symbol:4;
        uint8_t space:4;
    } l;

    buffer_data[buffer.first]--;

    if (buffer_data[bitmask_id] & 0x1) l.symbol = 3; /* DAH */
    else l.symbol = 1; /* DIT */

    /* shift bitmask */
    buffer_data[bitmask_id] >>= 1;

    if ((buffer_data[buffer.first] & 0xf) == 0) {
        /* last didah, send defined space */
        l.space = buffer_data[buffer.first] >> 4;

        /* shift buffer pointer to the next char */
        buffer.first = (buffer.first + 2) % AUDIO_BUFFER_SIZE;
    }
    else l.space = 1; // one space after didah

    /* setup sampling timer to output symbol and space */
    uint16_t len;
    len = (l.space + l.symbol) * dit_length; /* length of symbol + space */
    TC1H = len >> 8;
    OCR1C = len & 0xff;

    len = l.symbol * dit_length; /* length of symbol */
    TC1H = len >> 8;
    OCR1A = len & 0xff;

    /* reset timer and start outputing sound */
    dac_unmute();
    dac_begin();
    dac_output(sine(0));
    dac_end();

    /* reset wavetable */
    TCNT0H = 0;
    TCNT0L = 0;
    sine_id = 0;
}

void output_space(void)
{
    /* stop sound, output morse space */
    dac_mute();

    dac_begin();
    dac_output(sine(0));
    dac_end();
}

/* Initialize sampling timer for audio */
void audio_wav_init(uint16_t samplerate)
{
    audio_buffer_clear();
    morse_space = NULL;
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
    uint16_t cyclespersample;

    cyclespersample = F_CPU / samplerate;
    sample_clock = 1; /* clock select */

    while (cyclespersample > 1023) {
        cyclespersample >>= 1;
        sample_clock++; //increase prescaler divider (+1 multiplies prescaler by 2)
    }

    TC1H = cyclespersample >> 8;
    OCR1C = cyclespersample & 0xff; /* cycles per one sample */

    /* enable only overflow interrupt */
    TIMSK &= ~_BV(OCIE1A);
    TIMSK |= _BV(TOIE1);


    /*
      wavetable timer is disabled
    */
    TCCR0A = 0;
    TCCR0B = 0;
    wavetable_clock = 0;
    TIMSK &= ~_BV(OCIE0A);
}

/* Initialize sampling/wavetable timer for morse output

  Morse speed definitions

  PARIS = 50 dot time segments
  12wpm = 600 dots per minute = 10 dots per second
  20wpm = 1000 dots per minute = 16.66 dots per second
*/

void audio_morse_init(uint16_t pitch, uint8_t wpm)
{
    audio_buffer_clear();
    morse_space = &output_space;
    next_sample = &sample_morse;
    sine_id = 0;

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

       with prescaler 1024 it is:
          60 wpm = 156.4  cycles per 1 dit
          30 wpm = 312.5  -"-
          20 wpm = 438.75 -"-
          10 wpm = 937.5  -"-
           3 wpm = 3125   -"-
           1 wpm = 9375   -"-
    */
    sample_clock = 0b1011; /* clock select (prescaler 1024) */
    dit_length = 9375 / wpm; /* number of timer cycles for one dit at given speed */ 

    TC1H = 0;
    OCR1A = 0; /* symbol length */

    TC1H = 0;
    OCR1C = 0; /* symbol + space */

    TC1H = 0;
    TCNT1 = 0; /* actual counter value */ 

    /* enable compareA and overflow interrupts */
    TIMSK |= _BV(OCIE1A) | _BV(TOIE1);

    /* 
       wavetable timer
       timer0 16bit mode
       compare A resets the timer to 0
    */
    TCCR0A = _BV(TCW0);
    TCCR0B = 0;

    wavetable_clock = 0b001; /* prescaler 1 */
    OCR0B = (F_CPU / (pitch * sine_len)) >> 8;
    OCR0A = (F_CPU / (pitch * sine_len)) && 0xff; /* TOP value for the wavetable timer */

    TCNT0H = 0;
    TCNT0L = 0; /* actual counter value */

    TIMSK |= _BV(OCIE0A);
}

/* unmute and start needed timers */
void audio_start()
{
    dac_unmute();

    dac_begin();
    dac_output(sine(0));
    dac_end();

    TCCR1B = sample_clock;
    TCCR0B = wavetable_clock;
}

/* Stop both timers and reset AD to middle position + mute */
void audio_stop()
{
    TCCR1B = 0;
    TCCR0B = 0;

    dac_mute();

    dac_begin();
    dac_output(sine(0));
    dac_end();
}

/* Is the buffer full? */
uint8_t inline audio_buffer_full()
{
    return buffer.first == ((buffer.empty + 1) % AUDIO_BUFFER_SIZE);
}

/* Empty the buffer */
void audio_buffer_clear()
{
    buffer.first=0;
    buffer.empty=0;
}

/* Feed the buffer with wav data */
void audio_wav_data(uint8_t sample)
{
    if (audio_buffer_full()) return;

    buffer_data[buffer.empty] = sample;
    buffer.empty = (buffer.empty + 1) % AUDIO_BUFFER_SIZE;
}

/* Add morse symbol and following space to buffer */
void audio_morse_data(uint8_t len, uint8_t bitmask, uint8_t space)
{
    if (audio_buffer_full()) return;

    buffer_data[buffer.empty] = space << 4 | len;
    buffer.empty = (buffer.empty + 1) % AUDIO_BUFFER_SIZE;

    buffer_data[buffer.empty] = bitmask;
    buffer.empty = (buffer.empty + 1) % AUDIO_BUFFER_SIZE;
}
