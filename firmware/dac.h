#ifndef __MT_dac_MS_20120309_
#define __MT_dac_MS_20120309_

/* ports */
#define DAC_DDR DDRA
#define DAC_PORT PORTA
#define DAC_CS 0

/* initializes dac during bootup */
void dac_init();

/* before anything can be done, dac has to be selected using dac_begin */
void dac_begin();
void dac_end();

/* set audio volume 0 - 255 */
void dac_volume(uint8_t vol);

/* set dac output to value*Vcc/256 */
void dac_output(uint8_t value);

/* mute/unmute but remember volume */
void dac_mute();
void dac_unmute();

#endif /* __MT_dac_MS_20120309_ */
