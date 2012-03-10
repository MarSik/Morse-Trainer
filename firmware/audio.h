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
                   - 8kHz
                   - on OVF feeds wav next data byte from buffer to DAC
                   - moves buffer pointers to next byte
*/

/* number of buffer bytes, must be even number */
#define AUDIO_BUFFER_SIZE 32

/* Initialize sampling timer for audio */
void audio_wav_init(uint16_t samplerate);

/* Initialize sampling/wavetable timer for morse output */
void audio_morse_init(uint16_t pitch, uint8_t speed);

/* Prepare first sample/morse character to play, unmute and start needed timers */
void audio_start();

/* Stop both timers and reset AD to middle position + mute */
void audio_stop();

/* Is the buffer full? */
uint8_t audio_buffer_full();

/* Empty the buffer */
void audio_buffer_clear();

/* Feed the buffer with wav data */
void audio_wav_data(uint8_t sample);

/* Add morse symbol and following space to buffer */
void audio_morse_data(uint8_t len, uint8_t bitmask, uint8_t space);

#endif /* __MT_audio_MS_20120309_ */