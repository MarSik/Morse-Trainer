#ifndef __MT_spi_20120320__
#define __MT_spi_20120320__

#include <avr/io.h>


extern volatile uint8_t spi_pause_data;
extern volatile uint8_t spi_pause_clk;

uint8_t spi_transfer(uint8_t data);

inline void spi_pause(void)
{
    spi_pause_clk = PORTB & _BV(2);
    spi_pause_data = USIDR;
}

inline void spi_unpause(void)
{
    PORTB &= ~_BV(2);
    PORTB |= spi_pause_clk;
    USIDR = spi_pause_data;
}

#endif /* __MT_spi_20120320__ */
