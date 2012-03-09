#ifndef __MT_audio_MS_20120309_
#define __MT_audio_MS_20120309_

/* This module will take care of all counters needed to play audio or
   sinewaves.

   Two timers will be used:

   Sampling timer: - feeds audio data from buffer to DAC
                   - feeds sinewave data to DAC

   Timing timer:   - when in morse mode, starts and stops
                     sampling timer to transmit morse chars

                   - overflow prepares character and space
                     and starts sampling timer to send dit/dah

                   - compare stops sampling timer to send
                     proper space

                   - this timer has to be configurable for
                     10 different lengths while still keeping
                     the transmit speed (WPM) constant

                     dit 1 mark
                     dah 3 marks
                     space after dit/dah 1 mark
                     space after symbol 3 marks
                     space after word 7 marks
*/

/* Initialize sampling timer for audio */
void audio_wav_init(uint16_t samplerate);

/* Initialize sampling timer for morse output */
void audio_morse_init(uint16_t pitch, uint8_t speed);

/* Start or stop sampling timer (including returning the line to middle V position) */
void audio_start();
void audio_stop();

/* Is the buffer full? */
uint8_t audio_buffer_full();

/* Empty the buffer */
void audio_buffer_clear();

/* Feed the buffer with wav data */
void audio_wav_data(uint8_t sample);

/* Add morse symbol to buffer */
void audio_morse_data(uint8_t len, uint8_t bitmask);

#endif /* __MT_audio_MS_20120309_ */
