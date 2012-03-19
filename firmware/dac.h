#ifndef __MT_dac_MS_20120309_
#define __MT_dac_MS_20120309_

/* ports */
#define DAC_DDR DDRB
#define DAC_PORT PORTB
#define DAC_CS 3

/* initializes dac during bootup */
void dac_init(void);

/* before anything can be done, dac has to be selected using dac_begin */
void dac_begin(void);
void dac_end(void);

/* set audio volume 0 - 255 */
void dac_volume(uint8_t vol);

/* set dac output to value*Vcc/256 */
void dac_output(uint8_t value);

/* mute/unmute but remember volume */
void dac_mute(void);
void dac_unmute(void);

#endif /* __MT_dac_MS_20120309_ */
