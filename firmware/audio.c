#include <avr/io.h>
#include <avr/interrupt.h>

#include "audio.h"
#include "dac.h"
#include "sine.h"

typedef void (*int_routine)(void);

volatile int_routine next_sample; /* pointer to proper next sample method */

uint8_t sample_clock;     /* configuration for sample clock prescaler */
uint16_t dit_length;      /* number of sample clock cycles for one dit */

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
volatile uint8_t buffer_data[AUDIO_BUFFER_SIZE];
volatile struct _buffer_pointers {
    uint8_t first:4; /* points at the first valid item */
    uint8_t empty:4; /* points at the first available space */
    uint8_t finished:1; /* there was nothing to play when next_sample was started */
} buffer;


/* outputs next wav sample and moves buffer pointer */
void sample_wav(void)
{
    if (buffer.first == buffer.empty) {
        buffer.finished = 1;
        return;
    }
    dac_begin();
    dac_output(buffer_data[buffer.first]);
    dac_end();
    buffer.first = (buffer.first + 1) % AUDIO_BUFFER_SIZE;
}

/* configure next dit/dah and space to sampling timer
   and when at the end of character move buffer pointer to next char */
void sample_morse(void)
{
    if (buffer.first == buffer.empty) {
        buffer.finished = 1;
        return;
    }

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
    len = l.symbol * dit_length; /* length of symbol */

    cli();
    TC1H = len >> 8;
    OCR1A = len & 0xff;
    sei();

    len = (l.space + l.symbol) * dit_length; /* length of symbol + space */
    cli();
    TC1H = len >> 8;
    OCR1C = len & 0xff;
    sei();

    /* start outputing sound */
    dac_unmute();
    sine_start();
}

void inline output_space(void)
{
    /* stop sound, output morse space */
    dac_mute();
    sine_stop();
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

void audio_morse_init(uint16_t pitch, uint8_t wpm)
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

/* unmute, load first sample and start needed timers */
void audio_start()
{
    dac_unmute();
    next_sample();

    buffer.finished = 0;
    TCCR1B = sample_clock;
}

/* Stop both timers and reset AD to middle position + mute */
void audio_stop()
{
    TCCR1B = 0;
    TCCR0B = 0;

    dac_mute();
    sine_stop();
}

/* Is the buffer full? */
uint8_t audio_buffer_full(uint8_t needed)
{
    return ((buffer.empty + needed) % AUDIO_BUFFER_SIZE) == buffer.first;
}

uint8_t audio_buffer_empty(void)
{
    return buffer.first == buffer.empty;
}

uint8_t audio_buffer_finished(void)
{
    return buffer.finished;
}

/* Empty the buffer */
void audio_buffer_clear()
{
    buffer.first = 0;
    buffer.empty = 0;
}

/* Feed the buffer with wav data */
uint8_t audio_wav_data(uint8_t sample)
{
    if (audio_buffer_full(1)) return 0;

    buffer.finished = 0;

    buffer_data[buffer.empty] = sample;
    buffer.empty = (buffer.empty + 1) % AUDIO_BUFFER_SIZE;

    return 1;
}

/* Add morse symbol and following space to buffer */
uint8_t audio_morse_data(uint8_t len, uint8_t bitmask, uint8_t space)
{
    if (audio_buffer_full(2)) return 0;

    buffer.finished = 0;

    buffer_data[buffer.empty] = (space << 4) | (len & 0xf);
    buffer.empty = (buffer.empty + 1) % AUDIO_BUFFER_SIZE;

    buffer_data[buffer.empty] = bitmask;
    buffer.empty = (buffer.empty + 1) % AUDIO_BUFFER_SIZE;

    return 2;
}
