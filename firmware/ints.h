#ifndef __MT_ints_h_20120309_
#define __MT_ints_h_20120309_

/*
  This module will change interrupt routines on the fly to support
  switching between different timer modes (wav, morse) and SPI targets (Flash, ADC)
*/

void set_SPI(void (*f)());
void set_TimerOVF(void (*f)());
void set_TimerCOMP(void (*f)());

#endif /* __MT_ints_h_20120309_ */
