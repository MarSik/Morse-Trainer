#ifndef __MT_audio_MS_20120309_
#define __MT_audio_MS_20120309_

/* This module will take care of all counters needed to play audio or
   sinewaves.

   Two timers will be used:

   Wavetable timer:
                   - frequency of OVF depends on the pitch
                   - on OVF feeds sinewave data to DAC

   Sampling timer [morse]:
                   - starts and stops sampling timer to
                     transmit didahs/spaces

                   - OVF prepares next character and space timing
                     and starts wavetable timer to send dit/dah

                   - COMPARE stops sampling timer to send
                     proper space

                   - this timer has to be configurable for
                     10 different lengths while still keeping
                     the transmit speed (WPM) constant

                     dit 1 mark
                     dah 3 marks
                     space after dit/dah 1 mark
                     space after symbol 3 marks
                     space after word 7 marks

   Sampling timer [wav]:
                   - 16kHz
                   - on OVF feeds wav next data byte from buffer to DAC
                   - moves buffer pointers to next byte
*/

//volatile uint8_t buffer_state;
//!!! SHARED with dac interface, use only 4 high bits
#define buffer_state GPIOR2

#define DIT_LAST 4 /* last symbol sent from keyer mode was a dot */
#define PRESS_BREAKS 5 /* pressing button from mask breaks play methods */
#define BUFFER_FINISHED 6
#define BUFFER_READ_LAST 7

/* mu-law decoder consts */
#define AUDIO_MU_LAW_BITS 14
#define AUDIO_MU_LAW_FRAC_BITS (AUDIO_MU_LAW_BITS - 8)
#define AUDIO_MU_LAW_FRAC_MASK ((1 << AUDIO_MU_LAW_FRAC_BITS) - 1)
#define AUDIO_OVERSAMPLING 6

/* Initialize sampling timer for audio */
void audio_wav_init(uint16_t samplerate);

/* Initialize sampling/wavetable timer for morse output
   supported pitch 200 - 1000 Hz 
   supported wpm 6 - 60 WPM (when wpm == effective_wpm)
   
   for Farnsworth timing the supported wpm/effective_wpm pairs have
   to obey the above limitations and also the following formula:

   1024 > 38*floor(586/effective_wpm) - 28*floor(586/wpm)

   floor means to strip the partial part and keep only the whole part of the number

   Setting the effective_wpm to 0 switches to morse keying mode
*/
void audio_morse_init(uint16_t pitch, uint8_t wpm, uint8_t effective_wpm);

/*
  Enables blinking mode, speed is number of blinks per second
 */
void audio_wait_init(uint8_t speed);

/* Prepare first sample/morse character to play and start needed timers */
#define audio_play audio_start
void audio_start(void);

/* Stop both timers and reset AD to middle position */
void audio_stop(void);

/* Is the buffer full? */
uint8_t audio_buffer_full(void);

/* Is the buffer empty */
uint8_t audio_buffer_empty(void);

/* After last sample was played, there was nothing to play in the buffer */
uint8_t audio_buffer_finished(void);

/* Empty the buffer */
void audio_buffer_clear(void);

/* Feed the buffer with wav data */
uint8_t audio_wav_data(uint8_t sample);

/* Add morse symbol and following space to buffer */
uint8_t audio_morse_data(uint8_t len, uint8_t bitmask, uint8_t space);

#endif /* __MT_audio_MS_20120309_ */
